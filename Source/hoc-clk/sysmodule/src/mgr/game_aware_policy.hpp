#pragma once

#include <algorithm>
#include <cstdint>

namespace gameAware {

// Pure policy: all process identity and monotonic time come from the caller.
enum class Error { Success, NotApplication, NotOwner, BadArgs };

struct Decision {
    bool suppressed;
    bool changed;
    bool ownerGone;
    std::uint64_t profileTid;
};

class Policy {
    bool armed_ = false;
    bool lastApply_ = false;
    bool suppressed_ = false;
    std::uint64_t ownerPid_ = 0;
    std::uint64_t applyUntil_ = 0;
    std::uint32_t generation_ = 0;

    void Clear() {
        armed_ = false;
        ownerPid_ = 0;
        applyUntil_ = 0;
        lastApply_ = false;
        ++generation_;
    }

public:
    bool Armed() const { return armed_; }
    bool Applying(std::uint64_t nowNs) const { return armed_ && nowNs < applyUntil_; }
    std::uint32_t Generation() const { return generation_; }

    Error Arm(std::uint64_t callerPid, std::uint64_t applicationPid,
              std::uint64_t actualProgramId, std::uint64_t programId,
              std::uint32_t clientAbi, std::uint32_t reserved, bool sendPid) {
        if (!sendPid || clientAbi != 1 || reserved != 0)
            return Error::BadArgs;
        if (!callerPid || callerPid != applicationPid || actualProgramId != programId)
            return Error::NotApplication;
        if (!armed_ || ownerPid_ != callerPid) {
            armed_ = true;
            ownerPid_ = callerPid;
            applyUntil_ = 0;
            lastApply_ = false;
            ++generation_;
        }
        return Error::Success;
    }

    Error SetApply(std::uint64_t nowNs, std::uint64_t callerPid, std::uint8_t apply,
                   std::uint32_t leaseMs, bool reservedZero, bool sendPid) {
        if (!sendPid || apply > 1 || !reservedZero)
            return Error::BadArgs;
        if (!armed_ || callerPid != ownerPid_)
            return Error::NotOwner;
        if (lastApply_ != (apply != 0)) {
            ++generation_;
            lastApply_ = apply != 0;
        }
        applyUntil_ = apply ? nowNs + std::clamp(leaseMs, 500U, 5000U) * 1000000ULL : 0;
        return Error::Success;
    }

    Error Disarm(std::uint64_t callerPid, bool sendPid) {
        if (!sendPid)
            return Error::BadArgs;
        if (!armed_)
            return Error::Success;
        if (callerPid != ownerPid_)
            return Error::NotOwner;
        Clear();
        return Error::Success;
    }

    Decision Tick(std::uint64_t nowNs, std::uint64_t applicationPid, std::uint64_t applicationId) {
        const bool ownerGone = armed_ && ownerPid_ != applicationPid;
        if (ownerGone)
            Clear();
        const bool suppress = armed_ && nowNs >= applyUntil_;
        const bool changed = suppress != suppressed_;
        suppressed_ = suppress;
        return {suppress, changed, ownerGone, suppress ? 0 : applicationId};
    }

    // Display is deliberately outside the title-profile gate.
    static std::uint64_t ProfileTitleId(bool display, std::uint64_t profileTid, std::uint64_t applicationId) {
        return display ? applicationId : profileTid;
    }
};

} // namespace gameAware

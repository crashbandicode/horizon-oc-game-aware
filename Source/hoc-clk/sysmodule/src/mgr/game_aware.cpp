#include "game_aware.hpp"
#include "../file/config.hpp"
#include "../file/file_utils.hpp"
#include "../util/lockable_mutex.h"
#include <hocclk/errors.h>

namespace gameAware {
namespace {
    LockableMutex mutex;
    Policy policy;

    u64 NowNs() { return armTicksToNs(armGetSystemTick()); }

    Result ToResult(Error error) {
        switch (error) {
            case Error::Success: return 0;
            case Error::NotApplication: return HOCCLK_ERROR(GameAwareNotApplication);
            case Error::NotOwner: return HOCCLK_ERROR(GameAwareNotOwner);
            case Error::BadArgs: return HOCCLK_ERROR(GameAwareBadArgs);
        }
        return HOCCLK_ERROR(Generic);
    }
}

HocClkGameAwareInfo GetInfo() {
    return {0x41474f48, 1, 1, 5000, static_cast<u32>(config::GetConfigValue(HocClkConfigValue_PollingIntervalMs))};
}

Result Arm(bool sendPid, u64 callerPid, const HocClkGameAwareArmArgs &args, HocClkGameAwareArmResult &out) {
    std::scoped_lock lock{mutex};
    if (!sendPid || args.clientAbi != 1 || args.reserved != 0)
        return HOCCLK_ERROR(GameAwareBadArgs);
    u64 applicationPid = 0;
    u64 programId = 0;
    if (R_FAILED(pmdmntGetApplicationProcessId(&applicationPid)) ||
        R_FAILED(pminfoGetProgramId(&programId, callerPid)))
        return HOCCLK_ERROR(GameAwareNotApplication);
    const auto before = policy.Generation();
    const auto error = policy.Arm(callerPid, applicationPid, programId, args.programId,
                                  args.clientAbi, args.reserved, sendPid);
    out = {policy.Generation(), 0};
    if (error == Error::Success && before != policy.Generation())
        fileUtils::LogLine("[mgr] Title profile gate armed: pid=%lu generation=%u", callerPid, policy.Generation());
    return ToResult(error);
}

Result SetApply(bool sendPid, u64 callerPid, const HocClkGameAwareSetApplyArgs &args, HocClkGameAwareState &out) {
    std::scoped_lock lock{mutex};
    const u64 now = NowNs();
    const auto before = policy.Generation();
    const auto error = policy.SetApply(now, callerPid, args.apply, args.leaseMs,
                                      args.reserved[0] == 0 && args.reserved[1] == 0 && args.reserved[2] == 0, sendPid);
    out = {static_cast<u8>(policy.Armed()), static_cast<u8>(policy.Applying(now)), 0, policy.Generation()};
    if (error == Error::Success && before != policy.Generation())
        fileUtils::LogLine("[mgr] Title profile gate request: apply=%u seq=%u generation=%u", args.apply, args.seq, policy.Generation());
    return ToResult(error);
}

Result Disarm(bool sendPid, u64 callerPid) {
    std::scoped_lock lock{mutex};
    const auto before = policy.Generation();
    const auto error = policy.Disarm(callerPid, sendPid);
    if (error == Error::Success && before != policy.Generation())
        fileUtils::LogLine("[mgr] Title profile gate disarmed: pid=%lu generation=%u", callerPid, policy.Generation());
    return ToResult(error);
}

Decision Tick(u64 applicationPid, u64 applicationId) {
    std::scoped_lock lock{mutex};
    const auto decision = policy.Tick(NowNs(), applicationPid, applicationId);
    if (decision.ownerGone)
        fileUtils::LogLine("[mgr] Title profile gate disarmed: owner gone generation=%u", policy.Generation());
    return decision;
}
} // namespace gameAware

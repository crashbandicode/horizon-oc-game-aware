#include "../sysmodule/src/mgr/game_aware_policy.hpp"
#include <hocclk/ipc.h>
#include <hocclk/errors.h>
#include <cassert>
#include <iostream>

using gameAware::Policy;
using gameAware::Error;
constexpr std::uint64_t tid = 0x01006a800016e000ULL;
constexpr std::uint64_t ms = 1000000;

int main() {
    static_assert(HOCCLK_IPC_API_VERSION == 2);
    static_assert(HocClkIpcCmd_GameAwareGetInfo == 100 && HocClkIpcCmd_GameAwareDisarm == 103);
    static_assert(HOCCLK_ERROR(Generic) == 0x184);
    static_assert(HOCCLK_ERROR(GameAwareNotApplication) == 0xc984);
    static_assert(HOCCLK_ERROR(GameAwareNotOwner) == 0xcb84);
    static_assert(HOCCLK_ERROR(GameAwareBadArgs) == 0xcd84);
    Policy p;
    auto arm = [&](std::uint64_t pid = 10) { return p.Arm(pid, pid, tid, tid, 1, 0, true); };
    auto apply = [&](std::uint8_t want, std::uint32_t lease = 3000, std::uint64_t now = 100 * ms) {
        return p.SetApply(now, 10, want, lease, true, true);
    };
    auto d = p.Tick(0, 10, tid);
    for (unsigned m = 0; m < HocClkModule_EnumMax; ++m)
        assert(Policy::ProfileTitleId(m == HocClkModule_Display, d.profileTid, tid) == tid);
    assert(!d.suppressed && !d.changed && !d.ownerGone && p.Generation() == 0);
    assert(apply(0) == Error::NotOwner);
    assert(p.Disarm(99, true) == Error::Success && p.Generation() == 0);
    assert(p.Arm(11, 10, tid, tid, 1, 0, true) == Error::NotApplication);
    assert(p.Arm(10, 10, tid + 1, tid, 1, 0, true) == Error::NotApplication);
    assert(p.Arm(0, 0, tid, tid, 1, 0, true) == Error::NotApplication);
    assert(p.Arm(10, 10, tid, tid, 2, 0, true) == Error::BadArgs);
    assert(p.Arm(10, 10, tid, tid, 1, 1, true) == Error::BadArgs);
    assert(p.Arm(10, 10, tid, tid, 1, 0, false) == Error::BadArgs);
    assert(arm() == Error::Success && p.Generation() == 1);
    d = p.Tick(100 * ms, 10, tid);
    assert(d.suppressed && d.changed && d.profileTid == 0);
    for (unsigned m = 0; m < HocClkModule_EnumMax; ++m)
        assert(Policy::ProfileTitleId(m == HocClkModule_Display, d.profileTid, tid) == (m == HocClkModule_Display ? tid : 0));
    assert(arm() == Error::Success && p.Generation() == 1);
    assert(apply(0) == Error::Success && p.Generation() == 1);
    assert(apply(0) == Error::Success && p.Generation() == 1);
    assert(apply(1) == Error::Success && p.Generation() == 2);
    d = p.Tick(101 * ms, 10, tid);
    assert(!d.suppressed && d.changed && d.profileTid == tid && p.Applying(101 * ms));
    assert(arm() == Error::Success && p.Generation() == 2 && p.Applying(101 * ms));
    assert(apply(1) == Error::Success && p.Generation() == 2);
    assert(!p.Tick(3099 * ms, 10, tid).suppressed);
    d = p.Tick(3100 * ms, 10, tid);
    assert(d.suppressed && d.changed && p.Generation() == 2);
    assert(apply(1, 0) == Error::Success && p.Generation() == 2);
    assert(p.Applying(599 * ms) && !p.Applying(600 * ms));
    assert(apply(1, 99999) == Error::Success);
    assert(p.Applying(5099 * ms) && !p.Applying(5100 * ms));
    assert(apply(2) == Error::BadArgs);
    assert(p.SetApply(0, 10, 0, 0, false, true) == Error::BadArgs);
    assert(p.SetApply(0, 10, 0, 0, true, false) == Error::BadArgs);
    assert(p.SetApply(0, 11, 0, 0, true, true) == Error::NotOwner);
    assert(p.Disarm(11, true) == Error::NotOwner);
    assert(p.Disarm(10, false) == Error::BadArgs);
    assert(p.Generation() == 2);
    assert(apply(0) == Error::Success && p.Generation() == 3);
    assert(apply(0) == Error::Success && p.Generation() == 3);
    assert(p.Disarm(10, true) == Error::Success && p.Generation() == 4);
    assert(p.Disarm(10, true) == Error::Success && p.Generation() == 4);
    assert(!p.Tick(6000 * ms, 10, tid).suppressed);
    assert(arm() == Error::Success && p.Generation() == 5);
    d = p.Tick(6000 * ms, 11, tid);
    assert(d.ownerGone && !d.suppressed && d.profileTid == tid && !p.Armed() && p.Generation() == 6);
    assert(apply(1) == Error::NotOwner);
    assert(!p.Tick(6001 * ms, 11, tid).ownerGone);
    assert(arm(11) == Error::Success && p.Generation() == 7);
    assert(arm(12) == Error::Success && p.Generation() == 8);
    assert(p.Tick(0, 12, tid).suppressed);
    d = p.Tick(1, 0, 0);
    assert(d.ownerGone && d.changed && !p.Armed() && p.Generation() == 9);
    std::cout << "PASS: game-aware policy lifecycle, ownership, leases, generation, validation, module scope, wire layouts\n";
}

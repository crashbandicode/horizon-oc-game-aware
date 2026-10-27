#pragma once

#include <hocclk/ipc.h>
#include <switch.h>
#include "game_aware_policy.hpp"

namespace gameAware {
    HocClkGameAwareInfo GetInfo();
    Result Arm(bool sendPid, u64 callerPid, const HocClkGameAwareArmArgs &args, HocClkGameAwareArmResult &out);
    Result SetApply(bool sendPid, u64 callerPid, const HocClkGameAwareSetApplyArgs &args, HocClkGameAwareState &out);
    Result Disarm(bool sendPid, u64 callerPid);
    Decision Tick(u64 applicationPid, u64 applicationId);
}

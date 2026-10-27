/*
 * Copyright (c) Souldbminer, Lightos_ and Horizon OC Contributors
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/* --------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <p-sam@d3vs.net>, <natinusala@gmail.com>, <m4x@m4xw.net>
 * wrote this file. As long as you retain this notice you can do whatever you
 * want with this stuff. If you meet any of us some day, and you think this
 * stuff is worth it, you can buy us a beer in return.  - The sys-clk authors
 * --------------------------------------------------------------------------
 */


#pragma once

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include "board.h"
#include "clock_manager.h"

#define HOCCLK_IPC_API_VERSION 2
#define HOCCLK_IPC_SERVICE_NAME "hoc:clk"

enum HocClkIpcCmd
{
    HocClkIpcCmd_GetApiVersion = 0,
    HocClkIpcCmd_GetVersionString = 1,
    HocClkIpcCmd_GetCurrentContext = 2,
    HocClkIpcCmd_Exit = 3,
    HocClkIpcCmd_GetProfileCount = 4,
    HocClkIpcCmd_GetProfiles = 5,
    HocClkIpcCmd_SetProfiles = 6,
    HocClkIpcCmd_SetEnabled = 7,
    HocClkIpcCmd_SetOverride = 8,
    HocClkIpcCmd_GetConfigValues = 9,
    HocClkIpcCmd_SetConfigValues = 10,
    HocClkIpcCmd_GetFreqList = 11,
    HocClkIpcCmd_SetKipData = 12,
    HocClkIpcCmd_GetKipData = 13,
    HocClkIpcCmd_RequestGpuVoltage = 14,
    HocClkIpcCmd_GameAwareGetInfo = 100,
    HocClkIpcCmd_GameAwareArm = 101,
    HocClkIpcCmd_GameAwareSetApply = 102,
    HocClkIpcCmd_GameAwareDisarm = 103,
};

typedef struct
{
    uint64_t tid;
    HocClkTitleProfileList profiles;
} HocClkIpc_SetProfiles_Args;

typedef struct
{
    HocClkModule module;
    uint32_t hz;
} HocClkIpc_SetOverride_Args;

typedef struct
{
    HocClkModule module;
    uint32_t maxCount;
} HocClkIpc_GetFreqList_Args;

typedef struct {
    u32 voltage;
    u32 hz;
} HocClkIpc_RequestGpuVoltage_Args;

/* Game-aware extension v1: natural alignment, explicit reserved bytes. */
typedef struct {
    uint32_t magic;
    uint16_t version;
    uint16_t caps;
    uint32_t maxLeaseMs;
    uint32_t pollMs;
} HocClkGameAwareInfo;

typedef struct {
    uint64_t programId;
    uint32_t clientAbi;
    uint32_t reserved;
} HocClkGameAwareArmArgs;

typedef struct {
    uint32_t generation;
    uint32_t reserved;
} HocClkGameAwareArmResult;

typedef struct {
    uint8_t apply;
    uint8_t reserved[3];
    uint32_t leaseMs;
    uint32_t seq;
} HocClkGameAwareSetApplyArgs;

typedef struct {
    uint8_t armed;
    uint8_t applying;
    uint16_t reserved;
    uint32_t generation;
} HocClkGameAwareState;

#ifdef __cplusplus
#define HOCCLK_WIRE_ASSERT(c) static_assert(c, #c)
#else
#define HOCCLK_WIRE_ASSERT(c) _Static_assert(c, #c)
#endif
HOCCLK_WIRE_ASSERT(sizeof(HocClkGameAwareInfo) == 16);
HOCCLK_WIRE_ASSERT(offsetof(HocClkGameAwareInfo, magic) == 0);
HOCCLK_WIRE_ASSERT(offsetof(HocClkGameAwareInfo, version) == 4);
HOCCLK_WIRE_ASSERT(offsetof(HocClkGameAwareInfo, caps) == 6);
HOCCLK_WIRE_ASSERT(offsetof(HocClkGameAwareInfo, maxLeaseMs) == 8);
HOCCLK_WIRE_ASSERT(offsetof(HocClkGameAwareInfo, pollMs) == 12);
HOCCLK_WIRE_ASSERT(sizeof(HocClkGameAwareArmArgs) == 16);
HOCCLK_WIRE_ASSERT(offsetof(HocClkGameAwareArmArgs, programId) == 0);
HOCCLK_WIRE_ASSERT(offsetof(HocClkGameAwareArmArgs, clientAbi) == 8);
HOCCLK_WIRE_ASSERT(offsetof(HocClkGameAwareArmArgs, reserved) == 12);
HOCCLK_WIRE_ASSERT(sizeof(HocClkGameAwareArmResult) == 8);
HOCCLK_WIRE_ASSERT(offsetof(HocClkGameAwareArmResult, generation) == 0);
HOCCLK_WIRE_ASSERT(offsetof(HocClkGameAwareArmResult, reserved) == 4);
HOCCLK_WIRE_ASSERT(sizeof(HocClkGameAwareSetApplyArgs) == 12);
HOCCLK_WIRE_ASSERT(offsetof(HocClkGameAwareSetApplyArgs, apply) == 0);
HOCCLK_WIRE_ASSERT(offsetof(HocClkGameAwareSetApplyArgs, reserved) == 1);
HOCCLK_WIRE_ASSERT(offsetof(HocClkGameAwareSetApplyArgs, leaseMs) == 4);
HOCCLK_WIRE_ASSERT(offsetof(HocClkGameAwareSetApplyArgs, seq) == 8);
HOCCLK_WIRE_ASSERT(sizeof(HocClkGameAwareState) == 8);
HOCCLK_WIRE_ASSERT(offsetof(HocClkGameAwareState, armed) == 0);
HOCCLK_WIRE_ASSERT(offsetof(HocClkGameAwareState, applying) == 1);
HOCCLK_WIRE_ASSERT(offsetof(HocClkGameAwareState, reserved) == 2);
HOCCLK_WIRE_ASSERT(offsetof(HocClkGameAwareState, generation) == 4);
#undef HOCCLK_WIRE_ASSERT

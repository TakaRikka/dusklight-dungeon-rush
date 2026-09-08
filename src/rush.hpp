#pragma once

#include "mods/svc/config.h"
#include <os/OSTime.h>

#define DEFAULT_TIME_SETTING 99999999

namespace rush::game {
enum {
    RUN_INIT_e = -1,

    FOREST_TEMPLE_e,
    GORON_MINES_e,
    LAKEBED_TEMPLE_e,
    ARBITERS_GROUNDS_e,
    SNOWPEAK_RUINS_e,
    TEMPLE_OF_TIME_e,
    CITY_IN_THE_SKY_e,
    PALACE_OF_TWILIGHT_e,
    HYRULE_CASTLE_e,

    DUNGEON_COUNT_e,
};

enum {
    RUN_READY_e,
    RUN_START_e,
    RUN_END_e,
};

void cvarSetBestTimes();
void cvarSetSplitBestTimes(int i_dungeonNo);
OSTime cvarGetBestTime(ConfigVarHandle handle);
void cvarClearBestTimes();
ModResult cvarRegisterBestTimes(ModError* error);

void startRun();
void resetRun();
void checkDungeonTransition();

extern ConfigVarHandle g_cvarTotalTime;
extern ConfigVarHandle g_cvarSplitTimestamps[DUNGEON_COUNT_e];

extern u8 s_runState;
extern int s_currentDungeon;
extern OSTime s_startTimestamp;
extern OSTime s_endTimestamp;
extern OSTime s_splitTimestamps[DUNGEON_COUNT_e];
}
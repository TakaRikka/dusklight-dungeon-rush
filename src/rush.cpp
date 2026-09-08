#include <mods/svc/log.hpp>

#include "rush.hpp"
#include "timer.hpp"

#include "c/c_damagereaction.h"
#include "d/actor/d_a_alink.h"
#include "d/d_com_inf_game.h"
#include "d/d_item.h"
#include "d/d_meter2_info.h"

namespace rush::game {
u8 s_runState = RUN_READY_e;
int s_currentDungeon = RUN_INIT_e;

OSTime s_startTimestamp = 0;
OSTime s_endTimestamp = 0;
OSTime s_splitTimestamps[DUNGEON_COUNT_e] = {};

ConfigVarHandle g_cvarTotalTime = 0;
ConfigVarHandle g_cvarSplitTimestamps[DUNGEON_COUNT_e] = {};

void setupSavefileForRun() {
    dComIfGs_init();
    cDmr_SkipInfo = 0;
    dComIfGs_setLife(12);

    execItemGet(dItemNo_WEAR_KOKIRI_e);
    execItemGet(dItemNo_WEAR_ZORA_e);
    execItemGet(dItemNo_MASTER_SWORD_e);
    execItemGet(dItemNo_HYLIA_SHIELD_e);
    dMeter2Info_setShield(dItemNo_HYLIA_SHIELD_e, false);
    execItemGet(dItemNo_KANTERA_e);
    execItemGet(dItemNo_HVY_BOOTS_e);
    dComIfGs_setItem(SLOT_15, dItemNo_BOMB_BAG_LV1_e);

    dComIfGs_onEventBit(0x0C10);  // Midna on Z
    dComIfGs_onEventBit(0x0D04);  // Transform
    dComIfGs_onEventBit(0x0501);  // Midna Charge
    dComIfGs_onEventBit(0x2904);  // Ending Blow
    dComIfGs_onEventBit(0x1E08);  // MDH Completed (Midna healthy)
    dComIfGs_onEventBit(0x0C08);  // Midna took sword / shield
    dComIfGs_onEventBit(0x4308);  // Senses
    dComIfGs_onTransformLV(3);
}

void addNewHeart() {
    dComIfGs_setMaxLife(dComIfGs_getMaxLife() + 5);
    dComIfGs_setLife(dComIfGs_getMaxLife());
}

void setNextDungeon(int dungeonNo) {
    static struct {
        char name[8];
        s16 point;
        s8 roomNo;
    } l_dungeonInfo[] = {
        {"D_MN05", 0, 22},
        {"D_MN04", 1, 1},
        {"D_MN01", 0, 0},
        {"D_MN10", 0, 0},
        {"D_MN11", 2, 0},
        {"D_MN06", 0, 0},
        {"D_MN07", 2, 0},
        {"D_MN08", 10, 0},
        {"D_MN09", 0, 11},
    };

    auto& info = l_dungeonInfo[dungeonNo];
    mods::log::debug("doing dungeon transition... -> {} (point: {}, room: {})\n",
        info.name, info.point, info.roomNo);

    dComIfGp_setNextStage(info.name, info.point, info.roomNo, -1, 0.0f, 0, 1, 0, 0, 1, 3);
    if (s_currentDungeon >= 0) {
        s_splitTimestamps[s_currentDungeon] = OSGetTime();
        cvarSetSplitBestTimes(s_currentDungeon);
    }
    s_currentDungeon++;
}

void checkDungeonTransition() {
    switch (s_currentDungeon) {
        case RUN_INIT_e:
            setNextDungeon(FOREST_TEMPLE_e);
            break;
        case FOREST_TEMPLE_e:
            if (dComIfGs_isCollectCrystal(0)) {
                setNextDungeon(GORON_MINES_e);
                addNewHeart();
            }
            break;
        case GORON_MINES_e:
            if (dComIfGs_isCollectCrystal(1)) {
                setNextDungeon(LAKEBED_TEMPLE_e);
                addNewHeart();
            }
            break;
        case LAKEBED_TEMPLE_e:
            if (dComIfGs_isCollectCrystal(2)) {
                setNextDungeon(ARBITERS_GROUNDS_e);
                addNewHeart();
            }
            break;
        case ARBITERS_GROUNDS_e:
            if (dComIfGs_isCollectMirror(0)) {
                setNextDungeon(SNOWPEAK_RUINS_e);
                addNewHeart();
            }
            break;
        case SNOWPEAK_RUINS_e:
            if (dComIfGs_isCollectMirror(1)) {
                setNextDungeon(TEMPLE_OF_TIME_e);
                addNewHeart();
            }
            break;
        case TEMPLE_OF_TIME_e:
            if (dComIfGs_isCollectMirror(2)) {
                setNextDungeon(CITY_IN_THE_SKY_e);
                addNewHeart();
            }
            break;
        case CITY_IN_THE_SKY_e:
            if (dComIfGs_isCollectMirror(3)) {
                setNextDungeon(PALACE_OF_TWILIGHT_e);
                addNewHeart();
            }
            break;
        case PALACE_OF_TWILIGHT_e:
            if (dComIfGs_isCollectCrystal(3)) {
                setNextDungeon(HYRULE_CASTLE_e);
                addNewHeart();
            }
            break;
        default:
            break;
    }
}

void startRun() {
    if (s_runState == RUN_READY_e) {
        s_currentDungeon = RUN_INIT_e;
        setupSavefileForRun();
        s_runState = RUN_START_e;
        s_startTimestamp = OSGetTime();
        timer::g_rush_timer.create();
        timer::g_rush_timer.show();
        mods::log::info("run started");
    }
}

void resetRun() {
    s_runState = RUN_READY_e;
    s_currentDungeon = RUN_INIT_e;
    s_startTimestamp = 0;
    s_endTimestamp = 0;

    for (auto& split : s_splitTimestamps) {
        split = 0;
    }

    timer::g_rush_timer.hide();
}

ModResult cvarRegisterBestTimes(ModError* error) {
    ConfigVarDesc cvarDesc = CONFIG_VAR_DESC_INIT;
    cvarDesc.name = "timeTotal";
    cvarDesc.type = CONFIG_VAR_INT;
    cvarDesc.default_int = DEFAULT_TIME_SETTING;
    if (svc_config->register_var(mod_ctx, &cvarDesc, &g_cvarTotalTime) != MOD_OK) {
        return mods::set_error(error, MOD_ERROR, "failed to register cvar!");
    }

    for (int i = 0; i < DUNGEON_COUNT_e; ++i) {
        static const char* splitConfigNames[] = {
            "timeForest",
            "timeMines",
            "timeLakebed",
            "timeArbiters",
            "timeSnowpeak",
            "timeTemple",
            "timeCity",
            "timePalace",
            "timeHyrule",
        };

        ConfigVarDesc cvarDesc = CONFIG_VAR_DESC_INIT;
        cvarDesc.name = splitConfigNames[i];
        cvarDesc.type = CONFIG_VAR_INT;
        cvarDesc.default_int = DEFAULT_TIME_SETTING;
        if (svc_config->register_var(mod_ctx, &cvarDesc, &g_cvarSplitTimestamps[i]) != MOD_OK) {
            return mods::set_error(error, MOD_ERROR, "failed to register cvar!");
        }
    }

    return MOD_OK;
}

void cvarSetBestTimes() {
    OSTime diff = s_endTimestamp - s_startTimestamp;

    OSTime cvarTotalTime = 0;
    svc_config->get_int(mod_ctx, g_cvarTotalTime, &cvarTotalTime);

    // check to see that new time is less than saved time before updating
    if (diff < cvarTotalTime || cvarTotalTime == DEFAULT_TIME_SETTING) {
        mods::log::info("new best total time detected, saving...");
        svc_config->set_int(mod_ctx, g_cvarTotalTime, diff);
    }

    for (int i = 0; i < DUNGEON_COUNT_e; ++i) {
        diff = 0;
        if (i == 0 && s_splitTimestamps[i] != 0) {
            diff = s_splitTimestamps[i] - s_startTimestamp;
        } else if (i > 0 && s_splitTimestamps[i] != 0) {
            diff = s_splitTimestamps[i] - s_splitTimestamps[i - 1];
        }

        OSTime cvarSplitTime = 0;
        svc_config->get_int(mod_ctx, g_cvarSplitTimestamps[i], &cvarSplitTime);

        // check to see that new time is less than saved time before updating
        if (diff < cvarSplitTime || cvarSplitTime == DEFAULT_TIME_SETTING) {
            mods::log::info("new best split time detected, saving...");
            svc_config->set_int(mod_ctx, g_cvarSplitTimestamps[i], diff);
        }
    }
}

void cvarSetSplitBestTimes(int i_dungeonNo) {
    OSTime diff = 0;
    if (i_dungeonNo == 0 && s_splitTimestamps[i_dungeonNo] != 0) {
        diff = s_splitTimestamps[i_dungeonNo] - s_startTimestamp;
    } else if (i_dungeonNo > 0 && s_splitTimestamps[i_dungeonNo] != 0) {
        diff = s_splitTimestamps[i_dungeonNo] - s_splitTimestamps[i_dungeonNo - 1];
    }

    OSTime cvarSplitTime = 0;
    svc_config->get_int(mod_ctx, g_cvarSplitTimestamps[i_dungeonNo], &cvarSplitTime);

    // check to see that new time is less than saved time before updating
    if (diff < cvarSplitTime || cvarSplitTime == DEFAULT_TIME_SETTING) {
        mods::log::info("new best split time detected, saving...");
        svc_config->set_int(mod_ctx, g_cvarSplitTimestamps[i_dungeonNo], diff);
    }
}

OSTime cvarGetBestTime(ConfigVarHandle handle) {
    OSTime value = DEFAULT_TIME_SETTING;
    if (handle == 0 || svc_config->get_int(mod_ctx, handle, &value) != MOD_OK) {
        return DEFAULT_TIME_SETTING;
    }

    return value;
}

void cvarClearBestTimes() {
    svc_config->set_int(mod_ctx, g_cvarTotalTime, DEFAULT_TIME_SETTING);
    for (int i = 0; i < DUNGEON_COUNT_e; ++i) {
        svc_config->set_int(mod_ctx, g_cvarSplitTimestamps[i], DEFAULT_TIME_SETTING);
    }
}
}
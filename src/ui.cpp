#include "mods/svc/ui.h"

#include <mods/svc/log.hpp>

#include "ui.hpp"
#include "rush.hpp"

#include "d/d_com_inf_game.h"
#include "m_Do/m_Do_Reset.h"

namespace rush::ui {
UiWindowHandle g_windowHandle = 0;

void onWindowClosed(ModContext*, UiWindowHandle, void*) {
    g_windowHandle = 0;
}

ModResult buildDungeonRushTab(ModContext* ctx, UiWindowHandle window, UiElementHandle left_pane, UiElementHandle right_pane, void* user_data, ModError* out_error) {
    auto pane_add_button = [](const char* label, UiElementHandle pane, const char* help_txt, UiPressedFn pressCb) {
        UiControlDesc control = UI_CONTROL_DESC_INIT;
        control.kind = UI_CONTROL_BUTTON;
        control.label = label;
        control.help_rml = help_txt;
        control.on_pressed = pressCb;
        svc_ui->pane_add_control(mod_ctx, pane, &control, nullptr);
    };

    pane_add_button("Start", left_pane,
    "Start a new Dungeon Rush run!",
    [](ModContext* ctx, void* user_data) {
        game::startRun();
        svc_ui->window_close(ctx, g_windowHandle);
    });

    pane_add_button("Reset", left_pane,
    "Reset your current Dungeon Rush run and return to the Title Screen.",
    [](ModContext* ctx, void* user_data) {
        game::resetRun();
        mDoRst_resetCallBack(0, nullptr);
        svc_ui->window_close(ctx, g_windowHandle);
    });

    svc_ui->pane_add_section(ctx, left_pane, "Best Times");

    std::string bestTimesString = "<b>Best Dungeon Times</b><br/>";
    for (int i = 0; i < game::DUNGEON_COUNT_e; ++i) {
        static const char* dungeonNames[] = {
            "Forest Temple",
            "Goron Mines",
            "Lakebed Temple",
            "Arbiter's Grounds",
            "Snowpeak Ruins",
            "Temple of Time",
            "City in the Sky",
            "Palace of Twilight",
            "Hyrule Castle",
        };

        OSTime timestamp = game::cvarGetBestTime(game::g_cvarSplitTimestamps[i]);
        if (timestamp == DEFAULT_TIME_SETTING)
            timestamp = 0;

        OSCalendarTime segmentTime;
        OSTicksToCalendarTime(timestamp, &segmentTime);

        bestTimesString += fmt::format(
            "{:02d}:{:02d}:{:02d}.{:03d} {}<br/>",
            segmentTime.hour, segmentTime.min, segmentTime.sec, segmentTime.msec,
            dungeonNames[i]);
    }

    OSTime timestamp = game::cvarGetBestTime(game::g_cvarTotalTime);
    if (timestamp == DEFAULT_TIME_SETTING)
        timestamp = 0;
    OSCalendarTime totalTime;
    OSTicksToCalendarTime(timestamp, &totalTime);
    bestTimesString += "<br/><b>Best Run</b>";
    bestTimesString += fmt::format("<br/>{:02d}:{:02d}:{:02d}.{:03d}", totalTime.hour, totalTime.min, totalTime.sec, totalTime.msec);

    pane_add_button("Show Best Times", left_pane,
    bestTimesString.c_str(),
    [](ModContext* ctx, void* user_data) {});

    pane_add_button("Clear Best Times", left_pane,
    "Clear all saved Best Times.",
    [](ModContext* ctx, void* user_data) {
        game::cvarClearBestTimes();
    });

    svc_ui->pane_add_section(ctx, left_pane, "Debug");

    pane_add_button("Force End Run", left_pane,
    "Forcibly set run status to end state",
    [](ModContext* ctx, void* user_data) {
        game::s_runState = game::RUN_END_e;
        game::s_endTimestamp = OSGetTime();
        mods::log::debug("debug force run end");
    });

    pane_add_button("Load Next Dungeon", left_pane,
    "Mark dungeon as complete and go to next one",
    [](ModContext* ctx, void* user_data) {
        static int level = 0;
        switch (level) {
        case 0: dComIfGs_onCollectCrystal(0); level++; break;
        case 1: dComIfGs_onCollectCrystal(1); level++; break;
        case 2: dComIfGs_onCollectCrystal(2); level++; break;
        case 3: dComIfGs_onCollectMirror(0); level++; break;
        case 4: dComIfGs_onCollectMirror(1); level++; break;
        case 5: dComIfGs_onCollectMirror(2); level++; break;
        case 6: dComIfGs_onCollectMirror(3); level++; break;
        case 7: dComIfGs_onCollectCrystal(3); level++; break;
        default: break;
        }
    });

    pane_add_button("Force Save Best Times", left_pane,
    "Force save best time cvars to config.json",
    [](ModContext* ctx, void* user_data) {
        game::cvarSetBestTimes();
    });

    return MOD_OK;
}

void displayWindow() {
    if (g_windowHandle == 0) {
        UiTabDesc tabs[1] = {UI_TAB_DESC_INIT};
        tabs[0].title = "Dungeon Rush";
        tabs[0].build = buildDungeonRushTab;

        UiWindowDesc desc = UI_WINDOW_DESC_INIT;
        desc.tabs = tabs;
        desc.tab_count = 1;
        desc.on_closed = onWindowClosed;

        if (svc_ui->window_push(mod_ctx, &desc, &g_windowHandle) != MOD_OK) {
            mods::log::info("failed to open window");
        }
    }
}
}
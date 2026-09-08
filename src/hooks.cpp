#include <mods/svc/hook.hpp>
#include <mods/svc/log.hpp>

#include "hooks.hpp"
#include "timer.hpp"
#include "rush.hpp"

#include "d/actor/d_a_alink.h"
#include "d/d_meter2.h"

DEFINE_HOOK(&dMeter2_c::_draw, dMeter2_draw);
DEFINE_HOOK(&daAlink_c::procGanonFinishInit, daAlink_procGanonFinishInit);

namespace rush::hooks {
namespace {
void hookPostMeter2Draw(ModContext*, void*, void*, void*) {
    if (game::s_runState != game::RUN_READY_e && timer::g_rush_timer.mpDlst_RushTimer != nullptr) {
        OSTime time = OSGetTime();
        if (game::s_runState == game::RUN_END_e)
            time = game::s_endTimestamp;

        timer::g_rush_timer.mpDlst_RushTimer->setTime(OSTicksToMilliseconds(time - game::s_startTimestamp));
        timer::g_rush_timer.draw();
    }
}

void hookPostProcGanonFinishInit(ModContext*, void*, void*, void*) {
    if (game::s_runState == game::RUN_START_e) {
        game::s_runState = game::RUN_END_e;
        game::s_endTimestamp = OSGetTime();
        game::cvarSetBestTimes();
        mods::log::info("run ended");
    }
}
}

ModResult install() {
    if (mods::hook::add_post<dMeter2_draw>(svc_hook, hookPostMeter2Draw) != MOD_OK) {
        mods::log::info("failed to hook dMeter2::_draw");
        return MOD_ERROR;
    }

    if (mods::hook::add_post<daAlink_procGanonFinishInit>(svc_hook, hookPostProcGanonFinishInit) != MOD_OK) {
        mods::log::info("failed to hook daAlink_c::procGanonFinishInit");
        return MOD_ERROR;
    }

    return MOD_OK;
}

ModResult uninstall() {
    mods::hook::uninstall<dMeter2_draw>(svc_hook);
    mods::hook::uninstall<daAlink_procGanonFinishInit>(svc_hook);
    return MOD_OK;
}
}

#include "mods/service.hpp"
#include "mods/svc/hook.h"
#include "mods/svc/log.h"
#include "mods/svc/ui.h"
#include "mods/svc/resource.h"

#include <mods/svc/log.hpp>

#include "timer.hpp"
#include "hooks.hpp"
#include "rush.hpp"
#include "ui.hpp"

#include "m_Do/m_Do_controller_pad.h"

DEFINE_MOD();
IMPORT_SERVICE(LogService, svc_log);
IMPORT_SERVICE(HookService, svc_hook);
IMPORT_SERVICE(UiService, svc_ui);
IMPORT_SERVICE(ResourceService, svc_resource);
IMPORT_SERVICE(ConfigService, svc_config);

extern "C" {
MOD_EXPORT ModResult mod_initialize(ModError* error) {
    ModResult rt = rush::game::cvarRegisterBestTimes(error);
    if (rt != MOD_OK) {
        return rt;
    }

    rt = rush::hooks::install();
    if (rt != MOD_OK) {
        return rt;
    }

    mods::log::info("dungeon rush mod initialized");
    return MOD_OK;
}

MOD_EXPORT ModResult mod_update(ModError*) {
    if (mDoCPd_c::getHoldR(PAD_1) && mDoCPd_c::getTrigB(PAD_1)) {
        rush::ui::displayWindow();
    }

    if (rush::game::s_runState == rush::game::RUN_START_e) {
        rush::game::checkDungeonTransition();
    }

    return MOD_OK;
}

MOD_EXPORT ModResult mod_shutdown(ModError*) {
    rush::timer::g_rush_timer.delete_();

    rush::game::g_cvarTotalTime = 0;
    for (auto& handle : rush::game::g_cvarSplitTimestamps) {
        handle = 0;
    }

    rush::hooks::uninstall();

    mods::log::info("dungeon rush mod unloaded");
    return MOD_OK;
}
}

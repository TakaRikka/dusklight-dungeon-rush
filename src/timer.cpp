#include "timer.hpp"

#include "mods/svc/resource.h"
#include <mods/svc/log.hpp>

#include "JSystem/J2DGraph/J2DGrafContext.h"
#include "JSystem/J2DGraph/J2DOrthoGraph.h"
#include "JSystem/JKernel/JKRExpHeap.h"
#include "f_op/f_op_msg_mng.h"
#include "d/d_meter2_info.h"

namespace rush::timer {
    dRushTimer_c g_rush_timer;
}

BOOL dRushTimer_c::create() {
    mpHeap = fopMsgM_createExpHeap(0x11000, nullptr);
    JKRHEAP_NAME(mpHeap, "dRushTimer_c::mpHeap");
    JKRHeap* prev_heap = mDoExt_setCurrentHeap(mpHeap);

    if (mpHeap != nullptr) {
        mpDlst_RushTimer = JKR_NEW dDlst_RushTimer_c();
        if (mpDlst_RushTimer == nullptr) {
            return FALSE;
        }

        if (!mpDlst_RushTimer->setScreen()) {
            return FALSE;
        }

        mDoExt_setCurrentHeap(prev_heap);
    }

    return TRUE;
}

void dRushTimer_c::draw() {
    if (mpDlst_RushTimer != nullptr)
        dComIfGd_set2DOpaTop(mpDlst_RushTimer);
}

void dRushTimer_c::delete_() {
    if (mpHeap != nullptr) {
        JKRHeap* prev_heap = mDoExt_setCurrentHeap(mpHeap);

        mpDlst_RushTimer->deleteScreen();

        JKR_DELETE(mpDlst_RushTimer);
        mpDlst_RushTimer = nullptr;

        fopMsgM_destroyExpHeap(mpHeap);
        mpHeap = nullptr;
        mDoExt_setCurrentHeap(prev_heap);
    }
}

BOOL dRushTimer_c::dDlst_RushTimer_c::setScreen() {
    mpScrn = JKR_NEW J2DScreen();
    if (mpScrn == nullptr) {
        return FALSE;
    }

    ResourceBuffer buf = RESOURCE_BUFFER_INIT;
    if (svc_resource->load(mod_ctx, "rush_timer.blo", &buf) != MOD_OK) {
        mods::log::error("failed to load rush_timer.blo");
        return FALSE;
    }

    JSUMemoryInputStream stream(buf.data, buf.size);
    mpScrn->setPriority(&stream, 0x20000, dComIfGp_getMain2DArchive());
    dPaneClass_showNullPane(mpScrn);

    mpTimerParent = JKR_NEW CPaneMgr(mpScrn, MULTI_CHAR('p_time'), 2, nullptr);

    mpTimerParent->translate(280.0f, 35.0f);
    mpTimerParent->hide();

    mpTimerText[0][0] = mpScrn->search(MULTI_CHAR('p_d1'));
    mpTimerText[0][1] = mpScrn->search(MULTI_CHAR('p_d1_s'));
    mpTimerText[1][0] = mpScrn->search(MULTI_CHAR('p_d2'));
    mpTimerText[1][1] = mpScrn->search(MULTI_CHAR('p_d2_s'));

    mpTimerText[2][0] = mpScrn->search(MULTI_CHAR('p_d3'));
    mpTimerText[2][1] = mpScrn->search(MULTI_CHAR('p_d3_s'));
    mpTimerText[3][0] = mpScrn->search(MULTI_CHAR('p_d4'));
    mpTimerText[3][1] = mpScrn->search(MULTI_CHAR('p_d4_s'));

    mpTimerText[4][0] = mpScrn->search(MULTI_CHAR('p_d5'));
    mpTimerText[4][1] = mpScrn->search(MULTI_CHAR('p_d5_s'));
    mpTimerText[5][0] = mpScrn->search(MULTI_CHAR('p_d6'));
    mpTimerText[5][1] = mpScrn->search(MULTI_CHAR('p_d6_s'));

    mpTimerText[6][0] = mpScrn->search(MULTI_CHAR('p_d7'));
    mpTimerText[6][1] = mpScrn->search(MULTI_CHAR('p_d7_s'));
    mpTimerText[7][0] = mpScrn->search(MULTI_CHAR('p_d8'));
    mpTimerText[7][1] = mpScrn->search(MULTI_CHAR('p_d8_s'));

    svc_resource->free(mod_ctx, &buf);
    return TRUE;
}

void dRushTimer_c::dDlst_RushTimer_c::setTime(int i_timeMs) {
    int sec = (i_timeMs / 1000);
    int ms = i_timeMs - sec * 1000;
    int min = sec / 60;
    int hour = min / 60;

    sec = sec - min * 60;
    min = min % 60;

    if (hour > 99) {
        hour = 99;
        min = 59;
        sec = 59;
        ms = 999;
    }

    for (int i = 0; i < 2; i++) {
        if (mpTimerText[0][i] != nullptr) {
            changeNumberTexture(mpTimerText[0][i], hour / 10);
        }

        if (mpTimerText[1][i] != nullptr) {
            changeNumberTexture(mpTimerText[1][i], hour % 10);
        }

        if (mpTimerText[2][i] != nullptr) {
            changeNumberTexture(mpTimerText[2][i], min / 10);
        }

        if (mpTimerText[3][i] != nullptr) {
            changeNumberTexture(mpTimerText[3][i], min % 10);
        }

        if (mpTimerText[4][i] != nullptr) {
            changeNumberTexture(mpTimerText[4][i], sec / 10);
        }

        if (mpTimerText[5][i] != nullptr) {
            changeNumberTexture(mpTimerText[5][i], sec % 10);
        }

        if (mpTimerText[6][i] != nullptr) {
            changeNumberTexture(mpTimerText[6][i], ms / 100);
        }

        if (mpTimerText[7][i] != nullptr) {
            changeNumberTexture(mpTimerText[7][i], (ms % 100) / 10);
        }
    }
}

void dRushTimer_c::dDlst_RushTimer_c::changeNumberTexture(J2DPane* i_pane, int i_num) {
    if (i_num < 0 || i_num >= 10) {
        i_num = 0;
    }

    static_cast<J2DPicture*>(i_pane)->changeTexture(dMeter2Info_getNumberTextureName(i_num), 0);
}

void dRushTimer_c::dDlst_RushTimer_c::deleteScreen() {
    JKR_DELETE(mpScrn);
    mpScrn = nullptr;

    JKR_DELETE(mpTimerParent);
    mpTimerParent = nullptr;
}

void dRushTimer_c::dDlst_RushTimer_c::draw() {
    J2DGrafContext* graf_ctx = dComIfGp_getCurrentGrafPort();
    graf_ctx->setup2D();

    mpScrn->draw(0.0f, 0.0f, graf_ctx);
}
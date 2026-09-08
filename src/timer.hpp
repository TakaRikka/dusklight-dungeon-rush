#pragma once

#include "d/d_pane_class.h"
#include "d/d_timer.h"

class dRushTimer_c {
public:
    BOOL create();
    void draw();
    void delete_();

    void show() {
        if (mpDlst_RushTimer != nullptr)
            mpDlst_RushTimer->show();
    }

    void hide() {
        if (mpDlst_RushTimer != nullptr)
            mpDlst_RushTimer->hide();
    }

    class dDlst_RushTimer_c : public dDlst_base_c {
    public:
        dDlst_RushTimer_c() {}
        BOOL setScreen();
        void setTime(int i_timeMs);
        void changeNumberTexture(J2DPane* i_pane, int i_num);
        void deleteScreen();

        void show() {
            if (mpTimerParent != nullptr)
                mpTimerParent->show();
        }

        void hide() {
            if (mpTimerParent != nullptr)
                mpTimerParent->hide();
        }

        virtual ~dDlst_RushTimer_c() {}
        virtual void draw();

        J2DScreen* mpScrn;
        CPaneMgr* mpTimerParent;
        J2DPane* mpTimerText[8][2];
    };

    JKRExpHeap* mpHeap;
    dDlst_RushTimer_c* mpDlst_RushTimer;
};

namespace rush::timer {
extern dRushTimer_c g_rush_timer;
}
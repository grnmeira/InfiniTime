#include "displayapp/screens/TennisScoreTracker.h"

class TennisMatchModel {
public:
    struct Event {
        enum class Type {
            POINT_ME,
            POINT_OPP,
            GAME,
            SET,
            MATCH
        };
        Type type;
    };
    enum class GameScore {
        LOVE,
        S_15,
        S_30,
        S_40
        AD
    };
    struct MatchSummary {
        GameScore currentGame[2];
        unsigned char currentSet[2];
        unsigned char currentTieBreak[2];
    };
    void registerPointForMe() {
        events.emplace_back(Event{ Event::Type::POINT_ME });
    }
    void registerPointForOpp() {
        events.emplace_back(Event{ Event::Type::POINT_OPP });
    }
private:
    std::vector<Event> events;
};


namespace Pinetime::Applications::Screens {
    TennisScoreTracker::TennisScoreTracker(Pinetime::Applications::DisplayApp* app)
    : screens{
            app,
            0,
            {[this]() -> std::unique_ptr <Screen> {
                return createHomeScreen();
            },
             [this]() -> std::unique_ptr <Screen> {
                 return createMatchScreen();
             }},
            Screens::ScreenListModes::RightLeft
    },
    model(std::make_unique<TennisMatchModel>())
    {
    }

    TennisScoreTracker::~TennisScoreTracker() {
        lv_obj_clean(lv_scr_act());
    }

    bool TennisScoreTracker::OnTouchEvent(TouchEvents event) {
        if (event == TouchEvents::Tap) {
            model->registerPointForMe();
            return true;
        } else if (event == TouchEvents::DoubleTap) {
            model->registerPointForOpp();
            return true;
        }
        return screens.OnTouchEvent(event);
    }

    std::unique_ptr<Screen> TennisScoreTracker::createHomeScreen() {
        lv_obj_t* label = lv_label_create(lv_scr_act(), nullptr);
        lv_label_set_text_static(label, "home screen");
        lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
        lv_obj_align(label, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
        return std::make_unique<Screens::Label>(0, 5, label);
    }

    std::unique_ptr<Screen> TennisScoreTracker::createMatchScreen() {
        lv_obj_t* label = lv_label_create(lv_scr_act(), nullptr);
        lv_label_set_text_static(label, "match screen");
        lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
        lv_obj_align(label, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
        return std::make_unique<Screens::Label>(0, 5, label);
    }
}

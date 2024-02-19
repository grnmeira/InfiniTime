#pragma once

#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Label.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/ScreenList.h"
#include "displayapp/Controllers.h"

#include "Symbols.h"

class TennisMatchModel;

namespace Pinetime {
    namespace Applications {
        namespace Screens {
            class TennisScoreTracker : public Screen {
            public:
                TennisScoreTracker(Pinetime::Applications::DisplayApp* app,
                                   Controllers::MotorController& motorController,
                                   Controllers::DateTime& dateTimeController,
                                   Controllers::Timer& timer,
                                   Controllers::FS& fs);
                ~TennisScoreTracker() override;
                void Vibrate();
                void Refresh() override;
                bool OnTouchEvent(TouchEvents event) override;
            private:
                Controllers::MotorController& motorController;
                Controllers::DateTime& dateTimeController;
                Controllers::FS& fs;
                Controllers::Timer motorTimer;

                std::unique_ptr<TennisMatchModel> model;

                lv_obj_t* meGameScoreLabel;
                lv_obj_t* opGameScoreLabel;
                lv_obj_t* meSetScoreLabel[5];
                lv_obj_t* opSetScoreLabel[5];
            };
        }

        template <>
        struct AppTraits<Apps::TennisScoreTracker> {
            static constexpr Apps app = Apps::TennisScoreTracker;
            static constexpr const char* icon = Screens::Symbols::eye;
            static Screens::Screen* Create(AppControllers& controllers) {
                return new Screens::TennisScoreTracker(controllers.displayApp,
                                                       controllers.motorController,
                                                       controllers.dateTimeController,
                                                       controllers.timer,
                                                       controllers.filesystem);
            };
        };
    }
}
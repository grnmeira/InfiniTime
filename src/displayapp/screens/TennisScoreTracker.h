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
                TennisScoreTracker(Pinetime::Applications::DisplayApp* app);
                ~TennisScoreTracker() override;
                void Refresh() override;
                bool OnTouchEvent(TouchEvents event) override;
            private:
                std::unique_ptr<Screen> createHomeScreen();
                std::unique_ptr<Screen> createMatchScreen();

                ScreenList<2> screens;

                std::unique_ptr<TennisMatchModel> model;

                lv_obj_t* scoreLabel;
            };
        }

        template <>
        struct AppTraits<Apps::TennisScoreTracker> {
            static constexpr Apps app = Apps::TennisScoreTracker;
            static constexpr const char* icon = Screens::Symbols::eye;
            static Screens::Screen* Create(AppControllers& controllers) {
                return new Screens::TennisScoreTracker(controllers.displayApp);
            };
        };
    }
}
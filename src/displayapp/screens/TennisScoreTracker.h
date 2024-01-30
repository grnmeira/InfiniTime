#pragma once

#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include "Symbols.h"

namespace Pinetime {
    namespace Applications {
        namespace Screens {
            class TennisScoreTracker : public Screen {
            public:
                TennisScoreTracker();
                ~TennisScoreTracker() override;
            };
        }

        template <>
        struct AppTraits<Apps::TennisScoreTracker> {
            static constexpr Apps app = Apps::TennisScoreTracker;
            static constexpr const char* icon = Screens::Symbols::eye;
            static Screens::Screen* Create(AppControllers& controllers) {
                return new Screens::TennisScoreTracker();
            };
        };
    }
}
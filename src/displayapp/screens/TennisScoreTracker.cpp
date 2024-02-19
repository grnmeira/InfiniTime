#include "displayapp/screens/TennisScoreTracker.h"
#include "components/datetime/DateTimeController.h"
#include "components/motor/MotorController.h"
#include "components/timer/Timer.h"


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
        LOVE = 0,
        S_15,
        S_30,
        S_40,
        AD
    };

    struct PlayerScore {
        GameScore currentGame{};
        std::array<unsigned char, 5> sets{};
        std::array<unsigned char, 5> tieBreaks{};
    };

    struct MatchSummary {
        PlayerScore myScore;
        PlayerScore oppScore;
    };

    struct TieBreakConfig {
        unsigned char winningScore;
        unsigned char minDiff;
        unsigned char maxPoints;
    };
    
    struct MatchConfig {
        unsigned char totalSets;
        unsigned char maxDeuces;
        TieBreakConfig tieBreakConfig;
    };

    void registerPointForMe() {
        events.emplace_back(Event{ Event::Type::POINT_ME });
    }

    void registerPointForOpp() {
        events.emplace_back(Event{ Event::Type::POINT_OPP });
    }

    void undo() {
        if (!events.empty()) {
            events.pop_back();
        }
    }

    MatchSummary getMatchSummary() {
        MatchSummary summary;
        for (const auto& event : events) {
            if (event.type == Event::Type::POINT_ME) {
                computePoint(summary.myScore, summary.oppScore);
            } else if (event.type == Event::Type::POINT_OPP) {
                computePoint(summary.oppScore, summary.myScore);
            }
        }
        return summary;
    }

    static std::string gameScoreToString(GameScore gameScore) {
        switch (gameScore) {
            case GameScore::LOVE:
                return "LV";
            case GameScore::AD:
                return "AD";
            case GameScore::S_15:
                return "15";
            case GameScore::S_30:
                return "30";
            case GameScore::S_40:
                return "40";
            default:
                return "N/A";
        }
    }
private:
    void computePoint(PlayerScore& winner, PlayerScore& loser) {
        switch (winner.currentGame) {
            case GameScore::LOVE:
                if (loser.currentGame == GameScore::AD)
                {
                    winner.currentGame = GameScore::AD;
                    loser.currentGame = GameScore::LOVE;
                } else {
                    winner.currentGame = GameScore::S_15;
                }
                break;
            case GameScore::S_15:
                winner.currentGame = GameScore::S_30;
                break;
            case GameScore::S_30:
                winner.currentGame = GameScore::S_40;
                break;
            case GameScore::S_40:
                if (loser.currentGame == GameScore::S_40) {
                    winner.currentGame = GameScore::AD;
                    loser.currentGame = GameScore::LOVE;
                } else {
                    winner.currentGame = GameScore::LOVE;
                    loser.currentGame = GameScore::LOVE;
                    computeGame(winner, loser);
                }
                break;
            case GameScore::AD:
                winner.currentGame = GameScore::LOVE;
                loser.currentGame = GameScore::LOVE;
                computeGame(winner, loser);
                break;
        }
    }

    bool isSetFinished(const unsigned char scoreA, const unsigned char scoreB) {
        return (scoreA == 6 && 6 - scoreB >= 2) ||
               (scoreB == 6 && 6 - scoreA >= 2) ||
               (scoreA == 7 && scoreB == 6) ||
               (scoreB == 7 && scoreA == 6);

               // missing tie break rules for now
    }

    void computeGame(PlayerScore& winner, PlayerScore& loser) {
        for (auto i = 0; i < winner.sets.size(); i++) {
            if (isSetFinished(winner.sets[i], loser.sets[i])) {
                continue;
            }
            winner.sets[i]++;
            break;
        }
    }

    void computeSet(PlayerScore& winner, PlayerScore& loser) {
    }

    std::vector<Event> events;
};


namespace Pinetime::Applications::Screens {
    static void TimerCallback(TimerHandle_t xTimer) {
        auto* obj = static_cast<TennisScoreTracker*>(pvTimerGetTimerID(xTimer));
        obj->Vibrate();
    }

    TennisScoreTracker::TennisScoreTracker(Pinetime::Applications::DisplayApp* app,
                                           Controllers::MotorController& motorController,
                                           Controllers::DateTime& dateTimeController,
                                           Controllers::Timer& timerController,
                                           Controllers::FS& fs)
    : motorController(motorController),
    dateTimeController(dateTimeController),
    motorTimer(this, TimerCallback),
    fs(fs),
    model(std::make_unique<TennisMatchModel>())
    {
        const auto mainContainer = lv_cont_create(lv_scr_act(), nullptr);
        lv_obj_set_style_local_bg_color(mainContainer, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
        lv_obj_set_style_local_border_color(mainContainer, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
        lv_obj_set_style_local_border_width(mainContainer, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 0);
        lv_cont_set_fit(mainContainer, LV_FIT_PARENT);
        lv_cont_set_layout(mainContainer, LV_LAYOUT_PRETTY_MID);

        const auto gameScoreContainer = lv_cont_create(mainContainer, nullptr);
        lv_obj_set_style_local_bg_color(gameScoreContainer, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
        lv_obj_set_style_local_border_color(gameScoreContainer, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
        lv_obj_set_style_local_border_width(gameScoreContainer, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 0);
        lv_obj_set_style_local_pad_top(gameScoreContainer, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 5);
        lv_obj_set_style_local_pad_left(gameScoreContainer, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 5);
        lv_obj_set_style_local_pad_right(gameScoreContainer, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 5);
        lv_cont_set_fit2(gameScoreContainer, LV_FIT_PARENT, LV_FIT_TIGHT);
        lv_cont_set_layout(gameScoreContainer, LV_LAYOUT_PRETTY_MID);

        meGameScoreLabel = lv_label_create(gameScoreContainer, nullptr);
        lv_label_set_text_static(meGameScoreLabel, "LV");
        lv_obj_set_style_local_text_font(meGameScoreLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_76);
        lv_obj_set_style_local_border_color(meGameScoreLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
        lv_obj_set_style_local_border_width(meGameScoreLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 0);
        opGameScoreLabel = lv_label_create(gameScoreContainer, nullptr);
        lv_label_set_text_static(opGameScoreLabel, "LV");
        lv_obj_set_style_local_text_font(opGameScoreLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_76);
        lv_obj_set_style_local_border_color(opGameScoreLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
        lv_obj_set_style_local_border_width(opGameScoreLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 0);

        const auto playerLabelsContainer = lv_cont_create(mainContainer, nullptr);
        lv_obj_set_style_local_bg_color(playerLabelsContainer, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
        lv_cont_set_fit2(playerLabelsContainer, LV_FIT_PARENT, LV_FIT_TIGHT);
        lv_cont_set_layout(playerLabelsContainer, LV_LAYOUT_PRETTY_MID);
        lv_obj_set_style_local_pad_left(playerLabelsContainer, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 5);
        lv_obj_set_style_local_pad_right(playerLabelsContainer, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 5);

        const auto meLabel = lv_label_create(playerLabelsContainer, nullptr);
        lv_label_set_text_static(meLabel, "me");
        lv_obj_set_style_local_text_font(meLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20);
        lv_obj_set_style_local_text_color(meLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_YELLOW);

        const auto opLabel = lv_label_create(playerLabelsContainer, nullptr);
        lv_label_set_text_static(opLabel, "op");
        lv_obj_set_style_local_text_font(opLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20);
        lv_obj_set_style_local_text_color(opLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);

        const auto timeLabel = lv_label_create(mainContainer, nullptr);
        lv_label_set_text_static(timeLabel, "00:00");
        lv_obj_set_style_local_text_font(opLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_bold_20);
        lv_obj_set_style_local_text_color(timeLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);

        const auto setsContainer = lv_cont_create(mainContainer, nullptr);
        lv_obj_set_style_local_bg_color(setsContainer, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
        lv_cont_set_fit2(setsContainer, LV_FIT_PARENT, LV_FIT_TIGHT);
        lv_cont_set_layout(setsContainer, LV_LAYOUT_PRETTY_MID);
        lv_obj_set_style_local_pad_right(setsContainer, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 25);
        lv_obj_set_style_local_pad_left(setsContainer, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 25);
        lv_obj_set_style_local_pad_top(setsContainer, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 10);
        lv_obj_set_style_local_pad_bottom(setsContainer, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 10);
        lv_obj_set_style_local_pad_inner(setsContainer, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 5);
        lv_obj_set_style_local_border_color(setsContainer, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GREEN);
        lv_obj_set_style_local_border_width(setsContainer, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 0);

        const auto setContainer = lv_cont_create(setsContainer, nullptr);
        lv_obj_set_style_local_bg_color(setContainer, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
        lv_cont_set_fit2(setContainer, LV_FIT_TIGHT, LV_FIT_MAX);
        lv_cont_set_layout(setContainer, LV_LAYOUT_COLUMN_MID);

        const auto meSetLabel = lv_label_create(setContainer, nullptr);
        lv_label_set_text_static(meSetLabel, "me");
        lv_obj_set_style_local_text_color(meSetLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_YELLOW);

        const auto opSetLabel = lv_label_create(setContainer, nullptr);
        lv_label_set_text_static(opSetLabel, "op");
        lv_obj_set_style_local_text_color(opSetLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);

        for (auto i = 0; i < 5; i++) {
            const auto setContainer = lv_cont_create(setsContainer, nullptr);
            lv_obj_set_style_local_bg_color(setContainer, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
            lv_cont_set_fit2(setContainer, LV_FIT_TIGHT, LV_FIT_TIGHT);
            lv_cont_set_layout(setContainer, LV_LAYOUT_COLUMN_MID);

            meSetScoreLabel[i] = lv_label_create(setContainer, nullptr);
            lv_label_set_text_static(meSetScoreLabel[i], "0");
            lv_obj_set_style_local_text_color(meSetScoreLabel[i], LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);

            opSetScoreLabel[i] = lv_label_create(setContainer, nullptr);
            lv_label_set_text_static(opSetScoreLabel[i], "0");
            lv_obj_set_style_local_text_color(opSetScoreLabel[i], LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);

            if (i == 0) { // active set
                lv_obj_set_style_local_pad_right(setContainer, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 5);
                lv_obj_set_style_local_pad_left(setContainer, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 5);
                lv_obj_set_style_local_bg_color(setContainer, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GREEN);
            }
        }
    }

    TennisScoreTracker::~TennisScoreTracker() {
        lv_obj_clean(lv_scr_act());
    }

    void TennisScoreTracker::Vibrate() {
        motorController.RunForDuration(30);
    }

    bool TennisScoreTracker::OnTouchEvent(TouchEvents event) {
        if (event == TouchEvents::Tap ||
            event == TouchEvents::SwipeUp) {
            model->registerPointForMe();
            Refresh();
            motorController.RunForDuration(30);
            return true;
        } else if (event == TouchEvents::DoubleTap ||
                   event == TouchEvents::SwipeDown) {
            model->registerPointForOpp();
            motorController.RunForDuration(30);
            motorTimer.StartTimer(std::chrono::milliseconds(250));
            Refresh();
            return true;
        } else if (event == TouchEvents::LongTap) {
            model->undo();
            Refresh();
            return true;
        }

        return false;
    }

    void TennisScoreTracker::Refresh() {
//        if (scoreLabel && model) {
//            const auto summary = model->getMatchSummary();
//            lv_label_set_text_fmt(scoreLabel, "game %d - %d\n"
//                                              "set #1 %d - %d\n"
//                                              "set #2 %d - %d\n"
//                                              "set #3 %d - %d\n",
//                                              (int) summary.myScore.currentGame,
//                                              (int) summary.oppScore.currentGame,
//                                              (int) summary.myScore.sets[0],
//                                              (int) summary.oppScore.sets[0],
//                                              (int) summary.myScore.sets[1],
//                                              (int) summary.oppScore.sets[1],
//                                              (int) summary.myScore.sets[2],
//                                              (int) summary.oppScore.sets[2],
//                                              0, 0);
//        }
        const auto summary = model->getMatchSummary();
        if (summary.myScore.currentGame == TennisMatchModel::GameScore::AD)
        {
            lv_label_set_text_static(meGameScoreLabel, "AD");
            lv_label_set_text_static(opGameScoreLabel, "-");
        } else if (summary.oppScore.currentGame == TennisMatchModel::GameScore::AD)
        {
            lv_label_set_text_static(meGameScoreLabel, "-");
            lv_label_set_text_static(opGameScoreLabel, "AD");
        } else {
            lv_label_set_text_fmt(meGameScoreLabel, TennisMatchModel::gameScoreToString(summary.myScore.currentGame).c_str());
            lv_label_set_text_fmt(opGameScoreLabel, TennisMatchModel::gameScoreToString(summary.oppScore.currentGame).c_str());
        }

        for (auto i = 0; i < 5; i++)
        {
            lv_label_set_text_fmt(meSetScoreLabel[i], "%d", summary.myScore.sets[i]);
            lv_label_set_text_fmt(opSetScoreLabel[i], "%d", summary.oppScore.sets[i]);
        }
    }
}

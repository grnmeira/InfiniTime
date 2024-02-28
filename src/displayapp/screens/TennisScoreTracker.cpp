#include "displayapp/screens/TennisScoreTracker.h"
#include "components/datetime/DateTimeController.h"
#include "components/motor/MotorController.h"
#include "components/timer/Timer.h"


class TennisMatchModel {
public:
    enum class Player {
        PLAYER1,
        PLAYER2
    };

    struct Event {
        enum class Type {
            POINT_P1,
            POINT_P2,
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

    struct Set {
        bool isActive{false};
        unsigned char scoreP1{0};
        unsigned char scoreP2{0};
        bool isTieBreakActive{false};
        unsigned char tieBreakScoreP1{0};
        unsigned char tieBreakScoreP2{0};
    };

    struct MatchSummary {
        GameScore currentGameP1{GameScore::LOVE};
        GameScore currentGameP2{GameScore::LOVE};
        unsigned char currentSet{0};
        std::array<Set, 5> sets{};
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

    void registerPoint(const Player scoringPlayer) {
        computePoint(scoringPlayer);
    }

    void undo() {
        // TODO
    }

    MatchSummary getMatchSummary() {
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

    TennisMatchModel() {
        summary.sets[0].isActive = true;
    }
private:
    void computePoint(const Player scoringPlayer) {
        if (summary.sets[summary.currentSet].isTieBreakActive) {
            computeTieBreakPoint(scoringPlayer);
        } else {
            computeGamePoint(scoringPlayer);
        }
    }

    void computeGamePoint(const Player scoringPlayer) {
        auto& winnerGameScore = scoringPlayer == Player::PLAYER1 ? summary.currentGameP1 : summary.currentGameP2;
        auto& loserGameScore = scoringPlayer == Player::PLAYER1 ? summary.currentGameP2 : summary.currentGameP1;

        switch (winnerGameScore) {
            case GameScore::LOVE:
                if (loserGameScore == GameScore::AD)
                {
                    winnerGameScore = GameScore::AD;
                    loserGameScore = GameScore::LOVE;
                } else {
                    winnerGameScore = GameScore::S_15;
                }
                break;
            case GameScore::S_15:
                winnerGameScore = GameScore::S_30;
                break;
            case GameScore::S_30:
                winnerGameScore = GameScore::S_40;
                break;
            case GameScore::S_40:
                if (loserGameScore == GameScore::S_40) {
                    winnerGameScore = GameScore::AD;
                    loserGameScore = GameScore::LOVE;
                } else {
                    winnerGameScore = GameScore::LOVE;
                    loserGameScore = GameScore::LOVE;
                    computeGame(scoringPlayer);
                }
                break;
            case GameScore::AD:
                winnerGameScore = GameScore::LOVE;
                loserGameScore = GameScore::LOVE;
                computeGame(scoringPlayer);
                break;
        }
    }

    void computeTieBreakPoint(const Player scoringPlayer) {
        if (scoringPlayer == Player::PLAYER1) {
            summary.sets[summary.currentSet].tieBreakScoreP1++;
        } else {
            summary.sets[summary.currentSet].tieBreakScoreP2++;
        }

        if (isTieBreakFinished(summary.sets[summary.currentSet].tieBreakScoreP1,
                               summary.sets[summary.currentSet].tieBreakScoreP2)) {
            if (summary.currentSet < summary.sets.size()) {
                summary.currentSet++;
                summary.sets[summary.currentSet].isActive = true;
            } else {
                // Match is finished
            }
        }
    }

    bool isSetFinished(const unsigned char scoreA, const unsigned char scoreB) {
        return (scoreA == 6 && 6 - scoreB >= 2) ||
               (scoreB == 6 && 6 - scoreA >= 2) ||
               (scoreA == 7 && scoreB == 5) ||
               (scoreB == 7 && scoreA == 5);
    }

    bool isTieBreakSet(const unsigned char scoreA, const unsigned char scoreB) {
        return (scoreA == 6 && scoreB == 6) ||
               (scoreB == 6 && scoreA == 6);
    }

    bool isTieBreakFinished(const unsigned char scoreA, const unsigned char scoreB) {
        return (scoreA >= 7 && scoreA - scoreB >= 2) ||
               (scoreB >= 7 && scoreB - scoreA >= 2) ||
               scoreA == 10 || scoreB == 10;
    }

    void computeGame(const Player scoringPlayer) {
        auto& currentSet = summary.sets[summary.currentSet];

        if (scoringPlayer == Player::PLAYER1) {
            currentSet.scoreP1++;
        } else {
            currentSet.scoreP2++;
        }

        if (isTieBreakSet(currentSet.scoreP1, currentSet.scoreP2)) {
            currentSet.isTieBreakActive = true;
            return;
        }

        if (isSetFinished(currentSet.scoreP1, currentSet.scoreP2)) {
            if (summary.currentSet < summary.sets.size() - 1) {
                summary.currentSet++;
                summary.sets[summary.currentSet].isActive = true;
            } else {
                // end of match!
                return;
            }
        }
    }

    std::array<Event, 5> latestEvents;
    MatchSummary summary{};
};

static constexpr auto TB_0 = "⁰";
static constexpr auto TB_1 = "¹";
static constexpr auto TB_2 = "²";
static constexpr auto TB_3 = "³";
static constexpr auto TB_4 = "⁴";
static constexpr auto TB_5 = "⁵";
static constexpr auto TB_6 = "⁶";
static constexpr auto TB_7 = "⁷";
static constexpr auto TB_8 = "⁸";
static constexpr auto TB_9 = "⁹";

static constexpr const char* TB[] = {
        TB_0, TB_1, TB_2, TB_3, TB_4, TB_5, TB_6, TB_7, TB_8, TB_9
};

std::string makeTieBreakScoreString(unsigned char score) {
    std::string s;
    size_t lower_digit = score % 10;
    s = TB[lower_digit];
    if (score > 9) {
        size_t higher_digit = (score - lower_digit)/10;
        s = TB[higher_digit] + s;
    }
    return s;
}


namespace Pinetime::Applications::Screens {
    static void TimerCallback(TimerHandle_t xTimer) {
        auto* obj = static_cast<TennisScoreTracker*>(pvTimerGetTimerID(xTimer));
        obj->Vibrate();
    }

    TennisScoreTracker::TennisScoreTracker(Controllers::MotorController& motorController,
                                           Controllers::DateTime& dateTimeController,
                                           Controllers::FS& fs)
    : motorController(motorController),
    dateTimeController(dateTimeController),
    fs(fs),
    motorTimer(this, TimerCallback),
    model(std::make_unique<TennisMatchModel>()),
    matchStartInUptime(dateTimeController.Uptime())
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

        timeLabel = lv_label_create(mainContainer, nullptr);
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
            setContainers[i] = lv_cont_create(setsContainer, nullptr);
            lv_obj_set_style_local_bg_color(setContainers[i], LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
            lv_cont_set_fit2(setContainers[i], LV_FIT_TIGHT, LV_FIT_TIGHT);
            lv_cont_set_layout(setContainers[i], LV_LAYOUT_COLUMN_MID);

            meSetScoreLabel[i] = lv_label_create(setContainers[i], nullptr);
            lv_label_set_text_static(meSetScoreLabel[i], "0");
            lv_obj_set_style_local_text_color(meSetScoreLabel[i], LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);

            opSetScoreLabel[i] = lv_label_create(setContainers[i], nullptr);
            lv_label_set_text_static(opSetScoreLabel[i], "0");
            lv_obj_set_style_local_text_color(opSetScoreLabel[i], LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
        }

        Refresh();
    }

    TennisScoreTracker::~TennisScoreTracker() {
        lv_obj_clean(lv_scr_act());
    }

    void TennisScoreTracker::Vibrate() {
        motorController.RunForDuration(30);
    }

    bool TennisScoreTracker::OnTouchEvent(TouchEvents event) {
        // This is a workaround. When the firmware is running
        // on PineTime itself, it always gets a first tap event
        // when it's loaded from the app list. So for now, we
        // just reject the first tap.
        if (event == TouchEvents::Tap && !firstEventRejected) {
            firstEventRejected = true;
            return false;
        }

        if (event == TouchEvents::Tap ||
            event == TouchEvents::SwipeUp) {
            model->registerPoint(TennisMatchModel::Player::PLAYER1);
            Refresh();
            motorController.RunForDuration(30);
            return true;
        } else if (event == TouchEvents::DoubleTap ||
                   event == TouchEvents::SwipeDown) {
            model->registerPoint(TennisMatchModel::Player::PLAYER2);
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

    bool TennisScoreTracker::OnButtonPushed() {
        // Horrible feeling when you're in a middle
        // of a game and you press the button twice...
        // This way we won't lose the current match
        // state for now. A longer press returns to
        // the watchface.
        return true;
    }

    void TennisScoreTracker::Refresh() {
        const auto summary = model->getMatchSummary();
        if (summary.sets[summary.currentSet].isTieBreakActive) {
            lv_label_set_text_fmt(meGameScoreLabel, "%d", summary.sets[summary.currentSet].tieBreakScoreP1);
            lv_label_set_text_fmt(opGameScoreLabel, "%d", summary.sets[summary.currentSet].tieBreakScoreP2);
        } else if (summary.currentGameP1 == TennisMatchModel::GameScore::AD) {
            lv_label_set_text_static(meGameScoreLabel, "AD");
            lv_label_set_text_static(opGameScoreLabel, "-");
        } else if (summary.currentGameP2 == TennisMatchModel::GameScore::AD) {
            lv_label_set_text_static(meGameScoreLabel, "-");
            lv_label_set_text_static(opGameScoreLabel, "AD");
        } else {
            lv_label_set_text_fmt(meGameScoreLabel, TennisMatchModel::gameScoreToString(summary.currentGameP1).c_str());
            lv_label_set_text_fmt(opGameScoreLabel, TennisMatchModel::gameScoreToString(summary.currentGameP2).c_str());
        }

        const auto matchDuration = dateTimeController.Uptime() - matchStartInUptime;

        lv_label_set_text_fmt(timeLabel,
                              "%02d:%02d %dm",
                              dateTimeController.Hours(),
                              dateTimeController.Minutes(),
                              std::chrono::duration_cast<std::chrono::minutes>(matchDuration));

        auto lastActiveSet = 0;
        for (size_t i = 0; i < summary.sets.size(); i++) {
            const auto& set = summary.sets[i];
            if (set.isActive) {
                lastActiveSet = i;
                lv_obj_set_style_local_pad_right(setContainers[i], LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 0);
                lv_obj_set_style_local_pad_left(setContainers[i], LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 0);
                lv_obj_set_style_local_bg_color(setContainers[i], LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
                lv_obj_set_style_local_text_color(meSetScoreLabel[i], LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
                lv_obj_set_style_local_text_color(opSetScoreLabel[i], LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
            } else {
                lv_obj_set_style_local_pad_right(setContainers[i], LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 0);
                lv_obj_set_style_local_pad_left(setContainers[i], LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 0);
                lv_obj_set_style_local_bg_color(setContainers[i], LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
                lv_obj_set_style_local_text_color(meSetScoreLabel[i], LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
                lv_obj_set_style_local_text_color(opSetScoreLabel[i], LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
            }

            if (set.isTieBreakActive) {
                const auto tieBreakScoreP1 = makeTieBreakScoreString(set.tieBreakScoreP1);
                const auto tieBreakScoreP2 = makeTieBreakScoreString(set.tieBreakScoreP2);
                lv_label_set_text_fmt(meSetScoreLabel[i], "%d%s", set.scoreP1, tieBreakScoreP1.data());
                lv_label_set_text_fmt(opSetScoreLabel[i], "%d%s", set.scoreP2, tieBreakScoreP2.data());
            } else {
                lv_label_set_text_fmt(meSetScoreLabel[i], "%d", set.scoreP1);
                lv_label_set_text_fmt(opSetScoreLabel[i], "%d", set.scoreP2);
            }
        }
        lv_obj_set_style_local_pad_right(setContainers[lastActiveSet], LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 5);
        lv_obj_set_style_local_pad_left(setContainers[lastActiveSet], LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 5);
        lv_obj_set_style_local_bg_color(setContainers[lastActiveSet], LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GREEN);
    }
}

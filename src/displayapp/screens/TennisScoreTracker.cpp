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

    void registerPointForMe() {
        events.emplace_back(Event{ Event::Type::POINT_ME });
    }

    void registerPointForOpp() {
        events.emplace_back(Event{ Event::Type::POINT_OPP });
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

    void computeGame(PlayerScore& winner, PlayerScore& loser) {
        winner.sets[0]++;
    }

    void computeSet(PlayerScore& winner, PlayerScore& loser) {

    }

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
    model(std::make_unique<TennisMatchModel>()),
    scoreLabel(nullptr)
    {
        //taskRefresh = lv_task_create(RefreshTaskCallback, 300, LV_TASK_PRIO_MID, this);
    }

    TennisScoreTracker::~TennisScoreTracker() {
        //lv_task_del(taskRefresh);
        lv_obj_clean(lv_scr_act());
    }

    bool TennisScoreTracker::OnTouchEvent(TouchEvents event) {
        if (event == TouchEvents::SwipeUp) {
            model->registerPointForMe();
            Refresh();
            return true;
        } else if (event == TouchEvents::SwipeDown) {
            model->registerPointForOpp();
            Refresh();
            return true;
        } else if (event == TouchEvents::LongTap) {
            //lv_label_set_text_static(scoreLabel, "long tap");
        }

        return screens.OnTouchEvent(event);
    }

    void TennisScoreTracker::Refresh() {
        if (scoreLabel && model) {
            const auto summary = model->getMatchSummary();
            lv_label_set_text_fmt(scoreLabel, "game %d - %d\n"
                                              "set %d - %d\n"
                                              "match %d - %d",
                                              (int) summary.myScore.currentGame,
                                              (int) summary.oppScore.currentGame,
                                              (int) summary.myScore.sets[0],
                                              (int) summary.oppScore.sets[0],
                                              0, 0);
        }
    }

    std::unique_ptr<Screen> TennisScoreTracker::createHomeScreen() {
        lv_obj_t* label = lv_label_create(lv_scr_act(), nullptr);
        lv_label_set_text_static(label, "home screen");
        lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
        lv_obj_align(label, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
        return std::make_unique<Screens::Label>(0, 5, label);
    }

    std::unique_ptr<Screen> TennisScoreTracker::createMatchScreen() {
        scoreLabel = lv_label_create(lv_scr_act(), nullptr);
        const auto summary = model->getMatchSummary();
        lv_label_set_text_fmt(scoreLabel, "%d - %d", (int)summary.myScore.currentGame, (int)summary.oppScore.currentGame);
        lv_label_set_align(scoreLabel, LV_LABEL_ALIGN_CENTER);
        lv_obj_align(scoreLabel, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
        return std::make_unique<Screens::Label>(0, 5, scoreLabel);
    }
}

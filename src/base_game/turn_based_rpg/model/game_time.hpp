#ifndef INCLUDE_GAME_TIME_HPP_
#define INCLUDE_GAME_TIME_HPP_

namespace game::ecs {
struct GameTime {

    int day{1};
    int month{1};
    int year{1};

    bool new_month{false};
    bool new_year{false};

    static constexpr int days_in_month[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
};

void update_game_time(GameTime& time);

}
    



#endif // INCLUDE_GAME_TIME_HPP_
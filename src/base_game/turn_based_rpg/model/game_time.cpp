#include "game_time.hpp"

void game::ecs::update_game_time(GameTime& time)
{
    time.day++;

    time.new_month = time.day > GameTime::days_in_month[time.month];

    time.day = time.new_month? 1 : time.day;

    time.month = time.new_month? time.month + 1 : time.month;

    time.new_year = time.month > 12;

    time.month = time.new_year? 1 : time.month;

    time.year = time.new_year? time.year + 1 : time.year;
}
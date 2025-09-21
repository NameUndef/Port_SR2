#include "id.hpp"
#include <cstring>

bool operator==(const ID& lhs, const ID& rhs) noexcept
{
    return ((lhs.index() == STRING_INDEX && rhs.index() == STRING_INDEX) && std::get<std::string>(lhs) == std::get<std::string>(rhs))
        || ((lhs.index() == INT_INDEX && rhs.index() == INT_INDEX) && std::get<int>(lhs) == std::get<int>(rhs));
}

bool operator==(const ID& lhs, const char* rhs) noexcept
{
    return lhs.index() == STRING_INDEX && std::strcmp(std::get<std::string>(lhs).c_str(), rhs) == 0;
}

bool operator==(const ID& lhs, const std::string& rhs) noexcept
{
    return lhs.index() == STRING_INDEX && std::get<std::string>(lhs) == rhs;
}

bool operator==(const ID& lhs, int rhs) noexcept
{
    return lhs.index() == INT_INDEX && std::get<int>(lhs) == rhs;
}

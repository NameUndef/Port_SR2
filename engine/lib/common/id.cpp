#include "id.hpp"
#include <cstring>

bool operator==(const ID& lhs, const ID& rhs)
{
    return ((lhs.index() == 0 && rhs.index() == 0) && std::get<std::string>(lhs) == std::get<std::string>(rhs))
        || ((lhs.index() == 1 && rhs.index() == 1) && std::get<int>(lhs) == std::get<int>(rhs));
        
}

bool operator==(const ID& lhs, const char* rhs)
{
    return lhs.index() == 0 && std::strcmp(std::get<std::string>(lhs).c_str(), rhs) == 0;
}

bool operator==(const ID& lhs, const std::string& rhs)
{
    return lhs.index() == 0 && std::get<std::string>(lhs) == rhs;
}

bool operator==(const ID& lhs, int rhs)
{
    return lhs.index() == 1 && std::get<int>(lhs) == rhs;
}

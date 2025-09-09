#ifndef INCLUDE_ID_HPP_
#define INCLUDE_ID_HPP_

#include <variant>
#include <string>
#include <utility>

using ID = std::variant<std::string, int>;
constexpr std::size_t STRING_INDEX = 0;
constexpr std::size_t INT_INDEX = 1;

template<typename T>
using is_ID_compatible = std::disjunction<
    std::is_same<std::decay_t<T>, std::string>,
    std::is_same<std::decay_t<T>, const char*>,
    std::is_same<std::decay_t<T>, char*>,
    std::is_same<std::decay_t<T>, int>>;

bool operator==(const ID& lhs, const ID& rhs) noexcept;
bool operator==(const ID& lhs, const std::string& rhs) noexcept;
bool operator==(const ID& lhs, const char* rhs) noexcept;
bool operator==(const ID& lhs, int rhs) noexcept;

template <typename T>
inline typename std::enable_if_t<is_ID_compatible<T>::value, bool> 
operator==(T lhs, const ID& rhs) noexcept
{ 
    return rhs == lhs; 
};

template<
    typename T, 
    typename U, 
    bool T_is_ID = std::is_same_v<std::decay_t<T>, ID>,
    bool U_is_ID = std::is_same_v<std::decay_t<U>, ID>,
    bool T_is_ID_comp = is_ID_compatible<T>::value,
    bool U_is_ID_comp = is_ID_compatible<U>::value
>
inline typename std::enable_if_t<
    (T_is_ID && (U_is_ID || U_is_ID_comp)) || (U_is_ID && T_is_ID_comp),
bool>
operator!=(T lhs, U rhs) noexcept
{
    return !(lhs == rhs);
}

template <>
struct std::hash<ID>
{
    std::size_t operator()(const ID& key) const
    {
        return key.index() == INT_INDEX? std::hash<int>()(std::get<int>(key)) : std::hash<string>()(std::get<string>(key));
    }
};

#endif  // INCLUDE_ID_H_

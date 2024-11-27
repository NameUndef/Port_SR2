#ifndef INCLUDE_ID_HPP_
#define INCLUDE_ID_HPP_

#include <variant>
#include <string>

using ID = std::variant<std::string, int>;

template<typename T>
using is_ID_compatible = std::disjunction<
    std::is_same<std::decay_t<T>, std::string>,
    std::is_same<std::decay_t<T>, const char*>,
    std::is_same<std::decay_t<T>, char*>,
    std::is_same<std::decay_t<T>, int>>;

bool operator==(const ID& lhs, const ID& rhs);
bool operator==(const ID& lhs, const std::string& rhs);
bool operator==(const ID& lhs, const char* rhs);
bool operator==(const ID& lhs, int rhs);

template <typename T>
inline typename std::enable_if_t<is_ID_compatible<T>::value, bool> 
operator==(T lhs, const ID& rhs) 
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
operator!=(T lhs, U rhs)
{
    return !(lhs == rhs);
}

#endif  // INCLUDE_ID_H_

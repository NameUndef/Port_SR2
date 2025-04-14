#ifndef INCLUDE_ERROR_CODE_HPP_
#define INCLUDE_ERROR_CODE_HPP_

#include <variant>
#include <deque>
#include <utility>
#include <type_traits>

/* approximate analogue of std:excepted in C++23 */

constexpr int COMMON_GROUP = 0;
constexpr int ERROR_UNKNOWN = 0;
constexpr int ERROR_OK = 0;

class ErrorCode {

public:
    using ErrorNote = std::pair<int, int>;
    
private:
    std::variant<std::monostate, ErrorNote, std::deque<ErrorNote>> error_stack;
    std::size_t error_notes_count = 0;

public:
    ErrorCode();
    ErrorCode(int error_group, int error_code);

    void push(int error_group, int error_code);
    void clear();
    bool have_error() const;

    operator ErrorNote() const;
    bool operator==(const ErrorCode& rhs) const;
};

template <typename ReturnT>
using ReturnOrErrorCode = std::conditional_t<
    std::is_same_v<ReturnT, ErrorCode> || std::is_same_v<ReturnT, void>,
    std::variant<std::monostate, ErrorCode>,
    std::variant<ReturnT, ErrorCode>>;

template <typename ReturnOrErrorCodeT>
using get_return_t = std::variant_alternative_t<0, ReturnOrErrorCodeT>;

template <typename ReturnOrErrorCodeT>
constexpr auto get_return(ReturnOrErrorCodeT&& ret) 
{ 
    return std::get<0>(std::forward<ReturnOrErrorCodeT>(ret)); 
}

template <typename ReturnOrErrorCodeT>
constexpr ErrorCode get_error_code(ReturnOrErrorCodeT&& ret) 
{ 
    return std::get<1>(std::forward<ReturnOrErrorCodeT>(ret)); 
}

template <typename ReturnOrErrorCodeT>
constexpr bool is_error_code(ReturnOrErrorCodeT& ret)
{
    return std::holds_alternative<ErrorCode>(ret);
}

#endif  // INCLUDE_ERROR_CODE_HPP_

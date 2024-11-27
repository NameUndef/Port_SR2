#include <variant>
#include <deque>
#include <utility>
#include <type_traits>

constexpr int COMMON_GROUP = 0;
constexpr int ERROR_UNKNOWN = 0;
constexpr int ERROR_OK = 0;

struct ErrorCode {

    using ErrorNote = std::pair<int, int>;
    
    std::deque<ErrorNote> error_stack;
    std::size_t error_notes_count = 0;

    ErrorCode() = default;
    
    ErrorCode(int error_group, int error_code) 
    {
        push(error_group, error_code);
    }

    void push(int error_group, int error_code) 
    {
        if (error_code != ERROR_OK)
            error_notes_count++;

        error_stack.emplace_front(error_group, error_code);
    }

    void clear() 
    { 
        error_notes_count = 0; 
        error_stack.clear(); 
    }

    bool have_error() const 
    { 
        return error_notes_count > 0; 
    }

    operator ErrorNote() const
    {
        if (!error_stack.empty())
            return error_stack.front();
        else
            return std::make_pair(COMMON_GROUP, ERROR_OK);
    }

    bool operator==(const ErrorCode& rhs) const
    {
        return static_cast<ErrorNote>(*this) == static_cast<ErrorNote>(rhs);
    }
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

template <typename ReturnT>
bool is_error_code(ReturnOrErrorCode<ReturnT> ret) {
    return std::holds_alternative<ErrorCode>(ret);
}

#include "error_code.hpp"

ErrorCode::ErrorCode()
{
    error_stack = std::monostate();
};

ErrorCode::ErrorCode(int error_group, int error_code) 
{
    if (error_code != ERROR_OK)
        error_notes_count++;

    error_stack = ErrorNote(error_group, error_code);
}

void ErrorCode::push(int error_group, int error_code) 
{
    if (error_code != ERROR_OK)
        error_notes_count++;

    if (error_stack.index() == 0) {
        
        if (error_code != ERROR_OK)
            error_notes_count++;

        error_stack = ErrorNote(error_group, error_code);
        return;

    } else if (error_stack.index() == 1) {

        ErrorNote first_error = std::get<ErrorNote>(error_stack);
        error_stack = std::deque<ErrorNote>();
        std::get<std::deque<ErrorNote>>(error_stack).push_front(first_error);
    }

    if (error_code != ERROR_OK)
        error_notes_count++;

    std::get<std::deque<ErrorNote>>(error_stack).emplace_front(error_group, error_code);
}

void ErrorCode::clear() 
{ 
    error_notes_count = 0;

    if (error_stack.index() == 2)
        std::get<std::deque<ErrorNote>>(error_stack).clear();

    error_stack = std::monostate();
}

bool ErrorCode::have_error() const 
{ 
    return error_notes_count > 0; 
}

ErrorCode::operator ErrorNote() const
{
    if (error_stack.index() == 1)
        return std::get<ErrorNote>(error_stack);
    if (error_stack.index() == 2 && !std::get<std::deque<ErrorNote>>(error_stack).empty())
        return std::get<std::deque<ErrorNote>>(error_stack).front();
    else
        return std::make_pair(COMMON_GROUP, ERROR_OK);
}

bool ErrorCode::operator==(const ErrorCode& rhs) const
{
    return static_cast<ErrorNote>(*this) == static_cast<ErrorNote>(rhs);
}

#ifndef INCLUDE_ANY_ARGS_FUNC_HPP_
#define INCLUDE_ANY_ARGS_FUNC_HPP_

#include "error_code.hpp"
#include "debug_helper.hpp"
#include <vector>
#include <any>
#include <functional>
#include <utility>
#include <type_traits>

using AnyArgs = std::vector<std::any>;

template <typename ReturnT>
using AnyArgsFunc = std::function<ReturnOrErrorCode<ReturnT>(AnyArgs&)>;

namespace Inner {

    template <typename ArgT>
    void assert_arg_move_constructible() 
    {
        static_assert(std::is_move_constructible_v<ArgT>, "Argument must have move constructor");
    }

    template <
        typename ReturnT, 
        typename FuncT, 
        typename... ArgsT, 
        std::enable_if_t<std::is_same_v<ReturnT, void>, bool> = true
    >
    inline ReturnOrErrorCode<ReturnT>
    caller(FuncT func, ArgsT&&... args) 
    {
        ((void)assert_arg_move_constructible<ArgsT>(), ...);
        std::invoke(func, std::forward<ArgsT>(args)...);
        return ErrorCode();
    }

    template <
        typename ReturnT, 
        typename FuncT, 
        typename... ArgsT, 
        std::enable_if_t<!std::is_same_v<ReturnT, void>, bool> = true
    >
    inline ReturnOrErrorCode<ReturnT>
    caller(FuncT func, ArgsT&&... args) 
    {
        ((void)assert_arg_move_constructible<ArgsT>(), ...);
        return std::invoke(func, std::forward<ArgsT>(args)...);
    }

    template <typename T>
    using lvalue_ref_wrap_t =
    std::conditional_t<std::is_lvalue_reference_v<T>, std::reference_wrapper<std::remove_reference_t<T>>, T>;

    template <typename, typename = void>
    struct has_type_member : std::false_type {};

    template <typename T>
    struct has_type_member<T, std::void_t<typename T::type>> : std::true_type {};

    enum ArgType : std::size_t {
        ARG_TYPE_RW = 1,
        ARG_TYPE_RV = 2,
        ARG_TYPE_LV = 3
    };

    enum AnyArgType : std::size_t {
        ANY_ARG_TYPE_UNKNOWN = 0,
        ANY_ARG_TYPE_RW = 1,
        ANY_ARG_TYPE_V = 2
    };

    enum ArgCastType : std::size_t {
        ARG_CAST_ERROR = 0,
        ARG_CAST_FORWARD = 1,
        ARG_CAST_UNWRAP = 2
    };

    template <
        typename ArgT, 
        ArgType arg_type_id, 
        std::enable_if_t<(arg_type_id == ARG_TYPE_RW), bool> = true
    >
     AnyArgType get_any_arg_type_id(const std::type_info& any_arg_type) 
    { 
        return ANY_ARG_TYPE_RW * (any_arg_type == typeid(ArgT));
    }

    template <
        typename ArgT, 
        ArgType arg_type_id, 
        std::enable_if_t<(arg_type_id == ARG_TYPE_RV), bool> = true
    >
     AnyArgType get_any_arg_type_id(const std::type_info& any_arg_type) 
    { 
        return ANY_ARG_TYPE_V * (any_arg_type == typeid(ArgT));
    }

    template <
        typename ArgT, 
        ArgType arg_type_id, 
        std::enable_if_t<(arg_type_id == ARG_TYPE_LV), bool> = true
    >
     AnyArgType get_any_arg_type_id(const std::type_info& any_arg_type) 
    { 
        return static_cast<AnyArgType>(
            ANY_ARG_TYPE_RW * (any_arg_type == typeid(std::reference_wrapper<std::remove_reference_t<ArgT>>))
            + ANY_ARG_TYPE_V * (any_arg_type == typeid(ArgT))); 
    }
     

    template <typename ArgT>
    ArgCastType get_argument_cast_type(const std::type_info& any_arg_type)
    {
        constexpr std::size_t arg_is_rw = has_type_member<ArgT>::value;
        constexpr std::size_t arg_is_rv = std::is_rvalue_reference_v<ArgT> && !arg_is_rw;
        constexpr std::size_t arg_is_lv = !arg_is_rw && !arg_is_rv;

        constexpr ArgType arg_type_id = static_cast<ArgType>(
            ARG_TYPE_RW * arg_is_rw + ARG_TYPE_RV * arg_is_rv + ARG_TYPE_LV * arg_is_lv);

        AnyArgType any_arg_type_id = get_any_arg_type_id<ArgT, arg_type_id>(any_arg_type);

        return static_cast<ArgCastType>(
            (any_arg_type_id != ANY_ARG_TYPE_UNKNOWN) + (arg_is_lv && (any_arg_type_id == ANY_ARG_TYPE_RW)));
    }

    template <typename ArgT>
    inline ArgT arg_cast(std::any&& any_arg, ArgCastType arg_cast_type) 
    {
        static_assert(std::is_copy_constructible_v<ArgT>, "Argument must have copy constructor");

        switch (arg_cast_type) {
        case ARG_CAST_ERROR:
            break;
        case ARG_CAST_FORWARD:
            return std::any_cast<ArgT>(any_arg);
        case ARG_CAST_UNWRAP:
            return std::any_cast<std::reference_wrapper<std::remove_reference_t<ArgT>>>
            (static_cast<std::any&>(any_arg)).get();
        default:
            break;
        }
        
        return std::any_cast<ArgT>(any_arg);
    }

    using ArgCastTypes = std::vector<ArgCastType>;

    template <
        typename ReturnT, 
        typename... ArgsT, 
        typename FuncT, 
        std::size_t... arg_ids
    >
    inline ReturnOrErrorCode<ReturnT> 
    run_with_unpacked_args(
        FuncT func,
        AnyArgs& any_args,
        ArgCastTypes& arg_cast_types,
        std::index_sequence<arg_ids...>)
    {
        return caller<ReturnT>(func, arg_cast<ArgsT>(std::move(any_args[arg_ids]), arg_cast_types[arg_ids])...);
    }

    template <
        typename ReturnT, 
        typename... ArgsT,
        typename FuncT, 
        typename ObjectT,  
        std::size_t... arg_ids
    >
    inline ReturnOrErrorCode<ReturnT> 
    run_with_unpacked_args(
        ObjectT* object,
        FuncT func,
        AnyArgs& any_args,
        ArgCastTypes& arg_cast_types,
        std::index_sequence<arg_ids...>)
    {
        return caller<ReturnT>(
            func, 
            object, 
            arg_cast<ArgsT>(std::move(any_args[arg_ids]), arg_cast_types[arg_ids])...);
    }

    template <typename AnyArgsFuncT, typename ReturnT>
    constexpr bool is_any_args_func_v = std::is_same_v<AnyArgsFunc<ReturnT>, AnyArgsFuncT>;

    template <typename... ArgsT, std::size_t... arg_ids>
    inline bool assert_argument_cast_types(
        AnyArgs& any_args, 
        ArgCastTypes& arg_cast_types, 
        std::index_sequence<arg_ids...>) {

        return ((arg_cast_types.push_back(get_argument_cast_type<ArgsT>(any_args[arg_ids].type())), 
        arg_cast_types.back() != ARG_CAST_ERROR) && ...);
    }

    template <typename ReturnT, typename ObjectT, typename ArgT, typename... ArgsT, typename FuncT>
    inline AnyArgsFunc<ReturnT>
    make_any_args_method(FuncT func, bool make_safe)
    {
        if (make_safe)
            return [func](AnyArgs& any_args) -> ReturnOrErrorCode<ReturnT> {

                if (any_args.empty() || any_args.size() - 1 != sizeof...(ArgsT) + 1)
                    return ErrorCode(1, -1);

                auto& arg_obj = any_args[any_args.size() - 1];
                if (!arg_obj.has_value() || arg_obj.type() != typeid(ObjectT*))
                    return ErrorCode(1, -2);

                ObjectT* object = std::any_cast<ObjectT*>(any_args.back());
                if (object == nullptr)
                    return ErrorCode(1, -3);
                any_args.pop_back();

                auto arg_idx_seq = std::make_index_sequence<sizeof...(ArgsT) + 1>();
                ArgCastTypes arg_cast_types;
                arg_cast_types.reserve(sizeof...(ArgsT) + 1);

                if (assert_argument_cast_types<ArgT, ArgsT...>(any_args, arg_cast_types, arg_idx_seq) == false)
                    return ErrorCode(1, -4);

                return run_with_unpacked_args<ReturnT, ArgT, ArgsT...>(
                    object, 
                    func, 
                    any_args, 
                    arg_cast_types,
                    arg_idx_seq);
            };
        else
            return [func](AnyArgs& any_args) -> ReturnOrErrorCode<ReturnT> {

                ObjectT* object = std::any_cast<ObjectT*>(any_args.back());
                any_args.pop_back();

                auto arg_idx_seq = std::make_index_sequence<sizeof...(ArgsT) + 1>();
                ArgCastTypes arg_cast_types;
                arg_cast_types.reserve(sizeof...(ArgsT) + 1);

                assert_argument_cast_types<ArgT, ArgsT...>(any_args, arg_cast_types, arg_idx_seq);

                return run_with_unpacked_args<ReturnT, ArgT, ArgsT...>(
                    object, 
                    func, 
                    any_args, 
                    arg_cast_types,
                    arg_idx_seq);
            };
    }

    template <typename ReturnT, typename ArgT, typename... ArgsT, typename FuncT>
    inline AnyArgsFunc<ReturnT>
    make_any_args_func(FuncT func, bool make_safe)
    {
        if (make_safe)
            return [func](AnyArgs& any_args) -> ReturnOrErrorCode<ReturnT> {

                if (any_args.size() != sizeof...(ArgsT) + 1)
                    return ErrorCode(1, -1);

                auto arg_idx_seq = std::make_index_sequence<sizeof...(ArgsT) + 1>();
                ArgCastTypes arg_cast_types;
                arg_cast_types.reserve(sizeof...(ArgsT) + 1);

                if (assert_argument_cast_types<ArgT, ArgsT...>(any_args, arg_cast_types, arg_idx_seq) == false)
                    return ErrorCode(1, -4);

                return run_with_unpacked_args<ReturnT, ArgT, ArgsT...>(
                    func, 
                    any_args, 
                    arg_cast_types,
                    arg_idx_seq);
            };
        else
            return [func](AnyArgs& any_args) -> ReturnOrErrorCode<ReturnT> {

                auto arg_idx_seq = std::make_index_sequence<sizeof...(ArgsT) + 1>();
                ArgCastTypes arg_cast_types;
                arg_cast_types.reserve(sizeof...(ArgsT) + 1);

                assert_argument_cast_types<ArgT, ArgsT...>(any_args, arg_cast_types, arg_idx_seq);

                return run_with_unpacked_args<ReturnT, ArgT, ArgsT...>(
                    func, 
                    any_args, 
                    arg_cast_types,
                    arg_idx_seq);
            };
    } 

    template <typename ReturnT, typename ObjectT, typename FuncT>
    inline AnyArgsFunc<ReturnT>
    make_any_args_method(FuncT func, bool make_safe)
    {
        if (make_safe)
            return [func](AnyArgs& args) -> ReturnOrErrorCode<ReturnT> {

                if (args.size() != 1)
                    return ErrorCode{1, -1};

                if (!args[0].has_value() || args[0].type() != typeid(ObjectT*))
                    return ErrorCode{1, -2};

                ObjectT* object = std::any_cast<ObjectT*>(args[0]);
                if (object == nullptr)
                    return ErrorCode{1, -3};
                args.pop_back();

                return caller<ReturnT>(func, object);
            };
        else
            return [func](AnyArgs& args) -> ReturnOrErrorCode<ReturnT> {

                ObjectT* object = std::any_cast<ObjectT*>(args[0]);
                args.pop_back();

                return caller<ReturnT>(func, object);
            };
    }

    template <typename ReturnT, typename FuncT>
    inline AnyArgsFunc<ReturnT>
    make_any_args_func(FuncT func, bool make_safe) {

        if (make_safe)
            return [func](AnyArgs& args) -> ReturnOrErrorCode<ReturnT> {

                if (!args.empty())
                    return ErrorCode{1, -1};

                return caller<ReturnT>(func);
            };
        else
            return [func](AnyArgs&) -> ReturnOrErrorCode<ReturnT> {

                return caller<ReturnT>(func);
            };
    }
}

template <typename ReturnT, typename ObjectT, typename... ArgsT>
inline AnyArgsFunc<ReturnT>
make_any_args_func(ReturnT(ObjectT::*func)(ArgsT...), bool make_safe = true)
{
    return Inner::make_any_args_method<ReturnT, ObjectT, ArgsT...>(func, make_safe);
}

template <typename ReturnT, typename ObjectT, typename... ArgsT>
inline AnyArgsFunc<ReturnT>
make_any_args_func(ReturnT(ObjectT::*func)(ArgsT...) const, bool make_safe = true)
{
    return Inner::make_any_args_method<ReturnT, ObjectT, ArgsT...>(func, make_safe);
}

template <typename ReturnT, typename ObjectT, typename... ArgsT>
inline AnyArgsFunc<ReturnT>
make_any_args_func(ReturnT(ObjectT::*func)(ArgsT...) volatile, bool make_safe = true)
{
    return Inner::make_any_args_method<ReturnT, ObjectT, ArgsT...>(func, make_safe);
}

template <typename ReturnT, typename ObjectT, typename... ArgsT>
inline AnyArgsFunc<ReturnT>
make_any_args_func(ReturnT(ObjectT::*func)(ArgsT...) const volatile, bool make_safe = true)
{
    return Inner::make_any_args_method<ReturnT, ObjectT, ArgsT...>(func, make_safe);
}

template <typename ReturnT, typename... ArgsT>
inline AnyArgsFunc<ReturnT>
make_any_args_func(ReturnT(*func)(ArgsT...), bool make_safe = true)
{
    return Inner::make_any_args_func<ReturnT, ArgsT...>(func, make_safe);
}

template <typename ReturnT, typename... ArgsT>
inline AnyArgsFunc<ReturnT>
make_any_args_func(std::function<ReturnT(ArgsT...)> func, bool make_safe = true)
{
    return Inner::make_any_args_func<ReturnT, ArgsT...>(func, make_safe);
}

template <
    typename... ArgsT, 
    typename FuncT, 
    typename ReturnT = std::invoke_result_t<FuncT, ArgsT...>>
inline AnyArgsFunc<ReturnT>
make_any_args_func(FuncT func, bool make_safe = true)
{
    return Inner::make_any_args_func<ReturnT, ArgsT...>(func, make_safe);
}

template <typename... ArgsT>
inline AnyArgs make_any_args(ArgsT&&... args)
{
    return AnyArgs{std::forward<ArgsT>(args)...};
}

template <
    typename AnyArgsFuncT, 
    typename ObjectT, 
    typename... ArgsT,
    typename ReturnT = get_return_t<std::invoke_result_t<AnyArgsFuncT, AnyArgs&>>,
    std::enable_if_t<Inner::is_any_args_func_v<AnyArgsFuncT, ReturnT>, bool> = true
>
inline ReturnOrErrorCode<ReturnT> 
call_any_args_func(
    AnyArgsFuncT& func, 
    ObjectT* object, 
    ArgsT&&... args)
{
    /* since the creation of arguments occurs before the function is called directly, 
     * it is also advantageous to wrap non-reference lvalues ​​in references 
     */
    auto any_args = AnyArgs{Inner::lvalue_ref_wrap_t<ArgsT>(std::forward<ArgsT>(args))..., object};
    return func(any_args);
}

template <
    typename AnyArgsFuncT, 
    typename... ArgsT,
    typename ReturnT = get_return_t<std::invoke_result_t<AnyArgsFuncT, AnyArgs&>>,
    std::enable_if_t<Inner::is_any_args_func_v<AnyArgsFuncT, ReturnT>, bool> = true
>
inline ReturnOrErrorCode<ReturnT>
call_any_args_func(
    AnyArgsFuncT& func, 
    ArgsT&&... args)
{
    auto any_args = AnyArgs{Inner::lvalue_ref_wrap_t<ArgsT>(std::forward<ArgsT>(args))...};
    return func(any_args);
}

template <
    typename AnyArgsFuncT, 
    typename ObjectT,
    typename ReturnT = get_return_t<std::invoke_result_t<AnyArgsFuncT, AnyArgs&>>,
    std::enable_if_t<Inner::is_any_args_func_v<AnyArgsFuncT, ReturnT>, bool> = true
>
inline ReturnOrErrorCode<ReturnT>
call_any_args_func(
    AnyArgsFuncT& func, 
    ObjectT* object, 
    AnyArgs& args)
{
    args.push_back(object);
    return func(args);
}

template <
    typename AnyArgsFuncT, 
    typename ReturnT = get_return_t<std::invoke_result_t<AnyArgsFuncT, AnyArgs&>>,
    std::enable_if_t<Inner::is_any_args_func_v<AnyArgsFuncT, ReturnT>, bool> = true
>
inline ReturnOrErrorCode<ReturnT>
call_any_args_func(
    AnyArgsFuncT& func, 
    AnyArgs& args)
{
    return func(args);
}

#endif  // INCLUDE_ANY_ARGS_FUNC_HPP_

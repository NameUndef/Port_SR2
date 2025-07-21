#include "any_args_func.hpp"
#include "debug_helper.hpp"

#include <catch2/catch_test_macros.hpp>
#include <functional>
#include <iostream>

int func_sum(int a, int b) {return a + b;};
void func_add(int& a, int b) {a += b;};
int func_get() {return 42;}

void func_obj_helper_value_upd(ObjectHelper& obj) {obj.set_value(obj.get_value() + " updated");};
void func_obj_helper_value_print(ObjectHelper obj) {obj.print_value();};

class ObjectWithOperator_1
{
public:
    int value = 0;

    int operator()(int add)
    {
       return value += add;
    }

    int operator()(void)
    {
        return value;
    }
};

class ObjectWithOperator_2 {

    int& value;

public:
    ObjectWithOperator_2(int& value) : value(value) {}

    void operator()(int add)
    {
        value += add;
    }
    int operator()(void)
    {
        return value;
    }
};

TEST_CASE("any_args_func", "[inner_lib][common][any_args_func]")
{
    SECTION("lambda") {

        AnyArgsFunc<int> aa_func_sum =
            make_any_args_func(std::function<int(int, int)>([](auto a, auto b) {return a + b;}));

        REQUIRE(get_return(call_any_args_func(aa_func_sum, 1, 2)) == 3);
        REQUIRE(get_error_code(call_any_args_func(aa_func_sum, 1)) == ErrorCode(1, -1));

        auto lambda_str = std::function<std::string(std::string)>(
                [test_str = std::string("test")](auto str) {return str + test_str;});
        auto aa_func_str = make_any_args_func(lambda_str);
        REQUIRE(get_return(call_any_args_func(aa_func_str, std::string("test"))) == "testtest");

        std::string str;
        auto lambda_get_str = std::function<void(void)>([&str]() {str = "test";});
        auto aa_func_get_str = make_any_args_func(lambda_get_str);
        REQUIRE(get_error_code(call_any_args_func(aa_func_get_str)) == ErrorCode(0, 0));
        REQUIRE(str == "test");

        auto aa_func_mul = make_any_args_func<double, double>([](auto a, auto b) {return a * b;});
        REQUIRE(get_return(call_any_args_func(aa_func_mul, 2., 3.)) >= 6);
    }

    SECTION("functor object") {

        ObjectWithOperator_1 obj;
        obj.value = 5;
        auto aa_func_obj_add = make_any_args_func<int>(obj);    // int operator()(int add)
        REQUIRE(get_return(call_any_args_func(aa_func_obj_add, 5)) == 10);
        auto aa_func_obj_get = make_any_args_func(obj);         // int operator()(void)
        REQUIRE(get_return(call_any_args_func(aa_func_obj_get)) == 5);

        int value = 0;
        ObjectWithOperator_2 obj2(value);
        auto aa_func_obj2_add = make_any_args_func<int>(obj2);
        REQUIRE(get_error_code(call_any_args_func(aa_func_obj2_add, 5)) == ErrorCode(0, 0));

        auto aa_func_obj2_get = make_any_args_func(obj2);       // int operator()(void)
        REQUIRE(get_return(call_any_args_func(aa_func_obj2_get)) == 5);

        REQUIRE(get_error_code(call_any_args_func(aa_func_obj2_get, 0)) == ErrorCode(1, -1));
    }

    SECTION("function") {

        auto aa_func_sum = make_any_args_func(func_sum);
        REQUIRE(get_return(call_any_args_func(aa_func_sum, 1, 2)) == 3);

        int value = 0;
        auto aa_func_add = make_any_args_func(func_add);
        REQUIRE(get_error_code(call_any_args_func(aa_func_add, value, 1)) == ErrorCode(0, 0));
        REQUIRE(value == 1);

        auto aa_func_get_unsafe = make_any_args_func(func_get, false);
        REQUIRE(get_return(call_any_args_func(aa_func_get_unsafe)) == 42);

        auto aa_func_sum_unsafe = make_any_args_func(func_sum, false);
        REQUIRE(get_return(call_any_args_func(aa_func_sum_unsafe, 2, 3)) == 5);

        ObjectHelper obj_helper("test");
        auto aa_func_obj_helper_value_upd = make_any_args_func(func_obj_helper_value_upd);
        REQUIRE(get_error_code(call_any_args_func(
            aa_func_obj_helper_value_upd,
            obj_helper)) == ErrorCode(0, 0));
        REQUIRE(obj_helper.get_value() == "test updated");

        auto aa_func_obj_helper_value_print = make_any_args_func(func_obj_helper_value_print);
        REQUIRE(get_error_code(call_any_args_func(
            aa_func_obj_helper_value_print,
            obj_helper)) == ErrorCode(0, 0));

        auto aa_func_obj_helper_value_upd_args = make_any_args(std::ref(obj_helper));
        REQUIRE(get_error_code(call_any_args_func(
            aa_func_obj_helper_value_upd, 
            aa_func_obj_helper_value_upd_args)) == ErrorCode(0, 0));
        REQUIRE(obj_helper.get_value() == "test updated updated");
    }

    SECTION("member function") {

        ObjectHelper obj_helper_1("test 1");
        ObjectHelper obj_helper_2("test 2");

        auto aa_func_obj_helper_get_value = make_any_args_func(&ObjectHelper::get_value);
        REQUIRE(get_return(call_any_args_func(aa_func_obj_helper_get_value, &obj_helper_1)) == "test 1");
        REQUIRE(get_return(call_any_args_func(aa_func_obj_helper_get_value, &obj_helper_2)) == "test 2");

        auto aa_func_obj_helper_set_value = make_any_args_func(&ObjectHelper::set_value);
        REQUIRE(get_error_code(call_any_args_func(
            aa_func_obj_helper_set_value, 
            &obj_helper_1, 
            std::string("test 1 updated"))) == ErrorCode(0, 0));
        REQUIRE(get_error_code(call_any_args_func(
            aa_func_obj_helper_set_value, 
            &obj_helper_2, 
            std::string("test 2 updated"))) == ErrorCode(0, 0));
        REQUIRE(get_return(call_any_args_func(aa_func_obj_helper_get_value, &obj_helper_1)) == "test 1 updated");
        REQUIRE(get_return(call_any_args_func(aa_func_obj_helper_get_value, &obj_helper_2)) == "test 2 updated");

        auto aa_func_obj_helper_set_value_unsafe = make_any_args_func(&ObjectHelper::set_value, true);
        REQUIRE(get_error_code(call_any_args_func(
            aa_func_obj_helper_set_value_unsafe, 
            &obj_helper_1, 
            std::string("test 1 updated unsafe"))) == ErrorCode(0, 0));
        auto aa_func_obj_helper_get_value_unsafe = make_any_args_func(&ObjectHelper::get_value, false);
        REQUIRE(get_return(call_any_args_func(
            aa_func_obj_helper_get_value_unsafe, 
            &obj_helper_1)) == "test 1 updated unsafe");
    }

    SECTION("any args function class") {
        AnyArgsFunction<int> func_1(func_sum);
        std::function<int(int, int)> func_add = [](auto a, auto b) {return a + b;};
        AnyArgsFunction<int> func_2(func_add);
        AnyArgsFunc<int> aa_func = func_1;

        REQUIRE(get_return(func_1(1, 2)) == 3);
        REQUIRE(get_return(func_2(1, 2)) == 3);
        
        AnyArgsFunction<std::string> func_3(&ObjectHelper::get_value);
        ObjectHelper obj_helper_1("test 1");
        REQUIRE(get_return(func_3(&obj_helper_1)) == "test 1");
    }
}

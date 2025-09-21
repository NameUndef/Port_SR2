#include <catch2/catch_test_macros.hpp>
#include "id.hpp"

TEST_CASE( "IDs and values are compared", "[inner_lib][common][id]" )
{
    ID foo_id = "foo";
    ID bar_id = std::string("bar");
    ID num_0_id = 0;
    int num_0_int = 0;

    SECTION("compare strings") {
        std::string foo_str = "foo";
        std::string bar_str = "bar";

        // brackets for avoiding Catch2 bug with func matching with variant args
        REQUIRE((foo_id != bar_id));
        REQUIRE((bar_id != foo_id));
        REQUIRE((foo_id != bar_str));
        REQUIRE((bar_str != foo_id));
        REQUIRE((foo_str == foo_id));
    }

    SECTION("compare c-strings") {
        const char* foo_cstr = "foo";
        const char* bar_cstr = "bar";
        char* foo_cstr_nc = const_cast<char*>(foo_cstr);
        char* bar_cstr_nc = const_cast<char*>(bar_cstr);

        REQUIRE((foo_id != bar_cstr));
        REQUIRE((bar_cstr != foo_id));
        REQUIRE((foo_cstr == foo_id));

        REQUIRE((foo_id != bar_cstr_nc));
        REQUIRE((bar_cstr_nc != foo_id));
        REQUIRE((foo_cstr_nc == foo_id));
    }

    SECTION("compare nums") {
        ID num_1_id = 1;
        int num_1_int = 1;
        
        REQUIRE((num_0_id != num_1_id));
        REQUIRE((num_1_id != num_0_id));
        REQUIRE((num_0_id != num_1_int));
        REQUIRE((num_0_id == num_0_int));
        REQUIRE((num_0_int == num_0_id));
    }

    SECTION("compare strings and nums") {
        REQUIRE((foo_id != num_0_id));
        REQUIRE((num_0_id != foo_id));
        REQUIRE((foo_id != num_0_int));
        REQUIRE((num_0_int != foo_id));
    }
}

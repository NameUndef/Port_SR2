#include <catch2/catch_session.hpp>
#include <iostream>
int run_test_engine(int argc, char* argv[])
{
    int result = Catch::Session().run(argc, argv);
    return result;
}
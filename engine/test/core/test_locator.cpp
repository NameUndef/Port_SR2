#include <catch2/catch_test_macros.hpp>

class ISubprogramCommonInitializable {

    virtual void init() = 0;
    virtual void deinit() = 0;
};

class ISubprogramInitializable {

    virtual void init() = 0;
    virtual void deinit() = 0;
};

class ISubprogramStartable {

    virtual void start() = 0;
    virtual void stop() = 0;
};

class ISubprogramPausable {

    virtual void resume() = 0;
    virtual void pause() = 0;
};

class Locator {

};

TEST_CASE("Locator", "[core][locator]") {

    //Locator locator;
}
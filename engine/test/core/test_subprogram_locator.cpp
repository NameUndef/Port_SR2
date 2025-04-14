#include <catch2/catch_test_macros.hpp>
#include "core/subprogram_locator.hpp"
#include "any_args_func.hpp"

// SCENARIO("Subprogram_Locator", "[core][locator]") {

//     SubprogramInfo subprogram_test_template;

//     subprogram_test_template.init_ = make_any_args_func<bool>([](bool return_value) { return return_value; });
//     subprogram_test_template.deinit_ = 
//     subprogram_test_template.start_ = 
//     subprogram_test_template.stop_ = 
//     subprogram_test_template.resume_ = 
//     subprogram_test_template.pause_ =
//     subprogram_test_template.init_;
//     subprogram_test_template.is_multiinstance_supported_ = true;

//     GIVEN("one subprogram") {
//         SubprogramLocator subprogram_locator;
//         SubprogramInfo subprogram_test = subprogram_test_template;

//         REQUIRED(subprogram_locator.add(subprogram_test_template) == ErrorCode{0, 0});

//         WHEN("")
//         subprogram_locator.start(subprogram_test_template.name_);
//     }
// }

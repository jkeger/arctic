
#include <valarray>

#include "catch2/catch.hpp"
#include "traps.hpp"

TEST_CASE("Test trap species", "[traps]") {
    
    Trap trap_1(1.0, 1.0, 0.0);
    TrapInstantCapture trap_2(1.0, 2.0);
    
    SECTION("fill_fraction_from_time_elapsed") {        
        REQUIRE(trap_1.fill_fraction_from_time_elapsed(1.0) == exp(-1.0));
        REQUIRE(trap_2.fill_fraction_from_time_elapsed(1.0) == exp(-1.0 / 2.0));
        REQUIRE(trap_2.fill_fraction_from_time_elapsed(123.456) == exp(-123.456 / 2.0));
    }
}

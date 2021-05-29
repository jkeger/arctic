
#include <valarray>

#include "catch2/catch.hpp"
#include "traps.hpp"

TEST_CASE("Test trap species", "[traps]") {

    Trap trap_1(10.0, 1.0, 0.0);
    Trap trap_2(8.0, 1.0, 0.1);
    TrapInstantCapture trap_3(10.0, 2.0);
    
    SECTION("Initialisation") {
        REQUIRE(trap_1.watermark_type == watermark_type_standard);
        REQUIRE(trap_1.density == 10.0);
        REQUIRE(trap_1.release_timescale == 1.0);
        REQUIRE(trap_1.capture_timescale == 0.0);
        REQUIRE(trap_1.emission_rate == 1.0);
        REQUIRE(trap_1.capture_rate == 0.0);
        
        REQUIRE(trap_2.watermark_type == watermark_type_standard);
        REQUIRE(trap_2.density == 8.0);
        REQUIRE(trap_2.release_timescale == 1.0);
        REQUIRE(trap_2.capture_timescale == 0.1);
        REQUIRE(trap_2.emission_rate == 1.0);
        REQUIRE(trap_2.capture_rate == 10.0);
        
        REQUIRE(trap_3.watermark_type == watermark_type_instant_capture);
        REQUIRE(trap_3.density == 10.0);
        REQUIRE(trap_3.release_timescale == 2.0);
        REQUIRE(trap_3.capture_timescale == 0.0);
        REQUIRE(trap_3.emission_rate == 0.5);
        REQUIRE(trap_3.capture_rate == 0.0);
    }

    SECTION("Fill fraction from time elapsed") {
        REQUIRE(trap_1.fill_fraction_from_time_elapsed(1.0) == exp(-1.0));
        
        REQUIRE(trap_2.fill_fraction_from_time_elapsed(1.0) == exp(-1.0));
        
        REQUIRE(trap_3.fill_fraction_from_time_elapsed(1.0) == exp(-1.0 / 2.0));
        REQUIRE(trap_3.fill_fraction_from_time_elapsed(123.456) == exp(-123.456 / 2.0));
    }
}

TEST_CASE("Test continuum traps", "[traps]") {
    // Narrow and wide distributions of release lifetimes
    TrapContinuum trap_1(10.0, 1.0, 0.1);
    TrapContinuum trap_2(10.0, 1.0, 1.0);
    
    // Single release time for comparison
    TrapInstantCapture trap_ic(10.0, 1.0);
    
    SECTION("Initialisation") {
        REQUIRE(trap_1.watermark_type == watermark_type_continuum);
        REQUIRE(trap_1.density == 10.0);
        REQUIRE(trap_1.release_timescale == 1.0);
        REQUIRE(trap_1.release_timescale_sigma == 0.1);
        
        REQUIRE(trap_2.watermark_type == watermark_type_continuum);
        REQUIRE(trap_2.density == 10.0);
        REQUIRE(trap_2.release_timescale == 1.0);
        REQUIRE(trap_2.release_timescale_sigma == 1.0);
    }

    SECTION("Fill fraction from time elapsed") {
        double fill_1 = trap_1.fill_fraction_from_time_elapsed(1.0);
        double fill_2 = trap_2.fill_fraction_from_time_elapsed(1.0);
        double fill_ic = trap_ic.fill_fraction_from_time_elapsed(1.0);
    
        // Similar(ish) to the single-release-time trap
        REQUIRE(fill_1 == Approx(fill_ic).epsilon(1e-4));
        REQUIRE(fill_2 == Approx(fill_ic).epsilon(0.05));
        REQUIRE(fill_ic < fill_1);
        REQUIRE(fill_1 < fill_2);
        
        // Full and empty
        REQUIRE(trap_1.fill_fraction_from_time_elapsed(0.0) == Approx(1.0));
        REQUIRE(trap_1.fill_fraction_from_time_elapsed(
            std::numeric_limits<double>::max()) == Approx(0.0));
    }
}

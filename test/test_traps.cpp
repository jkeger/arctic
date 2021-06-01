
#include <valarray>

#include "catch2/catch.hpp"
#include "traps.hpp"

TEST_CASE("Test instant-capture and slow-capture traps", "[traps]") {

    TrapInstantCapture trap_1(10.0, 2.0);
    TrapSlowCapture trap_2(10.0, 1.0, 0.0);
    TrapSlowCapture trap_3(8.0, 1.0, 0.1);

    SECTION("Initialisation") {
        REQUIRE(trap_1.density == 10.0);
        REQUIRE(trap_1.release_timescale == 2.0);
        REQUIRE(trap_1.emission_rate == 0.5);

        REQUIRE(trap_2.density == 10.0);
        REQUIRE(trap_2.release_timescale == 1.0);
        REQUIRE(trap_2.capture_timescale == 0.0);
        REQUIRE(trap_2.emission_rate == 1.0);
        REQUIRE(trap_2.capture_rate == 0.0);

        REQUIRE(trap_3.density == 8.0);
        REQUIRE(trap_3.release_timescale == 1.0);
        REQUIRE(trap_3.capture_timescale == 0.1);
        REQUIRE(trap_3.emission_rate == 1.0);
        REQUIRE(trap_3.capture_rate == 10.0);
    }

    SECTION("Fill fraction from time elapsed") {
        REQUIRE(trap_1.fill_fraction_from_time_elapsed(1.0) == exp(-1.0 / 2.0));
        REQUIRE(trap_1.fill_fraction_from_time_elapsed(123.456) == exp(-123.456 / 2.0));

        REQUIRE(trap_2.fill_fraction_from_time_elapsed(1.0) == exp(-1.0));

        REQUIRE(trap_3.fill_fraction_from_time_elapsed(1.0) == exp(-1.0));
    }
}

TEST_CASE("Test continuum traps", "[traps]") {
    // Narrow and wide distributions of release lifetimes
    TrapContinuum trap_1(10.0, -1.0 / log(0.5), 0.1);
    TrapContinuum trap_2(10.0, -1.0 / log(0.5), 1.0);

    SECTION("Initialisation") {
        REQUIRE(trap_1.density == 10.0);
        REQUIRE(trap_1.release_timescale == -1.0 / log(0.5));
        REQUIRE(trap_1.release_timescale_sigma == 0.1);

        REQUIRE(trap_2.density == 10.0);
        REQUIRE(trap_2.release_timescale == -1.0 / log(0.5));
        REQUIRE(trap_2.release_timescale_sigma == 1.0);
    }

    SECTION("Fill fraction from time elapsed") {
        // Similar(ish) to a single release time
        REQUIRE(
            trap_1.fill_fraction_from_time_elapsed(1.0) == Approx(0.5).epsilon(0.01));
        REQUIRE(
            trap_2.fill_fraction_from_time_elapsed(1.0) == Approx(0.5).epsilon(0.05));

        REQUIRE(
            trap_1.fill_fraction_from_time_elapsed(2.0) == Approx(0.25).epsilon(0.01));
        REQUIRE(
            trap_2.fill_fraction_from_time_elapsed(2.0) == Approx(0.25).epsilon(0.2));

        // Full and empty
        REQUIRE(trap_1.fill_fraction_from_time_elapsed(0.0) == Approx(1.0));
        REQUIRE(
            trap_1.fill_fraction_from_time_elapsed(
                std::numeric_limits<double>::max()) == Approx(0.0));
    }

    SECTION("Time elapsed from fill fraction") {
        double time_max = 999;

        // Similar(ish) to a single release time
        REQUIRE(
            trap_1.time_elapsed_from_fill_fraction(0.5, time_max) ==
            Approx(1.0).epsilon(0.01));
        REQUIRE(
            trap_2.time_elapsed_from_fill_fraction(0.5, time_max) ==
            Approx(1.0).epsilon(0.1));

        REQUIRE(
            trap_1.time_elapsed_from_fill_fraction(0.25, time_max) ==
            Approx(2.0).epsilon(0.01));
        REQUIRE(
            trap_2.time_elapsed_from_fill_fraction(0.25, time_max) ==
            Approx(2.0).epsilon(0.25));

        // Full and empty
        REQUIRE(trap_1.time_elapsed_from_fill_fraction(1.0, time_max) == Approx(0.0));
        REQUIRE(
            trap_1.time_elapsed_from_fill_fraction(0.0, time_max) >=
            std::numeric_limits<double>::max());

        // Convert and back
        REQUIRE(
            1.234 == Approx(trap_1.time_elapsed_from_fill_fraction(
                         trap_1.fill_fraction_from_time_elapsed(1.234), time_max)));
        REQUIRE(
            2.468 == Approx(trap_2.time_elapsed_from_fill_fraction(
                         trap_2.fill_fraction_from_time_elapsed(2.468), time_max)));
    }
}

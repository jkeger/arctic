
#include <valarray>

#include "catch2/catch.hpp"
#include "traps.hpp"
#include "util.hpp"

TEST_CASE("Test instant-capture and slow-capture traps", "[traps]") {

    TrapInstantCapture trap_1(10.0, 2.0);
    TrapSlowCapture trap_2(10.0, 1.0, 0.0);
    TrapSlowCapture trap_3(8.0, 1.0, 0.1);
    TrapInstantCapture trap_4(10.0, 1.0, 0.6, 0.8);

    SECTION("Initialisation") {
        REQUIRE(trap_1.density == 10.0);
        REQUIRE(trap_1.release_timescale == 2.0);
        REQUIRE(trap_1.release_rate == 0.5);

        REQUIRE(trap_2.density == 10.0);
        REQUIRE(trap_2.release_timescale == 1.0);
        REQUIRE(trap_2.capture_timescale == 0.0);
        REQUIRE(trap_2.release_rate == 1.0);
        REQUIRE(trap_2.capture_rate == 0.0);

        REQUIRE(trap_3.density == 8.0);
        REQUIRE(trap_3.release_timescale == 1.0);
        REQUIRE(trap_3.capture_timescale == 0.1);
        REQUIRE(trap_3.release_rate == 1.0);
        REQUIRE(trap_3.capture_rate == 10.0);

        // Distribution with volume
        REQUIRE(trap_1.fractional_volume_none_exposed == 0.0);
        REQUIRE(trap_1.fractional_volume_full_exposed == 0.0);

        REQUIRE(trap_4.fractional_volume_none_exposed == 0.6);
        REQUIRE(trap_4.fractional_volume_full_exposed == 0.8);
        REQUIRE(trap_4.density == Approx(10.0 / 0.3));
        REQUIRE(trap_4.release_timescale == 1.0);
        REQUIRE(trap_4.release_rate == 1.0);
    }

    SECTION("Fill fraction from time elapsed") {
        REQUIRE(trap_1.fill_fraction_from_time_elapsed(1.0) == exp(-1.0 / 2.0));
        REQUIRE(trap_1.fill_fraction_from_time_elapsed(123.456) == exp(-123.456 / 2.0));

        REQUIRE(trap_2.fill_fraction_from_time_elapsed(1.0) == exp(-1.0));

        REQUIRE(trap_3.fill_fraction_from_time_elapsed(1.0) == exp(-1.0));
    }

    SECTION("Fraction traps exposed per fractional volume") {
        // Default, always 1
        TrapInstantCapture trap_A(1.0, 1.0);
        REQUIRE(trap_A.fraction_traps_exposed_per_fractional_volume(0.1, 0.2) == 1.0);
        REQUIRE(trap_A.fraction_traps_exposed_per_fractional_volume(0.0, 1.0) == 1.0);

        // Step function
        TrapInstantCapture trap_B(1.0, 1.0, 0.8, 0.8);
        REQUIRE(trap_B.fraction_traps_exposed_per_fractional_volume(0.1, 0.2) == 0.0);
        REQUIRE(trap_B.fraction_traps_exposed_per_fractional_volume(0.9, 1.0) == 1.0);
        REQUIRE(
            trap_B.fraction_traps_exposed_per_fractional_volume(0.0, 1.0) ==
            Approx(0.2));
        REQUIRE(
            trap_B.fraction_traps_exposed_per_fractional_volume(0.7, 0.9) ==
            Approx(0.5));

        // Linear
        TrapInstantCapture trap_C(1.0, 1.0, 0.4, 0.6);
        REQUIRE(trap_C.fraction_traps_exposed_per_fractional_volume(0.1, 0.2) == 0.0);
        REQUIRE(trap_C.fraction_traps_exposed_per_fractional_volume(0.9, 1.0) == 1.0);
        REQUIRE(
            trap_C.fraction_traps_exposed_per_fractional_volume(0.0, 1.0) ==
            Approx(0.1 + 0.4));
        REQUIRE(
            trap_C.fraction_traps_exposed_per_fractional_volume(0.0, 0.5) ==
            Approx(0.025 / 0.5));
        REQUIRE(
            trap_C.fraction_traps_exposed_per_fractional_volume(0.3, 0.5) ==
            Approx(0.025 / 0.2));
        REQUIRE(
            trap_C.fraction_traps_exposed_per_fractional_volume(0.5, 1.0) ==
            Approx((0.025 + 0.05 + 0.4) / 0.5));
        REQUIRE(
            trap_C.fraction_traps_exposed_per_fractional_volume(0.5, 0.7) ==
            Approx((0.05 + 0.025 + 0.1) / 0.2));
        REQUIRE(
            trap_C.fraction_traps_exposed_per_fractional_volume(0.4, 0.6) ==
            Approx(0.1 / 0.2));
        REQUIRE(
            trap_C.fraction_traps_exposed_per_fractional_volume(0.45, 0.55) ==
            Approx((0.025 + 0.025) / 0.1));

        // Linear 2
        TrapInstantCapture trap_D(1.0, 1.0, 0.6, 0.8);
        REQUIRE(trap_D.fraction_traps_exposed_per_fractional_volume(0.1, 0.2) == 0.0);
        REQUIRE(trap_D.fraction_traps_exposed_per_fractional_volume(0.9, 1.0) == 1.0);
        REQUIRE(
            trap_D.fraction_traps_exposed_per_fractional_volume(0.0, 1.0) ==
            Approx(0.1 + 0.2));
        REQUIRE(
            trap_D.fraction_traps_exposed_per_fractional_volume(0.0, 0.7) ==
            Approx(0.025 / 0.7));
        REQUIRE(
            trap_D.fraction_traps_exposed_per_fractional_volume(0.5, 0.7) ==
            Approx(0.025 / 0.2));
        REQUIRE(
            trap_D.fraction_traps_exposed_per_fractional_volume(0.7, 1.0) ==
            Approx((0.025 + 0.05 + 0.2) / 0.3));
        REQUIRE(
            trap_D.fraction_traps_exposed_per_fractional_volume(0.7, 0.9) ==
            Approx((0.05 + 0.025 + 0.1) / 0.2));
        REQUIRE(
            trap_D.fraction_traps_exposed_per_fractional_volume(0.6, 0.8) ==
            Approx(0.1 / 0.2));
        REQUIRE(
            trap_D.fraction_traps_exposed_per_fractional_volume(0.65, 0.75) ==
            Approx((0.025 + 0.025) / 0.1));
    }
}

TEST_CASE("Test instant-capture continuum traps", "[traps]") {
    // Narrow and wide distributions of release lifetimes
    TrapInstantCaptureContinuum trap_1(10.0, -1.0 / log(0.5), 0.1);
    TrapInstantCaptureContinuum trap_2(10.0, -1.0 / log(0.5), 1.0);
    // Small mu and sigma
    TrapInstantCaptureContinuum trap_3(10.0, -0.1 / log(0.5), 0.001);

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

        // Small mu and sigma
        REQUIRE(trap_3.fill_fraction_from_time_elapsed(0.1) == Approx(0.5));

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

        // Small mu and sigma
        REQUIRE(
            1.234 == Approx(trap_3.time_elapsed_from_fill_fraction(
                         trap_3.fill_fraction_from_time_elapsed(1.234), time_max)));
    }

    int n_intp = 1000;
    double time_min = 0.1;
    double time_max = 99;

    trap_1.prep_fill_fraction_and_time_elapsed_tables(time_min, time_max, n_intp);
    trap_2.prep_fill_fraction_and_time_elapsed_tables(time_min, time_max, n_intp);

    SECTION("Prep fill fraction and time elapsed table") {
        // Input variables
        REQUIRE(trap_1.n_intp == n_intp);
        REQUIRE(trap_1.fill_fraction_table.size() == n_intp);
        REQUIRE(trap_1.time_min == time_min);
        REQUIRE(trap_1.time_max == time_max);

        // Derived variables
        REQUIRE(trap_1.d_log_time == (log(time_max) - log(time_min)) / (n_intp - 1));
        double fill_min = trap_1.fill_fraction_from_time_elapsed(time_max);
        double fill_max = trap_1.fill_fraction_from_time_elapsed(time_min);
        REQUIRE(trap_1.fill_min == fill_min);
        REQUIRE(trap_1.fill_max == fill_max);

        // End table values
        REQUIRE(trap_1.fill_fraction_table[0] == Approx(fill_min));
        REQUIRE(trap_1.fill_fraction_table[n_intp - 1] == Approx(fill_max));
    }

    SECTION("Fill fraction from time elapsed from table") {
        // Inside table
        for (double log10_time = -1.0; log10_time <= 1.8; log10_time += 0.2) {
            double time = pow(10, log10_time);
            REQUIRE(
                trap_2.fill_fraction_from_time_elapsed_table(time) ==
                Approx(trap_2.fill_fraction_from_time_elapsed(time))
                    .epsilon(1e-4)
                    .margin(1e-7));
            REQUIRE(
                trap_2.fill_fraction_from_time_elapsed_table(time) ==
                Approx(trap_2.fill_fraction_from_time_elapsed(time))
                    .epsilon(1e-4)
                    .margin(1e-7));
        }

        // Outside table still close
        REQUIRE(
            trap_2.fill_fraction_from_time_elapsed_table(0.05) ==
            Approx(trap_2.fill_fraction_from_time_elapsed(0.05)).epsilon(1e-1));
        REQUIRE(
            trap_2.fill_fraction_from_time_elapsed_table(100) ==
            Approx(trap_2.fill_fraction_from_time_elapsed(100)).epsilon(1e-3));

        // Far outside table goes to full and empty
        REQUIRE(trap_2.fill_fraction_from_time_elapsed_table(0.01) == 1.0);
        REQUIRE(trap_2.fill_fraction_from_time_elapsed_table(200) == 0.0);

        // Full and empty
        REQUIRE(trap_2.fill_fraction_from_time_elapsed_table(0.0) == Approx(1.0));
        REQUIRE(
            trap_2.fill_fraction_from_time_elapsed_table(
                std::numeric_limits<double>::max()) == Approx(0.0));
    }

    SECTION("Time elapsed from fill fraction from table") {
        // Inside table
        for (double log10_fill = -2; log10_fill < -0.2; log10_fill += 0.2) {
            double fill = pow(10, log10_fill);
            REQUIRE(
                trap_2.time_elapsed_from_fill_fraction_table(fill) ==
                Approx(trap_2.time_elapsed_from_fill_fraction(fill, time_max))
                    .epsilon(1e-4));
            REQUIRE(
                trap_2.time_elapsed_from_fill_fraction_table(fill) ==
                Approx(trap_2.time_elapsed_from_fill_fraction(fill, time_max))
                    .epsilon(1e-4));
        }

        // Outside table
        double tiny_fill = trap_2.fill_fraction_from_time_elapsed(time_max * 1.1);
        REQUIRE(
            trap_2.time_elapsed_from_fill_fraction_table(tiny_fill) ==
            Approx(trap_2.time_elapsed_from_fill_fraction(tiny_fill, 2 * time_max))
                .epsilon(2e-2));
        REQUIRE(
            trap_2.time_elapsed_from_fill_fraction_table(0.95) ==
            Approx(trap_2.time_elapsed_from_fill_fraction(0.95, time_max))
                .margin(2e-2));

        // Far outside table
        tiny_fill = trap_2.fill_fraction_from_time_elapsed(time_max * 2);
        REQUIRE(
            trap_2.time_elapsed_from_fill_fraction_table(tiny_fill) ==
            Approx(trap_2.time_elapsed_from_fill_fraction(tiny_fill, 3 * time_max))
                .epsilon(0.5));
        REQUIRE(
            trap_2.time_elapsed_from_fill_fraction_table(0.99) ==
            Approx(trap_2.time_elapsed_from_fill_fraction(0.99, time_max))
                .margin(3e-2));

        // Full and empty
        REQUIRE(trap_2.time_elapsed_from_fill_fraction_table(1.0) == Approx(0.0));
        REQUIRE(
            trap_2.time_elapsed_from_fill_fraction_table(0.0) >=
            std::numeric_limits<double>::max());
    }
}

TEST_CASE("Test slow-capture continuum traps", "[traps]") {
    // Narrow and wide distributions, and fast and slow capture
    TrapSlowCaptureContinuum trap_1(10.0, -1.0 / log(0.5), 0.05, 0.1);
    TrapSlowCaptureContinuum trap_2(8.0, -1.0 / log(0.5), 0.5, 1.0);
    // Close to single-lifetime and instant-capture
    TrapSlowCaptureContinuum trap_3(10.0, -1.0 / log(0.5), 0.01, 0.01);
    // Small mu and sigma
    TrapSlowCaptureContinuum trap_4(10.0, -0.1 / log(0.5), 0.001, 0.001);

    SECTION("Initialisation") {
        REQUIRE(trap_1.density == 10.0);
        REQUIRE(trap_1.release_timescale == -1.0 / log(0.5));
        REQUIRE(trap_1.release_rate == -log(0.5));
        REQUIRE(trap_1.release_timescale_sigma == 0.05);
        REQUIRE(trap_1.capture_timescale == 0.1);
        REQUIRE(trap_1.capture_rate == 10.0);

        REQUIRE(trap_2.density == 8.0);
        REQUIRE(trap_2.release_timescale == -1.0 / log(0.5));
        REQUIRE(trap_2.release_rate == -log(0.5));
        REQUIRE(trap_2.release_timescale_sigma == 0.5);
        REQUIRE(trap_2.capture_timescale == 1.0);
        REQUIRE(trap_2.capture_rate == 1.0);
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

        // Small mu and sigma
        REQUIRE(
            trap_4.fill_fraction_from_time_elapsed(0.1) == Approx(0.5).epsilon(0.01));

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

        // Small mu and sigma
        REQUIRE(
            1.234 == Approx(trap_4.time_elapsed_from_fill_fraction(
                         trap_4.fill_fraction_from_time_elapsed(1.234), time_max)));
    }

    int n_intp = 1000;
    double time_min = 0.1;
    double time_max = 99;

    trap_1.prep_fill_fraction_and_time_elapsed_tables(time_min, time_max, n_intp);
    trap_2.prep_fill_fraction_and_time_elapsed_tables(time_min, time_max, n_intp);

    SECTION("Prep fill fraction and time elapsed table") {
        // Input variables
        REQUIRE(trap_1.n_intp == n_intp);
        REQUIRE(trap_1.fill_fraction_table.size() == n_intp);
        REQUIRE(trap_1.time_min == time_min);
        REQUIRE(trap_1.time_max == time_max);

        // Derived variables
        REQUIRE(trap_1.d_log_time == (log(time_max) - log(time_min)) / (n_intp - 1));
        double fill_min = trap_1.fill_fraction_from_time_elapsed(time_max);
        double fill_max = trap_1.fill_fraction_from_time_elapsed(time_min);
        REQUIRE(trap_1.fill_min == fill_min);
        REQUIRE(trap_1.fill_max == fill_max);

        // End table values
        REQUIRE(trap_1.fill_fraction_table[0] == Approx(fill_min));
        REQUIRE(trap_1.fill_fraction_table[n_intp - 1] == Approx(fill_max));
    }

    SECTION("Fill fraction from time elapsed from table") {
        // Inside table
        for (double log10_time = -1.0; log10_time <= 1.8; log10_time += 0.2) {
            double time = pow(10, log10_time);
            REQUIRE(
                trap_2.fill_fraction_from_time_elapsed_table(time) ==
                Approx(trap_2.fill_fraction_from_time_elapsed(time))
                    .epsilon(1e-4)
                    .margin(1e-7));
            REQUIRE(
                trap_2.fill_fraction_from_time_elapsed_table(time) ==
                Approx(trap_2.fill_fraction_from_time_elapsed(time))
                    .epsilon(1e-4)
                    .margin(1e-7));
        }

        // Outside table still close
        REQUIRE(
            trap_2.fill_fraction_from_time_elapsed_table(0.05) ==
            Approx(trap_2.fill_fraction_from_time_elapsed(0.05)).epsilon(1e-1));
        REQUIRE(
            trap_2.fill_fraction_from_time_elapsed_table(100) ==
            Approx(trap_2.fill_fraction_from_time_elapsed(100)).epsilon(1e-2));

        // Far outside table goes to full and empty
        REQUIRE(trap_2.fill_fraction_from_time_elapsed_table(0.01) == 1.0);
        REQUIRE(trap_2.fill_fraction_from_time_elapsed_table(200) == 0.0);

        // Full and empty
        REQUIRE(trap_2.fill_fraction_from_time_elapsed_table(0.0) == Approx(1.0));
        REQUIRE(
            trap_2.fill_fraction_from_time_elapsed_table(
                std::numeric_limits<double>::max()) == Approx(0.0));
    }

    SECTION("Time elapsed from fill fraction from table") {
        // Inside table
        for (double log10_fill = -2; log10_fill < -0.2; log10_fill += 0.2) {
            double fill = pow(10, log10_fill);
            REQUIRE(
                trap_2.time_elapsed_from_fill_fraction_table(fill) ==
                Approx(trap_2.time_elapsed_from_fill_fraction(fill, time_max))
                    .epsilon(1e-4));
            REQUIRE(
                trap_2.time_elapsed_from_fill_fraction_table(fill) ==
                Approx(trap_2.time_elapsed_from_fill_fraction(fill, time_max))
                    .epsilon(1e-4));
        }

        // Outside table still close
        double tiny_fill = trap_2.fill_fraction_from_time_elapsed(time_max * 1.1);
        REQUIRE(
            trap_2.time_elapsed_from_fill_fraction_table(tiny_fill) ==
            Approx(trap_2.time_elapsed_from_fill_fraction(tiny_fill, 2 * time_max))
                .epsilon(5e-2));
        REQUIRE(
            trap_2.time_elapsed_from_fill_fraction_table(0.95) ==
            Approx(trap_2.time_elapsed_from_fill_fraction(0.95, time_max))
                .margin(5e-2));

        // Far outside table still closeish
        tiny_fill = trap_2.fill_fraction_from_time_elapsed(time_max * 2);
        REQUIRE(
            trap_2.time_elapsed_from_fill_fraction_table(tiny_fill) ==
            Approx(trap_2.time_elapsed_from_fill_fraction(tiny_fill, 3 * time_max))
                .epsilon(0.5));
        REQUIRE(
            trap_2.time_elapsed_from_fill_fraction_table(0.99) ==
            Approx(trap_2.time_elapsed_from_fill_fraction(0.99, time_max))
                .margin(3e-2));

        // Full and empty
        REQUIRE(trap_2.time_elapsed_from_fill_fraction_table(1.0) == Approx(0.0));
        REQUIRE(
            trap_2.time_elapsed_from_fill_fraction_table(0.0) >=
            std::numeric_limits<double>::max());
    }

    SECTION("Fill fraction after slow capture") {
        double dwell_time = 1.0;

        // Almost-instant capture
        REQUIRE(
            trap_3.fill_fraction_after_slow_capture(1.0, dwell_time) ==
            Approx(1.0).epsilon(0.01));

        // Fairly fast capture
        REQUIRE(
            trap_1.fill_fraction_after_slow_capture(1.0, dwell_time) ==
            Approx(1.0).epsilon(0.1));

        // Small mu and sigma
        REQUIRE(
            trap_4.fill_fraction_after_slow_capture(1.0, dwell_time) ==
            Approx(1.0).epsilon(0.01));

        // Larger final fill from longer dwell time
        REQUIRE(
            trap_1.fill_fraction_after_slow_capture(1.0, 0.5) <
            trap_1.fill_fraction_after_slow_capture(1.0, 1.0));
        REQUIRE(
            trap_1.fill_fraction_after_slow_capture(1.0, 1.0) <
            trap_1.fill_fraction_after_slow_capture(1.0, 2.0));

        REQUIRE(
            trap_2.fill_fraction_after_slow_capture(1.0, 0.5) <
            trap_2.fill_fraction_after_slow_capture(1.0, 1.0));
        REQUIRE(
            trap_2.fill_fraction_after_slow_capture(1.0, 1.0) <
            trap_2.fill_fraction_after_slow_capture(1.0, 2.0));

        // Converges for long dwell times
        REQUIRE(
            trap_1.fill_fraction_after_slow_capture(1.0, 10.0) ==
            Approx(trap_1.fill_fraction_after_slow_capture(1.0, 20.0)));

        REQUIRE(
            trap_2.fill_fraction_after_slow_capture(1.0, 10.0) ==
            Approx(trap_2.fill_fraction_after_slow_capture(1.0, 20.0)));

        // Larger final fill from shorter elapsed time (i.e. larger initial fill)
        REQUIRE(
            trap_1.fill_fraction_after_slow_capture(0.0, dwell_time) >
            trap_1.fill_fraction_after_slow_capture(1.0, dwell_time));
        REQUIRE(
            trap_1.fill_fraction_after_slow_capture(10.0, dwell_time) >
            trap_1.fill_fraction_after_slow_capture(20.0, dwell_time));

        REQUIRE(
            trap_2.fill_fraction_after_slow_capture(0.0, dwell_time) >
            trap_2.fill_fraction_after_slow_capture(1.0, dwell_time));
        REQUIRE(
            trap_2.fill_fraction_after_slow_capture(10.0, dwell_time) >
            trap_2.fill_fraction_after_slow_capture(20.0, dwell_time));
    }

    double dwell_time = 1.0;
    trap_1.prep_fill_fraction_after_slow_capture_tables(
        dwell_time, time_min, time_max, n_intp);
    trap_2.prep_fill_fraction_after_slow_capture_tables(
        dwell_time, time_min, time_max, n_intp);

    SECTION("Prep fill fraction after slow capture table") {
        // Derived variables
        double fill_capture_min =
            trap_1.fill_fraction_after_slow_capture(time_max, dwell_time);
        double fill_capture_max =
            trap_1.fill_fraction_after_slow_capture(time_min, dwell_time);
        REQUIRE(trap_1.fill_capture_min == fill_capture_min);
        REQUIRE(trap_1.fill_capture_max == fill_capture_max);

        // End table values
        REQUIRE(trap_1.fill_fraction_capture_table[0] == Approx(fill_capture_min));
        REQUIRE(
            trap_1.fill_fraction_capture_table[n_intp - 1] == Approx(fill_capture_max));

        REQUIRE(
            trap_1.fill_capture_long_time ==
            trap_1.fill_fraction_after_slow_capture(time_max * 100, dwell_time));
    }

    SECTION("Fill fraction after slow capture from table") {
        // Inside table
        for (double log10_time = -1.0; log10_time <= 1.8; log10_time += 0.2) {
            double time = pow(10, log10_time);
            REQUIRE(
                trap_2.fill_fraction_after_slow_capture_table(time) ==
                Approx(trap_2.fill_fraction_after_slow_capture(time, dwell_time))
                    .epsilon(1e-4)
                    .margin(1e-7));
            REQUIRE(
                trap_2.fill_fraction_after_slow_capture_table(time) ==
                Approx(trap_2.fill_fraction_after_slow_capture(time, dwell_time))
                    .epsilon(1e-4)
                    .margin(1e-7));
        }

        // Outside table still close, and long times converge
        REQUIRE(
            trap_2.fill_fraction_after_slow_capture_table(0.05) ==
            Approx(trap_2.fill_fraction_after_slow_capture(0.05, dwell_time))
                .epsilon(1e-2));
        REQUIRE(
            trap_2.fill_fraction_after_slow_capture_table(100) ==
            Approx(trap_2.fill_fraction_after_slow_capture(100, dwell_time)));

        // Far outside table still closeish, and long times converge
        REQUIRE(
            trap_2.fill_fraction_after_slow_capture_table(0.01) ==
            Approx(trap_2.fill_fraction_after_slow_capture(0.01, dwell_time))
                .epsilon(1e-1));
        REQUIRE(
            trap_2.fill_fraction_after_slow_capture_table(1000) ==
            Approx(trap_2.fill_fraction_after_slow_capture(1000, dwell_time)));
        REQUIRE(
            trap_2.fill_fraction_after_slow_capture_table(
                std::numeric_limits<double>::max()) == trap_2.fill_capture_long_time);
    }
}

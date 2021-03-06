
#include <stdio.h>
#include <valarray>
#include <vector>

#include "catch2/catch.hpp"
#include "ccd.hpp"
#include "trap_managers.hpp"
#include "traps.hpp"
#include "util.hpp"

CCD ccd(1e4, 0.0, 1.0);

TEST_CASE("Test initialisation", "[trap_managers]") {
    Trap trap_1(1.0, 1.0, 0.0);
    Trap trap_2(2.0, 2.0, 0.0);
    TrapInstantCapture trap_3(3.0, 3.0);

    SECTION("Traps") {
        // Standard traps
        std::valarray<Trap> traps{trap_1, trap_2};
        TrapManager trap_manager(traps, 123, ccd);

        REQUIRE(trap_manager.n_traps == 2);
        REQUIRE(trap_manager.traps[0].density == trap_1.density);
        REQUIRE(trap_manager.traps[1].density == trap_2.density);

        // Instant-capture traps
        std::valarray<Trap> traps_ic{trap_3};
        TrapManagerInstantCapture trap_manager_ic(traps_ic, 123, ccd);

        REQUIRE(trap_manager_ic.n_traps == 1);
        REQUIRE(trap_manager_ic.traps[0].density == trap_3.density);
    }

    SECTION("CCD") {
        // Standard traps
        std::valarray<Trap> traps{trap_1, trap_2};
        TrapManager trap_manager(traps, 123, ccd);

        REQUIRE(trap_manager.ccd.full_well_depth == ccd.full_well_depth);
        REQUIRE(trap_manager.ccd.well_notch_depth == ccd.well_notch_depth);
        REQUIRE(trap_manager.ccd.well_fill_power == ccd.well_fill_power);

        // Instant-capture traps
        std::valarray<Trap> traps_ic{trap_3};
        TrapManagerInstantCapture trap_manager_ic(traps_ic, 123, ccd);

        REQUIRE(trap_manager_ic.ccd.full_well_depth == ccd.full_well_depth);
        REQUIRE(trap_manager_ic.ccd.well_notch_depth == ccd.well_notch_depth);
        REQUIRE(trap_manager_ic.ccd.well_fill_power == ccd.well_fill_power);
    }

    SECTION("Misc attributes") {
        // Standard traps
        TrapManager trap_manager(std::valarray<Trap>{trap_1, trap_2}, 123, ccd);

        REQUIRE(trap_manager.max_n_transfers == 123);
        REQUIRE(trap_manager.n_watermarks_per_transfer == 2);
        REQUIRE(trap_manager.empty_watermark == 0.0);
        REQUIRE(trap_manager.n_active_watermarks == 0);
        REQUIRE(trap_manager.i_first_active_wmk == 0);

        // Instant-capture traps
        TrapManagerInstantCapture trap_manager_ic(
            std::valarray<Trap>{trap_3}, 123, ccd);

        REQUIRE(trap_manager_ic.max_n_transfers == 123);
        REQUIRE(trap_manager_ic.n_watermarks_per_transfer == 1);
        REQUIRE(trap_manager_ic.empty_watermark == 0.0);
        REQUIRE(trap_manager_ic.n_active_watermarks == 0);
        REQUIRE(trap_manager_ic.i_first_active_wmk == 0);
    }

    SECTION("Initial watermarks") {
        // Standard traps
        TrapManager trap_manager(std::valarray<Trap>{trap_1, trap_2}, 3, ccd);
        trap_manager.initialise_trap_states();

        REQUIRE(trap_manager.n_watermarks == 7);
        REQUIRE(trap_manager.watermark_volumes.size() == 7);
        REQUIRE(trap_manager.watermark_fills.size() == 7 * 2);
        REQUIRE(
            trap_manager.watermark_volumes.sum() ==
            trap_manager.n_watermarks * trap_manager.empty_watermark);
        REQUIRE(
            trap_manager.watermark_fills.sum() ==
            trap_manager.n_watermarks * 2 * trap_manager.empty_watermark);

        TrapManager trap_manager_2(std::valarray<Trap>{trap_1}, 123, ccd);
        trap_manager_2.initialise_trap_states();

        REQUIRE(trap_manager_2.n_watermarks == 247);
        REQUIRE(trap_manager_2.watermark_volumes.size() == 247);
        REQUIRE(trap_manager_2.watermark_fills.size() == 247);
        REQUIRE(
            trap_manager_2.watermark_volumes.sum() ==
            trap_manager_2.n_watermarks * trap_manager_2.empty_watermark);
        REQUIRE(
            trap_manager_2.watermark_fills.sum() ==
            trap_manager_2.n_watermarks * trap_manager_2.empty_watermark);

        // Instant-capture traps
        TrapManagerInstantCapture trap_manager_ic(std::valarray<Trap>{trap_3}, 3, ccd);
        trap_manager_ic.initialise_trap_states();

        REQUIRE(trap_manager_ic.n_watermarks == 4);
        REQUIRE(trap_manager_ic.watermark_volumes.size() == 4);
        REQUIRE(trap_manager_ic.watermark_fills.size() == 4);
        REQUIRE(
            trap_manager_ic.watermark_volumes.sum() ==
            trap_manager_ic.n_watermarks * trap_manager_ic.empty_watermark);
        REQUIRE(
            trap_manager_ic.watermark_fills.sum() ==
            trap_manager_ic.n_watermarks * trap_manager_ic.empty_watermark);
    }
}

TEST_CASE("Test utilities", "[trap_managers]") {
    Trap trap_1(10.0, -1.0 / log(0.5), 0.0);
    Trap trap_2(8.0, -1.0 / log(0.2), 0.0);
    TrapInstantCapture trap_3(10.0, -1.0 / log(0.5));
    TrapInstantCapture trap_4(8.0, -1.0 / log(0.2));

    SECTION("Number of trapped electrons") {
        double n_trapped_electrons;

        // Standard traps
        TrapManager trap_manager(std::valarray<Trap>{trap_1}, 2, ccd);
        trap_manager.initialise_trap_states();
        trap_manager.n_active_watermarks = 3;
        trap_manager.watermark_volumes = {0.5, 0.2, 0.1, 0.0, 0.0};
        trap_manager.watermark_fills = {0.8, 0.4, 0.2, 0.0, 0.0};
        n_trapped_electrons = trap_manager.n_trapped_electrons_from_watermarks(
            trap_manager.watermark_volumes, trap_manager.watermark_fills);
        REQUIRE(
            n_trapped_electrons ==
            (0.5 * 0.8 + 0.2 * 0.4 + 0.1 * 0.2) * trap_1.density);

        TrapManager trap_manager_2(std::valarray<Trap>{trap_1, trap_2}, 2, ccd);
        trap_manager_2.initialise_trap_states();
        trap_manager_2.n_active_watermarks = 3;
        trap_manager_2.watermark_volumes = {0.5, 0.2, 0.1, 0.0, 0.0};
        trap_manager_2.watermark_fills = {
            // clang-format off
            0.8, 0.3, 
            0.4, 0.2,
            0.2, 0.1,
            0.0, 0.0,
            0.0, 0.0
            // clang-format on
        };
        n_trapped_electrons = trap_manager_2.n_trapped_electrons_from_watermarks(
            trap_manager_2.watermark_volumes, trap_manager_2.watermark_fills);
        REQUIRE(
            n_trapped_electrons ==
            (0.5 * 0.8 + 0.2 * 0.4 + 0.1 * 0.2) * trap_1.density +
                (0.5 * 0.3 + 0.2 * 0.2 + 0.1 * 0.1) * trap_2.density);

        // i_first_active_wmk > 0
        trap_manager_2.initialise_trap_states();
        trap_manager_2.n_active_watermarks = 3;
        trap_manager_2.i_first_active_wmk = 2;
        trap_manager_2.watermark_volumes = {0.4, 0.3, 0.5, 0.2, 0.1};
        trap_manager_2.watermark_fills = {
            // clang-format off
            0.8, 0.3,
            0.4, 0.2,
            0.8, 0.3,
            0.4, 0.2,
            0.2, 0.1,
            // clang-format on
        };
        n_trapped_electrons = trap_manager_2.n_trapped_electrons_from_watermarks(
            trap_manager_2.watermark_volumes, trap_manager_2.watermark_fills);
        REQUIRE(
            n_trapped_electrons ==
            (0.5 * 0.8 + 0.2 * 0.4 + 0.1 * 0.2) * trap_1.density +
                (0.5 * 0.3 + 0.2 * 0.2 + 0.1 * 0.1) * trap_2.density);

        // Instant-capture traps
        TrapManagerInstantCapture trap_manager_ic(std::valarray<Trap>{trap_3}, 3, ccd);
        trap_manager_ic.initialise_trap_states();
        trap_manager_ic.n_active_watermarks = 3;
        trap_manager_ic.watermark_volumes = {0.5, 0.2, 0.1, 0.0, 0.0};
        trap_manager_ic.watermark_fills = {0.8, 0.4, 0.2, 0.0, 0.0};
        n_trapped_electrons = trap_manager_ic.n_trapped_electrons_from_watermarks(
            trap_manager_ic.watermark_volumes, trap_manager_2.watermark_fills);
        REQUIRE(
            n_trapped_electrons ==
            (0.5 * 0.8 + 0.2 * 0.4 + 0.1 * 0.2) * trap_3.density);
    }

    SECTION("Fill probabilities") {
        // Standard traps
        //## todo, including non-zero capture times

        // Instant-capture traps
        TrapManagerInstantCapture trap_manager_ic(
            std::valarray<Trap>{trap_3, trap_4}, 3, ccd);

        trap_manager_ic.set_fill_probabilities_from_dwell_time(1.0);

        REQUIRE(trap_manager_ic.fill_probabilities_from_empty[0] == Approx(1.0));
        REQUIRE(trap_manager_ic.fill_probabilities_from_full[0] == Approx(0.5));
        REQUIRE(trap_manager_ic.fill_probabilities_from_release[0] == Approx(0.5));
        REQUIRE(trap_manager_ic.empty_probabilities_from_release[0] == Approx(0.5));

        REQUIRE(trap_manager_ic.fill_probabilities_from_empty[1] == Approx(1.0));
        REQUIRE(trap_manager_ic.fill_probabilities_from_full[1] == Approx(0.2));
        REQUIRE(trap_manager_ic.fill_probabilities_from_release[1] == Approx(0.2));
        REQUIRE(trap_manager_ic.empty_probabilities_from_release[1] == Approx(0.8));

        trap_manager_ic.set_fill_probabilities_from_dwell_time(2.0);

        REQUIRE(trap_manager_ic.fill_probabilities_from_empty[0] == Approx(1.0));
        REQUIRE(trap_manager_ic.fill_probabilities_from_full[0] == Approx(0.25));
        REQUIRE(trap_manager_ic.fill_probabilities_from_release[0] == Approx(0.25));
        REQUIRE(trap_manager_ic.empty_probabilities_from_release[0] == Approx(0.75));

        REQUIRE(trap_manager_ic.fill_probabilities_from_empty[1] == Approx(1.0));
        REQUIRE(trap_manager_ic.fill_probabilities_from_full[1] == Approx(0.04));
        REQUIRE(trap_manager_ic.fill_probabilities_from_release[1] == Approx(0.04));
        REQUIRE(trap_manager_ic.empty_probabilities_from_release[1] == Approx(0.96));
    }

    SECTION("Watermark index above cloud") {
        TrapManagerInstantCapture trap_manager(
            std::valarray<Trap>{trap_1, trap_2}, 4, ccd);
        int i_wmk_above_cloud;

        // First watermark
        trap_manager.initialise_trap_states();
        trap_manager.n_active_watermarks = 0;
        trap_manager.i_first_active_wmk = 0;
        i_wmk_above_cloud =
            trap_manager.watermark_index_above_cloud_from_volumes(
                trap_manager.watermark_volumes, 0.6);
        REQUIRE(i_wmk_above_cloud == 0);

        // Cloud below watermarks
        trap_manager.initialise_trap_states();
        trap_manager.n_active_watermarks = 3;
        trap_manager.i_first_active_wmk = 0;
        trap_manager.watermark_volumes = {0.5, 0.2, 0.1, 0.0, 0.0};
        i_wmk_above_cloud =
            trap_manager.watermark_index_above_cloud_from_volumes(
                trap_manager.watermark_volumes, 0.3);
        REQUIRE(i_wmk_above_cloud == 0);

        // Cloud above watermarks
        trap_manager.initialise_trap_states();
        trap_manager.n_active_watermarks = 3;
        trap_manager.i_first_active_wmk = 0;
        trap_manager.watermark_volumes = {0.5, 0.2, 0.1, 0.0, 0.0};
        i_wmk_above_cloud =
            trap_manager.watermark_index_above_cloud_from_volumes(
                trap_manager.watermark_volumes, 0.9);
        REQUIRE(i_wmk_above_cloud == 3);

        // Cloud between watermarks
        trap_manager.initialise_trap_states();
        trap_manager.n_active_watermarks = 3;
        trap_manager.i_first_active_wmk = 0;
        trap_manager.watermark_volumes = {0.5, 0.2, 0.1, 0.0, 0.0};
        i_wmk_above_cloud =
            trap_manager.watermark_index_above_cloud_from_volumes(
                trap_manager.watermark_volumes, 0.6);
        REQUIRE(i_wmk_above_cloud == 1);

        // Cloud between watermarks, i_first_active_wmk > 0
        trap_manager.initialise_trap_states();
        trap_manager.n_active_watermarks = 3;
        trap_manager.i_first_active_wmk = 2;
        trap_manager.watermark_volumes = {0.2, 0.1, 0.5, 0.2, 0.0};
        i_wmk_above_cloud =
            trap_manager.watermark_index_above_cloud_from_volumes(
                trap_manager.watermark_volumes, 0.6);
        REQUIRE(i_wmk_above_cloud == 3);
    }

    SECTION("Store, reset, and restore trap states") {
        TrapManagerInstantCapture trap_manager(
            std::valarray<Trap>{trap_1, trap_2}, 5, ccd);
        trap_manager.initialise_trap_states();
        trap_manager.n_active_watermarks = 3;
        trap_manager.i_first_active_wmk = 1;
        std::valarray<double> volumes = {0.3, 0.5, 0.2, 0.1, 0.0, 0.0};
        std::valarray<double> fills = {
            // clang-format off
            0.4, 0.2,
            0.8, 0.3,
            0.4, 0.2,
            0.2, 0.1,
            0.0, 0.0,
            0.0, 0.0
            // clang-format on
        };
        trap_manager.watermark_volumes = volumes;
        trap_manager.watermark_fills = fills;
        std::vector<double> test, answer;

        // Store
        trap_manager.store_trap_states();

        REQUIRE(trap_manager.stored_n_active_watermarks == 3);
        REQUIRE(trap_manager.stored_i_first_active_wmk == 1);
        answer.assign(std::begin(volumes), std::end(volumes));
        test.assign(
            std::begin(trap_manager.stored_watermark_volumes),
            std::end(trap_manager.stored_watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer));
        answer.assign(std::begin(fills), std::end(fills));
        test.assign(
            std::begin(trap_manager.stored_watermark_fills),
            std::end(trap_manager.stored_watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer));

        // Reset
        trap_manager.reset_trap_states();

        REQUIRE(trap_manager.n_active_watermarks == 0);
        REQUIRE(trap_manager.i_first_active_wmk == 0);
        answer = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer));
        answer = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer));

        // Restore
        trap_manager.restore_trap_states();

        REQUIRE(trap_manager.n_active_watermarks == 3);
        REQUIRE(trap_manager.i_first_active_wmk == 1);
        answer.assign(std::begin(volumes), std::end(volumes));
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer));
        answer.assign(std::begin(fills), std::end(fills));
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer));
    }
}

TEST_CASE("Test instant-capture traps: release", "[trap_managers]") {
    TrapInstantCapture trap_1(10.0, -1.0 / log(0.5));
    TrapInstantCapture trap_2(8.0, -1.0 / log(0.2));

    double n_electrons_released;
    std::vector<double> test, answer;
    
    SECTION("Empty release") {
        TrapManagerInstantCapture trap_manager(
            std::valarray<Trap>{trap_1, trap_2}, 4, ccd);
        trap_manager.initialise_trap_states();
    
        n_electrons_released = trap_manager.n_electrons_released();
    
        REQUIRE(n_electrons_released == Approx(0.0));
        answer = {0.0, 0.0, 0.0, 0.0, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer));
        answer = {
            // clang-format off
            0.0, 0.0,
            0.0, 0.0,
            0.0, 0.0,
            0.0, 0.0,
            0.0, 0.0
            // clang-format on
        };
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer));
    }
    
    SECTION("Single trap release") {
        TrapManagerInstantCapture trap_manager(std::valarray<Trap>{trap_1}, 4, ccd);
        trap_manager.initialise_trap_states();
        trap_manager.set_fill_probabilities_from_dwell_time(1.0);
        trap_manager.n_active_watermarks = 3;
        trap_manager.watermark_volumes = {0.5, 0.2, 0.1, 0.0, 0.0};
        trap_manager.watermark_fills = {0.8, 0.4, 0.2, 0.0, 0.0};
        n_electrons_released = trap_manager.n_electrons_released();
    
        REQUIRE(n_electrons_released == Approx(2.5));
        answer = {0.5, 0.2, 0.1, 0.0, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer));
        answer = {0.4, 0.2, 0.1, 0.0, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer));
    }

    SECTION("Single trap release, longer dwell time") {
        TrapManagerInstantCapture trap_manager(std::valarray<Trap>{trap_1}, 4, ccd);
        trap_manager.initialise_trap_states();
        trap_manager.set_fill_probabilities_from_dwell_time(2.0);
        trap_manager.n_active_watermarks = 3;
        trap_manager.watermark_volumes = {0.5, 0.2, 0.1, 0.0, 0.0};
        trap_manager.watermark_fills = {0.8, 0.4, 0.2, 0.0, 0.0};
        n_electrons_released = trap_manager.n_electrons_released();

        REQUIRE(n_electrons_released == Approx(3.75));
        answer = {0.5, 0.2, 0.1, 0.0, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer));
        answer = {0.2, 0.1, 0.05, 0.0, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer));
    }
    
    SECTION("Single trap release, smaller timescale and smaller dwell time") {
        // Should be same end result
        TrapInstantCapture trap(10.0, -0.5 / log(0.5));
        TrapManagerInstantCapture trap_manager(std::valarray<Trap>{trap}, 4, ccd);
        trap_manager.initialise_trap_states();
        trap_manager.set_fill_probabilities_from_dwell_time(0.5);
        trap_manager.n_active_watermarks = 3;
        trap_manager.watermark_volumes = {0.5, 0.2, 0.1, 0.0, 0.0};
        trap_manager.watermark_fills = {0.8, 0.4, 0.2, 0.0, 0.0};
        n_electrons_released = trap_manager.n_electrons_released();
    
        REQUIRE(n_electrons_released == Approx(2.5));
        answer = {0.5, 0.2, 0.1, 0.0, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer));
        answer = {0.4, 0.2, 0.1, 0.0, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer));
    }
    
    SECTION("Multiple traps release") {
        TrapManagerInstantCapture trap_manager(
            std::valarray<Trap>{trap_1, trap_2}, 4, ccd);
        trap_manager.initialise_trap_states();
        trap_manager.set_fill_probabilities_from_dwell_time(1.0);
        trap_manager.n_active_watermarks = 3;
        trap_manager.watermark_volumes = {0.5, 0.2, 0.1, 0.0, 0.0};
        trap_manager.watermark_fills = {
            // clang-format off
            0.8, 0.3,
            0.4, 0.2,
            0.2, 0.1,
            0.0, 0.0,
            0.0, 0.0
            // clang-format on
        };
    
        n_electrons_released = trap_manager.n_electrons_released();
    
        REQUIRE(n_electrons_released == Approx(2.5 + 1.28));
        answer = {0.5, 0.2, 0.1, 0.0, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer));
        answer = {
            // clang-format off
            0.4, 0.06,
            0.2, 0.04,
            0.1, 0.02,
            0.0, 0.0,
            0.0, 0.0
            // clang-format on
        };
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer));
    }
    
    //### add non-zero i_first_active_wmk 
}

TEST_CASE("Test instant-capture traps: capture A", "[trap_managers]") {
    // Base cases: i_first_active_wmk = 0, enough > 1
    
    TrapInstantCapture trap_1(10.0, -1.0 / log(0.5));
    TrapInstantCapture trap_2(8.0, -1.0 / log(0.2));

    double n_electrons_captured;
    std::vector<double> test, answer;
    
    SECTION("Multiple traps capture, first watermark") {
        TrapManagerInstantCapture trap_manager(
            std::valarray<Trap>{trap_1, trap_2}, 5, ccd);
        trap_manager.initialise_trap_states();
        trap_manager.set_fill_probabilities_from_dwell_time(1.0);
        trap_manager.n_active_watermarks = 0;
        n_electrons_captured = trap_manager.n_electrons_captured(6e3);
    
        REQUIRE(n_electrons_captured == Approx(0.6 * 10.0 + 0.6 * 8.0));
        REQUIRE(trap_manager.n_active_watermarks == 1);
        REQUIRE(trap_manager.i_first_active_wmk == 0);
    
        answer = {0.6, 0.0, 0.0, 0.0, 0.0, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer));
        answer = {
            // clang-format off
            1.0, 1.0,
            0.0, 0.0,
            0.0, 0.0,
            0.0, 0.0,
            0.0, 0.0,
            0.0, 0.0
            // clang-format on
        };
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer));
    }
    
    SECTION("Multiple traps capture, cloud below watermarks") {
        TrapManagerInstantCapture trap_manager(
            std::valarray<Trap>{trap_1, trap_2}, 5, ccd);
        trap_manager.initialise_trap_states();
        trap_manager.set_fill_probabilities_from_dwell_time(1.0);
        trap_manager.n_active_watermarks = 3;
        trap_manager.watermark_volumes = {0.5, 0.2, 0.1, 0.0, 0.0, 0.0};
        trap_manager.watermark_fills = {
            // clang-format off
            0.8, 0.7,
            0.4, 0.3,
            0.3, 0.2,
            0.0, 0.0,
            0.0, 0.0,
            0.0, 0.0
            // clang-format on
        };
        n_electrons_captured = trap_manager.n_electrons_captured(3e3);
    
        REQUIRE(n_electrons_captured == Approx((0.3 * 0.2) * 10.0 + (0.3 * 0.3) * 8.0));
        REQUIRE(trap_manager.n_active_watermarks == 4);
        REQUIRE(trap_manager.i_first_active_wmk == 0);
    
        answer = {0.3, 0.2, 0.2, 0.1, 0.0, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer).margin(1e-99));
        answer = {
            // clang-format off
            1.0, 1.0,
            0.8, 0.7,
            0.4, 0.3,
            0.3, 0.2,
            0.0, 0.0,
            0.0, 0.0
            // clang-format on
        };
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer).margin(1e-99));
    }
    
    SECTION("Multiple traps capture, cloud above watermarks") {
        TrapManagerInstantCapture trap_manager(
            std::valarray<Trap>{trap_1, trap_2}, 5, ccd);
        trap_manager.initialise_trap_states();
        trap_manager.set_fill_probabilities_from_dwell_time(1.0);
        trap_manager.n_active_watermarks = 3;
        trap_manager.watermark_volumes = {0.5, 0.2, 0.1, 0.0, 0.0, 0.0};
        trap_manager.watermark_fills = {
            // clang-format off
            0.8, 0.7,
            0.4, 0.3,
            0.3, 0.2,
            0.0, 0.0,
            0.0, 0.0,
            0.0, 0.0
            // clang-format on
        };
    
        n_electrons_captured = trap_manager.n_electrons_captured(9e3);
    
        REQUIRE(
            n_electrons_captured ==
            Approx(
                (0.5 * 0.2 + 0.2 * 0.6 + 0.1 * 0.7 + 0.1 * 1.0) * 10.0 +
                (0.5 * 0.3 + 0.2 * 0.7 + 0.1 * 0.8 + 0.1 * 1.0) * 8.0));
        REQUIRE(trap_manager.n_active_watermarks == 1);
        REQUIRE(trap_manager.i_first_active_wmk == 2);
    
        answer = {0.5, 0.2, 0.9, 0.0, 0.0, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer));
        answer = {
            // clang-format off
            0.8, 0.7,
            0.4, 0.3,
            1.0, 1.0,
            0.0, 0.0,
            0.0, 0.0,
            0.0, 0.0
            // clang-format on
        };
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer));
    }
    
    SECTION("Single traps capture, cloud between watermarks") {
        TrapManagerInstantCapture trap_manager(std::valarray<Trap>{trap_1}, 5, ccd);
        trap_manager.initialise_trap_states();
        trap_manager.set_fill_probabilities_from_dwell_time(1.0);
        trap_manager.n_active_watermarks = 3;
        trap_manager.watermark_volumes = {0.5, 0.2, 0.1, 0.0, 0.0, 0.0};
        trap_manager.watermark_fills = {0.8, 0.4, 0.3, 0.0, 0.0, 0.0};
        n_electrons_captured = trap_manager.n_electrons_captured(6e3);
    
        REQUIRE(n_electrons_captured == Approx((0.5 * 0.2 + 0.1 * 0.6) * 10.0));
        REQUIRE(trap_manager.n_active_watermarks == 3);
        REQUIRE(trap_manager.i_first_active_wmk == 0);
    
        answer = {0.6, 0.1, 0.1, 0.0, 0.0, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer));
        answer = {1.0, 0.4, 0.3, 0.0, 0.0, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer));
    }
    
    SECTION("Multiple traps capture, cloud between watermarks") {
        TrapManagerInstantCapture trap_manager(
            std::valarray<Trap>{trap_1, trap_2}, 5, ccd);
        trap_manager.initialise_trap_states();
        trap_manager.set_fill_probabilities_from_dwell_time(1.0);
        trap_manager.n_active_watermarks = 3;
        trap_manager.watermark_volumes = {0.5, 0.2, 0.1, 0.0, 0.0, 0.0};
        trap_manager.watermark_fills = {
            // clang-format off
            0.8, 0.7,
            0.4, 0.3,
            0.3, 0.2,
            0.0, 0.0,
            0.0, 0.0,
            0.0, 0.0
            // clang-format on
        };
        n_electrons_captured = trap_manager.n_electrons_captured(6e3);
    
        REQUIRE(
            n_electrons_captured ==
            Approx((0.5 * 0.2 + 0.1 * 0.6) * 10.0 + (0.5 * 0.3 + 0.1 * 0.7) * 8.0));
        REQUIRE(trap_manager.n_active_watermarks == 3);
        REQUIRE(trap_manager.i_first_active_wmk == 0);
    
        answer = {0.6, 0.1, 0.1, 0.0, 0.0, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer));
        answer = {
            // clang-format off
            1.0, 1.0,
            0.4, 0.3,
            0.3, 0.2,
            0.0, 0.0,
            0.0, 0.0,
            0.0, 0.0
            // clang-format on
        };
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer));
    }
    
    SECTION("Multiple traps capture, cloud between watermarks 2") {
        TrapManagerInstantCapture trap_manager(
            std::valarray<Trap>{trap_1, trap_2}, 5, ccd);
        trap_manager.initialise_trap_states();
        trap_manager.set_fill_probabilities_from_dwell_time(1.0);
        trap_manager.n_active_watermarks = 4;
        trap_manager.watermark_volumes = {0.5, 0.2, 0.1, 0.1, 0.0, 0.0};
        trap_manager.watermark_fills = {
            // clang-format off
            0.8, 0.7,
            0.4, 0.3,
            0.3, 0.2,
            0.2, 0.1,
            0.0, 0.0,
            0.0, 0.0
            // clang-format on
        };
        n_electrons_captured = trap_manager.n_electrons_captured(7.5e3);
    
        REQUIRE(
            n_electrons_captured == Approx(
                                        (0.5 * 0.2 + 0.2 * 0.6 + 0.05 * 0.7) * 10.0 +
                                        (0.5 * 0.3 + 0.2 * 0.7 + 0.05 * 0.8) * 8.0));
        REQUIRE(trap_manager.n_active_watermarks == 3);
        REQUIRE(trap_manager.i_first_active_wmk == 1);
    
        answer = {0.5, 0.75, 0.05, 0.1, 0.0, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer));
        answer = {
            // clang-format off
            0.8, 0.7,
            1.0, 1.0,
            0.3, 0.2,
            0.2, 0.1,
            0.0, 0.0,
            0.0, 0.0
            // clang-format on
        };
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer));
    }

    SECTION("Multiple traps capture, cloud between watermarks 3") {
        TrapManagerInstantCapture trap_manager(
            std::valarray<Trap>{trap_1, trap_2}, 5, ccd);
        trap_manager.initialise_trap_states();
        trap_manager.set_fill_probabilities_from_dwell_time(1.0);
        trap_manager.n_active_watermarks = 4;
        trap_manager.watermark_volumes = {0.5, 0.2, 0.1, 0.1, 0.0, 0.0};
        trap_manager.watermark_fills = {
            // clang-format off
            0.8, 0.7,
            0.4, 0.3,
            0.3, 0.2,
            0.2, 0.1,
            0.0, 0.0,
            0.0, 0.0
            // clang-format on
        };
        n_electrons_captured = trap_manager.n_electrons_captured(8.5e3);
    
        REQUIRE(
            n_electrons_captured ==
            Approx(
                (0.5 * 0.2 + 0.2 * 0.6 + 0.1 * 0.7 + 0.05 * 0.8) * 10.0 +
                (0.5 * 0.3 + 0.2 * 0.7 + 0.1 * 0.8 + 0.05 * 0.9) * 8.0));
        REQUIRE(trap_manager.n_active_watermarks == 2);
        REQUIRE(trap_manager.i_first_active_wmk == 2);
    
        answer = {0.5, 0.2, 0.85, 0.05, 0.0, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer));
        answer = {
            // clang-format off
            0.8, 0.7,
            0.4, 0.3,
            1.0, 1.0,
            0.2, 0.1,
            0.0, 0.0,
            0.0, 0.0
            // clang-format on
        };
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer));
    }
    
    SECTION("Same cloud volume as existing watermark") {
        TrapManagerInstantCapture trap_manager(
            std::valarray<Trap>{trap_1, trap_2}, 5, ccd);
        trap_manager.initialise_trap_states();
        trap_manager.set_fill_probabilities_from_dwell_time(1.0);
        trap_manager.n_active_watermarks = 4;
        trap_manager.watermark_volumes = {0.5, 0.2, 0.1, 0.1, 0.0, 0.0};
        trap_manager.watermark_fills = {
            // clang-format off
            0.8, 0.7,
            0.4, 0.3,
            0.3, 0.2,
            0.2, 0.1,
            0.0, 0.0,
            0.0, 0.0
            // clang-format on
        };
        n_electrons_captured = trap_manager.n_electrons_captured(7e3);
    
        REQUIRE(
            n_electrons_captured ==
            Approx((0.5 * 0.2 + 0.2 * 0.6) * 10.0 + (0.5 * 0.3 + 0.2 * 0.7) * 8.0));
        REQUIRE(trap_manager.n_active_watermarks == 3);
        REQUIRE(trap_manager.i_first_active_wmk == 1);
    
        answer = {0.5, 0.7, 0.1, 0.1, 0.0, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer));
        answer = {
            // clang-format off
            0.8, 0.7,
            1.0, 1.0,
            0.3, 0.2,
            0.2, 0.1,
            0.0, 0.0,
            0.0, 0.0
            // clang-format on
        };
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer));
    }
}

TEST_CASE("Test instant-capture traps: capture B", "[trap_managers]") {
    // More complicated cases: i_first_active_wmk != 0 and/or enough < 1

    TrapInstantCapture trap_1(10.0, -1.0 / log(0.5));
    TrapInstantCapture trap_2(8.0, -1.0 / log(0.2));

    double n_electrons_captured;
    std::vector<double> test, answer;
    
    SECTION("Multiple traps capture, cloud below watermarks") {
        TrapManagerInstantCapture trap_manager(
            std::valarray<Trap>{trap_1, trap_2}, 5, ccd);
        trap_manager.initialise_trap_states();
        trap_manager.set_fill_probabilities_from_dwell_time(1.0);
        trap_manager.n_active_watermarks = 3;
        trap_manager.i_first_active_wmk = 2;
        trap_manager.watermark_volumes = {0.3, 0.2, 0.5, 0.2, 0.1, 0.0};
        trap_manager.watermark_fills = {
            // clang-format off
            0.8, 0.7,
            0.4, 0.3,
            0.8, 0.7,
            0.4, 0.3,
            0.3, 0.2,
            0.0, 0.0
            // clang-format on
        };
        n_electrons_captured = trap_manager.n_electrons_captured(3e3);
    
        REQUIRE(n_electrons_captured == Approx((0.3 * 0.2) * 10.0 + (0.3 * 0.3) * 8.0));
        REQUIRE(trap_manager.n_active_watermarks == 4);
        REQUIRE(trap_manager.i_first_active_wmk == 2);
    
        answer = {0.3, 0.2, 0.3, 0.2, 0.2, 0.1};
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer).margin(1e-99));
        answer = {
            // clang-format off
            0.8, 0.7,
            0.4, 0.3,
            1.0, 1.0,
            0.8, 0.7,
            0.4, 0.3,
            0.3, 0.2
            // clang-format on
        };
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer).margin(1e-99));
    }

    SECTION("Multiple traps capture, cloud between watermarks") {
        TrapManagerInstantCapture trap_manager(
            std::valarray<Trap>{trap_1, trap_2}, 5, ccd);
        trap_manager.initialise_trap_states();
        trap_manager.set_fill_probabilities_from_dwell_time(1.0);
        trap_manager.n_active_watermarks = 4;
        trap_manager.i_first_active_wmk = 2;
        trap_manager.watermark_volumes = {0.3, 0.2, 0.5, 0.2, 0.1, 0.1};
        trap_manager.watermark_fills = {
            // clang-format off
            0.8, 0.7,
            0.4, 0.3,
            0.8, 0.7,
            0.4, 0.3,
            0.3, 0.2,
            0.2, 0.1
            // clang-format on
        };
        n_electrons_captured = trap_manager.n_electrons_captured(7.5e3);
    
        REQUIRE(
            n_electrons_captured == Approx(
                                        (0.5 * 0.2 + 0.2 * 0.6 + 0.05 * 0.7) * 10.0 +
                                        (0.5 * 0.3 + 0.2 * 0.7 + 0.05 * 0.8) * 8.0));
        REQUIRE(trap_manager.n_active_watermarks == 3);
        REQUIRE(trap_manager.i_first_active_wmk == 3);
    
        answer = {0.3, 0.2, 0.5, 0.75, 0.05, 0.1};
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer));
        answer = {
            // clang-format off
            0.8, 0.7,
            0.4, 0.3,
            0.8, 0.7,
            1.0, 1.0,
            0.3, 0.2,
            0.2, 0.1
            // clang-format on
        };
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer));
    }

    /* //###
    CCD ccd_2(1e4, 1e-7, 0.5);

    SECTION("Not-enough capture, first watermark") {
        TrapManagerInstantCapture trap_manager(std::valarray<Trap>{trap_1}, 5, ccd_2);
        trap_manager.initialise_trap_states();
        trap_manager.set_fill_probabilities_from_dwell_time(1.0);
        trap_manager.n_active_watermarks = 0;
        n_electrons_captured = trap_manager.n_electrons_captured(2.5e-3);
        // --> cloud_fractional_volume = 4.9999e-4, enough = 0.50001

        REQUIRE(n_electrons_captured == Approx(4.9999e-4 * 0.50001 * 10.0));
        REQUIRE(trap_manager.n_active_watermarks == 1);

        answer = {4.9999e-4, 0.0, 0.0, 0.0, 0.0, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer));
        answer = {0.50001, 0.0, 0.0, 0.0, 0.0, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer));
    }

    SECTION("Not-enough capture, cloud below watermarks") {
        TrapManagerInstantCapture trap_manager(
            std::valarray<Trap>{trap_1, trap_2}, 5, ccd_2);
        trap_manager.initialise_trap_states();
        trap_manager.set_fill_probabilities_from_dwell_time(1.0);
        trap_manager.n_active_watermarks = 3;
        trap_manager.watermark_volumes = {0.5, 0.2, 0.1, 0.0, 0.0, 0.0};
        trap_manager.watermark_fills = {
            // clang-format off
            0.5, 0.25,
            0.4, 0.3,
            0.3, 0.2,
            0.0, 0.0,
            0.0, 0.0,
            0.0, 0.0
            // clang-format on
        };
        n_electrons_captured = trap_manager.n_electrons_captured(2.5e-3);
        // --> cloud_fractional_volume = 4.9999e-4, enough = 0.454555
        double enough = 0.454555;

        REQUIRE(
            n_electrons_captured ==
            Approx(
                (4.9999e-4 * enough * 0.5) * 10.0 + (4.9999e-4 * enough * 0.75) * 8.0));
        REQUIRE(trap_manager.n_active_watermarks == 4);

        answer = {4.9999e-4, 0.5 - 4.9999e-4, 0.2, 0.1, 0.0, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer).margin(1e-99));
        answer = {
            // clang-format off
            0.5 + enough * (1.0 - 0.5), 0.25 + enough * (1.0 - 0.25),
            0.5, 0.25,
            0.4, 0.3,
            0.3, 0.2,
            0.0, 0.0,
            0.0, 0.0
            // clang-format on
        };
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer).margin(1e-99));
    }

    SECTION("Not-enough capture, cloud above watermarks") {
        TrapManagerInstantCapture trap_manager(
            std::valarray<Trap>{trap_1, trap_2}, 5, ccd_2);
        trap_manager.initialise_trap_states();
        trap_manager.set_fill_probabilities_from_dwell_time(1.0);
        trap_manager.n_active_watermarks = 2;
        trap_manager.watermark_volumes = {2e-4, 1e-4, 0.0, 0.0, 0.0, 0.0};
        trap_manager.watermark_fills = {
            // clang-format off
            0.8, 0.7,
            0.4, 0.3,
            0.0, 0.0,
            0.0, 0.0,
            0.0, 0.0,
            0.0, 0.0
            // clang-format on
        };
        n_electrons_captured = trap_manager.n_electrons_captured(2.5e-3);
        // --> cloud_fractional_volume = 4.9999e-4, enough = 0.443277
        double enough = 0.443277;

        REQUIRE(
            n_electrons_captured ==
            Approx(
                (2e-4 * enough * 0.2) * 10.0 + (2e-4 * enough * 0.3) * 8.0 +
                (1e-4 * enough * 0.6) * 10.0 + (1e-4 * enough * 0.7) * 8.0 +
                (4.9999e-4 * enough) * 10.0 + (4.9999e-4 * enough) * 8.0));
        REQUIRE(trap_manager.n_active_watermarks == 3);

        answer = {2e-4, 1e-4, 4.9999e-4, 0.0, 0.0, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer).margin(1e-99));
        answer = {
            // clang-format off
            0.8 + enough * (1.0 - 0.8), 0.7 + enough * (1.0 - 0.7),
            0.4 + enough * (1.0 - 0.4), 0.3 + enough * (1.0 - 0.3),
            enough, enough,
            0.0, 0.0,
            0.0, 0.0,
            0.0, 0.0
            // clang-format on
        };
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer).margin(1e-99));
    }

    SECTION("Not-enough capture, cloud between watermarks") {
        TrapManagerInstantCapture trap_manager(
            std::valarray<Trap>{trap_1, trap_2}, 5, ccd_2);
        trap_manager.initialise_trap_states();
        trap_manager.set_fill_probabilities_from_dwell_time(1.0);
        trap_manager.n_active_watermarks = 4;
        trap_manager.watermark_volumes = {2e-4, 1e-4, 0.2, 0.1, 0.0, 0.0};
        trap_manager.watermark_fills = {
            // clang-format off
            0.8, 0.7,
            0.4, 0.3,
            0.3, 0.2,
            0.2, 0.1,
            0.0, 0.0,
            0.0, 0.0
            // clang-format on
        };
        n_electrons_captured = trap_manager.n_electrons_captured(2.5e-3);
        // --> cloud_fractional_volume = 4.9999e-4, enough = 0.529676
        double enough = 0.529676;

        REQUIRE(
            n_electrons_captured ==
            Approx(
                (2e-4 * enough * 0.2) * 10.0 + (2e-4 * enough * 0.3) * 8.0 +
                (1e-4 * enough * 0.6) * 10.0 + (1e-4 * enough * 0.7) * 8.0 +
                (1.9999e-4 * enough * 0.7) * 10.0 + (1.9999e-4 * enough * 0.8) * 8.0));
        REQUIRE(trap_manager.n_active_watermarks == 5);

        answer = {2e-4, 1e-4, 1.9999e-4, 0.2 - 1.9999e-4, 0.1, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer).margin(1e-99));
        answer = {
            // clang-format off
            0.8 + enough * (1.0 - 0.8), 0.7 + enough * (1.0 - 0.7),
            0.4 + enough * (1.0 - 0.4), 0.3 + enough * (1.0 - 0.3),
            0.3 + enough * (1.0 - 0.3), 0.2 + enough * (1.0 - 0.2),
            0.3, 0.2,
            0.2, 0.1,
            0.0, 0.0
            // clang-format on
        };
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer).margin(1e-99));
    }
    */
}

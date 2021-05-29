
#include <stdio.h>
#include <valarray>
#include <vector>

#include "catch2/catch.hpp"
#include "ccd.hpp"
#include "roe.hpp"
#include "trap_managers.hpp"
#include "traps.hpp"
#include "util.hpp"

CCDPhase ccd_phase(1e4, 0.0, 1.0);

TEST_CASE("Test initialisation", "[trap_managers]") {
    TrapSlowCapture trap_1(1.0, 1.0, 0.0);
    TrapSlowCapture trap_2(2.0, 2.0, 0.0);
    TrapInstantCapture trap_3(3.0, 3.0);
    int max_n_transfers = 123;

    SECTION("Traps") {
        // Slow capture traps
        std::valarray<TrapSlowCapture> traps{trap_1, trap_2};
        TrapManagerSlowCapture trap_manager_sc(traps, max_n_transfers, ccd_phase);

        REQUIRE(trap_manager_sc.n_traps == 2);
        REQUIRE(trap_manager_sc.traps[0].density == trap_1.density);
        REQUIRE(trap_manager_sc.traps[1].density == trap_2.density);
        REQUIRE(trap_manager_sc.trap_densities[0] == trap_1.density);
        REQUIRE(trap_manager_sc.trap_densities[1] == trap_2.density);

        // Instant-capture traps
        std::valarray<TrapInstantCapture> traps_ic{trap_3};
        TrapManagerInstantCapture trap_manager_ic(traps_ic, max_n_transfers, ccd_phase);

        REQUIRE(trap_manager_ic.n_traps == 1);
        REQUIRE(trap_manager_ic.traps[0].density == trap_3.density);
        REQUIRE(trap_manager_ic.trap_densities[0] == trap_3.density);
    }

    SECTION("CCD Phase") {
        // Slow capture traps
        std::valarray<TrapSlowCapture> traps{trap_1, trap_2};
        TrapManagerSlowCapture trap_manager_sc(traps, max_n_transfers, ccd_phase);

        REQUIRE(trap_manager_sc.ccd_phase.full_well_depth == ccd_phase.full_well_depth);
        REQUIRE(
            trap_manager_sc.ccd_phase.well_notch_depth == ccd_phase.well_notch_depth);
        REQUIRE(trap_manager_sc.ccd_phase.well_fill_power == ccd_phase.well_fill_power);

        // Instant-capture traps
        std::valarray<TrapInstantCapture> traps_ic{trap_3};
        TrapManagerInstantCapture trap_manager_ic(traps_ic, max_n_transfers, ccd_phase);

        REQUIRE(trap_manager_ic.ccd_phase.full_well_depth == ccd_phase.full_well_depth);
        REQUIRE(
            trap_manager_ic.ccd_phase.well_notch_depth == ccd_phase.well_notch_depth);
        REQUIRE(trap_manager_ic.ccd_phase.well_fill_power == ccd_phase.well_fill_power);
    }

    SECTION("Misc attributes") {
        // Slow capture traps
        TrapManagerSlowCapture trap_manager_sc(
            std::valarray<TrapSlowCapture>{trap_1, trap_2}, max_n_transfers, ccd_phase);

        REQUIRE(trap_manager_sc.max_n_transfers == 123);
        REQUIRE(trap_manager_sc.n_watermarks_per_transfer == 2);
        REQUIRE(trap_manager_sc.empty_watermark == 0.0);
        REQUIRE(trap_manager_sc.n_active_watermarks == 0);
        REQUIRE(trap_manager_sc.i_first_active_wmk == 0);

        // Instant-capture traps
        TrapManagerInstantCapture trap_manager_ic(
            std::valarray<TrapInstantCapture>{trap_3}, max_n_transfers, ccd_phase);

        REQUIRE(trap_manager_ic.max_n_transfers == 123);
        REQUIRE(trap_manager_ic.n_watermarks_per_transfer == 1);
        REQUIRE(trap_manager_ic.empty_watermark == 0.0);
        REQUIRE(trap_manager_ic.n_active_watermarks == 0);
        REQUIRE(trap_manager_ic.i_first_active_wmk == 0);
    }

    SECTION("Initial watermarks") {
        // Slow capture traps
        TrapManagerSlowCapture trap_manager_sc(
            std::valarray<TrapSlowCapture>{trap_1, trap_2}, 3, ccd_phase);
        trap_manager_sc.initialise_trap_states();

        REQUIRE(trap_manager_sc.n_watermarks == 7);
        REQUIRE(trap_manager_sc.watermark_volumes.size() == 7);
        REQUIRE(trap_manager_sc.watermark_fills.size() == 7 * 2);
        REQUIRE(
            trap_manager_sc.watermark_volumes.sum() ==
            trap_manager_sc.n_watermarks * trap_manager_sc.empty_watermark);
        REQUIRE(
            trap_manager_sc.watermark_fills.sum() ==
            trap_manager_sc.n_watermarks * trap_manager_sc.n_traps *
                trap_manager_sc.empty_watermark);

        TrapManagerSlowCapture trap_manager_2(
            std::valarray<TrapSlowCapture>{trap_1}, max_n_transfers, ccd_phase);
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
        TrapManagerInstantCapture trap_manager_ic(
            std::valarray<TrapInstantCapture>{trap_3}, 3, ccd_phase);
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
    TrapSlowCapture trap_1(10.0, -1.0 / log(0.5), 0.0);
    TrapSlowCapture trap_2(8.0, -1.0 / log(0.2), 0.0);
    TrapInstantCapture trap_3(10.0, -1.0 / log(0.5));
    TrapInstantCapture trap_4(8.0, -1.0 / log(0.2));

    SECTION("Number of trapped electrons") {
        double n_trapped_electrons;

        // Slow capture traps
        TrapManagerSlowCapture trap_manager_sc(
            std::valarray<TrapSlowCapture>{trap_1}, 2, ccd_phase);
        trap_manager_sc.initialise_trap_states();
        trap_manager_sc.n_active_watermarks = 3;
        trap_manager_sc.watermark_volumes = {0.5, 0.2, 0.1, 0.0, 0.0};
        trap_manager_sc.watermark_fills = {0.8 * 10, 0.4 * 10, 0.2 * 10, 0.0, 0.0};
        n_trapped_electrons = trap_manager_sc.n_trapped_electrons_from_watermarks(
            trap_manager_sc.watermark_volumes, trap_manager_sc.watermark_fills);
        REQUIRE(
            n_trapped_electrons ==
            Approx((0.5 * 0.8 + 0.2 * 0.4 + 0.1 * 0.2) * trap_1.density));

        TrapManagerSlowCapture trap_manager_2(
            std::valarray<TrapSlowCapture>{trap_1, trap_2}, 2, ccd_phase);
        trap_manager_2.initialise_trap_states();
        trap_manager_2.n_active_watermarks = 3;
        trap_manager_2.watermark_volumes = {0.5, 0.2, 0.1, 0.0, 0.0};
        trap_manager_2.watermark_fills = {
            // clang-format off
            0.8 * 10, 0.3 * 8, 
            0.4 * 10, 0.2 * 8,
            0.2 * 10, 0.1 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            // clang-format on
        };
        n_trapped_electrons = trap_manager_2.n_trapped_electrons_from_watermarks(
            trap_manager_2.watermark_volumes, trap_manager_2.watermark_fills);
        REQUIRE(
            n_trapped_electrons ==
            Approx(
                (0.5 * 0.8 + 0.2 * 0.4 + 0.1 * 0.2) * trap_1.density +
                (0.5 * 0.3 + 0.2 * 0.2 + 0.1 * 0.1) * trap_2.density));

        // i_first_active_wmk > 0
        trap_manager_2.initialise_trap_states();
        trap_manager_2.n_active_watermarks = 3;
        trap_manager_2.i_first_active_wmk = 2;
        trap_manager_2.watermark_volumes = {0.4, 0.3, 0.5, 0.2, 0.1};
        trap_manager_2.watermark_fills = {
            // clang-format off
            0.8 * 10, 0.3 * 8,
            0.4 * 10, 0.2 * 8,
            0.8 * 10, 0.3 * 8,
            0.4 * 10, 0.2 * 8,
            0.2 * 10, 0.1 * 8,
            // clang-format on
        };
        n_trapped_electrons = trap_manager_2.n_trapped_electrons_from_watermarks(
            trap_manager_2.watermark_volumes, trap_manager_2.watermark_fills);
        REQUIRE(
            n_trapped_electrons ==
            Approx(
                (0.5 * 0.8 + 0.2 * 0.4 + 0.1 * 0.2) * trap_1.density +
                (0.5 * 0.3 + 0.2 * 0.2 + 0.1 * 0.1) * trap_2.density));

        // Instant-capture traps
        TrapManagerInstantCapture trap_manager_ic(
            std::valarray<TrapInstantCapture>{trap_3}, 3, ccd_phase);
        trap_manager_ic.initialise_trap_states();
        trap_manager_ic.n_active_watermarks = 3;
        trap_manager_ic.watermark_volumes = {0.5, 0.2, 0.1, 0.0, 0.0};
        trap_manager_ic.watermark_fills = {0.8 * 10, 0.4 * 10, 0.2 * 10, 0.0, 0.0};
        n_trapped_electrons = trap_manager_ic.n_trapped_electrons_from_watermarks(
            trap_manager_ic.watermark_volumes, trap_manager_ic.watermark_fills);
        REQUIRE(
            n_trapped_electrons ==
            Approx((0.5 * 0.8 + 0.2 * 0.4 + 0.1 * 0.2) * trap_3.density));
    }

    SECTION("Fill probabilities") {
        // Slow capture traps
        //## todo, including non-zero capture times

        // Instant-capture traps
        TrapManagerInstantCapture trap_manager_ic(
            std::valarray<TrapInstantCapture>{trap_3, trap_4}, 3, ccd_phase);

        trap_manager_ic.set_fill_probabilities_from_dwell_time(1.0);

        REQUIRE(trap_manager_ic.fill_probabilities_from_release[0] == Approx(0.5));
        REQUIRE(trap_manager_ic.empty_probabilities_from_release[0] == Approx(0.5));

        REQUIRE(trap_manager_ic.fill_probabilities_from_release[1] == Approx(0.2));
        REQUIRE(trap_manager_ic.empty_probabilities_from_release[1] == Approx(0.8));

        trap_manager_ic.set_fill_probabilities_from_dwell_time(2.0);

        REQUIRE(trap_manager_ic.fill_probabilities_from_release[0] == Approx(0.25));
        REQUIRE(trap_manager_ic.empty_probabilities_from_release[0] == Approx(0.75));

        REQUIRE(trap_manager_ic.fill_probabilities_from_release[1] == Approx(0.04));
        REQUIRE(trap_manager_ic.empty_probabilities_from_release[1] == Approx(0.96));
    }

    SECTION("Watermark index above cloud") {
        TrapManagerSlowCapture trap_manager(
            std::valarray<TrapSlowCapture>{trap_1, trap_2}, 4, ccd_phase);
        int i_wmk_above_cloud;

        // First watermark
        trap_manager.initialise_trap_states();
        trap_manager.n_active_watermarks = 0;
        trap_manager.i_first_active_wmk = 0;
        i_wmk_above_cloud = trap_manager.watermark_index_above_cloud(0.6);
        REQUIRE(i_wmk_above_cloud == 0);

        // Cloud below watermarks
        trap_manager.initialise_trap_states();
        trap_manager.n_active_watermarks = 3;
        trap_manager.i_first_active_wmk = 0;
        trap_manager.watermark_volumes = {0.5, 0.2, 0.1, 0.0, 0.0};
        i_wmk_above_cloud = trap_manager.watermark_index_above_cloud(0.3);
        REQUIRE(i_wmk_above_cloud == 0);

        // Cloud above watermarks
        trap_manager.initialise_trap_states();
        trap_manager.n_active_watermarks = 3;
        trap_manager.i_first_active_wmk = 0;
        trap_manager.watermark_volumes = {0.5, 0.2, 0.1, 0.0, 0.0};
        i_wmk_above_cloud = trap_manager.watermark_index_above_cloud(0.9);
        REQUIRE(i_wmk_above_cloud == 3);

        // Cloud between watermarks
        trap_manager.initialise_trap_states();
        trap_manager.n_active_watermarks = 3;
        trap_manager.i_first_active_wmk = 0;
        trap_manager.watermark_volumes = {0.5, 0.2, 0.1, 0.0, 0.0};
        i_wmk_above_cloud = trap_manager.watermark_index_above_cloud(0.6);
        REQUIRE(i_wmk_above_cloud == 1);

        // Cloud between watermarks, i_first_active_wmk > 0
        trap_manager.initialise_trap_states();
        trap_manager.n_active_watermarks = 3;
        trap_manager.i_first_active_wmk = 2;
        trap_manager.watermark_volumes = {0.2, 0.1, 0.5, 0.2, 0.0};
        i_wmk_above_cloud = trap_manager.watermark_index_above_cloud(0.6);
        REQUIRE(i_wmk_above_cloud == 3);
    }

    SECTION("Store, reset, and restore trap states") {
        TrapManagerSlowCapture trap_manager(
            std::valarray<TrapSlowCapture>{trap_1, trap_2}, 3, ccd_phase);
        trap_manager.initialise_trap_states();
        trap_manager.n_active_watermarks = 3;
        trap_manager.i_first_active_wmk = 1;
        std::valarray<double> volumes = {0.3, 0.5, 0.2, 0.1, 0.0, 0.0, 0.0};
        std::valarray<double> fills = {
            // clang-format off
            0.4, 0.2,
            0.8, 0.3,
            0.4, 0.2,
            0.2, 0.1,
            0.0, 0.0,
            0.0, 0.0,
            0.0, 0.0,
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
        answer = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer));
        answer = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
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

TEST_CASE("Test manager manager", "[trap_managers]") {
    TrapSlowCapture trap_1(1.0, 1.0, 0.0);
    TrapSlowCapture trap_2(2.0, 2.0, 0.0);
    TrapInstantCapture trap_3(3.0, 3.0);
    int max_n_transfers = 123;
    std::vector<double> test, answer;

    SECTION("Initialisation, single phase, one type of traps") {
        std::valarray<TrapSlowCapture> traps{trap_1, trap_2};
        std::valarray<TrapInstantCapture> traps_ic = {};
        ROE roe;
        CCD ccd(ccd_phase);

        TrapManagerManager trap_manager_manager(
            traps, traps_ic, max_n_transfers, ccd, roe.dwell_times);

        // TrapSlowCapture managers
        REQUIRE(trap_manager_manager.n_slow_capture_traps == 2);
        REQUIRE(trap_manager_manager.trap_managers_slow_capture.size() == 1);
        REQUIRE(trap_manager_manager.trap_managers_slow_capture[0].traps.size() == 2);

        REQUIRE(trap_manager_manager.n_instant_capture_traps == 0);
    }

    SECTION("Initialisation, single phase, two types of traps") {
        std::valarray<TrapSlowCapture> traps{trap_1, trap_2};
        std::valarray<TrapInstantCapture> traps_ic{trap_3};
        ROE roe;
        CCD ccd(ccd_phase);

        TrapManagerManager trap_manager_manager(
            traps, traps_ic, max_n_transfers, ccd, roe.dwell_times);

        // TrapSlowCapture managers
        REQUIRE(trap_manager_manager.n_slow_capture_traps == 2);
        REQUIRE(trap_manager_manager.trap_managers_slow_capture.size() == 1);
        REQUIRE(trap_manager_manager.trap_managers_slow_capture[0].traps.size() == 2);

        REQUIRE(trap_manager_manager.n_instant_capture_traps == 1);
        REQUIRE(trap_manager_manager.trap_managers_instant_capture.size() == 1);
        REQUIRE(
            trap_manager_manager.trap_managers_instant_capture[0].traps.size() == 1);
    }

    SECTION("Initialisation, multiphase, two types of traps") {
        std::valarray<TrapSlowCapture> traps{trap_1, trap_2};
        std::valarray<TrapInstantCapture> traps_ic{trap_3};
        std::valarray<double> dwell_times = {0.8, 0.1, 0.1};
        ROE roe(dwell_times);
        CCDPhase ccd_phase_2(2e4, 0.0, 0.8);
        std::valarray<CCDPhase> phases = {ccd_phase, ccd_phase_2, ccd_phase_2};
        std::valarray<double> fractions = {0.5, 0.25, 0.25};
        CCD ccd(phases, fractions);

        TrapManagerManager trap_manager_manager(
            traps, traps_ic, max_n_transfers, ccd, roe.dwell_times);

        // TrapSlowCapture managers
        REQUIRE(trap_manager_manager.n_slow_capture_traps == 2);
        REQUIRE(trap_manager_manager.trap_managers_slow_capture.size() == 3);
        REQUIRE(trap_manager_manager.trap_managers_slow_capture[0].traps.size() == 2);

        REQUIRE(trap_manager_manager.n_instant_capture_traps == 1);
        REQUIRE(trap_manager_manager.trap_managers_instant_capture.size() == 3);
        REQUIRE(
            trap_manager_manager.trap_managers_instant_capture[0].traps.size() == 1);

        // Initial watermarks, accounting for number of clock-sequence steps
        int n_levels = max_n_transfers * 3 * 2 + 1;
        REQUIRE(
            trap_manager_manager.trap_managers_slow_capture[0].n_watermarks ==
            n_levels);
        REQUIRE(
            trap_manager_manager.trap_managers_slow_capture[0]
                .watermark_volumes.size() == n_levels);
        REQUIRE(
            trap_manager_manager.trap_managers_slow_capture[0].watermark_fills.size() ==
            n_levels * 2);
        REQUIRE(
            trap_manager_manager.trap_managers_slow_capture[1].n_watermarks ==
            n_levels);
        REQUIRE(
            trap_manager_manager.trap_managers_slow_capture[1]
                .watermark_volumes.size() == n_levels);
        REQUIRE(
            trap_manager_manager.trap_managers_slow_capture[1].watermark_fills.size() ==
            n_levels * 2);

        n_levels = max_n_transfers * 3 + 1;
        REQUIRE(
            trap_manager_manager.trap_managers_instant_capture[0].n_watermarks ==
            n_levels);
        REQUIRE(
            trap_manager_manager.trap_managers_instant_capture[0]
                .watermark_volumes.size() == n_levels);
        REQUIRE(
            trap_manager_manager.trap_managers_instant_capture[0]
                .watermark_fills.size() == n_levels);

        // TrapSlowCapture densities modified by the CCD's fraction_of_traps_per_phase
        answer = {trap_1.density * 0.5, trap_2.density * 0.5};
        test.assign(
            std::begin(
                trap_manager_manager.trap_managers_slow_capture[0].trap_densities),
            std::end(
                trap_manager_manager.trap_managers_slow_capture[0].trap_densities));
        REQUIRE_THAT(test, Catch::Approx(answer));
        answer = {trap_1.density * 0.25, trap_2.density * 0.25};
        test.assign(
            std::begin(
                trap_manager_manager.trap_managers_slow_capture[1].trap_densities),
            std::end(
                trap_manager_manager.trap_managers_slow_capture[1].trap_densities));
        REQUIRE_THAT(test, Catch::Approx(answer));
        answer = {trap_1.density * 0.25, trap_2.density * 0.25};
        test.assign(
            std::begin(
                trap_manager_manager.trap_managers_slow_capture[2].trap_densities),
            std::end(
                trap_manager_manager.trap_managers_slow_capture[2].trap_densities));
        REQUIRE_THAT(test, Catch::Approx(answer));

        answer = {trap_3.density * 0.5};
        test.assign(
            std::begin(
                trap_manager_manager.trap_managers_instant_capture[0].trap_densities),
            std::end(
                trap_manager_manager.trap_managers_instant_capture[0].trap_densities));
        REQUIRE_THAT(test, Catch::Approx(answer));
        answer = {trap_3.density * 0.25};
        test.assign(
            std::begin(
                trap_manager_manager.trap_managers_instant_capture[1].trap_densities),
            std::end(
                trap_manager_manager.trap_managers_instant_capture[1].trap_densities));
        REQUIRE_THAT(test, Catch::Approx(answer));
        answer = {trap_3.density * 0.25};
        test.assign(
            std::begin(
                trap_manager_manager.trap_managers_instant_capture[2].trap_densities),
            std::end(
                trap_manager_manager.trap_managers_instant_capture[2].trap_densities));
        REQUIRE_THAT(test, Catch::Approx(answer));
    }

    SECTION("Store, reset, and restore all trap states") {
        std::valarray<TrapSlowCapture> traps{trap_1, trap_2};
        std::valarray<TrapInstantCapture> traps_ic{trap_3};
        std::valarray<double> dwell_times = {0.8, 0.1, 0.1};
        ROE roe(dwell_times);
        CCDPhase ccd_phase_2(2e4, 0.0, 0.8);
        std::valarray<CCDPhase> phases = {ccd_phase, ccd_phase_2, ccd_phase_2};
        std::valarray<double> fractions = {0.5, 0.25, 0.25};
        CCD ccd(phases, fractions);

        max_n_transfers = 4;
        TrapManagerManager t_m_m(
            traps, traps_ic, max_n_transfers, ccd, roe.dwell_times);

        // (Two possible levels per transfer, multiplied by three phases)
        std::valarray<double> volumes = {0.3, 0.5, 0.2, 0.1, 0.0, 0.0, 0.0, 0.0, 0.0,
                                         0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                                         0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
        std::valarray<double> fills = {
            // clang-format off
            0.4, 0.2,
            0.8, 0.3,
            0.4, 0.2,
            0.2, 0.1,
            0.0, 0.0,
            0.0, 0.0,
            0.0, 0.0,
            0.0, 0.0,
            0.0, 0.0,
            0.0, 0.0,
            0.0, 0.0,
            0.0, 0.0,
            0.0, 0.0,
            0.0, 0.0,
            0.0, 0.0,
            0.0, 0.0,
            0.0, 0.0,
            0.0, 0.0,
            0.0, 0.0,
            0.0, 0.0,
            0.0, 0.0,
            0.0, 0.0,
            0.0, 0.0,
            0.0, 0.0,
            0.0, 0.0,
            // clang-format on
        };
        std::valarray<double> volumes_ic = {0.3, 0.5, 0.2, 0.1, 0.0, 0.0, 0.0,
                                            0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
        std::valarray<double> fills_ic = {0.3, 0.7, 0.3, 0.1, 0.0, 0.0, 0.0,
                                          0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

        // Set watermarks, all phases and watermark types
        for (int phase_index = 0; phase_index < ccd.n_phases; phase_index++) {
            t_m_m.trap_managers_slow_capture[phase_index].n_active_watermarks = 3;
            t_m_m.trap_managers_slow_capture[phase_index].i_first_active_wmk = 1;
            t_m_m.trap_managers_slow_capture[phase_index].watermark_volumes = volumes;
            t_m_m.trap_managers_slow_capture[phase_index].watermark_fills = fills;

            t_m_m.trap_managers_instant_capture[phase_index].n_active_watermarks = 3;
            t_m_m.trap_managers_instant_capture[phase_index].i_first_active_wmk = 1;
            t_m_m.trap_managers_instant_capture[phase_index].watermark_volumes =
                volumes_ic;
            t_m_m.trap_managers_instant_capture[phase_index].watermark_fills = fills_ic;
        }

        // Store
        t_m_m.store_trap_states();

        for (int phase_index = 0; phase_index < ccd.n_phases; phase_index++) {
            REQUIRE(
                t_m_m.trap_managers_slow_capture[phase_index]
                    .stored_n_active_watermarks == 3);
            REQUIRE(
                t_m_m.trap_managers_slow_capture[phase_index]
                    .stored_i_first_active_wmk == 1);
            answer.assign(std::begin(volumes), std::end(volumes));
            test.assign(
                std::begin(t_m_m.trap_managers_slow_capture[phase_index]
                               .stored_watermark_volumes),
                std::end(t_m_m.trap_managers_slow_capture[phase_index]
                             .stored_watermark_volumes));
            REQUIRE_THAT(test, Catch::Approx(answer));
            answer.assign(std::begin(fills), std::end(fills));
            test.assign(
                std::begin(t_m_m.trap_managers_slow_capture[phase_index]
                               .stored_watermark_fills),
                std::end(t_m_m.trap_managers_slow_capture[phase_index]
                             .stored_watermark_fills));
            REQUIRE_THAT(test, Catch::Approx(answer));

            REQUIRE(
                t_m_m.trap_managers_instant_capture[phase_index]
                    .stored_n_active_watermarks == 3);
            REQUIRE(
                t_m_m.trap_managers_instant_capture[phase_index]
                    .stored_i_first_active_wmk == 1);
            answer.assign(std::begin(volumes_ic), std::end(volumes_ic));
            test.assign(
                std::begin(t_m_m.trap_managers_instant_capture[phase_index]
                               .stored_watermark_volumes),
                std::end(t_m_m.trap_managers_instant_capture[phase_index]
                             .stored_watermark_volumes));
            REQUIRE_THAT(test, Catch::Approx(answer));
            answer.assign(std::begin(fills_ic), std::end(fills_ic));
            test.assign(
                std::begin(t_m_m.trap_managers_instant_capture[phase_index]
                               .stored_watermark_fills),
                std::end(t_m_m.trap_managers_instant_capture[phase_index]
                             .stored_watermark_fills));
            REQUIRE_THAT(test, Catch::Approx(answer));
        }

        // Reset
        t_m_m.reset_trap_states();

        for (int phase_index = 0; phase_index < ccd.n_phases; phase_index++) {
            REQUIRE(
                t_m_m.trap_managers_slow_capture[phase_index].n_active_watermarks == 0);
            REQUIRE(
                t_m_m.trap_managers_slow_capture[phase_index].i_first_active_wmk == 0);
            answer = std::vector<double>(25, 0.0);
            test.assign(
                std::begin(
                    t_m_m.trap_managers_slow_capture[phase_index].watermark_volumes),
                std::end(
                    t_m_m.trap_managers_slow_capture[phase_index].watermark_volumes));
            REQUIRE_THAT(test, Catch::Approx(answer));
            answer = std::vector<double>(50, 0.0);
            test.assign(
                std::begin(
                    t_m_m.trap_managers_slow_capture[phase_index].watermark_fills),
                std::end(
                    t_m_m.trap_managers_slow_capture[phase_index].watermark_fills));
            REQUIRE_THAT(test, Catch::Approx(answer));

            REQUIRE(
                t_m_m.trap_managers_instant_capture[phase_index].n_active_watermarks ==
                0);
            REQUIRE(
                t_m_m.trap_managers_instant_capture[phase_index].i_first_active_wmk ==
                0);
            answer = std::vector<double>(13, 0.0);
            test.assign(
                std::begin(
                    t_m_m.trap_managers_instant_capture[phase_index].watermark_volumes),
                std::end(t_m_m.trap_managers_instant_capture[phase_index]
                             .watermark_volumes));
            REQUIRE_THAT(test, Catch::Approx(answer));
            answer = std::vector<double>(13, 0.0);
            test.assign(
                std::begin(
                    t_m_m.trap_managers_instant_capture[phase_index].watermark_fills),
                std::end(
                    t_m_m.trap_managers_instant_capture[phase_index].watermark_fills));
            REQUIRE_THAT(test, Catch::Approx(answer));
        }

        // Restore
        t_m_m.restore_trap_states();

        for (int phase_index = 0; phase_index < ccd.n_phases; phase_index++) {
            REQUIRE(
                t_m_m.trap_managers_slow_capture[phase_index].n_active_watermarks == 3);
            REQUIRE(
                t_m_m.trap_managers_slow_capture[phase_index].i_first_active_wmk == 1);
            answer.assign(std::begin(volumes), std::end(volumes));
            test.assign(
                std::begin(
                    t_m_m.trap_managers_slow_capture[phase_index].watermark_volumes),
                std::end(
                    t_m_m.trap_managers_slow_capture[phase_index].watermark_volumes));
            REQUIRE_THAT(test, Catch::Approx(answer));
            answer.assign(std::begin(fills), std::end(fills));
            test.assign(
                std::begin(
                    t_m_m.trap_managers_slow_capture[phase_index].watermark_fills),
                std::end(
                    t_m_m.trap_managers_slow_capture[phase_index].watermark_fills));
            REQUIRE_THAT(test, Catch::Approx(answer));

            REQUIRE(
                t_m_m.trap_managers_instant_capture[phase_index].n_active_watermarks ==
                3);
            REQUIRE(
                t_m_m.trap_managers_instant_capture[phase_index].i_first_active_wmk ==
                1);
            answer.assign(std::begin(volumes_ic), std::end(volumes_ic));
            test.assign(
                std::begin(
                    t_m_m.trap_managers_instant_capture[phase_index].watermark_volumes),
                std::end(t_m_m.trap_managers_instant_capture[phase_index]
                             .watermark_volumes));
            REQUIRE_THAT(test, Catch::Approx(answer));
            answer.assign(std::begin(fills_ic), std::end(fills_ic));
            test.assign(
                std::begin(
                    t_m_m.trap_managers_instant_capture[phase_index].watermark_fills),
                std::end(
                    t_m_m.trap_managers_instant_capture[phase_index].watermark_fills));
            REQUIRE_THAT(test, Catch::Approx(answer));
        }
    }
}

TEST_CASE("Test instant-capture traps: release", "[trap_managers]") {
    TrapInstantCapture trap_1(10.0, -1.0 / log(0.5));
    TrapInstantCapture trap_2(8.0, -1.0 / log(0.2));

    double n_electrons_released;
    std::vector<double> test, answer;

    SECTION("Empty release") {
        TrapManagerInstantCapture trap_manager(
            std::valarray<TrapInstantCapture>{trap_1, trap_2}, 4, ccd_phase);
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
        TrapManagerInstantCapture trap_manager(
            std::valarray<TrapInstantCapture>{trap_1}, 4, ccd_phase);
        trap_manager.initialise_trap_states();
        trap_manager.set_fill_probabilities_from_dwell_time(1.0);
        trap_manager.n_active_watermarks = 3;
        trap_manager.watermark_volumes = {0.5, 0.2, 0.1, 0.0, 0.0};
        trap_manager.watermark_fills = {0.8 * 10, 0.4 * 10, 0.2 * 10, 0.0, 0.0};
        n_electrons_released = trap_manager.n_electrons_released();

        REQUIRE(n_electrons_released == Approx(2.5));
        answer = {0.5, 0.2, 0.1, 0.0, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer));
        answer = {0.4 * 10, 0.2 * 10, 0.1 * 10, 0.0, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer));
    }

    SECTION("Single trap release, longer dwell time") {
        TrapManagerInstantCapture trap_manager(
            std::valarray<TrapInstantCapture>{trap_1}, 4, ccd_phase);
        trap_manager.initialise_trap_states();
        trap_manager.set_fill_probabilities_from_dwell_time(2.0);
        trap_manager.n_active_watermarks = 3;
        trap_manager.watermark_volumes = {0.5, 0.2, 0.1, 0.0, 0.0};
        trap_manager.watermark_fills = {0.8 * 10, 0.4 * 10, 0.2 * 10, 0.0, 0.0};
        n_electrons_released = trap_manager.n_electrons_released();

        REQUIRE(n_electrons_released == Approx(3.75));
        answer = {0.5, 0.2, 0.1, 0.0, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer));
        answer = {0.2 * 10, 0.1 * 10, 0.05 * 10, 0.0, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer));
    }

    SECTION("Single trap release, smaller timescale and smaller dwell time") {
        // Should be same end result
        TrapInstantCapture trap(10.0, -0.5 / log(0.5));
        TrapManagerInstantCapture trap_manager(
            std::valarray<TrapInstantCapture>{trap}, 4, ccd_phase);
        trap_manager.initialise_trap_states();
        trap_manager.set_fill_probabilities_from_dwell_time(0.5);
        trap_manager.n_active_watermarks = 3;
        trap_manager.watermark_volumes = {0.5, 0.2, 0.1, 0.0, 0.0};
        trap_manager.watermark_fills = {0.8 * 10, 0.4 * 10, 0.2 * 10, 0.0, 0.0};
        n_electrons_released = trap_manager.n_electrons_released();

        REQUIRE(n_electrons_released == Approx(2.5));
        answer = {0.5, 0.2, 0.1, 0.0, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer));
        answer = {0.4 * 10, 0.2 * 10, 0.1 * 10, 0.0, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer));
    }

    SECTION("Multiple traps release") {
        TrapManagerInstantCapture trap_manager(
            std::valarray<TrapInstantCapture>{trap_1, trap_2}, 4, ccd_phase);
        trap_manager.initialise_trap_states();
        trap_manager.set_fill_probabilities_from_dwell_time(1.0);
        trap_manager.n_active_watermarks = 3;
        trap_manager.watermark_volumes = {0.5, 0.2, 0.1, 0.0, 0.0};
        trap_manager.watermark_fills = {
            // clang-format off
            0.8 * 10, 0.3 * 8,
            0.4 * 10, 0.2 * 8,
            0.2 * 10, 0.1 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
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
            0.4 * 10, 0.06 * 8,
            0.2 * 10, 0.04 * 8,
            0.1 * 10, 0.02 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            // clang-format on
        };
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer));
    }

    SECTION("Multiple traps release, non-zero first active watermark") {
        TrapManagerInstantCapture trap_manager(
            std::valarray<TrapInstantCapture>{trap_1, trap_2}, 4, ccd_phase);
        trap_manager.initialise_trap_states();
        trap_manager.set_fill_probabilities_from_dwell_time(1.0);
        trap_manager.n_active_watermarks = 3;
        trap_manager.i_first_active_wmk = 2;
        trap_manager.watermark_volumes = {0.3, 0.2, 0.5, 0.2, 0.1};
        trap_manager.watermark_fills = {
            // clang-format off
            0.8 * 10, 0.3 * 8,
            0.4 * 10, 0.2 * 8,
            0.8 * 10, 0.3 * 8,
            0.4 * 10, 0.2 * 8,
            0.2 * 10, 0.1 * 8,
            // clang-format on
        };

        n_electrons_released = trap_manager.n_electrons_released();

        REQUIRE(n_electrons_released == Approx(2.5 + 1.28));
        answer = {0.3, 0.2, 0.5, 0.2, 0.1};
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer));
        answer = {
            // clang-format off
            0.8 * 10, 0.3 * 8,
            0.4 * 10, 0.2 * 8,
            0.4 * 10, 0.06 * 8,
            0.2 * 10, 0.04 * 8,
            0.1 * 10, 0.02 * 8,
            // clang-format on
        };
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer));
    }
}

TEST_CASE("Test instant-capture traps: capture A", "[trap_managers]") {
    // Base cases: i_first_active_wmk = 0, enough > 1

    TrapInstantCapture trap_1(10.0, -1.0 / log(0.5));
    TrapInstantCapture trap_2(8.0, -1.0 / log(0.2));

    double n_electrons_captured;
    std::vector<double> test, answer;

    SECTION("Multiple traps capture, first watermark") {
        TrapManagerInstantCapture trap_manager(
            std::valarray<TrapInstantCapture>{trap_1, trap_2}, 5, ccd_phase);
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
            1.0 * 10, 1.0 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            // clang-format on
        };
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer));
    }

    SECTION("Multiple traps capture, cloud below watermarks") {
        TrapManagerInstantCapture trap_manager(
            std::valarray<TrapInstantCapture>{trap_1, trap_2}, 5, ccd_phase);
        trap_manager.initialise_trap_states();
        trap_manager.set_fill_probabilities_from_dwell_time(1.0);
        trap_manager.n_active_watermarks = 3;
        trap_manager.watermark_volumes = {0.5, 0.2, 0.1, 0.0, 0.0, 0.0};
        trap_manager.watermark_fills = {
            // clang-format off
            0.8 * 10, 0.7 * 8,
            0.4 * 10, 0.3 * 8,
            0.3 * 10, 0.2 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
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
            1.0 * 10, 1.0 * 8,
            0.8 * 10, 0.7 * 8,
            0.4 * 10, 0.3 * 8,
            0.3 * 10, 0.2 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            // clang-format on
        };
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer).margin(1e-99));
    }

    SECTION("Multiple traps capture, cloud above watermarks") {
        TrapManagerInstantCapture trap_manager(
            std::valarray<TrapInstantCapture>{trap_1, trap_2}, 5, ccd_phase);
        trap_manager.initialise_trap_states();
        trap_manager.set_fill_probabilities_from_dwell_time(1.0);
        trap_manager.n_active_watermarks = 3;
        trap_manager.watermark_volumes = {0.5, 0.2, 0.1, 0.0, 0.0, 0.0};
        trap_manager.watermark_fills = {
            // clang-format off
            0.8 * 10, 0.7 * 8,
            0.4 * 10, 0.3 * 8,
            0.3 * 10, 0.2 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
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
            0.8 * 10, 0.7 * 8,
            0.4 * 10, 0.3 * 8,
            1.0 * 10, 1.0 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            // clang-format on
        };
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer));
    }

    SECTION("Single traps capture, cloud between watermarks") {
        TrapManagerInstantCapture trap_manager(
            std::valarray<TrapInstantCapture>{trap_1}, 5, ccd_phase);
        trap_manager.initialise_trap_states();
        trap_manager.set_fill_probabilities_from_dwell_time(1.0);
        trap_manager.n_active_watermarks = 3;
        trap_manager.watermark_volumes = {0.5, 0.2, 0.1, 0.0, 0.0, 0.0};
        trap_manager.watermark_fills = {0.8 * 10, 0.4 * 10, 0.3 * 10, 0.0, 0.0, 0.0};
        n_electrons_captured = trap_manager.n_electrons_captured(6e3);

        REQUIRE(n_electrons_captured == Approx((0.5 * 0.2 + 0.1 * 0.6) * 10.0));
        REQUIRE(trap_manager.n_active_watermarks == 3);
        REQUIRE(trap_manager.i_first_active_wmk == 0);

        answer = {0.6, 0.1, 0.1, 0.0, 0.0, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer));
        answer = {1.0 * 10, 0.4 * 10, 0.3 * 10, 0.0, 0.0, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer));
    }

    SECTION("Multiple traps capture, cloud between watermarks") {
        TrapManagerInstantCapture trap_manager(
            std::valarray<TrapInstantCapture>{trap_1, trap_2}, 5, ccd_phase);
        trap_manager.initialise_trap_states();
        trap_manager.set_fill_probabilities_from_dwell_time(1.0);
        trap_manager.n_active_watermarks = 3;
        trap_manager.watermark_volumes = {0.5, 0.2, 0.1, 0.0, 0.0, 0.0};
        trap_manager.watermark_fills = {
            // clang-format off
            0.8 * 10, 0.7 * 8,
            0.4 * 10, 0.3 * 8,
            0.3 * 10, 0.2 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
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
            1.0 * 10, 1.0 * 8,
            0.4 * 10, 0.3 * 8,
            0.3 * 10, 0.2 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            // clang-format on
        };
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer));
    }

    SECTION("Multiple traps capture, cloud between watermarks 2") {
        TrapManagerInstantCapture trap_manager(
            std::valarray<TrapInstantCapture>{trap_1, trap_2}, 5, ccd_phase);
        trap_manager.initialise_trap_states();
        trap_manager.set_fill_probabilities_from_dwell_time(1.0);
        trap_manager.n_active_watermarks = 4;
        trap_manager.watermark_volumes = {0.5, 0.2, 0.1, 0.1, 0.0, 0.0};
        trap_manager.watermark_fills = {
            // clang-format off
            0.8 * 10, 0.7 * 8,
            0.4 * 10, 0.3 * 8,
            0.3 * 10, 0.2 * 8,
            0.2 * 10, 0.1 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
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
            0.8 * 10, 0.7 * 8,
            1.0 * 10, 1.0 * 8,
            0.3 * 10, 0.2 * 8,
            0.2 * 10, 0.1 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            // clang-format on
        };
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer));
    }

    SECTION("Multiple traps capture, cloud between watermarks 3") {
        TrapManagerInstantCapture trap_manager(
            std::valarray<TrapInstantCapture>{trap_1, trap_2}, 5, ccd_phase);
        trap_manager.initialise_trap_states();
        trap_manager.set_fill_probabilities_from_dwell_time(1.0);
        trap_manager.n_active_watermarks = 4;
        trap_manager.watermark_volumes = {0.5, 0.2, 0.1, 0.1, 0.0, 0.0};
        trap_manager.watermark_fills = {
            // clang-format off
            0.8 * 10, 0.7 * 8,
            0.4 * 10, 0.3 * 8,
            0.3 * 10, 0.2 * 8,
            0.2 * 10, 0.1 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
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
            0.8 * 10, 0.7 * 8,
            0.4 * 10, 0.3 * 8,
            1.0 * 10, 1.0 * 8,
            0.2 * 10, 0.1 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            // clang-format on
        };
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer));
    }

    SECTION("Same cloud volume as existing watermark") {
        TrapManagerInstantCapture trap_manager(
            std::valarray<TrapInstantCapture>{trap_1, trap_2}, 5, ccd_phase);
        trap_manager.initialise_trap_states();
        trap_manager.set_fill_probabilities_from_dwell_time(1.0);
        trap_manager.n_active_watermarks = 4;
        trap_manager.watermark_volumes = {0.5, 0.2, 0.1, 0.1, 0.0, 0.0};
        trap_manager.watermark_fills = {
            // clang-format off
            0.8 * 10, 0.7 * 8,
            0.4 * 10, 0.3 * 8,
            0.3 * 10, 0.2 * 8,
            0.2 * 10, 0.1 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
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
            0.8 * 10, 0.7 * 8,
            1.0 * 10, 1.0 * 8,
            0.3 * 10, 0.2 * 8,
            0.2 * 10, 0.1 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
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

    SECTION("Capture, non-zero first active watermark, cloud below watermarks") {
        TrapManagerInstantCapture trap_manager(
            std::valarray<TrapInstantCapture>{trap_1, trap_2}, 5, ccd_phase);
        trap_manager.initialise_trap_states();
        trap_manager.set_fill_probabilities_from_dwell_time(1.0);
        trap_manager.n_active_watermarks = 3;
        trap_manager.i_first_active_wmk = 2;
        trap_manager.watermark_volumes = {0.3, 0.2, 0.5, 0.2, 0.1, 0.0};
        trap_manager.watermark_fills = {
            // clang-format off
            0.8 * 10, 0.7 * 8,
            0.4 * 10, 0.3 * 8,
            0.8 * 10, 0.7 * 8,
            0.4 * 10, 0.3 * 8,
            0.3 * 10, 0.2 * 8,
            0.0 * 10, 0.0 * 8,
            // clang-format on
        };
        n_electrons_captured = trap_manager.n_electrons_captured(3e3);

        REQUIRE(n_electrons_captured == Approx((0.3 * 0.2) * 10.0 + (0.3 * 0.3) * 8.0));
        REQUIRE(trap_manager.n_active_watermarks == 4);
        REQUIRE(trap_manager.i_first_active_wmk == 1);

        answer = {0.3, 0.3, 0.2, 0.2, 0.1, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer).margin(1e-99));
        answer = {
            // clang-format off
            0.8 * 10, 0.7 * 8,
            1.0 * 10, 1.0 * 8,
            0.8 * 10, 0.7 * 8,
            0.4 * 10, 0.3 * 8,
            0.3 * 10, 0.2 * 8,
            0.0 * 10, 0.0 * 8,
            // clang-format on
        };
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer).margin(1e-99));
    }

    SECTION("Capture, non-zero first active watermark, cloud between watermarks") {
        TrapManagerInstantCapture trap_manager(
            std::valarray<TrapInstantCapture>{trap_1, trap_2}, 5, ccd_phase);
        trap_manager.initialise_trap_states();
        trap_manager.set_fill_probabilities_from_dwell_time(1.0);
        trap_manager.n_active_watermarks = 4;
        trap_manager.i_first_active_wmk = 2;
        trap_manager.watermark_volumes = {0.3, 0.2, 0.5, 0.2, 0.1, 0.1};
        trap_manager.watermark_fills = {
            // clang-format off
            0.8 * 10, 0.7 * 8,
            0.4 * 10, 0.3 * 8,
            0.8 * 10, 0.7 * 8,
            0.4 * 10, 0.3 * 8,
            0.3 * 10, 0.2 * 8,
            0.2 * 10, 0.1 * 8,
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
            0.8 * 10, 0.7 * 8,
            0.4 * 10, 0.3 * 8,
            0.8 * 10, 0.7 * 8,
            1.0 * 10, 1.0 * 8,
            0.3 * 10, 0.2 * 8,
            0.2 * 10, 0.1 * 8,
            // clang-format on
        };
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer));
    }

    CCDPhase ccd_phase_2(1e4, 1e-7, 0.5);

    SECTION("Not-enough capture, first watermark") {
        TrapManagerInstantCapture trap_manager(
            std::valarray<TrapInstantCapture>{trap_1}, 5, ccd_phase_2);
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
        answer = {0.50001 * 10, 0.0, 0.0, 0.0, 0.0, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer));
    }

    SECTION("Not-enough capture, cloud below watermarks") {
        TrapManagerInstantCapture trap_manager(
            std::valarray<TrapInstantCapture>{trap_1, trap_2}, 5, ccd_phase_2);
        trap_manager.initialise_trap_states();
        trap_manager.set_fill_probabilities_from_dwell_time(1.0);
        trap_manager.n_active_watermarks = 3;
        trap_manager.watermark_volumes = {0.5, 0.2, 0.1, 0.0, 0.0, 0.0};
        trap_manager.watermark_fills = {
            // clang-format off
            0.5 * 10, 0.25 * 8,
            0.4 * 10, 0.3 * 8,
            0.3 * 10, 0.2 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
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
            (0.5 + enough * (1.0 - 0.5)) * 10, (0.25 + enough * (1.0 - 0.25)) * 8,
            0.5 * 10, 0.25 * 8,
            0.4 * 10, 0.3 * 8,
            0.3 * 10, 0.2 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            // clang-format on
        };
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer).margin(1e-99));
    }

    SECTION("Not-enough capture, cloud above watermarks") {
        TrapManagerInstantCapture trap_manager(
            std::valarray<TrapInstantCapture>{trap_1, trap_2}, 5, ccd_phase_2);
        trap_manager.initialise_trap_states();
        trap_manager.set_fill_probabilities_from_dwell_time(1.0);
        trap_manager.n_active_watermarks = 2;
        trap_manager.watermark_volumes = {2e-4, 1e-4, 0.0, 0.0, 0.0, 0.0};
        trap_manager.watermark_fills = {
            // clang-format off
            0.8 * 10, 0.7 * 8,
            0.4 * 10, 0.3 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
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
                (1.9999e-4 * enough) * 10.0 + (1.9999e-4 * enough) * 8.0));
        REQUIRE(trap_manager.n_active_watermarks == 3);

        answer = {2e-4, 1e-4, 1.9999e-4, 0.0, 0.0, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer).margin(1e-99));
        answer = {
            // clang-format off
            (0.8 + enough * (1.0 - 0.8)) * 10, (0.7 + enough * (1.0 - 0.7)) * 8,
            (0.4 + enough * (1.0 - 0.4)) * 10, (0.3 + enough * (1.0 - 0.3)) * 8,
            enough * 10, enough * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            // clang-format on
        };
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer).margin(1e-99));
    }

    SECTION("Not-enough capture, cloud between watermarks") {
        TrapManagerInstantCapture trap_manager(
            std::valarray<TrapInstantCapture>{trap_1, trap_2}, 5, ccd_phase_2);
        trap_manager.initialise_trap_states();
        trap_manager.set_fill_probabilities_from_dwell_time(1.0);
        trap_manager.n_active_watermarks = 4;
        trap_manager.watermark_volumes = {2e-4, 1e-4, 0.2, 0.1, 0.0, 0.0, 0.0};
        trap_manager.watermark_fills = {
            // clang-format off
            0.8 * 10, 0.7 * 8,
            0.4 * 10, 0.3 * 8,
            0.3 * 10, 0.2 * 8,
            0.2 * 10, 0.1 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
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

        answer = {2e-4, 1e-4, 1.9999e-4, 0.2 - 1.9999e-4, 0.1, 0.0, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer).margin(1e-99));
        answer = {
            // clang-format off
            (0.8 + enough * (1.0 - 0.8)) * 10, (0.7 + enough * (1.0 - 0.7)) * 8,
            (0.4 + enough * (1.0 - 0.4)) * 10, (0.3 + enough * (1.0 - 0.3)) * 8,
            (0.3 + enough * (1.0 - 0.3)) * 10, (0.2 + enough * (1.0 - 0.2)) * 8,
            0.3 * 10, 0.2 * 8,
            0.2 * 10, 0.1 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            // clang-format on
        };
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer).margin(1e-99));
    }

    SECTION("Not-enough capture, non-zero first active watermark, cloud between") {
        TrapManagerInstantCapture trap_manager(
            std::valarray<TrapInstantCapture>{trap_1, trap_2}, 6, ccd_phase_2);
        trap_manager.initialise_trap_states();
        trap_manager.set_fill_probabilities_from_dwell_time(1.0);
        trap_manager.n_active_watermarks = 4;
        trap_manager.i_first_active_wmk = 2;
        trap_manager.watermark_volumes = {0.3, 0.2, 2e-4, 1e-4, 0.2, 0.1, 0.0};
        trap_manager.watermark_fills = {
            // clang-format off
            0.8 * 10, 0.7 * 8,
            0.4 * 10, 0.3 * 8,
            0.8 * 10, 0.7 * 8,
            0.4 * 10, 0.3 * 8,
            0.3 * 10, 0.2 * 8,
            0.2 * 10, 0.1 * 8,
            0.0 * 10, 0.0 * 8,
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

        answer = {0.3, 0.2, 2e-4, 1e-4, 1.9999e-4, 0.2 - 1.9999e-4, 0.1};
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer).margin(1e-99));
        answer = {
            // clang-format off
            0.8 * 10, 0.7 * 8,
            0.4 * 10, 0.3 * 8,
            (0.8 + enough * (1.0 - 0.8)) * 10, (0.7 + enough * (1.0 - 0.7)) * 8,
            (0.4 + enough * (1.0 - 0.4)) * 10, (0.3 + enough * (1.0 - 0.3)) * 8,
            (0.3 + enough * (1.0 - 0.3)) * 10, (0.2 + enough * (1.0 - 0.2)) * 8,
            0.3 * 10, 0.2 * 8,
            0.2 * 10, 0.1 * 8,
            // clang-format on
        };
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer).margin(1e-99));
    }
}

TEST_CASE("Test slow-capture traps: release and capture", "[trap_managers]") {
    TrapSlowCapture trap_1(10.0, -1.0 / log(0.5), 0.3);
    TrapSlowCapture trap_2(8.0, -1.0 / log(0.2), 0.2);

    double n_released_and_captured;
    std::vector<double> test, answer;

    SECTION("Empty traps, no cloud") {
        TrapManagerSlowCapture trap_manager(
            std::valarray<TrapSlowCapture>{trap_1, trap_2}, 3, ccd_phase);
        trap_manager.initialise_trap_states();

        n_released_and_captured = trap_manager.n_electrons_released_and_captured(0.0);

        REQUIRE(n_released_and_captured == Approx(0.0));
        answer = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
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

    SECTION("Multiple traps, no cloud") {
        TrapManagerSlowCapture trap_manager(
            std::valarray<TrapSlowCapture>{trap_1, trap_2}, 3, ccd_phase);
        trap_manager.initialise_trap_states();
        trap_manager.set_fill_probabilities_from_dwell_time(1.0);
        trap_manager.n_active_watermarks = 3;
        trap_manager.watermark_volumes = {0.5, 0.2, 0.1, 0.0, 0.0, 0.0, 0.0};
        trap_manager.watermark_fills = {
            // clang-format off
            0.8 * 10, 0.3 * 8,
            0.4 * 10, 0.2 * 8,
            0.2 * 10, 0.1 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            // clang-format on
        };

        n_released_and_captured = trap_manager.n_electrons_released_and_captured(0.0);

        REQUIRE(n_released_and_captured == Approx(3.77631));
        answer = {3.78e-4, 0.5 - 3.78e-4, 0.2, 0.1, 0.0, 0.0, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer));
        answer = {
            // clang-format off
            0.820221 * 10, 0.755555 * 8,
            0.4 * 10, 0.06 * 8,
            0.2 * 10, 0.04 * 8,
            0.1 * 10, 0.02 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            // clang-format on
        };
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer));
    }

    SECTION("Empty traps, first cloud") {
        TrapManagerSlowCapture trap_manager(
            std::valarray<TrapSlowCapture>{trap_1, trap_2}, 3, ccd_phase);
        trap_manager.initialise_trap_states();
        trap_manager.set_fill_probabilities_from_dwell_time(1.0);
        trap_manager.n_active_watermarks = 0;
        n_released_and_captured = trap_manager.n_electrons_released_and_captured(6e3);

        REQUIRE(n_released_and_captured == Approx(-8.5048));
        REQUIRE(trap_manager.n_active_watermarks == 1);
        REQUIRE(trap_manager.i_first_active_wmk == 0);

        answer = {0.6, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer));
        answer = {
            // clang-format off
            0.813086 * 10, 0.755475 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            // clang-format on
        };
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer));
    }

    SECTION("Multiple traps, cloud below watermarks") {
        TrapManagerSlowCapture trap_manager(
            std::valarray<TrapSlowCapture>{trap_1, trap_2}, 3, ccd_phase);
        trap_manager.initialise_trap_states();
        trap_manager.set_fill_probabilities_from_dwell_time(1.0);
        trap_manager.n_active_watermarks = 3;
        trap_manager.watermark_volumes = {0.5, 0.2, 0.1, 0.0, 0.0, 0.0, 0.0};
        trap_manager.watermark_fills = {
            // clang-format off
            0.8 * 10, 0.7 * 8,
            0.4 * 10, 0.3 * 8,
            0.3 * 10, 0.2 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            // clang-format on
        };
        n_released_and_captured = trap_manager.n_electrons_released_and_captured(3e3);

        REQUIRE(n_released_and_captured == Approx(2.53801));
        REQUIRE(trap_manager.n_active_watermarks == 5);
        REQUIRE(trap_manager.i_first_active_wmk == 0);

        answer = {0.3, 2.758e-4, 0.2 - 2.758e-4, 0.2, 0.1, 0.0, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer).margin(1e-99));
        answer = {
            // clang-format off
            0.827356 * 10, 0.756418 * 8,
            0.820221 * 10, 0.755663 * 8,
            0.4 * 10, 0.14 * 8,
            0.2 * 10, 0.06 * 8,
            0.15 * 10, 0.04 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            // clang-format on
        };
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer).margin(1e-99));
    }

    SECTION("Multiple traps, cloud above watermarks") {
        TrapManagerSlowCapture trap_manager(
            std::valarray<TrapSlowCapture>{trap_1, trap_2}, 3, ccd_phase);
        trap_manager.initialise_trap_states();
        trap_manager.set_fill_probabilities_from_dwell_time(1.0);
        trap_manager.n_active_watermarks = 3;
        trap_manager.watermark_volumes = {0.5, 0.2, 0.1, 0.0, 0.0, 0.0, 0.0};
        trap_manager.watermark_fills = {
            // clang-format off
            0.8 * 10, 0.7 * 8,
            0.4 * 10, 0.3 * 8,
            0.3 * 10, 0.2 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            // clang-format on
        };

        n_released_and_captured = trap_manager.n_electrons_released_and_captured(9e3);

        REQUIRE(n_released_and_captured == Approx(-4.3128));
        REQUIRE(trap_manager.n_active_watermarks == 4);
        REQUIRE(trap_manager.i_first_active_wmk == 0);

        answer = {0.5, 0.2, 0.1, 0.1, 0.0, 0.0, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer));
        answer = {
            // clang-format off
            0.827356 * 10, 0.756418 * 8,
            0.820221 * 10, 0.755879 * 8,
            0.818438 * 10, 0.755744 * 8,
            0.813086 * 10, 0.755475 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            // clang-format on
        };
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer));
    }

    SECTION("Single traps, cloud between watermarks") {
        TrapManagerSlowCapture trap_manager(
            std::valarray<TrapSlowCapture>{trap_1}, 5, ccd_phase);
        trap_manager.initialise_trap_states();
        trap_manager.set_fill_probabilities_from_dwell_time(1.0);
        trap_manager.n_active_watermarks = 3;
        trap_manager.watermark_volumes = {0.5, 0.2, 0.1, 0.0, 0.0, 0.0};
        trap_manager.watermark_fills = {0.8 * 10, 0.4 * 10, 0.3 * 10, 0.0, 0.0, 0.0};
        n_released_and_captured = trap_manager.n_electrons_released_and_captured(6e3);

        REQUIRE(n_released_and_captured == Approx(-0.207217));
        REQUIRE(trap_manager.n_active_watermarks == 5);
        REQUIRE(trap_manager.i_first_active_wmk == 0);

        answer = {0.5, 0.1, 3.5e-5, 0.1 - 3.5e-5, 0.1, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer));
        answer = {0.827356 * 10, 0.820221 * 10, 0.816654 * 10,
                  0.2 * 10,      0.15 * 10,     0.0};
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer));
    }

    SECTION("Multiple traps, cloud between watermarks") {
        TrapManagerSlowCapture trap_manager(
            std::valarray<TrapSlowCapture>{trap_1, trap_2}, 3, ccd_phase);
        trap_manager.initialise_trap_states();
        trap_manager.set_fill_probabilities_from_dwell_time(1.0);
        trap_manager.n_active_watermarks = 3;
        trap_manager.watermark_volumes = {0.5, 0.2, 0.1, 0.0, 0.0, 0.0, 0.0};
        trap_manager.watermark_fills = {
            // clang-format off
            0.8 * 10, 0.7 * 8,
            0.4 * 10, 0.3 * 8,
            0.3 * 10, 0.2 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            // clang-format on
        };
        n_released_and_captured = trap_manager.n_electrons_released_and_captured(6e3);

        REQUIRE(n_released_and_captured == Approx(-0.478162));
        REQUIRE(trap_manager.n_active_watermarks == 5);
        REQUIRE(trap_manager.i_first_active_wmk == 0);

        answer = {0.5, 0.1, 6.7e-05, 0.1 - 6.7e-05, 0.1, 0.0, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer));
        answer = {
            // clang-format off
            0.827356 * 10, 0.756418 * 8,
            0.820221 * 10, 0.755879 * 8,
            0.816654 * 10, 0.755555 * 8,
            0.2 * 10, 0.06 * 8,
            0.15 * 10, 0.04 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            // clang-format on
        };
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer));
    }

    SECTION("Multiple traps, cloud between watermarks 2") {
        TrapManagerSlowCapture trap_manager(
            std::valarray<TrapSlowCapture>{trap_1, trap_2}, 3, ccd_phase);
        trap_manager.initialise_trap_states();
        trap_manager.set_fill_probabilities_from_dwell_time(1.0);
        trap_manager.n_active_watermarks = 4;
        trap_manager.watermark_volumes = {0.5, 0.2, 0.1, 0.1, 0.0, 0.0, 0.0};
        trap_manager.watermark_fills = {
            // clang-format off
            0.8 * 10, 0.7 * 8,
            0.4 * 10, 0.3 * 8,
            0.3 * 10, 0.2 * 8,
            0.2 * 10, 0.1 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            // clang-format on
        };
        n_released_and_captured = trap_manager.n_electrons_released_and_captured(7.5e3);

        REQUIRE(n_released_and_captured == Approx(-2.11119));
        REQUIRE(trap_manager.n_active_watermarks == 6);
        REQUIRE(trap_manager.i_first_active_wmk == 0);

        answer = {0.5, 0.2, 0.05, 3.03e-05, 0.05 - 3.03e-05, 0.1, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer));
        answer = {
            // clang-format off
            0.827356 * 10, 0.756418 * 8,
            0.820221 * 10, 0.755879 * 8,
            0.818438 * 10, 0.755744 * 8,
            0.815762 * 10, 0.755528 * 8,
            0.15 * 10, 0.04 * 8,
            0.1 * 10, 0.02 * 8,
            0.0 * 10, 0.0 * 8,
            // clang-format on
        };
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer));
    }

    SECTION("Multiple traps, cloud between watermarks 3") {
        TrapManagerSlowCapture trap_manager(
            std::valarray<TrapSlowCapture>{trap_1, trap_2}, 3, ccd_phase);
        trap_manager.initialise_trap_states();
        trap_manager.set_fill_probabilities_from_dwell_time(1.0);
        trap_manager.n_active_watermarks = 4;
        trap_manager.watermark_volumes = {0.5, 0.2, 0.1, 0.1, 0.0, 0.0, 0.0};
        trap_manager.watermark_fills = {
            // clang-format off
            0.8 * 10, 0.7 * 8,
            0.4 * 10, 0.3 * 8,
            0.3 * 10, 0.2 * 8,
            0.2 * 10, 0.1 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            // clang-format on
        };
        n_released_and_captured = trap_manager.n_electrons_released_and_captured(8.5e3);

        REQUIRE(n_released_and_captured == Approx(-3.38401));
        REQUIRE(trap_manager.n_active_watermarks == 6);
        REQUIRE(trap_manager.i_first_active_wmk == 0);

        answer = {0.5, 0.2, 0.1, 0.05, 8.2e-06, 0.05 - 8.2e-06, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer));
        answer = {
            // clang-format off
            0.827356 * 10, 0.756418 * 8,
            0.820221 * 10, 0.755879 * 8,
            0.818438 * 10, 0.755744 * 8,
            0.816654 * 10, 0.755609 * 8,
            0.81487 * 10, 0.755501 * 8,
            0.1 * 10, 0.02 * 8,
            0.0 * 10, 0.0 * 8,
            // clang-format on
        };
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer));
    }

    CCDPhase ccd_phase_2(1e4, 1e-7, 0.5);

    SECTION("Not-enough capture, first watermark") {
        TrapManagerSlowCapture trap_manager(
            std::valarray<TrapSlowCapture>{trap_1}, 3, ccd_phase_2);
        trap_manager.initialise_trap_states();
        trap_manager.set_fill_probabilities_from_dwell_time(1.0);
        trap_manager.n_active_watermarks = 0;
        n_released_and_captured =
            trap_manager.n_electrons_released_and_captured(2.5e-3);
        // --> cloud_fractional_volume = 4.9999e-4, enough = 0.50001

        REQUIRE(n_released_and_captured == Approx(-4.9999e-4 * 0.50001 * 10.0));
        REQUIRE(trap_manager.n_active_watermarks == 1);

        answer = {4.9999e-4, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer));
        answer = {0.50001 * 10, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer));
    }

    SECTION("Not-enough capture, cloud between watermarks") {
        TrapManagerSlowCapture trap_manager(
            std::valarray<TrapSlowCapture>{trap_1, trap_2}, 3, ccd_phase_2);
        trap_manager.initialise_trap_states();
        trap_manager.set_fill_probabilities_from_dwell_time(1.0);
        trap_manager.n_active_watermarks = 4;
        trap_manager.watermark_volumes = {2e-4, 1e-4, 0.2, 0.1, 0.0, 0.0, 0.0};
        trap_manager.watermark_fills = {
            // clang-format off
            0.0008 * 10, 0.0007 * 8,
            0.0004 * 10, 0.0003 * 8,
            0.0003 * 10, 0.0002 * 8,
            0.0002 * 10, 0.0001 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            // clang-format on
        };
        n_released_and_captured =
            trap_manager.n_electrons_released_and_captured(2.5e-3);

        REQUIRE(n_released_and_captured == Approx(-0.0025));
        REQUIRE(trap_manager.n_active_watermarks == 6);

        answer = {2e-4, 1e-4, 1.9999e-4, 6.74026e-05, 0.2 - 1.9999e-4 - 6.74026e-05,
                  0.1,  0.0};
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer).margin(1e-99));
        answer = {
            // clang-format off
            0.326139 * 10, 0.302999 * 8,
            0.325897 * 10, 0.302759 * 8,
            0.325836 * 10, 0.302699 * 8,
            0.325745 * 10, 0.302603 * 8,
            0.00015 * 10, 0.00004 * 8,
            0.0001 * 10, 0.00002 * 8,
            0.0 * 10, 0.0 * 8,
            // clang-format on
        };
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer).margin(1e-99));
    }
}

TEST_CASE("Test continuum traps: utilities", "[trap_managers]") {
    TrapContinuum trap_1(10.0, -1.0 / log(0.5), 0.1);
    TrapContinuum trap_2(10.0, -1.0 / log(0.5), 1.0);

    SECTION("Initial watermarks") {
        TrapManagerContinuum trap_manager(
            std::valarray<TrapContinuum>{trap_1, trap_2}, 3, ccd_phase);
        trap_manager.initialise_trap_states();

        REQUIRE(trap_manager.n_watermarks == 4);
        REQUIRE(trap_manager.watermark_volumes.size() == 4);
        REQUIRE(trap_manager.watermark_fills.size() == 8);
        REQUIRE(trap_manager.empty_watermark == -1);
        REQUIRE(
            trap_manager.watermark_volumes.sum() ==
            trap_manager.n_watermarks * trap_manager.empty_watermark);
        REQUIRE(
            trap_manager.watermark_fills.sum() ==
            2 * trap_manager.n_watermarks * trap_manager.empty_watermark);
    }

    SECTION("Number of trapped electrons") {
        double n_trapped_electrons;

        TrapManagerContinuum trap_manager(
            std::valarray<TrapContinuum>{trap_1}, 5, ccd_phase);
        trap_manager.initialise_trap_states();
        trap_manager.n_active_watermarks = 3;
        trap_manager.watermark_volumes = {0.5, 0.2, 0.1, 0.0, 0.0, 0.0};
        trap_manager.watermark_fills = {
            // clang-format off
            trap_1.time_elapsed_from_fill_fraction(0.8),
            trap_1.time_elapsed_from_fill_fraction(0.4),
            trap_1.time_elapsed_from_fill_fraction(0.2),
            -1.0,
            -1.0,
            -1.0,
            // clang-format on
        };

        n_trapped_electrons = trap_manager.n_trapped_electrons_from_watermarks(
            trap_manager.watermark_volumes, trap_manager.watermark_fills);
        REQUIRE(
            n_trapped_electrons ==
            Approx((0.5 * 0.8 + 0.2 * 0.4 + 0.1 * 0.2) * trap_1.density));
    }
}

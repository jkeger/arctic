
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
double dwell_time = 1.0;

TEST_CASE("Test initialisation", "[trap_managers]") {
    TrapInstantCapture trap_1(1.0, 1.0);
    TrapInstantCapture trap_2(2.0, 2.0);
    TrapSlowCapture trap_3(3.0, 3.0, 0.0);
    TrapInstantCaptureContinuum trap_4(10.0, 4.0, 0.1);
    TrapSlowCaptureContinuum trap_5(10.0, 5.0, 0.05, 0.1);
    int max_n_transfers = 123;

    SECTION("Traps") {
        // Instant-capture traps
        std::valarray<TrapInstantCapture> traps_ic = {trap_1, trap_2};
        TrapManagerInstantCapture trap_manager_ic(
            traps_ic, max_n_transfers, ccd_phase, dwell_time);

        REQUIRE(trap_manager_ic.n_traps == 2);
        REQUIRE(trap_manager_ic.traps[0].density == trap_1.density);
        REQUIRE(trap_manager_ic.traps[1].density == trap_2.density);
        REQUIRE(trap_manager_ic.trap_densities[0] == trap_1.density);
        REQUIRE(trap_manager_ic.trap_densities[1] == trap_2.density);

        // Slow capture traps
        std::valarray<TrapSlowCapture> traps{trap_3};
        TrapManagerSlowCapture trap_manager_sc(
            traps, max_n_transfers, ccd_phase, dwell_time);

        REQUIRE(trap_manager_sc.n_traps == 1);
        REQUIRE(trap_manager_sc.traps[0].density == trap_3.density);
        REQUIRE(trap_manager_sc.trap_densities[0] == trap_3.density);

        // Instant-capture continuum traps
        std::valarray<TrapInstantCaptureContinuum> traps_ic_co = {trap_4};
        TrapManagerInstantCaptureContinuum trap_manager_ic_co(
            traps_ic_co, max_n_transfers, ccd_phase, dwell_time);

        REQUIRE(trap_manager_ic_co.n_traps == 1);
        REQUIRE(trap_manager_ic_co.traps[0].density == trap_4.density);
        REQUIRE(trap_manager_ic_co.trap_densities[0] == trap_4.density);

        // Slow-capture continuum traps
        std::valarray<TrapSlowCaptureContinuum> traps_sc_co = {trap_5};
        TrapManagerSlowCaptureContinuum trap_manager_sc_co(
            traps_sc_co, max_n_transfers, ccd_phase, dwell_time);

        REQUIRE(trap_manager_sc_co.n_traps == 1);
        REQUIRE(trap_manager_sc_co.traps[0].density == trap_5.density);
        REQUIRE(trap_manager_sc_co.trap_densities[0] == trap_5.density);
    }

    SECTION("CCD Phase") {
        // Instant-capture traps
        std::valarray<TrapInstantCapture> traps_ic = {trap_1, trap_2};
        TrapManagerInstantCapture trap_manager_ic(
            traps_ic, max_n_transfers, ccd_phase, dwell_time);

        REQUIRE(trap_manager_ic.ccd_phase.full_well_depth == ccd_phase.full_well_depth);
        REQUIRE(
            trap_manager_ic.ccd_phase.well_notch_depth == ccd_phase.well_notch_depth);
        REQUIRE(trap_manager_ic.ccd_phase.well_fill_power == ccd_phase.well_fill_power);

        // Slow capture traps
        std::valarray<TrapSlowCapture> traps{trap_3};
        TrapManagerSlowCapture trap_manager_sc(
            traps, max_n_transfers, ccd_phase, dwell_time);

        REQUIRE(trap_manager_sc.ccd_phase.full_well_depth == ccd_phase.full_well_depth);
        REQUIRE(
            trap_manager_sc.ccd_phase.well_notch_depth == ccd_phase.well_notch_depth);
        REQUIRE(trap_manager_sc.ccd_phase.well_fill_power == ccd_phase.well_fill_power);

        // Instant-capture continuum traps
        std::valarray<TrapInstantCaptureContinuum> traps_ic_co = {trap_4};
        TrapManagerInstantCaptureContinuum trap_manager_ic_co(
            traps_ic_co, max_n_transfers, ccd_phase, dwell_time);

        REQUIRE(
            trap_manager_ic_co.ccd_phase.full_well_depth == ccd_phase.full_well_depth);
        REQUIRE(
            trap_manager_ic_co.ccd_phase.well_notch_depth ==
            ccd_phase.well_notch_depth);
        REQUIRE(
            trap_manager_ic_co.ccd_phase.well_fill_power == ccd_phase.well_fill_power);

        // Slow-capture continuum traps
        std::valarray<TrapSlowCaptureContinuum> traps_sc_co = {trap_5};
        TrapManagerSlowCaptureContinuum trap_manager_sc_co(
            traps_sc_co, max_n_transfers, ccd_phase, dwell_time);

        REQUIRE(
            trap_manager_sc_co.ccd_phase.full_well_depth == ccd_phase.full_well_depth);
        REQUIRE(
            trap_manager_sc_co.ccd_phase.well_notch_depth ==
            ccd_phase.well_notch_depth);
        REQUIRE(
            trap_manager_sc_co.ccd_phase.well_fill_power == ccd_phase.well_fill_power);
    }

    SECTION("Misc attributes") {
        // Instant-capture traps
        TrapManagerInstantCapture trap_manager_ic(
            std::valarray<TrapInstantCapture>{trap_1, trap_2}, max_n_transfers,
            ccd_phase, 1.0);

        REQUIRE(trap_manager_ic.max_n_transfers == 123);
        REQUIRE(trap_manager_ic.n_watermarks_per_transfer == 1);
        REQUIRE(trap_manager_ic.empty_watermark == 0.0);
        REQUIRE(trap_manager_ic.n_active_watermarks == 0);
        REQUIRE(trap_manager_ic.i_first_active_wmk == 0);
        REQUIRE(trap_manager_ic.dwell_time == 1.0);

        // Slow capture traps
        TrapManagerSlowCapture trap_manager_sc(
            std::valarray<TrapSlowCapture>{trap_3}, max_n_transfers, ccd_phase, 2.0);

        REQUIRE(trap_manager_sc.max_n_transfers == 123);
        REQUIRE(trap_manager_sc.n_watermarks_per_transfer == 2);
        REQUIRE(trap_manager_sc.empty_watermark == 0.0);
        REQUIRE(trap_manager_sc.n_active_watermarks == 0);
        REQUIRE(trap_manager_sc.i_first_active_wmk == 0);
        REQUIRE(trap_manager_sc.dwell_time == 2.0);

        // Instant-capture continuum traps
        TrapManagerInstantCaptureContinuum trap_manager_ic_co(
            std::valarray<TrapInstantCaptureContinuum>{trap_4}, max_n_transfers,
            ccd_phase, 3.0);

        REQUIRE(trap_manager_ic_co.max_n_transfers == 123);
        REQUIRE(trap_manager_ic_co.n_watermarks_per_transfer == 1);
        REQUIRE(trap_manager_ic_co.empty_watermark == 0.0);
        REQUIRE(trap_manager_ic_co.n_active_watermarks == 0);
        REQUIRE(trap_manager_ic_co.i_first_active_wmk == 0);
        REQUIRE(trap_manager_ic_co.dwell_time == 3.0);

        // Slow-capture continuum traps
        TrapManagerSlowCaptureContinuum trap_manager_sc_co(
            std::valarray<TrapSlowCaptureContinuum>{trap_5}, max_n_transfers, ccd_phase,
            4.0);

        REQUIRE(trap_manager_sc_co.max_n_transfers == 123);
        REQUIRE(trap_manager_sc_co.n_watermarks_per_transfer == 2);
        REQUIRE(trap_manager_sc_co.empty_watermark == 0.0);
        REQUIRE(trap_manager_sc_co.n_active_watermarks == 0);
        REQUIRE(trap_manager_sc_co.i_first_active_wmk == 0);
        REQUIRE(trap_manager_sc_co.dwell_time == 4.0);

        // Instant-capture traps, non-uniform volume distribution
        TrapInstantCapture trap_surface(10.0, -1.0 / log(0.5), 0.9, 0.9);
        TrapManagerInstantCapture trap_manager_ic_2(
            std::valarray<TrapInstantCapture>{trap_1, trap_2, trap_surface},
            max_n_transfers, ccd_phase, 1.0);
        REQUIRE(!trap_manager_ic.any_non_uniform_traps);
        REQUIRE(trap_manager_ic_2.any_non_uniform_traps);
    }

    SECTION("Initial watermarks") {
        // Instant-capture traps
        TrapManagerInstantCapture trap_manager_ic(
            std::valarray<TrapInstantCapture>{trap_1}, 3, ccd_phase, dwell_time);
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

        TrapManagerInstantCapture trap_manager_ic_2(
            std::valarray<TrapInstantCapture>{trap_1, trap_2}, max_n_transfers,
            ccd_phase, dwell_time);
        trap_manager_ic_2.initialise_trap_states();

        REQUIRE(trap_manager_ic_2.n_watermarks == 124);
        REQUIRE(trap_manager_ic_2.watermark_volumes.size() == 124);
        REQUIRE(trap_manager_ic_2.watermark_fills.size() == 248);
        REQUIRE(
            trap_manager_ic_2.watermark_volumes.sum() ==
            trap_manager_ic_2.n_watermarks * trap_manager_ic_2.empty_watermark);
        REQUIRE(
            trap_manager_ic_2.watermark_fills.sum() ==
            trap_manager_ic_2.n_traps * trap_manager_ic_2.n_watermarks *
                trap_manager_ic_2.empty_watermark);

        // Slow capture traps
        TrapManagerSlowCapture trap_manager_sc(
            std::valarray<TrapSlowCapture>{trap_3}, 3, ccd_phase, dwell_time);
        trap_manager_sc.initialise_trap_states();

        REQUIRE(trap_manager_sc.n_watermarks == 7);
        REQUIRE(trap_manager_sc.watermark_volumes.size() == 7);
        REQUIRE(trap_manager_sc.watermark_fills.size() == 7);
        REQUIRE(
            trap_manager_sc.watermark_volumes.sum() ==
            trap_manager_sc.n_watermarks * trap_manager_sc.empty_watermark);
        REQUIRE(
            trap_manager_sc.watermark_fills.sum() ==
            trap_manager_sc.n_watermarks * trap_manager_sc.empty_watermark);

        // Instant-capture continuum traps
        TrapManagerInstantCaptureContinuum trap_manager_ic_co(
            std::valarray<TrapInstantCaptureContinuum>{trap_4}, 3, ccd_phase,
            dwell_time);
        trap_manager_ic_co.initialise_trap_states();

        REQUIRE(trap_manager_ic_co.n_watermarks == 4);
        REQUIRE(trap_manager_ic_co.watermark_volumes.size() == 4);
        REQUIRE(trap_manager_ic_co.watermark_fills.size() == 4);
        REQUIRE(
            trap_manager_ic_co.watermark_volumes.sum() ==
            trap_manager_ic_co.n_watermarks * trap_manager_ic_co.empty_watermark);
        REQUIRE(
            trap_manager_ic_co.watermark_fills.sum() ==
            trap_manager_ic_co.n_watermarks * trap_manager_ic_co.empty_watermark);

        // Slow-capture continuum traps
        TrapManagerSlowCaptureContinuum trap_manager_sc_co(
            std::valarray<TrapSlowCaptureContinuum>{trap_5}, 3, ccd_phase, dwell_time);
        trap_manager_sc_co.initialise_trap_states();

        REQUIRE(trap_manager_sc_co.n_watermarks == 7);
        REQUIRE(trap_manager_sc_co.watermark_volumes.size() == 7);
        REQUIRE(trap_manager_sc_co.watermark_fills.size() == 7);
        REQUIRE(
            trap_manager_sc_co.watermark_volumes.sum() ==
            trap_manager_sc_co.n_watermarks * trap_manager_sc_co.empty_watermark);
        REQUIRE(
            trap_manager_sc_co.watermark_fills.sum() ==
            trap_manager_sc_co.n_watermarks * trap_manager_sc_co.empty_watermark);
    }
}

TEST_CASE("Test utilities", "[trap_managers]") {
    TrapInstantCapture trap_1(10.0, -1.0 / log(0.5));
    TrapInstantCapture trap_2(8.0, -1.0 / log(0.2));
    TrapSlowCapture trap_3(10.0, -1.0 / log(0.5), 0.1);
    TrapSlowCapture trap_4(8.0, -1.0 / log(0.2), 1.0);
    TrapInstantCaptureContinuum trap_5(10.0, -1.0 / log(0.5), 0.1);
    TrapSlowCaptureContinuum trap_6(10.0, -1.0 / log(0.5), 0.05, 0.1);

    SECTION("Number of trapped electrons") {
        double n_trapped_electrons;

        // Instant-capture traps
        // One trap
        TrapManagerInstantCapture trap_manager_ic(
            std::valarray<TrapInstantCapture>{trap_1}, 4, ccd_phase, dwell_time);
        trap_manager_ic.initialise_trap_states();
        trap_manager_ic.n_active_watermarks = 3;
        trap_manager_ic.watermark_volumes = {0.5, 0.2, 0.1, 0.0, 0.0};
        trap_manager_ic.watermark_fills = {0.8 * 10, 0.4 * 10, 0.2 * 10, 0.0, 0.0};
        n_trapped_electrons = trap_manager_ic.n_trapped_electrons_from_watermarks(
            trap_manager_ic.watermark_volumes, trap_manager_ic.watermark_fills);
        REQUIRE(
            n_trapped_electrons ==
            Approx((0.5 * 0.8 + 0.2 * 0.4 + 0.1 * 0.2) * trap_1.density));

        // Two traps
        trap_manager_ic = TrapManagerInstantCapture(
            std::valarray<TrapInstantCapture>{trap_1, trap_2}, 4, ccd_phase,
            dwell_time);
        trap_manager_ic.initialise_trap_states();
        trap_manager_ic.n_active_watermarks = 3;
        trap_manager_ic.watermark_volumes = {0.5, 0.2, 0.1, 0.0, 0.0};
        trap_manager_ic.watermark_fills = {
            // clang-format off
            0.8 * 10, 0.3 * 8,
            0.4 * 10, 0.2 * 8,
            0.2 * 10, 0.1 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            // clang-format on
        };
        n_trapped_electrons = trap_manager_ic.n_trapped_electrons_from_watermarks(
            trap_manager_ic.watermark_volumes, trap_manager_ic.watermark_fills);
        REQUIRE(
            n_trapped_electrons ==
            Approx(
                (0.5 * 0.8 + 0.2 * 0.4 + 0.1 * 0.2) * trap_1.density +
                (0.5 * 0.3 + 0.2 * 0.2 + 0.1 * 0.1) * trap_2.density));

        // i_first_active_wmk > 0
        trap_manager_ic.initialise_trap_states();
        trap_manager_ic.n_active_watermarks = 3;
        trap_manager_ic.i_first_active_wmk = 2;
        trap_manager_ic.watermark_volumes = {0.4, 0.3, 0.5, 0.2, 0.1};
        trap_manager_ic.watermark_fills = {
            // clang-format off
            0.8 * 10, 0.3 * 8,
            0.4 * 10, 0.2 * 8,
            0.8 * 10, 0.3 * 8,
            0.4 * 10, 0.2 * 8,
            0.2 * 10, 0.1 * 8,
            // clang-format on
        };
        n_trapped_electrons = trap_manager_ic.n_trapped_electrons_from_watermarks(
            trap_manager_ic.watermark_volumes, trap_manager_ic.watermark_fills);
        REQUIRE(
            n_trapped_electrons ==
            Approx(
                (0.5 * 0.8 + 0.2 * 0.4 + 0.1 * 0.2) * trap_1.density +
                (0.5 * 0.3 + 0.2 * 0.2 + 0.1 * 0.1) * trap_2.density));

        // Slow capture traps
        TrapManagerSlowCapture trap_manager_sc(
            std::valarray<TrapSlowCapture>{trap_3}, 2, ccd_phase, dwell_time);
        trap_manager_sc.initialise_trap_states();
        trap_manager_sc.n_active_watermarks = 3;
        trap_manager_sc.watermark_volumes = {0.5, 0.2, 0.1, 0.0, 0.0};
        trap_manager_sc.watermark_fills = {0.8 * 10, 0.4 * 10, 0.2 * 10, 0.0, 0.0};
        n_trapped_electrons = trap_manager_sc.n_trapped_electrons_from_watermarks(
            trap_manager_sc.watermark_volumes, trap_manager_sc.watermark_fills);
        REQUIRE(
            n_trapped_electrons ==
            Approx((0.5 * 0.8 + 0.2 * 0.4 + 0.1 * 0.2) * trap_3.density));

        // Instant-capture continuum traps
        TrapManagerInstantCaptureContinuum trap_manager_ic_co(
            std::valarray<TrapInstantCaptureContinuum>{trap_5}, 4, ccd_phase,
            dwell_time);
        trap_manager_ic_co.initialise_trap_states();
        trap_manager_ic_co.n_active_watermarks = 3;
        trap_manager_ic_co.watermark_volumes = {0.5, 0.2, 0.1, 0.0, 0.0};
        trap_manager_ic_co.watermark_fills = {0.8 * 10, 0.4 * 10, 0.2 * 10, 0.0, 0.0};
        n_trapped_electrons = trap_manager_ic_co.n_trapped_electrons_from_watermarks(
            trap_manager_ic_co.watermark_volumes, trap_manager_ic_co.watermark_fills);
        REQUIRE(
            n_trapped_electrons ==
            Approx((0.5 * 0.8 + 0.2 * 0.4 + 0.1 * 0.2) * trap_5.density));

        // Slow-capture continuum traps
        TrapManagerSlowCaptureContinuum trap_manager_sc_co(
            std::valarray<TrapSlowCaptureContinuum>{trap_6}, 4, ccd_phase, dwell_time);
        trap_manager_sc_co.initialise_trap_states();
        trap_manager_sc_co.n_active_watermarks = 3;
        trap_manager_sc_co.watermark_volumes = {0.5, 0.2, 0.1, 0.0, 0.0};
        trap_manager_sc_co.watermark_fills = {0.8 * 10, 0.4 * 10, 0.2 * 10, 0.0, 0.0};
        n_trapped_electrons = trap_manager_sc_co.n_trapped_electrons_from_watermarks(
            trap_manager_sc_co.watermark_volumes, trap_manager_sc_co.watermark_fills);
        REQUIRE(
            n_trapped_electrons ==
            Approx((0.5 * 0.8 + 0.2 * 0.4 + 0.1 * 0.2) * trap_6.density));
    }

    SECTION("Fill probabilities") {
        // Instant-capture traps
        TrapManagerInstantCapture trap_manager_ic(
            std::valarray<TrapInstantCapture>{trap_1, trap_2}, 4, ccd_phase,
            dwell_time);
        trap_manager_ic.set_fill_probabilities();

        REQUIRE(trap_manager_ic.empty_probabilities_from_release[0] == Approx(0.5));
        REQUIRE(trap_manager_ic.empty_probabilities_from_release[1] == Approx(0.8));

        // Longer dwell time
        trap_manager_ic.dwell_time = 2.0;
        trap_manager_ic.set_fill_probabilities();

        REQUIRE(trap_manager_ic.empty_probabilities_from_release[0] == Approx(0.75));
        REQUIRE(trap_manager_ic.empty_probabilities_from_release[1] == Approx(0.96));

        // Slow capture traps
        TrapManagerSlowCapture trap_manager_sc(
            std::valarray<TrapSlowCapture>{trap_3, trap_4}, 4, ccd_phase, dwell_time);
        trap_manager_sc.set_fill_probabilities();

        REQUIRE(trap_manager_sc.fill_probabilities_from_empty[0] == Approx(0.935157));
        REQUIRE(trap_manager_sc.fill_probabilities_from_full[0] == Approx(0.935180));
        REQUIRE(trap_manager_sc.empty_probabilities_from_release[0] == Approx(0.5));

        REQUIRE(trap_manager_sc.fill_probabilities_from_empty[1] == Approx(0.355028));
        REQUIRE(trap_manager_sc.fill_probabilities_from_full[1] == Approx(0.428604));
        REQUIRE(trap_manager_sc.empty_probabilities_from_release[1] == Approx(0.8));
    }

    SECTION("Prepare interpolation tables") {
        // Instant-capture continuum traps
        TrapManagerInstantCaptureContinuum trap_manager_ic_co(
            std::valarray<TrapInstantCaptureContinuum>{trap_5}, 4, ccd_phase,
            dwell_time);
        trap_manager_ic_co.prepare_interpolation_tables();

        REQUIRE(
            trap_manager_ic_co.traps[0].fill_fraction_table.size() ==
            trap_manager_ic_co.n_intp);

        // Slow-capture continuum traps
        TrapManagerSlowCaptureContinuum trap_manager_sc_co(
            std::valarray<TrapSlowCaptureContinuum>{trap_6}, 4, ccd_phase, dwell_time);
        trap_manager_sc_co.prepare_interpolation_tables();

        REQUIRE(
            trap_manager_sc_co.traps[0].fill_fraction_table.size() ==
            trap_manager_sc_co.n_intp);
    }

    SECTION("Watermark index above cloud") {
        TrapManagerInstantCapture trap_manager(
            std::valarray<TrapInstantCapture>{trap_1, trap_2}, 4, ccd_phase,
            dwell_time);
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
        TrapManagerInstantCapture trap_manager(
            std::valarray<TrapInstantCapture>{trap_1, trap_2}, 6, ccd_phase,
            dwell_time);
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
    TrapInstantCapture trap_1(1.0, 1.0);
    TrapInstantCapture trap_2(2.0, 2.0);
    TrapSlowCapture trap_3(3.0, 3.0, 0.3);
    TrapInstantCaptureContinuum trap_4(4.0, 4.0, 0.4);
    TrapSlowCaptureContinuum trap_5(4.0, 4.0, 0.4, 0.04);
    int max_n_transfers = 123;
    std::vector<double> test, answer;

    SECTION("Initialisation, single phase, one type of traps") {
        std::valarray<TrapInstantCapture> traps_ic = {trap_1, trap_2};
        std::valarray<TrapSlowCapture> traps_sc = {};
        std::valarray<TrapInstantCaptureContinuum> traps_ic_co = {};
        std::valarray<TrapSlowCaptureContinuum> traps_sc_co = {};
        ROE roe;
        CCD ccd(ccd_phase);

        TrapManagerManager trap_manager_manager(
            traps_ic, traps_sc, traps_ic_co, traps_sc_co, max_n_transfers, ccd,
            roe.dwell_times);

        REQUIRE(trap_manager_manager.n_traps_ic == 2);
        REQUIRE(trap_manager_manager.trap_managers_ic.size() == 1);
        REQUIRE(trap_manager_manager.trap_managers_ic[0].traps.size() == 2);
        REQUIRE(trap_manager_manager.trap_managers_ic[0].dwell_time == 1.0);

        REQUIRE(trap_manager_manager.n_traps_sc == 0);
        REQUIRE(trap_manager_manager.n_traps_ic_co == 0);
        REQUIRE(trap_manager_manager.n_traps_sc_co == 0);
    }

    SECTION("Initialisation, single phase, two types of traps") {
        std::valarray<TrapInstantCapture> traps_ic = {trap_1, trap_2};
        std::valarray<TrapSlowCapture> traps_sc = {trap_3};
        std::valarray<TrapInstantCaptureContinuum> traps_ic_co = {};
        std::valarray<TrapSlowCaptureContinuum> traps_sc_co = {};
        ROE roe;
        CCD ccd(ccd_phase);

        TrapManagerManager trap_manager_manager(
            traps_ic, traps_sc, traps_ic_co, traps_sc_co, max_n_transfers, ccd,
            roe.dwell_times);

        REQUIRE(trap_manager_manager.n_traps_ic == 2);
        REQUIRE(trap_manager_manager.trap_managers_ic.size() == 1);
        REQUIRE(trap_manager_manager.trap_managers_ic[0].traps.size() == 2);
        REQUIRE(trap_manager_manager.trap_managers_ic[0].dwell_time == 1.0);

        REQUIRE(trap_manager_manager.n_traps_sc == 1);
        REQUIRE(trap_manager_manager.trap_managers_sc.size() == 1);
        REQUIRE(trap_manager_manager.trap_managers_sc[0].traps.size() == 1);
        REQUIRE(trap_manager_manager.trap_managers_sc[0].dwell_time == 1.0);

        REQUIRE(trap_manager_manager.n_traps_ic_co == 0);
        REQUIRE(trap_manager_manager.n_traps_sc_co == 0);
    }

    SECTION("Initialisation, multiphase, all types of traps") {
        std::valarray<TrapInstantCapture> traps_ic = {trap_1, trap_2};
        std::valarray<TrapSlowCapture> traps_sc = {trap_3};
        std::valarray<TrapInstantCaptureContinuum> traps_ic_co = {trap_4};
        std::valarray<TrapSlowCaptureContinuum> traps_sc_co = {trap_5};
        std::valarray<double> dwell_times = {0.8, 0.1, 0.1};
        ROE roe(dwell_times);
        CCDPhase ccd_phase_2(2e4, 0.0, 0.8);
        std::valarray<CCDPhase> phases = {ccd_phase, ccd_phase_2, ccd_phase_2};
        std::valarray<double> fractions = {0.5, 0.25, 0.25};
        CCD ccd(phases, fractions);

        TrapManagerManager trap_manager_manager(
            traps_ic, traps_sc, traps_ic_co, traps_sc_co, max_n_transfers, ccd,
            roe.dwell_times);

        REQUIRE(trap_manager_manager.n_traps_ic == 2);
        REQUIRE(trap_manager_manager.trap_managers_ic.size() == 3);
        REQUIRE(trap_manager_manager.trap_managers_ic[0].traps.size() == 2);
        REQUIRE(trap_manager_manager.trap_managers_ic[0].dwell_time == 0.8);
        REQUIRE(trap_manager_manager.trap_managers_ic[1].dwell_time == 0.1);
        REQUIRE(trap_manager_manager.trap_managers_ic[2].dwell_time == 0.1);

        REQUIRE(trap_manager_manager.n_traps_sc == 1);
        REQUIRE(trap_manager_manager.trap_managers_sc.size() == 3);
        REQUIRE(trap_manager_manager.trap_managers_sc[0].traps.size() == 1);
        REQUIRE(trap_manager_manager.trap_managers_sc[0].dwell_time == 0.8);
        REQUIRE(trap_manager_manager.trap_managers_sc[1].dwell_time == 0.1);
        REQUIRE(trap_manager_manager.trap_managers_sc[2].dwell_time == 0.1);

        REQUIRE(trap_manager_manager.n_traps_ic_co == 1);
        REQUIRE(trap_manager_manager.trap_managers_ic_co.size() == 3);
        REQUIRE(trap_manager_manager.trap_managers_ic_co[0].traps.size() == 1);
        REQUIRE(trap_manager_manager.trap_managers_ic_co[0].dwell_time == 0.8);
        REQUIRE(trap_manager_manager.trap_managers_ic_co[1].dwell_time == 0.1);
        REQUIRE(trap_manager_manager.trap_managers_ic_co[2].dwell_time == 0.1);

        REQUIRE(trap_manager_manager.n_traps_sc_co == 1);
        REQUIRE(trap_manager_manager.trap_managers_sc_co.size() == 3);
        REQUIRE(trap_manager_manager.trap_managers_sc_co[0].traps.size() == 1);
        REQUIRE(trap_manager_manager.trap_managers_sc_co[0].dwell_time == 0.8);
        REQUIRE(trap_manager_manager.trap_managers_sc_co[1].dwell_time == 0.1);
        REQUIRE(trap_manager_manager.trap_managers_sc_co[2].dwell_time == 0.1);

        // Initial watermarks, accounting for number of clock-sequence steps
        REQUIRE(trap_manager_manager.max_n_transfers == max_n_transfers * 3);

        int n_levels = max_n_transfers * 3 + 1;
        REQUIRE(trap_manager_manager.trap_managers_ic[0].n_watermarks == n_levels);
        REQUIRE(
            trap_manager_manager.trap_managers_ic[0].watermark_volumes.size() ==
            n_levels);
        REQUIRE(
            trap_manager_manager.trap_managers_ic[0].watermark_fills.size() ==
            n_levels * 2);
        REQUIRE(trap_manager_manager.trap_managers_ic[1].n_watermarks == n_levels);
        REQUIRE(
            trap_manager_manager.trap_managers_ic[1].watermark_volumes.size() ==
            n_levels);
        REQUIRE(
            trap_manager_manager.trap_managers_ic[1].watermark_fills.size() ==
            n_levels * 2);
        REQUIRE(trap_manager_manager.trap_managers_ic[2].n_watermarks == n_levels);
        REQUIRE(
            trap_manager_manager.trap_managers_ic[2].watermark_volumes.size() ==
            n_levels);
        REQUIRE(
            trap_manager_manager.trap_managers_ic[2].watermark_fills.size() ==
            n_levels * 2);

        n_levels = max_n_transfers * 3 * 2 + 1;
        REQUIRE(trap_manager_manager.trap_managers_sc[0].n_watermarks == n_levels);
        REQUIRE(
            trap_manager_manager.trap_managers_sc[0].watermark_volumes.size() ==
            n_levels);
        REQUIRE(
            trap_manager_manager.trap_managers_sc[0].watermark_fills.size() ==
            n_levels);
        REQUIRE(trap_manager_manager.trap_managers_sc[1].n_watermarks == n_levels);
        REQUIRE(
            trap_manager_manager.trap_managers_sc[1].watermark_volumes.size() ==
            n_levels);
        REQUIRE(
            trap_manager_manager.trap_managers_sc[1].watermark_fills.size() ==
            n_levels);
        REQUIRE(trap_manager_manager.trap_managers_sc[2].n_watermarks == n_levels);
        REQUIRE(
            trap_manager_manager.trap_managers_sc[2].watermark_volumes.size() ==
            n_levels);
        REQUIRE(
            trap_manager_manager.trap_managers_sc[2].watermark_fills.size() ==
            n_levels);

        n_levels = max_n_transfers * 3 + 1;
        REQUIRE(trap_manager_manager.trap_managers_ic_co[0].n_watermarks == n_levels);
        REQUIRE(
            trap_manager_manager.trap_managers_ic_co[0].watermark_volumes.size() ==
            n_levels);
        REQUIRE(
            trap_manager_manager.trap_managers_ic_co[0].watermark_fills.size() ==
            n_levels);
        REQUIRE(trap_manager_manager.trap_managers_ic_co[1].n_watermarks == n_levels);
        REQUIRE(
            trap_manager_manager.trap_managers_ic_co[1].watermark_volumes.size() ==
            n_levels);
        REQUIRE(
            trap_manager_manager.trap_managers_ic_co[1].watermark_fills.size() ==
            n_levels);
        REQUIRE(trap_manager_manager.trap_managers_ic_co[2].n_watermarks == n_levels);
        REQUIRE(
            trap_manager_manager.trap_managers_ic_co[2].watermark_volumes.size() ==
            n_levels);
        REQUIRE(
            trap_manager_manager.trap_managers_ic_co[2].watermark_fills.size() ==
            n_levels);

        n_levels = max_n_transfers * 3 * 2 + 1;
        REQUIRE(trap_manager_manager.trap_managers_sc_co[0].n_watermarks == n_levels);
        REQUIRE(
            trap_manager_manager.trap_managers_sc_co[0].watermark_volumes.size() ==
            n_levels);
        REQUIRE(
            trap_manager_manager.trap_managers_sc_co[0].watermark_fills.size() ==
            n_levels);
        REQUIRE(trap_manager_manager.trap_managers_sc_co[1].n_watermarks == n_levels);
        REQUIRE(
            trap_manager_manager.trap_managers_sc_co[1].watermark_volumes.size() ==
            n_levels);
        REQUIRE(
            trap_manager_manager.trap_managers_sc_co[1].watermark_fills.size() ==
            n_levels);
        REQUIRE(trap_manager_manager.trap_managers_sc_co[2].n_watermarks == n_levels);
        REQUIRE(
            trap_manager_manager.trap_managers_sc_co[2].watermark_volumes.size() ==
            n_levels);
        REQUIRE(
            trap_manager_manager.trap_managers_sc_co[2].watermark_fills.size() ==
            n_levels);

        // Trap densities modified by the CCD's fraction_of_traps_per_phase
        answer = {trap_1.density * 0.5, trap_2.density * 0.5};
        test.assign(
            std::begin(trap_manager_manager.trap_managers_ic[0].trap_densities),
            std::end(trap_manager_manager.trap_managers_ic[0].trap_densities));
        REQUIRE_THAT(test, Catch::Approx(answer));
        answer = {trap_1.density * 0.25, trap_2.density * 0.25};
        test.assign(
            std::begin(trap_manager_manager.trap_managers_ic[1].trap_densities),
            std::end(trap_manager_manager.trap_managers_ic[1].trap_densities));
        REQUIRE_THAT(test, Catch::Approx(answer));
        answer = {trap_1.density * 0.25, trap_2.density * 0.25};
        test.assign(
            std::begin(trap_manager_manager.trap_managers_ic[2].trap_densities),
            std::end(trap_manager_manager.trap_managers_ic[2].trap_densities));
        REQUIRE_THAT(test, Catch::Approx(answer));

        answer = {trap_3.density * 0.5};
        test.assign(
            std::begin(trap_manager_manager.trap_managers_sc[0].trap_densities),
            std::end(trap_manager_manager.trap_managers_sc[0].trap_densities));
        REQUIRE_THAT(test, Catch::Approx(answer));
        answer = {trap_3.density * 0.25};
        test.assign(
            std::begin(trap_manager_manager.trap_managers_sc[1].trap_densities),
            std::end(trap_manager_manager.trap_managers_sc[1].trap_densities));
        REQUIRE_THAT(test, Catch::Approx(answer));
        answer = {trap_3.density * 0.25};
        test.assign(
            std::begin(trap_manager_manager.trap_managers_sc[2].trap_densities),
            std::end(trap_manager_manager.trap_managers_sc[2].trap_densities));
        REQUIRE_THAT(test, Catch::Approx(answer));

        answer = {trap_4.density * 0.5};
        test.assign(
            std::begin(trap_manager_manager.trap_managers_ic_co[0].trap_densities),
            std::end(trap_manager_manager.trap_managers_ic_co[0].trap_densities));
        REQUIRE_THAT(test, Catch::Approx(answer));
        answer = {trap_4.density * 0.25};
        test.assign(
            std::begin(trap_manager_manager.trap_managers_ic_co[1].trap_densities),
            std::end(trap_manager_manager.trap_managers_ic_co[1].trap_densities));
        REQUIRE_THAT(test, Catch::Approx(answer));
        answer = {trap_4.density * 0.25};
        test.assign(
            std::begin(trap_manager_manager.trap_managers_ic_co[2].trap_densities),
            std::end(trap_manager_manager.trap_managers_ic_co[2].trap_densities));
        REQUIRE_THAT(test, Catch::Approx(answer));

        answer = {trap_5.density * 0.5};
        test.assign(
            std::begin(trap_manager_manager.trap_managers_sc_co[0].trap_densities),
            std::end(trap_manager_manager.trap_managers_sc_co[0].trap_densities));
        REQUIRE_THAT(test, Catch::Approx(answer));
        answer = {trap_5.density * 0.25};
        test.assign(
            std::begin(trap_manager_manager.trap_managers_sc_co[1].trap_densities),
            std::end(trap_manager_manager.trap_managers_sc_co[1].trap_densities));
        REQUIRE_THAT(test, Catch::Approx(answer));
        answer = {trap_5.density * 0.25};
        test.assign(
            std::begin(trap_manager_manager.trap_managers_sc_co[2].trap_densities),
            std::end(trap_manager_manager.trap_managers_sc_co[2].trap_densities));
        REQUIRE_THAT(test, Catch::Approx(answer));

        // Instant-capture continuum interpolation tables
        // Limits set by each phase's dwell time
        REQUIRE(
            trap_manager_manager.trap_managers_ic_co[0].traps[0].time_min ==
            dwell_times[0]);
        REQUIRE(
            trap_manager_manager.trap_managers_ic_co[1].traps[0].time_min ==
            dwell_times[1]);
        REQUIRE(
            trap_manager_manager.trap_managers_ic_co[2].traps[0].time_min ==
            dwell_times[2]);
        REQUIRE(
            trap_manager_manager.trap_managers_ic_co[0].traps[0].time_max ==
            max_n_transfers * dwell_times.size() * dwell_times[0]);
        REQUIRE(
            trap_manager_manager.trap_managers_ic_co[1].traps[0].time_max ==
            max_n_transfers * dwell_times.size() * dwell_times[1]);
        REQUIRE(
            trap_manager_manager.trap_managers_ic_co[2].traps[0].time_max ==
            max_n_transfers * dwell_times.size() * dwell_times[2]);
        // Longer dwell time sets smaller tabulated fill fractions
        REQUIRE(
            trap_manager_manager.trap_managers_ic_co[0]
                .traps[0]
                .fill_fraction_table[0] < trap_manager_manager.trap_managers_ic_co[1]
                                              .traps[0]
                                              .fill_fraction_table[0]);
        REQUIRE(
            trap_manager_manager.trap_managers_ic_co[1]
                .traps[0]
                .fill_fraction_table[0] == trap_manager_manager.trap_managers_ic_co[2]
                                               .traps[0]
                                               .fill_fraction_table[0]);

        // Slow-capture continuum interpolation tables
        // Limits set by each phase's dwell time
        REQUIRE(
            trap_manager_manager.trap_managers_sc_co[0].traps[0].time_min ==
            dwell_times[0] / 30);
        REQUIRE(
            trap_manager_manager.trap_managers_sc_co[1].traps[0].time_min ==
            dwell_times[1] / 30);
        REQUIRE(
            trap_manager_manager.trap_managers_sc_co[2].traps[0].time_min ==
            dwell_times[2] / 30);
        REQUIRE(
            trap_manager_manager.trap_managers_sc_co[0].traps[0].time_max ==
            max_n_transfers * dwell_times.size() * dwell_times[0]);
        REQUIRE(
            trap_manager_manager.trap_managers_sc_co[1].traps[0].time_max ==
            max_n_transfers * dwell_times.size() * dwell_times[1]);
        REQUIRE(
            trap_manager_manager.trap_managers_sc_co[2].traps[0].time_max ==
            max_n_transfers * dwell_times.size() * dwell_times[2]);
        // Longer dwell time sets smaller tabulated fill fractions
        REQUIRE(
            trap_manager_manager.trap_managers_sc_co[0]
                .traps[0]
                .fill_fraction_table[0] < trap_manager_manager.trap_managers_sc_co[1]
                                              .traps[0]
                                              .fill_fraction_table[0]);
        REQUIRE(
            trap_manager_manager.trap_managers_sc_co[1]
                .traps[0]
                .fill_fraction_table[0] == trap_manager_manager.trap_managers_sc_co[2]
                                               .traps[0]
                                               .fill_fraction_table[0]);
    }

    SECTION("Store, reset, and restore all trap states") {
        std::valarray<TrapInstantCapture> traps_ic = {trap_1, trap_2};
        std::valarray<TrapSlowCapture> traps_sc = {trap_3};
        std::valarray<TrapInstantCaptureContinuum> traps_ic_co = {trap_4};
        std::valarray<TrapSlowCaptureContinuum> traps_sc_co = {trap_5};
        std::valarray<double> dwell_times = {0.8, 0.1, 0.1};
        ROE roe(dwell_times);
        CCDPhase ccd_phase_2(2e4, 0.0, 0.8);
        std::valarray<CCDPhase> phases = {ccd_phase, ccd_phase_2, ccd_phase_2};
        std::valarray<double> fractions = {0.5, 0.25, 0.25};
        CCD ccd(phases, fractions);

        max_n_transfers = 3;
        TrapManagerManager t_m_m(
            traps_ic, traps_sc, traps_ic_co, traps_sc_co, max_n_transfers, ccd,
            roe.dwell_times);

        // Account for the number of clock-sequence steps for the maximum transfers
        std::valarray<double> volumes_ic = {0.3, 0.5, 0.2, 0.1, 0.0,
                                            0.0, 0.0, 0.0, 0.0, 0.0};
        std::valarray<double> fills_ic = {
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
            // clang-format on
        };
        std::valarray<double> volumes_sc = {0.3, 0.5, 0.2, 0.1, 0.0, 0.0, 0.0,
                                            0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                                            0.0, 0.0, 0.0, 0.0, 0.0};
        std::valarray<double> fills_sc = {0.3, 0.7, 0.3, 0.1, 0.0, 0.0, 0.0,
                                          0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                                          0.0, 0.0, 0.0, 0.0, 0.0};
        std::valarray<double> volumes_co = {0.3, 0.5, 0.2, 0.1, 0.0,
                                            0.0, 0.0, 0.0, 0.0, 0.0};
        std::valarray<double> fills_co = {0.4, 0.8, 0.4, 0.2, 0.0,
                                          0.0, 0.0, 0.0, 0.0, 0.0};
        std::valarray<double> volumes_sc_co = {0.3, 0.5, 0.2, 0.1, 0.0, 0.0, 0.0,
                                               0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                                               0.0, 0.0, 0.0, 0.0, 0.0};
        std::valarray<double> fills_sc_co = {0.3, 0.7, 0.3, 0.1, 0.0, 0.0, 0.0,
                                             0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                                             0.0, 0.0, 0.0, 0.0, 0.0};

        // Set watermarks
        for (int phase_index = 0; phase_index < ccd.n_phases; phase_index++) {
            t_m_m.trap_managers_ic[phase_index].n_active_watermarks = 3;
            t_m_m.trap_managers_ic[phase_index].i_first_active_wmk = 1;
            t_m_m.trap_managers_ic[phase_index].watermark_volumes = volumes_ic;
            t_m_m.trap_managers_ic[phase_index].watermark_fills = fills_ic;

            t_m_m.trap_managers_sc[phase_index].n_active_watermarks = 3;
            t_m_m.trap_managers_sc[phase_index].i_first_active_wmk = 1;
            t_m_m.trap_managers_sc[phase_index].watermark_volumes = volumes_sc;
            t_m_m.trap_managers_sc[phase_index].watermark_fills = fills_sc;

            t_m_m.trap_managers_ic_co[phase_index].n_active_watermarks = 3;
            t_m_m.trap_managers_ic_co[phase_index].i_first_active_wmk = 1;
            t_m_m.trap_managers_ic_co[phase_index].watermark_volumes = volumes_co;
            t_m_m.trap_managers_ic_co[phase_index].watermark_fills = fills_co;

            t_m_m.trap_managers_sc_co[phase_index].n_active_watermarks = 3;
            t_m_m.trap_managers_sc_co[phase_index].i_first_active_wmk = 1;
            t_m_m.trap_managers_sc_co[phase_index].watermark_volumes = volumes_sc_co;
            t_m_m.trap_managers_sc_co[phase_index].watermark_fills = fills_sc_co;
        }

        // Store
        t_m_m.store_trap_states();

        for (int phase_index = 0; phase_index < ccd.n_phases; phase_index++) {
            REQUIRE(
                t_m_m.trap_managers_ic[phase_index].stored_n_active_watermarks == 3);
            REQUIRE(t_m_m.trap_managers_ic[phase_index].stored_i_first_active_wmk == 1);
            answer.assign(std::begin(volumes_ic), std::end(volumes_ic));
            test.assign(
                std::begin(
                    t_m_m.trap_managers_ic[phase_index].stored_watermark_volumes),
                std::end(t_m_m.trap_managers_ic[phase_index].stored_watermark_volumes));
            REQUIRE_THAT(test, Catch::Approx(answer));
            answer.assign(std::begin(fills_ic), std::end(fills_ic));
            test.assign(
                std::begin(t_m_m.trap_managers_ic[phase_index].stored_watermark_fills),
                std::end(t_m_m.trap_managers_ic[phase_index].stored_watermark_fills));
            REQUIRE_THAT(test, Catch::Approx(answer));

            REQUIRE(
                t_m_m.trap_managers_sc[phase_index].stored_n_active_watermarks == 3);
            REQUIRE(t_m_m.trap_managers_sc[phase_index].stored_i_first_active_wmk == 1);
            answer.assign(std::begin(volumes_sc), std::end(volumes_sc));
            test.assign(
                std::begin(
                    t_m_m.trap_managers_sc[phase_index].stored_watermark_volumes),
                std::end(t_m_m.trap_managers_sc[phase_index].stored_watermark_volumes));
            REQUIRE_THAT(test, Catch::Approx(answer));
            answer.assign(std::begin(fills_sc), std::end(fills_sc));
            test.assign(
                std::begin(t_m_m.trap_managers_sc[phase_index].stored_watermark_fills),
                std::end(t_m_m.trap_managers_sc[phase_index].stored_watermark_fills));
            REQUIRE_THAT(test, Catch::Approx(answer));

            REQUIRE(
                t_m_m.trap_managers_ic_co[phase_index].stored_n_active_watermarks == 3);
            REQUIRE(
                t_m_m.trap_managers_ic_co[phase_index].stored_i_first_active_wmk == 1);
            answer.assign(std::begin(volumes_co), std::end(volumes_co));
            test.assign(
                std::begin(
                    t_m_m.trap_managers_ic_co[phase_index].stored_watermark_volumes),
                std::end(
                    t_m_m.trap_managers_ic_co[phase_index].stored_watermark_volumes));
            REQUIRE_THAT(test, Catch::Approx(answer));
            answer.assign(std::begin(fills_co), std::end(fills_co));
            test.assign(
                std::begin(
                    t_m_m.trap_managers_ic_co[phase_index].stored_watermark_fills),
                std::end(
                    t_m_m.trap_managers_ic_co[phase_index].stored_watermark_fills));
            REQUIRE_THAT(test, Catch::Approx(answer));

            REQUIRE(
                t_m_m.trap_managers_sc_co[phase_index].stored_n_active_watermarks == 3);
            REQUIRE(
                t_m_m.trap_managers_sc_co[phase_index].stored_i_first_active_wmk == 1);
            answer.assign(std::begin(volumes_sc_co), std::end(volumes_sc_co));
            test.assign(
                std::begin(
                    t_m_m.trap_managers_sc_co[phase_index].stored_watermark_volumes),
                std::end(
                    t_m_m.trap_managers_sc_co[phase_index].stored_watermark_volumes));
            REQUIRE_THAT(test, Catch::Approx(answer));
            answer.assign(std::begin(fills_sc_co), std::end(fills_sc_co));
            test.assign(
                std::begin(
                    t_m_m.trap_managers_sc_co[phase_index].stored_watermark_fills),
                std::end(
                    t_m_m.trap_managers_sc_co[phase_index].stored_watermark_fills));
            REQUIRE_THAT(test, Catch::Approx(answer));
        }

        // Reset
        t_m_m.reset_trap_states();

        for (int phase_index = 0; phase_index < ccd.n_phases; phase_index++) {
            REQUIRE(t_m_m.trap_managers_ic[phase_index].n_active_watermarks == 0);
            REQUIRE(t_m_m.trap_managers_ic[phase_index].i_first_active_wmk == 0);
            answer = std::vector<double>(10, 0.0);
            test.assign(
                std::begin(t_m_m.trap_managers_ic[phase_index].watermark_volumes),
                std::end(t_m_m.trap_managers_ic[phase_index].watermark_volumes));
            REQUIRE_THAT(test, Catch::Approx(answer));
            answer = std::vector<double>(20, 0.0);
            test.assign(
                std::begin(t_m_m.trap_managers_ic[phase_index].watermark_fills),
                std::end(t_m_m.trap_managers_ic[phase_index].watermark_fills));
            REQUIRE_THAT(test, Catch::Approx(answer));

            REQUIRE(t_m_m.trap_managers_sc[phase_index].n_active_watermarks == 0);
            REQUIRE(t_m_m.trap_managers_sc[phase_index].i_first_active_wmk == 0);
            answer = std::vector<double>(19, 0.0);
            test.assign(
                std::begin(t_m_m.trap_managers_sc[phase_index].watermark_volumes),
                std::end(t_m_m.trap_managers_sc[phase_index].watermark_volumes));
            REQUIRE_THAT(test, Catch::Approx(answer));
            answer = std::vector<double>(19, 0.0);
            test.assign(
                std::begin(t_m_m.trap_managers_sc[phase_index].watermark_fills),
                std::end(t_m_m.trap_managers_sc[phase_index].watermark_fills));
            REQUIRE_THAT(test, Catch::Approx(answer));

            REQUIRE(t_m_m.trap_managers_ic_co[phase_index].n_active_watermarks == 0);
            REQUIRE(t_m_m.trap_managers_ic_co[phase_index].i_first_active_wmk == 0);
            answer = std::vector<double>(10, 0.0);
            test.assign(
                std::begin(t_m_m.trap_managers_ic_co[phase_index].watermark_volumes),
                std::end(t_m_m.trap_managers_ic_co[phase_index].watermark_volumes));
            REQUIRE_THAT(test, Catch::Approx(answer));
            answer = std::vector<double>(10, 0.0);
            test.assign(
                std::begin(t_m_m.trap_managers_ic_co[phase_index].watermark_fills),
                std::end(t_m_m.trap_managers_ic_co[phase_index].watermark_fills));
            REQUIRE_THAT(test, Catch::Approx(answer));

            REQUIRE(t_m_m.trap_managers_sc_co[phase_index].n_active_watermarks == 0);
            REQUIRE(t_m_m.trap_managers_sc_co[phase_index].i_first_active_wmk == 0);
            answer = std::vector<double>(19, 0.0);
            test.assign(
                std::begin(t_m_m.trap_managers_sc_co[phase_index].watermark_volumes),
                std::end(t_m_m.trap_managers_sc_co[phase_index].watermark_volumes));
            REQUIRE_THAT(test, Catch::Approx(answer));
            answer = std::vector<double>(19, 0.0);
            test.assign(
                std::begin(t_m_m.trap_managers_sc_co[phase_index].watermark_fills),
                std::end(t_m_m.trap_managers_sc_co[phase_index].watermark_fills));
            REQUIRE_THAT(test, Catch::Approx(answer));
        }

        // Restore
        t_m_m.restore_trap_states();

        for (int phase_index = 0; phase_index < ccd.n_phases; phase_index++) {
            REQUIRE(t_m_m.trap_managers_ic[phase_index].n_active_watermarks == 3);
            REQUIRE(t_m_m.trap_managers_ic[phase_index].i_first_active_wmk == 1);
            answer.assign(std::begin(volumes_ic), std::end(volumes_ic));
            test.assign(
                std::begin(t_m_m.trap_managers_ic[phase_index].watermark_volumes),
                std::end(t_m_m.trap_managers_ic[phase_index].watermark_volumes));
            REQUIRE_THAT(test, Catch::Approx(answer));
            answer.assign(std::begin(fills_ic), std::end(fills_ic));
            test.assign(
                std::begin(t_m_m.trap_managers_ic[phase_index].watermark_fills),
                std::end(t_m_m.trap_managers_ic[phase_index].watermark_fills));
            REQUIRE_THAT(test, Catch::Approx(answer));

            REQUIRE(t_m_m.trap_managers_sc[phase_index].n_active_watermarks == 3);
            REQUIRE(t_m_m.trap_managers_sc[phase_index].i_first_active_wmk == 1);
            answer.assign(std::begin(volumes_sc), std::end(volumes_sc));
            test.assign(
                std::begin(t_m_m.trap_managers_sc[phase_index].watermark_volumes),
                std::end(t_m_m.trap_managers_sc[phase_index].watermark_volumes));
            REQUIRE_THAT(test, Catch::Approx(answer));
            answer.assign(std::begin(fills_sc), std::end(fills_sc));
            test.assign(
                std::begin(t_m_m.trap_managers_sc[phase_index].watermark_fills),
                std::end(t_m_m.trap_managers_sc[phase_index].watermark_fills));
            REQUIRE_THAT(test, Catch::Approx(answer));

            REQUIRE(t_m_m.trap_managers_ic_co[phase_index].n_active_watermarks == 3);
            REQUIRE(t_m_m.trap_managers_ic_co[phase_index].i_first_active_wmk == 1);
            answer.assign(std::begin(volumes_co), std::end(volumes_co));
            test.assign(
                std::begin(t_m_m.trap_managers_ic_co[phase_index].watermark_volumes),
                std::end(t_m_m.trap_managers_ic_co[phase_index].watermark_volumes));
            REQUIRE_THAT(test, Catch::Approx(answer));
            answer.assign(std::begin(fills_co), std::end(fills_co));
            test.assign(
                std::begin(t_m_m.trap_managers_ic_co[phase_index].watermark_fills),
                std::end(t_m_m.trap_managers_ic_co[phase_index].watermark_fills));
            REQUIRE_THAT(test, Catch::Approx(answer));

            REQUIRE(t_m_m.trap_managers_sc_co[phase_index].n_active_watermarks == 3);
            REQUIRE(t_m_m.trap_managers_sc_co[phase_index].i_first_active_wmk == 1);
            answer.assign(std::begin(volumes_sc_co), std::end(volumes_sc_co));
            test.assign(
                std::begin(t_m_m.trap_managers_sc_co[phase_index].watermark_volumes),
                std::end(t_m_m.trap_managers_sc_co[phase_index].watermark_volumes));
            REQUIRE_THAT(test, Catch::Approx(answer));
            answer.assign(std::begin(fills_sc_co), std::end(fills_sc_co));
            test.assign(
                std::begin(t_m_m.trap_managers_sc_co[phase_index].watermark_fills),
                std::end(t_m_m.trap_managers_sc_co[phase_index].watermark_fills));
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
            std::valarray<TrapInstantCapture>{trap_1, trap_2}, 4, ccd_phase,
            dwell_time);
        trap_manager.setup();

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
            std::valarray<TrapInstantCapture>{trap_1}, 4, ccd_phase, dwell_time);
        trap_manager.setup();
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
            std::valarray<TrapInstantCapture>{trap_1}, 4, ccd_phase, 2.0);
        trap_manager.setup();
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
            std::valarray<TrapInstantCapture>{trap}, 4, ccd_phase, 0.5);
        trap_manager.setup();
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
            std::valarray<TrapInstantCapture>{trap_1, trap_2}, 4, ccd_phase,
            dwell_time);
        trap_manager.setup();
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
            std::valarray<TrapInstantCapture>{trap_1, trap_2}, 4, ccd_phase,
            dwell_time);
        trap_manager.setup();
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

TEST_CASE("Test instant-capture traps: simple capture", "[trap_managers]") {
    // Base cases: i_first_active_wmk = 0, enough > 1

    TrapInstantCapture trap_1(10.0, -1.0 / log(0.5));
    TrapInstantCapture trap_2(8.0, -1.0 / log(0.2));

    double n_electrons_captured;
    std::vector<double> test, answer;

    SECTION("Multiple traps capture, first watermark") {
        TrapManagerInstantCapture trap_manager(
            std::valarray<TrapInstantCapture>{trap_1, trap_2}, 5, ccd_phase,
            dwell_time);
        trap_manager.setup();
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
            std::valarray<TrapInstantCapture>{trap_1, trap_2}, 5, ccd_phase,
            dwell_time);
        trap_manager.setup();
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
            std::valarray<TrapInstantCapture>{trap_1, trap_2}, 5, ccd_phase,
            dwell_time);
        trap_manager.setup();
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
            std::valarray<TrapInstantCapture>{trap_1}, 5, ccd_phase, dwell_time);
        trap_manager.setup();
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
            std::valarray<TrapInstantCapture>{trap_1, trap_2}, 5, ccd_phase,
            dwell_time);
        trap_manager.setup();
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
            std::valarray<TrapInstantCapture>{trap_1, trap_2}, 5, ccd_phase,
            dwell_time);
        trap_manager.setup();
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
            std::valarray<TrapInstantCapture>{trap_1, trap_2}, 5, ccd_phase,
            dwell_time);
        trap_manager.setup();
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
            std::valarray<TrapInstantCapture>{trap_1, trap_2}, 5, ccd_phase,
            dwell_time);
        trap_manager.setup();
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

    SECTION("Cloud volume above full well volume") {
        TrapManagerInstantCapture trap_manager(
            std::valarray<TrapInstantCapture>{trap_1, trap_2}, 5, ccd_phase,
            dwell_time);
        trap_manager.setup();
        trap_manager.n_active_watermarks = 1;
        trap_manager.watermark_volumes = {0.5, 0.0, 0.0, 0.0, 0.0, 0.0};
        trap_manager.watermark_fills = {
            // clang-format off
            0.8 * 10, 0.7 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            // clang-format on
        };
        n_electrons_captured = trap_manager.n_electrons_captured(2e4);

        REQUIRE(
            n_electrons_captured ==
            Approx((0.5 * 0.2 + 0.5 * 1.0) * 10.0 + (0.5 * 0.3 + 0.5 * 1.0) * 8.0));
        REQUIRE(trap_manager.n_active_watermarks == 1);
        REQUIRE(trap_manager.i_first_active_wmk == 0);

        answer = {1.0, 0.0, 0.0, 0.0, 0.0, 0.0};
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

        // Release then another over-full capture
        trap_manager.watermark_fills = {
            // clang-format off
            0.8 * 10, 0.7 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            0.0 * 10, 0.0 * 8,
            // clang-format on
        };

        n_electrons_captured = trap_manager.n_electrons_captured(2e4);

        REQUIRE(n_electrons_captured == Approx(1.0 * 0.2 * 10.0 + 1.0 * 0.3 * 8.0));
        REQUIRE(trap_manager.n_active_watermarks == 1);
        REQUIRE(trap_manager.i_first_active_wmk == 0);

        answer = {1.0, 0.0, 0.0, 0.0, 0.0, 0.0};
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
}

TEST_CASE("Test instant-capture traps: other capture", "[trap_managers]") {
    // More complicated cases: i_first_active_wmk != 0 and/or enough < 1

    TrapInstantCapture trap_1(10.0, -1.0 / log(0.5));
    TrapInstantCapture trap_2(8.0, -1.0 / log(0.2));

    double n_electrons_captured;
    std::vector<double> test, answer;

    SECTION("Capture, non-zero first active watermark, cloud below watermarks") {
        TrapManagerInstantCapture trap_manager(
            std::valarray<TrapInstantCapture>{trap_1, trap_2}, 5, ccd_phase,
            dwell_time);
        trap_manager.setup();
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
            std::valarray<TrapInstantCapture>{trap_1, trap_2}, 5, ccd_phase,
            dwell_time);
        trap_manager.setup();
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
            std::valarray<TrapInstantCapture>{trap_1}, 5, ccd_phase_2, dwell_time);
        trap_manager.setup();
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
            std::valarray<TrapInstantCapture>{trap_1, trap_2}, 5, ccd_phase_2,
            dwell_time);
        trap_manager.setup();
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
            std::valarray<TrapInstantCapture>{trap_1, trap_2}, 5, ccd_phase_2,
            dwell_time);
        trap_manager.setup();
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
            std::valarray<TrapInstantCapture>{trap_1, trap_2}, 5, ccd_phase_2,
            dwell_time);
        trap_manager.setup();
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
            std::valarray<TrapInstantCapture>{trap_1, trap_2}, 6, ccd_phase_2,
            dwell_time);
        trap_manager.setup();
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

TEST_CASE(
    "Test instant-capture traps: non-uniform volume distribution", "[trap_managers]") {
    TrapInstantCapture trap_1(10.0, -1.0 / log(0.5), 0.9, 0.9);
    TrapInstantCapture trap_2(8.0, -1.0 / log(0.2), 0.6, 0.8);

    double n_electrons_captured, n_electrons_released;
    std::vector<double> test, answer;

    SECTION("Multiple traps capture, first watermark, low cloud") {
        TrapManagerInstantCapture trap_manager(
            std::valarray<TrapInstantCapture>{trap_1, trap_2}, 5, ccd_phase,
            dwell_time);
        trap_manager.setup();
        trap_manager.n_active_watermarks = 0;
        n_electrons_captured = trap_manager.n_electrons_captured(5e3);

        REQUIRE(n_electrons_captured == 0.0);
        REQUIRE(trap_manager.n_active_watermarks == 1);
        REQUIRE(trap_manager.i_first_active_wmk == 0);

        answer = {0.5, 0.0, 0.0, 0.0, 0.0, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer));
        answer = {
            // clang-format off
            1.0 * 10 / 0.1, 1.0 * 8 / 0.3,
            0.0 * 10 / 0.1, 0.0 * 8 / 0.3,
            0.0 * 10 / 0.1, 0.0 * 8 / 0.3,
            0.0 * 10 / 0.1, 0.0 * 8 / 0.3,
            0.0 * 10 / 0.1, 0.0 * 8 / 0.3,
            0.0 * 10 / 0.1, 0.0 * 8 / 0.3,
            // clang-format on
        };
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer));
    }

    SECTION("Multiple traps capture, first watermark, middle cloud") {
        TrapManagerInstantCapture trap_manager(
            std::valarray<TrapInstantCapture>{trap_1, trap_2}, 5, ccd_phase,
            dwell_time);
        trap_manager.setup();
        trap_manager.n_active_watermarks = 0;
        n_electrons_captured = trap_manager.n_electrons_captured(7e3);

        REQUIRE(n_electrons_captured == Approx(0.0 * 10.0 / 0.1 + 0.025 * 8.0 / 0.3));
        REQUIRE(trap_manager.n_active_watermarks == 1);
        REQUIRE(trap_manager.i_first_active_wmk == 0);

        answer = {0.7, 0.0, 0.0, 0.0, 0.0, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer));
        answer = {
            // clang-format off
            1.0 * 10 / 0.1, 1.0 * 8 / 0.3,
            0.0 * 10 / 0.1, 0.0 * 8 / 0.3,
            0.0 * 10 / 0.1, 0.0 * 8 / 0.3,
            0.0 * 10 / 0.1, 0.0 * 8 / 0.3,
            0.0 * 10 / 0.1, 0.0 * 8 / 0.3,
            0.0 * 10 / 0.1, 0.0 * 8 / 0.3,
            // clang-format on
        };
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer));
    }

    SECTION("Multiple traps capture, first watermark, high cloud") {
        TrapManagerInstantCapture trap_manager(
            std::valarray<TrapInstantCapture>{trap_1, trap_2}, 5, ccd_phase,
            dwell_time);
        trap_manager.setup();
        trap_manager.n_active_watermarks = 0;
        n_electrons_captured = trap_manager.n_electrons_captured(9.5e3);

        REQUIRE(
            n_electrons_captured ==
            Approx(0.05 * 10.0 / 0.1 + (0.1 + 0.15) * 8.0 / 0.3));
        REQUIRE(trap_manager.n_active_watermarks == 1);
        REQUIRE(trap_manager.i_first_active_wmk == 0);

        answer = {0.95, 0.0, 0.0, 0.0, 0.0, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer));
        answer = {
            // clang-format off
            1.0 * 10 / 0.1, 1.0 * 8 / 0.3,
            0.0 * 10 / 0.1, 0.0 * 8 / 0.3,
            0.0 * 10 / 0.1, 0.0 * 8 / 0.3,
            0.0 * 10 / 0.1, 0.0 * 8 / 0.3,
            0.0 * 10 / 0.1, 0.0 * 8 / 0.3,
            0.0 * 10 / 0.1, 0.0 * 8 / 0.3,
            // clang-format on
        };
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer));
    }

    SECTION("Multiple traps capture, first watermark, full cloud") {
        TrapManagerInstantCapture trap_manager(
            std::valarray<TrapInstantCapture>{trap_1, trap_2}, 5, ccd_phase,
            dwell_time);
        trap_manager.setup();
        trap_manager.n_active_watermarks = 0;
        n_electrons_captured = trap_manager.n_electrons_captured(1e4);

        REQUIRE(n_electrons_captured == Approx(10.0 + 8.0));
        REQUIRE(trap_manager.n_active_watermarks == 1);
        REQUIRE(trap_manager.i_first_active_wmk == 0);

        answer = {1.0, 0.0, 0.0, 0.0, 0.0, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer));
        answer = {
            // clang-format off
            1.0 * 10 / 0.1, 1.0 * 8 / 0.3,
            0.0 * 10 / 0.1, 0.0 * 8 / 0.3,
            0.0 * 10 / 0.1, 0.0 * 8 / 0.3,
            0.0 * 10 / 0.1, 0.0 * 8 / 0.3,
            0.0 * 10 / 0.1, 0.0 * 8 / 0.3,
            0.0 * 10 / 0.1, 0.0 * 8 / 0.3,
            // clang-format on
        };
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer));
    }

    SECTION("Multiple traps release") {
        TrapManagerInstantCapture trap_manager(
            std::valarray<TrapInstantCapture>{trap_1, trap_2}, 4, ccd_phase,
            dwell_time);
        trap_manager.setup();
        trap_manager.n_active_watermarks = 3;
        trap_manager.watermark_volumes = {0.5, 0.2, 0.25, 0.0, 0.0};
        trap_manager.watermark_fills = {
            // clang-format off
            0.8 * 10 / 0.1, 0.3 * 8 / 0.3,
            0.4 * 10 / 0.1, 0.2 * 8 / 0.3,
            0.2 * 10 / 0.1, 0.1 * 8 / 0.3,
            0.0 * 10 / 0.1, 0.0 * 8 / 0.3,
            0.0 * 10 / 0.1, 0.0 * 8 / 0.3,
            // clang-format on
        };

        n_electrons_released = trap_manager.n_electrons_released();

        REQUIRE(
            n_electrons_released ==
            Approx(
                0.5 * (0.5 * 0.2) * 10 +
                0.8 * (0.025 / 0.3 * 0.2 + (0.075 + 0.15) / 0.3 * 0.1) * 8));
        answer = {0.5, 0.2, 0.25, 0.0, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer));
        answer = {
            // clang-format off
            0.4 * 10 / 0.1, 0.06 * 8 / 0.3,
            0.2 * 10 / 0.1, 0.04 * 8 / 0.3,
            0.1 * 10 / 0.1, 0.02 * 8 / 0.3,
            0.0 * 10 / 0.1, 0.0 * 8 / 0.3,
            0.0 * 10 / 0.1, 0.0 * 8 / 0.3,
            // clang-format on
        };
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer));
    }
}

TEST_CASE("Test slow-capture traps: release and capture", "[trap_managers]") {
    TrapSlowCapture trap_1(10.0, -1.0 / log(0.5), 0.3);
    TrapSlowCapture trap_2(8.0, -1.0 / log(0.2), 0.2);

    double n_released_and_captured;
    std::vector<double> test, answer;

    SECTION("Empty traps, no cloud") {
        TrapManagerSlowCapture trap_manager(
            std::valarray<TrapSlowCapture>{trap_1, trap_2}, 3, ccd_phase, dwell_time);
        trap_manager.setup();

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
            std::valarray<TrapSlowCapture>{trap_1, trap_2}, 3, ccd_phase, dwell_time);
        trap_manager.setup();
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
            std::valarray<TrapSlowCapture>{trap_1, trap_2}, 3, ccd_phase, dwell_time);
        trap_manager.setup();
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
            std::valarray<TrapSlowCapture>{trap_1, trap_2}, 3, ccd_phase, dwell_time);
        trap_manager.setup();
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
            std::valarray<TrapSlowCapture>{trap_1, trap_2}, 3, ccd_phase, dwell_time);
        trap_manager.setup();
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
            std::valarray<TrapSlowCapture>{trap_1}, 5, ccd_phase, dwell_time);
        trap_manager.setup();
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
            std::valarray<TrapSlowCapture>{trap_1, trap_2}, 3, ccd_phase, dwell_time);
        trap_manager.setup();
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
            std::valarray<TrapSlowCapture>{trap_1, trap_2}, 3, ccd_phase, dwell_time);
        trap_manager.setup();
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
            std::valarray<TrapSlowCapture>{trap_1, trap_2}, 3, ccd_phase, dwell_time);
        trap_manager.setup();
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
            std::valarray<TrapSlowCapture>{trap_1}, 3, ccd_phase_2, dwell_time);
        trap_manager.setup();
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
            std::valarray<TrapSlowCapture>{trap_1, trap_2}, 3, ccd_phase_2, dwell_time);
        trap_manager.setup();
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

TEST_CASE("Test (narrow) instant-capture continuum traps: release", "[trap_managers]") {
    TrapInstantCaptureContinuum trap_1(10.0, -1.0 / log(0.5), 0.01);
    TrapInstantCaptureContinuum trap_2(8.0, -1.0 / log(0.2), 0.01);

    double n_electrons_released;
    std::vector<double> test, answer;

    SECTION("Empty release") {
        TrapManagerInstantCaptureContinuum trap_manager(
            std::valarray<TrapInstantCaptureContinuum>{trap_1, trap_2}, 4, ccd_phase,
            dwell_time);
        trap_manager.setup();

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
            0.0, 0.0,
            // clang-format on
        };
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer));
    }

    SECTION("Multiple traps release") {
        TrapManagerInstantCaptureContinuum trap_manager(
            std::valarray<TrapInstantCaptureContinuum>{trap_1, trap_2}, 4, ccd_phase,
            dwell_time);
        // Reduce time_min to get high test fill fractions within the table
        trap_manager.time_min /= 10.0;
        trap_manager.setup();
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

        REQUIRE(n_electrons_released == Approx(2.5 + 1.28).epsilon(1e-3));
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
        REQUIRE_THAT(test, Catch::Approx(answer).epsilon(1e-3));
    }

    SECTION("Multiple traps release, non-zero first active watermark") {
        TrapManagerInstantCaptureContinuum trap_manager(
            std::valarray<TrapInstantCaptureContinuum>{trap_1, trap_2}, 4, ccd_phase,
            dwell_time);
        // Reduce time_min to get high test fill fractions within the table
        trap_manager.time_min /= 10.0;
        trap_manager.setup();
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

        REQUIRE(n_electrons_released == Approx(2.5 + 1.28).epsilon(1e-3));
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
        REQUIRE_THAT(test, Catch::Approx(answer).epsilon(1e-3));
    }
}

TEST_CASE("Test instant-capture continuum traps: simple capture", "[trap_managers]") {
    // Base cases: i_first_active_wmk = 0, enough > 1

    // Identical to instant-capture traps

    TrapInstantCaptureContinuum trap_1(10.0, -1.0 / log(0.5), 0.1);
    TrapInstantCaptureContinuum trap_2(8.0, -1.0 / log(0.2), 1.0);

    double n_electrons_captured;
    std::vector<double> test, answer;

    SECTION("Multiple traps capture, first watermark") {
        TrapManagerInstantCaptureContinuum trap_manager(
            std::valarray<TrapInstantCaptureContinuum>{trap_1, trap_2}, 5, ccd_phase,
            dwell_time);
        trap_manager.setup();
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
        TrapManagerInstantCaptureContinuum trap_manager(
            std::valarray<TrapInstantCaptureContinuum>{trap_1, trap_2}, 5, ccd_phase,
            dwell_time);
        trap_manager.setup();
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
        TrapManagerInstantCaptureContinuum trap_manager(
            std::valarray<TrapInstantCaptureContinuum>{trap_1, trap_2}, 5, ccd_phase,
            dwell_time);
        trap_manager.setup();
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

    SECTION("Multiple traps capture, cloud between watermarks") {
        TrapManagerInstantCaptureContinuum trap_manager(
            std::valarray<TrapInstantCaptureContinuum>{trap_1, trap_2}, 5, ccd_phase,
            dwell_time);
        trap_manager.setup();
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

    SECTION("Same cloud volume as existing watermark") {
        TrapManagerInstantCaptureContinuum trap_manager(
            std::valarray<TrapInstantCaptureContinuum>{trap_1, trap_2}, 5, ccd_phase,
            dwell_time);
        trap_manager.setup();
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

TEST_CASE("Test instant-capture continuum traps: other capture", "[trap_managers]") {
    // More complicated cases: i_first_active_wmk != 0 and/or enough < 1

    // Identical to instant-capture traps

    TrapInstantCaptureContinuum trap_1(10.0, -1.0 / log(0.5), 0.1);
    TrapInstantCaptureContinuum trap_2(8.0, -1.0 / log(0.2), 1.0);

    double n_electrons_captured;
    std::vector<double> test, answer;

    SECTION("Capture, non-zero first active watermark, cloud below watermarks") {
        TrapManagerInstantCaptureContinuum trap_manager(
            std::valarray<TrapInstantCaptureContinuum>{trap_1, trap_2}, 5, ccd_phase,
            dwell_time);
        trap_manager.setup();
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
        TrapManagerInstantCaptureContinuum trap_manager(
            std::valarray<TrapInstantCaptureContinuum>{trap_1, trap_2}, 5, ccd_phase,
            dwell_time);
        trap_manager.setup();
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
        TrapManagerInstantCaptureContinuum trap_manager(
            std::valarray<TrapInstantCaptureContinuum>{trap_1}, 5, ccd_phase_2,
            dwell_time);
        trap_manager.setup();
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
        TrapManagerInstantCaptureContinuum trap_manager(
            std::valarray<TrapInstantCaptureContinuum>{trap_1, trap_2}, 5, ccd_phase_2,
            dwell_time);
        trap_manager.setup();
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
        TrapManagerInstantCaptureContinuum trap_manager(
            std::valarray<TrapInstantCaptureContinuum>{trap_1, trap_2}, 5, ccd_phase_2,
            dwell_time);
        trap_manager.setup();
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
        TrapManagerInstantCaptureContinuum trap_manager(
            std::valarray<TrapInstantCaptureContinuum>{trap_1, trap_2}, 5, ccd_phase_2,
            dwell_time);
        trap_manager.setup();
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
        TrapManagerInstantCaptureContinuum trap_manager(
            std::valarray<TrapInstantCaptureContinuum>{trap_1, trap_2}, 6, ccd_phase_2,
            dwell_time);
        trap_manager.setup();
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

TEST_CASE("Test slow-capture continuum traps: release and capture", "[trap_managers]") {
    // Duplicate tests as slow-capture traps, check for similarish results with
    // these fairly narrow traps
    TrapSlowCaptureContinuum trap_1(10.0, -1.0 / log(0.5), 0.005, 0.3);
    TrapSlowCaptureContinuum trap_2(8.0, -1.0 / log(0.2), 0.01, 0.2);

    double n_released_and_captured;
    std::vector<double> test, answer;

    SECTION("Empty traps, no cloud") {
        TrapManagerSlowCaptureContinuum trap_manager(
            std::valarray<TrapSlowCaptureContinuum>{trap_1, trap_2}, 3, ccd_phase,
            dwell_time);
        trap_manager.setup();

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
        TrapManagerSlowCaptureContinuum trap_manager(
            std::valarray<TrapSlowCaptureContinuum>{trap_1, trap_2}, 3, ccd_phase,
            dwell_time);
        trap_manager.setup();
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

        REQUIRE(n_released_and_captured == Approx(3.77631).epsilon(0.05));
        answer = {3.78e-4, 0.5 - 3.78e-4, 0.2, 0.1, 0.0, 0.0, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer).epsilon(0.05));
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
        REQUIRE_THAT(test, Catch::Approx(answer).epsilon(0.05));
    }

    SECTION("Empty traps, first cloud") {
        TrapManagerSlowCaptureContinuum trap_manager(
            std::valarray<TrapSlowCaptureContinuum>{trap_1, trap_2}, 3, ccd_phase,
            dwell_time);
        trap_manager.setup();
        trap_manager.n_active_watermarks = 0;
        n_released_and_captured = trap_manager.n_electrons_released_and_captured(6e3);

        REQUIRE(n_released_and_captured == Approx(-8.5048).epsilon(0.05));
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
        REQUIRE_THAT(test, Catch::Approx(answer).epsilon(0.05));
    }

    SECTION("Multiple traps, cloud below watermarks") {
        TrapManagerSlowCaptureContinuum trap_manager(
            std::valarray<TrapSlowCaptureContinuum>{trap_1, trap_2}, 3, ccd_phase,
            dwell_time);
        trap_manager.setup();
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

        REQUIRE(n_released_and_captured == Approx(2.53801).epsilon(0.05));
        REQUIRE(trap_manager.n_active_watermarks == 5);
        REQUIRE(trap_manager.i_first_active_wmk == 0);

        answer = {0.3, 2.758e-4, 0.2 - 2.758e-4, 0.2, 0.1, 0.0, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer).epsilon(0.05));
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
        REQUIRE_THAT(test, Catch::Approx(answer).epsilon(0.05));
    }

    SECTION("Multiple traps, cloud above watermarks") {
        TrapManagerSlowCaptureContinuum trap_manager(
            std::valarray<TrapSlowCaptureContinuum>{trap_1, trap_2}, 3, ccd_phase,
            dwell_time);
        trap_manager.setup();
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

        REQUIRE(n_released_and_captured == Approx(-4.3128).epsilon(0.05));
        REQUIRE(trap_manager.n_active_watermarks == 4);
        REQUIRE(trap_manager.i_first_active_wmk == 0);

        answer = {0.5, 0.2, 0.1, 0.1, 0.0, 0.0, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer).epsilon(0.05));
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
        REQUIRE_THAT(test, Catch::Approx(answer).epsilon(0.05));
    }

    SECTION("Single traps, cloud between watermarks") {
        TrapManagerSlowCaptureContinuum trap_manager(
            std::valarray<TrapSlowCaptureContinuum>{trap_1}, 5, ccd_phase, dwell_time);
        trap_manager.setup();
        trap_manager.n_active_watermarks = 3;
        trap_manager.watermark_volumes = {0.5, 0.2, 0.1, 0.0, 0.0, 0.0};
        trap_manager.watermark_fills = {0.8 * 10, 0.4 * 10, 0.3 * 10, 0.0, 0.0, 0.0};
        n_released_and_captured = trap_manager.n_electrons_released_and_captured(6e3);

        REQUIRE(n_released_and_captured == Approx(-0.207217).epsilon(0.05));
        REQUIRE(trap_manager.n_active_watermarks == 5);
        REQUIRE(trap_manager.i_first_active_wmk == 0);

        answer = {0.5, 0.1, 3.5e-5, 0.1 - 3.5e-5, 0.1, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer).epsilon(0.05));
        answer = {0.827356 * 10, 0.820221 * 10, 0.816654 * 10,
                  0.2 * 10,      0.15 * 10,     0.0};
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer).epsilon(0.05));
    }

    SECTION("Multiple traps, cloud between watermarks") {
        TrapManagerSlowCaptureContinuum trap_manager(
            std::valarray<TrapSlowCaptureContinuum>{trap_1, trap_2}, 3, ccd_phase,
            dwell_time);
        trap_manager.setup();
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

        REQUIRE(n_released_and_captured == Approx(-0.478162).epsilon(0.05));
        REQUIRE(trap_manager.n_active_watermarks == 5);
        REQUIRE(trap_manager.i_first_active_wmk == 0);

        answer = {0.5, 0.1, 6.7e-05, 0.1 - 6.7e-05, 0.1, 0.0, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer).epsilon(0.05));
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
        REQUIRE_THAT(test, Catch::Approx(answer).epsilon(0.05));
    }

    SECTION("Multiple traps, cloud between watermarks 2") {
        TrapManagerSlowCaptureContinuum trap_manager(
            std::valarray<TrapSlowCaptureContinuum>{trap_1, trap_2}, 3, ccd_phase,
            dwell_time);
        trap_manager.setup();
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

        REQUIRE(n_released_and_captured == Approx(-2.11119).epsilon(0.05));
        REQUIRE(trap_manager.n_active_watermarks == 6);
        REQUIRE(trap_manager.i_first_active_wmk == 0);

        answer = {0.5, 0.2, 0.05, 3.03e-05, 0.05 - 3.03e-05, 0.1, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer).epsilon(0.05));
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
        REQUIRE_THAT(test, Catch::Approx(answer).epsilon(0.05));
    }

    SECTION("Multiple traps, cloud between watermarks 3") {
        TrapManagerSlowCaptureContinuum trap_manager(
            std::valarray<TrapSlowCaptureContinuum>{trap_1, trap_2}, 3, ccd_phase,
            dwell_time);
        trap_manager.setup();
        // Extend root-finder bounds for the arbitrary test inputs
        trap_manager.max_n_transfers *= 10;
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

        REQUIRE(n_released_and_captured == Approx(-3.38401).epsilon(0.05));
        REQUIRE(trap_manager.n_active_watermarks == 6);
        REQUIRE(trap_manager.i_first_active_wmk == 0);

        answer = {0.5, 0.2, 0.1, 0.05, 8.2e-06, 0.05 - 8.2e-06, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer).epsilon(0.05));
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
        REQUIRE_THAT(test, Catch::Approx(answer).epsilon(0.05));
    }

    CCDPhase ccd_phase_2(1e4, 1e-7, 0.5);

    SECTION("Not-enough capture, first watermark") {
        TrapManagerSlowCaptureContinuum trap_manager(
            std::valarray<TrapSlowCaptureContinuum>{trap_1}, 3, ccd_phase_2,
            dwell_time);
        trap_manager.setup();
        trap_manager.n_active_watermarks = 0;
        n_released_and_captured =
            trap_manager.n_electrons_released_and_captured(2.5e-3);
        // --> cloud_fractional_volume = 4.9999e-4, enough = 0.50001

        REQUIRE(
            n_released_and_captured ==
            Approx(-4.9999e-4 * 0.50001 * 10.0).epsilon(0.05));
        REQUIRE(trap_manager.n_active_watermarks == 1);

        answer = {4.9999e-4, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer).epsilon(0.05));
        answer = {0.50001 * 10, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_fills),
            std::end(trap_manager.watermark_fills));
        REQUIRE_THAT(test, Catch::Approx(answer).epsilon(0.05));
    }

    SECTION("Not-enough capture, cloud between watermarks") {
        TrapManagerSlowCaptureContinuum trap_manager(
            std::valarray<TrapSlowCaptureContinuum>{trap_1, trap_2}, 3, ccd_phase_2,
            dwell_time);
        // Extend root-finder bounds for the arbitrary test inputs
        trap_manager.max_n_transfers *= 10;
        trap_manager.time_max *= 10;
        trap_manager.setup();
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
        // --> cloud_fractional_volume = 4.9999e-4, enough = 0.529676

        REQUIRE(n_released_and_captured == Approx(-0.0025).epsilon(0.05));
        REQUIRE(trap_manager.n_active_watermarks == 6);

        answer = {2e-4, 1e-4, 1.9999e-4, 6.74026e-05, 0.2 - 1.9999e-4 - 6.74026e-05,
                  0.1,  0.0};
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer).epsilon(0.05));
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
        REQUIRE_THAT(test, Catch::Approx(answer).epsilon(0.05));
    }
}

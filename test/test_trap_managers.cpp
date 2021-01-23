
#include <stdio.h>
#include <valarray>
#include <vector>

#include "catch2/catch.hpp"
#include "trap_managers.hpp"
#include "traps.hpp"
#include "util.hpp"

TEST_CASE("Test initialisation", "[trap_managers]") {
    Trap trap_1(1.0, 1.0, 0.0);
    Trap trap_2(2.0, 2.0, 0.0);
    TrapInstantCapture trap_3(3.0, 3.0);

    SECTION("Traps") {
        // Standard traps
        std::valarray<Trap> traps{trap_1, trap_2};
        TrapManager trap_manager(traps, 123);

        REQUIRE(trap_manager.n_traps == 2);
        REQUIRE(trap_manager.traps[0].density == trap_1.density);
        REQUIRE(trap_manager.traps[1].density == trap_2.density);

        // Instant-capture traps
        std::valarray<Trap> traps_ic{trap_3};
        TrapManagerInstantCapture trap_manager_ic(traps_ic, 123);

        REQUIRE(trap_manager_ic.n_traps == 1);
        REQUIRE(trap_manager_ic.traps[0].density == trap_3.density);
    }

    SECTION("Misc attributes") {
        // Standard traps
        TrapManager trap_manager(std::valarray<Trap>{trap_1, trap_2}, 123);

        REQUIRE(trap_manager.max_n_transfers == 123);
        REQUIRE(trap_manager.n_watermarks_per_transfer == 2);
        REQUIRE(trap_manager.empty_watermark == 0.0);
        REQUIRE(trap_manager.filled_watermark == 1.0);
        REQUIRE(trap_manager.n_active_watermarks == 0);

        // Instant-capture traps
        TrapManagerInstantCapture trap_manager_ic(std::valarray<Trap>{trap_3}, 123);

        REQUIRE(trap_manager_ic.max_n_transfers == 123);
        REQUIRE(trap_manager_ic.n_watermarks_per_transfer == 1);
        REQUIRE(trap_manager_ic.empty_watermark == 0.0);
        REQUIRE(trap_manager_ic.filled_watermark == 1.0);
        REQUIRE(trap_manager_ic.n_active_watermarks == 0);
    }

    SECTION("Initial watermarks") {
        // Standard traps
        TrapManager trap_manager(std::valarray<Trap>{trap_1, trap_2}, 3);
        trap_manager.initialise_watermarks();

        REQUIRE(trap_manager.n_watermarks == 7);
        REQUIRE(trap_manager.watermark_volumes.size() == 7);
        REQUIRE(trap_manager.watermark_fills.size() == 7 * 2);
        REQUIRE(
            trap_manager.watermark_volumes.sum() ==
            trap_manager.n_watermarks * trap_manager.empty_watermark);
        REQUIRE(
            trap_manager.watermark_fills.sum() ==
            trap_manager.n_watermarks * 2 * trap_manager.empty_watermark);

        TrapManager trap_manager_2(std::valarray<Trap>{trap_1}, 123);
        trap_manager_2.initialise_watermarks();

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
        TrapManagerInstantCapture trap_manager_ic(std::valarray<Trap>{trap_3}, 3);
        trap_manager_ic.initialise_watermarks();

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
        TrapManager trap_manager(std::valarray<Trap>{trap_1}, 2);
        trap_manager.initialise_watermarks();
        trap_manager.watermark_volumes = {0.5, 0.2, 0.1, 0.0, 0.0};
        trap_manager.watermark_fills = {0.8, 0.4, 0.2, 0.0, 0.0};
        n_trapped_electrons = trap_manager.n_trapped_electrons_from_watermarks(
            trap_manager.watermark_volumes, trap_manager.watermark_fills);

        REQUIRE(
            n_trapped_electrons ==
            (0.5 * 0.8 + 0.2 * 0.4 + 0.1 * 0.2) * trap_1.density);

        TrapManager trap_manager_2(std::valarray<Trap>{trap_1, trap_2}, 2);
        trap_manager_2.initialise_watermarks();
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

        // Instant-capture traps
        TrapManagerInstantCapture trap_manager_ic(std::valarray<Trap>{trap_3}, 3);
        trap_manager_ic.initialise_watermarks();
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
            std::valarray<Trap>{trap_3, trap_4}, 3);

        trap_manager_ic.set_fill_probabilities_from_dwell_time(1.0);

        REQUIRE(trap_manager_ic.fill_probabilities_from_empty[0] == Approx(1.0));
        REQUIRE(trap_manager_ic.fill_probabilities_from_full[0] == Approx(0.5));
        REQUIRE(trap_manager_ic.fill_probabilities_from_release[0] == Approx(0.5));

        REQUIRE(trap_manager_ic.fill_probabilities_from_empty[1] == Approx(1.0));
        REQUIRE(trap_manager_ic.fill_probabilities_from_full[1] == Approx(0.2));
        REQUIRE(trap_manager_ic.fill_probabilities_from_release[1] == Approx(0.2));

        trap_manager_ic.set_fill_probabilities_from_dwell_time(2.0);

        REQUIRE(trap_manager_ic.fill_probabilities_from_empty[0] == Approx(1.0));
        REQUIRE(trap_manager_ic.fill_probabilities_from_full[0] == Approx(0.25));
        REQUIRE(trap_manager_ic.fill_probabilities_from_release[0] == Approx(0.25));

        REQUIRE(trap_manager_ic.fill_probabilities_from_empty[1] == Approx(1.0));
        REQUIRE(trap_manager_ic.fill_probabilities_from_full[1] == Approx(0.04));
        REQUIRE(trap_manager_ic.fill_probabilities_from_release[1] == Approx(0.04));
    }

    SECTION("Watermark index above cloud") {
        TrapManagerInstantCapture trap_manager(std::valarray<Trap>{trap_1, trap_2}, 4);
        int watermark_index_above_cloud;

        // First watermark
        trap_manager.initialise_watermarks();
        trap_manager.n_active_watermarks = 0;
        watermark_index_above_cloud =
            trap_manager.watermark_index_above_cloud_from_volumes(
                trap_manager.watermark_volumes, 0.6);
        REQUIRE(watermark_index_above_cloud == 0);

        // Cloud below watermarks
        trap_manager.initialise_watermarks();
        trap_manager.n_active_watermarks = 3;
        trap_manager.watermark_volumes = {0.5, 0.2, 0.1, 0.0, 0.0};
        watermark_index_above_cloud =
            trap_manager.watermark_index_above_cloud_from_volumes(
                trap_manager.watermark_volumes, 0.3);
        REQUIRE(watermark_index_above_cloud == 0);

        // Cloud above watermarks
        trap_manager.initialise_watermarks();
        trap_manager.n_active_watermarks = 3;
        trap_manager.watermark_volumes = {0.5, 0.2, 0.1, 0.0, 0.0};
        watermark_index_above_cloud =
            trap_manager.watermark_index_above_cloud_from_volumes(
                trap_manager.watermark_volumes, 0.9);
        REQUIRE(watermark_index_above_cloud == 3);

        // Cloud between watermarks
        trap_manager.initialise_watermarks();
        trap_manager.n_active_watermarks = 3;
        trap_manager.watermark_volumes = {0.5, 0.2, 0.1, 0.0, 0.0};
        watermark_index_above_cloud =
            trap_manager.watermark_index_above_cloud_from_volumes(
                trap_manager.watermark_volumes, 0.6);
        REQUIRE(watermark_index_above_cloud == 1);
    }
}

TEST_CASE("Test instant-capture traps release", "[trap_managers]") {
    TrapInstantCapture trap_1(10.0, -1.0 / log(0.5));
    TrapInstantCapture trap_2(8.0, -1.0 / log(0.2));

    double n_electrons_released;
    std::vector<double> test, answer;

    SECTION("Empty release") {
        TrapManagerInstantCapture trap_manager(std::valarray<Trap>{trap_1, trap_2}, 4);
        trap_manager.initialise_watermarks();

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
        TrapManagerInstantCapture trap_manager(std::valarray<Trap>{trap_1}, 4);
        trap_manager.initialise_watermarks();
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
        TrapManagerInstantCapture trap_manager(std::valarray<Trap>{trap_1}, 4);
        trap_manager.initialise_watermarks();
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
        TrapManagerInstantCapture trap_manager(std::valarray<Trap>{trap}, 4);
        trap_manager.initialise_watermarks();
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
        TrapManagerInstantCapture trap_manager(std::valarray<Trap>{trap_1, trap_2}, 4);
        trap_manager.initialise_watermarks();
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
}

TEST_CASE("Test instant-capture traps capture", "[trap_managers]") {
    TrapInstantCapture trap_1(10.0, -1.0 / log(0.5));
    TrapInstantCapture trap_2(8.0, -1.0 / log(0.2));

    double n_electrons_captured;
    std::vector<double> test, answer;

    SECTION("Multiple traps capture, first watermark") {
        TrapManagerInstantCapture trap_manager(std::valarray<Trap>{trap_1, trap_2}, 5);
        trap_manager.initialise_watermarks();
        trap_manager.set_fill_probabilities_from_dwell_time(1.0);
        trap_manager.n_active_watermarks = 0;
        n_electrons_captured = trap_manager.n_electrons_captured(0.6);

        REQUIRE(n_electrons_captured == Approx(0.6 * 10.0 + 0.6 * 8.0));
        REQUIRE(trap_manager.n_active_watermarks == 1);

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
        TrapManagerInstantCapture trap_manager(std::valarray<Trap>{trap_1, trap_2}, 5);
        trap_manager.initialise_watermarks();
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
        n_electrons_captured = trap_manager.n_electrons_captured(0.3);

        REQUIRE(n_electrons_captured == Approx((0.3 * 0.2) * 10.0 + (0.3 * 0.3) * 8.0));
        REQUIRE(trap_manager.n_active_watermarks == 4);

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
        TrapManagerInstantCapture trap_manager(std::valarray<Trap>{trap_1, trap_2}, 5);
        trap_manager.initialise_watermarks();
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

        n_electrons_captured = trap_manager.n_electrons_captured(0.9);

        REQUIRE(
            n_electrons_captured ==
            Approx(
                (0.5 * 0.2 + 0.2 * 0.6 + 0.1 * 0.7 + 0.1 * 1.0) * 10.0 +
                (0.5 * 0.3 + 0.2 * 0.7 + 0.1 * 0.8 + 0.1 * 1.0) * 8.0));
        REQUIRE(trap_manager.n_active_watermarks == 1);

        answer = {0.9, 0.0, 0.0, 0.0, 0.0, 0.0};
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

    SECTION("Multiple traps capture, cloud between watermarks") {
        TrapManagerInstantCapture trap_manager(std::valarray<Trap>{trap_1, trap_2}, 5);
        trap_manager.initialise_watermarks();
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
        n_electrons_captured = trap_manager.n_electrons_captured(0.6);

        REQUIRE(
            n_electrons_captured ==
            Approx((0.5 * 0.2 + 0.1 * 0.6) * 10.0 + (0.5 * 0.3 + 0.1 * 0.7) * 8.0));
        REQUIRE(trap_manager.n_active_watermarks == 3);

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
        TrapManagerInstantCapture trap_manager(std::valarray<Trap>{trap_1, trap_2}, 5);
        trap_manager.initialise_watermarks();
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
        n_electrons_captured = trap_manager.n_electrons_captured(0.75);

        REQUIRE(
            n_electrons_captured == Approx(
                                        (0.5 * 0.2 + 0.2 * 0.6 + 0.05 * 0.7) * 10.0 +
                                        (0.5 * 0.3 + 0.2 * 0.7 + 0.05 * 0.8) * 8.0));
        REQUIRE(trap_manager.n_active_watermarks == 3);

        answer = {0.75, 0.05, 0.1, 0.0, 0.0, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer));
        answer = {
            // clang-format off
            1.0, 1.0,
            0.3, 0.2,
            0.2, 0.1,
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

    SECTION("Multiple traps capture, cloud between watermarks 3") {
        TrapManagerInstantCapture trap_manager(std::valarray<Trap>{trap_1, trap_2}, 5);
        trap_manager.initialise_watermarks();
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
        n_electrons_captured = trap_manager.n_electrons_captured(0.85);

        REQUIRE(
            n_electrons_captured ==
            Approx(
                (0.5 * 0.2 + 0.2 * 0.6 + 0.1 * 0.7 + 0.05 * 0.8) * 10.0 +
                (0.5 * 0.3 + 0.2 * 0.7 + 0.1 * 0.8 + 0.05 * 0.9) * 8.0));
        REQUIRE(trap_manager.n_active_watermarks == 2);

        answer = {0.85, 0.05, 0.0, 0.0, 0.0, 0.0};
        test.assign(
            std::begin(trap_manager.watermark_volumes),
            std::end(trap_manager.watermark_volumes));
        REQUIRE_THAT(test, Catch::Approx(answer));
        answer = {
            // clang-format off
            1.0, 1.0,
            0.2, 0.1,
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
}
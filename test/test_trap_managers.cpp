
#include <stdio.h>
#include <valarray>

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

        // Instant-capture traps
        TrapManagerInstantCapture trap_manager_ic(std::valarray<Trap>{trap_3}, 123);

        REQUIRE(trap_manager_ic.max_n_transfers == 123);
        REQUIRE(trap_manager_ic.n_watermarks_per_transfer == 1);
        REQUIRE(trap_manager_ic.empty_watermark == 0.0);
        REQUIRE(trap_manager_ic.filled_watermark == 1.0);
    }

    SECTION("Initial watermarks") {
        // Standard traps
        TrapManager trap_manager(std::valarray<Trap>{trap_1, trap_2}, 3);
        trap_manager.initialise_watermarks();

        REQUIRE(trap_manager.max_n_watermarks == 7);
        REQUIRE(trap_manager.watermarks.size() == 7);
        REQUIRE(trap_manager.watermarks[0].size() == 3);
        for (int i = 0; i < trap_manager.n_traps + 1; i++) {
            REQUIRE(
                trap_manager.watermarks[i].sum() ==
                trap_manager.max_n_watermarks * trap_manager.empty_watermark);
        }

        TrapManager trap_manager_2(std::valarray<Trap>{trap_1}, 123);
        trap_manager_2.initialise_watermarks();

        REQUIRE(trap_manager_2.max_n_watermarks == 247);
        REQUIRE(trap_manager_2.watermarks.size() == 247);
        REQUIRE(trap_manager_2.watermarks[0].size() == 2);
        for (int i = 0; i < trap_manager_2.n_traps + 1; i++) {
            REQUIRE(
                trap_manager_2.watermarks[i].sum() ==
                trap_manager_2.max_n_watermarks * trap_manager_2.empty_watermark);
        }

        // Instant-capture traps
        TrapManagerInstantCapture trap_manager_ic(std::valarray<Trap>{trap_3}, 3);
        trap_manager_ic.initialise_watermarks();

        REQUIRE(trap_manager_ic.max_n_watermarks == 4);
        REQUIRE(trap_manager_ic.watermarks.size() == 4);
        REQUIRE(trap_manager_ic.watermarks[0].size() == 2);
        for (int i = 0; i < trap_manager_ic.n_traps + 1; i++) {
            REQUIRE(
                trap_manager_ic.watermarks[i].sum() ==
                trap_manager_ic.max_n_watermarks * trap_manager_ic.empty_watermark);
        }
    }
}

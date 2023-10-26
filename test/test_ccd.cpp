
#include <valarray>

#include "catch2/catch.hpp"
#include "ccd.hpp"

TEST_CASE("Test CCDPhase", "[ccd]") {
    SECTION("Cloud fractional volume from electrons") {
        // Simple numbers
        CCDPhase ccd_phase(1e4, 0.0, 1.0, 0.0);

        REQUIRE(ccd_phase.full_well_depth == 1e4);
        REQUIRE(ccd_phase.well_notch_depth == 0.0);
        REQUIRE(ccd_phase.well_fill_power == 1.0);

        REQUIRE(ccd_phase.cloud_fractional_volume_from_electrons(0.0) == 0.0);
        REQUIRE(ccd_phase.cloud_fractional_volume_from_electrons(1e2) == 0.01);
        REQUIRE(ccd_phase.cloud_fractional_volume_from_electrons(1e3) == 0.1);
        REQUIRE(ccd_phase.cloud_fractional_volume_from_electrons(1e4) == 1.0);
        REQUIRE(ccd_phase.cloud_fractional_volume_from_electrons(1e5) == 1.0);

        // Non-unity power
        CCDPhase ccd_phase_2(1e4, 0.0, 0.8, 0.0);

        REQUIRE(ccd_phase_2.cloud_fractional_volume_from_electrons(0.0) == 0.0);
        REQUIRE(
            ccd_phase_2.cloud_fractional_volume_from_electrons(1e2) == pow(0.01, 0.8));
        REQUIRE(
            ccd_phase_2.cloud_fractional_volume_from_electrons(1e3) == pow(0.1, 0.8));
        REQUIRE(ccd_phase_2.cloud_fractional_volume_from_electrons(1e4) == 1.0);
        REQUIRE(ccd_phase_2.cloud_fractional_volume_from_electrons(1e5) == 1.0);

        // Non-zero notch
        CCDPhase ccd_phase_3(10010.0, 10.0, 1.0, 0.0);

        REQUIRE(ccd_phase_3.cloud_fractional_volume_from_electrons(0.0) == 0.0);
        REQUIRE(ccd_phase_3.cloud_fractional_volume_from_electrons(1.0) == 0.0);
        REQUIRE(ccd_phase_3.cloud_fractional_volume_from_electrons(10.0) == 0.0);
        REQUIRE(ccd_phase_3.cloud_fractional_volume_from_electrons(110.0) == 0.01);
        REQUIRE(ccd_phase_3.cloud_fractional_volume_from_electrons(1010.0) == 0.1);
        REQUIRE(ccd_phase_3.cloud_fractional_volume_from_electrons(1e4) < 1.0);
        REQUIRE(ccd_phase_3.cloud_fractional_volume_from_electrons(1e4+10) == 1.0);
        REQUIRE(ccd_phase_3.cloud_fractional_volume_from_electrons(1e5) == 1.0);
    }
}

TEST_CASE("Test CCD", "[ccd]") {
    std::vector<double> test, answer;

    SECTION("Initialisation, single-phase and multiphase") {
        // Single phase, single-style initialisation
        CCDPhase phase(1e4, 0.0, 1.0, 0.0);
        CCD ccd(phase);

        REQUIRE(ccd.n_phases == 1);
        REQUIRE(ccd.phases[0].full_well_depth == 1e4);
        REQUIRE(ccd.phases[0].well_notch_depth == 0.0);
        REQUIRE(ccd.phases[0].well_fill_power == 1.0);
        REQUIRE(ccd.fraction_of_traps_per_phase.size() == 1);
        REQUIRE(ccd.fraction_of_traps_per_phase[0] == 1.0);

        // Single phase, multi-style initialisation
        std::valarray<double> fractions = {1.0};
        std::valarray<CCDPhase> phases = {phase};
        CCD ccd_2(phases, fractions);

        REQUIRE(ccd_2.n_phases == 1);
        REQUIRE(ccd_2.phases[0].full_well_depth == 1e4);
        REQUIRE(ccd_2.phases[0].well_notch_depth == 0.0);
        REQUIRE(ccd_2.phases[0].well_fill_power == 1.0);
        REQUIRE(ccd_2.fraction_of_traps_per_phase.size() == 1);
        REQUIRE(ccd_2.fraction_of_traps_per_phase[0] == 1.0);

        // Multiphase
        CCDPhase phase_2(2e4, 0.0, 0.8, 0.0);
        std::valarray<CCDPhase> phases_2 = {phase, phase_2, phase_2};
        std::valarray<double> fractions_2 = {0.5, 0.25, 0.25};
        CCD ccd_3(phases_2, fractions_2);

        REQUIRE(ccd_3.n_phases == 3);
        REQUIRE(ccd_3.phases[0].full_well_depth == 1e4);
        REQUIRE(ccd_3.phases[0].well_notch_depth == 0.0);
        REQUIRE(ccd_3.phases[0].well_fill_power == 1.0);
        REQUIRE(ccd_3.phases[1].full_well_depth == 2e4);
        REQUIRE(ccd_3.phases[1].well_notch_depth == 0.0);
        REQUIRE(ccd_3.phases[1].well_fill_power == 0.8);
        REQUIRE(ccd_3.phases[2].full_well_depth == 2e4);
        REQUIRE(ccd_3.phases[2].well_notch_depth == 0.0);
        REQUIRE(ccd_3.phases[2].well_fill_power == 0.8);
        answer.assign(std::begin(fractions_2), std::end(fractions_2));
        test.assign(
            std::begin(ccd_3.fraction_of_traps_per_phase),
            std::end(ccd_3.fraction_of_traps_per_phase));
        REQUIRE_THAT(test, Catch::Approx(answer));
    }
}

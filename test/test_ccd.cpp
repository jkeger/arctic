
#include <valarray>

#include "catch2/catch.hpp"
#include "ccd.hpp"

TEST_CASE("Test CCD", "[ccd]") {
    SECTION("Cloud fractional volume from electrons") {
        // Simple numbers
        CCD ccd(1e4, 0.0, 1.0);

        REQUIRE(ccd.full_well_depth == 1e4);
        REQUIRE(ccd.well_notch_depth == 0.0);
        REQUIRE(ccd.well_fill_power == 1.0);

        REQUIRE(ccd.cloud_fractional_volume_from_electrons(0.0) == 0.0);
        REQUIRE(ccd.cloud_fractional_volume_from_electrons(1e2) == 0.01);
        REQUIRE(ccd.cloud_fractional_volume_from_electrons(1e3) == 0.1);
        REQUIRE(ccd.cloud_fractional_volume_from_electrons(1e4) == 1.0);
        REQUIRE(ccd.cloud_fractional_volume_from_electrons(1e5) == 1.0);

        // Non-unity power
        CCD ccd_2(1e4, 0.0, 0.8);

        REQUIRE(ccd_2.cloud_fractional_volume_from_electrons(0.0) == 0.0);
        REQUIRE(ccd_2.cloud_fractional_volume_from_electrons(1e2) == pow(0.01, 0.8));
        REQUIRE(ccd_2.cloud_fractional_volume_from_electrons(1e3) == pow(0.1, 0.8));
        REQUIRE(ccd_2.cloud_fractional_volume_from_electrons(1e4) == 1.0);
        REQUIRE(ccd_2.cloud_fractional_volume_from_electrons(1e5) == 1.0);

        // Non-zero notch
        CCD ccd_3(1e4, 10.0, 1.0);

        REQUIRE(ccd_3.cloud_fractional_volume_from_electrons(0.0) == 0.0);
        REQUIRE(ccd_3.cloud_fractional_volume_from_electrons(1.0) == 0.0);
        REQUIRE(ccd_3.cloud_fractional_volume_from_electrons(10.0) == 0.0);
        REQUIRE(ccd_3.cloud_fractional_volume_from_electrons(1e2) == 0.009);
        REQUIRE(ccd_3.cloud_fractional_volume_from_electrons(1e4) == 0.999);
        REQUIRE(ccd_3.cloud_fractional_volume_from_electrons(1e5) == 1.0);
    }
}

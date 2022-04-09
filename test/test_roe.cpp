
#include <valarray>

#include "catch2/catch.hpp"
#include "roe.hpp"
#include "util.hpp"

TEST_CASE("Test ROE", "[roe]") {
    SECTION("Initialisation and defaults") {
        ROE roe;
        REQUIRE(roe.type == roe_type_standard);
        REQUIRE(roe.n_steps == 1);
        REQUIRE(roe.dwell_times[0] == 1.0);
        REQUIRE(roe.prescan_offset == 0);
        REQUIRE(roe.overscan_start == -1);
        REQUIRE(roe.empty_traps_between_columns == true);
        REQUIRE(roe.empty_traps_for_first_transfers == false);
        REQUIRE(roe.force_release_away_from_readout == true);
        REQUIRE(roe.use_integer_express_matrix == false);

        std::valarray<double> dwell_times = {2.0};
        ROE roe_2(dwell_times);
        REQUIRE(roe_2.n_steps == 1);
        REQUIRE(roe_2.dwell_times[0] == 2.0);
        REQUIRE(roe_2.prescan_offset == 0);
        REQUIRE(roe_2.overscan_start == -1);
        REQUIRE(roe_2.empty_traps_between_columns == true);
        REQUIRE(roe_2.empty_traps_for_first_transfers == false);
        REQUIRE(roe_2.force_release_away_from_readout == true);
        REQUIRE(roe_2.use_integer_express_matrix == false);

        dwell_times = {3.0};
        ROE roe_3(dwell_times, 8, 2048, false);
        REQUIRE(roe_3.n_steps == 1);
        REQUIRE(roe_3.dwell_times[0] == 3.0);
        REQUIRE(roe_3.prescan_offset == 8);
        REQUIRE(roe_3.overscan_start == 2048);
        REQUIRE(roe_3.empty_traps_between_columns == false);
        REQUIRE(roe_3.empty_traps_for_first_transfers == false);
        REQUIRE(roe_3.force_release_away_from_readout == true);
        REQUIRE(roe_3.use_integer_express_matrix == false);

        dwell_times = {4.0};
        ROE roe_4(dwell_times, 0, -1, true, true, false, true);
        REQUIRE(roe_4.n_steps == 1);
        REQUIRE(roe_4.dwell_times[0] == 4.0);
        REQUIRE(roe_4.prescan_offset == 0);
        REQUIRE(roe_4.overscan_start == -1);
        REQUIRE(roe_4.empty_traps_between_columns == true);
        REQUIRE(roe_4.empty_traps_for_first_transfers == true);
        REQUIRE(roe_4.force_release_away_from_readout == false);
        REQUIRE(roe_4.use_integer_express_matrix == true);

        dwell_times = {0.5, 0.25, 0.25};
        ROE roe_6(dwell_times);
        REQUIRE(roe_6.n_steps == 3);
        REQUIRE(roe_6.dwell_times[0] == 0.5);
        REQUIRE(roe_6.dwell_times[1] == 0.25);
        REQUIRE(roe_6.dwell_times[2] == 0.25);
        REQUIRE(roe_6.prescan_offset == 0);
        REQUIRE(roe_6.overscan_start == -1);
        REQUIRE(roe_6.empty_traps_between_columns == true);
        REQUIRE(roe_6.empty_traps_for_first_transfers == false);
        REQUIRE(roe_6.force_release_away_from_readout == true);
        REQUIRE(roe_6.use_integer_express_matrix == false);
    }

    SECTION("Updating parameters") {
        ROE roe;
        roe.dwell_times = {0.5};
        roe.prescan_offset = 2;
        roe.overscan_start = 2066;
        roe.empty_traps_between_columns = false;
        roe.empty_traps_for_first_transfers = true;
        roe.use_integer_express_matrix = true;
        REQUIRE(roe.prescan_offset == 2);
        REQUIRE(roe.overscan_start == 2066);
        REQUIRE(roe.dwell_times[0] == 0.5);
        REQUIRE(roe.n_steps == 1);
        REQUIRE(roe.empty_traps_between_columns == false);
        REQUIRE(roe.empty_traps_for_first_transfers == true);
        REQUIRE(roe.use_integer_express_matrix == true);
    }

    SECTION("Assignment operator") {
        std::valarray<double> dwell_times = {1.0};
        ROE roe;
        roe = ROE(dwell_times, 4, 1024, false, true);
        REQUIRE(roe.type == roe_type_standard);
        REQUIRE(roe.n_steps == 1);
        REQUIRE(roe.dwell_times[0] == 1.0);
        REQUIRE(roe.prescan_offset == 4);
        REQUIRE(roe.overscan_start == 1024);
        REQUIRE(roe.empty_traps_between_columns == false);
        REQUIRE(roe.empty_traps_for_first_transfers == true);
        REQUIRE(roe.force_release_away_from_readout == true);
        REQUIRE(roe.use_integer_express_matrix == false);
    }
}

TEST_CASE("Test express matrix", "[roe]") {
    std::vector<double> test, answer;
    int n_rows = 12;
    int express = 0;
    int offset = 0;
    std::valarray<double> dwell_times = {1.0};

    SECTION("Integer express matrix, not empty for first transfers") {
        ROE roe(dwell_times, 0, -1, true, false, true, true);
        express = 1;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        answer = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
        test.assign(std::begin(roe.express_matrix), std::end(roe.express_matrix));
        REQUIRE(test == answer);
        REQUIRE(roe.n_express_passes == 1);

        express = 4;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        answer = {
            // clang-format off
            1, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
            0, 0, 0, 1, 2, 3, 3, 3, 3, 3, 3, 3,
            0, 0, 0, 0, 0, 0, 1, 2, 3, 3, 3, 3,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3,
            // clang-format on
        };
        test.assign(std::begin(roe.express_matrix), std::end(roe.express_matrix));
        REQUIRE(test == answer);
        REQUIRE(roe.n_express_passes == 4);

        express = 5;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        answer = {
            // clang-format off
            1, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
            0, 0, 0, 1, 2, 3, 3, 3, 3, 3, 3, 3,
            0, 0, 0, 0, 0, 0, 1, 2, 3, 3, 3, 3,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            // clang-format on
        };
        test.assign(std::begin(roe.express_matrix), std::end(roe.express_matrix));
        REQUIRE(test == answer);
        REQUIRE(roe.n_express_passes == 5);

        express = 12;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        answer = {
            // clang-format off
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1,
            0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1,
            0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1,
            0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
            0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
            // clang-format on
        };
        test.assign(std::begin(roe.express_matrix), std::end(roe.express_matrix));
        REQUIRE(test == answer);
        REQUIRE(roe.n_express_passes == 12);
    }

    SECTION("Offset express matrix") {
        ROE roe(dwell_times, 0, -1, true, false, true, true);
        offset = 5;

        express = 1;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        answer = {6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17};
        test.assign(std::begin(roe.express_matrix), std::end(roe.express_matrix));
        REQUIRE(test == answer);

        express = 3;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        answer = {
            // clang-format off
            6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
            0, 1, 2, 3, 4, 5, 6, 6, 6, 6, 6, 6,
            0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5,
            // clang-format on
        };
        test.assign(std::begin(roe.express_matrix), std::end(roe.express_matrix));
        REQUIRE(test == answer);
        REQUIRE(roe.n_express_passes == 3);

        express = 12;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        answer = {
            // clang-format off
            2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
            2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
            2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
            0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
            0, 0, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2,
            0, 0, 0, 0, 0, 1, 2, 2, 2, 2, 2, 2,
            0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 2, 2,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 2,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            // clang-format on
        };
        test.assign(std::begin(roe.express_matrix), std::end(roe.express_matrix));
        REQUIRE(test == answer);
        REQUIRE(roe.n_express_passes == 12);

        express = 0;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        answer = {
            // clang-format off
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1,
            0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1,
            0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1,
            0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
            0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
            // clang-format on
        };
        test.assign(std::begin(roe.express_matrix), std::end(roe.express_matrix));
        REQUIRE(test == answer);
        REQUIRE(roe.n_express_passes == 17);

        roe.empty_traps_for_first_transfers = true;
        express = 4;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        answer = {
            // clang-format off
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 3, 4,
            0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 1, 1, 2, 3, 4, 4, 4, 4, 4,
            0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            1, 2, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
            // clang-format on
        };
        test.assign(std::begin(roe.express_matrix), std::end(roe.express_matrix));
        REQUIRE(test == answer);
        REQUIRE(roe.n_express_passes == 17);
    }

    SECTION("Overscan in express matrix") {
        
        //Prescan and overscan, normal readout
        int overscan_start = 11;
        ROE roe(dwell_times, 0, overscan_start, true, false, true, true);

        express = 1;
        offset = 5;
        n_rows = 12;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        answer = {6, 7, 8, 9, 10, 10, 10, 10, 10, 10, 10, 10};
        test.assign(std::begin(roe.express_matrix), std::end(roe.express_matrix));
        REQUIRE(test == answer);

        express = 3;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        answer = {
            // clang-format off
            6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
            0, 1, 2, 3, 4, 4, 4, 4, 4, 4, 4, 4,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            // clang-format on
        };
        test.assign(std::begin(roe.express_matrix), std::end(roe.express_matrix));
        REQUIRE(test == answer);
        REQUIRE(roe.n_express_passes == 3);
        
        //Just overscan, no prescan
        offset = 0;
        express = 1;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        answer = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 10};
        test.assign(std::begin(roe.express_matrix), std::end(roe.express_matrix));
        REQUIRE(test == answer);

        express = 3;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        answer = {
            // clang-format off
            1, 2, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4,
            0, 0, 0, 0, 1, 2, 3, 4, 4, 4, 4, 4,
            0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 2,
            // clang-format on
        };
        test.assign(std::begin(roe.express_matrix), std::end(roe.express_matrix));
        REQUIRE(test == answer);
        REQUIRE(roe.n_express_passes == 3);

        
        //Prescan and overscan, charge injection readout
        ROEChargeInjection roeci(dwell_times, 0, overscan_start, true, true, true);

        express = 1;
        offset = 5;
        n_rows = 13;
        roeci.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        answer = {10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10};
        test.assign(std::begin(roeci.express_matrix), std::end(roeci.express_matrix));
        REQUIRE(test == answer);
        REQUIRE(roeci.n_express_passes == 1);

        express = 3;
        roeci.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        answer = {
            // clang-format off
            4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
            4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
            2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
            // clang-format on
        };
        test.assign(std::begin(roeci.express_matrix), std::end(roeci.express_matrix));
        REQUIRE(test == answer);
        REQUIRE(roeci.n_express_passes == 3);
        
        //Just overscan, no prescan
        offset = 0;
        express = 1;
        roeci.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        answer = {10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10};
        test.assign(std::begin(roeci.express_matrix), std::end(roeci.express_matrix));
        REQUIRE(test == answer);

        express = 3;
        roeci.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        answer = {
            // clang-format off
            4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
            4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
            2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
            // clang-format on
        };
        test.assign(std::begin(roeci.express_matrix), std::end(roeci.express_matrix));
        REQUIRE(test == answer);
        REQUIRE(roeci.n_express_passes == 3);        
    
    }


    SECTION("Non-integer express matrix, not empty for first transfers") {
        ROE roe(dwell_times, 0, -1, true, true, true, false);

        express = 4;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        answer = {
            // clang-format off
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 1, 0.75, 1.75, 2.75,
            0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 1, 0.5, 1.5, 2.5, 2.75, 2.75, 2.75,
            0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 1, 0.25, 1.25, 2.25, 2.75, 2.75, 2.75, 2.75, 2.75, 2.75,
            0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            1, 1, 2, 2.75, 2.75, 2.75, 2.75, 2.75, 2.75, 2.75, 2.75, 2.75,
            // clang-format on
        };
        test.assign(std::begin(roe.express_matrix), std::end(roe.express_matrix));
        REQUIRE(test == answer);
        REQUIRE(roe.n_express_passes == 12);

        // Unchanged for not empty_traps_for_first_transfers
        roe.empty_traps_for_first_transfers = false;
        express = 1;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        answer = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
        test.assign(std::begin(roe.express_matrix), std::end(roe.express_matrix));
        REQUIRE(test == answer);
        REQUIRE(roe.n_express_passes == 1);

        // Unchanged for no express
        roe.empty_traps_for_first_transfers = true;
        express = 12;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        answer = {
            // clang-format off
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
            0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
            0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
            0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1,
            0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1,
            0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1,
            0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            // clang-format on
            //1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            //0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            //0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            //0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            //0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1,
            //0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1,
            //0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1,
            //0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
            //0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
            //0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
            //0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
            //0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        };
        test.assign(std::begin(roe.express_matrix), std::end(roe.express_matrix));
        REQUIRE(test == answer);
        REQUIRE(roe.n_express_passes == 12);
    }

    SECTION("Empty traps for first transfers") {
        ROE roe(dwell_times, 0, -1, true, true, true, true);

        express = 1;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        answer = {
            // clang-format off
            0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,
            0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0,
            0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,
            0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  0,
            0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  0,  0,
            0,  0,  0,  0,  0,  0,  1,  0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,  1,  0,  0,  0,  0,  0,  0,
            0,  0,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,
            0,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,
            0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,
            0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
            1,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11,
            // clang-format on
        };
        test.assign(std::begin(roe.express_matrix), std::end(roe.express_matrix));
        REQUIRE(test == answer);
        REQUIRE(roe.n_express_passes == 12);

        express = 4;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        answer = {
            // clang-format off
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2,
            0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 1, 1, 2, 3, 3, 3,
            0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 1, 1, 2, 3, 3, 3, 3, 3, 3,
            0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            1, 1, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3,
            // clang-format on
        };
        test.assign(std::begin(roe.express_matrix), std::end(roe.express_matrix));
        REQUIRE(test == answer);
        REQUIRE(roe.n_express_passes == 12);

        express = 12;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        answer = {
            // clang-format off
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
            0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
            0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
            0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1,
            0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1,
            0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1,
            0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            // clang-format on
            //1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            //0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            //0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            //0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            //0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1,
            //0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1,
            //0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1,
            //0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
            //0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
            //0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
            //0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
            //0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
        };
        test.assign(std::begin(roe.express_matrix), std::end(roe.express_matrix));
        REQUIRE(test == answer);
        REQUIRE(roe.n_express_passes == 12);
    }


    SECTION("Check always sums to n_transfers") {
        std::valarray<int> rows = {5, 7, 17};
        std::valarray<int> expresses = {0, 1, 2, 7};
        std::valarray<int> offsets = {0, 1, 13};
        std::valarray<bool> integers = {true, false};
        std::valarray<bool> emptys = {true, false};
        int n_passes;
        ROE roe;

        for (int i_rows = 0; i_rows < rows.size(); i_rows++) {
            n_rows = rows[i_rows];

            for (int i_express = 0; i_express < expresses.size(); i_express++) {
                express = expresses[i_express];

                for (int i_offset = 0; i_offset < offsets.size(); i_offset++) {
                    offset = offsets[i_offset];

                    for (int i_integer = 0; i_integer < integers.size(); i_integer++) {
                        roe.use_integer_express_matrix = integers[i_integer];

                        for (int i_empty = 0; i_empty < emptys.size(); i_empty++) {
                            roe.empty_traps_for_first_transfers = emptys[i_empty];

                            roe.set_express_matrix_from_rows_and_express(
                                n_rows, express, offset);

                            n_passes = roe.express_matrix.size() / n_rows;

                            // Check each pixel is moved the correct number of
                            // times to reach the readout from its position
                            for (int row_index = 0; row_index < n_rows; row_index++) {
                                std::valarray<double> tmp_col =
                                    roe.express_matrix[std::slice(
                                        row_index, n_passes, n_rows)];

                                REQUIRE(round(tmp_col.sum()) == 1 + row_index + offset);
                            }
                        }
                    }
                }
            }
        }
    }
}

TEST_CASE("Test store trap states matrix", "[roe]") {
    std::vector<bool> test, answer;
    int n_rows = 12;
    int express = 0;
    int offset = 0;
    std::valarray<double> dwell_times = {1.0};

    SECTION("Empty traps for first transfers: no need to store trap states") {
        ROE roe(dwell_times, 0, -1, true, true, true, false);
        express = 1;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        roe.set_store_trap_states_matrix();
        answer = {
            // clang-format off
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            // clang-format on
        };
        test.assign(
            std::begin(roe.store_trap_states_matrix),
            std::end(roe.store_trap_states_matrix));
        REQUIRE(test == answer);

        express = 4;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        roe.set_store_trap_states_matrix();
        answer = {
            // clang-format off
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            // clang-format on
        };
        test.assign(
            std::begin(roe.store_trap_states_matrix),
            std::end(roe.store_trap_states_matrix));
        REQUIRE(test == answer);

        express = 12;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        roe.set_store_trap_states_matrix();
        answer = {
            // clang-format off
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            // clang-format on
        };
        test.assign(
            std::begin(roe.store_trap_states_matrix),
            std::end(roe.store_trap_states_matrix));
        REQUIRE(test == answer);
    }

    SECTION("Not empty traps for first transfers") {
        // Store on the pixel before where the next express pass will begin, so
        // that the trap states are appropriate for continuing in the next pass
        ROE roe(dwell_times, 0, -1, true, false, true, false);

        // But no need for express = 1
        express = 1;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        roe.set_store_trap_states_matrix();
        answer = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        test.assign(
            std::begin(roe.store_trap_states_matrix),
            std::end(roe.store_trap_states_matrix));
        REQUIRE(test == answer);

        express = 4;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        roe.set_store_trap_states_matrix();
        answer = {
            // clang-format off
            0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            // clang-format on
        };
        test.assign(
            std::begin(roe.store_trap_states_matrix),
            std::end(roe.store_trap_states_matrix));
        REQUIRE(test == answer);

        express = 12;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        roe.set_store_trap_states_matrix();
        answer = {
            // clang-format off
            1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            // clang-format on
        };
        test.assign(
            std::begin(roe.store_trap_states_matrix),
            std::end(roe.store_trap_states_matrix));
        REQUIRE(test == answer);
    }
}

TEST_CASE("Test clock sequence", "[roe]") {
    bool empty_traps_between_columns = true;
    bool empty_traps_for_first_transfers = true;
    bool force_release_away_from_readout = false;
    bool use_integer_express_matrix = false;
    ROEStepPhase* roe_step_phase;

    SECTION("Single phase") {
        /*
                      Pixel p-1  Pixel p  Pixel p+1
        Step            Phase0   Phase0   Phase0
        0              +------+ +------+ +------+
        Capture from   |      | |   p  | |      |
        Release to     |      | |   p  | |      |
                      -+      +-+      +-+      +-
        */
        std::valarray<double> dwell_times = {1.0};
        ROE roe(
            dwell_times, empty_traps_between_columns, empty_traps_for_first_transfers,
            force_release_away_from_readout, use_integer_express_matrix);
        roe.set_clock_sequence();

        REQUIRE(roe.n_steps == 1);
        REQUIRE(roe.n_phases == 1);
        REQUIRE(roe.clock_sequence.size() == roe.n_steps);
        REQUIRE(roe.clock_sequence[0].size() == roe.n_phases);

        // Single high phase
        roe_step_phase = &roe.clock_sequence[0][0];
        REQUIRE(roe_step_phase->is_high == true);
        REQUIRE(roe_step_phase->n_capture_pixels == 1);
        REQUIRE(roe_step_phase->capture_from_which_pixels[0] == 0);
        REQUIRE(roe_step_phase->n_release_pixels == 1);
        REQUIRE(roe_step_phase->release_to_which_pixels[0] == 0);
        REQUIRE(roe_step_phase->release_fraction_to_pixels[0] == 1.0);
    }

    SECTION("Two equal phases, one phase high") {
        /*
                      #  Pixel p-1  #   Pixel p   #  Pixel p+1  #
        Step           Phase1 Phase0 Phase1 Phase0 Phase1 Phase0
        0             +      +------+      +------+      +------+
        Capture from  |      |      |      |   p  |      |      |
        Release to    |      |      | p-1&p|   p  |      |      |
                      +------+      +------+      +------+      +
        1             +------+      +------+      +------+      +
        Capture from  |      |      |   p  |      |      |      |
        Release to    |      |      |   p  | p&p+1|      |      |
                      +      +------+      +------+      +------+
        */
        std::valarray<double> dwell_times(1.0 / 2.0, 2);
        ROE roe(
            dwell_times, 0, -1, empty_traps_between_columns, empty_traps_for_first_transfers,
            force_release_away_from_readout, use_integer_express_matrix);
        roe.set_clock_sequence();

        REQUIRE(roe.n_steps == 2);
        REQUIRE(roe.n_phases == 2);
        REQUIRE(roe.clock_sequence.size() == roe.n_steps);
        REQUIRE(roe.clock_sequence[0].size() == roe.n_phases);

        for (int i_step = 0; i_step < roe.n_steps; i_step++) {
            for (int i_phase = 0; i_phase < roe.n_phases; i_phase++) {
                roe_step_phase = &roe.clock_sequence[i_step][i_phase];

                // One high phase on each step
                if (i_step == i_phase) {
                    REQUIRE(roe_step_phase->is_high == true);

                    // Capture from and release to this pixel
                    REQUIRE(roe_step_phase->n_capture_pixels == 1);
                    REQUIRE(roe_step_phase->capture_from_which_pixels[0] == 0);
                    REQUIRE(roe_step_phase->n_release_pixels == 1);
                    REQUIRE(roe_step_phase->release_to_which_pixels[0] == 0);
                    REQUIRE(roe_step_phase->release_fraction_to_pixels[0] == 1.0);
                }
                // Other phases low, no capture
                else {
                    REQUIRE(roe_step_phase->is_high == false);
                    REQUIRE(roe_step_phase->n_capture_pixels == 0);
                }
            }
        }

        // Low phases release to pixels with closest high phase
        REQUIRE(roe.clock_sequence[0][1].n_release_pixels == 2);
        REQUIRE(roe.clock_sequence[0][1].release_to_which_pixels[0] == -1);
        REQUIRE(roe.clock_sequence[0][1].release_to_which_pixels[1] == 0);
        REQUIRE(roe.clock_sequence[0][1].release_fraction_to_pixels[0] == 0.5);
        REQUIRE(roe.clock_sequence[0][1].release_fraction_to_pixels[1] == 0.5);

        REQUIRE(roe.clock_sequence[1][0].n_release_pixels == 2);
        REQUIRE(roe.clock_sequence[1][0].release_to_which_pixels[0] == 0);
        REQUIRE(roe.clock_sequence[1][0].release_to_which_pixels[1] == 1);
        REQUIRE(roe.clock_sequence[1][0].release_fraction_to_pixels[0] == 0.5);
        REQUIRE(roe.clock_sequence[1][0].release_fraction_to_pixels[1] == 0.5);
    }

    SECTION("Three equal phases, one phase high") {
        /*
                #     Pixel p-1      #       Pixel p      #     Pixel p+1      #
    Step         Phase2 Phase1 Phase0 Phase2 Phase1 Phase0 Phase2 Phase1 Phase0
    0           +             +------+             +------+             +------+
    Capture from|             |      |             |   p  |             |      |
    Release to  |             |      |  p-1     p  |   p  |             |      |
                +-------------+      +-------------+      +-------------+      +
    1                  +------+             +------+             +------+
    Capture from       |      |             |   p  |             |      |
    Release to         |      |          p  |   p  |   p         |      |
                -------+      +-------------+      +-------------+      +-------
    2           +------+             +------+             +------+             +
    Capture from|      |             |   p  |             |      |             |
    Release to  |      |             |   p  |   p     p+1 |      |             |
                +      +-------------+      +-------------+      +-------------+
        */

        std::valarray<double> dwell_times(1.0 / 3.0, 3);
        ROE roe(
            dwell_times, 0, -1, empty_traps_between_columns, empty_traps_for_first_transfers,
            force_release_away_from_readout, use_integer_express_matrix);
        roe.set_clock_sequence();

        REQUIRE(roe.n_steps == 3);
        REQUIRE(roe.n_phases == 3);
        REQUIRE(roe.clock_sequence.size() == roe.n_steps);
        REQUIRE(roe.clock_sequence[0].size() == roe.n_phases);

        for (int i_step = 0; i_step < roe.n_steps; i_step++) {
            for (int i_phase = 0; i_phase < roe.n_phases; i_phase++) {
                roe_step_phase = &roe.clock_sequence[i_step][i_phase];

                // One high phase on each step
                if (i_step == i_phase) {
                    REQUIRE(roe_step_phase->is_high == true);

                    // Capture from and release to this pixel
                    REQUIRE(roe_step_phase->n_capture_pixels == 1);
                    REQUIRE(roe_step_phase->capture_from_which_pixels[0] == 0);
                    REQUIRE(roe_step_phase->n_release_pixels == 1);
                    REQUIRE(roe_step_phase->release_to_which_pixels[0] == 0);
                    REQUIRE(roe_step_phase->release_fraction_to_pixels[0] == 1.0);
                }
                // Other phases low, no capture
                else {
                    REQUIRE(roe_step_phase->is_high == false);
                    REQUIRE(roe_step_phase->n_capture_pixels == 0);
                }
            }
        }

        // Low phases release to pixel with closest high phase
        REQUIRE(roe.clock_sequence[0][1].n_release_pixels == 1);
        REQUIRE(roe.clock_sequence[0][1].release_to_which_pixels[0] == 0);
        REQUIRE(roe.clock_sequence[0][1].release_fraction_to_pixels[0] == 1.0);
        REQUIRE(roe.clock_sequence[0][2].n_release_pixels == 1);
        REQUIRE(roe.clock_sequence[0][2].release_to_which_pixels[0] == -1);
        REQUIRE(roe.clock_sequence[0][2].release_fraction_to_pixels[0] == 1.0);

        REQUIRE(roe.clock_sequence[1][0].n_release_pixels == 1);
        REQUIRE(roe.clock_sequence[1][0].release_to_which_pixels[0] == 0);
        REQUIRE(roe.clock_sequence[1][0].release_fraction_to_pixels[0] == 1.0);
        REQUIRE(roe.clock_sequence[1][2].n_release_pixels == 1);
        REQUIRE(roe.clock_sequence[1][2].release_to_which_pixels[0] == 0);
        REQUIRE(roe.clock_sequence[1][2].release_fraction_to_pixels[0] == 1.0);

        REQUIRE(roe.clock_sequence[2][0].n_release_pixels == 1);
        REQUIRE(roe.clock_sequence[2][0].release_to_which_pixels[0] == 1);
        REQUIRE(roe.clock_sequence[2][0].release_fraction_to_pixels[0] == 1.0);
        REQUIRE(roe.clock_sequence[2][1].n_release_pixels == 1);
        REQUIRE(roe.clock_sequence[2][1].release_to_which_pixels[0] == 0);
        REQUIRE(roe.clock_sequence[2][1].release_fraction_to_pixels[0] == 1.0);
    }

    SECTION("Four equal phases, one phase high") {
        /*
               Pixel p-1      #          Pixel p          #         Pixel p+1
    Step         Phase1 Phase0 Phase3 Phase2 Phase1 Phase0 Phase3 Phase2 Phase1
    0                  +------+                    +------+                    +
    Capture from       |      |                    |   p  |                    |
    Release to         |      |  p-1   p-1&p    p  |   p  |                    |
                -------+      +--------------------+      +--------------------+
    1           +------+                    +------+                    +------+
    Capture from|      |                    |   p  |                    |      |
    Release to  |      |        p-1&p    p  |   p  |   p                |      |
                +      +--------------------+      +--------------------+      +
    2           +                    +------+                    +------+
    Capture from|                    |   p  |                    |      |
    Release to  |                p   |   p  |   p    p&p+1       |      |
                +--------------------+      +--------------------+      +-------
    3                         +------+                    +------+
    Capture from              |   p  |                    |      |
    Release to                |   p  |   p    p&p+1  p+1  |      |
                --------------+      +--------------------+      +--------------
        */

        std::valarray<double> dwell_times(1.0 / 4.0, 4);
        ROE roe(
            dwell_times, 0, -1, empty_traps_between_columns, empty_traps_for_first_transfers,
            force_release_away_from_readout, use_integer_express_matrix);
        roe.set_clock_sequence();

        REQUIRE(roe.n_steps == 4);
        REQUIRE(roe.n_phases == 4);
        REQUIRE(roe.clock_sequence.size() == roe.n_steps);
        REQUIRE(roe.clock_sequence[0].size() == roe.n_phases);

        for (int i_step = 0; i_step < roe.n_steps; i_step++) {
            for (int i_phase = 0; i_phase < roe.n_phases; i_phase++) {
                roe_step_phase = &roe.clock_sequence[i_step][i_phase];

                // One high phase on each step
                if (i_step == i_phase) {
                    REQUIRE(roe_step_phase->is_high == true);

                    // Capture from and release to this pixel
                    REQUIRE(roe_step_phase->n_capture_pixels == 1);
                    REQUIRE(roe_step_phase->capture_from_which_pixels[0] == 0);
                    REQUIRE(roe_step_phase->n_release_pixels == 1);
                    REQUIRE(roe_step_phase->release_to_which_pixels[0] == 0);
                    REQUIRE(roe_step_phase->release_fraction_to_pixels[0] == 1.0);
                }
                // Other phases low, no capture
                else {
                    REQUIRE(roe_step_phase->is_high == false);
                    REQUIRE(roe_step_phase->n_capture_pixels == 0);
                }
            }
        }

        // Low phases release to pixel(s) with closest high phase
        REQUIRE(roe.clock_sequence[0][1].n_release_pixels == 1);
        REQUIRE(roe.clock_sequence[0][1].release_to_which_pixels[0] == 0);
        REQUIRE(roe.clock_sequence[0][1].release_fraction_to_pixels[0] == 1.0);
        REQUIRE(roe.clock_sequence[0][2].n_release_pixels == 2);
        REQUIRE(roe.clock_sequence[0][2].release_to_which_pixels[0] == -1);
        REQUIRE(roe.clock_sequence[0][2].release_to_which_pixels[1] == 0);
        REQUIRE(roe.clock_sequence[0][2].release_fraction_to_pixels[0] == 0.5);
        REQUIRE(roe.clock_sequence[0][2].release_fraction_to_pixels[1] == 0.5);
        REQUIRE(roe.clock_sequence[0][3].n_release_pixels == 1);
        REQUIRE(roe.clock_sequence[0][3].release_to_which_pixels[0] == -1);
        REQUIRE(roe.clock_sequence[0][3].release_fraction_to_pixels[0] == 1.0);

        REQUIRE(roe.clock_sequence[1][0].n_release_pixels == 1);
        REQUIRE(roe.clock_sequence[1][0].release_to_which_pixels[0] == 0);
        REQUIRE(roe.clock_sequence[1][0].release_fraction_to_pixels[0] == 1.0);
        REQUIRE(roe.clock_sequence[1][2].n_release_pixels == 1);
        REQUIRE(roe.clock_sequence[1][2].release_to_which_pixels[0] == 0);
        REQUIRE(roe.clock_sequence[1][2].release_fraction_to_pixels[0] == 1.0);
        REQUIRE(roe.clock_sequence[1][3].n_release_pixels == 2);
        REQUIRE(roe.clock_sequence[1][3].release_to_which_pixels[0] == -1);
        REQUIRE(roe.clock_sequence[1][3].release_to_which_pixels[1] == 0);
        REQUIRE(roe.clock_sequence[1][3].release_fraction_to_pixels[0] == 0.5);
        REQUIRE(roe.clock_sequence[1][3].release_fraction_to_pixels[1] == 0.5);

        REQUIRE(roe.clock_sequence[2][0].n_release_pixels == 2);
        REQUIRE(roe.clock_sequence[2][0].release_to_which_pixels[0] == 0);
        REQUIRE(roe.clock_sequence[2][0].release_to_which_pixels[1] == 1);
        REQUIRE(roe.clock_sequence[2][0].release_fraction_to_pixels[0] == 0.5);
        REQUIRE(roe.clock_sequence[2][0].release_fraction_to_pixels[1] == 0.5);
        REQUIRE(roe.clock_sequence[2][1].n_release_pixels == 1);
        REQUIRE(roe.clock_sequence[2][1].release_to_which_pixels[0] == 0);
        REQUIRE(roe.clock_sequence[2][1].release_fraction_to_pixels[0] == 1.0);
        REQUIRE(roe.clock_sequence[2][3].n_release_pixels == 1);
        REQUIRE(roe.clock_sequence[2][3].release_to_which_pixels[0] == 0);
        REQUIRE(roe.clock_sequence[2][3].release_fraction_to_pixels[0] == 1.0);

        REQUIRE(roe.clock_sequence[3][0].n_release_pixels == 1);
        REQUIRE(roe.clock_sequence[3][0].release_to_which_pixels[0] == 1);
        REQUIRE(roe.clock_sequence[3][0].release_fraction_to_pixels[0] == 1.0);
        REQUIRE(roe.clock_sequence[3][1].n_release_pixels == 2);
        REQUIRE(roe.clock_sequence[3][1].release_to_which_pixels[0] == 0);
        REQUIRE(roe.clock_sequence[3][1].release_to_which_pixels[1] == 1);
        REQUIRE(roe.clock_sequence[3][1].release_fraction_to_pixels[0] == 0.5);
        REQUIRE(roe.clock_sequence[3][1].release_fraction_to_pixels[1] == 0.5);
        REQUIRE(roe.clock_sequence[3][2].n_release_pixels == 1);
        REQUIRE(roe.clock_sequence[3][2].release_to_which_pixels[0] == 0);
        REQUIRE(roe.clock_sequence[3][2].release_fraction_to_pixels[0] == 1.0);
    }
}

TEST_CASE("Test charge injection ROE", "[roe]") {
    std::vector<double> test, answer;
    int n_rows = 12;
    int express = 0;
    int offset = 0;
    bool empty_traps_between_columns = true;
    bool force_release_away_from_readout = false;
    bool use_integer_express_matrix = true;
    std::valarray<double> dwell_times = {1.0};

    SECTION("Integer express matrix") {
        ROEChargeInjection roe(
            dwell_times, 0, -1, empty_traps_between_columns, force_release_away_from_readout,
            use_integer_express_matrix);
        REQUIRE(roe.type == roe_type_charge_injection);

        express = 1;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        answer = {12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12};
        test.assign(std::begin(roe.express_matrix), std::end(roe.express_matrix));
        REQUIRE(test == answer);
        REQUIRE(roe.n_express_passes == 1);

        express = 4;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        answer = {
            // clang-format off
            3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
            3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
            3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
            3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
            // clang-format on
        };
        test.assign(std::begin(roe.express_matrix), std::end(roe.express_matrix));
        REQUIRE(test == answer);
        REQUIRE(roe.n_express_passes == 4);

        express = 5;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        answer = {
            // clang-format off
            3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
            3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
            3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
            3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            // clang-format on
        };
        test.assign(std::begin(roe.express_matrix), std::end(roe.express_matrix));
        REQUIRE(test == answer);
        REQUIRE(roe.n_express_passes == 5);

        express = 12;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        answer = {
            // clang-format off
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            // clang-format on
        };
        test.assign(std::begin(roe.express_matrix), std::end(roe.express_matrix));
        REQUIRE(test == answer);
        REQUIRE(roe.n_express_passes == 12);
    }

    SECTION("Integer express matrix, with offset") {
        ROEChargeInjection roe(
            dwell_times, 0, -1, empty_traps_between_columns, force_release_away_from_readout,
            use_integer_express_matrix);
        offset = 5;

        express = 1;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        answer = {17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17};
        test.assign(std::begin(roe.express_matrix), std::end(roe.express_matrix));
        REQUIRE(test == answer);

        express = 3;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        answer = {
            // clang-format off
            6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
            6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
            5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
            // clang-format on
        };
        test.assign(std::begin(roe.express_matrix), std::end(roe.express_matrix));
        REQUIRE(test == answer);
        REQUIRE(roe.n_express_passes == 3);

        express = 12;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        answer = {
            // clang-format off
            2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
            2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
            2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
            2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
            2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
            2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
            2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
            2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            // clang-format on
        };
        test.assign(std::begin(roe.express_matrix), std::end(roe.express_matrix));
        REQUIRE(test == answer);
        REQUIRE(roe.n_express_passes == 12);

        express = 0;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        answer = {
            // clang-format off
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            // clang-format on
        };
        test.assign(std::begin(roe.express_matrix), std::end(roe.express_matrix));
        REQUIRE(test == answer);
        REQUIRE(roe.n_express_passes == 17);
    }

    SECTION("Non-integer express matrix, with and without offset") {
        double x;
        use_integer_express_matrix = false;
        ROEChargeInjection roe(
            dwell_times, 0, -1, empty_traps_between_columns, force_release_away_from_readout,
            use_integer_express_matrix);

        offset = 0;
        express = 5;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        answer = {
            // clang-format off
            2.4, 2.4, 2.4, 2.4, 2.4, 2.4, 2.4, 2.4, 2.4, 2.4, 2.4, 2.4,
            2.4, 2.4, 2.4, 2.4, 2.4, 2.4, 2.4, 2.4, 2.4, 2.4, 2.4, 2.4,
            2.4, 2.4, 2.4, 2.4, 2.4, 2.4, 2.4, 2.4, 2.4, 2.4, 2.4, 2.4,
            2.4, 2.4, 2.4, 2.4, 2.4, 2.4, 2.4, 2.4, 2.4, 2.4, 2.4, 2.4,
            2.4, 2.4, 2.4, 2.4, 2.4, 2.4, 2.4, 2.4, 2.4, 2.4, 2.4, 2.4,
            // clang-format on
        };
        test.assign(std::begin(roe.express_matrix), std::end(roe.express_matrix));
        REQUIRE(test == answer);
        REQUIRE(roe.n_express_passes == 5);

        offset = 5;
        express = 3;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        x = 17.0 / 3.0;
        answer = {
            // clang-format off
            x, x, x, x, x, x, x, x, x, x, x, x,
            x, x, x, x, x, x, x, x, x, x, x, x,
            x, x, x, x, x, x, x, x, x, x, x, x,
            // clang-format on
        };
        test.assign(std::begin(roe.express_matrix), std::end(roe.express_matrix));
        REQUIRE(test == answer);
        REQUIRE(roe.n_express_passes == 3);

        offset = 5;
        express = 12;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        x = 17.0 / 12.0;
        answer = {
            // clang-format off
            x, x, x, x, x, x, x, x, x, x, x, x,
            x, x, x, x, x, x, x, x, x, x, x, x,
            x, x, x, x, x, x, x, x, x, x, x, x,
            x, x, x, x, x, x, x, x, x, x, x, x,
            x, x, x, x, x, x, x, x, x, x, x, x,
            x, x, x, x, x, x, x, x, x, x, x, x,
            x, x, x, x, x, x, x, x, x, x, x, x,
            x, x, x, x, x, x, x, x, x, x, x, x,
            x, x, x, x, x, x, x, x, x, x, x, x,
            x, x, x, x, x, x, x, x, x, x, x, x,
            x, x, x, x, x, x, x, x, x, x, x, x,
            x, x, x, x, x, x, x, x, x, x, x, x,
            // clang-format on
        };
        test.assign(std::begin(roe.express_matrix), std::end(roe.express_matrix));
        REQUIRE(test == answer);
        REQUIRE(roe.n_express_passes == 12);
    }

    SECTION("Check always sums to n_transfers") {
        std::valarray<int> rows = {5, 7, 17};
        std::valarray<int> expresses = {0, 1, 2, 7};
        std::valarray<int> offsets = {0, 1, 13};
        std::valarray<bool> integers = {true, false};
        int n_passes;
        ROEChargeInjection roe;

        for (int i_rows = 0; i_rows < rows.size(); i_rows++) {
            n_rows = rows[i_rows];

            for (int i_express = 0; i_express < expresses.size(); i_express++) {
                express = expresses[i_express];

                for (int i_offset = 0; i_offset < offsets.size(); i_offset++) {
                    offset = offsets[i_offset];

                    for (int i_integer = 0; i_integer < integers.size(); i_integer++) {
                        roe.use_integer_express_matrix = integers[i_integer];

                        roe.set_express_matrix_from_rows_and_express(
                            n_rows, express, offset);

                        n_passes = roe.express_matrix.size() / n_rows;

                        // Check each pixel is moved the correct number of times
                        for (int row_index = 0; row_index < n_rows; row_index++) {
                            std::valarray<double> tmp_col =
                                roe.express_matrix[std::slice(
                                    row_index, n_passes, n_rows)];

                            REQUIRE(round(tmp_col.sum()) == n_rows + offset);
                        }
                    }
                }
            }
        }
    }

    SECTION("Store trap states matrix") {
        ROEChargeInjection roe(
            dwell_times, 0, -1, empty_traps_between_columns, force_release_away_from_readout,
            use_integer_express_matrix);

        express = 1;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        roe.set_store_trap_states_matrix();
        answer = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        test.assign(
            std::begin(roe.store_trap_states_matrix),
            std::end(roe.store_trap_states_matrix));
        REQUIRE(test == answer);

        express = 4;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        roe.set_store_trap_states_matrix();
        answer = {
            // clang-format off
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            // clang-format on
        };
        test.assign(
            std::begin(roe.store_trap_states_matrix),
            std::end(roe.store_trap_states_matrix));
        REQUIRE(test == answer);

        express = 12;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        roe.set_store_trap_states_matrix();
        answer = {
            // clang-format off
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            // clang-format on
        };
        test.assign(
            std::begin(roe.store_trap_states_matrix),
            std::end(roe.store_trap_states_matrix));
        REQUIRE(test == answer);
    }
}

TEST_CASE("Test trap pumping ROE", "[roe]") {
    std::vector<double> test, answer;
    int n_rows = 5;
    int n_pumps = 12;
    int express = 0;
    int offset = 0;
    bool empty_traps_for_first_transfers = false;
    bool use_integer_express_matrix = true;
    std::valarray<double> dwell_times(1.0 / 6.0, 6);

    SECTION("Integer express matrix") {
        ROETrapPumping roe(
            dwell_times, n_pumps, empty_traps_for_first_transfers,
            use_integer_express_matrix);
        REQUIRE(roe.type == roe_type_trap_pumping);

        express = 1;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        answer = {12, 12, 12, 12, 12};
        test.assign(std::begin(roe.express_matrix), std::end(roe.express_matrix));
        REQUIRE(test == answer);
        REQUIRE(roe.n_express_passes == 1);

        express = 4;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        answer = {
            // clang-format off
            3, 3, 3, 3, 3,
            3, 3, 3, 3, 3,
            3, 3, 3, 3, 3,
            3, 3, 3, 3, 3,
            // clang-format on
        };
        test.assign(std::begin(roe.express_matrix), std::end(roe.express_matrix));
        REQUIRE(test == answer);
        REQUIRE(roe.n_express_passes == 4);

        express = 5;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        answer = {
            // clang-format off
            3, 3, 3, 3, 3,
            3, 3, 3, 3, 3,
            3, 3, 3, 3, 3,
            3, 3, 3, 3, 3,
            0, 0, 0, 0, 0,
            // clang-format on
        };
        test.assign(std::begin(roe.express_matrix), std::end(roe.express_matrix));
        REQUIRE(test == answer);
        REQUIRE(roe.n_express_passes == 5);

        express = 12;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        answer = {
            // clang-format off
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            // clang-format on
        };
        test.assign(std::begin(roe.express_matrix), std::end(roe.express_matrix));
        REQUIRE(test == answer);
        REQUIRE(roe.n_express_passes == 12);
    }

    SECTION("Non-integer express matrix") {
        double x;
        use_integer_express_matrix = false;
        ROETrapPumping roe(
            dwell_times, n_pumps, empty_traps_for_first_transfers,
            use_integer_express_matrix);

        express = 1;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        answer = {12, 12, 12, 12, 12};
        test.assign(std::begin(roe.express_matrix), std::end(roe.express_matrix));
        REQUIRE(test == answer);
        REQUIRE(roe.n_express_passes == 1);

        express = 4;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        answer = {
            // clang-format off
            3, 3, 3, 3, 3,
            3, 3, 3, 3, 3,
            3, 3, 3, 3, 3,
            3, 3, 3, 3, 3,
            // clang-format on
        };
        test.assign(std::begin(roe.express_matrix), std::end(roe.express_matrix));
        REQUIRE(test == answer);
        REQUIRE(roe.n_express_passes == 4);

        express = 5;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        x = 12.0 / 5.0;
        answer = {
            // clang-format off
            x, x, x, x, x,
            x, x, x, x, x,
            x, x, x, x, x,
            x, x, x, x, x,
            x, x, x, x, x,
            // clang-format on
        };
        test.assign(std::begin(roe.express_matrix), std::end(roe.express_matrix));
        REQUIRE(test == answer);
        REQUIRE(roe.n_express_passes == 5);

        express = 12;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        answer = {
            // clang-format off
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            // clang-format on
        };
        test.assign(std::begin(roe.express_matrix), std::end(roe.express_matrix));
        REQUIRE(test == answer);
        REQUIRE(roe.n_express_passes == 12);
    }

    SECTION("Integer express matrix, empty traps for first transfers") {
        empty_traps_for_first_transfers = true;
        use_integer_express_matrix = true;
        ROETrapPumping roe(
            dwell_times, n_pumps, empty_traps_for_first_transfers,
            use_integer_express_matrix);

        express = 1;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        answer = {
            // clang-format off
            1, 1, 1, 1, 1,
            11, 11, 11, 11, 11,
            // clang-format on
        };
        test.assign(std::begin(roe.express_matrix), std::end(roe.express_matrix));
        REQUIRE(test == answer);
        REQUIRE(roe.n_express_passes == 2);

        express = 4;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        answer = {
            // clang-format off
            1, 1, 1, 1, 1,
            2, 2, 2, 2, 2,
            3, 3, 3, 3, 3,
            3, 3, 3, 3, 3,
            3, 3, 3, 3, 3,
            // clang-format on
        };
        test.assign(std::begin(roe.express_matrix), std::end(roe.express_matrix));
        REQUIRE(test == answer);
        REQUIRE(roe.n_express_passes == 5);

        express = 5;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        answer = {
            // clang-format off
            1, 1, 1, 1, 1,
            2, 2, 2, 2, 2,
            3, 3, 3, 3, 3,
            3, 3, 3, 3, 3,
            3, 3, 3, 3, 3,
            0, 0, 0, 0, 0,
            // clang-format on
        };
        test.assign(std::begin(roe.express_matrix), std::end(roe.express_matrix));
        REQUIRE(test == answer);
        REQUIRE(roe.n_express_passes == 6);

        express = 12;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        answer = {
            // clang-format off
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            // clang-format on
        };
        test.assign(std::begin(roe.express_matrix), std::end(roe.express_matrix));
        REQUIRE(test == answer);
        REQUIRE(roe.n_express_passes == 12);
    }

    SECTION("Non-integer express matrix, empty traps for first transfers") {
        double x;
        empty_traps_for_first_transfers = true;
        use_integer_express_matrix = false;
        ROETrapPumping roe(
            dwell_times, n_pumps, empty_traps_for_first_transfers,
            use_integer_express_matrix);

        express = 1;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        answer = {
            // clang-format off
            1, 1, 1, 1, 1,
            11, 11, 11, 11, 11,
            // clang-format on
        };
        test.assign(std::begin(roe.express_matrix), std::end(roe.express_matrix));
        REQUIRE(test == answer);
        REQUIRE(roe.n_express_passes == 2);

        express = 4;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        x = 11.0 / 4.0;
        answer = {
            // clang-format off
            1, 1, 1, 1, 1,
            x, x, x, x, x,
            x, x, x, x, x,
            x, x, x, x, x,
            x, x, x, x, x,
            // clang-format on
        };
        test.assign(std::begin(roe.express_matrix), std::end(roe.express_matrix));
        REQUIRE(test == answer);
        REQUIRE(roe.n_express_passes == 5);

        express = 12;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        answer = {
            // clang-format off
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            // clang-format on
        };
        test.assign(std::begin(roe.express_matrix), std::end(roe.express_matrix));
        REQUIRE(test == answer);
        REQUIRE(roe.n_express_passes == 12);
    }

    SECTION("Store trap states matrix") {
        ROETrapPumping roe(
            dwell_times, n_pumps, empty_traps_for_first_transfers,
            use_integer_express_matrix);

        express = 1;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        roe.set_store_trap_states_matrix();
        answer = {0, 0, 0, 0, 0};
        test.assign(
            std::begin(roe.store_trap_states_matrix),
            std::end(roe.store_trap_states_matrix));
        REQUIRE(test == answer);

        express = 4;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        roe.set_store_trap_states_matrix();
        answer = {
            // clang-format off
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            0, 0, 0, 0, 0,
            // clang-format on
        };
        test.assign(
            std::begin(roe.store_trap_states_matrix),
            std::end(roe.store_trap_states_matrix));
        REQUIRE(test == answer);

        express = 12;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        roe.set_store_trap_states_matrix();
        answer = {
            // clang-format off
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            0, 0, 0, 0, 0,
            // clang-format on
        };
        test.assign(
            std::begin(roe.store_trap_states_matrix),
            std::end(roe.store_trap_states_matrix));
        REQUIRE(test == answer);
    }

    SECTION("Store trap states matrix, empty traps for first transfers") {
        empty_traps_for_first_transfers = true;
        ROETrapPumping roe(
            dwell_times, n_pumps, empty_traps_for_first_transfers,
            use_integer_express_matrix);

        express = 1;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        roe.set_store_trap_states_matrix();
        answer = {
            // clang-format off
            1, 1, 1, 1, 1,
            0, 0, 0, 0, 0,
            // clang-format on
        };
        test.assign(
            std::begin(roe.store_trap_states_matrix),
            std::end(roe.store_trap_states_matrix));
        REQUIRE(test == answer);

        express = 4;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        roe.set_store_trap_states_matrix();
        answer = {
            // clang-format off
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            0, 0, 0, 0, 0,
            // clang-format on
        };
        test.assign(
            std::begin(roe.store_trap_states_matrix),
            std::end(roe.store_trap_states_matrix));
        REQUIRE(test == answer);

        express = 12;
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        roe.set_store_trap_states_matrix();
        answer = {
            // clang-format off
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            1, 1, 1, 1, 1,
            0, 0, 0, 0, 0,
            // clang-format on
        };
        test.assign(
            std::begin(roe.store_trap_states_matrix),
            std::end(roe.store_trap_states_matrix));
        REQUIRE(test == answer);
    }
}

TEST_CASE("Test trap pumping clock sequence", "[roe]") {
    int n_pumps = 1;
    bool empty_traps_for_first_transfers = true;
    bool use_integer_express_matrix = true;
    ROEStepPhase* roe_step_phase;

    SECTION("Two equal phases, one phase high") {
        /*
                      #  Pixel p-1  #   Pixel p   #  Pixel p+1  #
        Step           Phase1 Phase0 Phase1 Phase0 Phase1 Phase0
        0             +      +------+      +------+      +------+
        Capture from  |      |      |      |   p  |      |      |
        Release to    |      |      | p-1&p|   p  |      |      |
                      +------+      +------+      +------+      +
        1             +------+      +------+      +------+      +
        Capture from  |      |      |   p  |      |      |      |
        Release to    |      |      |   p  | p&p+1|      |      |
                      +      +------+      +------+      +------+
        2             +      +------+      +------+      +------+
        Capture from  |      |      |      |  p+1 |      |      |
        Release to    |      |      | p&p+1|  p+1 |      |      |
                      +------+      +------+      +------+      +
        3             +------+      +------+      +------+      +
        Capture from  |      |      |   p  |      |      |      |
        Release to    |      |      |   p  | p&p+1|      |      |
                      +      +------+      +------+      +------+
        */

        std::valarray<double> dwell_times(1.0 / 4.0, 4);
        ROETrapPumping roe(
            dwell_times, n_pumps, empty_traps_for_first_transfers,
            use_integer_express_matrix);
        roe.set_clock_sequence();

        REQUIRE(roe.n_steps == 4);
        REQUIRE(roe.n_phases == 2);
        REQUIRE(roe.clock_sequence.size() == roe.n_steps);
        REQUIRE(roe.clock_sequence[0].size() == roe.n_phases);

        int i_step_loop;
        for (int i_step = 0; i_step < roe.n_steps; i_step++) {
            for (int i_phase = 0; i_phase < roe.n_phases; i_phase++) {
                roe_step_phase = &roe.clock_sequence[i_step][i_phase];

                // Convert 0,1,2,3 to 0,1,2,1
                i_step_loop =
                    abs((i_step + roe.n_phases) % (2 * roe.n_phases) - roe.n_phases);

                // One high phase on each step
                if (i_step_loop % roe.n_phases == i_phase) {
                    REQUIRE(roe_step_phase->is_high == true);

                    // Capture from and release to this pixel (except step 2)
                    REQUIRE(roe_step_phase->n_capture_pixels == 1);
                    REQUIRE(roe_step_phase->n_release_pixels == 1);
                    REQUIRE(roe_step_phase->release_fraction_to_pixels[0] == 1.0);
                    if (i_step_loop == roe.n_phases) {
                        REQUIRE(roe_step_phase->capture_from_which_pixels[0] == 1);
                        REQUIRE(roe_step_phase->release_to_which_pixels[0] == 1);
                    } else {
                        REQUIRE(roe_step_phase->capture_from_which_pixels[0] == 0);
                        REQUIRE(roe_step_phase->release_to_which_pixels[0] == 0);
                    }
                }
                // Other phases low, no capture
                else {
                    REQUIRE(roe_step_phase->is_high == false);
                    REQUIRE(roe_step_phase->n_capture_pixels == 0);
                }
            }
        }

        // Low phases release to pixels with closest high phase
        REQUIRE(roe.clock_sequence[0][1].n_release_pixels == 2);
        REQUIRE(roe.clock_sequence[0][1].release_to_which_pixels[0] == -1);
        REQUIRE(roe.clock_sequence[0][1].release_to_which_pixels[1] == 0);
        REQUIRE(roe.clock_sequence[0][1].release_fraction_to_pixels[0] == 0.5);
        REQUIRE(roe.clock_sequence[0][1].release_fraction_to_pixels[1] == 0.5);

        REQUIRE(roe.clock_sequence[1][0].n_release_pixels == 2);
        REQUIRE(roe.clock_sequence[1][0].release_to_which_pixels[0] == 0);
        REQUIRE(roe.clock_sequence[1][0].release_to_which_pixels[1] == 1);
        REQUIRE(roe.clock_sequence[1][0].release_fraction_to_pixels[0] == 0.5);
        REQUIRE(roe.clock_sequence[1][0].release_fraction_to_pixels[1] == 0.5);

        REQUIRE(roe.clock_sequence[2][1].n_release_pixels == 2);
        REQUIRE(roe.clock_sequence[2][1].release_to_which_pixels[0] == 0);
        REQUIRE(roe.clock_sequence[2][1].release_to_which_pixels[1] == 1);
        REQUIRE(roe.clock_sequence[2][1].release_fraction_to_pixels[0] == 0.5);
        REQUIRE(roe.clock_sequence[2][1].release_fraction_to_pixels[1] == 0.5);

        REQUIRE(roe.clock_sequence[3][0].n_release_pixels == 2);
        REQUIRE(roe.clock_sequence[3][0].release_to_which_pixels[0] == 0);
        REQUIRE(roe.clock_sequence[3][0].release_to_which_pixels[1] == 1);
        REQUIRE(roe.clock_sequence[3][0].release_fraction_to_pixels[0] == 0.5);
        REQUIRE(roe.clock_sequence[3][0].release_fraction_to_pixels[1] == 0.5);
    }

    SECTION("Three equal phases, one phase high") {
        /*
                #     Pixel p-1      #       Pixel p      #     Pixel p+1      #
    Step         Phase2 Phase1 Phase0 Phase2 Phase1 Phase0 Phase2 Phase1 Phase0
    0           +             +------+             +------+             +------+
    Capture from|             |      |             |   p  |             |      |
    Release to  |             |      |  p-1     p  |   p  |             |      |
                +-------------+      +-------------+      +-------------+      +
    1                  +------+             +------+             +------+
    Capture from       |      |             |   p  |             |      |
    Release to         |      |          p  |   p  |   p         |      |
                -------+      +-------------+      +-------------+      +-------
    2           +------+             +------+             +------+             +
    Capture from|      |             |   p  |             |      |             |
    Release to  |      |             |   p  |   p     p+1 |      |             |
                +      +-------------+      +-------------+      +-------------+
    3           +             +------+             +------+             +------+
    Capture from|             |      |             |  p+1 |             |      |
    Release to  |             |      |   p     p+1 |  p+1 |             |      |
                +-------------+      +-------------+      +-------------+      |
    4           +------+             +------+             +------+             +
    Capture from|      |             |   p  |             |      |             |
    Release to  |      |             |   p  |   p     p+1 |      |             |
                +      +-------------+      +-------------+      +-------------+
    5                  +------+             +------+             +------+
    Capture from       |      |             |   p  |             |      |
    Release to         |      |          p  |   p  |   p         |      |
                -------+      +-------------+      +-------------+      +-------
        */

        std::valarray<double> dwell_times(1.0 / 6.0, 6);
        ROETrapPumping roe(
            dwell_times, n_pumps, empty_traps_for_first_transfers,
            use_integer_express_matrix);
        roe.set_clock_sequence();

        REQUIRE(roe.n_steps == 6);
        REQUIRE(roe.n_phases == 3);
        REQUIRE(roe.clock_sequence.size() == roe.n_steps);
        REQUIRE(roe.clock_sequence[0].size() == roe.n_phases);

        int i_step_loop;
        for (int i_step = 0; i_step < roe.n_steps; i_step++) {
            for (int i_phase = 0; i_phase < roe.n_phases; i_phase++) {
                roe_step_phase = &roe.clock_sequence[i_step][i_phase];

                // Convert 0,1,2,3,4,5 to 0,1,2,3,2,1
                i_step_loop =
                    abs((i_step + roe.n_phases) % (2 * roe.n_phases) - roe.n_phases);

                // One high phase on each step
                if (i_step_loop % roe.n_phases == i_phase) {
                    REQUIRE(roe_step_phase->is_high == true);

                    // Capture from and release to this pixel (except step 3)
                    REQUIRE(roe_step_phase->n_capture_pixels == 1);
                    REQUIRE(roe_step_phase->n_release_pixels == 1);
                    REQUIRE(roe_step_phase->release_fraction_to_pixels[0] == 1.0);
                    if (i_step_loop == roe.n_phases) {
                        REQUIRE(roe_step_phase->capture_from_which_pixels[0] == 1);
                        REQUIRE(roe_step_phase->release_to_which_pixels[0] == 1);
                    } else {
                        REQUIRE(roe_step_phase->capture_from_which_pixels[0] == 0);
                        REQUIRE(roe_step_phase->release_to_which_pixels[0] == 0);
                    }
                }
                // Other phases low, no capture
                else {
                    REQUIRE(roe_step_phase->is_high == false);
                    REQUIRE(roe_step_phase->n_capture_pixels == 0);
                }
            }
        }

        // Low phases release to pixel with closest high phase
        REQUIRE(roe.clock_sequence[0][1].n_release_pixels == 1);
        REQUIRE(roe.clock_sequence[0][1].release_to_which_pixels[0] == 0);
        REQUIRE(roe.clock_sequence[0][1].release_fraction_to_pixels[0] == 1.0);
        REQUIRE(roe.clock_sequence[0][2].n_release_pixels == 1);
        REQUIRE(roe.clock_sequence[0][2].release_to_which_pixels[0] == -1);
        REQUIRE(roe.clock_sequence[0][2].release_fraction_to_pixels[0] == 1.0);

        REQUIRE(roe.clock_sequence[1][0].n_release_pixels == 1);
        REQUIRE(roe.clock_sequence[1][0].release_to_which_pixels[0] == 0);
        REQUIRE(roe.clock_sequence[1][0].release_fraction_to_pixels[0] == 1.0);
        REQUIRE(roe.clock_sequence[1][2].n_release_pixels == 1);
        REQUIRE(roe.clock_sequence[1][2].release_to_which_pixels[0] == 0);
        REQUIRE(roe.clock_sequence[1][2].release_fraction_to_pixels[0] == 1.0);

        REQUIRE(roe.clock_sequence[2][0].n_release_pixels == 1);
        REQUIRE(roe.clock_sequence[2][0].release_to_which_pixels[0] == 1);
        REQUIRE(roe.clock_sequence[2][0].release_fraction_to_pixels[0] == 1.0);
        REQUIRE(roe.clock_sequence[2][1].n_release_pixels == 1);
        REQUIRE(roe.clock_sequence[2][1].release_to_which_pixels[0] == 0);
        REQUIRE(roe.clock_sequence[2][1].release_fraction_to_pixels[0] == 1.0);

        REQUIRE(roe.clock_sequence[3][1].n_release_pixels == 1);
        REQUIRE(roe.clock_sequence[3][1].release_to_which_pixels[0] == 1);
        REQUIRE(roe.clock_sequence[3][1].release_fraction_to_pixels[0] == 1.0);
        REQUIRE(roe.clock_sequence[3][2].n_release_pixels == 1);
        REQUIRE(roe.clock_sequence[3][2].release_to_which_pixels[0] == 0);
        REQUIRE(roe.clock_sequence[3][2].release_fraction_to_pixels[0] == 1.0);

        REQUIRE(roe.clock_sequence[4][0].n_release_pixels == 1);
        REQUIRE(roe.clock_sequence[4][0].release_to_which_pixels[0] == 1);
        REQUIRE(roe.clock_sequence[4][0].release_fraction_to_pixels[0] == 1.0);
        REQUIRE(roe.clock_sequence[4][1].n_release_pixels == 1);
        REQUIRE(roe.clock_sequence[4][1].release_to_which_pixels[0] == 0);
        REQUIRE(roe.clock_sequence[4][1].release_fraction_to_pixels[0] == 1.0);

        REQUIRE(roe.clock_sequence[5][0].n_release_pixels == 1);
        REQUIRE(roe.clock_sequence[5][0].release_to_which_pixels[0] == 0);
        REQUIRE(roe.clock_sequence[5][0].release_fraction_to_pixels[0] == 1.0);
        REQUIRE(roe.clock_sequence[5][2].n_release_pixels == 1);
        REQUIRE(roe.clock_sequence[5][2].release_to_which_pixels[0] == 0);
        REQUIRE(roe.clock_sequence[5][2].release_fraction_to_pixels[0] == 1.0);
    }

    SECTION("Four equal phases, one phase high") {
        /*
               Pixel p-1      #          Pixel p          #         Pixel p+1
    Step         Phase1 Phase0 Phase3 Phase2 Phase1 Phase0 Phase3 Phase2 Phase1
    0                  +------+                    +------+                    +
    Capture from       |      |                    |   p  |                    |
    Release to         |      |  p-1   p-1&p    p  |   p  |                    |
                -------+      +--------------------+      +--------------------+
    1           +------+                    +------+                    +------+
    Capture from|      |                    |   p  |                    |      |
    Release to  |      |        p-1&p    p  |   p  |   p                |      |
                +      +--------------------+      +--------------------+      +
    2           +                    +------+                    +------+
    Capture from|                    |   p  |                    |      |
    Release to  |                p   |   p  |   p    p&p+1       |      |
                +--------------------+      +--------------------+      +-------
    3                         +------+                    +------+
    Capture from              |   p  |                    |      |
    Release to                |   p  |   p    p&p+1  p+1  |      |
                --------------+      +--------------------+      +--------------
    4                  +------+                    +------+                    +
    Capture from       |      |                    |  p+1 |                    |
    Release to         |      |   p    p&p+1   p+1 |  p+1 |                    |
                -------+      +--------------------+      +--------------------+
    5                         +------+                    +------+
    Capture from              |   p  |                    |      |
    Release to                |   p  |   p    p&p+1  p+1  |      |
                --------------+      +--------------------+      +--------------
    6           +                    +------+                    +------+
    Capture from|                    |   p  |                    |      |
    Release to  |                p   |   p  |   p    p&p+1       |      |
                +--------------------+      +--------------------+      +-------
    7           +------+                    +------+                    +------+
    Capture from|      |                    |   p  |                    |      |
    Release to  |      |        p-1&p    p  |   p  |   p                |      |
                +      +--------------------+      +--------------------+      +
        */

        std::valarray<double> dwell_times(1.0 / 8.0, 8);
        ROETrapPumping roe(
            dwell_times, n_pumps, empty_traps_for_first_transfers,
            use_integer_express_matrix);
        roe.set_clock_sequence();

        REQUIRE(roe.n_steps == 8);
        REQUIRE(roe.n_phases == 4);
        REQUIRE(roe.clock_sequence.size() == roe.n_steps);
        REQUIRE(roe.clock_sequence[0].size() == roe.n_phases);

        int i_step_loop;
        for (int i_step = 0; i_step < roe.n_steps; i_step++) {
            for (int i_phase = 0; i_phase < roe.n_phases; i_phase++) {
                roe_step_phase = &roe.clock_sequence[i_step][i_phase];

                // Convert 0,1,2,3,4,5,6,7 to 0,1,2,3,4,3,2,1
                i_step_loop =
                    abs((i_step + roe.n_phases) % (2 * roe.n_phases) - roe.n_phases);

                // One high phase on each step
                if (i_step_loop % roe.n_phases == i_phase) {
                    REQUIRE(roe_step_phase->is_high == true);

                    // Capture from and release to this pixel (except step 4)
                    REQUIRE(roe_step_phase->n_capture_pixels == 1);
                    REQUIRE(roe_step_phase->n_release_pixels == 1);
                    REQUIRE(roe_step_phase->release_fraction_to_pixels[0] == 1.0);
                    if (i_step_loop == roe.n_phases) {
                        REQUIRE(roe_step_phase->capture_from_which_pixels[0] == 1);
                        REQUIRE(roe_step_phase->release_to_which_pixels[0] == 1);
                    } else {
                        REQUIRE(roe_step_phase->capture_from_which_pixels[0] == 0);
                        REQUIRE(roe_step_phase->release_to_which_pixels[0] == 0);
                    }
                }
                // Other phases low, no capture
                else {
                    REQUIRE(roe_step_phase->is_high == false);
                    REQUIRE(roe_step_phase->n_capture_pixels == 0);
                }
            }
        }

        // Low phases release to pixel(s) with closest high phase
        REQUIRE(roe.clock_sequence[0][1].n_release_pixels == 1);
        REQUIRE(roe.clock_sequence[0][1].release_to_which_pixels[0] == 0);
        REQUIRE(roe.clock_sequence[0][1].release_fraction_to_pixels[0] == 1.0);
        REQUIRE(roe.clock_sequence[0][2].n_release_pixels == 2);
        REQUIRE(roe.clock_sequence[0][2].release_to_which_pixels[0] == -1);
        REQUIRE(roe.clock_sequence[0][2].release_to_which_pixels[1] == 0);
        REQUIRE(roe.clock_sequence[0][2].release_fraction_to_pixels[0] == 0.5);
        REQUIRE(roe.clock_sequence[0][2].release_fraction_to_pixels[1] == 0.5);
        REQUIRE(roe.clock_sequence[0][3].n_release_pixels == 1);
        REQUIRE(roe.clock_sequence[0][3].release_to_which_pixels[0] == -1);
        REQUIRE(roe.clock_sequence[0][3].release_fraction_to_pixels[0] == 1.0);

        REQUIRE(roe.clock_sequence[1][0].n_release_pixels == 1);
        REQUIRE(roe.clock_sequence[1][0].release_to_which_pixels[0] == 0);
        REQUIRE(roe.clock_sequence[1][0].release_fraction_to_pixels[0] == 1.0);
        REQUIRE(roe.clock_sequence[1][2].n_release_pixels == 1);
        REQUIRE(roe.clock_sequence[1][2].release_to_which_pixels[0] == 0);
        REQUIRE(roe.clock_sequence[1][2].release_fraction_to_pixels[0] == 1.0);
        REQUIRE(roe.clock_sequence[1][3].n_release_pixels == 2);
        REQUIRE(roe.clock_sequence[1][3].release_to_which_pixels[0] == -1);
        REQUIRE(roe.clock_sequence[1][3].release_to_which_pixels[1] == 0);
        REQUIRE(roe.clock_sequence[1][3].release_fraction_to_pixels[0] == 0.5);
        REQUIRE(roe.clock_sequence[1][3].release_fraction_to_pixels[1] == 0.5);

        REQUIRE(roe.clock_sequence[2][0].n_release_pixels == 2);
        REQUIRE(roe.clock_sequence[2][0].release_to_which_pixels[0] == 0);
        REQUIRE(roe.clock_sequence[2][0].release_to_which_pixels[1] == 1);
        REQUIRE(roe.clock_sequence[2][0].release_fraction_to_pixels[0] == 0.5);
        REQUIRE(roe.clock_sequence[2][0].release_fraction_to_pixels[1] == 0.5);
        REQUIRE(roe.clock_sequence[2][1].n_release_pixels == 1);
        REQUIRE(roe.clock_sequence[2][1].release_to_which_pixels[0] == 0);
        REQUIRE(roe.clock_sequence[2][1].release_fraction_to_pixels[0] == 1.0);
        REQUIRE(roe.clock_sequence[2][3].n_release_pixels == 1);
        REQUIRE(roe.clock_sequence[2][3].release_to_which_pixels[0] == 0);
        REQUIRE(roe.clock_sequence[2][3].release_fraction_to_pixels[0] == 1.0);

        REQUIRE(roe.clock_sequence[3][0].n_release_pixels == 1);
        REQUIRE(roe.clock_sequence[3][0].release_to_which_pixels[0] == 1);
        REQUIRE(roe.clock_sequence[3][0].release_fraction_to_pixels[0] == 1.0);
        REQUIRE(roe.clock_sequence[3][1].n_release_pixels == 2);
        REQUIRE(roe.clock_sequence[3][1].release_to_which_pixels[0] == 0);
        REQUIRE(roe.clock_sequence[3][1].release_to_which_pixels[1] == 1);
        REQUIRE(roe.clock_sequence[3][1].release_fraction_to_pixels[0] == 0.5);
        REQUIRE(roe.clock_sequence[3][1].release_fraction_to_pixels[1] == 0.5);
        REQUIRE(roe.clock_sequence[3][2].n_release_pixels == 1);
        REQUIRE(roe.clock_sequence[3][2].release_to_which_pixels[0] == 0);
        REQUIRE(roe.clock_sequence[3][2].release_fraction_to_pixels[0] == 1.0);

        REQUIRE(roe.clock_sequence[4][1].n_release_pixels == 1);
        REQUIRE(roe.clock_sequence[4][1].release_to_which_pixels[0] == 1);
        REQUIRE(roe.clock_sequence[4][1].release_fraction_to_pixels[0] == 1.0);
        REQUIRE(roe.clock_sequence[4][2].n_release_pixels == 2);
        REQUIRE(roe.clock_sequence[4][2].release_to_which_pixels[0] == 0);
        REQUIRE(roe.clock_sequence[4][2].release_to_which_pixels[1] == 1);
        REQUIRE(roe.clock_sequence[4][2].release_fraction_to_pixels[0] == 0.5);
        REQUIRE(roe.clock_sequence[4][2].release_fraction_to_pixels[1] == 0.5);
        REQUIRE(roe.clock_sequence[4][3].n_release_pixels == 1);
        REQUIRE(roe.clock_sequence[4][3].release_to_which_pixels[0] == 0);
        REQUIRE(roe.clock_sequence[4][3].release_fraction_to_pixels[0] == 1.0);

        REQUIRE(roe.clock_sequence[5][0].n_release_pixels == 1);
        REQUIRE(roe.clock_sequence[5][0].release_to_which_pixels[0] == 1);
        REQUIRE(roe.clock_sequence[5][0].release_fraction_to_pixels[0] == 1.0);
        REQUIRE(roe.clock_sequence[5][1].n_release_pixels == 2);
        REQUIRE(roe.clock_sequence[5][1].release_to_which_pixels[0] == 0);
        REQUIRE(roe.clock_sequence[5][1].release_to_which_pixels[1] == 1);
        REQUIRE(roe.clock_sequence[5][1].release_fraction_to_pixels[0] == 0.5);
        REQUIRE(roe.clock_sequence[5][1].release_fraction_to_pixels[1] == 0.5);
        REQUIRE(roe.clock_sequence[5][2].n_release_pixels == 1);
        REQUIRE(roe.clock_sequence[5][2].release_to_which_pixels[0] == 0);
        REQUIRE(roe.clock_sequence[5][2].release_fraction_to_pixels[0] == 1.0);

        REQUIRE(roe.clock_sequence[6][0].n_release_pixels == 2);
        REQUIRE(roe.clock_sequence[6][0].release_to_which_pixels[0] == 0);
        REQUIRE(roe.clock_sequence[6][0].release_to_which_pixels[1] == 1);
        REQUIRE(roe.clock_sequence[6][0].release_fraction_to_pixels[0] == 0.5);
        REQUIRE(roe.clock_sequence[6][0].release_fraction_to_pixels[1] == 0.5);
        REQUIRE(roe.clock_sequence[6][1].n_release_pixels == 1);
        REQUIRE(roe.clock_sequence[6][1].release_to_which_pixels[0] == 0);
        REQUIRE(roe.clock_sequence[6][1].release_fraction_to_pixels[0] == 1.0);
        REQUIRE(roe.clock_sequence[6][3].n_release_pixels == 1);
        REQUIRE(roe.clock_sequence[6][3].release_to_which_pixels[0] == 0);
        REQUIRE(roe.clock_sequence[6][3].release_fraction_to_pixels[0] == 1.0);

        REQUIRE(roe.clock_sequence[7][0].n_release_pixels == 1);
        REQUIRE(roe.clock_sequence[7][0].release_to_which_pixels[0] == 0);
        REQUIRE(roe.clock_sequence[7][0].release_fraction_to_pixels[0] == 1.0);
        REQUIRE(roe.clock_sequence[7][2].n_release_pixels == 1);
        REQUIRE(roe.clock_sequence[7][2].release_to_which_pixels[0] == 0);
        REQUIRE(roe.clock_sequence[7][2].release_fraction_to_pixels[0] == 1.0);
        REQUIRE(roe.clock_sequence[7][3].n_release_pixels == 2);
        REQUIRE(roe.clock_sequence[7][3].release_to_which_pixels[0] == -1);
        REQUIRE(roe.clock_sequence[7][3].release_to_which_pixels[1] == 0);
        REQUIRE(roe.clock_sequence[7][3].release_fraction_to_pixels[0] == 0.5);
        REQUIRE(roe.clock_sequence[7][3].release_fraction_to_pixels[1] == 0.5);
    }
}



TEST_CASE("Test express matrix with prescan and overscan", "[roe]") {
    std::vector<double> test, answer;
    int n_rows = 12;
    int express = 0;
    int offset = 3;
    std::valarray<double> dwell_times = {1.0};

    SECTION("Equivalent input options for (window or prescan) offset") {

        // Charge injection redout 
        ROE roe_zero(dwell_times, 0, -1, true, true, true, true);
        roe_zero.set_express_matrix_from_rows_and_express(n_rows, express, 0);
        
        ROE roe_prescan(dwell_times, offset, -1, true, true, true, true);
        roe_prescan.set_express_matrix_from_rows_and_express(n_rows, express, 0);
        
        ROE roe(dwell_times, 0, -1, true, true, true, true);
        roe.set_express_matrix_from_rows_and_express(n_rows, express, offset);
        
        answer.assign(std::begin(roe_prescan.express_matrix), std::end(roe_prescan.express_matrix));
        test.assign(std::begin(roe.express_matrix), std::end(roe.express_matrix));
        REQUIRE(test == answer);
        
        //Overscan increases number of transfers
        REQUIRE(roe.express_matrix.sum() == roe_zero.express_matrix.sum() + 36);


        // Charge injection redout 
        ROEChargeInjection roeci_zero(dwell_times, 0, -1, true, true, true);
        roeci_zero.set_express_matrix_from_rows_and_express(n_rows, express, 0);
        
        ROEChargeInjection roeci_prescan(dwell_times, offset, -1, true, true, true);
        roeci_prescan.set_express_matrix_from_rows_and_express(n_rows, express, 0);
        
        ROEChargeInjection roeci(dwell_times, 0, -1, true, true, true);
        roeci.set_express_matrix_from_rows_and_express(n_rows, express, offset);

        answer.assign(std::begin(roeci_prescan.express_matrix), std::end(roeci_prescan.express_matrix));
        test.assign(std::begin(roeci.express_matrix), std::end(roeci.express_matrix));
        REQUIRE(test == answer);
        
        //Overscan increases number of transfers
        REQUIRE(roeci.express_matrix.sum() == roeci_zero.express_matrix.sum() + 36);        

    }
}

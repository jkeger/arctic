
#include <valarray>

#include "catch2/catch.hpp"
#include "roe.hpp"
#include "util.hpp"

TEST_CASE("Test ROE", "[roe]") {
    SECTION("Initialisation") {
        ROE roe;
        REQUIRE(roe.dwell_time == 1.0);
        REQUIRE(roe.empty_traps_for_first_transfers == true);
        REQUIRE(roe.use_integer_express_matrix == false);

        ROE roe_2(2.0);
        REQUIRE(roe_2.dwell_time == 2.0);
        REQUIRE(roe_2.empty_traps_for_first_transfers == true);
        REQUIRE(roe_2.use_integer_express_matrix == false);

        ROE roe_3(3.0, false);
        REQUIRE(roe_3.dwell_time == 3.0);
        REQUIRE(roe_3.empty_traps_for_first_transfers == false);
        REQUIRE(roe_3.use_integer_express_matrix == false);

        ROE roe_4(4.0, true, true);
        REQUIRE(roe_4.dwell_time == 4.0);
        REQUIRE(roe_4.empty_traps_for_first_transfers == true);
        REQUIRE(roe_4.use_integer_express_matrix == true);
    }

    SECTION("Updating parameters") {
        ROE roe;
        roe.dwell_time = 0.5;
        roe.empty_traps_for_first_transfers = true;
        roe.use_integer_express_matrix = true;
        REQUIRE(roe.dwell_time == 0.5);
        REQUIRE(roe.empty_traps_for_first_transfers == true);
        REQUIRE(roe.use_integer_express_matrix == true);
    }
}

TEST_CASE("Test express matrix", "[roe]") {
    std::vector<double> test, answer;
    int n_pixels = 12;
    int express = 0;
    int offset = 0;

    SECTION("Integer express matrix, not empty for first transfers") {
        ROE roe(1.0, false, true);
        express = 1;
        roe.set_express_matrix_from_pixels_and_express(n_pixels, express, offset);
        answer = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
        test.assign(std::begin(roe.express_matrix), std::end(roe.express_matrix));
        REQUIRE(test == answer);
        REQUIRE(roe.n_express_runs == 1);

        express = 4;
        roe.set_express_matrix_from_pixels_and_express(n_pixels, express, offset);
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
        REQUIRE(roe.n_express_runs == 4);

        express = 12;
        roe.set_express_matrix_from_pixels_and_express(n_pixels, express, offset);
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
        REQUIRE(roe.n_express_runs == 12);
    }

    SECTION("Offset") {
        ROE roe(1.0, false, true);
        offset = 5;

        express = 1;
        roe.set_express_matrix_from_pixels_and_express(n_pixels, express, offset);
        answer = {6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17};
        test.assign(std::begin(roe.express_matrix), std::end(roe.express_matrix));
        REQUIRE(test == answer);

        express = 3;
        roe.set_express_matrix_from_pixels_and_express(n_pixels, express, offset);
        answer = {
            // clang-format off
            6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
            0, 1, 2, 3, 4, 5, 6, 6, 6, 6, 6, 6,
            0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5,
            // clang-format on
        };
        test.assign(std::begin(roe.express_matrix), std::end(roe.express_matrix));
        REQUIRE(test == answer);
        REQUIRE(roe.n_express_runs == 3);

        express = 12;
        roe.set_express_matrix_from_pixels_and_express(n_pixels, express, offset);
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
        REQUIRE(roe.n_express_runs == 12);

        express = 0;
        roe.set_express_matrix_from_pixels_and_express(n_pixels, express, offset);
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
        REQUIRE(roe.n_express_runs == 17);

        roe.empty_traps_for_first_transfers = true;
        express = 4;
        roe.set_express_matrix_from_pixels_and_express(n_pixels, express, offset);
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
        REQUIRE(roe.n_express_runs == 17);
    }

    SECTION("Not integers, not empty for first transfers") {
        ROE roe(1.0, true, false);

        express = 4;
        roe.set_express_matrix_from_pixels_and_express(n_pixels, express, offset);
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
        REQUIRE(roe.n_express_runs == 12);

        // Unchanged for not empty_traps_for_first_transfers
        roe.empty_traps_for_first_transfers = false;
        express = 1;
        roe.set_express_matrix_from_pixels_and_express(n_pixels, express, offset);
        answer = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
        test.assign(std::begin(roe.express_matrix), std::end(roe.express_matrix));
        REQUIRE(test == answer);
        REQUIRE(roe.n_express_runs == 1);

        // Unchanged for no express
        roe.empty_traps_for_first_transfers = true;
        express = 12;
        roe.set_express_matrix_from_pixels_and_express(n_pixels, express, offset);
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
        REQUIRE(roe.n_express_runs == 12);
    }

    SECTION("Empty traps for first transfers") {
        ROE roe(1.0, true, true);

        express = 1;
        roe.set_express_matrix_from_pixels_and_express(n_pixels, express, offset);
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
            1,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11
            // clang-format on
        };
        test.assign(std::begin(roe.express_matrix), std::end(roe.express_matrix));
        REQUIRE(test == answer);
        REQUIRE(roe.n_express_runs == 12);

        express = 4;
        roe.set_express_matrix_from_pixels_and_express(n_pixels, express, offset);
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
        REQUIRE(roe.n_express_runs == 12);

        // Unchanged for no express
        express = 12;
        roe.set_express_matrix_from_pixels_and_express(n_pixels, express, offset);
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
        REQUIRE(roe.n_express_runs == 12);
    }

    SECTION("Check always sums to n_transfers") {
        std::valarray<int> pixels = {5, 7, 17};
        std::valarray<int> expresses = {0, 1, 2, 7};
        std::valarray<int> offsets = {0, 1, 13};
        std::valarray<bool> integers = {true, false};
        std::valarray<bool> emptys = {true, false};
        int n_rows;
        ROE roe;

        for (int i_pixels = 0; i_pixels < pixels.size(); i_pixels++) {
            n_pixels = pixels[i_pixels];
            for (int i_express = 0; i_express < expresses.size(); i_express++) {
                express = expresses[i_express];
                for (int i_offset = 0; i_offset < offsets.size(); i_offset++) {
                    offset = offsets[i_offset];
                    for (int i_integer = 0; i_integer < integers.size(); i_integer++) {
                        roe.use_integer_express_matrix = integers[i_integer];
                        for (int i_empty = 0; i_empty < emptys.size(); i_empty++) {
                            roe.empty_traps_for_first_transfers = emptys[i_empty];

                            roe.set_express_matrix_from_pixels_and_express(
                                n_pixels, express, offset);

                            n_rows = roe.express_matrix.size() / n_pixels;

                            // Check each pixel is moved the correct number of
                            // times to reach the readout from its position
                            for (int i_col = 0; i_col < n_pixels; i_col++) {
                                std::valarray<double> tmp_col =
                                    roe.express_matrix[std::slice(
                                        i_col, n_rows, n_pixels)];

                                REQUIRE(round(tmp_col.sum()) == 1 + i_col + offset);
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
    int n_pixels = 12;
    int express = 0;
    int offset = 0;

    SECTION("Empty traps for first transfers: no need to store trap states") {
        ROE roe(1.0, true, false);
        express = 1;
        roe.set_express_matrix_from_pixels_and_express(n_pixels, express, offset);
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
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
            // clang-format on
        };
        test.assign(
            std::begin(roe.store_trap_states_matrix),
            std::end(roe.store_trap_states_matrix));
        REQUIRE(test == answer);

        express = 4;
        roe.set_express_matrix_from_pixels_and_express(n_pixels, express, offset);
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
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
            // clang-format on
        };
        test.assign(
            std::begin(roe.store_trap_states_matrix),
            std::end(roe.store_trap_states_matrix));
        REQUIRE(test == answer);

        express = 12;
        roe.set_express_matrix_from_pixels_and_express(n_pixels, express, offset);
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
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
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
        ROE roe(1.0, false, false);

        // But no need for express = 1
        express = 1;
        roe.set_express_matrix_from_pixels_and_express(n_pixels, express, offset);
        roe.set_store_trap_states_matrix();
        answer = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        test.assign(
            std::begin(roe.store_trap_states_matrix),
            std::end(roe.store_trap_states_matrix));
        REQUIRE(test == answer);

        express = 4;
        roe.set_express_matrix_from_pixels_and_express(n_pixels, express, offset);
        roe.set_store_trap_states_matrix();
        answer = {
            // clang-format off
            0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
            // clang-format on
        };
        test.assign(
            std::begin(roe.store_trap_states_matrix),
            std::end(roe.store_trap_states_matrix));
        REQUIRE(test == answer);

        express = 12;
        roe.set_express_matrix_from_pixels_and_express(n_pixels, express, offset);
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
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
            // clang-format on
        };
        test.assign(
            std::begin(roe.store_trap_states_matrix),
            std::end(roe.store_trap_states_matrix));
        REQUIRE(test == answer);
    }
}
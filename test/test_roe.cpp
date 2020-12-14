
#include <valarray>

#include "catch2/catch.hpp"
#include "roe.hpp"
#include "util.hpp"

TEST_CASE("Test express matrix", "[roe]") {
    std::valarray<double> express_matrix;
    std::vector<double> answer, express_matrix_;
    int n_pixels = 12;
    int express = 0;
    int offset = 0;
    bool integer_express_matrix = true;
    bool empty_traps_for_first_transfers = false;

    SECTION("integer_express_matrix") {
        express = 1;
        express_matrix = express_matrix_from_pixels_and_express(
            n_pixels, express, offset, integer_express_matrix,
            empty_traps_for_first_transfers);
        answer = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
        express_matrix_.assign(std::begin(express_matrix), std::end(express_matrix));
        REQUIRE(express_matrix_ == answer);

        express = 4;
        express_matrix = express_matrix_from_pixels_and_express(
            n_pixels, express, offset, integer_express_matrix,
            empty_traps_for_first_transfers);
        answer = {
            // clang-format off
            1, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
            0, 0, 0, 1, 2, 3, 3, 3, 3, 3, 3, 3,
            0, 0, 0, 0, 0, 0, 1, 2, 3, 3, 3, 3,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3,
            // clang-format on
        };
        express_matrix_.assign(std::begin(express_matrix), std::end(express_matrix));
        REQUIRE(express_matrix_ == answer);

        express = 12;
        express_matrix = express_matrix_from_pixels_and_express(
            n_pixels, express, offset, integer_express_matrix,
            empty_traps_for_first_transfers);
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
        express_matrix_.assign(std::begin(express_matrix), std::end(express_matrix));
        REQUIRE(express_matrix_ == answer);
    }

    SECTION("offset") {
        offset = 5;

        express = 1;
        express_matrix = express_matrix_from_pixels_and_express(
            n_pixels, express, offset, integer_express_matrix,
            empty_traps_for_first_transfers);
        answer = {6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17};
        express_matrix_.assign(std::begin(express_matrix), std::end(express_matrix));
        REQUIRE(express_matrix_ == answer);

        express = 3;
        express_matrix = express_matrix_from_pixels_and_express(
            n_pixels, express, offset, integer_express_matrix,
            empty_traps_for_first_transfers);
        answer = {
            // clang-format off
            6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
            0, 1, 2, 3, 4, 5, 6, 6, 6, 6, 6, 6,
            0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5,
            // clang-format on
        };
        express_matrix_.assign(std::begin(express_matrix), std::end(express_matrix));
        REQUIRE(express_matrix_ == answer);

        express = 12;
        express_matrix = express_matrix_from_pixels_and_express(
            n_pixels, express, offset, integer_express_matrix,
            empty_traps_for_first_transfers);
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
        express_matrix_.assign(std::begin(express_matrix), std::end(express_matrix));
        REQUIRE(express_matrix_ == answer);

        express = 0;
        express_matrix = express_matrix_from_pixels_and_express(
            n_pixels, express, offset, integer_express_matrix,
            empty_traps_for_first_transfers);
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
        express_matrix_.assign(std::begin(express_matrix), std::end(express_matrix));
        REQUIRE(express_matrix_ == answer);

        empty_traps_for_first_transfers = true;
        express = 4;
        express_matrix = express_matrix_from_pixels_and_express(
            n_pixels, express, offset, integer_express_matrix,
            empty_traps_for_first_transfers);
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
        express_matrix_.assign(std::begin(express_matrix), std::end(express_matrix));
        REQUIRE(express_matrix_ == answer);
    }

    SECTION("Not integer_express_matrix") {
        integer_express_matrix = false;

        empty_traps_for_first_transfers = true;
        express = 4;
        express_matrix = express_matrix_from_pixels_and_express(
            n_pixels, express, offset, integer_express_matrix,
            empty_traps_for_first_transfers);
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
        express_matrix_.assign(std::begin(express_matrix), std::end(express_matrix));
        REQUIRE(express_matrix_ == answer);

        // Unchanged for not empty_traps_for_first_transfers
        empty_traps_for_first_transfers = false;
        express = 1;
        express_matrix = express_matrix_from_pixels_and_express(
            n_pixels, express, offset, integer_express_matrix,
            empty_traps_for_first_transfers);
        answer = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
        express_matrix_.assign(std::begin(express_matrix), std::end(express_matrix));
        REQUIRE(express_matrix_ == answer);

        // Unchanged for no express
        empty_traps_for_first_transfers = true;
        express = 12;
        express_matrix = express_matrix_from_pixels_and_express(
            n_pixels, express, offset, integer_express_matrix,
            empty_traps_for_first_transfers);
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
        express_matrix_.assign(std::begin(express_matrix), std::end(express_matrix));
        REQUIRE(express_matrix_ == answer);
    }

    SECTION("empty_traps_for_first_transfers") {
        empty_traps_for_first_transfers = true;

        express = 1;
        express_matrix = express_matrix_from_pixels_and_express(
            n_pixels, express, offset, integer_express_matrix,
            empty_traps_for_first_transfers);
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
        express_matrix_.assign(std::begin(express_matrix), std::end(express_matrix));
        REQUIRE(express_matrix_ == answer);

        express = 4;
        express_matrix = express_matrix_from_pixels_and_express(
            n_pixels, express, offset, integer_express_matrix,
            empty_traps_for_first_transfers);
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
        express_matrix_.assign(std::begin(express_matrix), std::end(express_matrix));
        REQUIRE(express_matrix_ == answer);

        // Unchanged for no express
        express = 12;
        express_matrix = express_matrix_from_pixels_and_express(
            n_pixels, express, offset, integer_express_matrix,
            empty_traps_for_first_transfers);
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
        express_matrix_.assign(std::begin(express_matrix), std::end(express_matrix));
        REQUIRE(express_matrix_ == answer);
    }

    SECTION("Check always sums to n_transfers") {
        std::valarray<int> pixels = {5, 7, 17};
        std::valarray<int> expresses = {0, 1, 2, 7};
        std::valarray<int> offsets = {0, 1, 13};
        std::valarray<bool> integers = {true, false};
        std::valarray<bool> emptys = {true, false};
        int n_row;

        for (int i_pixels = 0; i_pixels < pixels.size(); i_pixels++) {
            n_pixels = pixels[i_pixels];
            for (int i_express = 0; i_express < expresses.size(); i_express++) {
                express = expresses[i_express];
                for (int i_offset = 0; i_offset < offsets.size(); i_offset++) {
                    offset = offsets[i_offset];
                    for (int i_integer = 0; i_integer < integers.size(); i_integer++) {
                        integer_express_matrix = integers[i_integer];
                        for (int i_empty = 0; i_empty < emptys.size(); i_empty++) {
                            empty_traps_for_first_transfers = emptys[i_empty];

                            express_matrix = express_matrix_from_pixels_and_express(
                                n_pixels, express, offset, integer_express_matrix,
                                empty_traps_for_first_transfers);

                            n_row = express_matrix.size() / n_pixels;

                            // Check each pixel is moved the correct number of
                            // times to reach the readout from its position
                            for (int i_col = 0; i_col < n_pixels; i_col++) {
                                std::valarray<double> tmp_col =
                                    express_matrix[std::slice(i_col, n_row, n_pixels)];

                                REQUIRE(round(tmp_col.sum()) == 1 + i_col + offset);
                            }
                        }
                    }
                }
            }
        }
    }
}

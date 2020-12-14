
#include "catch2/catch.hpp"
#include "util.hpp"

#include <valarray>
#include <vector>

TEST_CASE("Test arange", "[util]") {
    std::valarray<double> array;
    std::vector<double> answer, array_;

    array = arange(0, 5);
    answer = {0, 1, 2, 3, 4};
    array_.assign(std::begin(array), std::end(array));
    REQUIRE(array_ == answer);

    array = arange(1.1, 4, 0.9);
    answer = {1.1, 2, 2.9, 3.8};
    array_.assign(std::begin(array), std::end(array));
    REQUIRE(array_ == answer);
}

TEST_CASE("Demo 2D-style valarray slicing", "[util]") {
    // More of an example reference than a test
    std::vector<double> answer, image_;

    int n_col = 3;
    int n_row = 4;

    // Initialise array, all zeros
    std::valarray<double> image(0., n_col * n_row);
    answer = {
        // clang-format off
        0, 0, 0, 
        0, 0, 0,
        0, 0, 0, 
        0, 0, 0,
        // clang-format on
    };
    image_.assign(std::begin(image), std::end(image));
    REQUIRE(image_ == answer);

    // Edit second column, all ones
    image[std::slice(1, n_row, n_col)] = 1;
    // Edit third column, arange
    image[std::slice(2, n_row, n_col)] = arange(3, 3 + n_row);
    answer = {
        // clang-format off
        0, 1, 3, 
        0, 1, 4, 
        0, 1, 5, 
        0, 1, 6,
        // clang-format on
    };
    image_.assign(std::begin(image), std::end(image));
    REQUIRE(image_ == answer);

    // Edit second row, all nines
    image[std::slice(1 * n_col, n_col, 1)] = 9;
    // Edit fourth row, add 2 (via valarray copy of the slice)
    std::valarray<double> tmp_row(0., n_col);
    tmp_row = image[std::slice(3 * n_col, n_col, 1)];
    image[std::slice(3 * n_col, n_col, 1)] = tmp_row + 2.;
    answer = {
        // clang-format off
        0, 1, 3, 
        9, 9, 9, 
        0, 1, 5, 
        2, 3, 8,
        // clang-format on
    };
    image_.assign(std::begin(image), std::end(image));
    REQUIRE(image_ == answer);

    // Set all zeros to fours
    image[image == 0.] = 4.;
    answer = {
        // clang-format off
        4, 1, 3, 
        9, 9, 9, 
        4, 1, 5, 
        2, 3, 8,
        // clang-format on
    };
    image_.assign(std::begin(image), std::end(image));
    REQUIRE(image_ == answer);
}

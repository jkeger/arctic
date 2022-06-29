
#include <valarray>
#include <vector>

#include "catch2/catch.hpp"
#include "util.hpp"

TEST_CASE("Test clamp", "[util]") {
    double value = 123.456;
    REQUIRE(clamp(value, 100.0, 200.0) == 123.456);
    REQUIRE(clamp(value, 0.0, 1.0) == 1.0);
    REQUIRE(clamp(value, 999.0, 1000.0) == 999.0);
}

TEST_CASE("Test flatten", "[util]") {
    std::vector<double> answer;
    std::valarray<std::valarray<double> > array{
        // clang-format off
        {0.0, 1.0, 2.0, 3.0},
        {4.0, 5.0, 6.0, 7.0},
        {8.0, 9.0, 10.0, 11.0},
        // clang-format on
    };
    answer = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0};
    REQUIRE_THAT(flatten(array), Catch::Approx(answer));
}

TEST_CASE("Test arange", "[util]") {
    std::valarray<double> array;
    std::vector<double> answer, test;

    array = arange(0, 5);
    answer = {0, 1, 2, 3, 4};
    test.assign(std::begin(array), std::end(array));
    REQUIRE_THAT(test, Catch::Approx(answer));

    array = arange(1.1, 4, 0.9);
    answer = {1.1, 2, 2.9, 3.8};
    test.assign(std::begin(array), std::end(array));
    REQUIRE_THAT(test, Catch::Approx(answer));
}

TEST_CASE("Test transpose", "[util]") {
    std::vector<double> test_row, answer_row;
    std::valarray<std::valarray<double> > test;
    std::valarray<std::valarray<double> > array{
        // clang-format off
        {0.0, 1.0, 2.0, 3.0},
        {4.0, 5.0, 6.0, 7.0},
        {8.0, 9.0, 10.0, 11.0},
        // clang-format on
    };
    std::valarray<std::valarray<double> > array_T{
        // clang-format off
        {0.0, 4.0, 8.0},
        {1.0, 5.0, 9.0},
        {2.0, 6.0, 10.0},
        {3.0, 7.0, 11.0},
        // clang-format on
    };

    SECTION("Transpose array to array_T") {
        test = transpose(array);
        REQUIRE(test.size() == array_T.size());
        REQUIRE(test[0].size() == array_T[0].size());
        for (unsigned int i_row = 0; i_row < test.size(); i_row++) {
            test_row.assign(std::begin(test[i_row]), std::end(test[i_row]));
            answer_row.assign(std::begin(array_T[i_row]), std::end(array_T[i_row]));
            REQUIRE_THAT(test_row, Catch::Approx(answer_row));
        }
    }

    SECTION("Transpose array_T to array") {
        test = transpose(array_T);
        REQUIRE(test.size() == array.size());
        REQUIRE(test[0].size() == array[0].size());
        for (unsigned int i_row = 0; i_row < test.size(); i_row++) {
            test_row.assign(std::begin(test[i_row]), std::end(test[i_row]));
            answer_row.assign(std::begin(array[i_row]), std::end(array[i_row]));
            REQUIRE_THAT(test_row, Catch::Approx(answer_row));
        }
    }
}

TEST_CASE("Demo 2D-style 1D valarray slicing", "[util]") {
    // More of an example reference than a test
    std::vector<double> answer, image_;

    int n_col = 3;
    int n_row = 4;

    // Initialise array, all zeros
    std::valarray<double> image(0.0, n_col * n_row);
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


#include <stdio.h>
#include <valarray>
#include <vector>

#include "catch2/catch.hpp"
#include "ccd.hpp"
#include "cti.hpp"
#include "roe.hpp"
#include "trap_managers.hpp"
#include "traps.hpp"
#include "util.hpp"

TEST_CASE("Test clock charge in one direction, compare with old arctic", "[cti]") {
    set_verbosity(0);

    // Compare with the python version (which was itself tested against the
    // previous IDL version)
    std::valarray<std::valarray<double>> image_pre_cti, image_post_cti, image_py;
    std::vector<double> test, answer;
    int express;
    std::valarray<double> dwell_times = {1.0};

    SECTION("Single pixel, various express") {
        // Nice numbers for easier manual checking
        TrapInstantCapture trap(10.0, -1.0 / log(0.5));
        ROE roe(dwell_times, true, false, true);
        CCD ccd(1e3, 0.0, 1.0);
        image_pre_cti =
            std::valarray<std::valarray<double>>(std::valarray<double>(0.0, 1), 20);
        image_pre_cti[2][0] = 800.0;

        express = 1;
        image_post_cti = clock_charge_in_one_direction(
            image_pre_cti, roe, ccd, std::valarray<Trap>{trap}, express);
        image_py = {{0.000000000}, {0.000000000}, {776.000000000}, {15.920000000},
                    {9.999750000}, {6.029849250}, {3.534999123},   {2.030099496},
                    {1.147640621}, {0.640766014}, {0.354183414},   {0.194156908},
                    {0.105694167}, {0.057196805}, {0.030794351},   {0.016505772},
                    {0.008812535}, {0.004688787}, {0.002487011},   {0.001315498}};
        REQUIRE_THAT(flatten(image_post_cti), Catch::Approx(flatten(image_py)));

        express = 2;
        image_post_cti = clock_charge_in_one_direction(
            image_pre_cti, roe, ccd, std::valarray<Trap>{trap}, express);
        image_py = {{0.000000000}, {0.000000000}, {776.000000000}, {15.920000000},
                    {9.999750000}, {6.029849250}, {3.534999123},   {2.030099496},
                    {1.147640621}, {0.640766014}, {0.351503820},   {0.195205130},
                    {0.107691677}, {0.059344104}, {0.032651256},   {0.017931889},
                    {0.009828225}, {0.005375242}, {0.002933389},   {0.001597286}};
        REQUIRE_THAT(flatten(image_post_cti), Catch::Approx(flatten(image_py)));

        express = 5;
        image_post_cti = clock_charge_in_one_direction(
            image_pre_cti, roe, ccd, std::valarray<Trap>{trap}, express);
        image_py = {{0.000000000}, {0.000000000}, {776.000000000}, {15.920000000},
                    {9.944726500}, {6.044398638}, {3.575964224},   {2.077645109},
                    {1.187409621}, {0.673921772}, {0.380110626},   {0.213191168},
                    {0.118767760}, {0.066068705}, {0.036687898},   {0.020332300},
                    {0.011229112}, {0.006203726}, {0.003426735},   {0.001891689}};
        REQUIRE_THAT(flatten(image_post_cti), Catch::Approx(flatten(image_py)));

        express = 10;
        image_post_cti = clock_charge_in_one_direction(
            image_pre_cti, roe, ccd, std::valarray<Trap>{trap}, express);
        image_py = {{0.000000000}, {0.000000000}, {776.160000000}, {15.681200000},
                    {9.859558480}, {5.988455305}, {3.543547476},   {2.064161346},
                    {1.186023190}, {0.675948795}, {0.382161311},   {0.215111905},
                    {0.120479246}, {0.067318409}, {0.037493555},   {0.020858307},
                    {0.011579269}, {0.006425514}, {0.003560596},   {0.001973188}};
        REQUIRE_THAT(flatten(image_post_cti), Catch::Approx(flatten(image_py)));

        express = 20;
        image_post_cti = clock_charge_in_one_direction(
            image_pre_cti, roe, ccd, std::valarray<Trap>{trap}, express);
        image_py = {{0.000000000}, {0.000000000}, {776.239200000}, {15.603586518},
                    {9.849325322}, {5.992674142}, {3.557803028},   {2.076188299},
                    {1.196521151}, {0.683175954}, {0.387335013},   {0.218424309},
                    {0.122662696}, {0.068664350}, {0.038342031},   {0.021369444},
                    {0.011892776}, {0.006611546}, {0.003672650},   {0.002038991}};
        REQUIRE_THAT(flatten(image_post_cti), Catch::Approx(flatten(image_py)));
    }

    SECTION("Single pixel, far from readout, various express") {
        // Nice numbers for easier manual checking
        TrapInstantCapture trap(10.0, -1.0 / log(0.5));
        ROE roe(dwell_times, true, false, true);
        CCD ccd(1e3, 0.0, 0.5);
        image_pre_cti =
            std::valarray<std::valarray<double>>(std::valarray<double>(0.0, 1), 120);
        image_pre_cti[102][0] = 800.0;

        express = 2;
        image_post_cti = clock_charge_in_one_direction(
            image_pre_cti, roe, ccd, std::valarray<Trap>{trap}, express);
        image_py = {{0.000000000},   {0.000000000},   {0.000000000},  {0.000000000},
                    {0.000000000},   {0.000000000},   {0.000000000},  {0.000000000},
                    {0.000000000},   {0.000000000},   {0.000000000},  {0.000000000},
                    {0.000000000},   {0.000000000},   {0.000000000},  {0.000000000},
                    {0.000000000},   {0.000000000},   {0.000000000},  {0.000000000},
                    {0.000000000},   {0.000000000},   {0.000000000},  {0.000000000},
                    {0.000000000},   {0.000000000},   {0.000000000},  {0.000000000},
                    {0.000000000},   {0.000000000},   {0.000000000},  {0.000000000},
                    {0.000000000},   {0.000000000},   {0.000000000},  {0.000000000},
                    {0.000000000},   {0.000000000},   {0.000000000},  {0.000000000},
                    {0.000000000},   {0.000000000},   {0.000000000},  {0.000000000},
                    {0.000000000},   {0.000000000},   {0.000000000},  {0.000000000},
                    {0.000000000},   {0.000000000},   {0.000000000},  {0.000000000},
                    {0.000000000},   {0.000000000},   {0.000000000},  {0.000000000},
                    {0.000000000},   {0.000000000},   {0.000000000},  {0.000000000},
                    {0.000000000},   {0.000000000},   {0.000000000},  {0.000000000},
                    {0.000000000},   {0.000000000},   {0.000000000},  {0.000000000},
                    {0.000000000},   {0.000000000},   {0.000000000},  {0.000000000},
                    {0.000000000},   {0.000000000},   {0.000000000},  {0.000000000},
                    {0.000000000},   {0.000000000},   {0.000000000},  {0.000000000},
                    {0.000000000},   {0.000000000},   {0.000000000},  {0.000000000},
                    {0.000000000},   {0.000000000},   {0.000000000},  {0.000000000},
                    {0.000000000},   {0.000000000},   {0.000000000},  {0.000000000},
                    {0.000000000},   {0.000000000},   {0.000000000},  {0.000000000},
                    {0.000000000},   {0.000000000},   {0.000000000},  {0.000000000},
                    {0.000000000},   {0.000000000},   {42.680486315}, {250.980554962},
                    {161.809667140}, {107.464416415}, {73.096930884}, {50.659682086},
                    {35.632283609},  {25.371790601},  {18.267464741}, {13.298313130},
                    {9.795079318},   {7.307731336},   {5.528453916},  {4.244633069},
                    {3.308717162},   {2.618116075},   {2.101444416},  {1.708983555}};
        REQUIRE_THAT(flatten(image_post_cti), Catch::Approx(flatten(image_py)));

        express = 20;
        image_post_cti = clock_charge_in_one_direction(
            image_pre_cti, roe, ccd, std::valarray<Trap>{trap}, express);
        image_py = {{0.000000000},   {0.000000000},  {0.000000000},   {0.000000000},
                    {0.000000000},   {0.000000000},  {0.000000000},   {0.000000000},
                    {0.000000000},   {0.000000000},  {0.000000000},   {0.000000000},
                    {0.000000000},   {0.000000000},  {0.000000000},   {0.000000000},
                    {0.000000000},   {0.000000000},  {0.000000000},   {0.000000000},
                    {0.000000000},   {0.000000000},  {0.000000000},   {0.000000000},
                    {0.000000000},   {0.000000000},  {0.000000000},   {0.000000000},
                    {0.000000000},   {0.000000000},  {0.000000000},   {0.000000000},
                    {0.000000000},   {0.000000000},  {0.000000000},   {0.000000000},
                    {0.000000000},   {0.000000000},  {0.000000000},   {0.000000000},
                    {0.000000000},   {0.000000000},  {0.000000000},   {0.000000000},
                    {0.000000000},   {0.000000000},  {0.000000000},   {0.000000000},
                    {0.000000000},   {0.000000000},  {0.000000000},   {0.000000000},
                    {0.000000000},   {0.000000000},  {0.000000000},   {0.000000000},
                    {0.000000000},   {0.000000000},  {0.000000000},   {0.000000000},
                    {0.000000000},   {0.000000000},  {0.000000000},   {0.000000000},
                    {0.000000000},   {0.000000000},  {0.000000000},   {0.000000000},
                    {0.000000000},   {0.000000000},  {0.000000000},   {0.000000000},
                    {0.000000000},   {0.000000000},  {0.000000000},   {0.000000000},
                    {0.000000000},   {0.000000000},  {0.000000000},   {0.000000000},
                    {0.000000000},   {0.000000000},  {0.000000000},   {0.000000000},
                    {0.000000000},   {0.000000000},  {0.000000000},   {0.000000000},
                    {0.000000000},   {0.000000000},  {0.000000000},   {0.000000000},
                    {0.000000000},   {0.000000000},  {0.000000000},   {0.000000000},
                    {0.000000000},   {0.000000000},  {0.000000000},   {0.000000000},
                    {0.000000000},   {0.000000000},  {134.107315325}, {163.827380242},
                    {117.926133487}, {85.891835006}, {63.638338544},  {47.923577796},
                    {36.632080525},  {28.440968253}, {22.409766004},  {17.905473657},
                    {14.495542574},  {11.880330414}, {9.831083567},   {8.237234364},
                    {6.976338700},   {5.965092714},  {5.144674575},   {4.472341096}};
        REQUIRE_THAT(flatten(image_post_cti), Catch::Approx(flatten(image_py)));
    }

    SECTION("Single pixel, longer release time") {
        // Nice numbers for easier manual checking
        TrapInstantCapture trap(10.0, 5);
        ROE roe(dwell_times, true, false, true);
        CCD ccd(1e3, 0.0, 0.5);
        image_pre_cti =
            std::valarray<std::valarray<double>>(std::valarray<double>(0.0, 1), 40);
        image_pre_cti[2][0] = 800.0;

        express = 40;
        image_post_cti = clock_charge_in_one_direction(
            image_pre_cti, roe, ccd, std::valarray<Trap>{trap}, express);
        image_py = {{0.000000000}, {0.000000000}, {773.317606690}, {5.999125213},
                    {6.144729845}, {6.060553754}, {5.823556488},   {5.494554880},
                    {5.115565639}, {4.715930005}, {4.315646031},   {3.927879015},
                    {3.560848061}, {3.219238329}, {2.905252783},   {2.619390334},
                    {2.361016844}, {2.128779852}, {1.920905910},   {1.735410221},
                    {1.570241225}, {1.423377295}, {1.292888592},   {1.176973854},
                    {1.073979488}, {0.982406427}, {0.900908787},   {0.828287261},
                    {0.763479374}, {0.705548089}, {0.653669808},   {0.607122439},
                    {0.565273990}, {0.527571936}, {0.493533505},   {0.462736917},
                    {0.434813587}, {0.409441225}, {0.386337783},   {0.365256168}};
        REQUIRE_THAT(flatten(image_post_cti), Catch::Approx(flatten(image_py)));
    }
}

TEST_CASE("Test add CTI", "[cti]") {
    set_verbosity(0);

    std::valarray<double> dwell_times = {1.0};
    int offset = 0;
    int start = 0;
    int stop = -1;

    SECTION("Parallel and serial, same result as calling clock charge directly") {
        std::valarray<std::valarray<double>> image_pre_cti, image_add, image_clock;
        int express;
        TrapInstantCapture trap(10.0, -1.0 / log(0.5));
        std::valarray<Trap> traps = {trap};
        ROE roe(dwell_times, true, false, true);
        CCD ccd(1e3, 0.0, 1.0);
        image_pre_cti = {
            // clang-format off
            {0.0,   0.0,   0.0,   0.0},
            {200.0, 0.0,   0.0,   0.0},
            {0.0,   200.0, 0.0,   0.0},
            {0.0,   0.0,   200.0, 0.0},
            {0.0,   0.0,   0.0,   0.0},
            {0.0,   0.0,   0.0,   0.0},
            // clang-format on
        };
        express = 0;

        // Parallel
        image_add = add_cti(image_pre_cti, &roe, &ccd, &traps, express);
        image_clock =
            clock_charge_in_one_direction(image_pre_cti, roe, ccd, traps, express);
        REQUIRE_THAT(flatten(image_add), Catch::Approx(flatten(image_clock)));

        // Add serial
        image_add = add_cti(
            image_add, nullptr, nullptr, nullptr, express, offset, start, stop, &roe,
            &ccd, &traps, express, offset, start, stop);
        image_clock = transpose(image_clock);
        image_clock =
            clock_charge_in_one_direction(image_clock, roe, ccd, traps, express);
        image_clock = transpose(image_clock);
        REQUIRE_THAT(flatten(image_add), Catch::Approx(flatten(image_clock)));

        // Both at once
        image_add = add_cti(
            image_pre_cti, &roe, &ccd, &traps, express, offset, start, stop, &roe, &ccd,
            &traps, express, offset, start, stop);
        REQUIRE_THAT(flatten(image_add), Catch::Approx(flatten(image_clock)));
    }
}

TEST_CASE("Test remove CTI", "[cti]") {
    set_verbosity(0);

    std::valarray<double> dwell_times = {1.0};
    int offset = 0;
    int start = 0;
    int stop = -1;

    SECTION("Parallel and serial, better removal with more iterations") {
        // Start with the same image as "Test add CTI"
        std::valarray<std::valarray<double>> image_pre_cti, image_add_cti,
            image_remove_cti;
        int express;
        TrapInstantCapture trap(10.0, -1.0 / log(0.5));
        std::valarray<Trap> traps = {trap};
        ROE roe(dwell_times, true, false, true);
        CCD ccd(1e3, 0.0, 1.0);
        image_pre_cti = {
            // clang-format off
            {0.0,   0.0,   0.0,   0.0},
            {200.0, 0.0,   0.0,   0.0},
            {0.0,   200.0, 0.0,   0.0},
            {0.0,   0.0,   200.0, 0.0},
            {0.0,   0.0,   0.0,   0.0},
            {0.0,   0.0,   0.0,   0.0},
            // clang-format on
        };
        express = 0;

        // Add CTI
        image_add_cti = add_cti(
            image_pre_cti, &roe, &ccd, &traps, express, offset, start, stop, &roe, &ccd,
            &traps, express, offset, start, stop);

        // Remove CTI
        for (int iterations = 2; iterations <= 6; iterations++) {
            image_remove_cti = remove_cti(
                image_add_cti, iterations, &roe, &ccd, &traps, express, offset, start,
                stop, &roe, &ccd, &traps, express, offset, start, stop);

            // Expect better results with more iterations
            double tolerance = pow(10.0, 1 - iterations);
            REQUIRE_THAT(
                flatten(image_remove_cti),
                Catch::Approx(flatten(image_pre_cti)).margin(tolerance));
        }
    }
}

TEST_CASE("Test offset and windows", "[cti]") {
    set_verbosity(0);

    std::valarray<std::valarray<double>> image_pre_cti, image_post_cti;
    int express, offset;
    TrapInstantCapture trap(10.0, -1.0 / log(0.5));
    std::valarray<Trap> traps = {trap};
    std::valarray<double> dwell_times = {1.0};
    ROE roe(dwell_times, true, false, true);
    CCD ccd(1e3, 0.0, 1.0);

    SECTION("Add CTI, single pixel, vary offset") {
        std::valarray<std::valarray<double>> image_pre_cti_manual_offset,
            image_post_cti_manual_offset, extract;
        image_pre_cti =
            std::valarray<std::valarray<double>>(std::valarray<double>(0.0, 1), 12);
        image_pre_cti[2][0] = 800.0;

        int offset_tests[3] = {1, 5, 11};
        int express_tests[3] = {1, 3, 12};

        for (int i_offset = 0; i_offset < 3; i_offset++) {
            offset = offset_tests[i_offset];

            // Manually add offset to input image
            image_pre_cti_manual_offset = std::valarray<std::valarray<double>>(
                std::valarray<double>(0.0, 1), 12 + offset);
            image_pre_cti_manual_offset[2 + offset][0] = 800.0;

            // Unaffected by express
            for (int i_express = 0; i_express < 3; i_express++) {
                express = express_tests[i_express];

                image_post_cti =
                    add_cti(image_pre_cti, &roe, &ccd, &traps, express, offset);

                image_post_cti_manual_offset = add_cti(
                    image_pre_cti_manual_offset, &roe, &ccd, &traps, express, 0);

                // Strip the offset
                extract = (std::valarray<std::valarray<double>>)
                    image_post_cti_manual_offset[std::slice(offset, 12, 1)];
                REQUIRE_THAT(flatten(image_post_cti), Catch::Approx(flatten(extract)));
            }
        }
    }

    SECTION("Add CTI, single pixel, vary parallel window") {
        std::valarray<std::valarray<double>> image_post_cti_full, test, answer;
        image_pre_cti =
            std::valarray<std::valarray<double>>(std::valarray<double>(0.0, 1), 12);
        image_pre_cti[2][0] = 800.0;
        offset = 0;

        int window_start, window_stop;
        int window_tests[5][2] = {
            {3, 12},  // After bright pixel so no trail
            {1, 5},   // Start of trail
            {1, 9},   // Most of trail
            {1, 12},  // Full trail
            {0, 12},  // Full image
        };
        int express_tests[3] = {1, 3, 12};

        // Unaffected by express
        for (int i_express = 0; i_express < 3; i_express++) {
            express = express_tests[i_express];

            // Full image
            image_post_cti_full = add_cti(image_pre_cti, &roe, &ccd, &traps, express);

            // Windows
            for (int i = 0; i < 5; i++) {
                window_start = window_tests[i][0];
                window_stop = window_tests[i][1];

                image_post_cti = add_cti(
                    image_pre_cti, &roe, &ccd, &traps, express, offset, window_start,
                    window_stop);

                if (i == 0) {
                    // Window misses the bright pixel so no trail
                    REQUIRE_THAT(
                        flatten(image_post_cti), Catch::Approx(flatten(image_pre_cti)));
                } else {
                    // Same result within the window region
                    test =
                        (std::valarray<std::valarray<double>>)image_post_cti[std::slice(
                            window_start, window_stop - window_start, 1)];
                    answer = (std::valarray<std::valarray<double>>)
                        image_post_cti_full[std::slice(
                            window_start, window_stop - window_start, 1)];
                    REQUIRE_THAT(flatten(test), Catch::Approx(flatten(answer)));
                }
            }
        }
    }

    SECTION("Add CTI, parallel and serial window") {
        std::valarray<std::valarray<double>> image_post_cti_full;
        std::valarray<double> answer_row, test_row;
        std::vector<double> test, answer;
        image_pre_cti = {
            // clang-format off
            {0.0,   0.0,   0.0,   0.0},
            {0.0,   0.0,   0.0,   0.0},
            {0.0,   200.0, 0.0,   0.0},
            {0.0,   0.0,   200.0, 0.0},
            {0.0,   0.0,   0.0,   0.0},
            {1.0,   2.0,   3.0,   4.0},
            // clang-format on
        };
        offset = 0;
        int express_tests[3] = {1, 3, 12};

        // Set a window on the middle region
        int parallel_start = 1;
        int parallel_stop = 5;
        int serial_start = 1;
        int serial_stop = 3;

        // Unaffected by express
        for (int i_express = 0; i_express < 3; i_express++) {
            express = express_tests[i_express];

            // Full image
            image_post_cti_full = add_cti(
                image_pre_cti, &roe, &ccd, &traps, express, offset, 0, -1, &roe, &ccd,
                &traps, express, offset, 0, -1);

            // Window
            image_post_cti = add_cti(
                image_pre_cti, &roe, &ccd, &traps, express, offset, parallel_start,
                parallel_stop, &roe, &ccd, &traps, express, offset, serial_start,
                serial_stop);

            for (unsigned int i_row = 0; i_row < image_pre_cti.size(); i_row++) {
                // Extract each row to compare
                test_row = (std::valarray<double>)image_post_cti[i_row][std::slice(
                    serial_start, serial_stop - serial_start, 1)];

                // Iutside the window region: unchanged from the input image
                if ((i_row < parallel_start) || (i_row >= parallel_stop))
                    answer_row = (std::valarray<double>)image_pre_cti[i_row][std::slice(
                        serial_start, serial_stop - serial_start, 1)];

                // Inside the window region: same as full result
                else
                    answer_row =
                        (std::valarray<double>)image_post_cti_full[i_row][std::slice(
                            serial_start, serial_stop - serial_start, 1)];

                test.assign(std::begin(test_row), std::end(test_row));
                answer.assign(std::begin(answer_row), std::end(answer_row));
                REQUIRE_THAT(test, Catch::Approx(answer));
            }
        }
    }
}

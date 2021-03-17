
#include <stdio.h>
#include <valarray>

#include "cti.hpp"
#include "roe.hpp"
#include "trap_managers.hpp"
#include "traps.hpp"
#include "util.hpp"


/*
    Test using the arctic shared library to add CTI to a test image.
    
    Compile from the top directory with `make lib_test`.
*/
int main(int argc, char** argv) {
    
    // Make the image
    std::valarray<std::valarray<double>> image_pre_cti = {
        // clang-format off
        {0.0,   0.0,   0.0,   0.0},
        {200.0, 0.0,   0.0,   0.0},
        {0.0,   200.0, 0.0,   0.0},
        {0.0,   0.0,   200.0, 0.0},
        {0.0,   0.0,   0.0,   0.0},
        {0.0,   0.0,   0.0,   0.0},
        // clang-format on
    };
    printf("Test image: \n");
    print_array_2D(image_pre_cti);
    
    // CTI model parameters
    TrapInstantCapture trap(10.0, -1.0 / log(0.5));
    std::valarray<std::valarray<Trap>> traps = {{}, {trap}};
    std::valarray<double> dwell_times = {1.0};
    ROE roe(dwell_times);
    CCD ccd(CCDPhase(1e3, 0.0, 1.0));
    int express = 3;
    int offset = 0;
    int start = 0;
    int stop = -1;
    
    // Add parallel and serial CTI
    std::valarray<std::valarray<double>> image_post_cti = add_cti(
        image_pre_cti, &roe, &ccd, &traps, express, offset, start, stop, &roe, &ccd,
        &traps, express, offset, start, stop);
    printf("Image with CTI added: \n");
    print_array_2D(image_post_cti);

    return 0;
}

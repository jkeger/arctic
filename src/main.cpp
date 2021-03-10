
#include <stdio.h>
#include <valarray>
#include <getopt.h>

#include "cti.hpp"
#include "roe.hpp"
#include "trap_managers.hpp"
#include "traps.hpp"
#include "util.hpp"

/*
    ...
    
    Parameters
    ----------
    -h 
        Print help information and exit.
    
    -v <int>
        The verbosity parameter to control the amount of printed information:
            0       No printing (except errors etc).
            1       Standard.
            2       Extra details.
*/
int main(int argc, char** argv) {
    
    // Parse parameters
    int opt;
    while ((opt = getopt(argc, argv, ":hv:")) != -1) {
        switch(opt) {
            case 'h':
                printf("ArCTIC \n"); //## todo
                return 0;
            case 'v':
                set_verbosity(atoi(optarg));
                break; 
            case ':':
                printf("Error: -%c requires a value. Run with -h for help. \n", optopt);
                return 1;
            case '?':
                printf("Error: -%c not recognised. Run with -h for help. \n", optopt);
                return 1;
        }
    }
    
    // Other parameters (currently unused)
    for(; optind < argc; optind++){
        printf("Unparsed parameter: %s \n", argv[optind]);
    }
    
    
    // Test image
    std::valarray<std::valarray<double>> image_pre_cti, image_post_cti, image_remove_cti;
    TrapInstantCapture trap(10.0, -1.0 / log(0.5));
    std::valarray<Trap> traps = {trap};
    ROE roe(1.0, true, false, true);
    CCD ccd(1e4, 0.0, 1.0);
    int express = 5;
    // image_pre_cti = load_image_from_txt((char*) "dev/hst_acs_jc0a01h8q_raw.txt");
    image_pre_cti = load_image_from_txt((char*) "dev/hst_acs_10_col.txt");
    // image_pre_cti = {
    //     // clang-format off
    //     {0.0,   0.0,   0.0,   0.0},
    //     {200.0, 0.0,   0.0,   0.0},
    //     {0.0,   200.0, 0.0,   0.0},
    //     {0.0,   0.0,   200.0, 0.0},
    //     {0.0,   0.0,   0.0,   0.0},
    //     {0.0,   0.0,   0.0,   0.0}
    //     // clang-format on
    // };
    // print_array_2D(image_pre_cti);
    image_post_cti = add_cti(image_pre_cti, &roe, &ccd, &traps, express);
    // print_array_2D(image_post_cti);
    // image_remove_cti = remove_cti(image_post_cti, 3, &roe, &ccd, &traps, express);
    // print_array_2D(image_remove_cti);
    save_image_to_txt((char*) "dev/image_out.txt", image_post_cti);
    
    return 0;
}

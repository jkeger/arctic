
#include <getopt.h>
#include <stdio.h>
#include <valarray>

#include "cti.hpp"
#include "roe.hpp"
#include "trap_managers.hpp"
#include "traps.hpp"
#include "util.hpp"

/*
    Print help information.
*/
void print_help() {
    printf(
        "ArCTIC \n"
        "====== \n"
        "AlgoRithm for Charge Transfer Inefficiency (CTI) Correction \n"
        "----------------------------------------------------------- \n"
        "Add or remove image trails due to charge transfer inefficiency in CCD "
        "detectors by modelling the trapping, releasing, and moving of charge along "
        "pixels. \n"
        "\n"
        "https://github.com/jkeger/arctic \n"
        "Jacob Kegerreis: jacob.kegerreis@durham.ac.uk \n"
        "\n"
        "Parameters \n"
        "---------- \n"
        "-h, --help \n"
        "    Print help information and exit. \n"
        "-v <int>, --verbosity=<int> \n"
        "    The verbosity parameter to control the amount of printed information: \n"
        "        0       No printing (except errors etc). \n"
        "        1       Standard. \n"
        "        2       Extra details. \n"
        "\n"
        "See README.md for more information. \n");
}

/*
    Parse input parameters. See main()'s documentation.
*/
void parse_parameters(int argc, char** argv) {
    // Short options
    const char* const short_opts = ":hv:";
    // Full options
    const option long_opts[] = {{"help", no_argument, nullptr, 'h'},
                                {"verbosity", required_argument, nullptr, 'v'},
                                {0, 0, 0, 0}};

    // Parse options
    while (true) {
        const auto opt = getopt_long(argc, argv, short_opts, long_opts, nullptr);

        if (opt == -1) break;

        switch (opt) {
            case 'h':
                print_help();
                exit(0);
            case 'v':
                set_verbosity(atoi(optarg));
                break;
            case ':':
                printf(
                    "Error: %s requires a value. Run with -h for help. \n",
                    argv[optind - 1]);
                exit(1);
            case '?':
                printf(
                    "Error: %s not recognised. Run with -h for help. \n",
                    argv[optind - 1]);
                exit(1);
        }
    }

    // Other parameters (currently unused)
    for (; optind < argc; optind++) {
        printf("Unparsed parameter: %s \n", argv[optind]);
    }
}

/*
    ...
    
    Parameters
    ----------
    -h, --help
        Print help information and exit.
    
    -v <int>, --verbosity=<int>
        The verbosity parameter to control the amount of printed information:
            0       No printing (except errors etc).
            1       Standard.
            2       Extra details.
*/
int main(int argc, char** argv) {

    parse_parameters(argc, argv);

    // Test image
    std::valarray<std::valarray<double>> image_pre_cti, image_post_cti,
        image_remove_cti;
    TrapInstantCapture trap(10.0, -1.0 / log(0.5));
    std::valarray<Trap> traps = {trap};
    ROE roe(1.0, true, false, true);
    CCD ccd(1e4, 0.0, 1.0);
    int express = 5;
    // image_pre_cti = load_image_from_txt((char*) "dev/hst_acs_jc0a01h8q_raw.txt");
    image_pre_cti = load_image_from_txt((char*)"dev/hst_acs_10_col.txt");
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
    save_image_to_txt((char*)"dev/image_out.txt", image_post_cti);

    return 0;
}


#include <getopt.h>
#include <stdio.h>
#include <valarray>

#include "cti.hpp"
#include "roe.hpp"
#include "trap_managers.hpp"
#include "traps.hpp"
#include "util.hpp"

static bool custom_mode = false;

/*
    Run arctic with --custom or -c to execute this manually-editable code.

    A good place to run your own quick tests or use arctic without any wrappers.
    Remember to call make to recompile after editing.

    Demo version:
        + Make a test image and save it to a txt file.
        + Load the image from txt.
        + Add parallel and serial CTI.
        + Remove the CTI and save the result to file.
*/
int run_custom_code() {

    // Write an example image to a txt file
    save_image_to_txt(
        (char*)"image_test_pre_cti.txt",
        // clang-format off
        std::valarray<std::valarray<double>>{
            {0.0,   0.0,   0.0,   0.0},
            {200.0, 0.0,   0.0,   0.0},
            {0.0,   200.0, 0.0,   0.0},
            {0.0,   0.0,   200.0, 0.0},
            {0.0,   0.0,   0.0,   0.0},
            {0.0,   0.0,   0.0,   0.0},
        }  // clang-format on
    );
    
    // Load the image
    std::valarray<std::valarray<double>> image_pre_cti =
        load_image_from_txt((char*)"image_test_pre_cti.txt");
    print_v(1, "Loaded test image from image_test_pre_cti.txt: \n");
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
    print_v(1, "Image with CTI added: \n");
    print_array_2D(image_post_cti);
    
    // Remove CTI
    int n_iterations = 4;
    std::valarray<std::valarray<double>> image_remove_cti = remove_cti(
        image_post_cti, n_iterations, &roe, &ccd, &traps, express, offset, start,
        stop, &roe, &ccd, &traps, express, offset, start, stop);
    print_v(1, "Image with CTI removed: \n");
    print_array_2D(image_remove_cti);
    
    // Save the final image
    save_image_to_txt((char*)"image_test_cti_removed.txt", image_remove_cti);
    print_v(1, "Saved final image to image_test_cti_removed.txt \n");

    return 0;
}

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
        "-h, --help \n"
        "    Print help information and exit. \n"
        "-v <int>, --verbosity=<int> \n"
        "    The verbosity parameter to control the amount of printed information: \n"
        "        0       No printing (except errors etc). \n"
        "        1       Standard. \n"
        "        2       Extra details. \n"
        "-c, --custom \n"
        "    Execute the custom code in the run_custom_code() function at the very \n"
        "    top of main.cpp. For manual editing to test or run arctic without using \n"
        "    any wrappers. The demo version adds then removes CTI from a test image. \n"
        "\n"
        "See README.md for more information. \n");
}

/*
    Parse input parameters. See main()'s documentation.
*/
void parse_parameters(int argc, char** argv) {
    // Short options
    const char* const short_opts = ":hv:c";
    // Full options
    const option long_opts[] = {{"help", no_argument, nullptr, 'h'},
                                {"verbosity", required_argument, nullptr, 'v'},
                                {"custom", no_argument, nullptr, 'c'},
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
            case 'c':
                custom_mode = true;
                break;
            case ':':
                printf(
                    "Error: Option %s requires a value. Run with -h for help. \n",
                    argv[optind - 1]);
                exit(1);
            case '?':
                printf(
                    "Error: Option %s not recognised. Run with -h for help. \n",
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
    Main program.

    Parameters
    ----------
    -h, --help
        Print help information and exit.

    -v <int>, --verbosity=<int>
        The verbosity parameter to control the amount of printed information:
            0       No printing (except errors etc).
            1       Standard.
            2       Extra details.

    -c, --custom
        Execute the custom code in the run_custom_code() function at the very
        top of this file. For manual editing to test or run arctic without using
        any wrappers. The demo version adds then removes CTI from a test image.
*/
int main(int argc, char** argv) {

    parse_parameters(argc, argv);

    if (custom_mode) {
        print_v(1, "# ArCTIC: Running custom code! \n\n");
        return run_custom_code();
    }

    return 0;
}


#include <stdio.h>
#include <valarray>

#include "interface.hpp"

/*
    Wrapper for arctic print_array()
*/
void print_array(double* array, int length) {
    // Convert to a valarray
    std::valarray<double> varray(array, length);

    print_array(varray);
}

/*
    Wrapper for arctic print_array_2D()
*/
void print_array_2D(double* array, int n_rows, int n_columns) {
    // Convert to a 2D valarray
    std::valarray<std::valarray<double>> varray(
        std::valarray<double>(0.0, n_columns), n_rows);

    for (int i_row = 0; i_row < n_rows; i_row++) {
        for (int i_col = 0; i_col < n_columns; i_col++) {
            varray[i_row][i_col] = array[i_row * n_columns + i_col];
        }
    }

    print_array_2D(varray);
}

/*
    Test wrapping add_cti()
*/
void add_cti(
    double* image, 
    int n_rows, 
    int n_columns, 
    // Traps
    double trap_density, 
    double trap_lifetime,
    // ROE
    double* dwell_times_in, 
    int n_steps, 
    bool empty_traps_between_columns, 
    bool empty_traps_for_first_transfers, 
    bool force_release_away_from_readout, 
    bool use_integer_express_matrix, 
    // CCD
    double* fraction_of_traps_per_phase_in, 
    int n_phases, 
    double* full_well_depths, 
    double* well_notch_depths, 
    double* well_fill_powers, 
    // Misc
    int express, 
    int offset, 
    int start, 
    int stop) {
    // Convert image to a 2D valarray
    std::valarray<std::valarray<double>> image_pre_cti(
        std::valarray<double>(0.0, n_columns), n_rows);

    for (int i_row = 0; i_row < n_rows; i_row++) {
        for (int i_col = 0; i_col < n_columns; i_col++) {
            image_pre_cti[i_row][i_col] = image[i_row * n_columns + i_col];
        }
    }

    // Convert trap species inputs
    TrapInstantCapture trap(trap_density, trap_lifetime);
    std::valarray<std::valarray<Trap>> traps = {{}, {trap}};

    // Convert ROE inputs
    std::valarray<double> dwell_times(0.0, n_steps);
    for (int i_step = 0; i_step < n_steps; i_step++) {
        dwell_times[i_step] = dwell_times_in[i_step];
    }
    ROE roe(
        dwell_times, empty_traps_between_columns, empty_traps_for_first_transfers, 
        force_release_away_from_readout, use_integer_express_matrix);

    // Convert CCD inputs
    std::valarray<double> fraction_of_traps_per_phase(0.0, n_phases);
    CCDPhase init_phase(0.0, 0.0, 0.0);
    std::valarray<CCDPhase> phases(init_phase, n_phases);
    for (int i_phase = 0; i_phase < n_phases; i_phase++) {
        fraction_of_traps_per_phase[i_phase] = fraction_of_traps_per_phase_in[i_phase];
        phases[i_phase].full_well_depth = full_well_depths[i_phase];
        phases[i_phase].well_notch_depth = well_notch_depths[i_phase];
        phases[i_phase].well_fill_power = well_fill_powers[i_phase];
    }
    CCD ccd(phases, fraction_of_traps_per_phase);

    // Add parallel CTI
    std::valarray<std::valarray<double>> image_post_cti =
        add_cti(image_pre_cti, &roe, &ccd, &traps, express, offset, start, stop);

    // Convert the results back to modify the input image array
    for (int i_row = 0; i_row < n_rows; i_row++) {
        for (int i_col = 0; i_col < n_columns; i_col++) {
            image[i_row * n_columns + i_col] = image_post_cti[i_row][i_col];
        }
    }
}


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
    Wrapper for arctic's add_cti() in src/cti.cpp.
    
    Add CTI trails to an image by trapping, releasing, and moving electrons
    along their independent columns, for parallel and/or serial clocking.
    
    This wrapper converts the individual numbers and arrays from the Cython 
    wrapper into C++ variables to pass to the main arcctic library. See 
    cy_add_cti() in arcticpy/wrapper.pyx and add_cti() in arcticpy/main.py.
*/
void add_cti(
    double* image, int n_rows, int n_columns,
    // ========
    // Parallel
    // ========
    // ROE
    double* parallel_dwell_times_in, int parallel_n_steps,
    bool parallel_empty_traps_between_columns,
    bool parallel_empty_traps_for_first_transfers,
    bool parallel_force_release_away_from_readout,
    bool parallel_use_integer_express_matrix,
    // CCD
    double* parallel_fraction_of_traps_per_phase_in, int parallel_n_phases,
    double* parallel_full_well_depths, double* parallel_well_notch_depths,
    double* parallel_well_fill_powers,
    // Traps
    double* parallel_trap_densities, double* parallel_trap_release_timescales,
    double* parallel_trap_capture_timescales, int parallel_n_traps_standard,
    int parallel_n_traps_instant_capture,
    // Misc
    int parallel_express, int parallel_offset, int parallel_window_start,
    int parallel_window_stop,
    // ========
    // Serial
    // ========
    // ROE
    double* serial_dwell_times_in, int serial_n_steps,
    bool serial_empty_traps_between_columns,
    bool serial_empty_traps_for_first_transfers,
    bool serial_force_release_away_from_readout, bool serial_use_integer_express_matrix,
    // CCD
    double* serial_fraction_of_traps_per_phase_in, int serial_n_phases,
    double* serial_full_well_depths, double* serial_well_notch_depths,
    double* serial_well_fill_powers,
    // Traps
    double* serial_trap_densities, double* serial_trap_release_timescales,
    double* serial_trap_capture_timescales, int serial_n_traps_standard,
    int serial_n_traps_instant_capture,
    // Misc
    int serial_express, int serial_offset, int serial_window_start,
    int serial_window_stop,
    // Output
    int verbosity) {

    set_verbosity(verbosity);

    // Convert the inputs into the relevant C++ objects

    // Image to 2D valarray
    std::valarray<std::valarray<double>> image_pre_cti(
        std::valarray<double>(0.0, n_columns), n_rows);
    for (int i_row = 0; i_row < n_rows; i_row++) {
        for (int i_col = 0; i_col < n_columns; i_col++) {
            image_pre_cti[i_row][i_col] = image[i_row * n_columns + i_col];
        }
    }

    // ========
    // Parallel
    // ========
    // ROE
    std::valarray<double> parallel_dwell_times(0.0, parallel_n_steps);
    for (int i_step = 0; i_step < parallel_n_steps; i_step++) {
        parallel_dwell_times[i_step] = parallel_dwell_times_in[i_step];
    }
    ROE parallel_roe(
        parallel_dwell_times, parallel_empty_traps_between_columns,
        parallel_empty_traps_for_first_transfers,
        parallel_force_release_away_from_readout, parallel_use_integer_express_matrix);

    // CCD
    std::valarray<double> parallel_fraction_of_traps_per_phase(0.0, parallel_n_phases);
    std::valarray<CCDPhase> parallel_phases(CCDPhase(0.0, 0.0, 0.0), parallel_n_phases);
    for (int i_phase = 0; i_phase < parallel_n_phases; i_phase++) {
        parallel_fraction_of_traps_per_phase[i_phase] =
            parallel_fraction_of_traps_per_phase_in[i_phase];
        parallel_phases[i_phase].full_well_depth = parallel_full_well_depths[i_phase];
        parallel_phases[i_phase].well_notch_depth = parallel_well_notch_depths[i_phase];
        parallel_phases[i_phase].well_fill_power = parallel_well_fill_powers[i_phase];
    }
    CCD parallel_ccd(parallel_phases, parallel_fraction_of_traps_per_phase);

    // Traps
    std::valarray<Trap> parallel_traps_standard(
        Trap(0.0, 0.0, 0.0), parallel_n_traps_standard);
    std::valarray<Trap> parallel_traps_instant_capture(
        TrapInstantCapture(0.0, 0.0), parallel_n_traps_instant_capture);
    for (int i_trap = 0; i_trap < parallel_n_traps_standard; i_trap++) {
        parallel_traps_standard[i_trap] = Trap(
            parallel_trap_densities[i_trap], parallel_trap_release_timescales[i_trap],
            parallel_trap_capture_timescales[i_trap]);
    }
    for (int i_trap = parallel_n_traps_standard;
         i_trap < parallel_n_traps_instant_capture; i_trap++) {
        parallel_traps_instant_capture[i_trap] = TrapInstantCapture(
            parallel_trap_densities[i_trap], parallel_trap_release_timescales[i_trap]);
    }
    std::valarray<std::valarray<Trap>> parallel_traps{parallel_traps_standard,
                                                      parallel_traps_instant_capture};

    // ========
    // Serial
    // ========
    // ROE
    std::valarray<double> serial_dwell_times(0.0, serial_n_steps);
    for (int i_step = 0; i_step < serial_n_steps; i_step++) {
        serial_dwell_times[i_step] = serial_dwell_times_in[i_step];
    }
    ROE serial_roe(
        serial_dwell_times, serial_empty_traps_between_columns,
        serial_empty_traps_for_first_transfers, serial_force_release_away_from_readout,
        serial_use_integer_express_matrix);

    // CCD
    std::valarray<double> serial_fraction_of_traps_per_phase(0.0, serial_n_phases);
    std::valarray<CCDPhase> serial_phases(CCDPhase(0.0, 0.0, 0.0), serial_n_phases);
    for (int i_phase = 0; i_phase < serial_n_phases; i_phase++) {
        serial_fraction_of_traps_per_phase[i_phase] =
            serial_fraction_of_traps_per_phase_in[i_phase];
        serial_phases[i_phase].full_well_depth = serial_full_well_depths[i_phase];
        serial_phases[i_phase].well_notch_depth = serial_well_notch_depths[i_phase];
        serial_phases[i_phase].well_fill_power = serial_well_fill_powers[i_phase];
    }
    CCD serial_ccd(serial_phases, serial_fraction_of_traps_per_phase);

    // Traps
    std::valarray<Trap> serial_traps_standard(
        Trap(0.0, 0.0, 0.0), serial_n_traps_standard);
    std::valarray<Trap> serial_traps_instant_capture(
        TrapInstantCapture(0.0, 0.0), serial_n_traps_instant_capture);
    for (int i_trap = 0; i_trap < serial_n_traps_standard; i_trap++) {
        serial_traps_standard[i_trap] = Trap(
            serial_trap_densities[i_trap], serial_trap_release_timescales[i_trap],
            serial_trap_capture_timescales[i_trap]);
    }
    for (int i_trap = serial_n_traps_standard; i_trap < serial_n_traps_instant_capture;
         i_trap++) {
        serial_traps_instant_capture[i_trap] = TrapInstantCapture(
            serial_trap_densities[i_trap], serial_trap_release_timescales[i_trap]);
    }
    std::valarray<std::valarray<Trap>> serial_traps{serial_traps_standard,
                                                    serial_traps_instant_capture};

    // ========
    // Add CTI
    // ========
    std::valarray<std::valarray<double>> image_post_cti;
    // No parallel, serial only
    if (parallel_n_traps_standard + parallel_n_traps_instant_capture == 0) {
        image_post_cti = add_cti(
            image_pre_cti,
            // Parallel
            nullptr, nullptr, nullptr, 0, 0, 0, -1,
            // Serial
            &serial_roe, &serial_ccd, &serial_traps, serial_express, serial_offset,
            serial_window_start, serial_window_stop);
    }
    // No serial, parallel only
    else if (serial_n_traps_standard + serial_n_traps_instant_capture == 0) {
        image_post_cti = add_cti(
            image_pre_cti,
            // Parallel
            &parallel_roe, &parallel_ccd, &parallel_traps, parallel_express,
            parallel_offset, parallel_window_start, parallel_window_stop,
            // Serial
            nullptr, nullptr, nullptr, 0, 0, 0, -1);
    }
    // Parallel and serial
    else {
        image_post_cti = add_cti(
            image_pre_cti,
            // Parallel
            &parallel_roe, &parallel_ccd, &parallel_traps, parallel_express,
            parallel_offset, parallel_window_start, parallel_window_stop,
            // Serial
            &serial_roe, &serial_ccd, &serial_traps, serial_express, serial_offset,
            serial_window_start, serial_window_stop);
    }

    // Convert the output image back to modify the input image array
    for (int i_row = 0; i_row < n_rows; i_row++) {
        for (int i_col = 0; i_col < n_columns; i_col++) {
            image[i_row * n_columns + i_col] = image_post_cti[i_row][i_col];
        }
    }
}

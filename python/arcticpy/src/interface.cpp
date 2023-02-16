
#include "interface.hpp"

#include <stdio.h>

#include <valarray>

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
    std::valarray<std::valarray<double> > varray(
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
    cy_add_cti() in wrapper.pyx and add_cti() in cti.py.
*/
void add_cti(
    double* image, int n_rows, int n_columns,
    // ========
    // Parallel
    // ========
    // ROE
    double* parallel_dwell_times_in, 
    int parallel_n_steps,
    int parallel_prescan_offset,
    int parallel_overscan_start,
    bool parallel_empty_traps_between_columns,
    bool parallel_empty_traps_for_first_transfers,
    bool parallel_force_release_away_from_readout,
    bool parallel_use_integer_express_matrix, 
    int parallel_n_pumps,
    int parallel_roe_type,
    // CCD
    double* parallel_fraction_of_traps_per_phase_in, int parallel_n_phases,
    double* parallel_full_well_depths, double* parallel_well_notch_depths,
    double* parallel_well_fill_powers, double* parallel_first_electron_fills,
    // Traps
    double* parallel_trap_densities, double* parallel_trap_release_timescales,
    double* parallel_trap_third_params, double* parallel_trap_fourth_params,
    int parallel_n_traps_ic, int parallel_n_traps_sc, int parallel_n_traps_ic_co,
    int parallel_n_traps_sc_co,
    // Misc
    int parallel_express, int parallel_offset, 
    int parallel_window_start, int parallel_window_stop,
    int parallel_time_start, int parallel_time_stop,
    double* parallel_prune_n_electrons, int parallel_prune_frequency,
    // ========
    // Serial
    // ========
    // ROE
    double* serial_dwell_times_in, 
    int serial_n_steps,
    int serial_prescan_offset,
    int serial_overscan_start,
    bool serial_empty_traps_between_columns,
    bool serial_empty_traps_for_first_transfers,
    bool serial_force_release_away_from_readout, 
    bool serial_use_integer_express_matrix,
    int serial_n_pumps, 
    int serial_roe_type,
    // CCD
    double* serial_fraction_of_traps_per_phase_in, int serial_n_phases,
    double* serial_full_well_depths, double* serial_well_notch_depths,
    double* serial_well_fill_powers, double* serial_first_electron_fills,
    // Traps
    double* serial_trap_densities, double* serial_trap_release_timescales,
    double* serial_trap_third_params, double* serial_trap_fourth_params,
    int serial_n_traps_ic, int serial_n_traps_sc, int serial_n_traps_ic_co,
    int serial_n_traps_sc_co,
    // Misc
    int serial_express, int serial_offset, 
    int serial_window_start, int serial_window_stop, 
    int serial_time_start, int serial_time_stop,
    double* serial_prune_n_electrons, int serial_prune_frequency,
    // ========
    // Combined
    // ========
    int allow_negative_pixels, 
    // Output
    int verbosity, int iteration) {

    set_verbosity(verbosity);

    // Convert the inputs into the relevant C++ objects

    // Image to 2D valarray
    std::valarray<std::valarray<double> > image_in(
        std::valarray<double>(0.0, n_columns), n_rows);
    for (int i_row = 0; i_row < n_rows; i_row++) {
        for (int i_col = 0; i_col < n_columns; i_col++) {
            image_in[i_row][i_col] = image[i_row * n_columns + i_col];
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
    ROE* p_parallel_roe = NULL;
    if (parallel_roe_type == 0) {
        p_parallel_roe = new ROE(
            parallel_dwell_times, 
            parallel_prescan_offset,
            parallel_overscan_start,
            parallel_empty_traps_between_columns,
            parallel_empty_traps_for_first_transfers,
            parallel_force_release_away_from_readout,
            parallel_use_integer_express_matrix);
    } else if (parallel_roe_type == 1) {
        p_parallel_roe = new ROEChargeInjection(
            parallel_dwell_times, 
            parallel_prescan_offset,
            parallel_overscan_start,
            parallel_empty_traps_between_columns,
            parallel_force_release_away_from_readout,
            parallel_use_integer_express_matrix);
    } else {
        p_parallel_roe = new ROETrapPumping(
            parallel_dwell_times, 
            parallel_n_pumps,
            parallel_empty_traps_for_first_transfers,
            parallel_use_integer_express_matrix);
    }

    // CCD
    std::valarray<double> parallel_fraction_of_traps_per_phase(0.0, parallel_n_phases);
    std::valarray<CCDPhase> parallel_phases(CCDPhase(0.0, 0.0, 0.0), parallel_n_phases);
    for (int i_phase = 0; i_phase < parallel_n_phases; i_phase++) {
        parallel_fraction_of_traps_per_phase[i_phase] =
            parallel_fraction_of_traps_per_phase_in[i_phase];
        parallel_phases[i_phase].full_well_depth = parallel_full_well_depths[i_phase];
        parallel_phases[i_phase].well_notch_depth = parallel_well_notch_depths[i_phase];
        parallel_phases[i_phase].well_fill_power = parallel_well_fill_powers[i_phase];
        parallel_phases[i_phase].first_electron_fill = parallel_first_electron_fills[i_phase];
    }
    CCD parallel_ccd(parallel_phases, parallel_fraction_of_traps_per_phase);

    // Traps
    std::valarray<TrapInstantCapture> parallel_traps_ic(
        TrapInstantCapture(0.0, 0.0), parallel_n_traps_ic);
    std::valarray<TrapSlowCapture> parallel_traps_sc(
        TrapSlowCapture(0.0, 0.0, 0.0), parallel_n_traps_sc);
    std::valarray<TrapInstantCaptureContinuum> parallel_traps_continuum(
        TrapInstantCaptureContinuum(0.0, 0.0, 0.0), parallel_n_traps_ic_co);
    std::valarray<TrapSlowCaptureContinuum> parallel_traps_sc_co(
        TrapSlowCaptureContinuum(0.0, 0.0, 0.0, 0.0), parallel_n_traps_sc_co);

    int n_traps_parallel = 0;
    for (int i_trap = n_traps_parallel; i_trap < n_traps_parallel + parallel_n_traps_ic;
         i_trap++) {
        parallel_traps_ic[i_trap] = TrapInstantCapture(
            parallel_trap_densities[i_trap], parallel_trap_release_timescales[i_trap],
            parallel_trap_third_params[i_trap], parallel_trap_fourth_params[i_trap]);
    }
    n_traps_parallel += parallel_n_traps_ic;
    for (int i_trap = n_traps_parallel; i_trap < n_traps_parallel + parallel_n_traps_sc;
         i_trap++) {
        parallel_traps_sc[i_trap - n_traps_parallel] = TrapSlowCapture(
            parallel_trap_densities[i_trap], parallel_trap_release_timescales[i_trap],
            parallel_trap_third_params[i_trap]);
    }
    n_traps_parallel += parallel_n_traps_sc;
    for (int i_trap = n_traps_parallel;
         i_trap < n_traps_parallel + parallel_n_traps_ic_co; i_trap++) {
        parallel_traps_continuum[i_trap - n_traps_parallel] = TrapInstantCaptureContinuum(
            parallel_trap_densities[i_trap], parallel_trap_release_timescales[i_trap],
            parallel_trap_third_params[i_trap]);
    }
    n_traps_parallel += parallel_n_traps_ic_co;
    for (int i_trap = n_traps_parallel;
         i_trap < n_traps_parallel + parallel_n_traps_sc_co; i_trap++) {
        parallel_traps_sc_co[i_trap - n_traps_parallel] = TrapSlowCaptureContinuum(
            parallel_trap_densities[i_trap], parallel_trap_release_timescales[i_trap],
            parallel_trap_third_params[i_trap], parallel_trap_fourth_params[i_trap]);
    }
    n_traps_parallel += parallel_n_traps_sc_co;

    // ========
    // Serial
    // ========
    // ROE
    std::valarray<double> serial_dwell_times(0.0, serial_n_steps);
    for (int i_step = 0; i_step < serial_n_steps; i_step++) {
        serial_dwell_times[i_step] = serial_dwell_times_in[i_step];
    }
    ROE* p_serial_roe = NULL;
    if (serial_roe_type == 0) {
        p_serial_roe = new ROE(
            serial_dwell_times, 
            serial_prescan_offset,
            serial_overscan_start,
            serial_empty_traps_between_columns,
            serial_empty_traps_for_first_transfers,
            serial_force_release_away_from_readout, 
            serial_use_integer_express_matrix);
    } else if (serial_roe_type == 1) {
        p_serial_roe = new ROEChargeInjection(
            serial_dwell_times, 
            serial_prescan_offset,
            serial_overscan_start,
            serial_empty_traps_between_columns,
            serial_force_release_away_from_readout, 
            serial_use_integer_express_matrix);
    } else {
        p_serial_roe = new ROETrapPumping(
            serial_dwell_times, 
            serial_n_pumps, 
            serial_empty_traps_for_first_transfers,
            serial_use_integer_express_matrix);
    }

    // CCD
    std::valarray<double> serial_fraction_of_traps_per_phase(0.0, serial_n_phases);
    std::valarray<CCDPhase> serial_phases(CCDPhase(0.0, 0.0, 0.0), serial_n_phases);
    for (int i_phase = 0; i_phase < serial_n_phases; i_phase++) {
        serial_fraction_of_traps_per_phase[i_phase] =
            serial_fraction_of_traps_per_phase_in[i_phase];
        serial_phases[i_phase].full_well_depth = serial_full_well_depths[i_phase];
        serial_phases[i_phase].well_notch_depth = serial_well_notch_depths[i_phase];
        serial_phases[i_phase].well_fill_power = serial_well_fill_powers[i_phase];
        serial_phases[i_phase].first_electron_fill = serial_first_electron_fills[i_phase];
    }
    CCD serial_ccd(serial_phases, serial_fraction_of_traps_per_phase);

    // Traps
    std::valarray<TrapInstantCapture> serial_traps_ic(
        TrapInstantCapture(0.0, 0.0), serial_n_traps_ic);
    std::valarray<TrapSlowCapture> serial_traps_sc(
        TrapSlowCapture(0.0, 0.0, 0.0), serial_n_traps_sc);
    std::valarray<TrapInstantCaptureContinuum> serial_traps_continuum(
        TrapInstantCaptureContinuum(0.0, 0.0, 0.0), serial_n_traps_ic_co);
    std::valarray<TrapSlowCaptureContinuum> serial_traps_sc_co(
        TrapSlowCaptureContinuum(0.0, 0.0, 0.0, 0.0), serial_n_traps_sc_co);

    int n_traps_serial = 0;
    for (int i_trap = n_traps_serial; i_trap < n_traps_serial + serial_n_traps_ic;
         i_trap++) {
        serial_traps_ic[i_trap] = TrapInstantCapture(
            serial_trap_densities[i_trap], serial_trap_release_timescales[i_trap],
            serial_trap_third_params[i_trap], serial_trap_fourth_params[i_trap]);
    }
    n_traps_serial += serial_n_traps_ic;
    for (int i_trap = n_traps_serial; i_trap < n_traps_serial + serial_n_traps_sc;
         i_trap++) {
        serial_traps_sc[i_trap - n_traps_serial] = TrapSlowCapture(
            serial_trap_densities[i_trap], serial_trap_release_timescales[i_trap],
            serial_trap_third_params[i_trap]);
    }
    n_traps_serial += serial_n_traps_sc;
    for (int i_trap = n_traps_serial; i_trap < n_traps_serial + serial_n_traps_ic_co;
         i_trap++) {
        serial_traps_continuum[i_trap - n_traps_serial] = TrapInstantCaptureContinuum(
            serial_trap_densities[i_trap], serial_trap_release_timescales[i_trap],
            serial_trap_third_params[i_trap]);
    }
    n_traps_serial += serial_n_traps_ic_co;
    for (int i_trap = n_traps_serial; i_trap < n_traps_serial + serial_n_traps_sc_co;
         i_trap++) {
        serial_traps_sc_co[i_trap - n_traps_serial] = TrapSlowCaptureContinuum(
            serial_trap_densities[i_trap], serial_trap_release_timescales[i_trap],
            serial_trap_third_params[i_trap], serial_trap_fourth_params[i_trap]);
    }
    n_traps_serial += serial_n_traps_sc_co;

    // Misc 
    //std::valarray<double> parallel_dwell_times(0.0, parallel_n_steps);
    //for (int i_step = 0; i_step < parallel_n_steps; i_step++) {
    //    parallel_dwell_times[i_step] = parallel_dwell_times_in[i_step];
    //}
    //double parallel_prune_n_electrons = parallel_prune_n_electrons_in[0];
    //parallel_prune_n_electronss[0] = parallel_prune_n_electrons;
    //double serial_prune_n_electrons = serial_prune_n_electrons_in[0];
    //serial_prune_n_electronss[0] = serial_prune_n_electrons;
    double prune_zero = 0;
    
    // ========
    // Add CTI
    // ========
    std::valarray<std::valarray<double> > image_out;
    // No parallel, serial only
    if (n_traps_parallel == 0) {
        image_out = add_cti(
            image_in,
            // Parallel
            nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 
            0, 0, parallel_window_start, parallel_window_stop, 0, 0, prune_zero, 0,
            // Serial
            p_serial_roe, &serial_ccd, &serial_traps_ic, &serial_traps_sc,
            &serial_traps_continuum, &serial_traps_sc_co, 
            serial_express, serial_offset,
            serial_window_start, serial_window_stop, 
            serial_time_start, serial_time_stop,
            serial_prune_n_electrons[0], serial_prune_frequency,
            // Combined
            allow_negative_pixels, 
            // Output
            verbosity, iteration);
    }
    // No serial, parallel only
    else if (n_traps_serial == 0) {
        image_out = add_cti(
            image_in,
            // Parallel
            p_parallel_roe, &parallel_ccd, &parallel_traps_ic, &parallel_traps_sc,
            &parallel_traps_continuum, &parallel_traps_sc_co, 
            parallel_express, parallel_offset, 
            parallel_window_start, parallel_window_stop,
            parallel_time_start, parallel_time_stop,
            parallel_prune_n_electrons[0], parallel_prune_frequency,
            // Serial
            nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 
            0, 0, serial_window_start, serial_window_stop, 0, 0, prune_zero, 0, 
            // Combined
            allow_negative_pixels, 
            // Output
            verbosity, iteration);
    }
    // Parallel and serial
    else {
        image_out = add_cti(
            image_in,
            // Parallel
            p_parallel_roe, &parallel_ccd, 
            &parallel_traps_ic, &parallel_traps_sc, &parallel_traps_continuum, &parallel_traps_sc_co, 
            parallel_express, parallel_offset, 
            parallel_window_start, parallel_window_stop,
            parallel_time_start, parallel_time_stop,
            parallel_prune_n_electrons[0], parallel_prune_frequency,
            // Serial
            p_serial_roe, &serial_ccd, 
            &serial_traps_ic, &serial_traps_sc, &serial_traps_continuum, &serial_traps_sc_co, 
            serial_express, serial_offset,
            serial_window_start, serial_window_stop,
            serial_time_start, serial_time_stop,
            serial_prune_n_electrons[0], serial_prune_frequency,
            // Combined
            allow_negative_pixels, 
            // Output
            verbosity, iteration);
    }

    // Delete serial/parallel ROE if previously allocated
    delete p_parallel_roe;
    delete p_serial_roe;

    // Convert the output image back to modify the input image array
    for (int i_row = 0; i_row < n_rows; i_row++) {
        for (int i_col = 0; i_col < n_columns; i_col++) {
            image[i_row * n_columns + i_col] = image_out[i_row][i_col];
        }
    }
}

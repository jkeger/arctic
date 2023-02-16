
#include "cti.hpp"
#include "roe.hpp"
#include "trap_managers.hpp"
#include "traps.hpp"
#include "util.hpp"

void print_array(double* array, int length);

void print_array_2D(double* array, int n_rows, int n_columns);

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
    bool serial_force_release_away_from_readout, bool serial_use_integer_express_matrix,
    int serial_n_pumps, int serial_roe_type,
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
    int verbosity, int iteration);

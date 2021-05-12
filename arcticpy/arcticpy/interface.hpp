
#include "cti.hpp"
#include "roe.hpp"
#include "trap_managers.hpp"
#include "traps.hpp"
#include "util.hpp"

void print_array(double* array, int length);

void print_array_2D(double* array, int n_rows, int n_columns);

void add_cti(
    double* image, int n_rows, int n_columns,
    // Traps
    double* trap_densities, double* trap_release_timescales,
    double* trap_capture_timescales, int n_traps_standard, int n_traps_instant_capture,
    // ROE
    double* dwell_times_in, int n_steps, bool empty_traps_between_columns,
    bool empty_traps_for_first_transfers, bool force_release_away_from_readout,
    bool use_integer_express_matrix,
    // CCD
    double* fraction_of_traps_per_phase_in, int n_phases, double* full_well_depths,
    double* well_notch_depths, double* well_fill_powers,
    // Misc
    int express, int offset, int start, int stop);

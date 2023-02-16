
cimport numpy as np
import numpy as np
from libcpp.string cimport string

cdef extern from "util.hpp":
    cdef string version_arctic()
    void print_version()

cdef extern from "interface.hpp":
    void print_array(double* array, int length)
    void print_array_2D(double* array, int n_rows, int n_columns)
    void add_cti(
        double* image,
        int n_rows,
        int n_columns,
        # ========
        # Parallel
        # ========
        # ROE
        double* parallel_dwell_times_in,
        int parallel_n_steps,
        int parallel_prescan_offset,
        int parallel_overscan_start,
        int parallel_empty_traps_between_columns,
        int parallel_empty_traps_for_first_transfers,
        int parallel_force_release_away_from_readout,
        int parallel_use_integer_express_matrix,
        int parallel_n_pumps,
        int parallel_roe_type,
        # CCD
        double* parallel_fraction_of_traps_per_phase_in,
        int parallel_n_phases,
        double* parallel_full_well_depths,
        double* parallel_well_notch_depths,
        double* parallel_well_fill_powers,
        double* parallel_first_electron_fills,
        # Traps
        double* parallel_trap_densities,
        double* parallel_trap_release_timescales,
        double* parallel_trap_third_params,
        double* parallel_trap_fourth_params,
        int parallel_n_traps_ic,
        int parallel_n_traps_sc,
        int parallel_n_traps_ic_co,
        int parallel_n_traps_sc_co,
        # Misc
        int parallel_express,
        int parallel_window_offset,
        int parallel_window_start,
        int parallel_window_stop,
        int parallel_time_start,
        int parallel_time_stop,
        double* parallel_prune_n_electrons, 
        int parallel_prune_frequency,
        # ========
        # Serial
        # ========
        # ROE
        double* serial_dwell_times_in,
        int serial_n_steps,
        int serial_prescan_offset,
        int serial_overscan_start,
        int serial_empty_traps_between_columns,
        int serial_empty_traps_for_first_transfers,
        int serial_force_release_away_from_readout,
        int serial_use_integer_express_matrix,
        int serial_n_pumps,
        int serial_roe_type,
        # CCD
        double* serial_fraction_of_traps_per_phase_in,
        int serial_n_phases,
        double* serial_full_well_depths,
        double* serial_well_notch_depths,
        double* serial_well_fill_powers,
        double* serial_first_electron_fills,
        # Traps
        double* serial_trap_densities,
        double* serial_trap_release_timescales,
        double* serial_trap_third_params,
        double* serial_trap_fourth_params,
        int serial_n_traps_ic,
        int serial_n_traps_sc,
        int serial_n_traps_ic_co,
        int serial_n_traps_sc_co,
        # Misc
        int serial_express,
        int serial_window_offset,
        int serial_window_start,
        int serial_window_stop,
        int serial_time_start,
        int serial_time_stop,
        double* serial_prune_n_electrons, 
        int serial_prune_frequency,
        # ========
        # Combined
        # ========
        int allow_negative_pixels,
        # Output
        int verbosity,
        int iteration
    )


def cy_print_version():
    print_version()

def cy_version_arctic():
    return version_arctic().decode("utf-8")

def check_contiguous(array):
    """ Make sure an array is contiguous and C-style. """
    if not array.flags['C_CONTIGUOUS']:
        return np.ascontiguousarray(array)
    else:
        return array


def cy_print_array(np.ndarray[np.double_t, ndim=1] array):
    array = check_contiguous(array)

    print_array(&array[0], array.shape[0])


def cy_print_array_2D(np.ndarray[np.double_t, ndim=2] array):
    array = check_contiguous(array)

    print_array_2D(&array[0, 0], array.shape[0], array.shape[1])


def cy_add_cti(
    np.ndarray[np.double_t, ndim=2] image,
    # ========
    # Parallel
    # ========
    # ROE
    np.ndarray[np.double_t, ndim=1] parallel_dwell_times,
    int parallel_prescan_offset,
    int parallel_overscan_start,
    int parallel_empty_traps_between_columns,
    int parallel_empty_traps_for_first_transfers,
    int parallel_force_release_away_from_readout,
    int parallel_use_integer_express_matrix,
    int parallel_n_pumps,
    int parallel_roe_type,
    # CCD
    np.ndarray[np.double_t, ndim=1] parallel_fraction_of_traps_per_phase,
    np.ndarray[np.double_t, ndim=1] parallel_full_well_depths,
    np.ndarray[np.double_t, ndim=1] parallel_well_notch_depths,
    np.ndarray[np.double_t, ndim=1] parallel_well_fill_powers,
    np.ndarray[np.double_t, ndim=1] parallel_first_electron_fills,
    # Traps
    np.ndarray[np.double_t, ndim=1] parallel_trap_densities,
    np.ndarray[np.double_t, ndim=1] parallel_trap_release_timescales,
    np.ndarray[np.double_t, ndim=1] parallel_trap_third_params,
    np.ndarray[np.double_t, ndim=1] parallel_trap_fourth_params,
    int parallel_n_traps_ic,
    int parallel_n_traps_sc,
    int parallel_n_traps_ic_co,
    int parallel_n_traps_sc_co,
    # Misc
    int parallel_express,
    int parallel_window_offset,
    int parallel_window_start,
    int parallel_window_stop,
    int parallel_time_start,
    int parallel_time_stop,
    np.ndarray[np.double_t, ndim=1] parallel_prune_n_electrons, 
    int parallel_prune_frequency,
    # ========
    # Serial
    # ========
    # ROE
    np.ndarray[np.double_t, ndim=1] serial_dwell_times,
    int serial_prescan_offset,
    int serial_overscan_start,
    int serial_empty_traps_between_columns,
    int serial_empty_traps_for_first_transfers,
    int serial_force_release_away_from_readout,
    int serial_use_integer_express_matrix,
    int serial_n_pumps,
    int serial_roe_type,
    # CCD
    np.ndarray[np.double_t, ndim=1] serial_fraction_of_traps_per_phase,
    np.ndarray[np.double_t, ndim=1] serial_full_well_depths,
    np.ndarray[np.double_t, ndim=1] serial_well_notch_depths,
    np.ndarray[np.double_t, ndim=1] serial_well_fill_powers,
    np.ndarray[np.double_t, ndim=1] serial_first_electron_fills,
    # Traps
    np.ndarray[np.double_t, ndim=1] serial_trap_densities,
    np.ndarray[np.double_t, ndim=1] serial_trap_release_timescales,
    np.ndarray[np.double_t, ndim=1] serial_trap_third_params,
    np.ndarray[np.double_t, ndim=1] serial_trap_fourth_params,
    int serial_n_traps_ic,
    int serial_n_traps_sc,
    int serial_n_traps_ic_co,
    int serial_n_traps_sc_co,
    # Misc
    int serial_express,
    int serial_window_offset,
    int serial_window_start,
    int serial_window_stop,
    int serial_time_start,
    int serial_time_stop,
    np.ndarray[np.double_t, ndim=1] serial_prune_n_electrons, 
    int serial_prune_frequency,
    # ========
    # Combined
    # ========
    int allow_negative_pixels,
    # Output
    int verbosity,
    int iteration,
):
    """
    Cython wrapper for arctic's add_cti() in src/cti.cpp.

    This wrapper passes the individual numbers and arrays extracted by the
    python wrapper to the C++ interface. See add_cti() in cti.py and add_cti()
    in interface.cpp.
    """
    image = check_contiguous(image)

    add_cti(
        &image[0, 0],
        image.shape[0],
        image.shape[1],
        # ========
        # Parallel
        # ========
        # ROE
        &parallel_dwell_times[0],
        len(parallel_dwell_times),
        parallel_prescan_offset,
        parallel_overscan_start,
        parallel_empty_traps_between_columns,
        parallel_empty_traps_for_first_transfers,
        parallel_force_release_away_from_readout,
        parallel_use_integer_express_matrix,
        parallel_n_pumps,
        parallel_roe_type,
        # CCD
        &parallel_fraction_of_traps_per_phase[0],
        len(parallel_fraction_of_traps_per_phase),
        &parallel_full_well_depths[0],
        &parallel_well_notch_depths[0],
        &parallel_well_fill_powers[0],
        &parallel_first_electron_fills[0],
        # Traps
        &parallel_trap_densities[0],
        &parallel_trap_release_timescales[0],
        &parallel_trap_third_params[0],
        &parallel_trap_fourth_params[0],
        parallel_n_traps_ic,
        parallel_n_traps_sc,
        parallel_n_traps_ic_co,
        parallel_n_traps_sc_co,
        # Misc
        parallel_express,
        parallel_window_offset,
        parallel_window_start,
        parallel_window_stop,
        parallel_time_start,
        parallel_time_stop,
        &parallel_prune_n_electrons[0], 
        parallel_prune_frequency,
        # ========
        # Serial
        # ========
        # ROE
        &serial_dwell_times[0],
        len(serial_dwell_times),
        serial_prescan_offset,
        serial_overscan_start,
        serial_empty_traps_between_columns,
        serial_empty_traps_for_first_transfers,
        serial_force_release_away_from_readout,
        serial_use_integer_express_matrix,
        serial_n_pumps,
        serial_roe_type,
        # CCD
        &serial_fraction_of_traps_per_phase[0],
        len(serial_fraction_of_traps_per_phase),
        &serial_full_well_depths[0],
        &serial_well_notch_depths[0],
        &serial_well_fill_powers[0],
        &serial_first_electron_fills[0],
        # Traps
        &serial_trap_densities[0],
        &serial_trap_release_timescales[0],
        &serial_trap_third_params[0],
        &serial_trap_fourth_params[0],
        serial_n_traps_ic,
        serial_n_traps_sc,
        serial_n_traps_ic_co,
        serial_n_traps_sc_co,
        # Misc
        serial_express,
        serial_window_offset,
        serial_window_start,
        serial_window_stop,
        serial_time_start,
        serial_time_stop,
        &serial_prune_n_electrons[0], 
        serial_prune_frequency,
        # ========
        # Combined
        # ========
        allow_negative_pixels,
        # Output
        verbosity,
        iteration
    )

    return image

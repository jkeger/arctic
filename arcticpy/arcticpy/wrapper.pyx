
cimport numpy as np
import numpy as np

cdef extern from "interface.hpp":
    void print_array(double* array, int length)
    void print_array_2D(double* array, int n_rows, int n_columns)
    void add_cti(
        double* image, 
        int n_rows, 
        int n_columns, 
        # Traps
        double trap_density, 
        double trap_lifetime, 
        # ROE
        double* dwell_times_in, 
        int n_steps, 
        int empty_traps_between_columns, 
        int empty_traps_for_first_transfers, 
        int force_release_away_from_readout, 
        int use_integer_express_matrix, 
        # CCD
        double* fraction_of_traps_per_phase_in, 
        int n_phases, 
        double* full_well_depths, 
        double* well_notch_depths, 
        double* well_fill_powers, 
        # Misc
        int express, 
        int offset, 
        int start, 
        int stop, 
    )


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
    # Traps
    double trap_density,
    double trap_lifetime,
    # ROE
    np.ndarray[np.double_t, ndim=1] dwell_times,
    int empty_traps_between_columns,
    int empty_traps_for_first_transfers,
    int force_release_away_from_readout,
    int use_integer_express_matrix,
    # CCD
    np.ndarray[np.double_t, ndim=1] fraction_of_traps_per_phase,
    np.ndarray[np.double_t, ndim=1] full_well_depths,
    np.ndarray[np.double_t, ndim=1] well_notch_depths,
    np.ndarray[np.double_t, ndim=1] well_fill_powers,
    # Misc
    int express,
    int offset,
    int start,
    int stop,
):
    image = check_contiguous(image)
    
    add_cti(
        &image[0, 0],
        image.shape[0],
        image.shape[1],
        # Traps
        trap_density,
        trap_lifetime,
        # &trap_densities[0],
        # len(trap_densities),
        # &trap_lifetimes[0],
        # len(trap_lifetimes),
        # &trap_instant_capture_densities[0],
        # len(trap_instant_capture_densities),
        # &trap_instant_capture_lifetimes[0],
        # len(trap_instant_capture_lifetimes),
        # ROE
        &dwell_times[0],
        len(dwell_times),
        empty_traps_between_columns,
        empty_traps_for_first_transfers,
        force_release_away_from_readout,
        use_integer_express_matrix,
        # CCD
        &fraction_of_traps_per_phase[0],
        len(fraction_of_traps_per_phase),
        &full_well_depths[0],
        &well_notch_depths[0],
        &well_fill_powers[0],
        # Misc
        express,
        offset,
        start,
        stop,
    )
    
    return image

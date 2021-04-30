
cimport numpy as np
import numpy as np

cdef extern from "interface.hpp":
    void print_array(double* array, int length)
    void print_array_2D(double* array, int n_rows, int n_columns)
    void test_add_cti(
        double* image, 
        int n_rows, 
        int n_columns, 
        double trap_density, 
        double trap_lifetime, 
        double dwell_time, 
        double ccd_full_well_depth, 
        double ccd_well_notch_depth, 
        double ccd_well_fill_power, 
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


def cy_test_add_cti(
    np.ndarray[np.double_t, ndim=2] image,
    double trap_density,
    double trap_lifetime,
    double dwell_time,
    double ccd_full_well_depth,
    double ccd_well_notch_depth,
    double ccd_well_fill_power,
    int express,
    int offset,
    int start,
    int stop,
):
    image = check_contiguous(image)
    
    test_add_cti(
        &image[0, 0],
        image.shape[0],
        image.shape[1],
        trap_density,
        trap_lifetime,
        dwell_time,
        ccd_full_well_depth,
        ccd_well_notch_depth,
        ccd_well_fill_power,
        express,
        offset,
        start,
        stop,
    )

    return image

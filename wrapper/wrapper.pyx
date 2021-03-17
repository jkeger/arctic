
import numpy as np
cimport numpy as np

cdef extern from "interface.hpp":
    double clamp_zero_one(double value)
    double clamp(double value, double minimum, double maximum)
    void print_array(double* array, int length)
    void print_array_2D(double* array, int n_rows, int n_columns)
    void print_2D_array(double* array, int n_rows, int n_columns)
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


def cy_clamp_zero_one(double value):
    return clamp_zero_one(value)

def cy_clamp(double value, double minimum, double maximum):
    return clamp(value, minimum, maximum)

def cy_print_array(np.ndarray[np.double_t, ndim=1] array):
    if not array.flags['C_CONTIGUOUS']:
        array = np.ascontiguousarray(array)
        
    cdef double[::1] array_memview = array
    
    print_array(&array_memview[0], array_memview.shape[0])

def cy_print_array_2D(np.ndarray[np.double_t, ndim=2] array):
    if not array.flags['C_CONTIGUOUS']:
        array = np.ascontiguousarray(array)

    cdef double[::1] flat_memview = array.flatten()
    
    print_array_2D(&flat_memview[0], array.shape[0], array.shape[1])

def cy_print_2D_array(np.ndarray[np.double_t, ndim=2] array):
    if not array.flags['C_CONTIGUOUS']:
        array = np.ascontiguousarray(array)

    cdef double[::1] flat_memview = array.flatten()
    
    print_2D_array(&flat_memview[0], array.shape[0], array.shape[1])

def cy_test_add_cti(
    np.ndarray[np.double_t, ndim=2] image_pre_cti,
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
    if not image_pre_cti.flags['C_CONTIGUOUS']:
        image_pre_cti = np.ascontiguousarray(image_pre_cti)

    cdef double[::1] flat_memview = image_pre_cti.flatten()
    
    test_add_cti(
        &flat_memview[0],
        image_pre_cti.shape[0],
        image_pre_cti.shape[1],
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
    
    return np.asarray(flat_memview)

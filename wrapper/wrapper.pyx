
import numpy as np
cimport numpy as np

cdef extern from "interface.hpp":
    double clamp_zero_one(double value)
    double clamp(double value, double minimum, double maximum)
    void print_array(double* array, int length)
    void print_array_2D(double* array, int n_rows, int n_columns)
    void print_2D_array(double* array, int n_rows, int n_columns)

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


import numpy as np
cimport numpy as np

cdef extern from "interface.hpp":
    double clamp_zero_one(double value)

def cy_clamp_zero_one(double value):
    return clamp_zero_one(value)


cdef extern from "interface.hpp":
    double clamp(double value, double minimum, double maximum)

def cy_clamp(double value, double minimum, double maximum):
    return clamp(value, minimum, maximum)


cdef extern from "interface.hpp":
    void print_array(double* array, int length)

def cy_print_array(np.ndarray[np.double_t, ndim=1] array):
    print_array(<double*> array.data, array.size)

# def cy_print_array(array):
#     if not array.flags['C_CONTIGUOUS']:
#         array = np.ascontiguousarray(array)
# 
#     cdef double[::1] array_memview = array
#     print_array(&array_memview[0], array_memview.shape[0])

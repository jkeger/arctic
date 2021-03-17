
cdef extern from "interface.hpp":
    double clamp_zero_one(double value)

cdef extern from "interface.hpp":
    double clamp(double value, double minimum, double maximum)

def cy_clamp_zero_one(double value):
    return clamp_zero_one(value)

def cy_clamp(double value, double minimum, double maximum):
    return clamp(value, minimum, maximum)

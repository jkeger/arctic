
from wrapper cimport clamp_zero_one

def cy_clamp_zero_one(double value):
    return clamp_zero_one(value)

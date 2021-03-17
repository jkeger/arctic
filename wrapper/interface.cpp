
#include <stdio.h>
#include <valarray>

#include "interface.hpp"

double clamp_zero_one(double value) {
    return clamp(value, 0.0, 1.0);
}

void print_array(double* array, int length) {
    std::valarray<double> varray(array, length);
    
    print_array(varray);
}

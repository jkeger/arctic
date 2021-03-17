
#include <stdio.h>
#include <valarray>

#include "interface.hpp"

/*
    Wrapper test.
*/
double clamp_zero_one(double value) {
    return clamp(value, 0.0, 1.0);
}

/*
    Wrapper for arctic print_array()
*/
void print_array(double* array, int length) {
    // Convert to a valarray
    std::valarray<double> varray(array, length);
    
    print_array(varray);
}

/*
    Wrappers for arctic print_array_2D()
*/
void print_array_2D(double* array, int n_rows, int n_columns) {
    // Convert to a valarray
    std::valarray<std::valarray<double>> 
        varray(std::valarray<double>(0.0, n_columns), n_rows);
    
    for (int i_row = 0; i_row < n_rows; i_row++) {
        for (int i_col = 0; i_col < n_columns; i_col++) {
            varray[i_row][i_col] = array[i_row * n_columns + i_col];
        }
    }
    
    print_array_2D(varray);
}

void print_2D_array(double* array, int n_rows, int n_columns) {
    // Convert to a valarray
    std::valarray<double> varray(array, n_rows * n_columns);
    
    print_array_2D(varray, n_columns);
}

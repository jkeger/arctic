
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
    // Convert to a 2D valarray
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

/*
    Test wrapping add_cti()
*/
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
    int stop) {
    // Convert to a 2D valarray
    std::valarray<std::valarray<double>> 
        image_pre_cti(std::valarray<double>(0.0, n_columns), n_rows);
    
    for (int i_row = 0; i_row < n_rows; i_row++) {
        for (int i_col = 0; i_col < n_columns; i_col++) {
            image_pre_cti[i_row][i_col] = image[i_row * n_columns + i_col];
        }
    }
    
    // CTI model parameters
    TrapInstantCapture trap(trap_density, trap_lifetime);
    std::valarray<std::valarray<Trap>> traps = {{}, {trap}};
    std::valarray<double> dwell_times = {dwell_time};
    ROE roe(dwell_times);
    CCD ccd(CCDPhase(ccd_full_well_depth, ccd_well_notch_depth, ccd_well_fill_power));
    
    // // Add parallel and serial CTI
    // std::valarray<std::valarray<double>> image_post_cti = add_cti(
    //     image_pre_cti, &roe, &ccd, &traps, express, offset, start, stop, &roe, &ccd,
    //     &traps, express, offset, start, stop);
    
    // Add parallel CTI
    std::valarray<std::valarray<double>> image_post_cti = add_cti(
        image_pre_cti, &roe, &ccd, &traps, express, offset, start, stop);
    
    // Convert the results back to modify the 1D input array
    for (int i_row = 0; i_row < n_rows; i_row++) {
        for (int i_col = 0; i_col < n_columns; i_col++) {
            image[i_row * n_columns + i_col] = image_post_cti[i_row][i_col];
        }
    }
}

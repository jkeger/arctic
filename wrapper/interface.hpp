
#include "cti.hpp"
#include "roe.hpp"
#include "trap_managers.hpp"
#include "traps.hpp"
#include "util.hpp"

void print_array(double* array, int length);

void print_array_2D(double* array, int n_rows, int n_columns);

void test_add_cti(
    double* image, int n_rows, int n_columns, double trap_density, double trap_lifetime,
    double dwell_time, double ccd_full_well_depth, double ccd_well_notch_depth,
    double ccd_well_fill_power, int express, int offset, int start, int stop);

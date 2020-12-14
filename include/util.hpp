
#ifndef ARCTIC_UTIL_HPP
#define ARCTIC_UTIL_HPP

#include <valarray>

void print_array(std::valarray<double>& array);

void print_array_2D(std::valarray<double>& image, int n_col);

std::valarray<double> arange(double start, double stop, double step = 1);

#endif  // ARCTIC_UTIL_HPP


#ifndef ARCTIC_UTIL_HPP
#define ARCTIC_UTIL_HPP

#include <valarray>
#include <vector>

double clamp(double value, double minimum, double maximum);

std::vector<double> flatten(std::valarray<std::valarray<double>>& array);

void print_array(std::valarray<double>& array);

void print_array_2D(std::valarray<double>& image, int n_col);

void print_array_2D(std::valarray<std::valarray<double>>& array);

std::valarray<double> arange(double start, double stop, double step = 1);

double gettimelapsed(struct timeval start, struct timeval end);

#endif  // ARCTIC_UTIL_HPP

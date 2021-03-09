
#ifndef ARCTIC_UTIL_HPP
#define ARCTIC_UTIL_HPP

#include <stdio.h>
#include <valarray>
#include <vector>

// ========
// Printing
// ========
/*
    Print an error message and exit.
*/
#define error(message, ...)                                                        \
    ({                                                                             \
        fflush(stdout);                                                            \
        fprintf(                                                                   \
            stderr, "%s:%s():%i: " message "\n", __FILE__, __FUNCTION__, __LINE__, \
            ##__VA_ARGS__);                                                        \
        exit(1);                                                                   \
    })

void print_array(std::valarray<double>& array);

void print_array_2D(std::valarray<double>& image, int n_col);

void print_array_2D(std::valarray<std::valarray<double>>& array);

// ========
// Arrays
// ========
std::vector<double> flatten(std::valarray<std::valarray<double>>& array);

std::valarray<double> arange(double start, double stop, double step = 1);

std::valarray<std::valarray<double>> transpose(
    std::valarray<std::valarray<double>>& array);

// ========
// I/O
// ========
std::valarray<std::valarray<double>> load_image_from_txt(char* filename);

void save_image_to_txt(char* filename, std::valarray<std::valarray<double>> image);

// ========
// Misc
// ========
double clamp(double value, double minimum, double maximum);

double gettimelapsed(struct timeval start, struct timeval end);

#endif  // ARCTIC_UTIL_HPP

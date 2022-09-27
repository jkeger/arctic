
#ifndef ARCTIC_UTIL_HPP
#define ARCTIC_UTIL_HPP

#include <string>
#include <stdio.h>
#include <string.h>

#include <valarray>
#include <vector>

// ==============
// Version number
// ==============
//extern std::string versionn;
std::string version_arctic();

// ========
// Printing
// ========
/*
    Global verbosity parameter to control the amount of printed information:

    0       No printing (except errors etc).
    1       Standard.
    2       Extra details.
*/
extern int verbosity;
void set_verbosity(int v);

/*
    Print if the global verbosity parameter is >= verbosity_min.

    If verbosity >= 2, also print the origin of the message.
*/
#define __FILENAME__ strrchr("/" __FILE__, '/') + 1
#define print_v(verbosity_min, message, ...)                                  \
    ({                                                                        \
        if (verbosity >= 2)                                                   \
            printf("%s:%i: " message, __FILENAME__, __LINE__, ##__VA_ARGS__); \
        else if (verbosity >= verbosity_min)                                  \
            printf(message, ##__VA_ARGS__);                                   \
    })

/*
    Print an error message, including its origin, and exit.
*/
#define error(message, ...)                                                            \
    ({                                                                                 \
        fflush(stdout);                                                                \
        fprintf(                                                                       \
            stderr, "%s:%s():%i: " message "\n", __FILENAME__, __FUNCTION__, __LINE__, \
            ##__VA_ARGS__);                                                            \
        exit(1);                                                                       \
    })

void print_version();

void print_array(std::valarray<double>& array);

void print_array_2D(std::valarray<double>& image, int n_col);

void print_array_2D(std::valarray<std::valarray<double> >& array);

// ========
// Arrays
// ========
std::vector<double> flatten(std::valarray<std::valarray<double> >& array);

std::valarray<double> arange(double start, double stop, double step = 1);

std::valarray<std::valarray<double> > transpose(
    std::valarray<std::valarray<double> >& array);

// ========
// I/O
// ========
std::valarray<std::valarray<double> > load_image_from_txt(const char* filename);

void save_image_to_txt(
    const char* filename, std::valarray<std::valarray<double> > image);

// ========
// Misc
// ========
double clamp(double value, double minimum, double maximum);

double gettimelapsed(struct timeval start, struct timeval end);

#endif  // ARCTIC_UTIL_HPP

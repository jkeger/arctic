
#include <stdio.h>
#include <valarray>

#include "cti.hpp"
#include "util.hpp"

int main() {
    // Test image
    int n_col = 2;
    int n_row = 6;
    std::valarray<double> image(0., n_row * n_col);
    image[0 * n_col + 1] = 1;
    image[1 * n_col + 2] = 1;

    print_array_2D(image, n_col);

    return 0;
}

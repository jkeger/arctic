
#ifndef ARCTIC_ROE_HPP
#define ARCTIC_ROE_HPP

#include <valarray>

std::valarray<double> express_matrix_from_pixels_and_express(
    int n_pixels, int express = 0, int offset = 0, bool integer_express_matrix = false,
    bool empty_traps_for_first_transfers = true);

#endif  // ARCTIC_ROE_HPP

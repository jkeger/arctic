
#ifndef ARCTIC_ROE_HPP
#define ARCTIC_ROE_HPP

#include <valarray>

class ROE {
   public:
    double dwell_time;
    bool empty_traps_for_first_transfers;
    bool integer_express_matrix;

    ROE(double dwell_time = 1.0, bool empty_traps_for_first_transfers = true,
        bool integer_express_matrix = false);
    ~ROE(){};

    std::valarray<double> express_matrix_from_pixels_and_express(
        int n_pixels, int express = 0, int offset = 0);
};

#endif  // ARCTIC_ROE_HPP

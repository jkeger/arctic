
#ifndef ARCTIC_ROE_HPP
#define ARCTIC_ROE_HPP

#include <valarray>

class ROE {
   public:
    ROE(double dwell_time = 1.0, bool empty_traps_between_columns = true,
        bool empty_traps_for_first_transfers = true,
        bool use_integer_express_matrix = false);
    ~ROE(){};

    std::valarray<double> express_matrix;
    std::valarray<bool> store_trap_states_matrix;

    double dwell_time;
    bool empty_traps_between_columns;
    bool empty_traps_for_first_transfers;
    bool use_integer_express_matrix;
    int n_express_passes;

    void set_express_matrix_from_pixels_and_express(
        int n_pixels, int express = 0, int offset = 0);
    void set_store_trap_states_matrix();
};

#endif  // ARCTIC_ROE_HPP

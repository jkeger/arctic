
#ifndef ARCTIC_ROE_HPP
#define ARCTIC_ROE_HPP

#include <valarray>

static std::valarray<double> dwell_times_default = {1.0};

class ROEStepPhase {
   public:
    ROEStepPhase(){};
    ROEStepPhase(
        bool is_high, std::valarray<int> capture_from_which_pixels,
        std::valarray<int> release_to_which_pixels,
        std::valarray<double> release_fraction_to_pixels);
    ~ROEStepPhase(){};

    bool is_high;
    std::valarray<int> capture_from_which_pixels;
    std::valarray<int> release_to_which_pixels;
    std::valarray<double> release_fraction_to_pixels;

    int n_capture_pixels;
    int n_release_pixels;
};

class ROE {
   public:
    ROE(std::valarray<double>& dwell_times_in = dwell_times_default,
        bool empty_traps_between_columns = true,
        bool empty_traps_for_first_transfers = true,
        bool force_release_away_from_readout = true,
        bool use_integer_express_matrix = false);
    ~ROE(){};

    std::valarray<double>& dwell_times;
    bool empty_traps_between_columns;
    bool empty_traps_for_first_transfers;
    bool force_release_away_from_readout;
    bool use_integer_express_matrix;

    std::valarray<double> express_matrix;
    std::valarray<bool> store_trap_states_matrix;
    std::valarray<std::valarray<ROEStepPhase>> clock_sequence;

    int n_steps;
    int n_phases;
    int n_express_passes;

    virtual void set_express_matrix_from_pixels_and_express(
        int n_pixels, int express = 0, int offset = 0);
    virtual void set_store_trap_states_matrix();
    virtual void set_clock_sequence();
};

class ROEChargeInjection : public ROE {
   public:
    ROEChargeInjection(
        std::valarray<double>& dwell_times_in = dwell_times_default,
        bool empty_traps_between_columns = true,
        bool force_release_away_from_readout = true,
        bool use_integer_express_matrix = false);
    ~ROEChargeInjection(){};

    void set_express_matrix_from_pixels_and_express(
        int n_pixels, int express = 0, int offset = 0);
    void set_store_trap_states_matrix();
};

#endif  // ARCTIC_ROE_HPP

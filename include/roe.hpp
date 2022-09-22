
#ifndef ARCTIC_ROE_HPP
#define ARCTIC_ROE_HPP

#include <valarray>

enum ROEType {
    roe_type_standard = 0,
    roe_type_charge_injection = 1,
    roe_type_trap_pumping = 2
};

static std::valarray<double> dwell_times_default = {1.0};
static std::valarray<double> dwell_times_trap_pumping_default = {0.5, 0.5};

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
    ROE(std::valarray<double>& dwell_times = dwell_times_default,
        int prescan_offset = 0,
        int overscan_start = -1,
        bool empty_traps_between_columns = true,
        bool empty_traps_for_first_transfers = false,
        bool force_release_away_from_readout = true,
        bool use_integer_express_matrix = false);
    virtual ~ROE(){};

    ROE& operator=(const ROE& roe);

    std::valarray<double>& dwell_times;
    int prescan_offset;
    int overscan_start;
    bool empty_traps_between_columns;
    bool empty_traps_for_first_transfers;
    bool force_release_away_from_readout;
    bool use_integer_express_matrix;

    std::valarray<double> express_matrix;
    std::valarray<bool> store_trap_states_matrix;
    std::valarray<std::valarray<ROEStepPhase> > clock_sequence;

    ROEType type;
    int n_steps;
    int n_phases;
    int n_express_passes;
    int n_pumps;

    virtual void set_express_matrix_from_rows_and_express(
        int n_rows, int express = 0, int window_offset = 0);
    virtual void set_store_trap_states_matrix();
    virtual void set_clock_sequence();
};

class ROEChargeInjection : public ROE {
   public:
    ROEChargeInjection(
        std::valarray<double>& dwell_times = dwell_times_default,
        int prescan_offset = 0,
        int overscan_start = -1,
        bool empty_traps_between_columns = true,
        bool force_release_away_from_readout = true,
        bool use_integer_express_matrix = false);
    virtual ~ROEChargeInjection(){};

    void set_express_matrix_from_rows_and_express(
        int n_rows, int express = 0, int window_offset = 0);
    void set_store_trap_states_matrix();
};

class ROETrapPumping : public ROE {
   public:
    ROETrapPumping(
        std::valarray<double>& dwell_times = dwell_times_trap_pumping_default,
        int n_pumps = 1, 
        bool empty_traps_for_first_transfers = true,
        bool use_integer_express_matrix = false);
    virtual ~ROETrapPumping(){};

    void set_express_matrix_from_rows_and_express(
        int n_rows, int express = 0, int window_offset = 0);
    void set_store_trap_states_matrix();
};

#endif  // ARCTIC_ROE_HPP

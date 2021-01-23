
#ifndef ARCTIC_TRAP_MANAGERS_HPP
#define ARCTIC_TRAP_MANAGERS_HPP

#include <valarray>

#include "traps.hpp"

class TrapManager {
   public:
    TrapManager(){};
    TrapManager(std::valarray<Trap> traps, int max_n_transfers);
    ~TrapManager(){};

    std::valarray<Trap> traps;
    std::valarray<double> watermarks;

    int n_traps;
    int max_n_transfers;
    int n_watermarks_per_transfer;
    int n_watermarks;
    int n_col_wmk;
    int n_active_watermarks;
    double empty_watermark;
    double filled_watermark;

    std::valarray<double> fill_probabilities_from_empty;
    std::valarray<double> fill_probabilities_from_full;
    std::valarray<double> fill_probabilities_from_release;

    void initialise_watermarks();
    void set_fill_probabilities_from_dwell_time(double dwell_time);
    double n_trapped_electrons_from_watermarks(std::valarray<double> wmks);
};

class TrapManagerInstantCapture : public TrapManager {
   public:
    TrapManagerInstantCapture(std::valarray<Trap> traps, int max_n_transfers);
    ~TrapManagerInstantCapture(){};

    double n_electrons_released();
};

#endif  // ARCTIC_TRAP_MANAGERS_HPP

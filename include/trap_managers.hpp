
#ifndef ARCTIC_TRAP_MANAGERS_HPP
#define ARCTIC_TRAP_MANAGERS_HPP

#include <valarray>

#include "traps.hpp"

class TrapManager {
   public:
    std::valarray<Trap> traps;
    std::valarray<double> watermarks;

    int n_traps;
    int max_n_transfers;
    int n_watermarks_per_transfer;
    int n_watermarks;
    int n_wmk_col;
    double empty_watermark;
    double filled_watermark;

    TrapManager(){};
    TrapManager(std::valarray<Trap> traps, int max_n_transfers);
    ~TrapManager(){};

    void initialise_watermarks();
    double n_trapped_electrons_from_watermarks(std::valarray<double> wmks);
};

class TrapManagerInstantCapture : public TrapManager {
   public:
    TrapManagerInstantCapture(std::valarray<Trap> traps, int max_n_transfers);
    ~TrapManagerInstantCapture(){};
};

#endif  // ARCTIC_TRAP_MANAGERS_HPP

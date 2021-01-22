
#ifndef ARCTIC_TRAP_MANAGERS_HPP
#define ARCTIC_TRAP_MANAGERS_HPP

#include <valarray>

#include "traps.hpp"

class TrapManager {
   public:
    std::valarray<Trap> traps;
    std::valarray<std::valarray<double>> watermarks;

    int n_traps;
    int max_n_transfers;
    int n_watermarks_per_transfer;
    double empty_watermark;
    double filled_watermark;

    TrapManager(){};
    TrapManager(std::valarray<Trap> traps, int max_n_transfers);
    ~TrapManager(){};
};

class TrapManagerInstantCapture : public TrapManager {
   public:
    TrapManagerInstantCapture(std::valarray<Trap> traps, int max_n_transfers);
    ~TrapManagerInstantCapture(){};
};

#endif  // ARCTIC_TRAP_MANAGERS_HPP


#ifndef ARCTIC_TRAP_MANAGERS_HPP
#define ARCTIC_TRAP_MANAGERS_HPP

#include <valarray>

#include "ccd.hpp"
#include "traps.hpp"

class TrapManagerBase {
   public:
    TrapManagerBase(){};
    TrapManagerBase(int max_n_transfers, CCDPhase ccd_phase);
    ~TrapManagerBase(){};

    int max_n_transfers;
    CCDPhase ccd_phase;

    std::valarray<double> watermark_volumes;
    std::valarray<double> watermark_fills;
    std::valarray<double> stored_watermark_volumes;
    std::valarray<double> stored_watermark_fills;

    int n_traps;
    double empty_watermark;
    int n_active_watermarks;
    int i_first_active_wmk;
    int n_watermarks_per_transfer;
    int n_watermarks;
    int stored_n_active_watermarks;
    int stored_i_first_active_wmk;

    std::valarray<double> trap_densities;
    std::valarray<double> fill_probabilities_from_empty;
    std::valarray<double> fill_probabilities_from_full;
    std::valarray<double> fill_probabilities_from_release;
    std::valarray<double> empty_probabilities_from_release;

    void initialise_trap_states();
    void reset_trap_states();
    void store_trap_states();
    void restore_trap_states();

    virtual double n_trapped_electrons_from_watermarks(
        std::valarray<double> wmk_volumes, std::valarray<double> wmk_fills);
    int watermark_index_above_cloud(double cloud_fractional_volume);
};

class TrapManager : public TrapManagerBase {
   public:
    TrapManager(){};
    TrapManager(std::valarray<Trap> traps, int max_n_transfers, CCDPhase ccd_phase);
    ~TrapManager(){};

    std::valarray<Trap> traps;

    void set_fill_probabilities_from_dwell_time(double dwell_time);
    virtual double n_electrons_released_and_captured(double n_free_electrons);
};

class TrapManagerInstantCapture : public TrapManagerBase {
   public:
    TrapManagerInstantCapture(){};
    TrapManagerInstantCapture(
        std::valarray<TrapInstantCapture> traps, int max_n_transfers,
        CCDPhase ccd_phase);
    ~TrapManagerInstantCapture(){};

    std::valarray<TrapInstantCapture> traps;

    void set_fill_probabilities_from_dwell_time(double dwell_time);
    double n_electrons_released();
    void update_watermarks_capture(
        double cloud_fractional_volume, int i_wmk_above_cloud);
    void update_watermarks_capture_not_enough(
        double cloud_fractional_volume, int i_wmk_above_cloud, double enough);
    double n_electrons_captured(double n_free_electrons);
    double n_electrons_released_and_captured(double n_free_electrons);
};

class TrapManagerContinuum : public TrapManagerBase {
   public:
    TrapManagerContinuum(){};
    TrapManagerContinuum(
        std::valarray<TrapContinuum> traps, int max_n_transfers, CCDPhase ccd_phase);
    ~TrapManagerContinuum(){};

    std::valarray<TrapContinuum> traps;

    virtual double n_trapped_electrons_from_watermarks(
        std::valarray<double> wmk_volumes, std::valarray<double> wmk_fills);
};

class TrapManagerManager {
   public:
    TrapManagerManager(){};
    TrapManagerManager(
        std::valarray<Trap>& standard_traps,
        std::valarray<TrapInstantCapture>& instant_capture_traps,
        int max_n_transfers, CCD ccd, std::valarray<double>& dwell_times);
    ~TrapManagerManager(){};

    std::valarray<Trap> standard_traps;
    std::valarray<TrapInstantCapture> instant_capture_traps;
    int max_n_transfers;
    CCD ccd;

    int n_standard_traps;
    int n_instant_capture_traps;
    int n_continuum_traps;
    std::valarray<TrapManager> trap_managers_standard;
    std::valarray<TrapManagerInstantCapture> trap_managers_instant_capture;

    void reset_trap_states();
    void store_trap_states();
    void restore_trap_states();
};

#endif  // ARCTIC_TRAP_MANAGERS_HPP

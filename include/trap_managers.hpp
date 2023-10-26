
#ifndef ARCTIC_TRAP_MANAGERS_HPP
#define ARCTIC_TRAP_MANAGERS_HPP

#include <valarray>

#include "ccd.hpp"
#include "traps.hpp"

class TrapManagerBase {
   public:
    TrapManagerBase(){};
    TrapManagerBase(int max_n_transfers, CCDPhase ccd_phase, double dwell_time);
    ~TrapManagerBase(){};

    int max_n_transfers;
    CCDPhase ccd_phase;
    double dwell_time;

    std::valarray<double> watermark_volumes;
    std::valarray<double> watermark_fills;
    std::valarray<double> stored_watermark_volumes;
    std::valarray<double> stored_watermark_fills;

    int n_traps;
    double empty_watermark;
    double zeroth_watermark;
    int n_active_watermarks;
    int i_first_active_wmk;
    int n_watermarks_per_transfer;
    int n_watermarks;
    int stored_n_active_watermarks;
    int stored_i_first_active_wmk;
    void prune_watermarks(double min_n_electrons = 0);
    void lower_zeroth_watermark(double min_n_electrons = 0);

    std::valarray<double> trap_densities;

    void initialise_trap_states();
    void reset_trap_states();
    void store_trap_states();
    void restore_trap_states();
    virtual void setup();

    virtual double n_trapped_electrons_in_watermark(int i_wmk);
    virtual std::valarray<double> n_trapped_electrons_per_watermark();
    virtual double n_trapped_electrons_total();
    virtual double n_trapped_electrons_from_watermarks(
        std::valarray<double> wmk_volumes, std::valarray<double> wmk_fills);
    int watermark_index_above_cloud(double cloud_fractional_volume);
    virtual double n_electrons_released_from_wmk_above_cloud(int i_wmk);
};

class TrapManagerInstantCapture : public TrapManagerBase {
   public:
    TrapManagerInstantCapture(){};
    TrapManagerInstantCapture(
        std::valarray<TrapInstantCapture> traps, int max_n_transfers,
        CCDPhase ccd_phase, double dwell_time);
    ~TrapManagerInstantCapture(){};

    std::valarray<TrapInstantCapture> traps;
    std::valarray<double> empty_probabilities_from_release;

    void set_fill_probabilities();
    void setup();

    bool any_non_uniform_traps;

    double n_electrons_released();
    void update_watermarks_capture(
        double cloud_fractional_volume, int i_wmk_above_cloud);
    void update_watermarks_capture_not_enough(
        double cloud_fractional_volume, int i_wmk_above_cloud, double enough);
    double n_electrons_captured(double n_free_electrons);
    double n_electrons_released_and_captured(double n_free_electrons);
    double n_electrons_released_from_wmk_above_cloud(int i_wmk);
};

class TrapManagerSlowCapture : public TrapManagerBase {
   public:
    TrapManagerSlowCapture(){};
    TrapManagerSlowCapture(
        std::valarray<TrapSlowCapture> traps, int max_n_transfers, CCDPhase ccd_phase,
        double dwell_time);
    ~TrapManagerSlowCapture(){};

    std::valarray<TrapSlowCapture> traps;

    std::valarray<double> empty_probabilities_from_release;
    std::valarray<double> fill_probabilities_from_empty;
    std::valarray<double> fill_probabilities_from_full;

    void set_fill_probabilities();
    void setup();

    double n_electrons_released_and_captured(double n_free_electrons);
    double n_electrons_released_from_wmk_above_cloud(int i_wmk);
};

class TrapManagerInstantCaptureContinuum : public TrapManagerBase {
   public:
    TrapManagerInstantCaptureContinuum(){};
    TrapManagerInstantCaptureContinuum(
        std::valarray<TrapInstantCaptureContinuum> traps, int max_n_transfers,
        CCDPhase ccd_phase, double dwell_time);
    ~TrapManagerInstantCaptureContinuum(){};

    std::valarray<TrapInstantCaptureContinuum> traps;

    double time_min;
    double time_max;
    int n_intp;

    void prepare_interpolation_tables();
    void setup();

    double n_electrons_released();
    void update_watermarks_capture(
        double cloud_fractional_volume, int i_wmk_above_cloud);
    void update_watermarks_capture_not_enough(
        double cloud_fractional_volume, int i_wmk_above_cloud, double enough);
    double n_electrons_captured(double n_free_electrons);
    double n_electrons_released_and_captured(double n_free_electrons);
    double n_electrons_released_from_wmk_above_cloud(int i_wmk);
};

class TrapManagerSlowCaptureContinuum : public TrapManagerBase {
   public:
    TrapManagerSlowCaptureContinuum(){};
    TrapManagerSlowCaptureContinuum(
        std::valarray<TrapSlowCaptureContinuum> traps, int max_n_transfers,
        CCDPhase ccd_phase, double dwell_time);
    ~TrapManagerSlowCaptureContinuum(){};

    std::valarray<TrapSlowCaptureContinuum> traps;

    double time_min;
    double time_max;
    int n_intp;

    void prepare_interpolation_tables();
    void setup();

    double n_electrons_released_and_captured(double n_free_electrons);
    double n_electrons_released_from_wmk_above_cloud(int i_wmk);
};

class TrapManagerManager {
   public:
    TrapManagerManager(){};
    TrapManagerManager(
        std::valarray<TrapInstantCapture>& traps_ic,
        std::valarray<TrapSlowCapture>& traps_sc,
        std::valarray<TrapInstantCaptureContinuum>& traps_ic_co,
        std::valarray<TrapSlowCaptureContinuum>& traps_sc_co, int max_n_transfers,
        CCD ccd, std::valarray<double>& dwell_times);
    ~TrapManagerManager(){};

    std::valarray<TrapInstantCapture> traps_ic;
    std::valarray<TrapSlowCapture> traps_sc;
    std::valarray<TrapInstantCaptureContinuum> traps_ic_co;
    std::valarray<TrapSlowCaptureContinuum> traps_sc_co;
    int max_n_transfers;
    CCD ccd;

    int n_traps_ic;
    int n_traps_sc;
    int n_traps_ic_co;
    int n_traps_sc_co;
    std::valarray<TrapManagerInstantCapture> trap_managers_ic;
    std::valarray<TrapManagerSlowCapture> trap_managers_sc;
    std::valarray<TrapManagerInstantCaptureContinuum> trap_managers_ic_co;
    std::valarray<TrapManagerSlowCaptureContinuum> trap_managers_sc_co;

    void reset_trap_states();
    void store_trap_states();
    void restore_trap_states();
    void prune_watermarks(double min_n_electrons = 0);
};

#endif  // ARCTIC_TRAP_MANAGERS_HPP

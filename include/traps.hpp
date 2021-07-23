
#ifndef ARCTIC_TRAPS_HPP
#define ARCTIC_TRAPS_HPP

#include <gsl/gsl_integration.h>

#include <valarray>

class TrapInstantCapture {
   public:
    TrapInstantCapture(
        double density, double release_timescale,
        double fractional_volume_none_exposed = 0.0,
        double fractional_volume_full_exposed = 0.0);
    ~TrapInstantCapture(){};

    double density;

    double release_timescale;
    double release_rate;

    virtual double fill_fraction_from_time_elapsed(double time_elapsed);

    double fractional_volume_none_exposed;
    double fractional_volume_full_exposed;

    double fraction_traps_exposed_per_fractional_volume(
        double fractional_volume_low, double fractional_volume_high);
};

class TrapSlowCapture : public TrapInstantCapture {
   public:
    TrapSlowCapture(double density, double release_timescale, double capture_timescale);
    ~TrapSlowCapture(){};

    double capture_timescale;
    double capture_rate;
};

class TrapInstantCaptureContinuum : public TrapInstantCapture {
   public:
    TrapInstantCaptureContinuum(
        double density, double release_timescale, double release_timescale_sigma);
    ~TrapInstantCaptureContinuum(){};

    double release_timescale_sigma;

    virtual double fill_fraction_from_time_elapsed(
        double time_elapsed, gsl_integration_workspace* workspace = nullptr);
    double time_elapsed_from_fill_fraction(
        double fill_fraction, double time_max,
        gsl_integration_workspace* workspace = nullptr);

    std::valarray<double> fill_fraction_table;
    int n_intp;
    double time_min;
    double time_max;
    double fill_min;
    double fill_max;
    double d_log_time;

    void prep_fill_fraction_and_time_elapsed_tables(
        double time_min, double time_max, int n_intp = 1000);
    double fill_fraction_from_time_elapsed_table(double time_elapsed);
    double time_elapsed_from_fill_fraction_table(double fill_fraction);
};

class TrapSlowCaptureContinuum : public TrapInstantCapture {
   public:
    TrapSlowCaptureContinuum(
        double density, double release_timescale, double release_timescale_sigma,
        double capture_timescale);
    ~TrapSlowCaptureContinuum(){};

    double release_timescale_sigma;
    double capture_timescale;
    double capture_rate;

    virtual double fill_fraction_from_time_elapsed(
        double time_elapsed, gsl_integration_workspace* workspace = nullptr);
    double time_elapsed_from_fill_fraction(
        double fill_fraction, double time_max,
        gsl_integration_workspace* workspace = nullptr);

    std::valarray<double> fill_fraction_table;
    int n_intp;
    double time_min;
    double time_max;
    double fill_min;
    double fill_max;
    double d_log_time;

    void prep_fill_fraction_and_time_elapsed_tables(
        double time_min, double time_max, int n_intp);
    double fill_fraction_from_time_elapsed_table(double time_elapsed);
    double time_elapsed_from_fill_fraction_table(double fill_fraction);

    double fill_fraction_after_slow_capture(
        double time_elapsed, double dwell_time,
        gsl_integration_workspace* workspace = nullptr);

    std::valarray<double> fill_fraction_capture_table;
    double fill_capture_min;
    double fill_capture_max;
    double fill_capture_long_time;

    void prep_fill_fraction_after_slow_capture_tables(
        double dwell_time, double time_min, double time_max, int n_intp);
    double fill_fraction_after_slow_capture_table(double time_elapsed);
};

#endif  // ARCTIC_TRAPS_HPP

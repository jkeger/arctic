
#include <gsl/gsl_errno.h>
#include <gsl/gsl_integration.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_roots.h>
#include <math.h>

#include "traps.hpp"
#include "util.hpp"

/*
    The number of watermark types available. i.e. the number of WatermarkType
    enum options.
*/
int n_watermark_types = 3;

// ========
// Trap::
// ========
/*
    Class Trap.

    A single trap species.

    Controls the density of traps and the timescales/probabilities of
    capture and release, along with utilities for the watermarking tracking
    of trap states and the calculation of capture and release.

    Parameters
    ----------
    density : double
        The density of the trap species in a pixel.

    release_timescale : double
        The release timescale of the trap, in the same units as the time
        spent in each pixel or phase.

    capture_timescale : double
        The capture timescale of the trap. Default 0 for instant capture.

    Attributes
    ----------
    watermark_type : WatermarkType : enum int
        The flag for the type of watermarks required for this type of trap. Used
        to set up the trap managers etc.

    emission_rate, capture_rate : double
        The emission and capture rates (Lindegren (1998) section 3.2).
*/
Trap::Trap(double density, double release_timescale, double capture_timescale)
    : density(density),
      release_timescale(release_timescale),
      capture_timescale(capture_timescale) {

    watermark_type = watermark_type_standard;

    emission_rate = 1.0 / release_timescale;
    if (capture_timescale != 0.0)
        capture_rate = 1.0 / capture_timescale;
    else
        capture_rate = 0.0;
}

/*
    Calculate the fraction of filled traps after an amount of elapsed time.

    Parameters
    ----------
    time_elapsed : double
        The total time elapsed since the traps were filled, in the same
        units as the trap timescales.

    Returns
    -------
    fill_fraction : double
        The remaining fraction of filled traps.
*/
double Trap::fill_fraction_from_time_elapsed(double time_elapsed) {
    return exp(-time_elapsed / release_timescale);
}

// ========
// TrapInstantCapture::
// ========
/*
    Class TrapInstantCapture.

    For the old release-then-instant-capture algorithm.

    Parameters
    ----------
    density : double
        The density of the trap species in a pixel.

    release_timescale : double
        The release timescale of the trap, in the same units as the time
        spent in each pixel or phase.

    Attributes
    ----------
    emission_rate : double
        The emission rate (Lindegren (1998) section 3.2).
*/
TrapInstantCapture::TrapInstantCapture(double density, double release_timescale)
    : Trap(density, release_timescale, 0.0) {

    watermark_type = watermark_type_instant_capture;
}

// ========
// TrapContinuum::
// ========
/*
    Class TrapContinuum.

    For a trap species with a continuum (log-normal distribution) of release
    timescales, and instant capture.

    i.e. Density as function of release timecsale is set by:
        exp[-(log(tau) - log(mu))^2 / (2*sigma^2)] / (tau * sigma * sqrt(2*pi))

    Parameters
    ----------
    density : double
        The density of the trap species in a pixel.

    release_timescale : double
        The median release timescale of the traps, in the same units as the time
        spent in each pixel or phase.

    release_timescale_sigma : double
        The sigma of release lifetimes of the traps.

    Attributes
    ----------
    emission_rate : double
        The emission rate (Lindegren (1998) section 3.2).
*/
TrapContinuum::TrapContinuum(
    double density, double release_timescale, double release_timescale_sigma)
    : Trap(density, release_timescale, 0.0),
      release_timescale_sigma(release_timescale_sigma) {

    watermark_type = watermark_type_continuum;
}

/*
    Calculate the fraction of filled traps after an amount of elapsed time.

    Found by integrating the trap fill fraction (exp[-t/tau]) multiplied by the
    trap density distribution with trap release timescales.
    
    (www.gnu.org/software/gsl/doc/html/integration.html#c.gsl_integration_qagiu)

    Parameters
    ----------
    time_elapsed : double
        The total time elapsed since the traps were filled, in the same
        units as the trap timescales.

    Returns
    -------
    fill_fraction : double
        The fraction of filled traps.
*/
struct ff_from_te_params {
    double time_elapsed;
    double mu;
    double sigma;
};

double ff_from_te_integrand(double tau, void* params) {
    struct ff_from_te_params* p = (struct ff_from_te_params*)params;

    // To integrate: e^(-time_elapsed / tau) * fill_fraction(tau) dtau
    return exp(-p->time_elapsed / tau) *
           exp(-pow(log(tau) - log(p->mu), 2) / (2 * p->sigma * p->sigma)) /
           (tau * p->sigma * sqrt(2 * M_PI));
}

double TrapContinuum::fill_fraction_from_time_elapsed(double time_elapsed) {
    // Prep the integration
    double result, error;
    const double min = 0.0;
    const double epsabs = 0.0;
    const double epsrel = 1e-6;
    const int limit = 100;
    gsl_integration_workspace* workspace = gsl_integration_workspace_alloc(limit);
    struct ff_from_te_params params = {time_elapsed, release_timescale,
                                       release_timescale_sigma};
    gsl_function F;
    F.function = &ff_from_te_integrand;
    F.params = &params;
    
    // Integrate F.function from min to +infinity
    int status = gsl_integration_qagiu(
        &F, min, epsabs, epsrel, limit, workspace, &result, &error);

    if (status) error("Integration failed, status %d", status);

    return result;
}

/*
    Calculate the amount of elapsed time from the fraction of filled traps.
    
    Found by finding where fill_fraction_from_time_elapsed(time_elapsed) is
    equal to the required fill_fraction, using a root finder.
    
    (www.gnu.org/software/gsl/doc/html/roots.html)

    Parameters
    ----------
    fill_fraction : double
        The fraction of filled traps.

    Returns
    -------
    time_elapsed : double
        The total time elapsed since the traps were filled, in the same
        units as the trap timescales.
*/
struct te_from_ff_params {
    TrapContinuum* trap;
    double fill_fraction;
};

double te_from_ff_root_function(double time_elapsed, void* params) {
    struct te_from_ff_params* p = (struct te_from_ff_params*)params;

    // To find time such that: fill_fraction(time) - fill_fraction = 0
    return p->trap->fill_fraction_from_time_elapsed(time_elapsed) - p->fill_fraction;
}

double TrapContinuum::time_elapsed_from_fill_fraction(double fill_fraction) {
    // Completely full or empty
    if (fill_fraction == 1.0) return 0.0;
    if (fill_fraction == 0.0) return std::numeric_limits<double>::max();

    // Prep the root finder
    double root;
    int status;
    const int max_iter = 100;
    const double epsabs = 0.0, epsrel = 1e-6;
    const gsl_root_fsolver_type* T;
    gsl_root_fsolver* s;
    T = gsl_root_fsolver_brent;
    s = gsl_root_fsolver_alloc(T);
    struct te_from_ff_params params = {this, fill_fraction};
    gsl_function F;
    F.function = &te_from_ff_root_function;
    F.params = &params;

    // Bounding values
    double x_lo = 0.0, x_hi = 999.0;  //## set using dwell time etc

    // Iterate the root finder
    int iter = 0;
    gsl_root_fsolver_set(s, &F, x_lo, x_hi);
    do {
        iter++;
        status = gsl_root_fsolver_iterate(s);
        root = gsl_root_fsolver_root(s);
        x_lo = gsl_root_fsolver_x_lower(s);
        x_hi = gsl_root_fsolver_x_upper(s);
        status = gsl_root_test_interval(x_lo, x_hi, epsabs, epsrel);
    } while (status == GSL_CONTINUE && iter < max_iter);

    return root;
}

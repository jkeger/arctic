
#include <gsl/gsl_integration.h>
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
// Input parameters for the integrand
struct ff_from_te_params {
    double t;
    double mu;
    double sigma;
};

// Integrand function
double ff_from_te_integrand(double tau, void* params) {
    struct ff_from_te_params* p = (struct ff_from_te_params*)params;

    return exp(-p->t / tau) *
           exp(-pow(log(tau) - log(p->mu), 2) / (2 * p->sigma * p->sigma)) /
           (tau * p->sigma * sqrt(2 * M_PI));
}

double TrapContinuum::fill_fraction_from_time_elapsed(double time_elapsed) {
    // Set the parameter values
    struct ff_from_te_params params = {time_elapsed, release_timescale,
                                       release_timescale_sigma};

    // Prep the integration
    double result, error;
    const double min = 0;
    const double epsabs = 1e-6;
    const double epsrel = 1e-6;
    const int limit = 100;
    gsl_integration_workspace* workspace = gsl_integration_workspace_alloc(limit);
    gsl_function F;
    F.function = &ff_from_te_integrand;
    F.params = &params;

    // Integrate F.function from min to +infinity
    int ret = gsl_integration_qagiu(
        &F, min, epsabs, epsrel, limit, workspace, &result, &error);

    if (ret) error("Integration failed, status %d", ret);

    return result;
}

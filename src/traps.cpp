
#include "traps.hpp"

#include <gsl/gsl_errno.h>
#include <gsl/gsl_integration.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_roots.h>
#include <math.h>

#include <valarray>
#include <limits>

#include "util.hpp"

// ========
// TrapInstantCapture::
// ========
/*
    Class TrapInstantCapture.

    For the standard release-then-instant-capture algorithm.

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

    fractional_volume_none_exposed : double (opt.)
    fractional_volume_full_exposed : double (opt.)
        To allow for a non-uniform distribution with volume: the fractional
        volume of a pixel below which no traps are exposed and above which traps
        are fully exposed, respectively. Default to 0.0, 0.0 for uniformly
        distributed traps. Between the two, the density of traps is set to
        increase linearly. Or set both to the same (non-zero) value for a step
        function of no traps below that volume and uniformly distributed traps
        above it. See fill_fraction_from_time_elapsed().

    Attributes
    ----------
    release_rate : double
        The release (or emission) rate (Lindegren (1998) section 3.2).
*/
TrapInstantCapture::TrapInstantCapture(
    double density, double release_timescale, double fractional_volume_none_exposed,
    double fractional_volume_full_exposed)
    : density(density),
      release_timescale(release_timescale),
      fractional_volume_none_exposed(fractional_volume_none_exposed),
      fractional_volume_full_exposed(fractional_volume_full_exposed) {

    release_rate = 1.0 / release_timescale;

    // For a non-uniform distribution with volume, modify the density by the
    // maximum fraction of exposable traps, i.e. the integration over the full
    // volume of the fractional distribution, such that the trap's total density
    // per pixel is preserved.
    if (fractional_volume_full_exposed > 0.0)
        this->density /= 1.0 - 0.5 * (fractional_volume_none_exposed +
                                      fractional_volume_full_exposed);
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
double TrapInstantCapture::fill_fraction_from_time_elapsed(double time_elapsed) {
    return exp(-time_elapsed / release_timescale);
}

/*
    Calculate the fraction of traps exposed between two fractional volumes, to
    allow for a non-uniform distribution with volume.

    Simple idea but hard to put into words...

    Assume a linear increase with volume (v) in the fraction of traps between
    fractional_volume_none_exposed (a) and fractional_volume_full_exposed (a),
    i.e. f(v) = (v - a) / (b - a), capped between zero and one. Here integrate
    f(v) from fractional_volume_low (v_l) to high (v_h), then divide by the
    total fractional volume to get the right "units", as it were.

    Parameters
    ----------
    fractional_volume_low : double
    fractional_volume_high : double
        Two volumes (e.g. of a watermark) as a fraction of the pixel (or phase).

    Returns
    -------
    fraction_exposed_traps : double
        The fraction of traps exposed between these volumes, divided by the
        volume. For default evenly distributed traps, the fraction exposed and
        the fractional volume are the same, in which case this is always 1.0.
*/
double TrapInstantCapture::fraction_traps_exposed_per_fractional_volume(
    double fractional_volume_low, double fractional_volume_high) {

    if (fractional_volume_low >= fractional_volume_full_exposed)
        return 1.0;
    else if (fractional_volume_high <= fractional_volume_none_exposed)
        return 0.0;

    double fraction;
    // Integrate f(v) from v_l to v_h
    if (fractional_volume_none_exposed == fractional_volume_full_exposed) {
        // Step function. v_h - b
        fraction = fractional_volume_high - fractional_volume_full_exposed;
    } else {
        // v_l = max(v_l, a), v_h = min(v_h, b)
        // int_v_l^v_h f dv = (1/2 v_h^2 - a v_h - 1/2 v_l^2 + a v_l) / (b - a)
        double v_low = std::max(fractional_volume_low, fractional_volume_none_exposed);
        double v_high =
            std::min(fractional_volume_high, fractional_volume_full_exposed);
        fraction = (0.5 * v_high * v_high - fractional_volume_none_exposed * v_high -
                    0.5 * v_low * v_low + fractional_volume_none_exposed * v_low) /
                   (fractional_volume_full_exposed - fractional_volume_none_exposed);
        // + (v_h - b)
        if (fractional_volume_high > fractional_volume_full_exposed)
            fraction += fractional_volume_high - fractional_volume_full_exposed;
    }

    // Divide by the total fractional volume
    return fraction / (fractional_volume_high - fractional_volume_low);
}

// ========
// TrapSlowCapture::
// ========
/*
    Class TrapSlowCapture.

    For traps with a non-instant capture time.

    Parameters
    ----------
    density : double
        The density of the trap species in a pixel.

    release_timescale : double
        The release timescale of the trap, in the same units as the time
        spent in each pixel or phase.

    capture_timescale : double
        The capture timescale of the trap.

    Attributes
    ----------
    release_rate, capture_rate : double
        The release and capture rates (Lindegren (1998) section 3.2).
*/
TrapSlowCapture::TrapSlowCapture(
    double density, double release_timescale, double capture_timescale)
    : TrapInstantCapture(density, release_timescale),
      capture_timescale(capture_timescale) {

    if (capture_timescale != 0.0)
        capture_rate = 1.0 / capture_timescale;
    else
        capture_rate = 0.0;
}

// ========
// TrapInstantCaptureContinuum::
// ========
/*
    Class TrapInstantCaptureContinuum.

    For a trap species with a continuum (log-normal distribution) of release
    timescales, and instant capture.

    i.e. Density as function of release timecsale is set by:
    n(tau) = exp[-(log(tau) - log(mu))^2 / (2*sigma^2)] / (tau * sigma * sqrt(2*pi))

    Parameters
    ----------
    density : double
        The density of the trap species in a pixel.

    release_timescale : double
        The median release timescale of the traps, in the same units as the time
        spent in each pixel or phase.

    release_timescale_sigma : double
        The sigma of release lifetimes of the traps.
*/
TrapInstantCaptureContinuum::TrapInstantCaptureContinuum(
    double density, double release_timescale, double release_timescale_sigma)
    : TrapInstantCapture(density, release_timescale),
      release_timescale_sigma(release_timescale_sigma) {}

/*
    Calculate the fraction of filled traps after an amount of elapsed time.

    Found by integrating the trap fill fraction multiplied by the trap density
    distribution with trap release timescales:
        f = int_0^inf n(tau) exp(-t_e/tau) dtau

    Integrating from 0 to infinity can fail for very small mu and/or sigma, so
    instead integrate from 0 to (mu + 100 sigma) for a more reliable and very
    close approximation.

    (www.gnu.org/software/gsl/doc/html/integration.html#qag-adaptive-integration)

    Parameters
    ----------
    time_elapsed : double
        The total time elapsed since the traps were filled, in the same units
        as the trap timescales.

    workspace : gsl_integration_workspace* (opt.)
        An existing GSL workspace memory handler for the integration, or nullptr
        to create a new one.

    Returns
    -------
    fill_fraction : double
        The fraction of filled traps.
*/
struct TrICCo_ff_from_te_params {
    double time_elapsed;
    double mu;
    double sigma;
};

double TrICCo_ff_from_te_integrand(double tau, void* params) {
    struct TrICCo_ff_from_te_params* p = (struct TrICCo_ff_from_te_params*)params;

    // Distribution density
    double n = exp(-pow(log(tau) - log(p->mu), 2.0) / (2.0 * p->sigma * p->sigma)) /
               (tau * p->sigma * sqrt(2.0 * M_PI));

    // To integrate: n(tau) * fill_frac(tau) dtau
    return n * exp(-p->time_elapsed / tau);
}

double TrapInstantCaptureContinuum::fill_fraction_from_time_elapsed(
    double time_elapsed, gsl_integration_workspace* workspace) {
    // Completely full or empty, or unset watermark
    if (time_elapsed == 0.0)
        return 1.0;
    else if (time_elapsed >= std::numeric_limits<double>::max())
        return 0.0;
    else if (time_elapsed == -1.0)
        return 0.0;

    // Prep the integration
    double result, error;
    const double min = 0.0;
    const double max = release_timescale + 100 * release_timescale_sigma;
    const double epsabs = 0.0;
    const double epsrel = 1e-6;
    const int limit = 100;
    const int key = GSL_INTEG_GAUSS51;
    if (!workspace) {
        workspace = gsl_integration_workspace_alloc(limit);
    }
    struct TrICCo_ff_from_te_params params = {
        time_elapsed, release_timescale, release_timescale_sigma};
    gsl_function F;
    F.function = &TrICCo_ff_from_te_integrand;
    F.params = &params;

    // Integrate F.function from min to max
    int status = gsl_integration_qag(
        &F, min, max, epsabs, epsrel, limit, key, workspace, &result, &error);

    if (status) error("Integration failed, status %d", status);

    return result;
}

/*
    Calculate the amount of elapsed time from the fraction of filled traps.

    Found by finding where fill_fraction_from_time_elapsed(time_elapsed) is
    equal to the required fill_fraction, using a root finder:
        f(t_e) - f' = 0

    (www.gnu.org/software/gsl/doc/html/roots.html)

    Parameters
    ----------
    fill_fraction : double
        The fraction of filled traps.

    time_max : double
        The maximum possible time, used to initialise the root finder. e.g. the
        cumulative dwell time over all transfers.

    workspace : gsl_integration_workspace* (opt.)
        An existing GSL workspace memory handler for the integration, or nullptr
        to create a new one.

    Returns
    -------
    time_elapsed : double
        The total time elapsed since the traps were filled, in the same units
        as the trap timescales.
*/
struct TrICCo_te_from_ff_params {
    TrapInstantCaptureContinuum* trap;
    double fill_fraction;
    gsl_integration_workspace* workspace;
};

double TrICCo_te_from_ff_root_function(double time_elapsed, void* params) {
    struct TrICCo_te_from_ff_params* p = (struct TrICCo_te_from_ff_params*)params;

    // To find time such that: fill_fraction(time) - fill_fraction = 0
    return p->trap->fill_fraction_from_time_elapsed(time_elapsed, p->workspace) -
           p->fill_fraction;
}

double TrapInstantCaptureContinuum::time_elapsed_from_fill_fraction(
    double fill_fraction, double time_max, gsl_integration_workspace* workspace) {
    // Completely full or empty, or unset watermark
    if (fill_fraction == 1.0)
        return 0.0;
    else if (fill_fraction == 0.0)
        return std::numeric_limits<double>::max();
    else if (fill_fraction == -1.0)
        return 0.0;

    // Prep for the integration
    if (!workspace) {
        const int limit = 100;
        workspace = gsl_integration_workspace_alloc(limit);
    }

    // Prep the root finder
    double root;
    int status;
    const int max_iter = 100;
    const double epsabs = 0.0;
    const double epsrel = 1e-6;
    const gsl_root_fsolver_type* T;
    gsl_root_fsolver* s;
    T = gsl_root_fsolver_brent;
    s = gsl_root_fsolver_alloc(T);
    struct TrICCo_te_from_ff_params params = {this, fill_fraction, workspace};
    gsl_function F;
    F.function = &TrICCo_te_from_ff_root_function;
    F.params = &params;

    // Bounding values
    double x_lo = 0.0;
    double x_hi = time_max;

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

/*
    Prepare tables of fill fractions and elapsed times for interpolation.

    Use logarithmically spaced (decreasing) times to calculate the corresponding
    (monotonically increasing) table of fill fractions. So no need to actually
    store the elapsed times.

    Parameters
    ----------
    time_min : double
    time_max : double
        The minimum and maximum elapsed times to set the table limits, e.g. a
        single dwell time and the cumulative dwell time over all transfers.

    n_intp : int
        The number of interpolation values in the arrays.

    Sets
    ----
    fill_fraction_table : std::valarray<double>
        The array of fill fractions.

    fill_min : double
    fill_max : double
        The fill fractions corresponding to the maximum and minimum times.

    d_log_time : double
        The logarithmic interval between successive (decreasing) times.
*/
void TrapInstantCaptureContinuum::prep_fill_fraction_and_time_elapsed_tables(
    double time_min, double time_max, int n_intp) {

    // Prep for the GSL integration
    const int limit = 100;
    gsl_integration_workspace* workspace = gsl_integration_workspace_alloc(limit);

    // Set up the arrays and limits
    fill_fraction_table = std::valarray<double>(0.0, n_intp);
    this->n_intp = n_intp;
    this->time_min = time_min;
    this->time_max = time_max;
    fill_min = fill_fraction_from_time_elapsed(time_max, workspace);
    fill_max = fill_fraction_from_time_elapsed(time_min, workspace);
    d_log_time = (log(time_max) - log(time_min)) / (n_intp - 1);
    double time_i;

    // Tabulate the values corresponding to the equally log-spaced inputs
    for (int i = 0; i < n_intp; i++) {
        time_i = exp(log(time_max) - i * d_log_time);
        fill_fraction_table[i] = fill_fraction_from_time_elapsed(time_i, workspace);
    }
}

/*
    Calculate the fraction of filled traps after an amount of elapsed time,
    using previously tabulated values for interpolation.

    Parameters
    ----------
    time_elapsed : double
        The total time elapsed since the traps were filled, in the same units
        as the trap timescales.

    Returns
    -------
    fill_fraction : double
        The fraction of filled traps.
*/
double TrapInstantCaptureContinuum::fill_fraction_from_time_elapsed_table(
    double time_elapsed) {
    // Completely full or empty, or unset watermark
    if (time_elapsed == 0.0)
        return 1.0;
    else if (time_elapsed >= std::numeric_limits<double>::max())
        return 0.0;
    else if (time_elapsed == -1.0)
        return 0.0;

    // Get the index and interpolation factor
    double intp = (log(time_max) - log(time_elapsed)) / d_log_time;
    int idx = (int)std::floor(intp);
    intp = intp - idx;

    // Extrapolate if outside the table
    if (idx < 0) {
        intp += idx;
        idx = 0;
    } else if (idx >= n_intp - 1) {
        intp += idx - (n_intp - 2);
        idx = n_intp - 2;
    }

    // Interpolate
    double fill =
        (1.0 - intp) * fill_fraction_table[idx] + intp * fill_fraction_table[idx + 1];

    return clamp(fill, 0.0, 1.0);
}

/*
    Calculate the amount of elapsed time from the fraction of filled traps,
    using previously tabulated values for interpolation.

    Parameters
    ----------
    fill_fraction : double
        The fraction of filled traps.

    Returns
    -------
    time_elapsed : double
        The total time elapsed since the traps were filled, in the same units
        as the trap timescales.
*/
double TrapInstantCaptureContinuum::time_elapsed_from_fill_fraction_table(
    double fill_fraction) {
    // Completely full or empty, or unset watermark
    if (fill_fraction == 1.0)
        return 0.0;
    else if (fill_fraction == 0.0)
        return std::numeric_limits<double>::max();
    else if (fill_fraction == -1.0)
        return 0.0;

    // Find the index by searching the table
    int idx = std::upper_bound(
                  std::begin(fill_fraction_table), std::end(fill_fraction_table),
                  fill_fraction) -
              std::begin(fill_fraction_table) - 1;

    // Extrapolate if outside the table
    if (idx < 0)
        idx = 0;
    else if (idx == n_intp - 1)
        idx = n_intp - 2;

    // Interpolation factor
    double intp = (fill_fraction - fill_fraction_table[idx]) /
                  (fill_fraction_table[idx + 1] - fill_fraction_table[idx]);

    // Interpolate
    return exp(log(time_max) - (idx + intp) * d_log_time);
}

// ========
// TrapSlowCaptureContinuum::
// ========
/*
    Class TrapSlowCaptureContinuum.

    For traps with a non-instant capture time, and a continuum (log-normal
    distribution) of release timescales.

    i.e. A combination of the TrapSlowCapture and TrapInstantCaptureContinuum models.

    Parameters
    ----------
    density : double
        The density of the trap species in a pixel.

    release_timescale : double
        The median release timescale of the traps, in the same units as the time
        spent in each pixel or phase.

    release_timescale_sigma : double
        The sigma of release lifetimes of the traps.

    capture_timescale : double
        The capture timescale of the trap.
*/
TrapSlowCaptureContinuum::TrapSlowCaptureContinuum(
    double density, double release_timescale, double release_timescale_sigma,
    double capture_timescale)
    : TrapInstantCapture(density, release_timescale),
      release_timescale_sigma(release_timescale_sigma),
      capture_timescale(capture_timescale) {

    if (capture_timescale != 0.0)
        capture_rate = 1.0 / capture_timescale;
    else
        capture_rate = 0.0;
}

/*
    Same as TrapInstantCaptureContinuum
*/
struct TrSCCo_ff_from_te_params {
    double time_elapsed;
    double mu;
    double sigma;
};

double TrSCCo_ff_from_te_integrand(double tau, void* params) {
    struct TrSCCo_ff_from_te_params* p = (struct TrSCCo_ff_from_te_params*)params;

    // Distribution density
    double n = exp(-pow(log(tau) - log(p->mu), 2.0) / (2.0 * p->sigma * p->sigma)) /
               (tau * p->sigma * sqrt(2.0 * M_PI));

    // To integrate: n(tau) * fill_frac(tau) dtau
    return n * exp(-p->time_elapsed / tau);
}

double TrapSlowCaptureContinuum::fill_fraction_from_time_elapsed(
    double time_elapsed, gsl_integration_workspace* workspace) {
    // Completely full or empty, or unset watermark
    if (time_elapsed == 0.0)
        return 1.0;
    else if (time_elapsed >= std::numeric_limits<double>::max())
        return 0.0;
    else if (time_elapsed == -1.0)
        return 0.0;

    // Prep the integration
    double result, error;
    const double min = 0.0;
    const double max = release_timescale + 100 * release_timescale_sigma;
    const double epsabs = 0.0;
    const double epsrel = 1e-6;
    const int limit = 100;
    const int key = GSL_INTEG_GAUSS51;
    if (!workspace) {
        workspace = gsl_integration_workspace_alloc(limit);
    }
    struct TrSCCo_ff_from_te_params params = {
        time_elapsed, release_timescale, release_timescale_sigma};
    gsl_function F;
    F.function = &TrSCCo_ff_from_te_integrand;
    F.params = &params;

    // Integrate F.function from min to max
    int status = gsl_integration_qag(
        &F, min, max, epsabs, epsrel, limit, key, workspace, &result, &error);

    if (status) error("Integration failed, status %d", status);

    return result;
}

/*
    Same as TrapInstantCaptureContinuum
*/
struct TrSCCo_te_from_ff_params {
    TrapSlowCaptureContinuum* trap;
    double fill_fraction;
    gsl_integration_workspace* workspace;
};

double TrSCCo_te_from_ff_root_function(double time_elapsed, void* params) {
    struct TrSCCo_te_from_ff_params* p = (struct TrSCCo_te_from_ff_params*)params;

    // To find time such that: fill_fraction(time) - fill_fraction = 0
    return p->trap->fill_fraction_from_time_elapsed(time_elapsed, p->workspace) -
           p->fill_fraction;
}

double TrapSlowCaptureContinuum::time_elapsed_from_fill_fraction(
    double fill_fraction, double time_max, gsl_integration_workspace* workspace) {
    // Completely full or empty, or unset watermark
    if (fill_fraction == 1.0)
        return 0.0;
    else if (fill_fraction == 0.0)
        return std::numeric_limits<double>::max();
    else if (fill_fraction == -1.0)
        return 0.0;

    // Prep for the integration
    if (!workspace) {
        const int limit = 100;
        workspace = gsl_integration_workspace_alloc(limit);
    }

    // Prep the root finder
    double root;
    int status;
    const int max_iter = 100;
    const double epsabs = 0.0;
    const double epsrel = 1e-6;
    const gsl_root_fsolver_type* T;
    gsl_root_fsolver* s;
    T = gsl_root_fsolver_brent;
    s = gsl_root_fsolver_alloc(T);
    struct TrSCCo_te_from_ff_params params = {this, fill_fraction, workspace};
    gsl_function F;
    F.function = &TrSCCo_te_from_ff_root_function;
    F.params = &params;

    // Bounding values
    double x_lo = 0.0;
    double x_hi = time_max;

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

/*
    Same as TrapInstantCaptureContinuum
*/
void TrapSlowCaptureContinuum::prep_fill_fraction_and_time_elapsed_tables(
    double time_min, double time_max, int n_intp) {

    // Prep for the GSL integration
    const int limit = 100;
    gsl_integration_workspace* workspace = gsl_integration_workspace_alloc(limit);

    // Set up the arrays and limits
    fill_fraction_table = std::valarray<double>(0.0, n_intp);
    this->n_intp = n_intp;
    this->time_min = time_min;
    this->time_max = time_max;
    fill_min = fill_fraction_from_time_elapsed(time_max, workspace);
    fill_max = fill_fraction_from_time_elapsed(time_min, workspace);
    d_log_time = (log(time_max) - log(time_min)) / (n_intp - 1);
    double time_i;

    // Tabulate the values corresponding to the equally log-spaced inputs
    for (int i = 0; i < n_intp; i++) {
        time_i = exp(log(time_max) - i * d_log_time);
        fill_fraction_table[i] = fill_fraction_from_time_elapsed(time_i, workspace);
    }
}

/*
    Same as TrapInstantCaptureContinuum
*/
double TrapSlowCaptureContinuum::fill_fraction_from_time_elapsed_table(
    double time_elapsed) {
    // Completely full or empty, or unset watermark
    if (time_elapsed == 0.0)
        return 1.0;
    else if (time_elapsed >= std::numeric_limits<double>::max())
        return 0.0;
    else if (time_elapsed == -1.0)
        return 0.0;

    // Get the index and interpolation factor
    double intp = (log(time_max) - log(time_elapsed)) / d_log_time;
    int idx = (int)std::floor(intp);
    intp = intp - idx;

    // Extrapolate if outside the table
    if (idx < 0) {
        intp += idx;
        idx = 0;
    } else if (idx >= n_intp - 1) {
        intp += idx - (n_intp - 2);
        idx = n_intp - 2;
    }

    // Interpolate
    double fill =
        (1.0 - intp) * fill_fraction_table[idx] + intp * fill_fraction_table[idx + 1];

    return clamp(fill, 0.0, 1.0);
}

/*
    Same as TrapInstantCaptureContinuum
*/
double TrapSlowCaptureContinuum::time_elapsed_from_fill_fraction_table(
    double fill_fraction) {
    // Completely full or empty, or unset watermark
    if (fill_fraction == 1.0)
        return 0.0;
    else if (fill_fraction == 0.0)
        return std::numeric_limits<double>::max();
    else if (fill_fraction == -1.0)
        return 0.0;

    // Find the index by searching the table
    int idx = std::upper_bound(
                  std::begin(fill_fraction_table), std::end(fill_fraction_table),
                  fill_fraction) -
              std::begin(fill_fraction_table) - 1;

    // Extrapolate if outside the table
    if (idx < 0)
        idx = 0;
    else if (idx == n_intp - 1)
        idx = n_intp - 2;

    // Interpolation factor
    double intp = (fill_fraction - fill_fraction_table[idx]) /
                  (fill_fraction_table[idx + 1] - fill_fraction_table[idx]);

    // Interpolate
    return exp(log(time_max) - (idx + intp) * d_log_time);
}

/*
    Calculate the fraction of filled traps after slow-capture (and release).

    Found by integrating the trap density distribution multiplied by the final
    fill fraction, which must be calculated accounting for the dependency of
    both the initial fill fraction and the fill probabilities on the release
    lifetime distribution:
        f(tau) = f_0(tau) f_f(tau) + (1 - f_0(tau)) f_e(tau)
        f = int_0^inf n(tau) f(tau) dtau

    Integrating from 0 to infinity can fail for very small mu and/or sigma, so
    instead integrate from 0 to (mu + 100 sigma) for a more reliable and very
    close approximation.

    (www.gnu.org/software/gsl/doc/html/integration.html#qag-adaptive-integration)

    Parameters
    ----------
    time_elapsed : double
        The approximate, effective time elapsed since the traps were filled, in
        the same units as the trap timescales.

    dwell_time : double
        The time spent in this pixel or phase, in the same units as the trap
        timescales.

    workspace : gsl_integration_workspace* (opt.)
        An existing GSL workspace memory handler for the integration, or nullptr
        to create a new one.

    Returns
    -------
    fill_fraction : double
        The fraction of filled traps.
*/
struct TrSCCo_ff_after_sc_params {
    double time_elapsed;
    double mu;
    double sigma;
    double capture_rate;
    double dwell_time;
};

double TrSCCo_ff_after_sc_integrand(double tau, void* params) {
    struct TrSCCo_ff_after_sc_params* p = (struct TrSCCo_ff_after_sc_params*)params;

    // Distribution density
    double n = exp(-pow(log(tau) - log(p->mu), 2.0) / (2.0 * p->sigma * p->sigma)) /
               (tau * p->sigma * sqrt(2.0 * M_PI));

    // Rates and fill probabilities
    double release_rate = 1.0 / tau;
    double total_rate = p->capture_rate + release_rate;
    double exponential_factor = (1 - exp(-total_rate * p->dwell_time)) / total_rate;
    double fill_probability_from_empty = p->capture_rate * exponential_factor;
    double fill_probability_from_full = 1.0 - release_rate * exponential_factor;

    // Initial fill fraction
    double f_0;
    if (p->time_elapsed == 0.0)
        f_0 = 1.0;
    else if (p->time_elapsed >= std::numeric_limits<double>::max())
        f_0 = 0.0;
    else
        f_0 = exp(-p->time_elapsed * release_rate);

    // To integrate: n(tau) * new_fill(tau) dtau
    return n * (f_0 * fill_probability_from_full +
                (1.0 - f_0) * fill_probability_from_empty);
}

double TrapSlowCaptureContinuum::fill_fraction_after_slow_capture(
    double time_elapsed, double dwell_time, gsl_integration_workspace* workspace) {
    // Prep the integration
    double result, error;
    const double min = 0.0;
    const double max = release_timescale + 100 * release_timescale_sigma;
    const double epsabs = 0.0;
    const double epsrel = 1e-6;
    const int limit = 100;
    const int key = GSL_INTEG_GAUSS51;
    if (!workspace) {
        workspace = gsl_integration_workspace_alloc(limit);
    }
    struct TrSCCo_ff_after_sc_params params = {
        time_elapsed, release_timescale, release_timescale_sigma, capture_rate,
        dwell_time};
    gsl_function F;
    F.function = &TrSCCo_ff_after_sc_integrand;
    F.params = &params;

    // Integrate F.function from min to max
    int status = gsl_integration_qag(
        &F, min, max, epsabs, epsrel, limit, key, workspace, &result, &error);

    if (status) error("Integration failed, status %d", status);

    return result;
}

/*
    Prepare tables of fill fractions and elapsed times for interpolation of
    fill fractions after slow capture (and release).

    Use logarithmically spaced (decreasing) times to calculate the corresponding
    (monotonically increasing) table of fill fractions. So no need to actually
    store the elapsed times.

    Parameters
    ----------
    dwell_time : double
        The time spent in this pixel or phase, in the same units as the trap
        timescales.

    time_min : double
    time_max : double
        The minimum and maximum elapsed times to set the table limits, e.g. a
        single dwell time and the cumulative dwell time over all transfers.

    n_intp : int
        The number of interpolation values in the arrays.

    Sets
    ----
    fill_fraction_capture_table : std::valarray<double>
        The array of fill fractions.

    fill_capture_long_time : double
        The should-be-converged fill fraction from a very long elapsed time.
*/
void TrapSlowCaptureContinuum::prep_fill_fraction_after_slow_capture_tables(
    double dwell_time, double time_min, double time_max, int n_intp) {
    // Prep for the GSL integration
    const int limit = 100;
    gsl_integration_workspace* workspace = gsl_integration_workspace_alloc(limit);

    // Set up the arrays and limits
    fill_fraction_capture_table = std::valarray<double>(0.0, n_intp);
    this->n_intp = n_intp;
    this->time_min = time_min;
    this->time_max = time_max;
    fill_capture_min =
        fill_fraction_after_slow_capture(time_max, dwell_time, workspace);
    fill_capture_max =
        fill_fraction_after_slow_capture(time_min, dwell_time, workspace);
    fill_capture_long_time =
        fill_fraction_after_slow_capture(time_max * 100, dwell_time, workspace);
    d_log_time = (log(time_max) - log(time_min)) / (n_intp - 1);
    double time_i;

    // Tabulate the values corresponding to the equally log-spaced inputs
    for (int i = 0; i < n_intp; i++) {
        time_i = exp(log(time_max) - i * d_log_time);
        fill_fraction_capture_table[i] =
            fill_fraction_after_slow_capture(time_i, dwell_time, workspace);
    }
}

/*
    Calculate the fraction of filled traps after slow-capture (and release),
    using previously tabulated values for interpolation.

    Parameters
    ----------
    time_elapsed : double
        The approximate, effective time elapsed since the traps were filled, in
        the same units as the trap timescales.

    Returns
    -------
    fill_fraction : double
        The fraction of filled traps.
*/
double TrapSlowCaptureContinuum::fill_fraction_after_slow_capture_table(
    double time_elapsed) {
    // Very long elapsed time, or unset watermark
    if (time_elapsed >= std::numeric_limits<double>::max())
        return fill_capture_long_time;
    else if (time_elapsed == -1.0)
        return 0.0;

    // Get the index and interpolation factor
    double intp = (log(time_max) - log(time_elapsed)) / d_log_time;
    int idx = (int)std::floor(intp);
    intp = intp - idx;

    // Extrapolate if outside the table
    if (idx < 0) {
        intp += idx;
        idx = 0;
    } else if (idx >= n_intp - 1) {
        intp += idx - (n_intp - 2);
        idx = n_intp - 2;
    }

    // Interpolate
    double fill = (1.0 - intp) * fill_fraction_capture_table[idx] +
                  intp * fill_fraction_capture_table[idx + 1];

    return clamp(fill, 0.0, 1.0);
}


#include <math.h>

#include "traps.hpp"

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
    capture_rate, emission_rate : double
        The capture and emission rates (Lindegren (1998) section 3.2).
*/
Trap::Trap(double density, double release_timescale, double capture_timescale)
    : density(density),
      release_timescale(release_timescale),
      capture_timescale(capture_timescale) {
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
    : Trap(density, release_timescale, 0.0) {}
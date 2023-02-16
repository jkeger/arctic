
#include "ccd.hpp"

#include <math.h>

#include "util.hpp"


int sgn(double v) {
  return (v < 0) ? -1 : ((v > 0) ? 1 : 0);
}

// ========
// CCDPhase::
// ========
/*
    Class CCDPhase.

    Parameters to describe how electrons fill the volume inside (one phase of)
    a pixel in a CCD detector.

    Parameters
    ----------
    full_well_depth : double
        The maximum number of electrons that can be contained within a
        pixel/phase. For multiphase clocking, if only one value is supplied,
        that is (by default) replicated to all phases. However, different
        physical widths of phases can be set by specifying the full well
        depth as a list containing different values. If the potential in
        more than one phase is held high during any stage in the clocking
        cycle, their full well depths are added together. This value is
        indpependent of the fraction of traps allocated to each phase.

    well_notch_depth : double
        The number of electrons that fit inside a 'notch' at the bottom of a
        potential well, occupying negligible volume and therefore being
        immune to trapping. These electrons still count towards the full
        well depth. The notch depth can, in  principle, vary between phases.

    well_fill_power : double
        The exponent in a power-law model of the volume occupied by a cloud
        of electrons. This can, in principle, vary between phases.
*/
CCDPhase::CCDPhase(
    double full_well_depth, 
    double well_notch_depth, 
    double well_fill_power, 
    double first_electron_fill)
    : full_well_depth(full_well_depth),
      well_notch_depth(well_notch_depth),
      well_fill_power(well_fill_power),
      first_electron_fill(first_electron_fill) {}

/*
    Calculate the fractional volume that a charge cloud reaches in the pixel.

    Parameters
    ----------
    n_electrons : double
        The number of electrons in the charge cloud.

    Returns
    -------
    cloud_fractional_volume : double
        The volume of the charge cloud as a fraction of the pixel (or phase).
*/
double CCDPhase::cloud_fractional_volume_from_electrons(double n_electrons) {
    
    
    //double frac = (n_electrons - well_notch_depth) / full_well_depth;
    //if (n_electrons == 0.0)
    if (n_electrons <= 0.0)
        return first_electron_fill ; //0.0;
    else
        //return sgn(frac) * pow( abs(frac), well_fill_power );
        //return pow( clamp(frac, 0.0, 1.0), well_fill_power);
        return first_electron_fill + (1 - first_electron_fill) * pow( 
            clamp( (n_electrons - well_notch_depth) / 
                   (full_well_depth - well_notch_depth), 0.0, 1.0), 
            well_fill_power);
}

// ========
// CCD::
// ========
/*
    Class CCD.

    A set of CCD phases to describe how electrons fill the volume inside (all
    phases of) a pixel in a CCD detector.

    Parameters
    ----------
    phases : std::valarray<CCDPhase>
        The array of one or more CCD phase objects.

    fraction_of_traps_per_phase : std::valarray<double>
        The proportion of traps distributed in each phase. If the traps have
        uniform density throughout the CCD, this would be the physical width of
        each phase as a fraction of the pixel width.

        Defaults to equal fractions in each phase (e.g. {1} or {1/3, 1/3, 1/3}).
*/
CCD::CCD(
    std::valarray<CCDPhase>& phases, std::valarray<double>& fraction_of_traps_per_phase)
    : phases(phases), fraction_of_traps_per_phase(fraction_of_traps_per_phase) {

    n_phases = phases.size();

    if (fraction_of_traps_per_phase.size() != n_phases)
        error(
            "Sizes of phases (%ld) and fraction_of_traps_per_phase (%ld) don't match.",
            phases.size(), fraction_of_traps_per_phase.size());
}

/*
    Alternative initialisation for single-phase clocking.

    Parameters
    ----------
    phase : CCDPhase
        The single CCD phase object, for single-phase clocking.
*/
CCD::CCD(CCDPhase phase) {
    phases = {phase};
    n_phases = 1;
    fraction_of_traps_per_phase = {1.0};
}


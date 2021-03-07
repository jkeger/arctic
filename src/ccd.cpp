
#include <math.h>

#include "ccd.hpp"
#include "util.hpp"

// ========
// CCD::
// ========
/*
    Class CCD.
    
    Parameters to describe how electrons fill the volume inside (each phase of)
    a pixel in a CCD detector.
    
    ##todo: reimplement multiple phases
    
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
CCD::CCD(double full_well_depth, double well_notch_depth, double well_fill_power)
    : full_well_depth(full_well_depth),
      well_notch_depth(well_notch_depth),
      well_fill_power(well_fill_power) {}

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
double CCD::cloud_fractional_volume_from_electrons(double n_electrons) {
    if (n_electrons == 0.0)
        return 0.0;
    else
        return pow(
            clamp((n_electrons - well_notch_depth) / full_well_depth, 0.0, 1.0),
            well_fill_power);
}
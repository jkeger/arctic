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
        independent of the fraction of traps allocated to each phase.

    well_notch_depth : double
        If positive:
        The number of electrons that fit inside a 'notch' or s'upplementary 
        buried channel' at the bottom of a potential well, occupying 
        negligible volume and therefore being immune to trapping. These 
        electrons still count towards the full well depth. The notch depth 
        can, in  principle, vary between phases. This is used with HST.
        If negative:
        The (negative) fraction of the volume of a pixel that remains 
        permanently filled, even by a nominally small (zero) number of
        electrons, because quantum mecahnics means electrons are fuzzy. 
        In this case, there is necessarily no 'notch'. Used with Euclid.

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
  
/*
    // HST version of volume filling function, from Massey et al. (2025)
    //double frac = (n_electrons - well_notch_depth) / full_well_depth;
    if (n_electrons <= 0.0)
        return first_electron_fill ; //0.0;
    else
        //return sgn(frac) * pow( abs(frac), well_fill_power );
        //return pow( clamp(frac, 0.0, 1.0), well_fill_power);
        return first_electron_fill + (1 - first_electron_fill) * pow( 
            clamp( (n_electrons - well_notch_depth) / 
                   (full_well_depth - well_notch_depth), 0.0, 1.0), 
            well_fill_power);
*/
    
    // Euclid version of volume filling function, inspired by figure 3 of 
    // Clarke et al. (2012) http://dx.doi.org/doi:10.1117/12.925887
    // although the functional form there was V=alpha + gamma * n_electrons^beta, 
    //  [where for parallel CTI, alpha=0.89, beta=0.51, gamma=0.09
    //   and for serial CTI, alpha=3.195, beta=0.59, gamma=0.04]
    //
    // Instead the model here (so everything stays properly normalised) is
    // V = ((n - d)/(w - d))^beta   for n>0
    // V = a exp ( b * n )          for n<0 
    // where a = ( -d / (w-d))^beta
    // and   b = -beta / d                  so V & dV/dn are matched at n=0
    //
    // This should be used with d<0 to reproduce fig 3 of Clarke et al.
    // and specifically to make V(0)/V(w) = Vclarke(0)/Vclarke(w), we should use 
    // d = w/(1 - (1+gamma w^beta /alpha)^(1/beta) ) \approx -(alpha/gamma)^(1/beta)
    //
    // i.e for parallel (fig 3a) beta=0.51, d=-86, w=200000
    // and for serial (fig 3b)   beta=0.59, d=-1530, w=200000
    //
    // Note that parameter d may be degenerate with beta. It might be possible to
    // parameterise this instead with d'=-d^beta, but this destroys the sign information,
    // or at least breaks/removes continuous symmetry if d passes through zero in MCMC.
    //
    if (n_electrons > 0.0) {
        return first_electron_fill + (1 - first_electron_fill) * pow( 
            clamp( (n_electrons - well_notch_depth) / 
                   (full_well_depth - well_notch_depth), 0.0, 1.0), 
            well_fill_power);
    } else if (n_electrons < 0.0) {
        if (well_notch_depth >= 0.0) {
        	return first_electron_fill ; 
		} else {
			double a = (1 - first_electron_fill) * pow( 
				( well_notch_depth / ( well_notch_depth - full_well_depth ) ), 
           	well_fill_power) ;
            double b = - n_electrons * well_fill_power / well_notch_depth ;
        	if (b < -700) b = -700;
        	return first_electron_fill + a * std::exp(b);
		}
	} else {
        if (well_notch_depth >= 0.0) {
        	return first_electron_fill ; 
		} else {
			double a = (1 - first_electron_fill) * pow( 
				( well_notch_depth / ( well_notch_depth - full_well_depth ) ), 
           	well_fill_power) ;
        	return first_electron_fill + a ;
        }
    }

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


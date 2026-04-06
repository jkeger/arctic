#include "ccd.hpp"
#include <math.h>
#include "util.hpp"
#include <iostream>

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

// --------------------------------------------------------
// Constructor with parameter initialisation
// --------------------------------------------------------
CCDPhase::CCDPhase(double fwd, double wnd, double wfp, double fef)
    : full_well_depth(fwd),
      well_notch_depth(wnd),
      well_fill_power(wfp),
      first_electron_fill(fef)
{
    constants_need_update_ = true;
}
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


// --------------------------------------------------------
// Compute precomputed constants (lazy evaluated)
// --------------------------------------------------------
void CCDPhase::compute_constants() const
{
    later_electrons_fill_ = 1.0 - first_electron_fill;

    // d would be a much better parameter than well_notch_depth or alpha, because it has a
    // coherent effect on V(n_e) as it changes from positive (Hubble) to negative (Euclid)
    //
    // alpha is V(n_e=0)
    double alpha = 0.0;
    if (well_notch_depth < 0.0) {
		alpha = std::pow(10.0, well_notch_depth);
		//static constexpr double ln10 = 2.30258509299404590109; // 1ULP accurate
		//double alpha = std::exp(well_notch_depth * ln10);

		// Does the following work properly with nonzero first_electron_fill?
		d_ = full_well_depth / (1.0 - std::pow(alpha, -1.0/well_fill_power) );
		
		//Can recover alpha (within floating point error) via
		//const double alpha_ = std::pow( -d_ * denom_ , well_fill_power);
		
		volume_at_zero_ = later_electrons_fill_ * alpha ;
    } else {
        d_ = well_notch_depth;
        volume_at_zero_ = 0.0;
    }
    fef_plus_volume_at_zero_ = first_electron_fill + volume_at_zero_;

    if (std::abs(d_) > 1e-30)
        exp_slope_ = -well_fill_power / d_;
    else
        exp_slope_ = 0.0;

    const double w_minus_d = full_well_depth - d_;
    denom_ = (std::abs(w_minus_d) > 1e-30) ? (1.0 / w_minus_d) : 0.0;

    // Checksum
    checksum_ =full_well_depth + well_notch_depth + well_fill_power + first_electron_fill;
    constants_need_update_ = false;

/*
	std::cout << std::string("pcc full_well_depth = ")          + std::to_string(full_well_depth)          + "\n"
	          << std::string("pcc first_electron_fill = ")      + std::to_string(first_electron_fill)      + "\n"
	          << std::string("pcc later_electrons_fill_ = ")    + std::to_string(later_electrons_fill_)    + "\n"
	          << std::string("pcc well_fill_power = ")          + std::to_string(well_fill_power)          + "\n"
	          << std::string("pcc well_notch_depth = ")         + std::to_string(well_notch_depth)         + "\n"
	          << std::string("pcc alpha = ")                    + std::to_string(alpha)                    + "\n"
	          << std::string("pcc d = ")                        + std::to_string(d_)                       + "\n"
	          << std::string("pcc denom = ")                    + std::to_string(denom_)                   + "\n"
	          << std::string("pcc w_minus_d = ")                + std::to_string(w_minus_d)                + "\n"
	          << std::string("pcc 1/w_minus_d = ")              + std::to_string(1.0/w_minus_d)            + "\n"
	          << std::string("pcc exp_slope = ")                + std::to_string(exp_slope_)               + "\n"
	          << std::string("pcc volume_at_zero_ = ")          + std::to_string(volume_at_zero_)          + "\n"
	          << std::string("pcc fef_plus_volume_at_zero_ = ") + std::to_string(fef_plus_volume_at_zero_) + "\n"
	          << std::string("pcc checksum_ = ")                + std::to_string(checksum_)                + "\n";
	//exit(0);
*/
}

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
        Note that the behaviour of this function is discontinuous at full_well_depth = 0.
        Do not let a parameter search cross from positive to negative full_well_depth!
        
        If well_notch_depth is positive or zero, this calculates the volume filling 
    	function appropriate for HST from Massey et al. (2025), 
    			V = { ((n_e - d)/(w - d))^beta , for all n_e > d
    			    { 0                        , for all n_e < d
    	where d=well_notch_depth (only if this is positive!) and w=full_well_depth. 
    	
    	If well_notch_depth is negative, this calculates the volume filling function
    	appropriate for Euclid DR2 and inspired by figure 3 of Clarke et al. (2012) 
    	http://dx.doi.org/doi:10.1117/12.925887 Although their functional form shifts a 
    	power law up, as V=alpha + gamma * n_electrons^beta, here we shift it sideways
    			V = { ((n_e - d)/(w - d))^beta , for all n_e > 0 as before
		        	{  alpha * exp ( b * n_e ) , for all n_e < 0 
		This alternative has a finite gradient at n_e=0, so the function can be extended 
		to negative n_e (which is needed in the presence of measurement noise). So that 
		positive and negative noise excursions of a "bias" or "dark exposure" image with
		values near zero are trailed/filled identically, we enforce smoothness, ie. V and 
		dV/dn_e are matched at n_e=0. Since 
    			V -> { ( -d / (w-d))^beta , as n_e -> 0 from above
    			     { alpha              , as n_e -> 0 from below
		this requires (after some algebra)
    			d = w / (1 - alpha^(-1/beta)) .                                        [1]
    	Since		
    			dV/dn_e -> { beta * (-d)^(beta-1) / (w-d)^beta , as n_e -> 0 from above
    			           { b * alpha                         , as n_e -> 0 from below
		this requires 
    			b = - beta / d.                                                        [2]
		
		You might think that d=well_notch_depth would remain an ideal parameterisation, 
		creating a coherent change in V(n_e) as it transitions from positive (Hubble) to 
		negative (Euclid). However, d and beta are highly degnerate when constrained 
		using EPER trails in Euclid charge injection data. Instead, we (mis)use the 
		parameter well_notch_depth. If well_notch_depth is negative, we calculate
    			alpha = 10^(well_notch_depth)
		where the exponent again helps search what could be a potentially huge parameter 
		space with a linear prior on well_notch_depth (avoiding logarithmic priors in 
		pyautocti, which seem to be temperamental). We then calculate b and d following
		equations [1] and [2]. This is nasty because values like well_notch_depth=-8 
		produce very similar trails to well_notch_depth=0, but well_notch_depth=-0.1 is
		very different (lower amplitude trails into an overscan).
		
    	Measurements in Clarke et al. (2012) for parallel CTI indicate alpha=0.89, 
    	beta=0.51, gamma=0.09 and for serial CTI, alpha=3.195, beta=0.59, gamma=0.04.
    	This implies approximately 
    		 d = w/(1 - (1+gamma w^beta /alpha)^(1/beta) ) \approx -(alpha/gamma)^(1/beta)
		i.e for parallel (fig 3a) beta=0.51, d=-86, w=200000
		and for serial (fig 3b)   beta=0.59, d=-1530, w=200000.

*/
double CCDPhase::cloud_fractional_volume_from_electrons(double n_electrons) const
{

    //// Pure HST version of volume filling function, from Massey et al. (2025)
    //if (n_electrons <= 0.0)
    //    return first_electron_fill;
    //else
    //    return first_electron_fill + (1 - first_electron_fill) * pow( 
    //        clamp( (n_electrons - well_notch_depth) / 
    //               (full_well_depth - well_notch_depth), 0.0, 1.0), 
    //        well_fill_power);
    
    
    if (constants_need_update_)
        compute_constants();

    // ---------------------------------------------
    // n_electrons > 0 : power-law branch
    // ---------------------------------------------
    if (n_electrons > 0.0) {
        double x = (n_electrons - d_) * denom_;
        if (x < 0.0) return first_electron_fill;
        else if (x > 1.0) return 1.0;

        return first_electron_fill +
               later_electrons_fill_ * std::pow(x, well_fill_power);

    } else if (n_electrons < 0.0) {

	    // ---------------------------------------------
	    // n_electrons < 0 : exponential branch
	    // ---------------------------------------------

        if (volume_at_zero_ == 0.0)
            return first_electron_fill;

        // Precompute n_electrons * slope efficiently
        double exponent = n_electrons * exp_slope_;
        if (exponent < -700.0) return first_electron_fill; //catch NaN & underflow-to-zero

        return first_electron_fill + volume_at_zero_ * std::exp(exponent);

    } else {

	    // ---------------------------------------------
	 	// n_electrons == 0
	    // ---------------------------------------------
	    return fef_plus_volume_at_zero_;
    
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

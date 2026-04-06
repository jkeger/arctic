
#ifndef ARCTIC_CCD_HPP
#define ARCTIC_CCD_HPP

#include <valarray>

class CCDPhase {
public:

    // ---- Public API variables with setters ----
    double full_well_depth;
    double well_notch_depth;
    double well_fill_power;
    double first_electron_fill;

    CCDPhase()
        : full_well_depth(100000.0),
          well_notch_depth(0.0001),
          well_fill_power(1.0),
          first_electron_fill(0.0)
    {}

    CCDPhase(double fwd, double wnd, double wfp, double fef = 0.0);

    // --- Automatic-cache-invalidating setters ---
    void set_full_well_depth(double v)        { full_well_depth = v; invalidate(); }
    void set_well_notch_depth(double v)       { well_notch_depth = v; invalidate(); }
    void set_well_fill_power(double v)        { well_fill_power = v; invalidate(); }
    void set_first_electron_fill(double v)    { first_electron_fill = v; invalidate(); }

    double cloud_fractional_volume_from_electrons(double n_electrons) const;

private:
    mutable bool constants_need_update_ = true;
    mutable double d_ = std::numeric_limits<double>::quiet_NaN();
    mutable double denom_ = std::numeric_limits<double>::quiet_NaN();
    mutable double later_electrons_fill_ = std::numeric_limits<double>::quiet_NaN();
    mutable double volume_at_zero_ = std::numeric_limits<double>::quiet_NaN();
    mutable double fef_plus_volume_at_zero_ = std::numeric_limits<double>::quiet_NaN();
    mutable double exp_slope_ = std::numeric_limits<double>::quiet_NaN();
    mutable double checksum_ = std::numeric_limits<double>::quiet_NaN();

    inline void invalidate() { constants_need_update_ = true; }

    void compute_constants() const;
};

class CCD {
public:

    std::valarray<CCDPhase> phases;
    std::valarray<double> fraction_of_traps_per_phase;
    unsigned int n_phases;

    CCD(){};
    CCD(std::valarray<CCDPhase>& phases,
        std::valarray<double>& fraction_of_traps_per_phase);
    
    CCD(CCDPhase phase);
    ~CCD(){};
};

#endif  // ARCTIC_CCD_HPP



#ifndef ARCTIC_CCD_HPP
#define ARCTIC_CCD_HPP

#include <valarray>

class CCDPhase {
   public:
    CCDPhase(){};
    CCDPhase(double full_well_depth, 
            double well_notch_depth, 
            double well_fill_power,
            double first_electron_fill = 0);
    ~CCDPhase(){};

    double full_well_depth;
    double well_notch_depth;
    double well_fill_power;
    double first_electron_fill;

    virtual double cloud_fractional_volume_from_electrons(double n_electrons);
};

class CCD {
   public:
    CCD(){};
    CCD(std::valarray<CCDPhase>& phases,
        std::valarray<double>& fraction_of_traps_per_phase);
    CCD(CCDPhase phase);
    ~CCD(){};

    std::valarray<CCDPhase> phases;
    std::valarray<double> fraction_of_traps_per_phase;

    unsigned int n_phases;
};

#endif  // ARCTIC_CCD_HPP

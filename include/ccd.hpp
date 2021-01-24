
#ifndef ARCTIC_CCD_HPP
#define ARCTIC_CCD_HPP

class CCD {
   public:
    double full_well_depth;
    double well_notch_depth;
    double well_fill_power;

    CCD(double full_well_depth, double well_notch_depth, double well_fill_power);
    ~CCD(){};

    double cloud_fractional_volume_from_electrons(double n_electrons);
};

#endif  // ARCTIC_CCD_HPP

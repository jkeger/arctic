
#ifndef ARCTIC_TRAPS_HPP
#define ARCTIC_TRAPS_HPP

class TrapInstantCapture {
   public:
    TrapInstantCapture(double density, double release_timescale);
    ~TrapInstantCapture(){};

    double density;
    
    double release_timescale;
    double emission_rate;

    virtual double fill_fraction_from_time_elapsed(double time_elapsed);
};

class TrapSlowCapture : public TrapInstantCapture {
   public:
    TrapSlowCapture(double density, double release_timescale, double capture_timescale);
    ~TrapSlowCapture(){};
    
    double capture_timescale;    
    double capture_rate;
};

class TrapContinuum : public TrapInstantCapture {
   public:
    TrapContinuum(
        double density, double release_timescale, double release_timescale_sigma);
    ~TrapContinuum(){};

    double release_timescale_sigma;

    virtual double fill_fraction_from_time_elapsed(double time_elapsed);
    double time_elapsed_from_fill_fraction(double fill_fraction);
};

#endif  // ARCTIC_TRAPS_HPP


#ifndef ARCTIC_TRAPS_HPP
#define ARCTIC_TRAPS_HPP

class Trap {
   public:
    double density;
    double release_timescale;
    double capture_timescale;
    double capture_rate;
    double emission_rate;

    Trap(double density, double release_timescale, double capture_timescale);
    ~Trap(){};

    double fill_fraction_from_time_elapsed(double time_elapsed);
};

class TrapInstantCapture : public Trap {
   public:
    TrapInstantCapture(double density, double release_timescale);
    ~TrapInstantCapture(){};
};

#endif  // ARCTIC_TRAPS_HPP

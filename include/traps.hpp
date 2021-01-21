
#ifndef ARCTIC_TRAPS_HPP
#define ARCTIC_TRAPS_HPP

class Trap {
   public:
    float density;
    float release_timescale;
    float capture_timescale;
    float capture_rate;
    float emission_rate;

    Trap(float density, float release_timescale, float capture_timescale);
    ~Trap(){};

    float fill_fraction_from_time_elapsed(float time_elapsed);
};

class TrapInstantCapture : public Trap {
   public:
    TrapInstantCapture(float density, float release_timescale);
    ~TrapInstantCapture(){};
};

#endif  // ARCTIC_TRAPS_HPP

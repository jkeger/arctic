
#ifndef ARCTIC_TRAPS_HPP
#define ARCTIC_TRAPS_HPP

extern int n_watermark_types;
enum WatermarkType {
    watermark_type_standard = 0,
    watermark_type_instant_capture = 1,
    watermark_type_continuum = 2
};

class Trap {
   public:
    Trap(double density, double release_timescale, double capture_timescale);
    ~Trap(){};

    double density;
    double release_timescale;
    double capture_timescale;

    WatermarkType watermark_type;
    double emission_rate;
    double capture_rate;

    virtual double fill_fraction_from_time_elapsed(double time_elapsed);
};

class TrapInstantCapture : public Trap {
   public:
    TrapInstantCapture(double density, double release_timescale);
    ~TrapInstantCapture(){};
};

class TrapContinuum : public Trap {
   public:
    TrapContinuum(
        double density, double release_timescale, double release_timescale_sigma);
    ~TrapContinuum(){};

    double release_timescale_sigma;

    virtual double fill_fraction_from_time_elapsed(double time_elapsed);
};

#endif  // ARCTIC_TRAPS_HPP

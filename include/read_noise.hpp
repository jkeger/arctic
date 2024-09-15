
#ifndef ARCTIC_READ_NOISE_HPP
#define ARCTIC_READ_NOISE_HPP

void determine_read_noise_model(
    const double* imageIn, const double* imageOut, const int rows, const int cols,
    const double readNoiseAmp, const double readNoiseAmpFraction, const int smoothCol,
    double* output);

#endif  // ARCTIC_READ_NOISE_HPP

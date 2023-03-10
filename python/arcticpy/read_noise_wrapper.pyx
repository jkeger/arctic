import numpy as np
cimport numpy as np
cimport cython

cdef extern void determine_noise_model(const double* imageIn, const double* imageOut, const int rows, const int cols, const double readNoiseAmp, const double readNoiseAmpFraction, const int smoothCol, double* output);

def determine_noise_model_c(np.ndarray[np.float64_t, ndim=2] arr1, np.ndarray[np.float64_t, ndim=2] arr2, readNoiseAmp, readNoiseAmpFraction, smoothCol):
    cdef int rows = arr1.shape[0]
    cdef int cols = arr1.shape[1]
    cdef np.ndarray[np.float64_t, ndim=2] out = np.zeros((rows, cols), dtype=np.float64)
    determine_noise_model(&arr1[0,0], &arr2[0,0], rows, cols, readNoiseAmp, readNoiseAmpFraction, smoothCol, &out[0,0])
    return out

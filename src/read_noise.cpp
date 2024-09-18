#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <iostream>

#include "util.hpp"
using namespace std;

/*
    ###
*/
void determine_read_noise_model(
    const double* imageIn, const double* imageOut, const int rows, const int cols,
    const double readNoiseAmp, const double readNoiseAmpFraction, const int smoothCol,
    double* output) {

    double* dval0 =
        output;  // using already allocated output buffer to temporarily store the
                 // results to avoid allocating more memory (safe as long as we do not
                 // modify output before storing dval0 there and not use dval0 after
                 // modifying output for the first time)
    double* dval9 = (double*)malloc(rows * cols * sizeof(double));

    double mod_clip = readNoiseAmp * readNoiseAmpFraction;
    double readNoiseAmp2 = sq(readNoiseAmp);

    int idx;
    print_v(1, "hello");     //###
    cout << "Hello World!";  //###

    for (int i = 0; i < rows * cols; i++) {
        dval0[i] = imageIn[i] - imageOut[i];
        dval9[i] = dval0[i];
    }

    // get the dval9 average
    // inner part
    for (int i = 1; i < rows - 1; i++) {
#pragma omp parallel for private(idx)
        for (int j = 1; j < cols - 1; j++) {
            idx = i * cols + j;
            dval9[idx] +=
                dval0[idx + cols + 1];  // comparison with bottom-left neighbour
            dval9[idx] +=
                dval0[idx + cols];  // comparison with bottom-central neighbour
            dval9[idx] +=
                dval0[idx + cols - 1];     // comparison with bottom-right neighbour
            dval9[idx] += dval0[idx + 1];  // comparison with middle-left neighbour
            dval9[idx] += dval0[idx - 1];  // comparison with middle-right neighbour
            dval9[idx] += dval0[idx - cols + 1];  // comparison with top-left neighbour
            dval9[idx] += dval0[idx - cols];  // comparison with top-central neighbour
            dval9[idx] += dval0[idx - cols - 1];  // comparison with top-right neighbour
            dval9[idx] /= 9;
        }
    }
    // edges (excl. corners)
#pragma omp parallel for private(idx)
    for (int i = 1; i < rows - 1; i++) {
        // left edge (j=0)
        idx = i * cols;
        dval9[idx] += dval0[idx + cols + 1];  // comparison with bottom-left neighbour
        dval9[idx] += dval0[idx + cols];  // comparison with bottom-central neighbour
        dval9[idx] += dval0[idx + 1];     // comparison with middle-left neighbour
        dval9[idx] += dval0[idx - cols + 1];  // comparison with top-left neighbour
        dval9[idx] += dval0[idx - cols];      // comparison with top-central neighbour
        dval9[idx] /= 6.0;
        // right edge (j=cols-1)
        idx = (i + 1) * cols - 1;
        dval9[idx] += dval0[idx + cols];  // comparison with bottom-central neighbour
        dval9[idx] += dval0[idx + cols - 1];  // comparison with bottom-right neighbour
        dval9[idx] += dval0[idx - 1];         // comparison with middle-right neighbour
        dval9[idx] += dval0[idx - cols];      // comparison with top-central neighbour
        dval9[idx] += dval0[idx - cols - 1];  // comparison with top-right neighbour
        dval9[idx] /= 6.0;
    }

#pragma omp parallel for private(idx), shared(dval0, dval9)
    for (int j = 1; j < cols - 1; j++) {
        // top edge (i=0)
        idx = j;
        dval9[idx] += dval0[idx + cols + 1];  // comparison with bottom-left neighbour
        dval9[idx] += dval0[idx + cols];  // comparison with bottom-central neighbour
        dval9[idx] += dval0[idx + cols - 1];  // comparison with bottom-right neighbour
        dval9[idx] += dval0[idx + 1];         // comparison with middle-left neighbour
        dval9[idx] += dval0[idx - 1];         // comparison with middle-right neighbour
        dval9[idx] /= 6.0;
        // bottom edge (i=rows-1)
        idx = (rows - 1) * cols + j;
        dval9[idx] += dval0[idx + 1];         // comparison with middle-left neighbour
        dval9[idx] += dval0[idx - 1];         // comparison with middle-right neighbour
        dval9[idx] += dval0[idx - cols + 1];  // comparison with top-left neighbour
        dval9[idx] += dval0[idx - cols];      // comparison with top-central neighbour
        dval9[idx] += dval0[idx - cols - 1];  // comparison with top-right neighbour
        dval9[(rows - 1) * cols + j] /= 6.0;
    }
    // corners
    dval9[0] += dval0[cols + 1];  // comparison with bottom-left neighbour
    dval9[0] += dval0[cols];      // comparison with bottom-central neighbour
    dval9[0] += dval0[1];         // comparison with middle-left neighbour
    dval9[0] /= 4.0;

    idx = cols - 1;
    dval9[idx] += dval0[idx + cols];      // comparison with bottom-central neighbour
    dval9[idx] += dval0[idx + cols - 1];  // comparison with bottom-right neighbour
    dval9[idx] += dval0[idx - 1];         // comparison with middle-right neighbour
    dval9[idx] /= 4.0;

    idx = (rows - 1) * cols;
    dval9[idx] += dval0[idx + 1];         // comparison with middle-left neighbour
    dval9[idx] += dval0[idx - cols + 1];  // comparison with top-left neighbour
    dval9[idx] += dval0[idx - cols];      // comparison with top-central neighbour
    dval9[idx] /= 4.0;

    idx = rows * cols - 1;
    dval9[idx] += dval0[idx - 1];         // comparison with middle-right neighbour
    dval9[idx] += dval0[idx - cols];      // comparison with top-central neighbour
    dval9[idx] += dval0[idx - cols - 1];  // comparison with top-right neighbour
    dval9[idx] /= 4.0;

#pragma omp parallel for shared(output, dval0, dval9)
    for (int i = 0; i < cols * rows; i++) {

        // setting dval0u to output
        output[i] = fmin(1.0, fmax(-1.0, dval0[i])) * sq(dval0[i]) /
                    (sq(dval0[i]) + 4.0 * readNoiseAmp2);
        // adding dval9u to output
        output[i] += fmax(fmin(dval9[i], mod_clip), -mod_clip) * sq(dval9[i]) /
                     (sq(dval9[i]) + 18.0 * readNoiseAmp2);

        int rcol = i % cols;
        double dmod1, dmod2;
        if (i < cols) {  // first row
            dmod1 = 0.0;
        } else {
            dmod1 = imageOut[i - cols] - imageOut[i];
        }
        output[i] += fmax(fmin(dmod1, mod_clip), -mod_clip) * 4 * readNoiseAmp2 /
                     (sq(dmod1) + 4.0 * readNoiseAmp2);

        if (i >= (rows - 1) * cols) {  // last row
            dmod2 = 0.0;
        } else {
            dmod2 = imageOut[i + cols] - imageOut[i];
        }
        output[i] += fmax(fmin(dmod2, mod_clip), -mod_clip) * 4 * readNoiseAmp2 /
                     (sq(dmod2) + 4.0 * readNoiseAmp2);

        if (smoothCol) {
            double cmod1, cmod2;
            if (rcol == 0) {  // first column
                cmod1 = 0.0;
            } else {
                cmod1 = imageOut[i - 1] - imageOut[i];
            }
            output[i] += fmax(fmin(cmod1, mod_clip), -mod_clip) * 4 * readNoiseAmp2 /
                         (sq(cmod1) + 4.0 * readNoiseAmp2);
            if (rcol == cols - 1) {  // last column
                cmod2 = 0.0;
            } else {
                cmod2 = imageOut[i + 1] - imageOut[i];
            }
            output[i] += fmax(fmin(cmod2, mod_clip), -mod_clip) * 4 * readNoiseAmp2 /
                         (sq(cmod2) + 4.0 * readNoiseAmp2);

            output[i] /= 6.0;
        } else {
            output[i] /= 4.0;
        }
    }

    free(dval9);
}

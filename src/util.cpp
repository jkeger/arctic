
#include <math.h>
#include <stdio.h>
#include <valarray>
#include <vector>

/*
    Neatly print a 1D array.
*/
void print_array(std::valarray<double>& array) {
    int n_col = array.size();

    printf("[");
    for (int i_col = 0; i_col < n_col; ++i_col) {
        printf("%g", array[i_col]);
        if (i_col != n_col - 1) printf(", ");
    }
    printf("]\n");

    return;
}

/*
    Neatly print an array as 2D with n_col columns (2nd dimension).
*/
void print_array_2D(std::valarray<double>& array, int n_col) {
    int n_tot = array.size();
    int n_row = n_tot / n_col;

    printf("[");
    for (int i_row = 0; i_row < n_row; ++i_row) {
        if (i_row == 0)
            printf("[");
        else
            printf(" [");
        for (int i_col = 0; i_col < n_col; ++i_col) {
            printf("%g", array[i_row * n_col + i_col]);
            if (i_col != n_col - 1)
                printf(", ");
            else if (i_row != n_row - 1)
                printf("]\n");
            else
                printf("]]\n");
        }
    }

    return;
}

/*
    Basic equivalent of numpy.arange().
*/
std::valarray<double> arange(double start, double stop, double step = 1) {
    // Create the array more easily as a vector
    std::vector<double> tmp_array;
    for (double value = start; value < stop; value += step) tmp_array.push_back(value);

    // Convert to a valarray
    std::valarray<double> array(tmp_array.data(), tmp_array.size());
    return array;
}

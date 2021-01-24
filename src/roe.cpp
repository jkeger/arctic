
#include <math.h>
#include <stdio.h>
#include <valarray>

#include "util.hpp"

/*
    Calculate the matrices of express multipliers and when to monitor traps.

    To reduce runtime, instead of calculating the effects of every
    pixel-to-pixel transfer, it is possible to approximate readout by
    processing each transfer once (Anderson et al. 2010) or a few times
    (Massey et al. 2014, section 2.1.5), then multiplying the effect of
    that transfer by the number of transfers it represents. This function
    computes the multiplicative factor, and returns it in a matrix that can
    be easily looped over.

    Parameters
    ----------
    n_pixels : int
        The number of pixels in one column of the image. i.e. the number of
        pixel-to-pixel transfers to move charge from the furthest pixel to the
        readout register (if there is no offset).

    express : int
        The number of times the pixel-to-pixel transfers are computed,
        determining the balance between accuracy (high values) and speed
        (low values).
            n_pixels    (slower, accurate) Compute every pixel-to-pixel
                        transfer. The default 0 is an alias for n_pixels.
            k           Recompute on k occasions the effect of each transfer.
                        After a few transfers (and e.g. eroded leading edges),
                        the incremental effect of subsequent transfers can
                        change.
            1           (faster, approximate) Compute the effect of each
                        transfer only once.

    offset : int (>= 0)
        Consider all pixels to be offset by this number of pixels from the
        readout register. Useful if working out the matrix for a postage
        stamp image, or to account for prescan pixels whose data is not
        stored.

    integer_express_matrix : bool
        Old versions of this algorithm assumed (unnecessarily) that all
        express multipliers must be integers. If
        force_release_away_from_readout ## is true (no effect if false), then
        it's slightly more efficient if this requirement is dropped, but the
        option to force it is included for backwards compatability.
    
    empty_traps_for_first_transfers : bool
        If True and if using express != n_pixels, then tweak the express
        algorithm to treat every first pixel-to-pixel transfer separately
        to the rest.

        Physically, the first pixel that a charge cloud finds itself in will
        start with empty traps; whereas every subsequent transfer sees traps
        that may have been filled previously. With the default express
        algorithm, this means the inherently different first-transfer would
        be replicated many times for some pixels but not others. This
        modification prevents that issue by modelling the first single
        transfer for each pixel separately and then using the express
        algorithm normally for the remainder.

    Returns
    -------
    express_matrix : [[float]]
        The express multiplier value for each pixel-to-pixel transfer.
*/
std::valarray<double> express_matrix_from_pixels_and_express(
    int n_pixels, int express = 0, int offset = 0, bool integer_express_matrix = false,
    bool empty_traps_for_first_transfers = true) {

    int n_transfers = n_pixels + offset;

    // Set default express to all transfers, and check no larger
    if (express == 0)
        express = n_transfers;
    else
        express = std::min(express, n_transfers);

    // Temporarily ignore the first pixel-to-pixel transfer, if it is to be
    // handled differently than the rest
    if ((empty_traps_for_first_transfers) && (express < n_pixels)) n_transfers--;

    // Initialise an array with enough pixels to contain the supposed image,
    // including offset
    std::valarray<double> express_matrix(0., express * n_transfers);

    // Temporary array for manipulating slices
    std::valarray<double> tmp_row(0., n_transfers);

    // Populate every row in the matrix with a range from 1 to n_transfers
    // (plus 1 because it starts at 1 not 0)
    for (int express_index = 0; express_index < express; express_index++)
        express_matrix[std::slice(express_index * n_transfers, n_transfers, 1)] =
            arange(1, n_transfers + 1);

    // Compute the multiplier factors
    double max_multiplier = (double)n_transfers / express;
    if (integer_express_matrix) max_multiplier = ceil(max_multiplier);
    // Offset each row to account for the pixels that have already been read out
    for (int express_index = 0; express_index < express; express_index++) {
        tmp_row =
            express_matrix[std::slice(express_index * n_transfers, n_transfers, 1)];
        express_matrix[std::slice(express_index * n_transfers, n_transfers, 1)] =
            tmp_row - express_index * max_multiplier;
    }
    // Truncate all values to between 0 and max_multiplier
    express_matrix[express_matrix < 0.] = 0.;
    express_matrix[express_matrix > max_multiplier] = max_multiplier;

    // Add an extra (first) transfer for every pixel, the effect of which
    // will only ever be counted once, because it is physically different
    // from the other transfers (it sees only empty traps)
    if ((empty_traps_for_first_transfers) && (express < n_pixels)) {
        // Create a new matrix for the full number of transfers
        n_transfers++;
        std::valarray<double> express_matrix_full(0., n_transfers * n_transfers);
        int i_transfer;
        for (int express_index = 0; express_index < n_transfers; express_index++) {
            i_transfer = n_transfers - express_index - 1;
            express_matrix_full[express_index * n_transfers + i_transfer] = 1;
        }

        // Insert the original transfers into the new matrix at appropriate places
        // print_array_2D(express_matrix_full, n_transfers);
        int new_index;
        for (int old_index = 0; old_index < express; old_index++) {
            // Count the number of non-zero transfers in each original row
            tmp_row = express_matrix[std::slice(
                old_index * (n_transfers - 1), n_transfers - 1, 1)];
            tmp_row[tmp_row > 0.] = 1;
            new_index = (int)tmp_row.sum();

            // Insert the original transfers
            express_matrix_full[std::slice(
                new_index * n_transfers + 1, n_transfers - 1, 1)] +=
                express_matrix[std::slice(
                    old_index * (n_transfers - 1), n_transfers - 1, 1)];
        }

        // Remove the offset (which is not represented in the image pixels)
        std::valarray<double> express_matrix_trim(0., n_transfers * n_pixels);

        // Copy the post-offset slices of each row
        for (int express_index = 0; express_index < n_transfers; express_index++) {
            express_matrix_trim[std::slice(express_index * n_pixels, n_pixels, 1)] =
                express_matrix_full[std::slice(
                    express_index * n_transfers + offset, n_pixels, 1)];
        }

        return express_matrix_trim;
    } else {
        // Remove the offset (which is not represented in the image pixels)
        std::valarray<double> express_matrix_trim(0., express * n_pixels);

        // Copy the post-offset slices of each row
        for (int express_index = 0; express_index < express; express_index++) {
            express_matrix_trim[std::slice(express_index * n_pixels, n_pixels, 1)] =
                express_matrix[std::slice(
                    express_index * n_transfers + offset, n_pixels, 1)];
        }

        return express_matrix_trim;
    }
}
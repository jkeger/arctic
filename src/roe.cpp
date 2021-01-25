
#include <math.h>
#include <stdio.h>
#include <valarray>

#include "roe.hpp"
#include "util.hpp"

/*
    Class ROE.

    Information about the read-out electronics.
    
    ##todo: reimplement multiple phases
    
    Parameters
    ----------
    dwell_times : double (opt.)
        The time between steps in the clocking sequence, in the same units
        as the trap capture/release timescales. Default 1.0.
                   
    empty_traps_between_columns : bool (opt.)
        true:  Each column has independent traps (appropriate for parallel
               clocking)
        false: Each column moves through the same traps, which therefore
               preserve occupancy, allowing trails to extend onto the next
               column (appropriate for serial clocking, if all prescan and
               overscan pixels are included in the image array). Default true.
    
    empty_traps_for_first_transfers : bool (opt.)
        If true (and express != n_pixels), then tweak the express algorithm to
        treat every first pixel-to-pixel transfer separately to the rest.
        Default true.
        
        Physically, the first pixel that a charge cloud finds itself in will
        start with empty traps, whereas every subsequent transfer sees traps
        that may have been filled previously. With the default express
        algorithm, this means the inherently different first-transfer would
        be replicated many times for some pixels but not others. This
        modification prevents that issue by modelling the first single
        transfer for each pixel separately and then using the express
        algorithm normally for the remainder.
        
    use_integer_express_matrix : bool (opt.)
        Old versions of this algorithm assumed (unnecessarily) that all
        express multipliers must be integers. It can be slightly more efficient
        if this requirement is dropped, but the option to force it is included
        for backwards compatability. Default false.
*/
ROE::ROE(
    double dwell_time, bool empty_traps_between_columns,
    bool empty_traps_for_first_transfers, bool use_integer_express_matrix)
    : dwell_time(dwell_time),
      empty_traps_for_first_transfers(empty_traps_for_first_transfers),
      empty_traps_between_columns(empty_traps_between_columns),
      use_integer_express_matrix(use_integer_express_matrix) {}

/*
    Set the matrix of express multipliers.

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

    offset : int (opt.)
        Consider all pixels to be offset by this number of pixels from the
        readout register. Useful if working out the matrix for a postage
        stamp image, or to account for prescan pixels whose data is not
        stored. Default 0.

    Sets
    ----
    express_matrix : std::valarray<double>
        The express multiplier value for each pixel-to-pixel transfer, as a
        2D-style 1D array.
        
    n_express_passes : int
        The number of express passes to run, i.e. the number of rows in the
        matrices.
*/
void ROE::set_express_matrix_from_pixels_and_express(
    int n_pixels, int express, int offset) {

    int n_transfers = n_pixels + offset;
    int n_rows;

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
    std::valarray<double> tmp_express_matrix(0.0, express * n_transfers);

    // Temporary array for manipulating slices
    std::valarray<double> tmp_row(0.0, n_transfers);

    // Populate every row in the matrix with a range from 1 to n_transfers
    // (plus 1 because it starts at 1 not 0)
    for (int express_index = 0; express_index < express; express_index++)
        tmp_express_matrix[std::slice(express_index * n_transfers, n_transfers, 1)] =
            arange(1, n_transfers + 1);

    // Compute the multiplier factors
    double max_multiplier = (double)n_transfers / express;
    if (use_integer_express_matrix) max_multiplier = ceil(max_multiplier);
    // Offset each row to account for the pixels that have already been read out
    for (int express_index = 0; express_index < express; express_index++) {
        tmp_row =
            tmp_express_matrix[std::slice(express_index * n_transfers, n_transfers, 1)];
        tmp_express_matrix[std::slice(express_index * n_transfers, n_transfers, 1)] =
            tmp_row - express_index * max_multiplier;
    }
    // Truncate all values to between 0 and max_multiplier
    tmp_express_matrix[tmp_express_matrix < 0.0] = 0.0;
    tmp_express_matrix[tmp_express_matrix > max_multiplier] = max_multiplier;

    // Add an extra (first) transfer for every pixel, the effect of which
    // will only ever be counted once, because it is physically different
    // from the other transfers (it sees only empty traps)
    if ((empty_traps_for_first_transfers) && (express < n_pixels)) {
        // Create a new matrix for the full number of transfers
        n_transfers++;
        std::valarray<double> express_matrix_full(0.0, n_transfers * n_transfers);
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
            tmp_row = tmp_express_matrix[std::slice(
                old_index * (n_transfers - 1), n_transfers - 1, 1)];
            tmp_row[tmp_row > 0.0] = 1.0;
            new_index = (int)tmp_row.sum();

            // Insert the original transfers
            express_matrix_full[std::slice(
                new_index * n_transfers + 1, n_transfers - 1, 1)] +=
                tmp_express_matrix[std::slice(
                    old_index * (n_transfers - 1), n_transfers - 1, 1)];
        }

        tmp_express_matrix = express_matrix_full;
        n_express_passes = n_transfers;
    } else {
        n_express_passes = express;
    }

    // Remove the offset (which is not represented in the image pixels)
    std::valarray<double> express_matrix_trim(0.0, n_express_passes * n_pixels);

    // Copy the post-offset slices of each row
    for (int express_index = 0; express_index < n_express_passes; express_index++) {
        express_matrix_trim[std::slice(express_index * n_pixels, n_pixels, 1)] =
            tmp_express_matrix[std::slice(
                express_index * n_transfers + offset, n_pixels, 1)];
    }

    express_matrix = express_matrix_trim;
}

/*
    Set the accompanying array to the express matrix of when to store the trap
    occupancy states.
    
    Allows the next express iteration to continue from an (approximately)
    suitable configuration by restoring the saved states.
    
    If the traps start empty (rather than restored), then the first capture in
    each express loop is different from the rest: many electrons are lost. This
    behaviour may be appropriate for the first pixel-to-pixel transfer of
    each charge cloud, but not for subsequent transfers. It particularly
    causes problems if the first transfer is used to represent many
    transfers through the express mechanism, as the large loss of electrons
    is multiplied up.
     
    Returns
    -------
    store_trap_states_matrix : std::valarray<bool>
        For each pixel-to-pixel transfer, set true to store the trap states, as
        a 2D-style 1D array.
*/
void ROE::set_store_trap_states_matrix() {
    store_trap_states_matrix = std::valarray<bool>(false, express_matrix.size());
    int n_transfers = express_matrix.size() / n_express_passes;
    int row_index;

    // No need to store states if already using empty traps for first transfers
    if (empty_traps_for_first_transfers) return;

    // Store on the pixel before where the next express pass will begin, so
    // that the trap states are appropriate for continuing in the next pass
    for (int express_index = 0; express_index < n_express_passes - 1; express_index++) {
        for (row_index = 0; row_index < n_transfers - 1; row_index++) {
            if (express_matrix[(express_index + 1) * n_transfers + row_index + 1] > 0.0)
                break;
        }
        store_trap_states_matrix[express_index * n_transfers + row_index] = true;
    }
}
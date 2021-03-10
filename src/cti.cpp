
#include <stdio.h>
#include <sys/time.h>
#include <valarray>

#include "ccd.hpp"
#include "cti.hpp"
#include "roe.hpp"
#include "trap_managers.hpp"
#include "traps.hpp"
#include "util.hpp"

/*
    Add CTI trails to an image by trapping, releasing, and moving electrons
    along their independent columns.
    
    See add_cti() for more detail and e.g. parallel vs serial clocking.

    Parameters
    ----------
    image_in : std::valarray<std::valarray<double>>
        The input array of pixel values, assumed to be in units of electrons.

        The first dimension is the "row" index, the second is the "column"
        index. Charge is transferred "up" from row n to row 0 along each
        independent column.

    roe : ROE
    ccd : CCD
    traps : std::valarray<Trap>
    express : int (opt.)
    offset : int (opt.)
        See add_cti(). Same as the corresponding parallel_* argument docstrings,
        except here not as pointers since there's no need for nullptr defaults.

    row_start, row_stop : int (opt.)
        The subset of row pixels to model, to save time when only a specific
        region of the image is of interest. Defaults to 0, n_rows for the full
        image.

    column_start, column_stop : int (opt.)
        The subset of column pixels to model, to save time when only a specific
        region of the image is of interest. Defaults to 0, n_columns for the
        full image.

    Returns
    -------
    image : std::valarray<std::valarray<double>>
        The output array of pixel values.
*/
std::valarray<std::valarray<double>> clock_charge_in_one_direction(
    std::valarray<std::valarray<double>>& image_in, ROE roe, CCD ccd,
    std::valarray<Trap> traps, int express, int offset, int row_start, int row_stop,
    int column_start, int column_stop) {

    // Initialise the output image as a copy of the input image
    std::valarray<std::valarray<double>> image = image_in;

    // Image shape
    int n_rows = image.size();
    int n_columns = image[0].size();

    // Defaults
    if (row_stop == 0) row_stop = n_rows;
    if (column_stop == 0) column_stop = n_columns;

    // Number of rows and columns to process
    n_rows = row_stop - row_start;
    n_columns = column_stop - column_start;

    roe.set_express_matrix_from_pixels_and_express(n_rows, express, offset);
    roe.set_store_trap_states_matrix();

    double n_free_electrons;
    double n_electrons_released_and_captured;
    double express_multiplier;

    // Set up the trap manager
    TrapManagerInstantCapture trap_manager(traps, n_rows, ccd);
    trap_manager.initialise_trap_states();
    trap_manager.set_fill_probabilities_from_dwell_time(roe.dwell_time);

    // Measure wall-clock time taken for the primary loop
    struct timeval wall_time_start;
    struct timeval wall_time_end;
    double wall_time_elapsed;
    gettimeofday(&wall_time_start, nullptr);

    // ========
    // Clock each column of pixels through the column of traps
    // ========
    for (int column_index = column_start; column_index < column_stop; column_index++) {
        print_v(2, "# column_index %d \n", column_index);

        // Monitor the traps in every pixel (express=n_rows), or just one
        // (express=1) or a few (express=a few) then replicate their effect
        for (int express_index = 0; express_index < roe.n_express_passes;
             express_index++) {
            print_v(2, "express_index %d \n", express_index);

            // Restore the trap occupancy levels, either to empty or to a saved
            // state from a previous express pass
            trap_manager.restore_trap_states();

            // Each pixel
            for (int row_index = row_start; row_index < row_stop; row_index++) {
                print_v(2, "# row_index %d \n", row_index);

                express_multiplier =
                    roe.express_matrix[express_index * n_rows + row_index];
                if (express_multiplier == 0) continue;

                n_free_electrons = image[row_index][column_index];

                print_v(2, "express_multiplier %g \n", express_multiplier);
                print_v(2, "n_free_electrons %g \n", n_free_electrons);

                // Release and capture electrons with the traps in this pixel
                n_electrons_released_and_captured =
                    trap_manager.n_electrons_released_and_captured(n_free_electrons);

                image[row_index][column_index] +=
                    n_electrons_released_and_captured * express_multiplier;

                // Store the trap states if needed for the next express pass
                if (roe.store_trap_states_matrix[express_index * n_rows + row_index])
                    trap_manager.store_trap_states();

                print_v(
                    2, "n_electrons_released_and_captured %g \n",
                    n_electrons_released_and_captured);
                print_v(
                    2, "image[%d][%d] %g \n", row_index, column_index,
                    image[row_index][column_index]);
            }
        }

        // Reset the trap states to empty and/or store them for the next column
        if (roe.empty_traps_between_columns) trap_manager.reset_trap_states();
        trap_manager.store_trap_states();
    }

    // Time taken
    gettimeofday(&wall_time_end, nullptr);
    wall_time_elapsed = gettimelapsed(wall_time_start, wall_time_end);
    print_v(1, "Wall-clock time elapsed: %.4g s \n", wall_time_elapsed);

    return image;
}

/*
    Add CTI trails to an image by trapping, releasing, and moving electrons
    along their independent columns, for parallel and/or serial clocking.

    Parameters
    ----------
    image_in : std::valarray<std::valarray<double>>
        The input array of pixel values, assumed to be in units of electrons.

        The first dimension is the "row" index, the second is the "column"
        index. By default (for parallel clocking), charge is transfered "up"
        from row n to row 0 along each independent column. i.e. the readout
        register is above row 0. (For serial clocking, the image is rotated
        before modelling, such that charge moves from column n to column 0.)

        e.g.
        Initial image with one bright pixel in the first three columns:
            [[0.0,     0.0,     0.0,     0.0  ],
             [200.0,   0.0,     0.0,     0.0  ],
             [0.0,     200.0,   0.0,     0.0  ],
             [0.0,     0.0,     200.0,   0.0  ],
             [0.0,     0.0,     0.0,     0.0  ],
             [0.0,     0.0,     0.0,     0.0  ]]
        Image with parallel CTI trails:
            [[0.0,     0.0,     0.0,     0.0  ],
             [196.0,   0.0,     0.0,     0.0  ],
             [3.0,     194.1,   0.0,     0.0  ],
             [2.0,     3.9,     192.1,   0.0  ],
             [1.3,     2.5,     4.8,     0.0  ],
             [0.8,     1.5,     2.9,     0.0  ]]
        Final image with parallel and serial CTI trails:
            [[0.0,     0.0,     0.0,     0.0  ],
             [194.1,   1.9,     1.5,     0.9  ],
             [2.9,     190.3,   2.9,     1.9  ],
             [1.9,     3.8,     186.5,   3.7  ],
             [1.2,     2.4,     4.7,     0.1  ],
             [0.7,     1.4,     2.8,     0.06 ]]

    parallel_roe : ROE* (opt.)
        The object describing the clocking read-out electronics for parallel
        clocking. Default nullptr to not do parallel clocking.

    parallel_ccd : CCD* (opt.)
        The object describing the CCD volume for parallel clocking. For
        multi-phase clocking optionally use a list of different CCD volumes
        for each phase, in the same size list as parallel_roe.dwell_times.

    parallel_traps : std::valarray<Trap>* (opt.)
        A list of one or more trap species objects for parallel clocking.

    parallel_express : int (opt.)
       The number of times the transfers are computed, determining the
       balance between accuracy (high values) and speed (low values), for
       parallel clocking (Massey et al. 2014, section 2.1.5).
           n_rows  (slower, accurate) Compute every pixel-to-pixel
                   transfer. The default, 0, is an alias for n_rows.
           k       Recompute on k occasions the effect of each transfer.
                   After a few transfers (and e.g. eroded leading edges),
                   the incremental effect of subsequent transfers can change.
           1       (faster, approximate) Compute the effect of each
                   transfer only once.

    parallel_offset : int (>= 0) (opt.)
        The number of (e.g. prescan) pixels separating the supplied image from
        the readout register. i.e. Treat the input image as a sub-image that is
        offset this number of pixels from readout, increasing the number of
        pixel-to-pixel transfers. Defaults to 0.

    // parallel_window_range : range
    //     For speed, calculate only the effect on this subset of pixels. Defaults
    //     to range(0, n_pixels) for the full image.
    //
    //     Note that, because of edge effects, the range should be started several
    //     pixels before the actual region of interest.
    //
    //     For a single pixel (e.g. for trap pumping), can enter just the single
    //     integer index of the pumping traps to monitor, which will be converted
    //     to range(index, index + 1).

    serial_* : * (opt.)
        The same as the parallel_* objects described above but for serial
        clocking instead. Default nullptr to not do serial clocking.

    // time_window_range : range
    //     The subset of transfers to implement. Defaults to range(0, n_pixels) for
    //     the full image. e.g. range(0, n_pixels/3) to do only the first third of
    //     the pixel-to-pixel transfers.
    //
    //     The entire readout is still modelled, but only the results from this
    //     subset of transfers are implemented in the final image.
    //
    //     This could be used to e.g. add cosmic rays during readout of simulated
    //     images. Successive calls to complete the readout should start at
    //     the same value that the previous one ended, e.g. range(0, 1000) then
    //     range(1000, 2000). Be careful not to divide the readout too finely, as
    //     there is only as much temporal resolution as there are rows (not rows *
    //     phases) in the image. Also, for each time that readout is split between
    //     successive calls to this function, the output in one row of pixels
    //     will change slightly (unless express=0) because trap occupancy is
    //     not stored between calls.

    Returns
    -------
    image : std::valarray<std::valarray<double>>
        The output array of pixel values with CTI added.
*/
std::valarray<std::valarray<double>> add_cti(
    std::valarray<std::valarray<double>>& image_in, ROE* parallel_roe,
    CCD* parallel_ccd, std::valarray<Trap>* parallel_traps, int parallel_express,
    int parallel_offset, ROE* serial_roe, CCD* serial_ccd,
    std::valarray<Trap>* serial_traps, int serial_express, int serial_offset) {

    // Initialise the output image as a copy of the input image
    std::valarray<std::valarray<double>> image = image_in;

    // Parallel clocking along columns, transfer charge towards row 0
    if (parallel_traps) {
        image = clock_charge_in_one_direction(
            image, *parallel_roe, *parallel_ccd, *parallel_traps, parallel_express,
            parallel_offset);
    }

    // Serial clocking along rows, transfer charge towards column 0
    if (serial_traps) {
        image = transpose(image);

        image = clock_charge_in_one_direction(
            image, *serial_roe, *serial_ccd, *serial_traps, serial_express,
            serial_offset);

        image = transpose(image);
    }

    return image;
}

/*
    Remove CTI trails from an image by first modelling the addition of CTI.

    See add_cti()'s documentation for the forward modelling. This function
    iteratively models the addition of more CTI trails to the input image to
    then extract the corrected image without the original trails.

    Parameters
    ----------
    All parameters are identical to those of add_cti() as described in its
    documentation, with the exception of:

    iterations : int
        The number of times CTI-adding clocking is run to perform the correction
        via forward modelling.

    Returns
    -------
    image : std::valarray<std::valarray<double>>
        The output array of pixel values with CTI removed.
*/
std::valarray<std::valarray<double>> remove_cti(
    std::valarray<std::valarray<double>>& image_in, int iterations, ROE* parallel_roe,
    CCD* parallel_ccd, std::valarray<Trap>* parallel_traps, int parallel_express,
    int parallel_offset, ROE* serial_roe, CCD* serial_ccd,
    std::valarray<Trap>* serial_traps, int serial_express, int serial_offset) {

    // Initialise the output image as a copy of the input image
    std::valarray<std::valarray<double>> image_remove_cti = image_in;
    std::valarray<std::valarray<double>> image_add_cti;

    // Estimate the image with removed CTI more accurately each iteration
    for (int iteration = 1; iteration <= iterations; iteration++) {
        // Model the effect of adding CTI trails
        image_add_cti = add_cti(
            image_remove_cti, parallel_roe, parallel_ccd, parallel_traps,
            parallel_express, parallel_offset, serial_roe, serial_ccd, serial_traps,
            serial_express, serial_offset);

        // Improve the estimate of the image with CTI trails removed
        image_remove_cti += image_in - image_add_cti;
    }

    return image_remove_cti;
}

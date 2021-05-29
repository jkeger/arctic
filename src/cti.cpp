
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
        index. Charge is transferred "up" from row N to row 0 along each
        independent column.

    roe : ROE*
    ccd : CCD*
    instant_capture_traps : std::valarray<TrapInstantCapture>*
    slow_capture_traps : std::valarray<TrapSlowCapture>*
    express : int (opt.)
    offset : int (opt.)
        See add_cti()'s docstring. Same as the corresponding parallel_*
        parameters.

    row_start, row_stop : int (opt.)
        The subset of row pixels to model, to save time when only a specific
        region of the image is of interest. Defaults to 0, n_rows for the full
        image.
        
        For trap pumping, it is currently assumed that only a single pixel is
        active and contains traps, so row_stop must be row_start + 1. See
        ROETrapPumping for more detail.

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
    std::valarray<std::valarray<double>>& image_in, ROE* roe, CCD* ccd,
    std::valarray<TrapInstantCapture>* instant_capture_traps,
    std::valarray<TrapSlowCapture>* slow_capture_traps, int express, int offset,
    int row_start, int row_stop, int column_start, int column_stop) {

    // Initialise the output image as a copy of the input image
    std::valarray<std::valarray<double>> image = image_in;

    // Image shape
    unsigned int n_rows = image.size();
    unsigned int n_columns = image[0].size();

    // Defaults
    if (row_stop == -1) row_stop = n_rows;
    if (column_stop == -1) column_stop = n_columns;

    // Number of active rows and columns
    unsigned int n_active_rows = row_stop - row_start;
    unsigned int n_active_columns = column_stop - column_start;
    print_v(
        1, "Clock charge in %d column(s) [%d to %d] and %d row(s) [%d to %d] \n",
        n_active_columns, column_start, column_stop, n_active_rows, row_start,
        row_stop);

    // Checks for non-standard modes
    if ((roe->type == roe_type_trap_pumping) && (n_active_rows != 1))
        error(
            "TrapSlowCapture pumping currently requires the number of active rows (%d) "
            "to be 1",
            n_active_rows);

    // Set up the readout electronics and express arrays
    roe->set_clock_sequence();
    roe->set_express_matrix_from_rows_and_express(n_rows, express, offset);
    roe->set_store_trap_states_matrix();
    if (ccd->n_phases != roe->n_phases)
        error(
            "Number of CCD phases (%d) and ROE phases (%d) don't match.", ccd->n_phases,
            roe->n_phases);

    // Set empty arrays for nullptr trap lists
    if (slow_capture_traps == nullptr) {
        std::valarray<TrapSlowCapture> no_slow_capture_traps = {};
        slow_capture_traps = &no_slow_capture_traps;
    }
    if (instant_capture_traps == nullptr) {
        std::valarray<TrapInstantCapture> no_instant_capture_traps = {};
        instant_capture_traps = &no_instant_capture_traps;
    }

    // Set up the trap managers
    TrapManagerManager trap_manager_manager(
        *slow_capture_traps, *instant_capture_traps, row_stop - row_start, *ccd,
        roe->dwell_times);

    unsigned int column_index;
    unsigned int row_index;
    unsigned int row_read;
    unsigned int row_write;
    double n_free_electrons;
    double n_electrons_released_and_captured;
    double express_multiplier;
    ROEStepPhase* roe_step_phase;

    // Print all inputs
    if (verbosity >= 2) {
        printf("\n");
        printf(
            "express = %d \n"
            "offset = %d, row_start = %d, row_stop = %d, column_start = %d, "
            "column_stop = %d \n",
            express, offset, row_start, row_stop, column_start, column_stop);

        printf("ROE n_steps = %d \n", roe->n_steps);
        printf("  dwell_times = ");
        print_array(roe->dwell_times);
        printf(
            "  empty_traps_between_columns = %d \n", roe->empty_traps_between_columns);
        printf(
            "  empty_traps_for_first_transfers = %d \n",
            roe->empty_traps_for_first_transfers);
        printf(
            "  force_release_away_from_readout = %d \n",
            roe->force_release_away_from_readout);
        printf("  use_integer_express_matrix = %d \n", roe->use_integer_express_matrix);

        printf("CCD n_phases = %d \n", ccd->n_phases);
        printf("  fraction_of_traps_per_phase = ");
        print_array(ccd->fraction_of_traps_per_phase);
        for (int i_phase = 0; i_phase < ccd->n_phases; i_phase++) {
            printf(
                "  full_well_depth = %g, well_notch_depth = %g, well_fill_power = %g "
                "\n",
                ccd->phases[i_phase].full_well_depth,
                ccd->phases[i_phase].well_notch_depth,
                ccd->phases[i_phase].well_fill_power);
        }

        printf("Standard traps n = %d \n", trap_manager_manager.n_slow_capture_traps);
        for (int i_trap = 0; i_trap < trap_manager_manager.n_slow_capture_traps;
             i_trap++) {
            printf(
                "  density = %g, release_timescale = %g, capture_timescale = %g \n",
                trap_manager_manager.trap_managers_slow_capture[0]
                    .traps[i_trap]
                    .density,
                trap_manager_manager.trap_managers_slow_capture[0]
                    .traps[i_trap]
                    .release_timescale,
                trap_manager_manager.trap_managers_slow_capture[0]
                    .traps[i_trap]
                    .capture_timescale);
        }
        printf(
            "Instant-capture traps n = %d \n",
            trap_manager_manager.n_instant_capture_traps);
        for (int i_trap = 0; i_trap < trap_manager_manager.n_instant_capture_traps;
             i_trap++) {
            printf(
                "  density = %g, release_timescale = %g \n",
                trap_manager_manager.trap_managers_instant_capture[0]
                    .traps[i_trap]
                    .density,
                trap_manager_manager.trap_managers_instant_capture[0]
                    .traps[i_trap]
                    .release_timescale);
        }
        printf("\n");
    }

    // Measure wall-clock time taken for the primary loop
    struct timeval wall_time_start;
    struct timeval wall_time_end;
    double wall_time_elapsed;
    gettimeofday(&wall_time_start, nullptr);

    // ========
    // Clock each column of pixels through the column of traps
    // ========
    // Loop over:
    //   Columns > Express passes > Rows > Clock-sequence steps > Pixel phases
    for (unsigned int i_column = 0; i_column < n_active_columns; i_column++) {
        column_index = column_start + i_column;

        print_v(
            2, "# # # #  i_column, column_index  %d,  %d \n", i_column, column_index);

        // Monitor the traps for every transfer (express=n_rows), or just one
        // (express=1) or a few (express=a few) then replicate their effect
        for (unsigned int express_index = 0; express_index < roe->n_express_passes;
             express_index++) {

            print_v(2, "# # #  express_index  %d \n", express_index);

            // Restore the trap occupancy levels, either to empty or to a saved
            // state from a previous express pass
            trap_manager_manager.restore_trap_states();

            // Each pixel
            for (unsigned int i_row = 0; i_row < n_active_rows; i_row++) {
                row_index = row_start + i_row;

                print_v(2, "# #  i_row, row_index  %d,  %d \n", i_row, row_index);

                express_multiplier =
                    roe->express_matrix[express_index * n_rows + row_index];
                if (express_multiplier == 0) continue;

                print_v(2, "express_multiplier  %g \n", express_multiplier);

                // Each step in the clock sequence
                for (unsigned int i_step = 0; i_step < roe->n_steps; i_step++) {

                    // Each phase in the pixel
                    for (unsigned int i_phase = 0; i_phase < ccd->n_phases; i_phase++) {

                        if ((roe->n_steps > 1) || (ccd->n_phases > 1))
                            print_v(
                                2, "#  i_step, i_phase  %d,  %d \n", i_step, i_phase);

                        // State of the ROE in this step and phase of the sequence
                        roe_step_phase = &roe->clock_sequence[i_step][i_phase];

                        // Get the initial charge from the relevant pixel(s)
                        n_free_electrons = 0;
                        for (int i = 0; i < roe_step_phase->n_capture_pixels; i++) {
                            row_read = row_index +
                                       roe_step_phase->capture_from_which_pixels[i];

                            n_free_electrons += image[row_read][column_index];
                        }

                        print_v(2, "row_read  %d \n", row_read);
                        print_v(2, "n_free_electrons  %g \n", n_free_electrons);

                        // Release and capture electrons with the traps in this
                        // pixel/phase, for each type of traps
                        n_electrons_released_and_captured = 0;
                        if (trap_manager_manager.n_slow_capture_traps > 0)
                            n_electrons_released_and_captured +=
                                trap_manager_manager.trap_managers_slow_capture[i_phase]
                                    .n_electrons_released_and_captured(
                                        n_free_electrons);
                        if (trap_manager_manager.n_instant_capture_traps > 0)
                            n_electrons_released_and_captured +=
                                trap_manager_manager
                                    .trap_managers_instant_capture[i_phase]
                                    .n_electrons_released_and_captured(
                                        n_free_electrons);

                        print_v(
                            2, "n_electrons_released_and_captured  %g \n",
                            n_electrons_released_and_captured);

                        // Return the charge to the relevant pixel(s)
                        for (int i = 0; i < roe_step_phase->n_release_pixels; i++) {
                            row_write =
                                row_index + roe_step_phase->release_to_which_pixels[i];

                            image[row_write][column_index] +=
                                n_electrons_released_and_captured * express_multiplier *
                                roe_step_phase->release_fraction_to_pixels[i];

                            // Make sure image counts don't go negative, which
                            // could happen with a too-large express multiplier
                            if (image[row_write][column_index] < 0.0)
                                image[row_write][column_index] = 0.0;

                            print_v(2, "row_write  %d \n", row_write);
                            print_v(
                                2, "image[%d][%d]  %g \n", row_write, column_index,
                                image[row_write][column_index]);
                        }
                    }
                }

                // Store the trap states if needed for the next express pass
                if (roe->store_trap_states_matrix[express_index * n_rows + row_index]) {
                    trap_manager_manager.store_trap_states();

                    print_v(2, "store_trap_states \n");
                }
            }
        }

        // Reset the trap states to empty and/or store them for the next column
        if (roe->empty_traps_between_columns) trap_manager_manager.reset_trap_states();
        trap_manager_manager.store_trap_states();
    }

    // Time taken
    gettimeofday(&wall_time_end, nullptr);
    wall_time_elapsed = gettimelapsed(wall_time_start, wall_time_end);
    print_v(1, "Wall-clock time elapsed:  %.4g s \n", wall_time_elapsed);

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
        from row N to row 0 along each independent column. i.e. the readout
        register is above row 0. For serial clocking, the image is rotated
        before modelling, such that charge moves from column M to column 0.

        e.g.
        Initial image with one bright pixel in the first three columns:
            {{  0.0,     0.0,     0.0,     0.0  },
             {  200.0,   0.0,     0.0,     0.0  },
             {  0.0,     200.0,   0.0,     0.0  },
             {  0.0,     0.0,     200.0,   0.0  },
             {  0.0,     0.0,     0.0,     0.0  },
             {  0.0,     0.0,     0.0,     0.0  }}
        Image with parallel CTI trails:
            {{  0.0,     0.0,     0.0,     0.0  },
             {  196.0,   0.0,     0.0,     0.0  },
             {  3.0,     194.1,   0.0,     0.0  },
             {  2.0,     3.9,     192.1,   0.0  },
             {  1.3,     2.5,     4.8,     0.0  },
             {  0.8,     1.5,     2.9,     0.0  }}
        Final image with parallel and serial CTI trails:
            {{  0.0,     0.0,     0.0,     0.0  },
             {  194.1,   1.9,     1.5,     0.9  },
             {  2.9,     190.3,   2.9,     1.9  },
             {  1.9,     3.8,     186.5,   3.7  },
             {  1.2,     2.4,     4.7,     0.1  },
             {  0.7,     1.4,     2.8,     0.06 }}

    parallel_roe : ROE* (opt.)
        The object describing the clocking read-out electronics, for parallel
        clocking. Default nullptr to not do parallel clocking.

    parallel_ccd : CCD* (opt.)
        The object describing the CCD volume, for parallel clocking.

    parallel_instant_capture_traps : std::valarray<TrapInstantCapture>* (opt.)
    parallel_slow_capture_traps : std::valarray<TrapSlowCapture>* (opt.)
        The arrays of trap species objects, one for each type (which can be
        empty, or nullptr), for parallel clocking.

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
        offset by this number of pixels from readout, increasing the number of
        pixel-to-pixel transfers. Defaults to 0.

    parallel_window_start, parallel_window_stop : int (opt.)
        Calculate only the effect on this subset of pixels, to save time when
        only a specific region of the image is of interest. Defaults to 0,
        n_rows for the full image.

        Note that, because of edge effects, the range should be started several
        pixels before the actual region of interest.

    serial_* : * (opt.)
        The same as the parallel_* objects described above but for serial
        clocking instead. Default nullptr to not do serial clocking.

    Returns
    -------
    image : std::valarray<std::valarray<double>>
        The output array of pixel values with CTI added.
*/
std::valarray<std::valarray<double>> add_cti(
    std::valarray<std::valarray<double>>& image_in,
    // Parallel
    ROE* parallel_roe, CCD* parallel_ccd,
    std::valarray<TrapInstantCapture>* parallel_instant_capture_traps,
    std::valarray<TrapSlowCapture>* parallel_slow_capture_traps, int parallel_express,
    int parallel_offset, int parallel_window_start, int parallel_window_stop,
    // Serial
    ROE* serial_roe, CCD* serial_ccd,
    std::valarray<TrapInstantCapture>* serial_instant_capture_traps,
    std::valarray<TrapSlowCapture>* serial_slow_capture_traps, int serial_express,
    int serial_offset, int serial_window_start, int serial_window_stop) {

    // Initialise the output image as a copy of the input image
    std::valarray<std::valarray<double>> image = image_in;

    // Parallel clocking along columns, transfer charge towards row 0
    if (parallel_instant_capture_traps || parallel_slow_capture_traps) {
        image = clock_charge_in_one_direction(
            image, parallel_roe, parallel_ccd, parallel_instant_capture_traps,
            parallel_slow_capture_traps, parallel_express, parallel_offset,
            parallel_window_start, parallel_window_stop, serial_window_start,
            serial_window_stop);
    }

    // Serial clocking along rows, transfer charge towards column 0
    if (serial_instant_capture_traps || serial_slow_capture_traps) {
        image = transpose(image);

        image = clock_charge_in_one_direction(
            image, serial_roe, serial_ccd, serial_instant_capture_traps,
            serial_slow_capture_traps, serial_express, serial_offset,
            serial_window_start, serial_window_stop, parallel_window_start,
            parallel_window_stop);

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

    n_iterations : int
        The number of times CTI-adding clocking is run to perform the correction
        via forward modelling. More iterations provide better results at the
        cost of longer runtime. In practice, two or three iterations are often
        sufficient.

    Returns
    -------
    image : std::valarray<std::valarray<double>>
        The output array of pixel values with CTI removed.
*/
std::valarray<std::valarray<double>> remove_cti(
    std::valarray<std::valarray<double>>& image_in, int n_iterations,
    // Parallel
    ROE* parallel_roe, CCD* parallel_ccd,
    std::valarray<TrapInstantCapture>* parallel_instant_capture_traps,
    std::valarray<TrapSlowCapture>* parallel_slow_capture_traps, int parallel_express,
    int parallel_offset, int parallel_window_start, int parallel_window_stop,
    // Serial
    ROE* serial_roe, CCD* serial_ccd,
    std::valarray<TrapInstantCapture>* serial_instant_capture_traps,
    std::valarray<TrapSlowCapture>* serial_slow_capture_traps, int serial_express,
    int serial_offset, int serial_window_start, int serial_window_stop) {

    // Initialise the output image as a copy of the input image
    std::valarray<std::valarray<double>> image_remove_cti = image_in;
    std::valarray<std::valarray<double>> image_add_cti;

    int n_rows = image_in.size();

    // Estimate the image with removed CTI more accurately each iteration
    for (int iteration = 1; iteration <= n_iterations; iteration++) {
        // Model the effect of adding CTI trails
        image_add_cti = add_cti(
            image_remove_cti, parallel_roe, parallel_ccd,
            parallel_instant_capture_traps, parallel_slow_capture_traps,
            parallel_express, parallel_offset, parallel_window_start,
            parallel_window_stop, serial_roe, serial_ccd, serial_instant_capture_traps,
            serial_slow_capture_traps, serial_express, serial_offset,
            serial_window_start, serial_window_stop);

        // Improve the estimate of the image with CTI trails removed
        image_remove_cti += image_in - image_add_cti;

        // Prevent negative image values
        for (int row_index = 0; row_index < n_rows; row_index++) {
            image_remove_cti[row_index][image_remove_cti[row_index] < 0.0] = 0.0;
        }
    }

    return image_remove_cti;
}


#include "cti.hpp"

#include <stdio.h>
#include <sys/time.h>

#include <valarray>
#include <iostream>

#include "ccd.hpp"
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
    image_in : std::valarray<std::valarray<double> >
        The input array of pixel values, assumed to be in units of electrons.

        The first dimension is the "row" index, the second is the "column"
        index. Charge is transferred "up" from row N to row 0 along each
        independent column.

    roe : ROE*
    ccd : CCD*
    traps_ic : std::valarray<TrapInstantCapture>*
    traps_sc : std::valarray<TrapSlowCapture>*
    traps_ic_co : std::valarray<TrapInstantCaptureContinuum>*
    traps_sc_co : std::valarray<TrapSlowCaptureContinuum>*
    express : int (opt.)
    row_offset : int (opt.)
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

    print_inputs : int (opt.)
        Whether or not to print the model inputs. Defaults to True if
        verbosity >= 1.

    Returns
    -------
    image : std::valarray<std::valarray<double> >
        The output array of pixel values.
*/
std::valarray<std::valarray<double> > clock_charge_in_one_direction(
    std::valarray<std::valarray<double> >& image_in, ROE* roe, CCD* ccd,
    std::valarray<TrapInstantCapture>* traps_ic,
    std::valarray<TrapSlowCapture>* traps_sc,
    std::valarray<TrapInstantCaptureContinuum>* traps_ic_co,
    std::valarray<TrapSlowCaptureContinuum>* traps_sc_co, 
    int express, int row_offset,
    int row_start, int row_stop, 
    int column_start, int column_stop, 
    int time_start, int time_stop, 
    double prune_n_electrons, int prune_frequency,
    int allow_negative_pixels, int print_inputs) {
    
    // Initialise the output image as a copy of the input image
    std::valarray<std::valarray<double> > image = image_in;

    // Image shape
    unsigned int n_rows = image.size();
    unsigned int n_columns = image[0].size();

    // Defaults
    if (row_stop == -1) row_stop = n_rows;
    if (column_stop == -1) column_stop = n_columns;

    // Number of active rows and columns
    unsigned int n_active_rows = row_stop - row_start;
    unsigned int n_active_columns = column_stop - column_start;
    unsigned int max_n_transfers = n_active_rows + row_offset;
    print_v(
        1, "%d column(s) [%d to %d], %d row(s) [%d to %d] \n", n_active_columns,
        column_start, column_stop, n_active_rows, row_start, row_stop);

    // Checks for non-standard modes
    if ((roe->type == roe_type_trap_pumping) && (n_active_rows != 1))
        error(
            "TrapSlowCapture pumping currently requires the number of active rows (%d) "
            "to be 1",
            n_active_rows);

    // Set up the readout electronics and express arrays
    roe->set_clock_sequence();
    roe->set_express_matrix_from_rows_and_express(n_rows, express, row_offset);
    roe->set_store_trap_states_matrix();
    if (ccd->n_phases != roe->n_phases)
        error(
            "Number of CCD phases (%d) and ROE phases (%d) don't match.", ccd->n_phases,
            roe->n_phases);
    if (!roe->empty_traps_between_columns) {
        // Account for the complete set of capture/release events that might
        // need to be tracked if the traps are never reset
        max_n_transfers *= n_columns;
    }

    // Set empty arrays for nullptr trap lists
    std::valarray<TrapInstantCapture> no_traps_ic = {};
    if (traps_ic == nullptr) {
        traps_ic = &no_traps_ic;
    }
    std::valarray<TrapSlowCapture> no_traps_sc = {};
    if (traps_sc == nullptr) {
        traps_sc = &no_traps_sc;
    }
    std::valarray<TrapInstantCaptureContinuum> no_continuum_traps = {};
    if (traps_ic_co == nullptr) {
        traps_ic_co = &no_continuum_traps;
    }
    std::valarray<TrapSlowCaptureContinuum> no_traps_sc_co = {};
    if (traps_sc_co == nullptr) {
        traps_sc_co = &no_traps_sc_co;
    }

    // Set up the trap managers
    TrapManagerManager trap_manager_manager(
        *traps_ic, *traps_sc, *traps_ic_co, *traps_sc_co, max_n_transfers, *ccd,
        roe->dwell_times);

    unsigned int column_index;
    unsigned int row_index;
    unsigned int row_read;
    unsigned int row_write;
    double n_free_electrons;
    double n_electrons_released_and_captured;
    double express_multiplier;
    ROEStepPhase* roe_step_phase;

    // Print model inputs
    //if (print_inputs == -1) print_inputs = verbosity >= 1;
    if (print_inputs > 0) {
        print_v(2, "\n");
        printf("  express = %d \n", express);
        if (row_offset != 0) printf("  row_offset = %d \n", row_offset);

        printf("  ROE type = %d, n_steps = %d \n", roe->type, roe->n_steps);
        printf("    dwell_times = ");
        print_array(roe->dwell_times);
        printf(
            "    empty_traps_between_columns = %d \n",
            roe->empty_traps_between_columns);
        printf(
            "    empty_traps_for_first_transfers = %d \n",
            roe->empty_traps_for_first_transfers);
        if (roe->n_steps != 1)
            printf(
                "    force_release_away_from_readout = %d \n",
                roe->force_release_away_from_readout);
        if (roe->use_integer_express_matrix)
            printf(
                "    use_integer_express_matrix = %d \n",
                roe->use_integer_express_matrix);
        if (roe->type == roe_type_trap_pumping)
            printf("    n_pumps = %d \n", roe->n_pumps);

        printf("  CCD n_phases = %d \n", ccd->n_phases);
        if (ccd->n_phases != 1) {
            printf("    fraction_of_traps_per_phase = ");
            print_array(ccd->fraction_of_traps_per_phase);
        }
        for (int i_phase = 0; i_phase < ccd->n_phases; i_phase++) {
            printf(
                "    full_well_depth = %g, well_notch_depth = %g, well_fill_power = %g "
                "\n",
                ccd->phases[i_phase].full_well_depth,
                ccd->phases[i_phase].well_notch_depth,
                ccd->phases[i_phase].well_fill_power);
        }

        if (trap_manager_manager.n_traps_ic != 0) {
            printf(
                "  Instant-capture traps n = %d \n", trap_manager_manager.n_traps_ic);
            for (int i_trap = 0; i_trap < trap_manager_manager.n_traps_ic; i_trap++) {
                printf(
                    "    density = %g, release_timescale = %g \n",
                    trap_manager_manager.trap_managers_ic[0].traps[i_trap].density,
                    trap_manager_manager.trap_managers_ic[0]
                        .traps[i_trap]
                        .release_timescale);
                if (trap_manager_manager.trap_managers_ic[0]
                        .traps[i_trap]
                        .fractional_volume_full_exposed != 0.0)
                    printf(
                        "      fractional_volume_none_exposed = %g, "
                        "fractional_volume_full_exposed = %g \n",
                        trap_manager_manager.trap_managers_ic[0]
                            .traps[i_trap]
                            .fractional_volume_none_exposed,
                        trap_manager_manager.trap_managers_ic[0]
                            .traps[i_trap]
                            .fractional_volume_full_exposed);
            }
        }
        if (trap_manager_manager.n_traps_sc != 0) {
            printf("  Slow-capture traps n = %d \n", trap_manager_manager.n_traps_sc);
            for (int i_trap = 0; i_trap < trap_manager_manager.n_traps_sc; i_trap++) {
                printf(
                    "    density = %g, release_timescale = %g, capture_timescale = %g "
                    "\n",
                    trap_manager_manager.trap_managers_sc[0].traps[i_trap].density,
                    trap_manager_manager.trap_managers_sc[0]
                        .traps[i_trap]
                        .release_timescale,
                    trap_manager_manager.trap_managers_sc[0]
                        .traps[i_trap]
                        .capture_timescale);
            }
        }
        if (trap_manager_manager.n_traps_ic_co != 0) {
            printf("  Continuum traps n = %d \n", trap_manager_manager.n_traps_ic_co);
            for (int i_trap = 0; i_trap < trap_manager_manager.n_traps_ic_co;
                 i_trap++) {
                printf(
                    "    density = %g, release_timescale = %g, release_timescale_sigma "
                    "= %g "
                    "\n",
                    trap_manager_manager.trap_managers_ic_co[0].traps[i_trap].density,
                    trap_manager_manager.trap_managers_ic_co[0]
                        .traps[i_trap]
                        .release_timescale,
                    trap_manager_manager.trap_managers_ic_co[0]
                        .traps[i_trap]
                        .release_timescale_sigma);
            }
        }
        if (trap_manager_manager.n_traps_sc_co != 0) {
            printf(
                "  Slow-capture continuum traps n = %d \n",
                trap_manager_manager.n_traps_sc_co);
            for (int i_trap = 0; i_trap < trap_manager_manager.n_traps_sc_co;
                 i_trap++) {
                printf(
                    "    density = %g, release_timescale = %g, release_timescale_sigma "
                    "= %g, "
                    "capture_timescale = %g \n",
                    trap_manager_manager.trap_managers_sc_co[0].traps[i_trap].density,
                    trap_manager_manager.trap_managers_sc_co[0]
                        .traps[i_trap]
                        .release_timescale,
                    trap_manager_manager.trap_managers_sc_co[0]
                        .traps[i_trap]
                        .release_timescale_sigma,
                    trap_manager_manager.trap_managers_sc_co[0]
                        .traps[i_trap]
                        .capture_timescale);
            }
        }
        print_v(2, "\n");
    }

    // Measure wall-clock time taken for the primary loop
    struct timeval wall_time_start;
    struct timeval wall_time_end;
    double wall_time_elapsed;
    gettimeofday(&wall_time_start, nullptr);



/*    
    // Print express matrix
    print_array_2D(roe->express_matrix, roe->n_express_passes);
    for (unsigned int i_column = 0; i_column < n_active_columns; i_column++) {
        column_index = column_start + i_column;
        print_v(0, "express_multiplier \n", express_multiplier);
        for (unsigned int express_index = 0; express_index < roe->n_express_passes;
             express_index++) {
            // Each pixel
            for (unsigned int i_row = 0; i_row < n_active_rows; i_row++) {
                row_index = row_start + i_row;
                express_multiplier =
                    roe->express_matrix[express_index * n_rows + row_index];
                print_v(0, "%g", express_multiplier);

                if (roe->store_trap_states_matrix[express_index * n_rows + row_index]) {
                    trap_manager_manager.store_trap_states();

                    print_v(0, "*");
                }
            }
            print_v(0, "\n");
        }
    }    
*/

    // ========
    // Clock each column of pixels through the column of traps
    // ========
    // Print express matrix
    //print_array_2D(roe->express_matrix, n_active_rows);
    //print_array_2D((int)roe->store_trap_states_matrix, n_active_rows);
    // Loop over:
    //   Columns > Express passes > Rows > Clock-sequence steps > Pixel phases
    #pragma omp parallel for private(column_index, row_index, row_read, row_write, n_free_electrons, \
				     n_electrons_released_and_captured, express_multiplier, roe_step_phase) \
                             firstprivate(trap_manager_manager)
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
                        if (trap_manager_manager.n_traps_ic > 0)
                            n_electrons_released_and_captured +=
                                trap_manager_manager.trap_managers_ic[i_phase]
                                    .n_electrons_released_and_captured(
                                        n_free_electrons +
                                        n_electrons_released_and_captured);
                        if (trap_manager_manager.n_traps_sc > 0)
                            n_electrons_released_and_captured +=
                                trap_manager_manager.trap_managers_sc[i_phase]
                                    .n_electrons_released_and_captured(
                                        n_free_electrons +
                                        n_electrons_released_and_captured);
                        if (trap_manager_manager.n_traps_ic_co > 0)
                            n_electrons_released_and_captured +=
                                trap_manager_manager.trap_managers_ic_co[i_phase]
                                    .n_electrons_released_and_captured(
                                        n_free_electrons +
                                        n_electrons_released_and_captured);
                        if (trap_manager_manager.n_traps_sc_co > 0)
                            n_electrons_released_and_captured +=
                                trap_manager_manager.trap_managers_sc_co[i_phase]
                                    .n_electrons_released_and_captured(
                                        n_free_electrons +
                                        n_electrons_released_and_captured);
                      
                        print_v(
                            2, "n_electrons_released_and_captured  %g \n",
                            n_electrons_released_and_captured);

                        print_v(
                           2, "n_trapped_electrons_from_watermarks  %g \n",
                            trap_manager_manager.trap_managers_ic[i_phase].n_trapped_electrons_from_watermarks(trap_manager_manager.trap_managers_ic[i_phase].watermark_volumes,trap_manager_manager.trap_managers_ic[i_phase].watermark_fills));

                        print_v(2, "n_free_electrons  %g \n", n_free_electrons);


                        // Return the charge to the relevant pixel(s)
                        for (int i = 0; i < roe_step_phase->n_release_pixels; i++) {
                            row_write =
                                row_index + roe_step_phase->release_to_which_pixels[i];

                            image[row_write][column_index] +=
                                n_electrons_released_and_captured * express_multiplier *
                                roe_step_phase->release_fraction_to_pixels[i];

                            // Make sure image counts don't go negative, which
                            // could happen with a too-large express multiplier
                            if (!allow_negative_pixels) {
                                if (image[row_write][column_index] < 0.0)
                                    image[row_write][column_index] = 0.0;
                            }
                            
                            print_v(2, "row_write  %d \n", row_write);
                            print_v(
                                2, "image[%d][%d]  %g \n", row_write, column_index,
                                image[row_write][column_index]);
                        }
                    }
                }

                // Absorb really small watermarks  into others, for speed
                if (prune_frequency > 0) {
                    if (((i_row + 1) % prune_frequency) == 0) {
                        trap_manager_manager.prune_watermarks(prune_n_electrons);
                    }
                }
                
                // Store the trap states if needed for the next express pass
                if (roe->store_trap_states_matrix[express_index * n_rows + row_index]) {
                    print_v(2, "store_trap_states \n");
                    trap_manager_manager.store_trap_states();
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
    print_v(1, "Wall-clock time elapsed: %.4g s \n", wall_time_elapsed);

    return image;
}

/*
    Add CTI trails to an image by trapping, releasing, and moving electrons
    along their independent columns, for parallel and/or serial clocking.

    Parameters
    ----------
    image_in : std::valarray<std::valarray<double> >
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

    parallel_traps_ic : std::valarray<TrapInstantCapture>* (opt.)
    parallel_traps_sc : std::valarray<TrapSlowCapture>* (opt.)
    parallel_traps_ic_co : std::valarray<TrapInstantCaptureContinuum>* (opt.)
    parallel_traps_sc_co : std::valarray<TrapSlowCaptureContinuum>* (opt.)
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
        
    allow_negative_pixels : bool (opt.)
        Allows pixel values to go below zero (or the lowest in the input image,
        whichever is lower). This ensures absence of bias. However, if you know
        the image must be positive definite, forcing allow_negative_pixels=false
        will catch numerical errors during CTI addition, and can speed up the
        iteration during CTI removal.

    iteration : int (opt.)
        The interation when being called by remove_cti(), default 0 otherwise.
        Only used to control printing.

    Returns
    -------
    image : std::valarray<std::valarray<double> >
        The output array of pixel values with CTI added.
*/
std::valarray<std::valarray<double> > add_cti(
    std::valarray<std::valarray<double> >& image_in,
    // Parallel
    ROE* parallel_roe, CCD* parallel_ccd,
    std::valarray<TrapInstantCapture>* parallel_traps_ic,
    std::valarray<TrapSlowCapture>* parallel_traps_sc,
    std::valarray<TrapInstantCaptureContinuum>* parallel_traps_ic_co,
    std::valarray<TrapSlowCaptureContinuum>* parallel_traps_sc_co, 
    int parallel_express, int parallel_offset, 
    int parallel_window_start, int parallel_window_stop,
    int parallel_time_start, int parallel_time_stop,
    double parallel_prune_n_electrons, int parallel_prune_frequency,
    // Serial
    ROE* serial_roe, CCD* serial_ccd,
    std::valarray<TrapInstantCapture>* serial_traps_ic,
    std::valarray<TrapSlowCapture>* serial_traps_sc,
    std::valarray<TrapInstantCaptureContinuum>* serial_traps_ic_co,
    std::valarray<TrapSlowCaptureContinuum>* serial_traps_sc_co, 
    int serial_express, int serial_offset, 
    int serial_window_start, int serial_window_stop, 
    int serial_time_start, int serial_time_stop,
    double serial_prune_n_electrons, int serial_prune_frequency,
    // Combined
    int allow_negative_pixels, 
    // Output
    int verbosity, int iteration) {
    
 
    // Print unless being called by remove_cti()
    if (!iteration) print_version();
    
    // Don't print model inputs every iteration
    int print_inputs = (iteration > 1) ? 0 : verbosity >= 1;

    // Initialise the output image as a copy of the input image
    std::valarray<std::valarray<double> > image = image_in;

    // Parallel clocking along columns, transfer charge towards row 0
    if (parallel_traps_ic || parallel_traps_sc || parallel_traps_ic_co ||
        parallel_traps_sc_co) {
        print_v(1, "Parallel: ");
        image = clock_charge_in_one_direction(
            image, parallel_roe, parallel_ccd, parallel_traps_ic, parallel_traps_sc,
            parallel_traps_ic_co, parallel_traps_sc_co, 
            parallel_express, parallel_offset, 
            parallel_window_start, parallel_window_stop,
            serial_window_start, serial_window_stop, 
            parallel_time_start, parallel_time_stop,
            parallel_prune_n_electrons, parallel_prune_frequency,
            allow_negative_pixels, print_inputs);
    }

    // Serial clocking along rows, transfer charge towards column 0
    if (serial_traps_ic || serial_traps_sc || serial_traps_ic_co || 
        serial_traps_sc_co) {

        print_v(1, "Serial: ");
        image = transpose(image);
        image = clock_charge_in_one_direction(
            image, serial_roe, serial_ccd, serial_traps_ic, serial_traps_sc,
            serial_traps_ic_co, serial_traps_sc_co, 
            serial_express, serial_offset,
            serial_window_start, serial_window_stop, 
            parallel_window_start, parallel_window_stop, 
            serial_time_start, serial_time_stop,
            serial_prune_n_electrons, serial_prune_frequency,
            allow_negative_pixels, print_inputs);

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
    image : std::valarray<std::valarray<double> >
        The output array of pixel values with CTI removed.
*/
std::valarray<std::valarray<double> > remove_cti(
    std::valarray<std::valarray<double> >& image_in, 
    int n_iterations,
    // Parallel
    ROE* parallel_roe, CCD* parallel_ccd,
    std::valarray<TrapInstantCapture>* parallel_traps_ic,
    std::valarray<TrapSlowCapture>* parallel_traps_sc,
    std::valarray<TrapInstantCaptureContinuum>* parallel_traps_ic_co,
    std::valarray<TrapSlowCaptureContinuum>* parallel_traps_sc_co, 
    int parallel_express, int parallel_offset, 
    int parallel_window_start, int parallel_window_stop,
    int parallel_time_start, int parallel_time_stop,
    double parallel_prune_n_electrons, int parallel_prune_frequency,
    // Serial
    ROE* serial_roe, CCD* serial_ccd,
    std::valarray<TrapInstantCapture>* serial_traps_ic,
    std::valarray<TrapSlowCapture>* serial_traps_sc,
    std::valarray<TrapInstantCaptureContinuum>* serial_traps_ic_co,
    std::valarray<TrapSlowCaptureContinuum>* serial_traps_sc_co, 
    int serial_express, int serial_offset, 
    int serial_window_start, int serial_window_stop,
    int serial_time_start, int serial_time_stop,
    double serial_prune_n_electrons, int serial_prune_frequency,
    // Combined
    int allow_negative_pixels) {

    print_version();

    // Initialise the output image as a copy of the input image
    std::valarray<std::valarray<double> > image_remove_cti = image_in;
    std::valarray<std::valarray<double> > image_add_cti;

    int n_rows = image_in.size();

    // Estimate the image with removed CTI more accurately each iteration
    for (int iteration = 1; iteration <= n_iterations; iteration++) {
        print_v(1, "Iter %d: ", iteration);

        // Model the effect of adding CTI trails
        image_add_cti = add_cti(
            image_remove_cti, parallel_roe, parallel_ccd, parallel_traps_ic,
            parallel_traps_sc, parallel_traps_ic_co, parallel_traps_sc_co,
            parallel_express, parallel_offset, 
            parallel_window_start, parallel_window_stop, 
            parallel_time_start, parallel_time_stop,
            parallel_prune_n_electrons, parallel_prune_frequency,
            serial_roe, serial_ccd, serial_traps_ic,
            serial_traps_sc, serial_traps_ic_co, serial_traps_sc_co, serial_express,
            serial_offset, serial_window_start, serial_window_stop, 
            serial_time_start, serial_time_stop, 
            serial_prune_n_electrons, serial_prune_frequency,
            allow_negative_pixels, 0, iteration);

        // Improve the estimate of the image with CTI trails removed
        image_remove_cti += image_in - image_add_cti;

        // Prevent negative image values
        if (!allow_negative_pixels) {
            for (int row_index = 0; row_index < n_rows; row_index++) {
                image_remove_cti[row_index][image_remove_cti[row_index] < 0.0] = 0.0;
            }
        }
    }

    return image_remove_cti;
}

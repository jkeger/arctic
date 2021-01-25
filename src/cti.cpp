
#include <stdio.h>
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

    Parameters
    ----------
    image : std::valarray<std::valarray<double>>
        The input array of pixel values, assumed to be in units of electrons.

        The first dimension is the "row" index, the second is the "column"
        index. By default (for parallel clocking), charge is transferred "up"
        from row n to row 0 along each independent column. i.e. the readout
        register is above row 0. (For serial clocking, the image is rotated
        beforehand, outside of this function, see add_cti().)

        e.g. (with arbitrary trap parameters)
        Initial image with one bright pixel in the first three columns:
            [[0.0,     0.0,     0.0,     0.0  ],
             [200.0,   0.0,     0.0,     0.0  ],
             [0.0,     200.0,   0.0,     0.0  ],
             [0.0,     0.0,     200.0,   0.0  ],
             [0.0,     0.0,     0.0,     0.0  ],
             [0.0,     0.0,     0.0,     0.0  ]]
        Final image with CTI trails behind each bright pixel:
            [[0.0,     0.0,     0.0,     0.0  ],
             [196.0,   0.0,     0.0,     0.0  ],
             [3.0,     194.1,   0.0,     0.0  ],
             [2.0,     3.9,     192.1,   0.0  ],
             [1.3,     2.5,     4.8,     0.0  ],
             [0.8,     1.5,     2.9,     0.0  ]]
        
    roe : ROE
        An object describing the timing and direction(s) in which electrons are
        moved during readout.
        
    ccd : CCD
        An object to describe how electrons fill the volume inside (each phase
        of) a pixel in a CCD detector.
        
    traps : std::valarray<Trap>
        A list of one or more trap species.
        
    express : int (opt.)
        The number of times the pixel-to-pixel transfers are computed,
        determining the balance between accuracy (high values) and speed
        (low values) (Massey et al. 2014, section 2.1.5).
            n_rows  (slower, accurate) Compute every pixel-to-pixel
                    transfer. The default, 0, is an alias for n_rows.
            k       Recompute on k occasions the effect of each transfer.
                    After a few transfers (and e.g. eroded leading edges),
                    the incremental effect of subsequent transfers can change.
            1       (faster, approximate) Compute the effect of each
                    transfer only once.
        Runtime scales approximately as O(express^0.5). ###WIP
        
    offset : int (opt.)
        The number of (e.g. prescan) pixels separating the supplied image from
        the readout register. Defaults to 0.
        
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

    // Clock one column of pixels through the (column of) traps
    for (int column_index = column_start; column_index < column_stop; column_index++) {

        // Monitor the traps in every pixel (express=n_rows), or just one
        // (express=1) or a few (express=a few) then replicate their effect
        for (int express_index = 0; express_index < roe.n_express_passes;
             express_index++) {

            // Restore the trap occupancy levels (either to empty or to a saved
            // state from a previous express pass)
            trap_manager.restore_trap_states();

            // Each pixel
            for (int row_index = row_start; row_index < row_stop; row_index++) {

                express_multiplier =
                    roe.express_matrix[express_index * n_rows + row_index];
                if (express_multiplier == 0) continue;

                n_free_electrons = image[row_index][column_index];

                n_electrons_released_and_captured =
                    trap_manager.n_electrons_released_and_captured(n_free_electrons);

                image[row_index][column_index] +=
                    n_electrons_released_and_captured * express_multiplier;

                // Store the trap states if needed for the next express pass
                if (roe.store_trap_states_matrix[express_index * n_rows + row_index])
                    trap_manager.store_trap_states();
            }
        }

        // Reset the trap states to empty and/or store them for the next column
        if (roe.empty_traps_between_columns) trap_manager.reset_trap_states();
        trap_manager.store_trap_states();
    }

    // print_array_2D(image);

    return image;
}

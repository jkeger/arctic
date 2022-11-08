
#include "roe.hpp"

#include <math.h>
#include <stdio.h>

#include <valarray>

#include <iostream>
#include "util.hpp"

// ========
// ROEStepPhase::
// ========
/*
    Class ROEStepPhase.

    Information about the readout electronics at one step in a clocking sequence
    at one phase in the pixel. Used to determine the pixels and phases in
    which electrons can be held, captured from, and released to, at this point
    in the sequence.

    Parameters
    ----------
    is_high : bool
        Whether or not the potential is currently held "high", i.e. able to
        contain free electrons.

    capture_from_which_pixels : std::valarray<int>
        The relative row number of the pixel to capture from.

    release_to_which_pixels : std::valarray<int>
        The relative row number of the pixel to release to.

    release_fraction_to_pixels : std::valarray<double>
        The fraction of the electrons to be released into this pixel.

    Attributes
    ----------
    n_capture_pixels, n_release_pixels : int
        The lengths of the *_which_pixels arrays.
*/
ROEStepPhase::ROEStepPhase(
    bool is_high, std::valarray<int> capture_from_which_pixels,
    std::valarray<int> release_to_which_pixels,
    std::valarray<double> release_fraction_to_pixels)
    : is_high(is_high),
      capture_from_which_pixels(capture_from_which_pixels),
      release_to_which_pixels(release_to_which_pixels),
      release_fraction_to_pixels(release_fraction_to_pixels) {

    n_capture_pixels = capture_from_which_pixels.size();
    n_release_pixels = release_to_which_pixels.size();
}

// ========
// ROE::
// ========
/*
    Class ROE.

    Information about the readout electronics.

    For the standard mode, charge is read out from the pixels in which they
    start to the readout register, so are transferred across a different number
    of pixels depending on their initial distance from readout.

    Each transfer from pixel to pixel may be a single step or decomposed into
    multiple steps and multiple pixel phases, set by the clocking sequence.
    Note that unlike the pixels, where pixel 0 is closest to readout and charge
    moves from pixel p to p-1, for phases with a pixel, phase 0 is furthest from
    readout and charge moves from phase i to i+1, as illustrated by the diagrams
    for set_clock_sequence().

    Parameters
    ----------
    dwell_times : std::valarray<double> (opt.)
        The time between steps in the clocking sequence, in the same units
        as the trap capture/release timescales. Default {1.0}.
        
    prescan_offset : int (opt.)
        The number of pixels not present in the input array, through which
        the charge had to pass in order to reach the readout amplifier.
        Default [0]

    overscan_start : int (opt.)
        The number of pixels after which the input array that correspond to a 
        virtual overscan, rather than physical pixels. Charge does not get  
        trailed when it passes through these pixels (but it does through the   
        rest of the CCD). This means that the input array can be larger than 
        the CCD. Default [-1] means "no overscan".

    empty_traps_between_columns : bool (opt.)
        true:  Each column has independent traps (appropriate for parallel
               clocking)
        false: Each column moves through the same traps, which therefore
               preserve occupancy, allowing trails to extend onto the next
               column (appropriate for serial clocking, if all prescan and
               overscan pixels are included in the image array). Default true.

    empty_traps_for_first_transfers : bool (opt.)
        If true, then tweak the express algorithm to treat every first 
        pixel-to-pixel transfer separately to the rest. This is a bit slower,
        but helps to conserve charge is the image size is small enough to be 
        comparable to trail lengths.
        Default false.

        Physically, the first pixel that a charge cloud finds itself in will
        start with empty traps, whereas every subsequent transfer sees traps
        that may have been filled previously. With the default express
        algorithm, this means the inherently different first-transfer would
        be replicated many times for some pixels but not others. This
        modification prevents that issue by modelling the first single
        transfer for each pixel separately and then using the express
        algorithm normally for the remainder, at the cost of modelling more
        transfers.

    force_release_away_from_readout : bool (opt.)
        If true then force electrons to be released in a pixel not closer to
        the readout. See set_clock_sequence() for more context. Default true.

    use_integer_express_matrix : bool (opt.)
        Old versions of this algorithm assumed (unnecessarily) that all
        express multipliers must be integers. It can be slightly more efficient
        if this requirement is dropped, but the option to force it is included
        for backwards compatability. Default false.

    Attributes
    ----------
    type : ROEType : enum int
        The flag for the type of ROE. Used to tweak behaviour in CTI functions.

    n_steps : int
        The number of steps in the clocking sequence.

    n_phases : int
        The number of phases in each pixel. Defaults to n_steps, but may be
        different for a non-standard type of clock sequence, e.g. trap pumping.
*/
ROE::ROE(
    std::valarray<double>& dwell_times, 
    int prescan_offset,
    int overscan_start,
    bool empty_traps_between_columns,
    bool empty_traps_for_first_transfers, 
    bool force_release_away_from_readout,
    bool use_integer_express_matrix)
    : dwell_times(dwell_times),
      prescan_offset(prescan_offset),
      overscan_start(overscan_start),
      empty_traps_between_columns(empty_traps_between_columns),
      empty_traps_for_first_transfers(empty_traps_for_first_transfers),
      force_release_away_from_readout(force_release_away_from_readout),
      use_integer_express_matrix(use_integer_express_matrix) {

    type = roe_type_standard;
    if ( prescan_offset < 0 ) throw std::invalid_argument( "prescan_offset must be zero or positive" );
    if ( overscan_start != -1) {
        if ( overscan_start <= 0 ) throw std::invalid_argument( "overscan_start must be positive" );
    }
    n_steps = dwell_times.size();
    n_phases = n_steps;
}

/* Assignment operator. */
ROE& ROE::operator=(const ROE& roe) {
    dwell_times = roe.dwell_times;
    prescan_offset = roe.prescan_offset;
    overscan_start = roe.overscan_start;
    empty_traps_between_columns = roe.empty_traps_between_columns;
    empty_traps_for_first_transfers = roe.empty_traps_for_first_transfers;
    force_release_away_from_readout = roe.force_release_away_from_readout;
    use_integer_express_matrix = roe.use_integer_express_matrix;
    type = roe.type;
    n_steps = roe.n_steps;
    n_phases = roe.n_phases;
    return *this;
}

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
    n_rows : int
        The number of rows in each column of the image. i.e. the number of
        pixel-to-pixel transfers to move charge from the furthest pixel to the
        readout register (if there is no offset).

    express : int
        The number of times the pixel-to-pixel transfers are computed for each
        pixel, determining the balance between accuracy (high values) and speed
        (low values).
            n_rows      (slower, accurate) Compute every pixel-to-pixel
                        transfer. The default 0 is an alias for n_rows.
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
void ROE::set_express_matrix_from_rows_and_express(
    int n_rows, int express, int window_offset) {

    // Set defaults
    int offset = window_offset + prescan_offset;
    int n_transfers = n_rows + offset; // transfers taken by farthest included pixel 
    int overscan_in_image = 0; // number of pixels in the supplied image that are overscan
    if (overscan_start >= 0)
        overscan_in_image = std::max(n_rows + window_offset + 1 - overscan_start, 0);
    if (express == 0)
        express = n_transfers; // default express to all transfers, and check no larger
    else
        express = std::min(express, n_transfers);

    /*print_v(
        0,"offset: %d %d %d, n_transfers: %d, overscan: %d %d \n", 
        offset, window_offset, prescan_offset, n_transfers, overscan_in_image, overscan_start
    );*/ 
    
 
    // Set default express to all transfers, and check no larger

    // Temporarily ignore the first pixel-to-pixel transfer, if it is to be
    // handled differently than the rest
    if ((empty_traps_for_first_transfers) && (express < n_rows)) n_transfers--;

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
    double max_multiplier = (double) n_transfers / express;
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
    if ((empty_traps_for_first_transfers) && (express >= n_transfers)) {
        // Reverse order of operations, so that first transfer always sees empty traps
        std::valarray<double> express_matrix_full = tmp_express_matrix;
        for (int i = 0; i < express; i++) {
            tmp_express_matrix[std::slice(
                (express - i - 1) * n_transfers, n_transfers, 1)] =
                express_matrix_full[std::slice(
                i * n_transfers, n_transfers, 1)];
        }
        n_express_passes = express;
        
    } else if ((empty_traps_for_first_transfers) && (express < n_transfers)) {
        // Create a new matrix for the full number of transfers
        // (this will eventually overwrite tmp_express_matrix, but that is first used)
        n_transfers++;
        std::valarray<double> express_matrix_full(0.0, n_transfers * n_transfers);
        int i_transfer;
        for (int express_index = 0; express_index < n_transfers; express_index++) {
            i_transfer = n_transfers - express_index - 1;
            express_matrix_full[express_index * n_transfers + i_transfer] = 1;
        }

        // Insert the original transfers into the new matrix at appropriate places
        int new_index;
        for (int old_index = 0; old_index < express; old_index++) {
            // Count the number of non-zero transfers in each original row
            tmp_row = tmp_express_matrix[std::slice(
                old_index * (n_transfers - 1), n_transfers - 1, 1)];
            tmp_row[tmp_row > 0.0] = 1.0;
            new_index = (int)tmp_row.sum();

            // Insert the original transfers
            // (Using slice here doesn't compile on mac for some odd reason...)
            for (int i = 0; i < n_transfers - 1; i++)
                express_matrix_full[new_index * n_transfers + 1 + i] +=
                    tmp_express_matrix[old_index * (n_transfers - 1) + i];
        }

        tmp_express_matrix = express_matrix_full;
        n_express_passes = n_transfers;
    } else {
        n_express_passes = express;
    }
    express_matrix = tmp_express_matrix;
    
    // Remove the offset (which is not represented in the image pixels)
    // std::cout << offset ;
    if (offset > 0) {
        //print_array_2D(tmp_express_matrix, n_transfers);
        std::valarray<double> express_matrix_trim(0.0, n_express_passes * n_rows);

        // Copy the post-offset slices of each row
        for (int express_index = 0; express_index < n_express_passes; express_index++) {
            express_matrix_trim[std::slice(express_index * n_rows, n_rows, 1)] =
                express_matrix[std::slice(
                    express_index * n_transfers + offset, n_rows, 1)];
        }
        //print_array_2D(express_matrix_trim, n_rows);
        express_matrix = express_matrix_trim;
    }

    // Truncate number of transfers in regions of the image that represent overscan
    //print_array_2D(express_matrix, n_rows);  
    //std::cout << "overscan " << overscan_start << " f " << overscan_in_image << "\n";
    if (overscan_in_image > 0) {
        int n_express_rows = express_matrix.size() / n_rows;
        for (int i_row = 0; (i_row < overscan_in_image); i_row++) {
            double to_remove = overscan_in_image - i_row;
            double removed = 0;
            int i_express = 0;
            while (removed < to_remove) {
                int index = (n_express_rows - i_express) * n_rows - i_row - 1;
                if ( index < 0 ) throw std::invalid_argument( "Accessing pixel that does not exist" );
                //std::cout << i_express << i_row << " " << to_remove << removed << " index " << index << "\n";
                removed += express_matrix[index];
                express_matrix[index] = fmax(removed - to_remove, 0);
                i_express++;
            }
            
        }
    }
    //print_array_2D(express_matrix, n_rows);
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

    Sets
    ----
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

/*
    Set the clock sequence 2D array of ROEStepPhase objects for each clocking
    step and phase.

    Sets
    ----
    clock_sequence : std::valarray<std::valarray<ROEStepPhase> >
        The array of ROEStepPhase objects to describe the state of the readout
        electronics in each phase of the pixel at each step in the clocking
        sequence.

    The first diagram below illustrates the steps in the standard sequence
    (where the number of steps equals the number of phases) for three phases,
    where a single phase each step has its potential held high to hold the
    charge cloud. The cloud is shifted from phase 0 to phase N towards the
    previous pixel and the readout register.

    See ROETrapPumping::ROETrapPumping()'s docstring for the behaviour produced
    by this function when the number of steps is double the number of phases,
    to make an oscillating sequence for trap pumping.

    The trap species in each phase of pixel p can capture electrons when that
    phase's potential is high and a charge cloud is present. The "Capture from"
    lines refer to the original pixel that the cloud was in. In this mode, this
    is always the current pixel, but can be different for e.g. trap pumping.

    When the traps release charge, it is assumed to move directly to the nearest
    high potential, which may be in a different pixel. The "Release to" lines
    show the pixel to which electrons released by that phase's traps will move.

    Three phases
    ============
                #     Pixel p-1      #       Pixel p      #     Pixel p+1      #
    Step         Phase2 Phase1 Phase0 Phase2 Phase1 Phase0 Phase2 Phase1 Phase0
    0           +             +------+             +------+             +------+
    Capture from|             |      |             |   p  |             |      |
    Release to  |             |      |  p-1     p  |   p  |             |      |
                +-------------+      +-------------+      +-------------+      +
    1                  +------+             +------+             +------+
    Capture from       |      |             |   p  |             |      |
    Release to         |      |          p  |   p  |   p         |      |
                -------+      +-------------+      +-------------+      +-------
    2           +------+             +------+             +------+             +
    Capture from|      |             |   p  |             |      |             |
    Release to  |      |             |   p  |   p     p+1 |      |             |
                +      +-------------+      +-------------+      +-------------+

    Below are corresponding illustrations for one, two, and four phases. For an
    even number of phases, one phase in each step will be equidistant from two
    high potentials. So any released charge is assumed to split equally between
    the two pixels, as indicated by the "Release to" lines.

    One phase
    =========
                  Pixel p-1  Pixel p  Pixel p+1
    Step            Phase0   Phase0   Phase0
    0              +------+ +------+ +------+
    Capture from   |      | |   p  | |      |
    Release to     |      | |   p  | |      |
                  -+      +-+      +-+      +-

    Two phases
    ==========
                  #  Pixel p-1  #   Pixel p   #  Pixel p+1  #
    Step           Phase1 Phase0 Phase1 Phase0 Phase1 Phase0
    0             +      +------+      +------+      +------+
    Capture from  |      |      |      |   p  |      |      |
    Release to    |      |      | p-1&p|   p  |      |      |
                  +------+      +------+      +------+      +
    1             +------+      +------+      +------+      +
    Capture from  |      |      |   p  |      |      |      |
    Release to    |      |      |   p  | p&p+1|      |      |
                  +      +------+      +------+      +------+

    Four phases
    ===========
               Pixel p-1      #          Pixel p          #         Pixel p+1
    Step         Phase1 Phase0 Phase3 Phase2 Phase1 Phase0 Phase3 Phase2 Phase1
    0                  +------+                    +------+                    +
    Capture from       |      |                    |   p  |                    |
    Release to         |      |  p-1   p-1&p    p  |   p  |                    |
                -------+      +--------------------+      +--------------------+
    1           +------+                    +------+                    +------+
    Capture from|      |                    |   p  |                    |      |
    Release to  |      |        p-1&p    p  |   p  |   p                |      |
                +      +--------------------+      +--------------------+      +
    2           +                    +------+                    +------+
    Capture from|                    |   p  |                    |      |
    Release to  |                p   |   p  |   p    p&p+1       |      |
                +--------------------+      +--------------------+      +-------
    3                         +------+                    +------+
    Capture from              |   p  |                    |      |
    Release to                |   p  |   p    p&p+1  p+1  |      |
                --------------+      +--------------------+      +--------------

    ## Document force_release_away_from_readout = true
*/
void ROE::set_clock_sequence() {

    bool is_high;
    std::valarray<int> capture_from_which_pixels;
    std::valarray<int> release_to_which_pixels;
    std::valarray<int> zero_one = {0, 1};
    std::valarray<double> release_fraction_to_pixels;
    int i_step_loop;
    int i_phase_high;
    int i_phase_split_release;

    clock_sequence.resize(n_steps);

    // Set the ROEStepPhase objects for each step in the sequence
    for (int i_step = 0; i_step < n_steps; i_step++) {
        clock_sequence[i_step].resize(n_phases);

        // Convert e.g. 0,1,2,3,4,5 to 0,1,2,3,2,1 used for trap pumping, has
        // no effect with standard n_phases = n_steps
        i_step_loop = abs((i_step + n_phases) % (2 * n_phases) - n_phases);

        // Index of the high phase in this step
        i_phase_high = i_step_loop % n_phases;

        // Index of the phase (if any) in this step for which released charge
        // will split and move into two pixels because the nearest high phases
        // are equidistant in both directions
        if (n_phases % 2 == 0)
            i_phase_split_release = (i_phase_high + n_phases / 2) % n_phases;
        else
            i_phase_split_release = -1;

        // Each phase in this step
        for (int i_phase = 0; i_phase < n_phases; i_phase++) {
            // Is this phase high?
            if (i_phase == i_phase_high)
                is_high = true;
            else
                is_high = false;

            // The pixel to capture from, if any
            if (is_high)
                // Capture from this pixel's charge cloud unless the previous
                // pixel's charge cloud has been shifted into this pixel
                if (i_step_loop > n_phases - 1)
                    capture_from_which_pixels = {1};
                else
                    capture_from_which_pixels = {0};
            else
                capture_from_which_pixels = {};

            // The pixel(s) to release to
            if (i_phase == i_phase_split_release) {
                // Split the release between this and the pixel with the joint-
                // nearest high phase
                if (i_phase < i_phase_high)
                    release_to_which_pixels = zero_one;
                else
                    release_to_which_pixels = zero_one - 1;
                release_fraction_to_pixels = {0.5, 0.5};
            } else {
                // For high phases, release to same pixel as capture
                if (is_high)
                    release_to_which_pixels = capture_from_which_pixels;
                else {
                    // Release to the single pixel with the nearest high phase
                    if (i_phase - i_phase_high < -n_phases / 2)
                        release_to_which_pixels = {1};
                    else if (i_phase - i_phase_high > n_phases / 2)
                        release_to_which_pixels = {-1};
                    else
                        release_to_which_pixels = {0};
                }
                release_fraction_to_pixels = {1.0};
            }
            // Release from low phases to the previous pixel's charge cloud if
            // it's been shifted into this pixel
            if ((!is_high) && (i_step_loop > n_phases - 1))
                release_to_which_pixels += 1;

            // Replace capture/release operations that include a closer-to-
            // readout pixel to instead act on the further-from-readout pixel
            // (i.e. the same operation but on the next pixel in the loop)
            //## should this instead just find and change {-1}s to {0}s?
            if ((force_release_away_from_readout) && (i_phase > i_phase_high)) {
                capture_from_which_pixels += 1;
                release_to_which_pixels += 1;
            }

            // Set the info for this step and phase
            clock_sequence[i_step][i_phase] = ROEStepPhase(
                is_high, capture_from_which_pixels, release_to_which_pixels,
                release_fraction_to_pixels);
        }
    }
}

// ========
// ROEChargeInjection::
// ========
/*
    Class ROEChargeInjection.

    Modified ROE for charge injection modes.

    Instead of charge starting in each pixel and moving different distances to
    the readout register, for charge injection the electrons are directly
    created at the far end of the CCD, then are all transferred the same number
    of times through the full image of pixels to the readout register.

    Parameters
    ----------
    Same as ROE, but empty_traps_for_first_transfers is automatically false,
    since only the single leading pixel of the charge injection in each column
    will see the untouched traps.
*/
ROEChargeInjection::ROEChargeInjection(
    std::valarray<double>& dwell_times, 
    int prescan_offset,
    int overscan_start,
    bool empty_traps_between_columns,
    bool force_release_away_from_readout, 
    bool use_integer_express_matrix)
    : ROE(dwell_times, prescan_offset, overscan_start, empty_traps_between_columns, false,
          force_release_away_from_readout, use_integer_express_matrix) {

    type = roe_type_charge_injection;
}

/*
    See ROE::set_express_matrix_from_rows_and_express().

    For charge injection, all charges are clocked the same number of times
    through all the pixels to the readout register.
*/
void ROEChargeInjection::set_express_matrix_from_rows_and_express(
    int n_rows, int express, int window_offset) {
    
/*    
    // Can almost do with (just) the following
    if (overscan_start <= 0)
        overscan_start = n_rows + window_offset + 1;
    int overscan_start_temp = overscan_start;
    prescan_offset += (overscan_start - 1); // Add all pixels into the "prescan"
    overscan_start = 1; // Start overscan immediately after the prescan
    ROE::set_express_matrix_from_rows_and_express(n_rows, express, window_offset);
    overscan_start = overscan_start_temp;
    prescan_offset -= (overscan_start - 1); 
*/    

    // Set defaults
    //int offset = window_offset + prescan_offset;
    int n_transfers = prescan_offset + window_offset + n_rows; // transfers taken by farthest included pixel 
    if (overscan_start >= 0) n_transfers = prescan_offset + overscan_start - 1;
    if (express == 0)
        express = n_transfers; // default express to all transfers, and check no larger
    else
        express = std::min(express, n_transfers);
    n_express_passes = express;

    // Compute the multiplier factors
    double max_multiplier = (double)n_transfers / express;
    if (use_integer_express_matrix) max_multiplier = ceil(max_multiplier);

    // Initialise an array that will become the express matrix (can't create the array itself)
    std::valarray<double> tmp_express_matrix(max_multiplier, express * n_rows);

    // Adjust integer multipliers to correct the total number of transfers
    if ((use_integer_express_matrix) && (n_transfers % express != 0)) {
        double current_n_transfers;
        double reduced_multiplier;

        for (int express_index = express - 1; express_index >= 0; express_index--) {
            // Count the current number of transfers for this pixel
            current_n_transfers = 0.0;
            for (int i = 0; i <= express_index; i++) {
                current_n_transfers += tmp_express_matrix[i * n_rows];
            }

            // Reduce the multipliers until no longer have too many transfers
            if (current_n_transfers <= n_transfers) break;
            reduced_multiplier =
                std::max(0.0, max_multiplier + n_transfers - current_n_transfers);
            tmp_express_matrix[std::slice(
                express_index * n_rows, n_rows, 1)] = reduced_multiplier;
        }
    }
    express_matrix = tmp_express_matrix;

}

/*
    See ROE::set_store_trap_states_matrix().

    For charge injection, the first charge cloud in each column will always
    encounter empty traps in every new pixel. So no need to store trap states
    between transfers.
*/
void ROEChargeInjection::set_store_trap_states_matrix() {
    store_trap_states_matrix = std::valarray<bool>(false, express_matrix.size());
}

// ========
// ROETrapPumping::
// ========
/*
    Class ROETrapPumping.

    Modified ROE for trap pumping (AKA pocket pumping) modes.

    Instead of clocking the charge in all pixels towards the readout register,
    trap pumping shifts the charge back and forth, to end up in the same place
    they began. If one pixel (and phase) contains traps and its neighbours do
    not, then a charge dipole can be created as charge is captured and released
    asymmetrically in the active and adjacent pixel. This allows direct study of
    the trap species in a CCD.

    The number of phases is assumed to be half the number of steps in the clock
    sequence, which must be even.

    Currently, this algorithm assumes that only a single pixel is active and
    contains traps.

    Parameters
    ----------
    Same as ROE, but empty_traps_between_columns is automatically true, and
    force_release_away_from_readout is automatically false.

    n_pumps : int
        The number of times the charge is pumped back and forth.

    The diagram below illustrates the steps in the clocking sequence produced
    by ROE::set_clock_sequence() in this mode, for three phases. The first three
    steps are the same as the standard case. However, now there are three
    additional steps in which the charge cloud is shifted one step further into
    the next pixel, but then back to where it began in the original pixel,
    instead of continuing on towards the readout register.

    This means that, unlike in the standard case, the traps in this pixel can
    capture charge that originated in a different pixel, as shown in step 3,
    where the high phase in pixel p contains the charge cloud that started in
    pixel p+1.

    Three phases
    ============
                #     Pixel p-1      #       Pixel p      #     Pixel p+1      #
    Step         Phase2 Phase1 Phase0 Phase2 Phase1 Phase0 Phase2 Phase1 Phase0
    0           +             +------+             +------+             +------+
    Capture from|             |      |             |   p  |             |      |
    Release to  |             |      |  p-1     p  |   p  |             |      |
                +-------------+      +-------------+      +-------------+      +
    1                  +------+             +------+             +------+
    Capture from       |      |             |   p  |             |      |
    Release to         |      |          p  |   p  |   p         |      |
                -------+      +-------------+      +-------------+      +-------
    2           +------+             +------+             +------+             +
    Capture from|      |             |   p  |             |      |             |
    Release to  |      |             |   p  |   p     p+1 |      |             |
                +      +-------------+      +-------------+      +-------------+
    3           +             +------+             +------+             +------+
    Capture from|             |      |             |  p+1 |             |      |
    Release to  |             |      |   p     p+1 |  p+1 |             |      |
                +-------------+      +-------------+      +-------------+      |
    4           +------+             +------+             +------+             +
    Capture from|      |             |   p  |             |      |             |
    Release to  |      |             |   p  |   p     p+1 |      |             |
                +      +-------------+      +-------------+      +-------------+
    5                  +------+             +------+             +------+
    Capture from       |      |             |   p  |             |      |
    Release to         |      |          p  |   p  |   p         |      |
                -------+      +-------------+      +-------------+      +-------

    Below are corresponding illustrations for two and four phases. As in the
    standard case, an even number of phases leads to released charge being split
    in one phase each step between twp pixels.

    Two phases
    ==========
                  #  Pixel p-1  #   Pixel p   #  Pixel p+1  #
    Step           Phase1 Phase0 Phase1 Phase0 Phase1 Phase0
    0             +      +------+      +------+      +------+
    Capture from  |      |      |      |   p  |      |      |
    Release to    |      |      | p-1&p|   p  |      |      |
                  +------+      +------+      +------+      +
    1             +------+      +------+      +------+      +
    Capture from  |      |      |   p  |      |      |      |
    Release to    |      |      |   p  | p&p+1|      |      |
                  +      +------+      +------+      +------+
    2             +      +------+      +------+      +------+
    Capture from  |      |      |      |  p+1 |      |      |
    Release to    |      |      | p&p+1|  p+1 |      |      |
                  +------+      +------+      +------+      +
    3             +------+      +------+      +------+      +
    Capture from  |      |      |   p  |      |      |      |
    Release to    |      |      |   p  | p&p+1|      |      |
                  +      +------+      +------+      +------+

    Four phases
    ===========
               Pixel p-1      #          Pixel p          #         Pixel p+1
    Step         Phase1 Phase0 Phase3 Phase2 Phase1 Phase0 Phase3 Phase2 Phase1
    0                  +------+                    +------+                    +
    Capture from       |      |                    |   p  |                    |
    Release to         |      |  p-1   p-1&p    p  |   p  |                    |
                -------+      +--------------------+      +--------------------+
    1           +------+                    +------+                    +------+
    Capture from|      |                    |   p  |                    |      |
    Release to  |      |        p-1&p    p  |   p  |   p                |      |
                +      +--------------------+      +--------------------+      +
    2           +                    +------+                    +------+
    Capture from|                    |   p  |                    |      |
    Release to  |                p   |   p  |   p    p&p+1       |      |
                +--------------------+      +--------------------+      +-------
    3                         +------+                    +------+
    Capture from              |   p  |                    |      |
    Release to                |   p  |   p    p&p+1  p+1  |      |
                --------------+      +--------------------+      +--------------
    4                  +------+                    +------+                    +
    Capture from       |      |                    |  p+1 |                    |
    Release to         |      |   p    p&p+1   p+1 |  p+1 |                    |
                -------+      +--------------------+      +--------------------+
    5                         +------+                    +------+
    Capture from              |   p  |                    |      |
    Release to                |   p  |   p    p&p+1  p+1  |      |
                --------------+      +--------------------+      +--------------
    6           +                    +------+                    +------+
    Capture from|                    |   p  |                    |      |
    Release to  |                p   |   p  |   p    p&p+1       |      |
                +--------------------+      +--------------------+      +-------
    7           +------+                    +------+                    +------+
    Capture from|      |                    |   p  |                    |      |
    Release to  |      |        p-1&p    p  |   p  |   p                |      |
                +      +--------------------+      +--------------------+      +
*/
ROETrapPumping::ROETrapPumping(
    std::valarray<double>& dwell_times, int n_pumps,
    bool empty_traps_for_first_transfers, bool use_integer_express_matrix)
    : ROE(dwell_times, 0, -1, true, empty_traps_for_first_transfers, false,
          use_integer_express_matrix) {

    type = roe_type_trap_pumping;
    this->n_pumps = n_pumps;

    if (n_steps % 2 != 0)
        error("The number of steps for trap pumping (%d) must be even", n_steps);
    n_phases = n_steps / 2;
}

/*
    See ROE::set_express_matrix_from_rows_and_express().

    For trap pumping, instead of charge being transferred from pixel to pixel
    until readout, only the back-and-forth pumping clock sequence is repeated a
    number of times for the active pixel(s) containing charge.

    Currently, this algorithm assumes that only a single pixel is active and
    contains traps. So, rather than the number of pixels and offset (which must
    be 1 and 0) the number of express passes is controlled by the number of
    pumps back and forth.

    Conveniently, the required express multipliers are the same as the ones for
    the pixel furthest from readout with standard clocking.
*/
void ROETrapPumping::set_express_matrix_from_rows_and_express(
    int n_rows, int express, int window_offset) {

    // Set default express to all transfers, and check no larger
    if (express == 0)
        express = n_pumps;
    else
        express = std::min(express, n_pumps);

    // Start with the standard express matrix for n_transfers = n_pumps
    ROE::set_express_matrix_from_rows_and_express(n_pumps, express, window_offset);

    // Extract the relevant express multipliers of the final pixel
    std::valarray<double> tmp_col(0.0, n_express_passes);
    for (int express_index = 0; express_index < n_express_passes; express_index++) {
        tmp_col[express_index] = express_matrix[n_pumps - 1 + express_index * n_pumps];
    }

    // Extract the non-zero elements if doing first transfers separately
    if ((empty_traps_for_first_transfers) && (express < n_pumps)) {
        std::valarray<double> tmp_col_2 = tmp_col[tmp_col != 0.0];

        n_express_passes = express + 1;
        tmp_col.resize(n_express_passes);
        // Set the non-zero elements, which won't include the final entry for
        // mismatched integer multipliers
        if ((use_integer_express_matrix) && (n_pumps % express != 0)) {
            // (Using slice here doesn't compile on mac for some odd reason...)
            for (int i = 0; i < n_express_passes - 1; i++) tmp_col[i] = tmp_col_2[i];

            // Put back the final zero
            tmp_col[n_express_passes - 1] = 0.0;
        } else
            tmp_col = tmp_col_2;
    }

    express_matrix.resize(n_rows * n_express_passes);
    // Set multipliers for all rows, even though only one row will be active and
    // actually used
    for (int row_index = 0; row_index < n_rows; row_index++) {
        express_matrix[std::slice(row_index, n_express_passes, n_rows)] = tmp_col;
    }
}

/*
    See ROE::set_store_trap_states_matrix().

    For trap pumping, trap states must be stored after every pump sequence of
    the same trap, until the final pump.
*/
void ROETrapPumping::set_store_trap_states_matrix() {
    // Store after each active pass
    store_trap_states_matrix = std::valarray<bool>(false, express_matrix.size());
    store_trap_states_matrix[express_matrix != 0.0] = true;

    // Don't store the final pass
    int n_rows = express_matrix.size() / n_express_passes;
    store_trap_states_matrix[std::slice((n_express_passes - 1) * n_rows, n_rows, 1)] =
        false;
}

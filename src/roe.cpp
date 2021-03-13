
#include <math.h>
#include <stdio.h>
#include <valarray>

#include "roe.hpp"
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
ROEStepPhase::ROEStepPhase(bool is_high, std::valarray<int> capture_from_which_pixels,
    std::valarray<int> release_to_which_pixels, 
    std::valarray<double> release_fraction_to_pixels)
    : is_high(is_high), capture_from_which_pixels(capture_from_which_pixels),
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

    Parameters
    ----------
    dwell_times : std::valarray<double> (opt.)
        The time between steps in the clocking sequence, in the same units
        as the trap capture/release timescales. Default {1.0}.

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

    empty_traps_between_columns : bool (opt.)
        true:  Each column has independent traps (appropriate for parallel
               clocking)
        false: Each column moves through the same traps, which therefore
               preserve occupancy, allowing trails to extend onto the next
               column (appropriate for serial clocking, if all prescan and
               overscan pixels are included in the image array). Default true.
        
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
    n_steps : int
        The number of steps in the clocking sequence.
        
    n_phases : int
        The number of phases in each pixel. Defaults to n_steps, but may be 
        different for a non-standard type of clock sequence, e.g. trap pumping.
    
    clock_sequence : std::valarray<std::valarray<ROEStepPhase>>
        The array of ROEStepPhase objects to describe the state of the readout 
        electronics at each step in the clocking sequence and each phase of the
        pixel.
*/
ROE::ROE(
    std::valarray<double>& dwell_times, bool empty_traps_between_columns,
    bool empty_traps_for_first_transfers, bool force_release_away_from_readout, 
    bool use_integer_express_matrix)
    : dwell_times(dwell_times),
      empty_traps_for_first_transfers(empty_traps_for_first_transfers),
      empty_traps_between_columns(empty_traps_between_columns),
      force_release_away_from_readout(force_release_away_from_readout),
      use_integer_express_matrix(use_integer_express_matrix) {

    n_steps = dwell_times.size();
    n_phases = n_steps;
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
    clock_sequence : std::valarray<std::valarray<ROEStepPhase>>
        The array of ROEStepPhase objects to describe the state of the readout 
        electronics in each phase of the pixel at each step in the clocking 
        sequence.
    
    The first diagram below illustrates the steps in the standard sequence for 
    three phases, where a single phase each step has its potential held high to 
    hold the charge cloud. The cloud is shifted phase by phase towards the 
    previous pixel and the readout register.
    
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
        else i_phase_split_release = -1;
        
        // Each phase in this step
        for (int i_phase = 0; i_phase < n_phases; i_phase++) {
            // Is this phase high?
            if (i_phase == i_phase_high) is_high = true;
            else is_high = false;
            
            // The pixel to capture from, if any
            if (is_high) capture_from_which_pixels = {0};
            else capture_from_which_pixels = {};
            
            // The pixel(s) to release to
            if (i_phase == i_phase_split_release) {
                // Split the release between this and the pixel with the joint-
                // nearest high phase
                if (i_phase < i_phase_high)
                    release_to_which_pixels = zero_one;
                else 
                    release_to_which_pixels = zero_one - 1;
                release_fraction_to_pixels = {0.5, 0.5};
            }
            else {
                // Release to the single pixel with the nearest high phase
                if (i_phase - i_phase_high < -n_phases / 2)
                    release_to_which_pixels = {1};
                else if (i_phase - i_phase_high > n_phases / 2)
                    release_to_which_pixels = {-1};
                else 
                    release_to_which_pixels = {0};
                release_fraction_to_pixels = {1.0};
            }
            
            // Replace capture/release operations that include a closer-to-
            // readout pixel to instead act on the further-from-readout pixel
            // (i.e. the same operation but on the next pixel in the loop)
            //## should this instead just find and change {-1}s to {0}s?
            if ((force_release_away_from_readout) && (i_phase > i_phase_high)){
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

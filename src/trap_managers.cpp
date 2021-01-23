
#include <math.h>
#include <stdio.h>
#include <valarray>

#include "trap_managers.hpp"
#include "traps.hpp"
#include "util.hpp"

/*
    Class TrapManager.

    The manager for one or multiple trap species that are able to use
    watermarks in the same way as each other.

    Parameters
    ----------
    traps : std::valarray<Trap>
        A list of one or more trap species. Species listed together must be
        able to share watermarks - i.e. they must be similarly distributed
        throughout the pixel volume, and all their states must be stored
        either by occupancy or by time since filling.
        
    max_n_transfers : int
        The number of pixel transfers containing traps that charge will be
        expected to go through. This feeds in to the maximum number of possible
        capture/release events that could create new watermark levels, and is
        used to initialise the watermark array to be only as large as needed.
            
    Attributes
    ----------
    watermarks : std::valarray<double>
        2D-style 1D array of watermark fractional volumes and fill fractions to
        describe the trap states. Lists each (active) watermark fractional
        volume and the corresponding fill fractions of each trap species.
        
        [[volume, fill, fill, ...],
         [volume, fill, fill, ...],
         ...                       ]    (flattened to a 1D array)
         
        Examples of interpreting the array as 2D:
            The element of the ith watermark volume, and jth trap fill:
                watermarks[i * n_col_wmk + 0]
                watermarks[i * n_col_wmk + j + 1]
             
            The volumes column, and the ith trap column:
                watermarks[std::slice(0, n_watermarks, n_col_wmk)]
                watermarks[std::slice(1 + i, n_watermarks, n_col_wmk)]
                
            The ith watermark row:
                watermarks[std::slice(i * n_col_wmk, n_col_wmk, 1)]
        
    n_traps : int
        The number of trap species.
    
    n_watermarks_per_transfer : int
        The number of new watermarks that could be made in each transfer.
    
    empty_watermark, filled_watermark : double
        The watermark values corresponding to empty and filled traps.

    n_watermarks : int
        The total number of available watermarks.
        
    n_col_wmk : int
        The number of columns in the 2D-style watermarks 1D array.
        
    n_active_watermarks : int
        The number of currently active watermark levels.
*/
TrapManager::TrapManager(std::valarray<Trap> traps, int max_n_transfers)
    : traps(traps), max_n_transfers(max_n_transfers) {

    n_traps = traps.size();
    n_watermarks_per_transfer = 2;
    empty_watermark = 0.0;
    filled_watermark = 1.0;
    n_col_wmk = n_traps + 1;
    n_active_watermarks = 0;
}

/*
    Initialise the watermarks array.
    
    Sets
    ----
    n_watermarks : int
        The total number of available watermarks.
    
    watermarks : std::valarray<double>
        The initial empty watermarks array. See TrapManager().
*/
void TrapManager::initialise_watermarks() {
    n_watermarks = max_n_transfers * n_watermarks_per_transfer + 1;

    watermarks = std::valarray<double>(empty_watermark, (1 + n_traps) * n_watermarks);
}

/*
    Set the probabilities of traps being full after release and/or capture.
        
    See Lindegren (1998) section 3.2.
    
    ## Can be extended to 2D arrays for multi-phase clocking
    
    Parameters
    ----------
    dwell_time : double
        The time spent in this pixel or phase, in the same units as the
        trap timescales.
    
    Sets
    ----
    fill_probabilities_from_empty : std::valarray<double>
        The fraction of traps that were empty that become full.
    
    fill_probabilities_from_full : std::valarray<double>
        The fraction of traps that were full that stay full.
    
    fill_probabilities_from_release : std::valarray<double>
        The fraction of traps that were full that stay full after release.
*/
void TrapManager::set_fill_probabilities_from_dwell_time(double dwell_time) {
    double total_rate, exponential_factor;
    fill_probabilities_from_empty = std::valarray<double>(0.0, n_traps);
    fill_probabilities_from_full = std::valarray<double>(0.0, n_traps);
    fill_probabilities_from_release = std::valarray<double>(0.0, n_traps);

    // Set probabilities for each trap species
    for (int i_trap = 0; i_trap < n_traps; i_trap++) {
        // Common factors
        total_rate = traps[i_trap].capture_rate + traps[i_trap].emission_rate;
        exponential_factor = (1 - exp(-total_rate * dwell_time)) / total_rate;

        // Resulting fill fraction for empty traps (Eqn. 20)
        if (traps[i_trap].capture_rate == 0.0)
            // Instant capture
            fill_probabilities_from_empty[i_trap] = 1.0;
        else
            fill_probabilities_from_empty[i_trap] =
                traps[i_trap].capture_rate * exponential_factor;

        // Resulting fill fraction for filled traps (Eqn. 21)
        fill_probabilities_from_full[i_trap] =
            1 - traps[i_trap].emission_rate * exponential_factor;

        // Resulting fill fraction from only release
        fill_probabilities_from_release[i_trap] =
            exp(-traps[i_trap].emission_rate * dwell_time);
    }
}

/*
    Sum the total number of electrons currently held in traps.

    Parameters
    ----------
    watermarks : std::valarray<double>
        An array of watermarks. See TrapManager().
    
    Returns
    -------
    n_trapped_electrons : double
        The number of electrons stored in traps.
*/
double TrapManager::n_trapped_electrons_from_watermarks(std::valarray<double> wmks) {
    double n_trapped_electrons = 0.0;
    std::valarray<double> n_trapped_electrons_each_watermark(0.0, n_watermarks);

    // Each trap species
    for (int i_trap = 0; i_trap < n_traps; i_trap++) {
        // Store the fill fractions in a 1D array
        n_trapped_electrons_each_watermark =
            wmks[std::slice(1 + i_trap, n_watermarks, n_col_wmk)];

        // Multiply the fill fractions by the fractional volumes
        n_trapped_electrons_each_watermark *=
            wmks[std::slice(0, n_watermarks, n_col_wmk)];

        // Sum the number of electrons in each watermark level and multiply by
        // the trap density
        n_trapped_electrons +=
            n_trapped_electrons_each_watermark.sum() * traps[i_trap].density;
    }

    return n_trapped_electrons;
}

/*
    Class TrapManagerInstantCapture.
    
    For the old release-then-instant-capture algorithm.
*/
TrapManagerInstantCapture::TrapManagerInstantCapture(
    std::valarray<Trap> traps, int max_n_transfers)
    : TrapManager(traps, max_n_transfers) {

    n_watermarks_per_transfer = 1;
}

/*
    Release electrons from traps and update the watermarks.

    ## Can be extended to take the phase to choose the right dwell time / fill
        probabilities for multi-phase clocking
        
    Returns
    -------
    n_electrons_released : double
        The number of released electrons.
    
    Updates
    -------
    watermarks : std::valarray<double>
        The updated watermarks. See TrapManager().
*/
double TrapManagerInstantCapture::n_electrons_released() {
    double n_trapped_electrons_initial =
        n_trapped_electrons_from_watermarks(watermarks);

    // Each trap species
    for (int i_trap = 0; i_trap < n_traps; i_trap++) {
        // Each active watermark
        for (int i_wmk = 0; i_wmk < n_active_watermarks; i_wmk++) {
            // Update the watermark fill fraction
            watermarks[i_wmk * n_col_wmk + i_trap + 1] *=
                fill_probabilities_from_release[i_trap];
        }
    }

    double n_trapped_electrons_final = n_trapped_electrons_from_watermarks(watermarks);

    return n_trapped_electrons_initial - n_trapped_electrons_final;
}
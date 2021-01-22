
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
         
        e.g. extract the volumes subarray and the ith trap subarray:
            watermarks[std::slice(0, n_watermarks, n_wmk_col)]
            watermarks[std::slice(1 + i, n_watermarks, n_wmk_col)]
            
        e.g. extract the kth watermark subarray:
            watermarks[std::slice(i * n_wmk_col, n_wmk_col, 1)]
        
    n_traps : int
        The number of trap species.
    
    n_watermarks_per_transfer : int
        The number of new watermarks that could be made in each transfer.
    
    empty_watermark, filled_watermark : double
        The watermark values corresponding to empty and filled traps.

    n_watermarks : int
        The total number of available watermarks.
        
    n_wmk_col : int
        The number of columns in the 2D-style watermarks 1D array.
*/
TrapManager::TrapManager(std::valarray<Trap> traps, int max_n_transfers)
    : traps(traps), max_n_transfers(max_n_transfers) {

    n_traps = traps.size();
    n_watermarks_per_transfer = 2;
    empty_watermark = 0.0;
    filled_watermark = 1.0;
    n_wmk_col = n_traps + 1;
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
            wmks[std::slice(1 + i_trap, n_watermarks, n_wmk_col)];

        // Multiply the fill fractions by the fractional volumes
        n_trapped_electrons_each_watermark *=
            wmks[std::slice(0, n_watermarks, n_wmk_col)];

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

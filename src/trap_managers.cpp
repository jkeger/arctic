
#include <math.h>
#include <stdio.h>
#include <valarray>

#include "trap_managers.hpp"
#include "traps.hpp"

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
    watermarks : std::valarray<std::valarray<double>>
        Array of watermark fractional volumes and fill fractions to describe
        the trap states. Lists each (active) watermark fractional volume and
        the corresponding fill fractions of each trap species. Inactive
        elements are set to 0.

        [[volume, fill, fill, ...],
         [volume, fill, fill, ...],
         ...                       ]
         
    n_traps : int
        The number of trap species.
    
    n_watermarks_per_transfer : int
        The number of possible new watermarks that could be made in each
        transfer.
    
    empty_watermark, filled_watermark : double
        The watermark values corresponding to empty and filled traps.
         
    // n_traps_per_pixel : np.ndarray
    //     The densities of all the trap species.
    //
    // capture_rates, emission_rates, total_rates : np.ndarray
    //     The rates of capture, emission, and their sum for all the traps.
*/
TrapManager::TrapManager(std::valarray<Trap> traps, int max_n_transfers)
    : traps(traps), max_n_transfers(max_n_transfers) {

    n_traps = traps.size();
    n_watermarks_per_transfer = 2;
    empty_watermark = 0.0;
    filled_watermark = 1.0;
}

/*
    Initialise the watermarks array.
*/
void TrapManager::initialise_watermarks() {
    max_n_watermarks = max_n_transfers * n_watermarks_per_transfer + 1;

    watermarks = std::valarray<std::valarray<double>>(
        std::valarray<double>(empty_watermark, 1 + n_traps), max_n_watermarks);
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

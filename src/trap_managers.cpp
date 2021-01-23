
#include <math.h>
#include <stdio.h>
#include <valarray>

#include "trap_managers.hpp"
#include "traps.hpp"
#include "util.hpp"

//
// TrapManager::
//

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
    watermark_volumes : std::valarray<double>
        Array of watermark fractional volumes to describe the trap states, i.e.
        the proportion of the pixel volume occupied by each (active) watermark.
        
    watermark_fills : std::valarray<double>
        2D-style 1D array of watermark fill fractions to describe the trap
        states, i.e. the proportion of traps that are filled in each (active)
        watermark, for each trap species.
        
        Examples of slicing the arrays:
            The ith watermark "row" of the fills:
                watermark_fills[ std::slice(i * n_traps, n_traps, 1) ]
                
            The ith trap species "column" of the fills:
                watermark_fills[ std::slice(i, n_watermarks, n_traps) ]
                
            The ith-to-jth watermark slices of the volumes and (all) the fills:
                watermark_volumes[ std::slice(i, j - i, 1) ]
                watermark_fills[ std::slice(i * n_traps, (j - i) * n_traps, 1) ]
        
    n_traps : int
        The number of trap species.
    
    n_watermarks_per_transfer : int
        The number of new watermarks that could be made in each transfer.
    
    empty_watermark, filled_watermark : double
        The watermark values corresponding to empty and filled traps.

    n_watermarks : int
        The total number of available watermark levels.
        
    n_active_watermarks : int
        The number of currently active watermark levels.
*/
TrapManager::TrapManager(std::valarray<Trap> traps, int max_n_transfers)
    : traps(traps), max_n_transfers(max_n_transfers) {

    n_traps = traps.size();
    n_watermarks_per_transfer = 2;
    empty_watermark = 0.0;
    filled_watermark = 1.0;
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

    watermark_volumes = std::valarray<double>(empty_watermark, n_watermarks);
    watermark_fills = std::valarray<double>(empty_watermark, n_traps * n_watermarks);
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
    wmk_volumes, wmk_fills : std::valarray<double>
        Watermark arrays. See TrapManager().
    
    Returns
    -------
    n_trapped_electrons : double
        The number of electrons stored in traps.
*/
double TrapManager::n_trapped_electrons_from_watermarks(
    std::valarray<double> wmk_volumes, std::valarray<double> wmk_fills) {
    double n_trapped_electrons = 0.0;
    std::valarray<double> n_trapped_electrons_each_watermark(0.0, n_watermarks);

    // Each trap species
    for (int i_trap = 0; i_trap < n_traps; i_trap++) {
        // Store the fill fractions in a 1D array
        n_trapped_electrons_each_watermark =
            wmk_fills[std::slice(i_trap, n_watermarks, n_traps)];

        // Multiply the fill fractions by the fractional volumes
        n_trapped_electrons_each_watermark *= wmk_volumes;

        // Sum the number of electrons in each watermark level and multiply by
        // the trap density
        n_trapped_electrons +=
            n_trapped_electrons_each_watermark.sum() * traps[i_trap].density;
    }

    return n_trapped_electrons;
}

/*
    Find the index of the watermark with a volume that reaches above the cloud.

    Parameters
    ----------
    wmk_volumes : std::valarray<double>
        Watermark volumes. See TrapManager().
        
    cloud_fractional_volume : double
        The fractional volume the electron cloud reaches in the pixel well.
    
    Returns
    -------
    watermark_index_above_cloud : int
        The index of the watermark that reaches above the cloud.
*/
int TrapManager::watermark_index_above_cloud_from_volumes(
    std::valarray<double> wmk_volumes, double cloud_fractional_volume) {

    double cumulative_volume = 0.0;

    // Sum up the fractional volumes until surpassing the cloud volume
    for (int i_wmk = 0; i_wmk < n_active_watermarks; i_wmk++) {
        // Total volume so far
        cumulative_volume += watermark_volumes[i_wmk];

        if (cumulative_volume > cloud_fractional_volume) return i_wmk;
    }

    // Cloud volume above all watermarks
    return n_active_watermarks;
}

//
// TrapManagerInstantCapture::
//

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
    watermark_volumes, watermark_fills : std::valarray<double>
        The updated watermarks. See TrapManager().
*/
double TrapManagerInstantCapture::n_electrons_released() {
    double n_trapped_electrons_initial =
        n_trapped_electrons_from_watermarks(watermark_volumes, watermark_fills);

    // Each active watermark
    for (int i_wmk = 0; i_wmk < n_active_watermarks; i_wmk++) {
        // Each trap species
        for (int i_trap = 0; i_trap < n_traps; i_trap++) {
            // Update the watermark fill fraction
            watermark_fills[i_wmk * n_traps + i_trap] *=
                fill_probabilities_from_release[i_trap];
        }
    }

    double n_trapped_electrons_final =
        n_trapped_electrons_from_watermarks(watermark_volumes, watermark_fills);

    return n_trapped_electrons_initial - n_trapped_electrons_final;
}

/*
    Capture electrons in traps and update the watermarks.

    Parameters
    ----------
    cloud_fractional_volume : double
        The fractional volume the electron cloud reaches in the pixel well.

    Returns
    -------
    n_electrons_captured : double
        The number of captured electrons.

    Updates
    -------
    watermark_volumes, watermark_fills : std::valarray<double>
        The updated watermarks. See TrapManager().
*/
double TrapManagerInstantCapture::n_electrons_captured(double cloud_fractional_volume) {
    // No capture
    if (cloud_fractional_volume == 0.0) return 0.0;

    double n_trapped_electrons_initial =
        n_trapped_electrons_from_watermarks(watermark_volumes, watermark_fills);
    int watermark_index_above_cloud = watermark_index_above_cloud_from_volumes(
        watermark_volumes, cloud_fractional_volume);

    // First capture: set new watermark
    if (n_active_watermarks == 0) {
        // Set fractional volume
        watermark_volumes[0] = cloud_fractional_volume;

        // Set fill fractions for all trap species
        watermark_fills[std::slice(0, n_traps, 1)] = filled_watermark;

        // Update count of active watermarks
        n_active_watermarks++;
    }

    // Cloud below all current watermarks: set new watermark and update the rest
    else if (watermark_index_above_cloud == 0) {
        // Copy-paste all higher watermarks up one to make room
        watermark_volumes[std::slice(1, n_active_watermarks, 1)] =
            (std::valarray<double>)
                watermark_volumes[std::slice(0, n_active_watermarks, 1)];
        watermark_fills[std::slice(n_traps, n_active_watermarks * n_traps, 1)] =
            (std::valarray<double>)
                watermark_fills[std::slice(0, n_active_watermarks * n_traps, 1)];

        // Update count of active watermarks
        n_active_watermarks++;

        // New watermark
        watermark_volumes[0] = cloud_fractional_volume;
        watermark_fills[std::slice(0, n_traps, 1)] = filled_watermark;

        // Update fractional volume of the partially overwritten watermark above
        watermark_volumes[1] -= cloud_fractional_volume;
    }

    // Cloud above all current watermarks: overwrite all watermarks
    else if (watermark_index_above_cloud == n_active_watermarks) {
        // New lowest watermark
        watermark_volumes[0] = cloud_fractional_volume;
        watermark_fills[std::slice(0, n_traps, 1)] = filled_watermark;

        // Empty all other watermarks
        watermark_volumes[std::slice(1, n_active_watermarks, 1)] = empty_watermark;
        watermark_fills[std::slice(n_traps, n_active_watermarks * n_traps, 1)] =
            empty_watermark;

        // Update count of active watermarks
        n_active_watermarks = 1;
    }

    // Cloud between current watermarks
    else {
        // Update fractional volume of the partially overwritten watermark
        double previous_total_volume =
            ((std::valarray<double>)
                 watermark_volumes[std::slice(0, watermark_index_above_cloud + 1, 1)])
                .sum();
        watermark_volumes[watermark_index_above_cloud] =
            previous_total_volume - cloud_fractional_volume;

        // New lowest watermark
        watermark_volumes[0] = cloud_fractional_volume;
        watermark_fills[std::slice(0, n_traps, 1)] = filled_watermark;

        // Update count of active watermarks
        n_active_watermarks++;

        // Copy-paste all higher watermarks down to just above the new one
        watermark_volumes[std::slice(
            1, n_active_watermarks - watermark_index_above_cloud, 1)] =
            (std::valarray<double>)watermark_volumes[std::slice(
                watermark_index_above_cloud,
                n_active_watermarks - watermark_index_above_cloud, 1)];
        watermark_fills[std::slice(
            n_traps, (n_active_watermarks - watermark_index_above_cloud) * n_traps,
            1)] =
            (std::valarray<double>)watermark_fills[std::slice(
                watermark_index_above_cloud * n_traps,
                (n_active_watermarks - watermark_index_above_cloud) * n_traps, 1)];

        // Empty all other watermarks
        watermark_volumes[std::slice(
            n_active_watermarks - watermark_index_above_cloud + 1,
            watermark_index_above_cloud - 1, 1)] = empty_watermark;
        watermark_fills[std::slice(
            (n_active_watermarks - watermark_index_above_cloud + 1) * n_traps,
            (watermark_index_above_cloud - 1) * n_traps, 1)] = empty_watermark;

        // Update count of active watermarks
        n_active_watermarks -= watermark_index_above_cloud;
    }

    // Not-enough capture
    // ##todo

    double n_trapped_electrons_final =
        n_trapped_electrons_from_watermarks(watermark_volumes, watermark_fills);

    return n_trapped_electrons_final - n_trapped_electrons_initial;
}

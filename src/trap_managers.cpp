
#include <math.h>
#include <stdio.h>
#include <valarray>
#include <sys/time.h>//###

#include "ccd.hpp"
#include "trap_managers.hpp"
#include "traps.hpp"
#include "util.hpp"


// ========
// TrapManager::
// ========
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
        
    ccd : CCD
        Parameters to describe how electrons fill the volume inside (each phase
        of) a pixel in a CCD detector.
            
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
            The ith watermark, jth trap fill:
                watermark_fills[ i * n_traps + j ]
        
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
    
    empty_watermark : double
        The watermark value corresponding to empty traps.

    n_watermarks : int
        The total number of available watermark levels.
        
    n_active_watermarks : int
        The number of currently active watermark levels.
        
    first_active_watermark : int
        The effective starting point for the first active index of the 
        watermark arrays. i.e. The number of old watermark levels to ignore 
        because they've been overwritten by subsequent capture.
*/
TrapManager::TrapManager(std::valarray<Trap> traps, int max_n_transfers, CCD ccd)
    : traps(traps), max_n_transfers(max_n_transfers), ccd(ccd) {

    n_traps = traps.size();
    n_watermarks_per_transfer = 2;
    empty_watermark = 0.0;
    n_active_watermarks = 0;
    first_active_watermark = 0;
}

/*
    Initialise the watermark arrays.
    
    Sets
    ----
    n_watermarks : int
        The total number of available watermarks.
    
    watermark_volumes, watermark_fills : std::valarray<double>
        The initial empty watermark arrays. See TrapManager().
*/
void TrapManager::initialise_trap_states() {
    n_watermarks = max_n_transfers * n_watermarks_per_transfer + 1;

    watermark_volumes = std::valarray<double>(empty_watermark, n_watermarks);
    watermark_fills = std::valarray<double>(empty_watermark, n_traps * n_watermarks);

    // Initialise the stored trap states too
    store_trap_states();
}

/*
    Reset the watermark arrays to empty.
*/
void TrapManager::reset_trap_states() {
    n_active_watermarks = 0;
    first_active_watermark = 0;
    watermark_volumes = std::valarray<double>(empty_watermark, n_watermarks);
    watermark_fills = std::valarray<double>(empty_watermark, n_traps * n_watermarks);
}

/*
    Store the watermark arrays to be loaded again later.
*/
void TrapManager::store_trap_states() {
    stored_n_active_watermarks = n_active_watermarks;
    stored_first_active_watermark = first_active_watermark;
    stored_watermark_volumes = watermark_volumes;
    stored_watermark_fills = watermark_fills;
}

/*
    Restore the watermark arrays to their saved values.
*/
void TrapManager::restore_trap_states() {
    n_active_watermarks = stored_n_active_watermarks;
    first_active_watermark = stored_first_active_watermark;
    watermark_volumes = stored_watermark_volumes;
    watermark_fills = stored_watermark_fills;
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
    
    empty_probabilities_from_release : std::valarray<double>
        The fraction of traps that were full that become empty after release.
*/
void TrapManager::set_fill_probabilities_from_dwell_time(double dwell_time) {
    double total_rate, exponential_factor;
    fill_probabilities_from_empty = std::valarray<double>(0.0, n_traps);
    fill_probabilities_from_full = std::valarray<double>(0.0, n_traps);
    fill_probabilities_from_release = std::valarray<double>(0.0, n_traps);
    empty_probabilities_from_release = std::valarray<double>(0.0, n_traps);

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
        empty_probabilities_from_release[i_trap] = 
            1.0 - fill_probabilities_from_release[i_trap];
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
    
    //### Only search up to n_active_watermarks

    // Each trap species
    for (int i_trap = 0; i_trap < n_traps; i_trap++) {
        // Store the fill fractions in a 1D array
        n_trapped_electrons_each_watermark =
            wmk_fills[std::slice(
                first_active_watermark * n_traps + i_trap, n_watermarks, n_traps)];

        // Multiply the fill fractions by the fractional volumes
        n_trapped_electrons_each_watermark *= wmk_volumes[
            std::slice(first_active_watermark, n_watermarks + 1, 1)];

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


// ========
// TrapManagerInstantCapture::
// ========
/*
    Class TrapManagerInstantCapture.
    
    For the old release-then-instant-capture algorithm.
*/
TrapManagerInstantCapture::TrapManagerInstantCapture(
    std::valarray<Trap> traps, int max_n_transfers, CCD ccd)
    : TrapManager(traps, max_n_transfers, ccd) {

    // Overwrite default parameter values
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
    double n_released = 0;
    double n_released_this_trap;
    double frac_released;

    // Each active watermark
    for (int i_wmk = 0; i_wmk < n_active_watermarks; i_wmk++) {
        n_released_this_trap = 0;
        
        // Each trap species
        for (int i_trap = 0; i_trap < n_traps; i_trap++) {
            // Fraction of released electrons
            frac_released = watermark_fills[i_wmk * n_traps + i_trap] *
                empty_probabilities_from_release[i_trap];
            
            // Multiply by the trap density
            n_released_this_trap += frac_released * traps[i_trap].density;
            
            // Update the watermark fill fraction
            watermark_fills[i_wmk * n_traps + i_trap] -= frac_released;
        }
        
        // Multiply by the watermark fractional volume
        n_released += n_released_this_trap * watermark_volumes[i_wmk];
    }

    return n_released;
}

/*
    Capture electrons in traps and update the watermarks.
    
    Parameters
    ----------
    n_free_electrons : double
        The number of available electrons for trapping.

    Returns
    -------
    n_electrons_captured : double
        The number of captured electrons.

    Updates
    -------
    watermark_volumes, watermark_fills : std::valarray<double>
        The updated watermarks. See TrapManager().
*/
double TrapManagerInstantCapture::n_electrons_captured(double n_free_electrons) {
    // The fractional volume the electron cloud reaches in the pixel well.
    double cloud_fractional_volume =
        ccd.cloud_fractional_volume_from_electrons(n_free_electrons);

    // No capture
    if (cloud_fractional_volume == 0.0) return 0.0;

    double n_trapped_electrons_initial =
        n_trapped_electrons_from_watermarks(watermark_volumes, watermark_fills);
    int watermark_index_above_cloud = watermark_index_above_cloud_from_volumes(
        watermark_volumes, cloud_fractional_volume);

    // Temporarily copy initial watermarks in case needed for not-enough capture
    tmp_watermark_volumes = watermark_volumes;
    tmp_watermark_fills = watermark_fills;
    int tmp_n_active_watermarks = n_active_watermarks;

    // First capture: set new watermark
    if (n_active_watermarks == 0) {
        // Set fractional volume
        watermark_volumes[0] = cloud_fractional_volume;

        // Set fill fractions for all trap species
        watermark_fills[std::slice(0, n_traps, 1)] = 1.0;

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
        watermark_fills[std::slice(0, n_traps, 1)] = 1.0;

        // Update fractional volume of the partially overwritten watermark above
        watermark_volumes[1] -= cloud_fractional_volume;
    }

    // Cloud above all current watermarks: ignore all old watermarks
    else if (watermark_index_above_cloud == n_active_watermarks) {
        // Skip all overwritten watermarks
        first_active_watermark = watermark_index_above_cloud - 1;
        
        // New first watermark
        watermark_volumes[first_active_watermark] = cloud_fractional_volume;
        watermark_fills[std::slice(first_active_watermark * n_traps, n_traps, 1)] = 1.0;

        // Update count of active watermarks
        n_active_watermarks = 1;
    }

    // Cloud between current watermarks: ignore watermarks below
    else {
        // Update fractional volume of the partially overwritten watermark
        double previous_total_volume =
            ((std::valarray<double>)
                 watermark_volumes[std::slice(0, watermark_index_above_cloud + 1, 1)])
                .sum();
        watermark_volumes[watermark_index_above_cloud] =
            previous_total_volume - cloud_fractional_volume;
        
        // Skip all overwritten watermarks
        first_active_watermark = watermark_index_above_cloud - 1;

        // New first watermark
        watermark_volumes[first_active_watermark] = cloud_fractional_volume;
        watermark_fills[std::slice(first_active_watermark * n_traps, n_traps, 1)] = 1.0;

        // Update count of active watermarks
        n_active_watermarks += 1 - watermark_index_above_cloud;
    }

    double n_trapped_electrons_final =
        n_trapped_electrons_from_watermarks(watermark_volumes, watermark_fills);

    // Not enough available electrons to capture
    double enough =
        n_free_electrons / (n_trapped_electrons_final - n_trapped_electrons_initial);
    // if (enough < 1.0) {}

    return n_trapped_electrons_final - n_trapped_electrons_initial;
}

/*
    Release and capture electrons and update the trap watermarks.

    Parameters
    ----------
    n_free_electrons : double
        The number of available electrons for trapping.
    
    Returns
    -------
    n_electrons_released_and_captured : double
        The number of released electrons.
        
    Updates
    -------
    watermark_volumes, watermark_fills : std::valarray<double>
        The updated watermarks. See TrapManager().
*/
double TrapManagerInstantCapture::n_electrons_released_and_captured(
    double n_free_electrons) {

    double n_released = n_electrons_released();
    // printf("  n_released %g \n", n_released);

    double n_captured = n_electrons_captured(n_free_electrons + n_released);
    // printf("  n_captured %g \n", n_captured);
    
    return n_released - n_captured;
}

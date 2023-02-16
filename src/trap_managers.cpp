
#include "trap_managers.hpp"

#include <math.h>
#include <stdio.h>

#include <valarray>

#include "ccd.hpp"
#include "traps.hpp"
#include "util.hpp"
#include <iostream>

// ========
// TrapManagerBase::
// ========
/*
    Class TrapManagerBase.

    Abstract base class for the trap manager of one or multiple trap species
    that are able to use watermarks in the same way as each other.

    Parameters
    ----------
    traps : std::valarray<Trap>
        A list of one or more trap species of the specific trap manager's type.

        Not included in this base class, since the different managers require
        different class types for the array.

    max_n_transfers : int
        The number of pixel transfers containing traps that charge will be
        expected to go through. This feeds in to the maximum number of possible
        capture/release events that could create new watermark levels, and is
        used to initialise the watermark array to be only as large as needed.

    ccd_phase : CCDPhase
        Parameters to describe how electrons fill the volume inside (one phase
        of) a pixel in a CCD detector.

    dwell_time : double
        The time spent in this pixel or phase, in the same units as the trap
        timescales.

    Attributes
    ----------
    n_traps : int
        The number of trap species.

    trap_densities : std::valarray<double>
        The density of each trap species.

    watermark_volumes : std::valarray<double>
        Array of watermark fractional volumes to describe the trap states, i.e.
        the proportion of the pixel volume occupied by each (active) watermark.

    watermark_fills : std::valarray<double>
        2D-style 1D array of watermark fill fractions to describe the trap
        states, i.e. the proportion of traps that are filled in each (active)
        watermark, multiplied by the trap's density, for each trap species.

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

    empty_watermark : double
        The watermark fill value corresponding to empty traps.

    zeroth_watermark : double
        The lowest watermark height value yet considered.

    n_active_watermarks : int
        The number of currently active watermark levels.

    i_first_active_wmk : int
        The index of the first active watermark. The effective starting point
        for the active region of the watermark arrays. i.e. watermark levels
        below this are ignored because they've been superseded.

        This is primarily used by instant-capture traps where a charge cloud
        will entirely fill all traps below it, making any pre-existing lower
        watermarks no longer necessary to track.

    n_watermarks_per_transfer : int
        The number of new watermarks that could be made in each transfer.

    n_watermarks : int
        The total number of available watermark levels, determined by the number
        of potential watermark-creating transfers and the watermarking scheme.
*/
TrapManagerBase::TrapManagerBase(
    int max_n_transfers, CCDPhase ccd_phase, double dwell_time)
    : max_n_transfers(max_n_transfers), ccd_phase(ccd_phase), dwell_time(dwell_time) {

    empty_watermark = 0.0;
    zeroth_watermark = 0.0;
    n_active_watermarks = 0;
    i_first_active_wmk = 0;
    n_watermarks_per_transfer = 1;
}

/*
    Initialise the watermark arrays.

    Sets
    ----
    n_watermarks : int
        The total number of available watermarks.

    watermark_volumes, watermark_fills : std::valarray<double>
        The initial empty watermark arrays.
*/
void TrapManagerBase::initialise_trap_states() {
    n_watermarks = max_n_transfers * n_watermarks_per_transfer + 1;

    watermark_volumes = std::valarray<double>(zeroth_watermark, n_watermarks);
    watermark_fills = std::valarray<double>(empty_watermark, n_traps * n_watermarks);
    //empty_probabilities_from_release = std::valarray<double>(0.0, n_traps);
    
    // Initialise the stored trap states too
    store_trap_states();
}

/*
    Reset the watermark arrays to empty.
*/
void TrapManagerBase::reset_trap_states() {
    n_active_watermarks = 0;
    i_first_active_wmk = 0;
    watermark_volumes = zeroth_watermark;
    watermark_fills = empty_watermark;
}

/*
    Store the watermark arrays to be loaded again later.
*/
void TrapManagerBase::store_trap_states() {
    stored_n_active_watermarks = n_active_watermarks;
    stored_i_first_active_wmk = i_first_active_wmk;
    stored_watermark_volumes = watermark_volumes;
    stored_watermark_fills = watermark_fills;
}

/*
    Restore the watermark arrays to their saved values.
*/
void TrapManagerBase::restore_trap_states() {
    n_active_watermarks = stored_n_active_watermarks;
    i_first_active_wmk = stored_i_first_active_wmk;
    watermark_volumes = stored_watermark_volumes;
    watermark_fills = stored_watermark_fills;
}

/*
    Call any necessary initialisation functions, etc.
*/
void TrapManagerBase::setup() { initialise_trap_states(); }

/*
    Report the number of electrons currently held in all traps
    within a single watermark.

    Returns
    -------
    n_trapped_electrons_per_watermark : std::valarray<double>
        The total number of electrons (summed over different species of 
        traps) stored in a particular watermark.
*/
double TrapManagerBase::n_trapped_electrons_in_watermark(int i_wmk) {

    // Fill fraction * trap density (an array for each trap species)
    std::valarray<double> fill_fractions_in_this_wmk =
        watermark_fills[std::slice(i_wmk * n_traps, n_traps, 1)];
    // Multiplied by volume
    double n_trapped_electrons_in_this_wmk = 
        fill_fractions_in_this_wmk.sum() * watermark_volumes[i_wmk];

    return n_trapped_electrons_in_this_wmk;
}

/*
    Report the number of electrons currently held in all traps.

    Returns
    -------
    n_trapped_electrons_per_watermark : std::valarray<double>
        The total number of electrons (summed over different species of 
        traps) stored in all watermarks.
*/
std::valarray<double> TrapManagerBase::n_trapped_electrons_per_watermark() {

    // Count the total number of electrons in each active watermark
    std::valarray<double> fill_fractions_this_wmk(n_traps);
    std::valarray<double> n_trapped_electrons_per_wmk(n_watermarks);
    double n_trapped_electrons_in_total = 0;
    double highest_watermark = 0;


    for (int i_wmk = i_first_active_wmk;
        i_wmk < i_first_active_wmk + n_active_watermarks; i_wmk++) {

        // Extract the fill fractions in a 1D array
        fill_fractions_this_wmk =
            watermark_fills[std::slice(i_wmk * n_traps, n_traps, 1)];
        // Sum the fill fractions and multiply by the fractional volume
        n_trapped_electrons_per_wmk[i_wmk] = 
            fill_fractions_this_wmk.sum() * watermark_volumes[i_wmk];
                
        //print_v(0,"watermArk: %d volume: %g fill: %g n_electrons: %g n_traps: %d \n", 
        //    i_wmk,watermark_volumes[i_wmk],
        //    fill_fractions_this_wmk.sum(),
        //    n_trapped_electrons_per_wmk[i_wmk],n_traps);

    }
    //print_v(0,"Total n_electrons %16.12g, highest watermark %16.12g\n",n_trapped_electrons_in_total,highest_watermark);

    return n_trapped_electrons_per_wmk;
}
double TrapManagerBase::n_trapped_electrons_total() {

    // No watermarks
    if (n_active_watermarks == 0) return 0.0;
    
    // Sum over all watermarks
    double n_trapped_electrons = (n_trapped_electrons_per_watermark()).sum();

    return n_trapped_electrons;
}



/*
    Sum the total number of electrons currently held in traps.

    Parameters
    ----------
    wmk_volumes, wmk_fills : std::valarray<double>
        The watermark arrays.

    Returns
    -------
    n_trapped_electrons : double
        The number of electrons stored in traps.
*/
double TrapManagerBase::n_trapped_electrons_from_watermarks(
    std::valarray<double> wmk_volumes, std::valarray<double> wmk_fills) {

    // No watermarks
    if (n_active_watermarks == 0) return 0.0;

    double n_trapped_electrons_total = 0.0;
    std::valarray<double> n_trapped_electrons_this_wmk(n_traps);

    // Each active watermark
    for (int i_wmk = i_first_active_wmk;
         i_wmk < i_first_active_wmk + n_active_watermarks; i_wmk++) {
        // Extract the fill fractions in a 1D array
        n_trapped_electrons_this_wmk =
            wmk_fills[std::slice(i_wmk * n_traps, n_traps, 1)];

        // Sum the fill fractions and multiply by the fractional volume
        n_trapped_electrons_total += n_trapped_electrons_this_wmk.sum() * wmk_volumes[i_wmk];
    }

    return n_trapped_electrons_total;
}



/*
    Reduce the value of the lowest watermark ever seen. It starts at zero, so this
    function is only activated if the image contains negative pixels. 

    Parameters
    ----------
    cloud_fractional_volume : double
        The fractional volume the electron cloud reaches in the pixel well.

    Returns
    -------
    i_wmk_above_cloud : int
        The index of the first active watermark that reaches above the cloud.
*/
void TrapManagerBase::lower_zeroth_watermark(double n_free_electrons) {
    //zeroth_watermark = n_free_electrons;
}



/*
    Remove watermarks containing very low numbers of electrons, for speed later.
    
    To remove watermark i that contains Ni electrons in volume Vi,
    we push all those electrons down to the next lower watermark b such that
      Vb' = Vb + Vb Ni/Nb
      fb' = fb
      Nb' = Nb + Ni   ( = Vb' * fb' )
    and increase the volume of the watermark above such that
      Va' = Va + Vi - Vb Ni/Nb
      fa' = fa * Va/Va'
      Na' = Na        ( = Va' * fa' )
    
    Parameters
    ----------
    min_n_electrons : double Default [0] i.e. do nothing
        If this is positive...
        If a watermark contains fewer than this number of electrons, it will be
        dissolved.
        If this is negative...
        If a watermark will release fewer than this number of electrons in the next
        time setp, it will be dissolved.
*/
void TrapManagerBase::prune_watermarks(double min_n_electrons) {

    

    // With only one watermark, not much can be done
    if (n_active_watermarks <= 1) return; // Cannot prune if there is only a trunk
    if (n_trapped_electrons_in_watermark(i_first_active_wmk) <= 0) return; // Something has gone wrong to get here
    print_v(3, "prune watermarks continaing fewer than %g electrons. Watermarks: %d %d\n", min_n_electrons, i_first_active_wmk, n_active_watermarks);

    // Count the total number of electrons in each watermark
    print_v(3,"\n\n Fill fractions before prune (first %d n %d)\n",i_first_active_wmk, n_active_watermarks);
    std::valarray<double> n_trapped_electrons_per_wmk = n_trapped_electrons_per_watermark();
    double n_trapped_electrons_start = n_trapped_electrons_per_wmk.sum();
    double n_trapped_electrons_in_this_wmk;
    double test_value;
    double test_criterion = abs(min_n_electrons);
              
    // Consider each active watermark
    int n_watermarks_pruned = 0;
    for (int i_wmk = i_first_active_wmk + 1;
        i_wmk < i_first_active_wmk + n_active_watermarks; i_wmk++) {
        
        int i_wmk_prime = i_wmk - n_watermarks_pruned;
                
        // Work out how many electrons are in this watermark
        n_trapped_electrons_in_this_wmk = n_trapped_electrons_in_watermark(i_wmk_prime);
        if (min_n_electrons > 0) {
            test_value = n_trapped_electrons_in_this_wmk;
        } else {
            test_value = n_electrons_released_from_wmk_above_cloud(i_wmk_prime);
        } 
        if (test_value < test_criterion) {
            
            // Push the small number of electrons down into previous watermark
            // (redistributing them between trap species by keeping fill fractions 
            // as before, but increasing the watermark volume. This is not ideal
            // but ensures no fill fractions ever exceed 1)
            double n_trapped_electrons_in_last_wmk = n_trapped_electrons_in_watermark(i_wmk_prime - 1);
                        
            double delta_volume_below = watermark_volumes[i_wmk_prime - 1] *
                n_trapped_electrons_in_this_wmk / n_trapped_electrons_in_last_wmk;

/*            // Temp report
            std::valarray<double> fill_fractions_in_down_wmk =
                watermark_fills[std::slice((i_wmk_prime-1) * n_traps, n_traps, 1)];
            double n_trapped_electrons_in_down_wmk = 
                fill_fractions_in_down_wmk.sum() * watermark_volumes[i_wmk_prime-1]; 
            print_v(0,"DownIncTo: %d volume: %g fill: %g n_electrons: %g \n", 
                i_wmk_prime-1,watermark_volumes[i_wmk_prime-1],
                fill_fractions_in_down_wmk.sum(),
                n_trapped_electrons_in_down_wmk);

            // Temp report
            std::valarray<double> fill_fractions_in_this_wmk =
                watermark_fills[std::slice(i_wmk_prime * n_traps, n_traps, 1)];
            double n_trapped_electrons_in_this_wmk = 
                fill_fractions_in_this_wmk.sum() * watermark_volumes[i_wmk_prime]; 
            print_v(0,"ThisRemov: %d volume: %g fill: %g n_electrons: %g test: %g \n", 
                i_wmk_prime,watermark_volumes[i_wmk_prime],
                fill_fractions_in_this_wmk.sum(),
                n_trapped_electrons_in_this_wmk,test_value);
*/            
            
            
            watermark_volumes[i_wmk_prime - 1] += delta_volume_below;
            
            // If there is a higher watermark...
            if ((i_wmk + 1) < (i_first_active_wmk + n_active_watermarks)) {
                
                

/*                // Temp report
                std::valarray<double> fill_fractions_in_next_wmk =
                    watermark_fills[std::slice((i_wmk_prime+1) * n_traps, n_traps, 1)];
                double n_trapped_electrons_in_next_wmk = 
                    fill_fractions_in_next_wmk.sum() * watermark_volumes[i_wmk_prime+1]; 
                print_v(0,"ReplaceBy: %d volume: %g fill: %g n_electrons: %g \n", 
                    i_wmk_prime,watermark_volumes[i_wmk_prime+1],
                    fill_fractions_in_next_wmk.sum(),
                    n_trapped_electrons_in_next_wmk);
*/
         
                
                // ...increase its volume to compensate,
                double delta_volume_above = watermark_volumes[i_wmk_prime] - delta_volume_below;
                watermark_volumes[i_wmk_prime + 1] += delta_volume_above;
                
                // Prevent division by zero if Va = Vi = 0
                if (watermark_volumes[i_wmk_prime + 1] > 0) {

                    // ...reduce its fill fraction to preserve number of electrons, 
                    double fill_multiplier = 
                        (watermark_volumes[i_wmk_prime + 1] - delta_volume_above) /
                        watermark_volumes[i_wmk_prime + 1];
                    for (int i_trap = 0; i_trap < n_traps; i_trap++) {
                        watermark_fills[(i_wmk_prime + 1) * n_traps + i_trap] *= fill_multiplier;
                    }
                
                } //else { print_v(0,"Problem watermark!\n"); } 
                // These fill fractions should be zero (I hope), so don't change them anyway.
                
                // ...then shuffle all higher watermarks down to earlier watermark position
                for (int j_wmk = i_wmk + 1;
                    j_wmk < i_first_active_wmk + n_active_watermarks; j_wmk++) {
                
                    int j_wmk_prime = j_wmk - n_watermarks_pruned - 1;
                    watermark_volumes[j_wmk_prime] = watermark_volumes[j_wmk_prime + 1];
                    watermark_fills[std::slice(j_wmk_prime * n_traps, n_traps, 1)] = 
                        watermark_fills[std::slice((j_wmk_prime + 1) * n_traps, n_traps, 1)];
                }
            }
            n_watermarks_pruned += 1;          
        } else {

/*            // Temp report
            std::valarray<double> fill_fractions_in_this_wmk =
                watermark_fills[std::slice(i_wmk_prime * n_traps, n_traps, 1)];
            double n_trapped_electrons_in_this_wmk = 
                fill_fractions_in_this_wmk.sum() * watermark_volumes[i_wmk_prime]; 
            print_v(0,"ThisStays: %d volume: %g fill: %g n_electrons: %g test: %g \n", 
                i_wmk_prime,watermark_volumes[i_wmk_prime],
                fill_fractions_in_this_wmk.sum(),
                n_trapped_electrons_in_this_wmk,test_value);
 */       
        }
    }
    n_active_watermarks -= n_watermarks_pruned;

    // Count the total number of electrons in each watermark
    print_v(3,"\n\n Fill fractions after prune (first %d n %d)\n",i_first_active_wmk, n_active_watermarks);
    
    //int flag;
    //flag = std::cin.get();
    //if(flag == 'q') { abort(); }
}

/*
    Find the index of the watermark with a volume that reaches above the cloud.

    Parameters
    ----------
    cloud_fractional_volume : double
        The fractional volume the electron cloud reaches in the pixel well.

    Returns
    -------
    i_wmk_above_cloud : int
        The index of the first active watermark that reaches above the cloud.
*/
int TrapManagerBase::watermark_index_above_cloud(double cloud_fractional_volume) {
    double cumulative_volume = zeroth_watermark;
    
    // Sum up the fractional volumes until surpassing the cloud volume
    for (int i_wmk = i_first_active_wmk;
         i_wmk < i_first_active_wmk + n_active_watermarks; i_wmk++) {
        // Total volume so far
        cumulative_volume += watermark_volumes[i_wmk];

        if (cumulative_volume > cloud_fractional_volume) return i_wmk;
    }

    // Cloud volume above all watermarks
    return i_first_active_wmk + n_active_watermarks;
}

/*
    How many electrons will be released from a watermark above the cloud,
    during the next timestep.

    Parameters
    ----------
    cloud_fractional_volume : double
        The fractional volume the electron cloud reaches in the pixel well.

    Returns
    -------
    i_wmk_above_cloud : int
        The index of the first active watermark that reaches above the cloud.
*/
double TrapManagerBase::n_electrons_released_from_wmk_above_cloud(int i_wmk) {
    throw std::runtime_error( "this should get replaced for any trap type" );
    return 0;
}










// ========
// TrapManagerInstantCapture::
// ========
/*
    Class TrapManagerInstantCapture.

    For the standard release-then-instant-capture algorithm.

    Attributes
    ----------
    any_non_uniform_traps : double
        Default false, set to true if any of the traps have a non-uniform
        distribution with volume, in which case some small extra steps are
        required in the release and capture functions.
*/
TrapManagerInstantCapture::TrapManagerInstantCapture(
    std::valarray<TrapInstantCapture> traps, int max_n_transfers, CCDPhase ccd_phase,
    double dwell_time)
    : TrapManagerBase(max_n_transfers, ccd_phase, dwell_time), traps(traps) {

    n_traps = traps.size();
    trap_densities = std::valarray<double>(n_traps);
    for (int i_trap = 0; i_trap < n_traps; i_trap++) {
        trap_densities[i_trap] = traps[i_trap].density;
    }

    any_non_uniform_traps = false;
    for (int i_trap = 0; i_trap < n_traps; i_trap++) {
        if (traps[i_trap].fractional_volume_full_exposed > 0.0) {
            any_non_uniform_traps = true;
            break;
        }
    }
}

/*
    Set the probabilities of traps being full after release.

    Sets
    ----
    empty_probabilities_from_release : std::valarray<double>
        The fraction of traps that were full that become empty after release.
*/
void TrapManagerInstantCapture::set_fill_probabilities() {
    empty_probabilities_from_release = std::valarray<double>(0.0, n_traps);

    // Set probabilities for each trap species
    for (int i_trap = 0; i_trap < n_traps; i_trap++) {
        // Resulting empty fraction from release
        empty_probabilities_from_release[i_trap] =
            1.0 - exp(-traps[i_trap].release_rate * dwell_time);
    }
}

/*
    Call any necessary initialisation functions, etc.
*/
void TrapManagerInstantCapture::setup() {
    initialise_trap_states();
    set_fill_probabilities();
}

/*
    Release electrons from traps and update the watermarks.

    Returns
    -------
    n_electrons_released : double
        The number of released electrons.

    Updates
    -------
    watermark_volumes, watermark_fills : std::valarray<double>
        The updated watermarks.
*/
double TrapManagerInstantCapture::n_electrons_released() {
    double n_released = 0.0;
    double n_released_this_wmk;
    double frac_released;
    double frac_exposed_per_volume;
    double cumulative_volume = 0.0;
    double next_cumulative_volume = 0.0;

    // Each active watermark
    for (int i_wmk = i_first_active_wmk;
         i_wmk < i_first_active_wmk + n_active_watermarks; i_wmk++) {
        n_released_this_wmk = 0.0;

        if (any_non_uniform_traps) {
            // Total volume at the bottom and top of this watermark
            cumulative_volume = next_cumulative_volume;
            next_cumulative_volume += watermark_volumes[i_wmk];
        }

        // Each trap species
        for (int i_trap = 0; i_trap < n_traps; i_trap++) {
            // Fraction of released electrons
            frac_released = watermark_fills[i_wmk * n_traps + i_trap] *
                            empty_probabilities_from_release[i_trap];

            // Account for non-uniform distribution with volume
            if (traps[i_trap].fractional_volume_full_exposed == 0.0)
                frac_exposed_per_volume = 1.0;
            else
                frac_exposed_per_volume =
                    traps[i_trap].fraction_traps_exposed_per_fractional_volume(
                        cumulative_volume, next_cumulative_volume);

            // Number released
            n_released_this_wmk += frac_released * frac_exposed_per_volume;

            // Update the watermark fill fraction
            watermark_fills[i_wmk * n_traps + i_trap] -= frac_released;
        }

        // Multiply by the watermark volume
        n_released += n_released_this_wmk * watermark_volumes[i_wmk];
    }

    return n_released;
}

/*
    How many electrons will be released from a watermark above the cloud,
    during the next timestep.

    Parameters
    ----------
    cloud_fractional_volume : double
        The fractional volume the electron cloud reaches in the pixel well.

    Returns
    -------
    i_wmk_above_cloud : int
        The index of the first active watermark that reaches above the cloud.
*/
double TrapManagerInstantCapture::n_electrons_released_from_wmk_above_cloud(int i_wmk) {
    
    //print_v(0,"IC child version of n_electrons_released_from_wmk_above_cloud %g \n",empty_probabilities_from_release[0]);;

    // Fraction of electrons released from each trap species
    double frac_released_this_wmk = 0.0;
    for (int i_trap = 0; i_trap < n_traps; i_trap++) {
        // Fraction of released electrons
        frac_released_this_wmk +=
            watermark_fills[i_wmk * n_traps + i_trap] *
            empty_probabilities_from_release[i_trap];
    }

    // Multiply by the watermark volume
    return frac_released_this_wmk * watermark_volumes[i_wmk];
}

/*
    Modify the watermarks for normal capture.

    Parameters
    ----------
    cloud_fractional_volume : double
        The fractional volume the electron cloud reaches in the pixel well.

    i_wmk_above_cloud : int
        The index of the first active watermark that reaches above the cloud.

    Updates
    -------
    watermark_volumes, watermark_fills : std::valarray<double>
        The updated watermarks.
*/
void TrapManagerInstantCapture::update_watermarks_capture(
    double cloud_fractional_volume, int i_wmk_above_cloud) {
    // First capture
    if (n_active_watermarks == 0) {
        // Set fractional volume
        watermark_volumes[0] = cloud_fractional_volume - zeroth_watermark;

        // Set fill fractions for all trap species
        watermark_fills[std::slice(0, n_traps, 1)] = trap_densities;

        // Update count of active watermarks
        n_active_watermarks++;
    }

    // Cloud below all current watermarks
    else if (i_wmk_above_cloud == i_first_active_wmk) {

        // Make room for the new lowest watermark
        if (i_first_active_wmk > 0) {
            // Use existing room below the current first active watermark
            i_first_active_wmk--;
        } else {
            // Copy-paste all higher watermarks up one to make room
            for (int i_wmk = i_first_active_wmk + n_active_watermarks - 1;
                 i_wmk >= i_first_active_wmk; i_wmk--) {
                watermark_volumes[i_wmk + 1] = watermark_volumes[i_wmk];
                for (int i_trap = 0; i_trap < n_traps; i_trap++) {
                    watermark_fills[(i_wmk + 1) * n_traps + i_trap] =
                        watermark_fills[i_wmk * n_traps + i_trap];
                }
            }
        }

        // Update count of active watermarks
        n_active_watermarks++;

        // New watermark
        watermark_volumes[i_first_active_wmk] = cloud_fractional_volume - zeroth_watermark;
        watermark_fills[std::slice(i_first_active_wmk * n_traps, n_traps, 1)] =
            trap_densities;

        // Update fractional volume of the partially overwritten watermark above
        watermark_volumes[i_first_active_wmk + 1] -= (cloud_fractional_volume - zeroth_watermark);
    }

    // Cloud above all current watermarks
    else if (i_wmk_above_cloud == i_first_active_wmk + n_active_watermarks) {
        // Skip all overwritten watermarks
        i_first_active_wmk = i_wmk_above_cloud - 1;

        // New first watermark
        watermark_volumes[i_first_active_wmk] = cloud_fractional_volume - zeroth_watermark;
        watermark_fills[std::slice(i_first_active_wmk * n_traps, n_traps, 1)] =
            trap_densities;

        // Update count of active watermarks
        print_v(2,"Resetting watermarks\n");
        n_active_watermarks = 1;
    }

    // Cloud between current watermarks
    else {
        // Update fractional volume of the partially overwritten watermark
        double previous_total_volume = zeroth_watermark;
        for (int i_wmk = i_first_active_wmk; i_wmk <= i_wmk_above_cloud; i_wmk++) {
            previous_total_volume += watermark_volumes[i_wmk];
        }
        watermark_volumes[i_wmk_above_cloud] =
            previous_total_volume - cloud_fractional_volume;

        // Update count of active watermarks
        n_active_watermarks += i_first_active_wmk - i_wmk_above_cloud + 1;

        // Skip all overwritten watermarks
        i_first_active_wmk = i_wmk_above_cloud - 1;

        // New first watermark
        watermark_volumes[i_first_active_wmk] = cloud_fractional_volume - zeroth_watermark;
        watermark_fills[std::slice(i_first_active_wmk * n_traps, n_traps, 1)] =
            trap_densities;
    }

    return;
}

/*
    Modify the watermarks for capture when not enough electrons are available.

    Each watermark is partially filled a fraction (`enough`) of the way to full,
    such that the resulting number of captured electrons is restricted to the
    number actually available for capture.

    This only becomes relevant for tiny numbers of electrons, where the cloud
    can reach a disproportionately large volume in the pixel (reaching
    correspondingly many traps) for the small amount of charge.

    Parameters
    ----------
    cloud_fractional_volume : double
        The fractional volume the electron cloud reaches in the pixel well.

    i_wmk_above_cloud : int
        The index of the first active watermark that reaches above the cloud.

    enough : double
        The amount of electrons available as a fraction of the number that
        could be captured by the watermarks reached by the cloud volume.

    Updates
    -------
    watermark_volumes, watermark_fills : std::valarray<double>
        The updated watermarks.
*/
void TrapManagerInstantCapture::update_watermarks_capture_not_enough(
    double cloud_fractional_volume, int i_wmk_above_cloud, double enough) {
    // First capture
    if (n_active_watermarks == 0) {
        // Set fractional volume
        watermark_volumes[0] = cloud_fractional_volume - zeroth_watermark;

        // Set fill fractions for all trap species
        watermark_fills[std::slice(0, n_traps, 1)] = trap_densities * enough;

        // Update count of active watermarks
        n_active_watermarks++;
    }

    // Cloud below all current watermarks
    else if (i_wmk_above_cloud == i_first_active_wmk) {

        // Make room for the new lowest watermark
        if (i_first_active_wmk > 0) {
            // Use existing room below the current first active watermark
            i_first_active_wmk--;
        } else {
            // Copy-paste all higher watermarks up one to make room
            for (int i_wmk = i_first_active_wmk + n_active_watermarks - 1;
                 i_wmk >= i_first_active_wmk; i_wmk--) {
                watermark_volumes[i_wmk + 1] = watermark_volumes[i_wmk];
                for (int i_trap = 0; i_trap < n_traps; i_trap++) {
                    watermark_fills[(i_wmk + 1) * n_traps + i_trap] =
                        watermark_fills[i_wmk * n_traps + i_trap];
                }
            }
        }

        // New watermark
        watermark_volumes[i_first_active_wmk] = cloud_fractional_volume - zeroth_watermark;
        watermark_fills[std::slice(i_first_active_wmk * n_traps, n_traps, 1)] =
            (std::valarray<double>)watermark_fills[std::slice(
                i_first_active_wmk * n_traps, n_traps, 1)] *
                (1.0 - enough) +
            enough * trap_densities; // is this wrong? the new watermark level should be created full

        // Update fractional volume of the partially overwritten watermark above
        watermark_volumes[i_first_active_wmk + 1] -= (cloud_fractional_volume - zeroth_watermark);

        // Update count of active watermarks
        n_active_watermarks++;
    }

    // Cloud above all current watermarks
    else if (i_wmk_above_cloud == i_first_active_wmk + n_active_watermarks) {
        // Cumulative volume of the watermark just below the new one
        double volume_below = zeroth_watermark;
        for (int i_wmk = i_first_active_wmk; i_wmk < i_wmk_above_cloud; i_wmk++) {
            volume_below += watermark_volumes[i_wmk];
        }

        // New watermark
        watermark_volumes[i_wmk_above_cloud] = cloud_fractional_volume - volume_below;
        //print_v(0,"Above cloud notenough. i %d wv %g cfv %g vb %g\n",i_wmk_above_cloud,watermark_volumes[i_wmk_above_cloud],cloud_fractional_volume,volume_below);
        watermark_fills[std::slice(
            (i_first_active_wmk + n_active_watermarks) * n_traps, n_traps, 1)] =
            enough * trap_densities;

        // Update all other watermarks part-way to full
        for (int i_wmk = i_first_active_wmk;
             i_wmk < i_first_active_wmk + n_active_watermarks; i_wmk++) {
            watermark_fills[std::slice(i_wmk * n_traps, n_traps, 1)] =
                (std::valarray<double>)
                        watermark_fills[std::slice(i_wmk * n_traps, n_traps, 1)] *
                    (1.0 - enough) +
                enough * trap_densities;
        }

        // Update count of active watermarks
        n_active_watermarks++;
    }

    // Cloud between current watermarks
    else {
        // Copy-paste all higher watermarks up one to make room
        for (int i_wmk = i_first_active_wmk + n_active_watermarks - 1;
             i_wmk >= i_wmk_above_cloud; i_wmk--) {
            watermark_volumes[i_wmk + 1] = watermark_volumes[i_wmk];
            for (int i_trap = 0; i_trap < n_traps; i_trap++) {
                watermark_fills[(i_wmk + 1) * n_traps + i_trap] =
                    watermark_fills[i_wmk * n_traps + i_trap];
            }
        }

        // Cumulative volume of the watermark just below the new one
        double volume_below = zeroth_watermark;
        for (int i_wmk = i_first_active_wmk; i_wmk < i_wmk_above_cloud; i_wmk++) {
            volume_below += watermark_volumes[i_wmk];
        }

        // New watermark
        watermark_volumes[i_wmk_above_cloud] = cloud_fractional_volume - volume_below;

        // Update volume of the partially overwritten watermark
        //print_v(0,"wmv %g %g\n",watermark_volumes[i_wmk_above_cloud + 1],watermark_volumes[i_wmk_above_cloud + 1]-watermark_volumes[i_wmk_above_cloud]);
        watermark_volumes[i_wmk_above_cloud + 1] -=
            watermark_volumes[i_wmk_above_cloud];

        // Update all watermarks, including the new one, part-way to full
        for (int i_wmk = i_first_active_wmk; i_wmk <= i_wmk_above_cloud; i_wmk++) {
            watermark_fills[std::slice(i_wmk * n_traps, n_traps, 1)] =
                (std::valarray<double>)
                        watermark_fills[std::slice(i_wmk * n_traps, n_traps, 1)] *
                    (1.0 - enough) +
                enough * trap_densities;
        }

        // Update count of active watermarks
        n_active_watermarks++;
    }

    return;
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
        The updated watermarks.
*/
double TrapManagerInstantCapture::n_electrons_captured(double n_free_electrons) {
    // The fractional volume the electron cloud reaches in the pixel well
    double cloud_fractional_volume =
        ccd_phase.cloud_fractional_volume_from_electrons(n_free_electrons);

    // No capture
    if (cloud_fractional_volume == 0.0) return 0.0;

    // ========
    // Count the number of electrons that can be captured by each watermark
    // ========
    double n_captured = 0.0;
    double n_captured_this_wmk;
    double cumulative_volume = zeroth_watermark;
    double next_cumulative_volume = zeroth_watermark;
    double volume_top;
    double frac_exposed_per_volume;

    int i_wmk_above_cloud = watermark_index_above_cloud(cloud_fractional_volume);

    // Each active watermark
    for (int i_wmk = i_first_active_wmk; i_wmk <= i_wmk_above_cloud; i_wmk++) {
        n_captured_this_wmk = 0.0;

        // Total volume at the bottom and top of this watermark
        cumulative_volume = next_cumulative_volume;
        next_cumulative_volume += watermark_volumes[i_wmk];

        // Capture up to the cloud volume for the last watermark
        if (i_wmk == i_wmk_above_cloud) {
            volume_top = cloud_fractional_volume;
        }
        // Capture up to the next watermark for watermarks below the cloud
        else {
            volume_top = next_cumulative_volume;
        }

        // Each trap species
        for (int i_trap = 0; i_trap < n_traps; i_trap++) {
            // Account for non-uniform distribution with volume
            if (traps[i_trap].fractional_volume_full_exposed == 0.0)
                frac_exposed_per_volume = 1.0;
            else
                frac_exposed_per_volume =
                    traps[i_trap].fraction_traps_exposed_per_fractional_volume(
                        cumulative_volume, volume_top);

            n_captured_this_wmk +=
                (trap_densities[i_trap] - watermark_fills[i_wmk * n_traps + i_trap]) *
                frac_exposed_per_volume;
        }

        // Capture from the bottom to the top of the watermark
        n_captured += n_captured_this_wmk * (volume_top - cumulative_volume);
    }

    // ========
    // Update the watermarks
    // ========
    // Check enough available electrons to capture
    //
    // Would be really nice to do this including the express multiplier.
    // That would also prevent image pixels from changing sign when zeroth_watermark<0.
    //
    double enough = abs(n_free_electrons / n_captured);
    //print_v(0,"enough %g %g %g\n", enough, n_free_electrons, n_captured);

    // Normal full capture
    if ( enough >= 1.0) {
        update_watermarks_capture(cloud_fractional_volume, i_wmk_above_cloud);
    }
    // Partial capture
    else {
        update_watermarks_capture_not_enough(
            cloud_fractional_volume, i_wmk_above_cloud, enough);

        n_captured *= enough;
    }

    return n_captured;
}

/*
    Release and capture electrons and update the trap watermarks.

    The interaction between traps and the charge cloud during its dwell time in
    this pixel (and phase) is modelled as follows:
    + First the previously filled traps release charge.
    + This may update the number of free electrons in the cloud that are
        available for capture.
    + Then any unfilled traps within the cloud volume capture charge.
    + In the rare case that not enough electrons are available for capture,
        all traps within the cloud capture proportionally less charge to match.

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
        The updated watermarks.
*/
double TrapManagerInstantCapture::n_electrons_released_and_captured(
    double n_free_electrons) {

    //print_v(0, "n_free_electrons  %g \n", n_free_electrons);
    //if (n_free_electrons < zeroth_watermark) {
    //    lower_zeroth_watermark(n_free_electrons);
    //}

    double n_released = n_electrons_released();
    print_v(2, "n_electrons_released  %g \n", n_released);

    double n_captured = n_electrons_captured(n_free_electrons + n_released);
    print_v(2, "n_electrons_captured  %g \n", n_captured);

    return n_released - n_captured;
}













// ========
// TrapManagerSlowCapture::
// ========
/*
    Class TrapManagerSlowCapture.

    For traps with a non-zero capture time.
*/
TrapManagerSlowCapture::TrapManagerSlowCapture(
    std::valarray<TrapSlowCapture> traps, int max_n_transfers, CCDPhase ccd_phase,
    double dwell_time)
    : TrapManagerBase(max_n_transfers, ccd_phase, dwell_time), traps(traps) {

    n_traps = traps.size();
    trap_densities = std::valarray<double>(n_traps);
    for (int i_trap = 0; i_trap < n_traps; i_trap++) {
        trap_densities[i_trap] = traps[i_trap].density;
    }

    // Overwrite default parameter values
    n_watermarks_per_transfer = 2;
}

/*
    Set the probabilities of traps being full after release and/or capture.

    See Lindegren (1998) section 3.2.

    Sets
    ----
    fill_probabilities_from_empty : std::valarray<double>
        The fraction of traps that were empty that become full.

    fill_probabilities_from_full : std::valarray<double>
        The fraction of traps that were full that stay full.

    empty_probabilities_from_release : std::valarray<double>
        The fraction of traps that were full that become empty after release.
*/
void TrapManagerSlowCapture::set_fill_probabilities() {
    double total_rate, exponential_factor;
    fill_probabilities_from_empty = std::valarray<double>(0.0, n_traps);
    fill_probabilities_from_full = std::valarray<double>(0.0, n_traps);
    empty_probabilities_from_release = std::valarray<double>(0.0, n_traps);

    // Set probabilities for each trap species
    for (int i_trap = 0; i_trap < n_traps; i_trap++) {
        // Common factors
        total_rate = traps[i_trap].capture_rate + traps[i_trap].release_rate;
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
            1.0 - traps[i_trap].release_rate * exponential_factor;

        // Resulting empty fraction from only release
        empty_probabilities_from_release[i_trap] =
            1.0 - exp(-traps[i_trap].release_rate * dwell_time);
    }
}

/*
    Call any necessary initialisation functions, etc.
*/
void TrapManagerSlowCapture::setup() {
    initialise_trap_states();
    set_fill_probabilities();
}

/*
    Same as TrapManagerInstantCapture
*/
double TrapManagerSlowCapture::n_electrons_released_from_wmk_above_cloud(int i_wmk) {
    
    //print_v(0,"SC child version of n_electrons_released_from_wmk_above_cloud %g \n",empty_probabilities_from_release[0]);;

    // Fraction of electrons released from each trap species
    double frac_released_this_wmk = 0.0;
    for (int i_trap = 0; i_trap < n_traps; i_trap++) {
        // Fraction of released electrons
        frac_released_this_wmk +=
            watermark_fills[i_wmk * n_traps + i_trap] *
            empty_probabilities_from_release[i_trap];
    }

    // Multiply by the watermark volume
    return frac_released_this_wmk * watermark_volumes[i_wmk];
}
/*
    Release and capture electrons and update the trap watermarks.

    The interaction between traps and the charge cloud during its dwell time in
    this pixel (and phase) is modelled as follows:
    + First any previously filled traps that are not within the cloud volume
        release charge.
    + This may update the number of free electrons in the cloud that are
        available for capture.
    + Then all traps within the cloud volume release and capture charge.
    + In the rare case that not enough electrons are available for capture,
        all traps within the cloud capture proportionally less charge to match.

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
        The updated watermarks.
*/
double TrapManagerSlowCapture::n_electrons_released_and_captured(
    double n_free_electrons) {
    // The fractional volume the electron cloud reaches in the pixel well
    double cloud_fractional_volume =
        ccd_phase.cloud_fractional_volume_from_electrons(n_free_electrons);

    int i_wmk_above_cloud = watermark_index_above_cloud(cloud_fractional_volume);

    // Add a new watermark at the cloud height
    if (cloud_fractional_volume > 0.0) {
        // First capture
        if (n_active_watermarks == 0) {
            // Set fractional volume
            watermark_volumes[0] = cloud_fractional_volume;

            // Update count of active watermarks
            n_active_watermarks++;
        }

        // Cloud above all current watermarks
        else if (i_wmk_above_cloud == i_first_active_wmk + n_active_watermarks) {
            double previous_total_volume = 0.0;
            for (int i_wmk = i_first_active_wmk; i_wmk <= i_wmk_above_cloud; i_wmk++) {
                previous_total_volume += watermark_volumes[i_wmk];
            }

            // Set fractional volume
            watermark_volumes[i_wmk_above_cloud] =
                cloud_fractional_volume - previous_total_volume;

            // Update count of active watermarks
            n_active_watermarks++;
        }

        // Cloud between or below current watermarks
        else {
            // Original total volume of the to-be-split watermark
            double previous_total_volume = 0.0;
            for (int i_wmk = i_first_active_wmk; i_wmk <= i_wmk_above_cloud; i_wmk++) {
                previous_total_volume += watermark_volumes[i_wmk];
            }

            // Copy-paste all higher watermarks up one to make room
            for (int i_wmk = i_first_active_wmk + n_active_watermarks - 1;
                 i_wmk >= i_wmk_above_cloud; i_wmk--) {
                watermark_volumes[i_wmk + 1] = watermark_volumes[i_wmk];
                for (int i_trap = 0; i_trap < n_traps; i_trap++) {
                    watermark_fills[(i_wmk + 1) * n_traps + i_trap] =
                        watermark_fills[i_wmk * n_traps + i_trap];
                }
            }

            // Update count of active watermarks
            n_active_watermarks++;
            i_wmk_above_cloud++;

            // New watermark
            watermark_volumes[i_wmk_above_cloud - 1] =
                watermark_volumes[i_wmk_above_cloud] -
                (previous_total_volume - cloud_fractional_volume);

            // Update fractional volume of the partially overwritten watermark
            watermark_volumes[i_wmk_above_cloud] =
                previous_total_volume - cloud_fractional_volume;
        }
    }

    // ========
    // Release electrons from any watermarks above the cloud
    // ========
    double n_released = 0.0;
    double frac_released_this_wmk;
    double frac_released_this_wmk_this_trap;
    double cumulative_volume = 0.0;
    double next_cumulative_volume = 0.0;

    // Count the released electrons and update the watermarks
    for (int i_wmk = i_wmk_above_cloud;
         i_wmk < i_first_active_wmk + n_active_watermarks; i_wmk++) {
        frac_released_this_wmk = 0.0;

        // Each trap species
        for (int i_trap = 0; i_trap < n_traps; i_trap++) {
            // Fraction of released electrons
            frac_released_this_wmk_this_trap = 
                watermark_fills[i_wmk * n_traps + i_trap] *
                empty_probabilities_from_release[i_trap];

            // Update the watermark fill fraction
            watermark_fills[i_wmk * n_traps + i_trap] -= 
                frac_released_this_wmk_this_trap;
            
            // Keep stock of newly free (fraction of) electrons
            frac_released_this_wmk += frac_released_this_wmk_this_trap;
        }

        // Multiply by the watermark volume
        n_released += frac_released_this_wmk * watermark_volumes[i_wmk];
    }

    // Update the electron cloud
    n_free_electrons += n_released;
    cloud_fractional_volume =
        ccd_phase.cloud_fractional_volume_from_electrons(n_free_electrons);
    i_wmk_above_cloud = watermark_index_above_cloud(cloud_fractional_volume);

    // ========
    // Capture and release electrons below the cloud
    // ========
    // No capture
    if (cloud_fractional_volume == 0.0) return 0.0;

    // Add a new watermark at the new cloud height
    if (n_released > 0.0) {
        // Original total volume of the watermark
        double previous_total_volume = 0.0;
        for (int i_wmk = i_first_active_wmk; i_wmk <= i_wmk_above_cloud; i_wmk++) {
            previous_total_volume += watermark_volumes[i_wmk];
        }

        // Copy-paste any higher watermarks up one to make room
        for (int i_wmk = i_first_active_wmk + n_active_watermarks - 1;
             i_wmk >= i_wmk_above_cloud; i_wmk--) {
            watermark_volumes[i_wmk + 1] = watermark_volumes[i_wmk];
            for (int i_trap = 0; i_trap < n_traps; i_trap++) {
                watermark_fills[(i_wmk + 1) * n_traps + i_trap] =
                    watermark_fills[i_wmk * n_traps + i_trap];
            }
        }

        // Update count of active watermarks
        n_active_watermarks++;
        i_wmk_above_cloud++;

        // New watermark
        watermark_volumes[i_wmk_above_cloud - 1] =
            watermark_volumes[i_wmk_above_cloud] -
            (previous_total_volume - cloud_fractional_volume);

        // Update fractional volume of the partially overwritten watermark
        watermark_volumes[i_wmk_above_cloud] =
            previous_total_volume - cloud_fractional_volume;
    }

    // Release and capture electrons in each watermark below the cloud
    double n_released_and_captured = 0.0;
    double n_released_and_captured_this_wmk = 0.0;
    double new_fill;
    for (int i_wmk = i_first_active_wmk; i_wmk < i_wmk_above_cloud; i_wmk++) {
        n_released_and_captured_this_wmk = 0.0;

        // Total volume at the bottom and top of this watermark
        cumulative_volume = next_cumulative_volume;
        next_cumulative_volume += watermark_volumes[i_wmk];

        // Each trap species
        for (int i_trap = 0; i_trap < n_traps; i_trap++) {
            // Fraction of full traps that remain full plus fraction of empty
            // traps that become full
            new_fill = fill_probabilities_from_full[i_trap] *
                           watermark_fills[i_wmk * n_traps + i_trap] +
                       fill_probabilities_from_empty[i_trap] *
                           (trap_densities[i_trap] -
                            watermark_fills[i_wmk * n_traps + i_trap]);

            // Net released minus captured electrons
            n_released_and_captured_this_wmk +=
                watermark_fills[i_wmk * n_traps + i_trap] - new_fill;
        }

        // Multiply by the watermark volume
        n_released_and_captured += n_released_and_captured_this_wmk *
                                   (next_cumulative_volume - cumulative_volume);
    }

    // ========
    // Update the watermarks
    // ========
    // Check enough available electrons to capture, if more captured than released
    double enough;
    if (n_released_and_captured < 0.0)
        enough = n_free_electrons / -n_released_and_captured;
    else
        enough = 1.0;

    // Update the watermarks for release and capture below the cloud
    for (int i_wmk = i_first_active_wmk; i_wmk < i_wmk_above_cloud; i_wmk++) {
        n_released_and_captured_this_wmk = 0.0;

        // Total volume at the bottom and top of this watermark
        cumulative_volume = next_cumulative_volume;
        next_cumulative_volume += watermark_volumes[i_wmk];

        // Each trap species
        for (int i_trap = 0; i_trap < n_traps; i_trap++) {
            // Fraction of full traps that remain full plus fraction of empty
            // traps that become full
            new_fill = fill_probabilities_from_full[i_trap] *
                           watermark_fills[i_wmk * n_traps + i_trap] +
                       fill_probabilities_from_empty[i_trap] *
                           (trap_densities[i_trap] -
                            watermark_fills[i_wmk * n_traps + i_trap]);

            // Modify for not-enough capture
            if (enough < 1.0) {
                new_fill = enough * new_fill +
                           (1.0 - enough) * watermark_fills[i_wmk * n_traps + i_trap];
            }

            // Update watermark fill fractions
            watermark_fills[i_wmk * n_traps + i_trap] = new_fill;
        }
    }

    if (enough < 1.0) n_released_and_captured *= enough;

    return n_released + n_released_and_captured;
}





// ========
// TrapManagerInstantCaptureContinuum::
// ========
/*
    Class TrapManagerInstantCaptureContinuum.

    For trap species with continous distributions of release timescales, and
    the standard release-then-instant-capture algorithm.

    For release, the watermark fill fractions are converted into the total time
    elapsed since the traps were filled in order to update them.

    Capture is identical to instant-capture traps.

    Attributes (in addition to TrapManagerBase)
    ----------
    time_min : double
    time_max : double
        The minimum and maximum elapsed times to set the interpolation table
        limits. See prep_fill_fraction_and_time_elapsed_tables().

    n_intp : int
        The number of interpolation values in the arrays. Currently set here
        manually. See prep_fill_fraction_and_time_elapsed_tables().
*/
TrapManagerInstantCaptureContinuum::TrapManagerInstantCaptureContinuum(
    std::valarray<TrapInstantCaptureContinuum> traps, int max_n_transfers,
    CCDPhase ccd_phase, double dwell_time)
    : TrapManagerBase(max_n_transfers, ccd_phase, dwell_time), traps(traps) {

    n_traps = traps.size();
    trap_densities = std::valarray<double>(n_traps);
    for (int i_trap = 0; i_trap < n_traps; i_trap++) {
        trap_densities[i_trap] = traps[i_trap].density;
    }

    time_min = dwell_time;
    time_max = max_n_transfers * dwell_time;
    n_intp = 1000;
}

/*
    Set the interpolation table values for converting between fill fractions
    and elapsed times.

    See TrapInstantCaptureContinuum.prep_fill_fraction_and_time_elapsed_tables().
*/
void TrapManagerInstantCaptureContinuum::prepare_interpolation_tables() {
    // Prepare interpolation tables for each trap species
    for (int i_trap = 0; i_trap < n_traps; i_trap++) {
        traps[i_trap].prep_fill_fraction_and_time_elapsed_tables(
            time_min, time_max, n_intp);
    }
}

/*
    Call any necessary initialisation functions, etc.
*/
void TrapManagerInstantCaptureContinuum::setup() {
    initialise_trap_states();
    prepare_interpolation_tables();
}

/*
    How many electrons will be released from a watermark above the cloud,
    during the next timestep.

    Parameters
    ----------
    cloud_fractional_volume : double
        The fractional volume the electron cloud reaches in the pixel well.

    Returns
    -------
    i_wmk_above_cloud : int
        The index of the first active watermark that reaches above the cloud.
*/
double TrapManagerInstantCaptureContinuum::n_electrons_released_from_wmk_above_cloud(int i_wmk) {
    
//    print_v(0,"ICCR child version of n_electrons_released_from_wmk_above_cloud %g \n",empty_probabilities_from_release[0]);;
//    return 0;
    // Fraction of electrons released from each trap species
    double frac_released_this_wmk = 0.0;
    double fill_initial;
    double time_initial;
    double fill_final;
    for (int i_trap = 0; i_trap < n_traps; i_trap++) {
        // Initial fill and conversion to elapsed time
        fill_initial = watermark_fills[i_wmk * n_traps + i_trap];
        time_initial = traps[i_trap].time_elapsed_from_fill_fraction_table(
            fill_initial / trap_densities[i_trap]);

        // New fill fraction from updated elapsed time
        fill_final = trap_densities[i_trap] *
            traps[i_trap].fill_fraction_from_time_elapsed_table(
                time_initial + dwell_time);

        // Number released from difference in fill fractions
        frac_released_this_wmk += fill_initial - fill_final;
    }

    // Multiply by the watermark volume
    return frac_released_this_wmk * watermark_volumes[i_wmk];

}
/*
    Same as TrapManagerInstantCapture, except for the conversion between fill
    fractions and elapsed times to update the trap states.
*/
double TrapManagerInstantCaptureContinuum::n_electrons_released() {
    double n_released = 0.0;
    double n_released_this_wmk;
    double fill_initial;
    double time_initial;

    // Each active watermark
    for (int i_wmk = i_first_active_wmk;
         i_wmk < i_first_active_wmk + n_active_watermarks; i_wmk++) {
        n_released_this_wmk = 0.0;

        // Each trap species
        for (int i_trap = 0; i_trap < n_traps; i_trap++) {
            // Initial fill and conversion to elapsed time
            fill_initial = watermark_fills[i_wmk * n_traps + i_trap];
            time_initial = traps[i_trap].time_elapsed_from_fill_fraction_table(
                fill_initial / trap_densities[i_trap]);

            // New fill fraction from updated elapsed time
            watermark_fills[i_wmk * n_traps + i_trap] =
                trap_densities[i_trap] *
                traps[i_trap].fill_fraction_from_time_elapsed_table(
                    time_initial + dwell_time);

            // Number released from difference in fill fractions
            n_released_this_wmk +=
                fill_initial - watermark_fills[i_wmk * n_traps + i_trap];
        }

        // Multiply by the watermark volume
        n_released += n_released_this_wmk * watermark_volumes[i_wmk];
    }

    return n_released;
}

/*
    Same as TrapManagerInstantCapture.
*/
void TrapManagerInstantCaptureContinuum::update_watermarks_capture(
    double cloud_fractional_volume, int i_wmk_above_cloud) {
    // First capture
    if (n_active_watermarks == 0) {
        // Set fractional volume
        watermark_volumes[0] = cloud_fractional_volume;

        // Set fill fractions for all trap species
        watermark_fills[std::slice(0, n_traps, 1)] = trap_densities;

        // Update count of active watermarks
        n_active_watermarks++;
    }

    // Cloud below all current watermarks
    else if (i_wmk_above_cloud == i_first_active_wmk) {
        // Make room for the new lowest watermark
        if (i_first_active_wmk > 0) {
            // Use existing room below the current first active watermark
            i_first_active_wmk--;
        } else {
            // Copy-paste all higher watermarks up one to make room
            for (int i_wmk = i_first_active_wmk + n_active_watermarks - 1;
                 i_wmk >= i_first_active_wmk; i_wmk--) {
                watermark_volumes[i_wmk + 1] = watermark_volumes[i_wmk];
                for (int i_trap = 0; i_trap < n_traps; i_trap++) {
                    watermark_fills[(i_wmk + 1) * n_traps + i_trap] =
                        watermark_fills[i_wmk * n_traps + i_trap];
                }
            }
        }

        // Update count of active watermarks
        n_active_watermarks++;

        // New watermark
        watermark_volumes[i_first_active_wmk] = cloud_fractional_volume;
        watermark_fills[std::slice(i_first_active_wmk * n_traps, n_traps, 1)] =
            trap_densities;

        // Update fractional volume of the partially overwritten watermark above
        watermark_volumes[i_first_active_wmk + 1] -= cloud_fractional_volume;
    }

    // Cloud above all current watermarks
    else if (i_wmk_above_cloud == i_first_active_wmk + n_active_watermarks) {
        // Skip all overwritten watermarks
        i_first_active_wmk = i_wmk_above_cloud - 1;

        // New first watermark
        watermark_volumes[i_first_active_wmk] = cloud_fractional_volume;
        watermark_fills[std::slice(i_first_active_wmk * n_traps, n_traps, 1)] =
            trap_densities;

        // Update count of active watermarks
        n_active_watermarks = 1;
    }

    // Cloud between current watermarks
    else {
        // Update fractional volume of the partially overwritten watermark
        double previous_total_volume = 0.0;
        for (int i_wmk = i_first_active_wmk; i_wmk <= i_wmk_above_cloud; i_wmk++) {
            previous_total_volume += watermark_volumes[i_wmk];
        }
        watermark_volumes[i_wmk_above_cloud] =
            previous_total_volume - cloud_fractional_volume;

        // Update count of active watermarks
        n_active_watermarks += i_first_active_wmk - i_wmk_above_cloud + 1;

        // Skip all overwritten watermarks
        i_first_active_wmk = i_wmk_above_cloud - 1;

        // New first watermark
        watermark_volumes[i_first_active_wmk] = cloud_fractional_volume;
        watermark_fills[std::slice(i_first_active_wmk * n_traps, n_traps, 1)] =
            trap_densities;
    }

    return;
}

/*
    Same as TrapManagerInstantCapture.
*/
void TrapManagerInstantCaptureContinuum::update_watermarks_capture_not_enough(
    double cloud_fractional_volume, int i_wmk_above_cloud, double enough) {
    // First capture
    if (n_active_watermarks == 0) {
        // Set fractional volume
        watermark_volumes[0] = cloud_fractional_volume;

        // Set fill fractions for all trap species
        watermark_fills[std::slice(0, n_traps, 1)] = trap_densities * enough;

        // Update count of active watermarks
        n_active_watermarks++;
    }

    // Cloud below all current watermarks
    else if (i_wmk_above_cloud == i_first_active_wmk) {
        // Make room for the new lowest watermark
        if (i_first_active_wmk > 0) {
            // Use existing room below the current first active watermark
            i_first_active_wmk--;
        } else {
            // Copy-paste all higher watermarks up one to make room
            for (int i_wmk = i_first_active_wmk + n_active_watermarks - 1;
                 i_wmk >= i_first_active_wmk; i_wmk--) {
                watermark_volumes[i_wmk + 1] = watermark_volumes[i_wmk];
                for (int i_trap = 0; i_trap < n_traps; i_trap++) {
                    watermark_fills[(i_wmk + 1) * n_traps + i_trap] =
                        watermark_fills[i_wmk * n_traps + i_trap];
                }
            }
        }

        // Update count of active watermarks
        n_active_watermarks++;

        // New watermark
        watermark_volumes[i_first_active_wmk] = cloud_fractional_volume;
        watermark_fills[std::slice(i_first_active_wmk * n_traps, n_traps, 1)] =
            (std::valarray<double>)watermark_fills[std::slice(
                i_first_active_wmk * n_traps, n_traps, 1)] *
                (1.0 - enough) +
            enough * trap_densities;

        // Update fractional volume of the partially overwritten watermark above
        watermark_volumes[i_first_active_wmk + 1] -= cloud_fractional_volume;
    }

    // Cloud above all current watermarks
    else if (i_wmk_above_cloud == i_first_active_wmk + n_active_watermarks) {
        // Cumulative volume of the watermark just below the new one
        double volume_below = 0.0;
        for (int i_wmk = i_first_active_wmk; i_wmk < i_wmk_above_cloud; i_wmk++) {
            volume_below += watermark_volumes[i_wmk];
        }

        // New watermark
        watermark_volumes[i_wmk_above_cloud] = cloud_fractional_volume - volume_below;
        watermark_fills[std::slice(
            (i_first_active_wmk + n_active_watermarks) * n_traps, n_traps, 1)] =
            enough * trap_densities;

        // Update all other watermarks part-way to full
        for (int i_wmk = i_first_active_wmk;
             i_wmk < i_first_active_wmk + n_active_watermarks; i_wmk++) {
            watermark_fills[std::slice(i_wmk * n_traps, n_traps, 1)] =
                (std::valarray<double>)
                        watermark_fills[std::slice(i_wmk * n_traps, n_traps, 1)] *
                    (1.0 - enough) +
                enough * trap_densities;
        }

        // Update count of active watermarks
        n_active_watermarks++;
    }

    // Cloud between current watermarks
    else {
        // Copy-paste all higher watermarks up one to make room
        for (int i_wmk = i_first_active_wmk + n_active_watermarks - 1;
             i_wmk >= i_wmk_above_cloud; i_wmk--) {
            watermark_volumes[i_wmk + 1] = watermark_volumes[i_wmk];
            for (int i_trap = 0; i_trap < n_traps; i_trap++) {
                watermark_fills[(i_wmk + 1) * n_traps + i_trap] =
                    watermark_fills[i_wmk * n_traps + i_trap];
            }
        }

        // Cumulative volume of the watermark just below the new one
        double volume_below = 0.0;
        for (int i_wmk = i_first_active_wmk; i_wmk < i_wmk_above_cloud; i_wmk++) {
            volume_below += watermark_volumes[i_wmk];
        }

        // New watermark
        watermark_volumes[i_wmk_above_cloud] = cloud_fractional_volume - volume_below;

        // Update volume of the partially overwritten watermark
        watermark_volumes[i_wmk_above_cloud + 1] -=
            watermark_volumes[i_wmk_above_cloud];

        // Update all watermarks, including the new one, part-way to full
        for (int i_wmk = i_first_active_wmk; i_wmk <= i_wmk_above_cloud; i_wmk++) {
            watermark_fills[std::slice(i_wmk * n_traps, n_traps, 1)] =
                (std::valarray<double>)
                        watermark_fills[std::slice(i_wmk * n_traps, n_traps, 1)] *
                    (1.0 - enough) +
                enough * trap_densities;
        }

        // Update count of active watermarks
        n_active_watermarks++;
    }

    return;
}

/*
    Same as TrapManagerInstantCapture.
*/
double TrapManagerInstantCaptureContinuum::n_electrons_captured(
    double n_free_electrons) {
    // The fractional volume the electron cloud reaches in the pixel well
    double cloud_fractional_volume =
        ccd_phase.cloud_fractional_volume_from_electrons(n_free_electrons);

    // No capture
    if (cloud_fractional_volume == 0.0) return 0.0;

    // ========
    // Count the number of electrons that can be captured by each watermark
    // ========
    double n_captured = 0.0;
    double n_captured_this_wmk;
    double cumulative_volume = 0.0;
    double next_cumulative_volume = 0.0;

    int i_wmk_above_cloud = watermark_index_above_cloud(cloud_fractional_volume);

    // Each active watermark
    for (int i_wmk = i_first_active_wmk; i_wmk <= i_wmk_above_cloud; i_wmk++) {
        n_captured_this_wmk = 0.0;

        // Total volume at the bottom and top of this watermark
        cumulative_volume = next_cumulative_volume;
        next_cumulative_volume += watermark_volumes[i_wmk];

        // Each trap species
        for (int i_trap = 0; i_trap < n_traps; i_trap++) {
            n_captured_this_wmk +=
                trap_densities[i_trap] - watermark_fills[i_wmk * n_traps + i_trap];
        }

        // Capture from the bottom of the last watermark up to the cloud volume
        if (i_wmk == i_wmk_above_cloud) {
            n_captured +=
                n_captured_this_wmk * (cloud_fractional_volume - cumulative_volume);
        }
        // Capture from the bottom to top of watermark volumes below the cloud
        else {
            n_captured +=
                n_captured_this_wmk * (next_cumulative_volume - cumulative_volume);
        }
    }

    // ========
    // Update the watermarks
    // ========
    // Check enough available electrons to capture
    double enough = n_free_electrons / n_captured;

    // Normal full capture
    if (enough >= 1.0) {
        update_watermarks_capture(cloud_fractional_volume, i_wmk_above_cloud);
    }
    // Partial capture
    else {
        update_watermarks_capture_not_enough(
            cloud_fractional_volume, i_wmk_above_cloud, enough);

        n_captured *= enough;
    }

    return n_captured;
}

/*
    Same as TrapManagerInstantCapture.
*/
double TrapManagerInstantCaptureContinuum::n_electrons_released_and_captured(
    double n_free_electrons) {

    double n_released = n_electrons_released();
    print_v(2, "n_electrons_released  %g \n", n_released);

    double n_captured = n_electrons_captured(n_free_electrons + n_released);
    print_v(2, "n_electrons_captured  %g \n", n_captured);

    return n_released - n_captured;
}




// ========
// TrapManagerSlowCaptureContinuum::
// ========
/*
    Class TrapManagerSlowCaptureContinuum.

    For traps with a non-instant capture time and a continuum (log-normal
    distribution) of release timescales.

    Attributes (where different to TrapManagerInstantCaptureContinuum)
    ----------
    time_min : double
        Unlike instant capture, need a much smaller minimum time so that the
        interpolation tables cover the not-completely-filled traps from capture.
*/
TrapManagerSlowCaptureContinuum::TrapManagerSlowCaptureContinuum(
    std::valarray<TrapSlowCaptureContinuum> traps, int max_n_transfers,
    CCDPhase ccd_phase, double dwell_time)
    : TrapManagerBase(max_n_transfers, ccd_phase, dwell_time), traps(traps) {

    n_traps = traps.size();
    trap_densities = std::valarray<double>(n_traps);
    for (int i_trap = 0; i_trap < n_traps; i_trap++) {
        trap_densities[i_trap] = traps[i_trap].density;
    }

    time_min = dwell_time / 30;
    time_max = max_n_transfers * dwell_time;
    n_intp = 1000;

    // Overwrite default parameter values
    n_watermarks_per_transfer = 2;
}

/*
    Set the interpolation table values for converting between fill fractions
    and elapsed times.

    See TrapManagerSlowCaptureContinuum.prep_fill_fraction_and_time_elapsed_tables()
    and prep_fill_fraction_after_slow_capture_tables().
*/
void TrapManagerSlowCaptureContinuum::prepare_interpolation_tables() {
    // Prepare interpolation tables for each trap species
    for (int i_trap = 0; i_trap < n_traps; i_trap++) {
        traps[i_trap].prep_fill_fraction_and_time_elapsed_tables(
            time_min, time_max, n_intp);
        traps[i_trap].prep_fill_fraction_after_slow_capture_tables(
            dwell_time, time_min, time_max, n_intp);
    }
}

/*
    Call any necessary initialisation functions, etc.
*/
void TrapManagerSlowCaptureContinuum::setup() {
    initialise_trap_states();
    prepare_interpolation_tables();
}

/*
    Same as InstantCaptureContinuum
*/
double TrapManagerSlowCaptureContinuum::n_electrons_released_from_wmk_above_cloud(int i_wmk) {
    
    //print_v(0,"SCCR child version of n_electrons_released_from_wmk_above_cloud %g \n",empty_probabilities_from_release[0]);;
    
    // Fraction of electrons released from each trap species
    double frac_released_this_wmk = 0.0;
    double fill_initial;
    double time_initial;
    double fill_final;
    for (int i_trap = 0; i_trap < n_traps; i_trap++) {
        // Initial fill and conversion to elapsed time
        fill_initial = watermark_fills[i_wmk * n_traps + i_trap];
        time_initial = traps[i_trap].time_elapsed_from_fill_fraction_table(
            fill_initial / trap_densities[i_trap]);

        // New fill fraction from updated elapsed time
        fill_final = trap_densities[i_trap] *
            traps[i_trap].fill_fraction_from_time_elapsed_table(
                time_initial + dwell_time);

        // Number released from difference in fill fractions
        frac_released_this_wmk += fill_initial - fill_final;
    }

    // Multiply by the watermark volume
    return frac_released_this_wmk * watermark_volumes[i_wmk];
}
/*
    Same as TrapManagerSlowCapture, except for the conversion between fill
    fractions and elapsed times to update the trap states.
*/
double TrapManagerSlowCaptureContinuum::n_electrons_released_and_captured(
    double n_free_electrons) {
    // The fractional volume the electron cloud reaches in the pixel well
    double cloud_fractional_volume =
        ccd_phase.cloud_fractional_volume_from_electrons(n_free_electrons);

    int i_wmk_above_cloud = watermark_index_above_cloud(cloud_fractional_volume);

    // Add a new watermark at the cloud height
    if (cloud_fractional_volume > 0.0) {
        // First capture
        if (n_active_watermarks == 0) {
            // Set fractional volume
            watermark_volumes[0] = cloud_fractional_volume;

            // Update count of active watermarks
            n_active_watermarks++;
        }

        // Cloud above all current watermarks
        else if (i_wmk_above_cloud == i_first_active_wmk + n_active_watermarks) {
            double previous_total_volume = 0.0;
            for (int i_wmk = i_first_active_wmk; i_wmk <= i_wmk_above_cloud; i_wmk++) {
                previous_total_volume += watermark_volumes[i_wmk];
            }

            // Set fractional volume
            watermark_volumes[i_wmk_above_cloud] =
                cloud_fractional_volume - previous_total_volume;

            // Update count of active watermarks
            n_active_watermarks++;
        }

        // Cloud between or below current watermarks
        else {
            // Original total volume of the to-be-split watermark
            double previous_total_volume = 0.0;
            for (int i_wmk = i_first_active_wmk; i_wmk <= i_wmk_above_cloud; i_wmk++) {
                previous_total_volume += watermark_volumes[i_wmk];
            }

            // Copy-paste all higher watermarks up one to make room
            for (int i_wmk = i_first_active_wmk + n_active_watermarks - 1;
                 i_wmk >= i_wmk_above_cloud; i_wmk--) {
                watermark_volumes[i_wmk + 1] = watermark_volumes[i_wmk];
                for (int i_trap = 0; i_trap < n_traps; i_trap++) {
                    watermark_fills[(i_wmk + 1) * n_traps + i_trap] =
                        watermark_fills[i_wmk * n_traps + i_trap];
                }
            }

            // Update count of active watermarks
            n_active_watermarks++;
            i_wmk_above_cloud++;

            // New watermark
            watermark_volumes[i_wmk_above_cloud - 1] =
                watermark_volumes[i_wmk_above_cloud] -
                (previous_total_volume - cloud_fractional_volume);

            // Update fractional volume of the partially overwritten watermark
            watermark_volumes[i_wmk_above_cloud] =
                previous_total_volume - cloud_fractional_volume;
        }
    }

    // ========
    // Release electrons from any watermarks above the cloud
    // ========
    double n_released = 0.0;
    double n_released_this_wmk = 0.0;
    double fill_initial;
    double time_initial;
    double cumulative_volume = 0.0;
    double next_cumulative_volume = 0.0;

    // Count the released electrons and update the watermarks
    // Same as TrapManagerInstantCaptureContinuum.n_electrons_released()
    for (int i_wmk = i_wmk_above_cloud;
         i_wmk < i_first_active_wmk + n_active_watermarks; i_wmk++) {
        n_released_this_wmk = 0.0;

        // Each trap species
        for (int i_trap = 0; i_trap < n_traps; i_trap++) {
            // Initial fill and conversion to elapsed time
            fill_initial = watermark_fills[i_wmk * n_traps + i_trap];
            time_initial = traps[i_trap].time_elapsed_from_fill_fraction_table(
                fill_initial / trap_densities[i_trap]);

            // New fill fraction from updated elapsed time
            watermark_fills[i_wmk * n_traps + i_trap] =
                trap_densities[i_trap] *
                traps[i_trap].fill_fraction_from_time_elapsed_table(
                    time_initial + dwell_time);

            // Number released from difference in fill fractions
            n_released_this_wmk +=
                fill_initial - watermark_fills[i_wmk * n_traps + i_trap];
        }

        // Multiply by the watermark volume
        n_released += n_released_this_wmk * watermark_volumes[i_wmk];
    }

    // Update the electron cloud
    n_free_electrons += n_released;
    cloud_fractional_volume =
        ccd_phase.cloud_fractional_volume_from_electrons(n_free_electrons);
    i_wmk_above_cloud = watermark_index_above_cloud(cloud_fractional_volume);

    // ========
    // Capture and release electrons below the cloud
    // ========
    // No capture
    if (cloud_fractional_volume == 0.0) return 0.0;

    // Add a new watermark at the new cloud height
    if (n_released > 0.0) {
        // Original total volume of the watermark
        double previous_total_volume = 0.0;
        for (int i_wmk = i_first_active_wmk; i_wmk <= i_wmk_above_cloud; i_wmk++) {
            previous_total_volume += watermark_volumes[i_wmk];
        }

        // Copy-paste any higher watermarks up one to make room
        for (int i_wmk = i_first_active_wmk + n_active_watermarks - 1;
             i_wmk >= i_wmk_above_cloud; i_wmk--) {
            watermark_volumes[i_wmk + 1] = watermark_volumes[i_wmk];
            for (int i_trap = 0; i_trap < n_traps; i_trap++) {
                watermark_fills[(i_wmk + 1) * n_traps + i_trap] =
                    watermark_fills[i_wmk * n_traps + i_trap];
            }
        }

        // Update count of active watermarks
        n_active_watermarks++;
        i_wmk_above_cloud++;

        // New watermark
        watermark_volumes[i_wmk_above_cloud - 1] =
            watermark_volumes[i_wmk_above_cloud] -
            (previous_total_volume - cloud_fractional_volume);

        // Update fractional volume of the partially overwritten watermark
        watermark_volumes[i_wmk_above_cloud] =
            previous_total_volume - cloud_fractional_volume;
    }

    // Release and capture electrons in each watermark below the cloud
    double n_released_and_captured = 0.0;
    double n_released_and_captured_this_wmk = 0.0;
    double new_fill;
    for (int i_wmk = i_first_active_wmk; i_wmk < i_wmk_above_cloud; i_wmk++) {
        n_released_and_captured_this_wmk = 0.0;

        // Total volume at the bottom and top of this watermark
        cumulative_volume = next_cumulative_volume;
        next_cumulative_volume += watermark_volumes[i_wmk];

        // Each trap species
        for (int i_trap = 0; i_trap < n_traps; i_trap++) {
            // Initial fill and conversion to elapsed time
            fill_initial = watermark_fills[i_wmk * n_traps + i_trap];
            time_initial = traps[i_trap].time_elapsed_from_fill_fraction_table(
                fill_initial / trap_densities[i_trap]);

            // Integrate over the fraction of full traps that remain full plus
            // fraction of empty traps that become full over the distribution
            new_fill =
                traps[i_trap].fill_fraction_after_slow_capture_table(time_initial);

            // Include the trap density
            new_fill *= trap_densities[i_trap];

            // Net released minus captured electrons
            n_released_and_captured_this_wmk +=
                watermark_fills[i_wmk * n_traps + i_trap] - new_fill;
        }

        // Multiply by the watermark volume
        n_released_and_captured += n_released_and_captured_this_wmk *
                                   (next_cumulative_volume - cumulative_volume);
    }

    // ========
    // Update the watermarks
    // ========
    // Check enough available electrons to capture, if more captured than released
    double enough;
    if (n_released_and_captured < 0.0)
        enough = n_free_electrons / -n_released_and_captured;
    else
        enough = 1.0;

    // Update the watermarks for release and capture below the cloud
    for (int i_wmk = i_first_active_wmk; i_wmk < i_wmk_above_cloud; i_wmk++) {
        n_released_and_captured_this_wmk = 0.0;

        // Total volume at the bottom and top of this watermark
        cumulative_volume = next_cumulative_volume;
        next_cumulative_volume += watermark_volumes[i_wmk];

        // Each trap species
        for (int i_trap = 0; i_trap < n_traps; i_trap++) {
            // Initial fill and conversion to elapsed time
            fill_initial = watermark_fills[i_wmk * n_traps + i_trap];
            time_initial = traps[i_trap].time_elapsed_from_fill_fraction_table(
                fill_initial / trap_densities[i_trap]);

            // Integrate over the fraction of full traps that remain full plus
            // fraction of empty traps that become full over the distribution
            new_fill =
                traps[i_trap].fill_fraction_after_slow_capture_table(time_initial);

            // Include the trap density
            new_fill *= trap_densities[i_trap];

            // Modify for not-enough capture
            if (enough < 1.0) {
                new_fill = enough * new_fill +
                           (1.0 - enough) * watermark_fills[i_wmk * n_traps + i_trap];
            }

            // Update watermark fill fractions
            watermark_fills[i_wmk * n_traps + i_trap] = new_fill;
        }
    }

    if (enough < 1.0) n_released_and_captured *= enough;

    return n_released + n_released_and_captured;
}

// ========
// TrapManagerManager::
// ========
/*
    Class TrapManagerManager.

    Handles the one or multiple trap managers required for models with a mix of
    trap species and/or multiphase clocking.

    Each individual trap manager tracks the states of one or more trap species.
    A different trap manager is required for any traps of different types, and
    separate trap managers are also required for each phase in multiphase
    clocking, which corresponds to an independent population of traps.

    On declaration, automatically creates the trap managers for each phase
    and/or watermark type and initialises their watermark arrays. The trap
    managers of each type are held in the trap_managers_* arrays, each
    containing one manager of that type for each phase.

    Also, if relevant, modifies the trap managers' trap densities to account for
    the fraction of traps in different CCD pixel phases.

    Parameters
    ----------
    traps_ic : std::valarray<TrapInstantCapture>
    traps_sc : std::valarray<TrapSlowCapture>
    traps_ic_co : std::valarray<TrapInstantCaptureContinuum>
    traps_sc_co : std::valarray<TrapSlowCaptureContinuum>
        The arrays of trap species, one for each type (which can be empty).

    max_n_transfers : int
        Same as TrapManagerBase. Is modified internally to include any extra
        intra-pixel transfers for multiphase clocking.

    ccd : CCD
        Parameters to describe how electrons fill the volume inside (all phases
        of) a pixel in a CCD detector.

    dwell_times : std::valarray<double>
        The time between steps in the clocking sequence, as stored by an ROE
        object.

        Note: currently assumes the dwell time in each phase is the same for all
        steps, which might not be true in sequences with n_steps > n_phases.

    Attributes
    ----------
    n_traps_ic : int
    n_traps_sc : int
    n_traps_ic_co : int
    n_traps_sc_co : int
        The number of trap species (if any) of each watermark type.

    trap_managers_ic : std::valarray<TrapManagerInstantCapture>
    trap_managers_sc : std::valarray<TrapManagerSlowCapture>
    trap_managers_ic_co : std::valarray<TrapManagerInstantCaptureContinuum>
    trap_managers_sc_co :
   std::valarray<TrapManagerSlowCaptureContinuum> For each watermark type, the list of
   trap manager objects for each phase. Ignored if the corresponding n_*_traps is 0.
*/
TrapManagerManager::TrapManagerManager(
    std::valarray<TrapInstantCapture>& traps_ic,
    std::valarray<TrapSlowCapture>& traps_sc,
    std::valarray<TrapInstantCaptureContinuum>& traps_ic_co,
    std::valarray<TrapSlowCaptureContinuum>& traps_sc_co, int max_n_transfers, CCD ccd,
    std::valarray<double>& dwell_times)
    : traps_ic(traps_ic),
      traps_sc(traps_sc),
      traps_ic_co(traps_ic_co),
      traps_sc_co(traps_sc_co),
      max_n_transfers(max_n_transfers),
      ccd(ccd) {

    // The number of trap species (if any) of each watermark type
    n_traps_ic = traps_ic.size();
    n_traps_sc = traps_sc.size();
    n_traps_ic_co = traps_ic_co.size();
    n_traps_sc_co = traps_sc_co.size();

    // Account for the number of clock-sequence steps for the maximum transfers
    max_n_transfers *= dwell_times.size();
    this->max_n_transfers = max_n_transfers;

    // ========
    // Set up the trap manager for each phase, for each watermark type
    // ========
    if (n_traps_ic > 0) {
        trap_managers_ic.resize(ccd.n_phases);

        // Initialise manager and watermarks for each phase
        for (int phase_index = 0; phase_index < ccd.n_phases; phase_index++) {
            trap_managers_ic[phase_index] = TrapManagerInstantCapture(
                traps_ic, max_n_transfers, ccd.phases[phase_index],
                dwell_times[phase_index]);

            // Modify the trap densities in different phases
            trap_managers_ic[phase_index].trap_densities *=
                ccd.fraction_of_traps_per_phase[phase_index];

            trap_managers_ic[phase_index].setup();
        }
    }

    if (n_traps_sc > 0) {
        trap_managers_sc.resize(ccd.n_phases);

        // Initialise manager and watermarks for each phase
        for (int phase_index = 0; phase_index < ccd.n_phases; phase_index++) {
            trap_managers_sc[phase_index] = TrapManagerSlowCapture(
                traps_sc, max_n_transfers, ccd.phases[phase_index],
                dwell_times[phase_index]);

            // Modify the trap densities in different phases
            trap_managers_sc[phase_index].trap_densities *=
                ccd.fraction_of_traps_per_phase[phase_index];

            trap_managers_sc[phase_index].setup();
        }
    }

    if (n_traps_ic_co > 0) {
        trap_managers_ic_co.resize(ccd.n_phases);

        // Initialise manager and watermarks for each phase
        for (int phase_index = 0; phase_index < ccd.n_phases; phase_index++) {
            trap_managers_ic_co[phase_index] = TrapManagerInstantCaptureContinuum(
                traps_ic_co, max_n_transfers, ccd.phases[phase_index],
                dwell_times[phase_index]);

            // Modify the trap densities in different phases
            trap_managers_ic_co[phase_index].trap_densities *=
                ccd.fraction_of_traps_per_phase[phase_index];

            trap_managers_ic_co[phase_index].setup();
        }
    }

    if (n_traps_sc_co > 0) {
        trap_managers_sc_co.resize(ccd.n_phases);

        // Initialise manager and watermarks for each phase
        for (int phase_index = 0; phase_index < ccd.n_phases; phase_index++) {
            trap_managers_sc_co[phase_index] = TrapManagerSlowCaptureContinuum(
                traps_sc_co, max_n_transfers, ccd.phases[phase_index],
                dwell_times[phase_index]);

            // Modify the trap densities in different phases
            trap_managers_sc_co[phase_index].trap_densities *=
                ccd.fraction_of_traps_per_phase[phase_index];

            trap_managers_sc_co[phase_index].setup();
        }
    }
}

/*
    Reset the watermark arrays to empty, for all trap managers.
*/
void TrapManagerManager::reset_trap_states() {
    if (n_traps_ic > 0)
        for (int phase_index = 0; phase_index < ccd.n_phases; phase_index++) {
            trap_managers_ic[phase_index].reset_trap_states();
        }
    if (n_traps_sc > 0)
        for (int phase_index = 0; phase_index < ccd.n_phases; phase_index++) {
            trap_managers_sc[phase_index].reset_trap_states();
        }
    if (n_traps_ic_co > 0)
        for (int phase_index = 0; phase_index < ccd.n_phases; phase_index++) {
            trap_managers_ic_co[phase_index].reset_trap_states();
        }
    if (n_traps_sc_co > 0)
        for (int phase_index = 0; phase_index < ccd.n_phases; phase_index++) {
            trap_managers_sc_co[phase_index].reset_trap_states();
        }
}

/*
    Store the watermark arrays to be loaded again later, for all trap managers.
*/
void TrapManagerManager::store_trap_states() {
    if (n_traps_ic > 0)
        for (int phase_index = 0; phase_index < ccd.n_phases; phase_index++) {
            trap_managers_ic[phase_index].store_trap_states();
        }
    if (n_traps_sc > 0)
        for (int phase_index = 0; phase_index < ccd.n_phases; phase_index++) {
            trap_managers_sc[phase_index].store_trap_states();
        }
    if (n_traps_ic_co > 0)
        for (int phase_index = 0; phase_index < ccd.n_phases; phase_index++) {
            trap_managers_ic_co[phase_index].store_trap_states();
        }
    if (n_traps_sc_co > 0)
        for (int phase_index = 0; phase_index < ccd.n_phases; phase_index++) {
            trap_managers_sc_co[phase_index].store_trap_states();
        }
}

/*
    Restore the watermark arrays to their saved values, for all trap managers.
*/
void TrapManagerManager::restore_trap_states() {
    if (n_traps_ic > 0)
        for (int phase_index = 0; phase_index < ccd.n_phases; phase_index++) {
            trap_managers_ic[phase_index].restore_trap_states();
        }
    if (n_traps_sc > 0)
        for (int phase_index = 0; phase_index < ccd.n_phases; phase_index++) {
            trap_managers_sc[phase_index].restore_trap_states();
        }
    if (n_traps_ic_co > 0)
        for (int phase_index = 0; phase_index < ccd.n_phases; phase_index++) {
            trap_managers_ic_co[phase_index].restore_trap_states();
        }
    if (n_traps_sc_co > 0)
        for (int phase_index = 0; phase_index < ccd.n_phases; phase_index++) {
            trap_managers_sc_co[phase_index].restore_trap_states();
        }
}

/*
    Prune redundant watermarks from watermark arrays, for all trap managers.
*/
void TrapManagerManager::prune_watermarks(double min_n_electrons) {
    //print_v(0,"IC traps\n");
    if (n_traps_ic > 0)
        for (int phase_index = 0; phase_index < ccd.n_phases; phase_index++) {
            trap_managers_ic[phase_index].prune_watermarks(min_n_electrons);
        }
    //print_v(0,"SC traps\n");
    if (n_traps_sc > 0)
        for (int phase_index = 0; phase_index < ccd.n_phases; phase_index++) {
            trap_managers_sc[phase_index].prune_watermarks(min_n_electrons);
        }
    //print_v(0,"ICCR traps\n");
    if (n_traps_ic_co > 0)
        for (int phase_index = 0; phase_index < ccd.n_phases; phase_index++) {
            trap_managers_ic_co[phase_index].prune_watermarks(min_n_electrons);
        }
    //print_v(0,"SCCR traps\n");
    if (n_traps_sc_co > 0)
        for (int phase_index = 0; phase_index < ccd.n_phases; phase_index++) {
            trap_managers_sc_co[phase_index].prune_watermarks(min_n_electrons);
        }
}

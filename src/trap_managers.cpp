
#include <math.h>
#include <stdio.h>
#include <valarray>

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

    ccd_phase : CCDPhase
        Parameters to describe how electrons fill the volume inside (one phase
        of) a pixel in a CCD detector.

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

    i_first_active_wmk : int
        The index of the first active watermark. The effective starting point
        for the active region of the watermark arrays. i.e. watermark levels
        below this are ignored because they've been superseded.

    n_active_watermarks : int
        The number of currently active watermark levels. So the index of the
        last active watermark is (i_first_active_wmk + n_active_watermarks - 1).

    n_watermarks_per_transfer : int
        The number of new watermarks that could be made in each transfer.

    n_watermarks : int
        The total number of available watermark levels, determined by the number
        of potential watermark-creating transfers and the watermarking scheme.

    empty_watermark : double
        The watermark value corresponding to empty traps.
*/
TrapManager::TrapManager(
    std::valarray<Trap> traps, int max_n_transfers, CCDPhase ccd_phase)
    : traps(traps), max_n_transfers(max_n_transfers), ccd_phase(ccd_phase) {

    n_traps = traps.size();
    n_watermarks_per_transfer = 2;
    empty_watermark = 0.0;
    n_active_watermarks = 0;
    i_first_active_wmk = 0;
    trap_densities = std::valarray<double>(n_traps);
    for (int i_trap = 0; i_trap < n_traps; i_trap++) {
        trap_densities[i_trap] = traps[i_trap].density;
    }
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
    i_first_active_wmk = 0;
    watermark_volumes = std::valarray<double>(empty_watermark, n_watermarks);
    watermark_fills = std::valarray<double>(empty_watermark, n_traps * n_watermarks);
}

/*
    Store the watermark arrays to be loaded again later.
*/
void TrapManager::store_trap_states() {
    stored_n_active_watermarks = n_active_watermarks;
    stored_i_first_active_wmk = i_first_active_wmk;
    stored_watermark_volumes = watermark_volumes;
    stored_watermark_fills = watermark_fills;
}

/*
    Restore the watermark arrays to their saved values.
*/
void TrapManager::restore_trap_states() {
    n_active_watermarks = stored_n_active_watermarks;
    i_first_active_wmk = stored_i_first_active_wmk;
    watermark_volumes = stored_watermark_volumes;
    watermark_fills = stored_watermark_fills;
}

/*
    Set the probabilities of traps being full after release and/or capture.

    See Lindegren (1998) section 3.2.

    Parameters
    ----------
    dwell_time : double
        The time spent in this pixel or phase, in the same units as the trap
        timescales.

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

    ##Make sure no copies of the arrays are made, either with pointers or
      just using the class attributes instead of arguments.

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

    // No watermarks
    if (n_active_watermarks == 0) return 0.0;

    double n_trapped_electrons = 0.0;
    std::valarray<double> n_trapped_electrons_this_wmk(n_traps);

    // Each active watermark
    for (int i_wmk = i_first_active_wmk;
         i_wmk < i_first_active_wmk + n_active_watermarks; i_wmk++) {
        // Extract the fill fractions in a 1D array
        n_trapped_electrons_this_wmk =
            wmk_fills[std::slice(i_wmk * n_traps, n_traps, 1)];

        // Sum the fill fractions and multiply by the fractional volume
        n_trapped_electrons += n_trapped_electrons_this_wmk.sum() * wmk_volumes[i_wmk];
    }

    return n_trapped_electrons;
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
int TrapManager::watermark_index_above_cloud(double cloud_fractional_volume) {
    double cumulative_volume = 0.0;

    // Sum up the fractional volumes until surpassing the cloud volume
    for (int i_wmk = i_first_active_wmk;
         i_wmk < i_first_active_wmk + n_active_watermarks; i_wmk++) {
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
    std::valarray<Trap> traps, int max_n_transfers, CCDPhase ccd_phase)
    : TrapManager(traps, max_n_transfers, ccd_phase) {

    // Overwrite default parameter values
    n_watermarks_per_transfer = 1;
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
        The updated watermarks. See TrapManager().
*/
double TrapManagerInstantCapture::n_electrons_released() {
    double n_released = 0.0;
    double n_released_this_wmk;
    double frac_released;

    // Each active watermark
    for (int i_wmk = i_first_active_wmk;
         i_wmk < i_first_active_wmk + n_active_watermarks; i_wmk++) {
        n_released_this_wmk = 0.0;

        // Each trap species
        for (int i_trap = 0; i_trap < n_traps; i_trap++) {
            // Fraction of released electrons
            frac_released = watermark_fills[i_wmk * n_traps + i_trap] *
                            empty_probabilities_from_release[i_trap];
            n_released_this_wmk += frac_released;

            // Update the watermark fill fraction
            watermark_fills[i_wmk * n_traps + i_trap] -= frac_released;
        }

        // Multiply by the watermark volume
        n_released += n_released_this_wmk * watermark_volumes[i_wmk];
    }

    return n_released;
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
        The updated watermarks. See TrapManager().
*/
void TrapManagerInstantCapture::update_watermarks_capture(
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
            for (int i_wmk = i_first_active_wmk + n_active_watermarks;
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
        The updated watermarks. See TrapManager().
*/
void TrapManagerInstantCapture::update_watermarks_capture_not_enough(
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
            for (int i_wmk = i_first_active_wmk + n_active_watermarks;
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
        for (int i_wmk =
                 i_wmk_above_cloud - 1 + n_active_watermarks - i_first_active_wmk;
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
    // The fractional volume the electron cloud reaches in the pixel well
    double cloud_fractional_volume =
        ccd_phase.cloud_fractional_volume_from_electrons(n_free_electrons);

    // No capture
    if (cloud_fractional_volume == 0.0) return 0.0;

    // ========
    // Count the number of electrons that can be captured by each watermark
    // ========
    double n_captured = 0.0;
    double n_captured_this_wmk = 0.0;
    double cumulative_volume = 0.0;
    double next_cumulative_volume = 0.0;

    int i_wmk_above_cloud = watermark_index_above_cloud(cloud_fractional_volume);

    // Each active watermark
    for (int i_wmk = i_first_active_wmk; i_wmk <= i_wmk_above_cloud; i_wmk++) {
        n_captured_this_wmk = 0;

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

    double n_captured = n_electrons_captured(n_free_electrons + n_released);

    return n_released - n_captured;
}

// ========
// TrapManagerManager::
// ========
/*
    Class TrapManagerManager.

    Handles the one or multiple trap managers required for models with a mix of
    trap species and/or multiphase clocking.

    Each individual trap manager tracks the states of one or more trap species.
    A different trap manager is required for any traps that use a different
    type of watermarks, and separate trap managers are also required for each
    phase in multiphase clocking, which corresponds to an independent set of
    traps.

    Parameters
    ----------
    all_traps : std::valarray<std::valarray<Trap>>
        The array of all trap species, organised by watermark type. The first
        dimension must be n_watermark_types long, with one array of traps
        (which can be empty) for each watermark type. See enum WatermarkType.
        
        e.g. {{trap_1, trap_2}, {trap_3}} for two standard traps and one
        instant-capture trap. Or {{}, {trap_1, trap_2}} for no standard traps
        and two instant capture traps.

    max_n_transfers : int
        Same as TrapManager.

    ccd : CCD
        Parameters to describe how electrons fill the volume inside (all phases
        of) a pixel in a CCD detector.

    dwell_times : std::valarray<double>
        The time between steps in the clocking sequence, as stored by an ROE
        object.

    Attributes
    ----------
    do_standard_traps, do_instant_capture_traps : bool
        Whether or not to manage traps of each watermark type.
*/
TrapManagerManager::TrapManagerManager(
    std::valarray<std::valarray<Trap>>& all_traps, int max_n_transfers, CCD ccd,
    std::valarray<double>& dwell_times)
    : all_traps(all_traps), max_n_transfers(max_n_transfers), ccd(ccd) {

    // Check correct number of trap types provided
    if (all_traps.size() != n_watermark_types)
        error(
            "Size of all_traps (%ld) doesn't match n_watermark_types (%d).",
            all_traps.size(), n_watermark_types);

    // Check matching numbers of dwell times and phases
    if (ccd.n_phases != dwell_times.size())
        error(
            "Number of phases (%d) and dwell times (%ld) don't match.", ccd.n_phases,
            dwell_times.size());

    // Whether we have trap species of each watermark type
    do_standard_traps = all_traps[watermark_type_standard].size();
    do_instant_capture_traps = all_traps[watermark_type_instant_capture].size();

    // ========
    // Set up the trap manager for each phase for each watermark type
    // ========
    if (do_standard_traps) {
        trap_managers.resize(ccd.n_phases);

        // Initialise manager and watermarks for each phase
        for (int phase_index = 0; phase_index < ccd.n_phases; phase_index++) {
            trap_managers[phase_index] = TrapManager(
                all_traps[watermark_type_standard], max_n_transfers,
                ccd.phases[phase_index]);

            trap_managers[phase_index].initialise_trap_states();
            trap_managers[phase_index].set_fill_probabilities_from_dwell_time(
                dwell_times[phase_index]);
        }

        // Check correct trap types
        for (int i_trap = 0; i_trap < trap_managers[0].n_traps; i_trap++) {
            if (trap_managers[0].traps[i_trap].watermark_type !=
                watermark_type_standard)
                error(
                    "Trap [%d]'s watermark type (%d) doesn't match "
                    "watermark_type_standard (%d).",
                    i_trap, trap_managers[0].traps[i_trap].watermark_type,
                    watermark_type_standard);
        }
    }

    if (do_instant_capture_traps) {
        trap_managers_instant_capture.resize(ccd.n_phases);

        // Initialise manager and watermarks for each phase
        for (int phase_index = 0; phase_index < ccd.n_phases; phase_index++) {
            trap_managers_instant_capture[phase_index] = TrapManagerInstantCapture(
                all_traps[watermark_type_instant_capture], max_n_transfers,
                ccd.phases[phase_index]);

            trap_managers_instant_capture[phase_index].initialise_trap_states();
            trap_managers_instant_capture[phase_index]
                .set_fill_probabilities_from_dwell_time(dwell_times[phase_index]);
        }

        // Check correct trap types
        for (int i_trap = 0; i_trap < trap_managers_instant_capture[0].n_traps;
             i_trap++) {
            if (trap_managers_instant_capture[0].traps[i_trap].watermark_type !=
                watermark_type_instant_capture)
                error(
                    "Trap [%d]'s watermark type (%d) doesn't match "
                    "watermark_type_instant_capture (%d).",
                    i_trap,
                    trap_managers_instant_capture[0].traps[i_trap].watermark_type,
                    watermark_type_instant_capture);
        }
    }
}

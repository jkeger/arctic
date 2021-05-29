
#include <math.h>
#include <stdio.h>
#include <valarray>

#include "ccd.hpp"
#include "trap_managers.hpp"
#include "traps.hpp"
#include "util.hpp"

// ========
// TrapManagerBase::
// ========
/*
    Class TrapManagerBase.
    
    Abstract base class for the trap manager of one or multiple trap species
    that are able to use watermarks in the same way as each other.

    Parameters
    ----------
    traps : std::valarray<TrapSlowCapture>
        A list of one or more trap species of the specific trap manager's type.
        Not part of the base class, since the different managers require
        different class types for the array.

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

    empty_watermark : double
        The watermark value corresponding to empty traps.

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
TrapManagerBase::TrapManagerBase(int max_n_transfers, CCDPhase ccd_phase)
    : max_n_transfers(max_n_transfers), ccd_phase(ccd_phase) {

    empty_watermark = 0.0;
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
        The initial empty watermark arrays. See TrapManagerSlowCapture().
*/
void TrapManagerBase::initialise_trap_states() {
    n_watermarks = max_n_transfers * n_watermarks_per_transfer + 1;

    watermark_volumes = std::valarray<double>(empty_watermark, n_watermarks);
    watermark_fills = std::valarray<double>(empty_watermark, n_traps * n_watermarks);

    // Initialise the stored trap states too
    store_trap_states();
}

/*
    Reset the watermark arrays to empty.
*/
void TrapManagerBase::reset_trap_states() {
    n_active_watermarks = 0;
    i_first_active_wmk = 0;
    watermark_volumes = std::valarray<double>(empty_watermark, n_watermarks);
    watermark_fills = std::valarray<double>(empty_watermark, n_traps * n_watermarks);
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
    Sum the total number of electrons currently held in traps.

    Parameters
    ----------
    wmk_volumes, wmk_fills : std::valarray<double>
        Watermark arrays. See TrapManagerSlowCapture().

    Returns
    -------
    n_trapped_electrons : double
        The number of electrons stored in traps.
*/
double TrapManagerBase::n_trapped_electrons_from_watermarks(
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
int TrapManagerBase::watermark_index_above_cloud(double cloud_fractional_volume) {
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

    For the old-standard, release-then-instant-capture algorithm.
*/
TrapManagerInstantCapture::TrapManagerInstantCapture(
    std::valarray<TrapInstantCapture> traps, int max_n_transfers, CCDPhase ccd_phase)
    : TrapManagerBase(max_n_transfers, ccd_phase), traps(traps) {

    n_traps = traps.size();
    trap_densities = std::valarray<double>(n_traps);
    for (int i_trap = 0; i_trap < n_traps; i_trap++) {
        trap_densities[i_trap] = traps[i_trap].density;
    }
}

/*
    Set the probabilities of traps being full after release.

    Parameters
    ----------
    dwell_time : double
        The time spent in this pixel or phase, in the same units as the trap
        timescales.

    Sets
    ----
    fill_probabilities_from_release : std::valarray<double>
        The fraction of traps that were full that stay full after release.

    empty_probabilities_from_release : std::valarray<double>
        The fraction of traps that were full that become empty after release.
*/
void TrapManagerInstantCapture::set_fill_probabilities_from_dwell_time(
    double dwell_time) {
    fill_probabilities_from_release = std::valarray<double>(0.0, n_traps);
    empty_probabilities_from_release = std::valarray<double>(0.0, n_traps);

    // Set probabilities for each trap species
    for (int i_trap = 0; i_trap < n_traps; i_trap++) {
        // Resulting fill fraction from release
        fill_probabilities_from_release[i_trap] =
            exp(-traps[i_trap].emission_rate * dwell_time);
        empty_probabilities_from_release[i_trap] =
            1.0 - fill_probabilities_from_release[i_trap];
    }
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
        The updated watermarks. See TrapManagerSlowCapture().
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
        The updated watermarks. See TrapManagerSlowCapture().
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
        The updated watermarks. See TrapManagerSlowCapture().
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
        The updated watermarks. See TrapManagerSlowCapture().
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
        The updated watermarks. See TrapManagerSlowCapture().
*/
double TrapManagerInstantCapture::n_electrons_released_and_captured(
    double n_free_electrons) {

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
    std::valarray<TrapSlowCapture> traps, int max_n_transfers, CCDPhase ccd_phase)
    : TrapManagerBase(max_n_transfers, ccd_phase), traps(traps) {

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
void TrapManagerSlowCapture::set_fill_probabilities_from_dwell_time(double dwell_time) {
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
            1.0 - traps[i_trap].emission_rate * exponential_factor;

        // Resulting fill fraction from only release
        fill_probabilities_from_release[i_trap] =
            exp(-traps[i_trap].emission_rate * dwell_time);
        empty_probabilities_from_release[i_trap] =
            1.0 - fill_probabilities_from_release[i_trap];
    }
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
        The updated watermarks. See TrapManagerSlowCapture().
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
            for (int i_wmk = i_first_active_wmk + n_active_watermarks;
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
    double frac_released;
    double cumulative_volume = 0.0;
    double next_cumulative_volume = 0.0;

    // Count the released electrons and update the watermarks
    for (int i_wmk = i_wmk_above_cloud;
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
        for (int i_wmk = i_first_active_wmk + n_active_watermarks;
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
// TrapManagerContinuum::
// ========
/*
    Class TrapManagerContinuum.

    For trap species with continous distributions of release timesacles, and
    the old-standard, release-then-instant-capture algorithm.

    In this case, the watermark_fills array tracks the total time elapsed since
    the traps were filled, instead of the fill fractions. Empty watermarks are
    set by -1 instead of 0.
*/
TrapManagerContinuum::TrapManagerContinuum(
    std::valarray<TrapContinuum> traps, int max_n_transfers, CCDPhase ccd_phase)
    : TrapManagerBase(max_n_transfers, ccd_phase), traps(traps) {

    n_traps = traps.size();
    trap_densities = std::valarray<double>(n_traps);
    for (int i_trap = 0; i_trap < n_traps; i_trap++) {
        trap_densities[i_trap] = traps[i_trap].density;
    }

    // Overwrite default parameter values
    empty_watermark = -1.0;
}

/*
    Sum the total number of electrons currently held in traps.

    Parameters
    ----------
    wmk_volumes, wmk_fills : std::valarray<double>
        Watermark arrays. See TrapManagerContinuum().

    Returns
    -------
    n_trapped_electrons : double
        The number of electrons stored in traps.
*/
double TrapManagerContinuum::n_trapped_electrons_from_watermarks(
    std::valarray<double> wmk_volumes, std::valarray<double> wmk_fills) {

    // No watermarks
    if (n_active_watermarks == 0) return 0.0;

    double n_trapped_electrons = 0.0;
    double n_trapped_electrons_this_wmk;
    double fill_fraction;

    // Each active watermark
    for (int i_wmk = i_first_active_wmk;
         i_wmk < i_first_active_wmk + n_active_watermarks; i_wmk++) {
        n_trapped_electrons_this_wmk = 0.0;

        // Each trap species
        for (int i_trap = 0; i_trap < n_traps; i_trap++) {
            // Fraction of trapped electrons
            fill_fraction = traps[i_trap].fill_fraction_from_time_elapsed(
                wmk_fills[i_wmk * n_traps + i_trap]);

            n_trapped_electrons_this_wmk += fill_fraction * traps[i_trap].density;
        }

        // Multiply by the fractional volume
        n_trapped_electrons += n_trapped_electrons_this_wmk * wmk_volumes[i_wmk];
    }

    return n_trapped_electrons;
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

    On declaration, automatically creates the trap managers for each phase
    and/or watermark type and initialises their watermark arrays. The trap
    managers of each type are held in the trap_managers_* arrays, each
    containing one manager of that type for each phase.

    Also, if relevant, modifies the trap managers' trap densities to account for
    the fraction of traps in different CCD pixel phases.

    Parameters
    ----------
    slow_capture_traps : std::valarray<TrapSlowCapture>
    instant_capture_traps : std::valarray<TrapInstantCapture>
        The arrays of trap species, one for each type (which can be empty).

    max_n_transfers : int
        Same as TrapManagerSlowCapture.

    ccd : CCD
        Parameters to describe how electrons fill the volume inside (all phases
        of) a pixel in a CCD detector.

    dwell_times : std::valarray<double>
        The time between steps in the clocking sequence, as stored by an ROE
        object.

    Attributes
    ----------
    n_slow_capture_traps, n_instant_capture_traps : int
        The number of trap species (if any) of each watermark type.

    trap_managers_slow_capture, trap_managers_instant_capture :
        std::valarray<TrapManagerSlowCapture>, std::valarray<TrapManagerInstantCapture>

        For each watermark type, the list of trap manager objects for each
        phase. Ignored if the corresponding n_*_traps is 0.
*/
TrapManagerManager::TrapManagerManager(
    std::valarray<TrapSlowCapture>& slow_capture_traps,
    std::valarray<TrapInstantCapture>& instant_capture_traps,
    // std::valarray<TrapContinuum>& continuum_traps,
    int max_n_transfers, CCD ccd, std::valarray<double>& dwell_times)
    : slow_capture_traps(slow_capture_traps),
      instant_capture_traps(instant_capture_traps),
      max_n_transfers(max_n_transfers),
      ccd(ccd) {

    // The number of trap species (if any) of each watermark type
    n_slow_capture_traps = slow_capture_traps.size();
    n_instant_capture_traps = instant_capture_traps.size();

    // Account for the number of clock-sequence steps for the maximum transfers
    max_n_transfers *= dwell_times.size();

    // ========
    // Set up the trap manager for each phase, for each watermark type
    // ========
    if (n_slow_capture_traps > 0) {
        trap_managers_slow_capture.resize(ccd.n_phases);

        // Initialise manager and watermarks for each phase
        for (int phase_index = 0; phase_index < ccd.n_phases; phase_index++) {
            trap_managers_slow_capture[phase_index] = TrapManagerSlowCapture(
                slow_capture_traps, max_n_transfers, ccd.phases[phase_index]);

            // Modify the trap densities in different phases
            trap_managers_slow_capture[phase_index].trap_densities *=
                ccd.fraction_of_traps_per_phase[phase_index];

            trap_managers_slow_capture[phase_index].initialise_trap_states();
            //## This assumes dwell times are the same in each phase even in
            // e.g. trap-pumping clock sequences with n_steps > n_phases
            trap_managers_slow_capture[phase_index]
                .set_fill_probabilities_from_dwell_time(dwell_times[phase_index]);
        }

        // Check correct watermark types
        for (int i_trap = 0; i_trap < trap_managers_slow_capture[0].n_traps; i_trap++) {
            if (trap_managers_slow_capture[0].traps[i_trap].watermark_type !=
                watermark_type_slow_capture)
                error(
                    "TrapSlowCapture [%d]'s watermark type (%d) doesn't match "
                    "watermark_type_slow_capture (%d).",
                    i_trap, trap_managers_slow_capture[0].traps[i_trap].watermark_type,
                    watermark_type_slow_capture);
        }
    }

    if (n_instant_capture_traps > 0) {
        trap_managers_instant_capture.resize(ccd.n_phases);

        // Initialise manager and watermarks for each phase
        for (int phase_index = 0; phase_index < ccd.n_phases; phase_index++) {
            trap_managers_instant_capture[phase_index] = TrapManagerInstantCapture(
                instant_capture_traps, max_n_transfers, ccd.phases[phase_index]);

            // Modify the trap densities in different phases
            trap_managers_instant_capture[phase_index].trap_densities *=
                ccd.fraction_of_traps_per_phase[phase_index];

            trap_managers_instant_capture[phase_index].initialise_trap_states();
            trap_managers_instant_capture[phase_index]
                .set_fill_probabilities_from_dwell_time(dwell_times[phase_index]);
        }

        // Check correct watermark types
        for (int i_trap = 0; i_trap < trap_managers_instant_capture[0].n_traps;
             i_trap++) {
            if (trap_managers_instant_capture[0].traps[i_trap].watermark_type !=
                watermark_type_instant_capture)
                error(
                    "TrapSlowCapture [%d]'s watermark type (%d) doesn't match "
                    "watermark_type_instant_capture (%d).",
                    i_trap,
                    trap_managers_instant_capture[0].traps[i_trap].watermark_type,
                    watermark_type_instant_capture);
        }
    }
}

/*
    Reset the watermark arrays to empty, for all trap managers.
*/
void TrapManagerManager::reset_trap_states() {
    if (n_slow_capture_traps > 0)
        for (int phase_index = 0; phase_index < ccd.n_phases; phase_index++) {
            trap_managers_slow_capture[phase_index].reset_trap_states();
        }
    if (n_instant_capture_traps > 0)
        for (int phase_index = 0; phase_index < ccd.n_phases; phase_index++) {
            trap_managers_instant_capture[phase_index].reset_trap_states();
        }
}

/*
    Store the watermark arrays to be loaded again later, for all trap managers.
*/
void TrapManagerManager::store_trap_states() {
    if (n_slow_capture_traps > 0)
        for (int phase_index = 0; phase_index < ccd.n_phases; phase_index++) {
            trap_managers_slow_capture[phase_index].store_trap_states();
        }
    if (n_instant_capture_traps > 0)
        for (int phase_index = 0; phase_index < ccd.n_phases; phase_index++) {
            trap_managers_instant_capture[phase_index].store_trap_states();
        }
}

/*
    Restore the watermark arrays to their saved values, for all trap managers.
*/
void TrapManagerManager::restore_trap_states() {
    if (n_slow_capture_traps > 0)
        for (int phase_index = 0; phase_index < ccd.n_phases; phase_index++) {
            trap_managers_slow_capture[phase_index].restore_trap_states();
        }
    if (n_instant_capture_traps > 0)
        for (int phase_index = 0; phase_index < ccd.n_phases; phase_index++) {
            trap_managers_instant_capture[phase_index].restore_trap_states();
        }
}

import numpy as np
import arcticpy.wrapper as w
from arcticpy.src.ccd import CCDPhase, CCD
from arcticpy.src.roe import ROE
from arcticpy.src.traps import TrapInstantCapture, TrapSlowCapture, TrapContinuum


def _extract_trap_parameters(traps):
    """Extract trap parameters for add/remove_cti() to pass to the wrapper.

    Returns the converted arguments in the formats and types required by the
    cython wrapper's cy_add/remove_cti().
    """
    # Extract trap inputs
    traps_instant_capture = [trap for trap in traps if type(trap) == TrapInstantCapture]
    traps_slow_capture = [trap for trap in traps if type(trap) == TrapSlowCapture]
    traps_continuum = [trap for trap in traps if type(trap) == TrapContinuum]
    n_traps_instant_capture = len(traps_instant_capture)
    n_traps_slow_capture = len(traps_slow_capture)
    n_traps_continuum = len(traps_continuum)
    if n_traps_slow_capture + n_traps_instant_capture + n_traps_continuum != len(traps):
        raise Exception(
            "Not all traps extracted successfully (%d instant capture, %d slow capture, %d continuum, %d total)"
            % (
                n_traps_instant_capture,
                n_traps_slow_capture,
                n_traps_continuum,
                len(traps),
            )
        )

    # Make sure the order is correct
    traps = traps_instant_capture + traps_slow_capture + traps_continuum
    trap_densities = np.array([trap.density for trap in traps], dtype=np.double)
    trap_release_timescales = np.array(
        [trap.release_timescale for trap in traps], dtype=np.double
    )
    # Third parameter for some trap types
    trap_third_params = []
    for trap in traps:
        if type(trap) == TrapInstantCapture:
            trap_third_params.append(0.0)
        elif type(trap) == TrapSlowCapture:
            trap_third_params.append(trap.capture_timescale)
        elif type(trap) == TrapContinuum:
            trap_third_params.append(trap.release_timescale_sigma)
    trap_third_params = np.array(trap_third_params, dtype=np.double)

    return (
        trap_densities,
        trap_release_timescales,
        trap_third_params,
        n_traps_instant_capture,
        n_traps_slow_capture,
        n_traps_continuum,
    )


def _set_dummy_parameters():
    """Set dummy variables for add/remove_cti() to pass to the wrapper.

    Returns placeholder arguments in the formats and types required by the
    cython wrapper's cy_add/remove_cti() for when one of parallel or serial
    clocking is not being used.
    """
    roe = ROE()
    ccd = CCD([CCDPhase(0.0, 0.0, 0.0)], [0.0])
    trap_densities = np.array([0.0], dtype=np.double)
    trap_release_timescales = np.array([0.0], dtype=np.double)
    trap_third_params = np.array([0.0], dtype=np.double)
    n_traps_instant_capture = 0
    n_traps_slow_capture = 0
    n_traps_continuum = 0
    express = 0
    offset = 0
    window_start = 0
    window_stop = 0

    return (
        roe,
        ccd,
        trap_densities,
        trap_release_timescales,
        trap_third_params,
        n_traps_instant_capture,
        n_traps_slow_capture,
        n_traps_continuum,
        express,
        offset,
        window_start,
        window_stop,
    )


def add_cti(
    image,
    # Parallel
    parallel_ccd=None,
    parallel_roe=None,
    parallel_traps=None,
    parallel_express=0,
    parallel_offset=0,
    parallel_window_start=0,
    parallel_window_stop=-1,
    # Serial
    serial_ccd=None,
    serial_roe=None,
    serial_traps=None,
    serial_express=0,
    serial_offset=0,
    serial_window_start=0,
    serial_window_stop=-1,
    # Output
    verbosity=1,
):
    """
    Wrapper for arctic's add_cti() in src/cti.cpp, see its documentation.

    Add CTI trails to an image by trapping, releasing, and moving electrons
    along their independent columns, for parallel and/or serial clocking.

    This wrapper extracts individual numbers and arrays from the user-input
    objects to pass to the C++ via Cython. See cy_add_cti() in
    arcticpy/wrapper.pyx and add_cti() in arcticpy/interface.cpp.

    Parameters (where different to add_cti() in src/cti.cpp)
    ----------
    parallel_traps : [Trap]
    serial_traps : [Trap]
        The 1D arrays of all trap species objects, for parallel and serial
        clocking. The core arctic's add_cti() requires the different types of
        traps to be provided in separate arrays. Here, mutliple trap types can
        be passed in a single array, which will be separated by the wrapper.

    verbosity : int (opt.)
        The verbosity parameter to control the amount of printed information:
            0   No printing (except errors etc).
            1   Standard.
            2   Extra details.
    """
    image = np.copy(image).astype(np.double)

    # ========
    # Extract inputs and/or set dummy variables to pass to the wrapper
    # ========
    # Parallel
    if parallel_traps is not None:
        (
            parallel_trap_densities,
            parallel_trap_release_timescales,
            parallel_trap_third_params,
            parallel_n_traps_instant_capture,
            parallel_n_traps_slow_capture,
            parallel_n_traps_continuum,
        ) = _extract_trap_parameters(parallel_traps)
    else:
        # No parallel clocking, set dummy variables instead
        (
            parallel_roe,
            parallel_ccd,
            parallel_trap_densities,
            parallel_trap_release_timescales,
            parallel_trap_third_params,
            parallel_n_traps_instant_capture,
            parallel_n_traps_slow_capture,
            parallel_n_traps_continuum,
            parallel_express,
            parallel_offset,
            parallel_window_start,
            parallel_window_stop,
        ) = _set_dummy_parameters()

    # Serial
    if serial_traps is not None:
        (
            serial_trap_densities,
            serial_trap_release_timescales,
            serial_trap_third_params,
            serial_n_traps_instant_capture,
            serial_n_traps_slow_capture,
            serial_n_traps_continuum,
        ) = _extract_trap_parameters(serial_traps)
    else:
        # No serial clocking, set dummy variables instead
        (
            serial_roe,
            serial_ccd,
            serial_trap_densities,
            serial_trap_release_timescales,
            serial_trap_third_params,
            serial_n_traps_instant_capture,
            serial_n_traps_slow_capture,
            serial_n_traps_continuum,
            serial_express,
            serial_offset,
            serial_window_start,
            serial_window_stop,
        ) = _set_dummy_parameters()

    # ========
    # Add CTI
    # ========
    # Pass the extracted inputs to C++ via the cython wrapper
    return w.cy_add_cti(
        image,
        # ========
        # Parallel
        # ========
        # ROE
        parallel_roe.dwell_times,
        parallel_roe.empty_traps_between_columns,
        parallel_roe.empty_traps_for_first_transfers,
        parallel_roe.force_release_away_from_readout,
        parallel_roe.use_integer_express_matrix,
        # CCD
        parallel_ccd.fraction_of_traps_per_phase,
        parallel_ccd.full_well_depths,
        parallel_ccd.well_notch_depths,
        parallel_ccd.well_fill_powers,
        # Traps
        parallel_trap_densities,
        parallel_trap_release_timescales,
        parallel_trap_third_params,
        parallel_n_traps_instant_capture,
        parallel_n_traps_slow_capture,
        parallel_n_traps_continuum,
        # Misc
        parallel_express,
        parallel_offset,
        parallel_window_start,
        parallel_window_stop,
        # ========
        # Serial
        # ========
        # ROE
        serial_roe.dwell_times,
        serial_roe.empty_traps_between_columns,
        serial_roe.empty_traps_for_first_transfers,
        serial_roe.force_release_away_from_readout,
        serial_roe.use_integer_express_matrix,
        # CCD
        serial_ccd.fraction_of_traps_per_phase,
        serial_ccd.full_well_depths,
        serial_ccd.well_notch_depths,
        serial_ccd.well_fill_powers,
        # Traps
        serial_trap_densities,
        serial_trap_release_timescales,
        serial_trap_third_params,
        serial_n_traps_instant_capture,
        serial_n_traps_slow_capture,
        serial_n_traps_continuum,
        # Misc
        serial_express,
        serial_offset,
        serial_window_start,
        serial_window_stop,
        # Output
        verbosity,
    )


def remove_cti(
    image,
    n_iterations,
    # Parallel
    parallel_ccd=None,
    parallel_roe=None,
    parallel_traps=None,
    parallel_express=0,
    parallel_offset=0,
    parallel_window_start=0,
    parallel_window_stop=-1,
    # Serial
    serial_ccd=None,
    serial_roe=None,
    serial_traps=None,
    serial_express=0,
    serial_offset=0,
    serial_window_start=0,
    serial_window_stop=-1,
    # Output
    verbosity=1,
):
    """
    Wrapper for arctic's remove_cti() in src/cti.cpp, see its documentation.

    Remove CTI trails from an image by first modelling the addition of CTI, for
    parallel and/or serial clocking.

    This wrapper extracts individual numbers and arrays from the user-input
    objects to pass to the C++ via Cython. See cy_remove_cti() in
    arcticpy/wrapper.pyx and remove_cti() in arcticpy/interface.cpp.

    Parameters (where different to remove_cti() in src/cti.cpp)
    ----------
    parallel_traps : [Trap]
    serial_traps : [Trap]
        The 1D arrays of all trap species objects, for parallel and serial
        clocking. The core arctic's add_cti() requires the different types of
        traps to be provided in separate arrays. Here, mutliple trap types can
        be passed in a single array, which will be separated by the wrapper.

    verbosity : int (opt.)
        The verbosity parameter to control the amount of printed information:
            0   No printing (except errors etc).
            1   Standard.
            2   Extra details.
    """
    image = np.copy(image).astype(np.double)

    # ========
    # Extract inputs and/or set dummy variables to pass to the wrapper
    # ========
    # Parallel
    if parallel_traps is not None:
        (
            parallel_trap_densities,
            parallel_trap_release_timescales,
            parallel_trap_third_params,
            parallel_n_traps_slow_capture,
            parallel_n_traps_instant_capture,
        ) = _extract_trap_parameters(parallel_traps)
    else:
        # No parallel clocking, set dummy variables instead
        (
            parallel_roe,
            parallel_ccd,
            parallel_trap_densities,
            parallel_trap_release_timescales,
            parallel_trap_third_params,
            parallel_n_traps_slow_capture,
            parallel_n_traps_instant_capture,
            parallel_express,
            parallel_offset,
            parallel_window_start,
            parallel_window_stop,
        ) = _set_dummy_parameters()

    # Serial
    if serial_traps is not None:
        (
            serial_trap_densities,
            serial_trap_release_timescales,
            serial_trap_third_params,
            serial_n_traps_slow_capture,
            serial_n_traps_instant_capture,
        ) = _extract_trap_parameters(serial_traps)
    else:
        # No serial clocking, set dummy variables instead
        (
            serial_roe,
            serial_ccd,
            serial_trap_densities,
            serial_trap_release_timescales,
            serial_trap_third_params,
            serial_n_traps_slow_capture,
            serial_n_traps_instant_capture,
            serial_express,
            serial_offset,
            serial_window_start,
            serial_window_stop,
        ) = _set_dummy_parameters()

    # ========
    # Remove CTI
    # ========
    # Pass the extracted inputs to C++ via the cython wrapper
    return w.cy_remove_cti(
        image,
        n_iterations,
        # ========
        # Parallel
        # ========
        # ROE
        parallel_roe.dwell_times,
        parallel_roe.empty_traps_between_columns,
        parallel_roe.empty_traps_for_first_transfers,
        parallel_roe.force_release_away_from_readout,
        parallel_roe.use_integer_express_matrix,
        # CCD
        parallel_ccd.fraction_of_traps_per_phase,
        parallel_ccd.full_well_depths,
        parallel_ccd.well_notch_depths,
        parallel_ccd.well_fill_powers,
        # Traps
        parallel_trap_densities,
        parallel_trap_release_timescales,
        parallel_trap_third_params,
        parallel_n_traps_instant_capture,
        parallel_n_traps_slow_capture,
        parallel_n_traps_continuum,
        # Misc
        parallel_express,
        parallel_offset,
        parallel_window_start,
        parallel_window_stop,
        # ========
        # Serial
        # ========
        # ROE
        serial_roe.dwell_times,
        serial_roe.empty_traps_between_columns,
        serial_roe.empty_traps_for_first_transfers,
        serial_roe.force_release_away_from_readout,
        serial_roe.use_integer_express_matrix,
        # CCD
        serial_ccd.fraction_of_traps_per_phase,
        serial_ccd.full_well_depths,
        serial_ccd.well_notch_depths,
        serial_ccd.well_fill_powers,
        # Traps
        serial_trap_densities,
        serial_trap_release_timescales,
        serial_trap_third_params,
        serial_n_traps_instant_capture,
        serial_n_traps_slow_capture,
        serial_n_traps_continuum,
        # Misc
        serial_express,
        serial_offset,
        serial_window_start,
        serial_window_stop,
        # Output
        verbosity,
    )

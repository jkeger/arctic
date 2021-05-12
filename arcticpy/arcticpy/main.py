import numpy as np
import wrapper as w
from arcticpy.classes import Trap, TrapInstantCapture


def add_cti(
    image_pre_cti,
    traps,
    roe,
    ccd,
    express,
    offset,
    start,
    stop,
):
    # Extract trap inputs
    traps_standard = [trap for trap in traps if type(trap) == Trap]
    traps_instant_capture = [trap for trap in traps if type(trap) == TrapInstantCapture]
    n_traps_standard = len(traps_standard)
    n_traps_instant_capture = len(traps_instant_capture)
    if n_traps_standard + n_traps_instant_capture != len(traps):
        raise Exception(
            "Not all traps extracted successfully (%d standard, %d instant capture, % total)"
            % (n_traps_standard, n_traps_instant_capture, len(traps))
        )
    # Make sure the order is correct
    traps = traps_standard + traps_instant_capture
    trap_densities = np.array([trap.density for trap in traps])
    trap_release_timescales = np.array([trap.release_timescale for trap in traps])
    trap_capture_timescales = np.array([trap.capture_timescale for trap in traps])

    # Extract CCD phase inputs
    ccd.full_well_depths = np.array([phase.full_well_depth for phase in ccd.phases])
    ccd.well_notch_depths = np.array([phase.well_notch_depth for phase in ccd.phases])
    ccd.well_fill_powers = np.array([phase.well_fill_power for phase in ccd.phases])

    return w.cy_add_cti(
        image_pre_cti,
        # Traps
        trap_densities,
        trap_release_timescales,
        trap_capture_timescales,
        n_traps_standard,
        n_traps_instant_capture,
        # ROE
        roe.dwell_times,
        roe.empty_traps_between_columns,
        roe.empty_traps_for_first_transfers,
        roe.force_release_away_from_readout,
        roe.use_integer_express_matrix,
        # CCD
        ccd.fraction_of_traps_per_phase,
        ccd.full_well_depths,
        ccd.well_notch_depths,
        ccd.well_fill_powers,
        # Misc
        express,
        offset,
        start,
        stop,
    )

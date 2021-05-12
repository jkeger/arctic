import numpy as np
import wrapper as w
from arcticpy.classes import ROE


def add_cti(
    image_pre_cti,
    trap_density,
    trap_lifetime,
    roe,
    ccd,
    express,
    offset,
    start,
    stop,
):
    return w.cy_add_cti(
        image_pre_cti,
        # Traps
        trap_density,
        trap_lifetime,
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

import numpy as np
import os
import sys

sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)), ".."))
import arcticpy as ac

image_pre_cti = np.array(
    [
        [0.0, 0.0, 0.0, 0.0],
        [200.0, 0.0, 0.0, 0.0],
        [0.0, 200.0, 0.0, 0.0],
        [0.0, 0.0, 200.0, 0.0],
        [0.0, 0.0, 0.0, 0.0],
        [0.0, 0.0, 0.0, 0.0],
    ]
)
# image_pre_cti = np.loadtxt("dev/hst_acs_10_col.txt", skiprows=1)

roe = ac.ROE(
    dwell_times=[1.0],
    empty_traps_between_columns=True,
    empty_traps_for_first_transfers=False,
    force_release_away_from_readout=True,
    use_integer_express_matrix=False,
)
ccd = ac.CCD(
    phases=[
        ac.CCDPhase(full_well_depth=1e3, well_notch_depth=0.0, well_fill_power=1.0)
    ],
    fraction_of_traps_per_phase=[1.0],
)
traps = [ac.TrapInstantCapture(density=10.0, release_timescale=-1.0 / np.log(0.5))]
express = 0
offset = 0
start = 0
stop = -1

ac.print_array_2D(image_pre_cti)

image_post_cti = ac.add_cti(
    image_pre_cti=image_pre_cti,
    parallel_roe=roe,
    parallel_ccd=ccd,
    parallel_traps=traps,
    parallel_express=express,
    parallel_offset=offset,
    parallel_window_start=start,
    parallel_window_stop=stop,
    # serial_roe=roe,
    # serial_ccd=ccd,
    # serial_traps=traps,
    # serial_express=express,
    # serial_offset=offset,
    # serial_window_start=start,
    # serial_window_stop=stop,
    # verbosity=2,
)

ac.print_array_2D(image_post_cti)

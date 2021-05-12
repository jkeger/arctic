import numpy as np
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

roe = ac.ROE([1.0], True, False, False, False)
ccd = ac.CCD([ac.CCDPhase(1e4, 0.0, 1.0)], [1.0])
traps = [ac.TrapInstantCapture(10.0, -1.0 / np.log(0.5))]
express = 5
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
)

ac.print_array_2D(image_post_cti)

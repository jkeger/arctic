
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
# image_pre_cti = np.loadtxt("../dev/hst_acs_10_col.txt", skiprows=1)

traps = [ac.TrapInstantCapture(10.0, -1.0 / np.log(0.5))]
roe = ac.ROE([1.0], True, False, False, False)
ccd = ac.CCD([ac.CCDPhase(1e4, 0.0, 1.0)], [1.0])
express = 5
offset = 0
start = 0
stop = -1

ac.print_array_2D(image_pre_cti)

image_post_cti = ac.add_cti(
    image_pre_cti,
    traps,
    roe,
    ccd,
    express,
    offset,
    start,
    stop,
)

ac.print_array_2D(image_post_cti)

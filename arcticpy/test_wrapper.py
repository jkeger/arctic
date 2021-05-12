
import numpy as np
import wrapper as w

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

trap_density = 10.0
trap_lifetime = -1.0 / np.log(0.5)
dwell_time = 1.0
ccd_full_well_depth = 1e4
ccd_well_notch_depth = 0.0
ccd_well_fill_power = 1.0
express = 5
offset = 0
start = 0
stop = -1

w.cy_print_array_2D(image_pre_cti)

image_post_cti = w.cy_test_add_cti(
    image_pre_cti,
    trap_density,
    trap_lifetime,
    dwell_time,
    ccd_full_well_depth,
    ccd_well_notch_depth,
    ccd_well_fill_power,
    express,
    offset,
    start,
    stop,
)

w.cy_print_array_2D(image_post_cti)

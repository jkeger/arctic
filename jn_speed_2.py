from astropy.io import fits
import numpy as np
import arcticpy
import time

np.set_printoptions(linewidth=205, edgeitems=56, suppress=True)

# parallel_prune_n_electrons=1e-18
# parallel_prune_frequency=5
parallel_express = 2

hdu_list = fits.open("slow_serial.fits", do_not_scale_image_data=False)

# Transpose data is necessary due to orientation of fits.

image_model = np.array(hdu_list[0].data).astype("float64").T

print(image_model)

trap_0 = arcticpy.TrapInstantCapture(
    density=0.1019478899846178, release_timescale=1.2049949540017573
)
trap_1 = arcticpy.TrapInstantCapture(
    density=0.1757817374972805, release_timescale=5.2634561852450545
)
trap_2 = arcticpy.TrapInstantCapture(
    density=13.325436739291849, release_timescale=16.295735476619164
)

parallel_traps = [trap_0, trap_1, trap_2]

parallel_ccd = arcticpy.CCD(full_well_depth=200000, well_fill_power=0.38350948311740113)
parallel_roe = arcticpy.ROE()


#
# Noisy image
#
image_pre_cti = image_model

start = time.time_ns()

image_post_cti = arcticpy.add_cti(
    image=image_pre_cti,
    parallel_traps=parallel_traps,
    parallel_ccd=parallel_ccd,
    parallel_roe=parallel_roe,
    parallel_express=parallel_express,
    #   parallel_prune_n_electrons=parallel_prune_n_electrons,
    #   parallel_prune_frequency=parallel_prune_frequency,
    verbosity=0,
)

print(f"Clocking Time = {((time.time_ns() - start)/1e9)} s")

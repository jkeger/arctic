from astropy.io import fits
import numpy as np
import arcticpy
import time

np.set_printoptions(linewidth=205, edgeitems=56, suppress=True)

# serial_prune_n_electrons=1e-18
# serial_prune_frequency=5
serial_express = 2

hdu_list = fits.open("slow_serial.fits", do_not_scale_image_data=False)
image_model = np.array(hdu_list[0].data).astype("float64")

trap_0 = arcticpy.TrapInstantCapture(
    density=0.1019478899846178, release_timescale=1.2049949540017573
)
trap_1 = arcticpy.TrapInstantCapture(
    density=0.1757817374972805, release_timescale=5.2634561852450545
)
trap_2 = arcticpy.TrapInstantCapture(
    density=13.325436739291849, release_timescale=16.295735476619164
)

serial_traps = [trap_0, trap_1, trap_2]

serial_ccd = arcticpy.CCD(full_well_depth=200000, well_fill_power=0.38350948311740113)
serial_roe = arcticpy.ROE()


#
# Noisy image
#
image_pre_cti = image_model

start = time.time_ns()

image_post_cti = arcticpy.add_cti(
    image=image_pre_cti,
    serial_traps=serial_traps,
    serial_ccd=serial_ccd,
    serial_roe=serial_roe,
    serial_express=serial_express,
    #   serial_prune_n_electrons=serial_prune_n_electrons,
    #   serial_prune_frequency=serial_prune_frequency,
    verbosity=0,
)

print(f"Clocking Time = {((time.time_ns() - start)/1e9)} s")

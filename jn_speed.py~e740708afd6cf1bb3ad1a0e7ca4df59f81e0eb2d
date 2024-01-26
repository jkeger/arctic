import numpy as np
import arcticpy
import time

np.set_printoptions(linewidth=205, edgeitems=56, suppress=True)


parallel_prune_n_electrons = 1e-18
parallel_prune_frequency = 5
parallel_express = 5

#
# Test different values of pruning (edit here)
#
parallel_prune_n_electrons = -1e-12
parallel_prune_frequency = 1000
parallel_express = 1

#
# Set up test image
#
image_model = np.zeros((2000, 10)) + 200
image_model[0:5, :] += 700
# image_model[500:505,:]+=700
# image_model[1000:1005,:]+=700
# image_model[1500:1505,:]+=700

parallel_traps = [
    arcticpy.TrapInstantCapture(density=10.0, release_timescale=(-1 / np.log(0.5))),
    # arcticpy.TrapInstantCapture(density=10.0, release_timescale=100),
    # arcticpy.TrapSlowCapture(density=10.0, release_timescale=(-1/np.log(0.5)), capture_timescale=0.001),
    # arcticpy.TrapSlowCapture(density=10.0, release_timescale=(4), capture_timescale=0.001),
    # arcticpy.TrapInstantCaptureContinuum(density=10.0, release_timescale=(1.), release_timescale_sigma=0.1),
    # arcticpy.TrapSlowCaptureContinuum(density=10.0, release_timescale=(1.), capture_timescale=1, release_timescale_sigma=0.1),
]

parallel_ccd = arcticpy.CCD(full_well_depth=1000, well_fill_power=1.0)
parallel_roe = arcticpy.ROE(
    empty_traps_between_columns=True,
    empty_traps_for_first_transfers=False,
    overscan_start=1990,
    prescan_offset=10,
)


#
# Noisy image
#
image_pre_cti = image_model + np.random.normal(0, 0.01, image_model.shape)
# image_pre_cti = np.maximum(image_pre_cti,np.zeros(image_pre_cti.shape));
# print(image_pre_cti[0:10,0])

start = time.time_ns()

image_post_cti = arcticpy.add_cti(
    image=image_pre_cti,
    parallel_traps=parallel_traps,
    parallel_ccd=parallel_ccd,
    parallel_roe=parallel_roe,
    parallel_express=parallel_express,
    parallel_prune_n_electrons=parallel_prune_n_electrons,
    parallel_prune_frequency=parallel_prune_frequency,
    verbosity=0,
)

print(f"Clocking Time Noisy = {((time.time_ns() - start)/1e9)} s")
# print(image_post_cti[0:10,0])

#
# Noise-free image
#

image_pre_cti = image_model
# print(image_pre_cti[0:10,0])

start = time.time_ns()

image_post_cti = arcticpy.add_cti(
    image=image_pre_cti,
    parallel_traps=parallel_traps,
    parallel_ccd=parallel_ccd,
    parallel_roe=parallel_roe,
    parallel_express=parallel_express,
    parallel_prune_n_electrons=parallel_prune_n_electrons,
    parallel_prune_frequency=parallel_prune_frequency,
    verbosity=0,
)

print(f"Clocking Time No Noise = {((time.time_ns() - start)/1e9)} s")
print(image_post_cti[0:10, 0])

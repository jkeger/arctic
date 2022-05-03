import numpy as np
import arcticpy
import time
np.set_printoptions(linewidth=105,edgeitems=56,suppress=True)

#
# Set up test image
#

image_model = np.zeros((200,1))+200
image_model[0,:]+=700

parallel_traps = [
    #arcticpy.TrapInstantCapture(density=10.0, release_timescale=(-1/np.log(0.5))),
    arcticpy.TrapSlowCapture(density=2.0, release_timescale=(1.), capture_timescale=0.1),
    arcticpy.TrapInstantCaptureContinuum(density=2.0, release_timescale=(1.), release_timescale_sigma=0.1),
]

parallel_ccd = arcticpy.CCD(full_well_depth=1000, well_fill_power=1.0)
parallel_roe = arcticpy.ROE(
    empty_traps_between_columns=True,
    empty_traps_for_first_transfers=False,
    overscan_start=1990
)

print(parallel_roe.prescan_offset)
print(parallel_roe.overscan_start)



#
# Noisy image
#

image_pre_cti = image_model + np.random.normal(0,0.,image_model.shape)
#image_pre_cti+=np.random.normal(0,10.,image_pre_cti.shape)
#image_pre_cti = np.maximum(image_pre_cti,np.zeros(image_pre_cti.shape));
print(image_pre_cti[4:10,0])

start = time.time_ns()

image_post_cti = arcticpy.add_cti(
    image=image_pre_cti,
    parallel_traps=parallel_traps,
    parallel_ccd=parallel_ccd,
    parallel_roe=parallel_roe,
    parallel_express=0,
    parallel_prune_n_electrons=1e-18,
    parallel_prune_frequency=1,
    verbosity=0
)

print(f"Clocking Time Noisy = {((time.time_ns() - start)/1e9)} s")
print(image_post_cti[4:10,0])

#
# Noise-free image
#

image_pre_cti = image_model
print(image_pre_cti[4:10,0])

start = time.time_ns()

image_post_cti = arcticpy.add_cti(
    image=image_pre_cti,
    parallel_traps=parallel_traps,
    parallel_ccd=parallel_ccd,
    parallel_roe=parallel_roe,
    parallel_express=0,
    parallel_prune_n_electrons=1e-8,
    parallel_prune_frequency=1,
    verbosity=0
)

print(f"Clocking Time No Noise = {((time.time_ns() - start)/1e9)} s")
print(image_post_cti[4:10,0])

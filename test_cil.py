import numpy as np
import arcticpy as ac

#from arcticpy.src import cti
#import autocti as ac


image_pre_cti = np.array([[1.0, 1.0, 1.0],
                         [0.0, 0.0, 0.0],
                         [0.0, 0.0, 0.0]])

parallel_trap_0 = ac.TrapInstantCapture(density=0.07275, release_timescale=0.8)

parallel_trap_list = [parallel_trap_0]

parallel_ccd = ac.CCD(
    well_fill_power=0.58, well_notch_depth=0.0, full_well_depth=200000.0
)


image_post_cti_0 = ac.add_cti(
    image=image_pre_cti,
    parallel_express=0,
    parallel_traps=parallel_trap_list,
    parallel_ccd=parallel_ccd,
    parallel_roe=ac.ROE()
)


image_post_cti_1 = ac.add_cti(
    image=image_pre_cti,
    parallel_express=0,
    parallel_traps=parallel_trap_list,
    parallel_ccd=parallel_ccd,
    parallel_roe=ac.ROEChargeInjection()
)


print(((image_post_cti_0)))
print(((image_post_cti_1)))
print(((image_post_cti_0 - image_post_cti_1)))
print(np.max(abs(image_post_cti_0 - image_post_cti_1)))

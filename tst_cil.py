import numpy as np
import arcticpy as ac
import numpy as np
import arcticpy
import time
import matplotlib.pyplot as plt

np.set_printoptions(linewidth=205, edgeitems=56, suppress=True)


#
# Test different values of pruning (edit here)
#
parallel_prune_n_electrons = 1e-9
parallel_prune_frequency = 20
parallel_express = 0

#
# Set up test image
#
image_pre_cti = np.zeros((2000, 1)) + 200
image_pre_cti[0:5, :] += 700
image_pre_cti[500:505, :] += 700
image_pre_cti[1000:1005, :] += 7000
image_pre_cti[1500:1505, :] += 70000

parallel_trap_list = [ac.TrapInstantCapture(density=3, release_timescale=2)]

parallel_slowtrap_list = [
    ac.TrapSlowCapture(density=3, release_timescale=2, capture_timescale=5.0)
]

parallel_ccd = ac.CCD(
    well_fill_power=0.58, well_notch_depth=0.0, full_well_depth=200000.0
)

image_post_cti_0 = ac.add_cti(
    image=image_pre_cti,
    parallel_express=1,
    parallel_traps=parallel_trap_list,
    parallel_ccd=parallel_ccd,
    parallel_roe=ac.ROEChargeInjection(),
)


image_post_cti_1 = ac.add_cti(
    image=image_pre_cti,
    parallel_express=1,
    parallel_traps=parallel_slowtrap_list,
    parallel_ccd=parallel_ccd,
    parallel_roe=ac.ROEChargeInjection(),
)


print(np.max(abs(image_post_cti_0 - image_post_cti_1)))

rn = ac.ReadNoise(sigma=-4.5)
print(rn.sigma)


#
# Plot
#
n_rows_in_image, n_columns_in_image = image_pre_cti.shape
pixels = np.arange(n_rows_in_image)
colours = ["#1199ff", "#ee4400", "#7711dd", "#44dd44", "#775533"]
fig, ax = plt.subplots()
ax.plot(pixels, image_pre_cti[:, 0], alpha=0.8, label="%d")
ax.plot(pixels, image_post_cti_0[:, 0], alpha=0.8, label="%d")
ax.plot(pixels, image_post_cti_1[:, 0], alpha=0.8, label="%d")

ax.set(
    xlabel="pixel", ylabel="offset bias [n_e]", title="Effect of CTI"  # yscale='log',
)
ax.grid()
plt.tight_layout()
plt.show(block=False)
plt.show()

print(pixels.shape, image_post_cti_1[:, 0].shape)

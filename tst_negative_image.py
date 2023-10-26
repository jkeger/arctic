import numpy as np
import arcticpy
import time
import matplotlib.pyplot as plt

np.set_printoptions(linewidth=205, edgeitems=56, suppress=True)


parallel_prune_n_electrons = 1e-18
parallel_prune_frequency = 0
parallel_express = 1

#
# Test different values of pruning (edit here)
#
parallel_prune_n_electrons = 1e-6
parallel_prune_frequency = 10
parallel_express = 0

#
# Set up test image
#
image_model = np.zeros((50, 1)) + 0
image_model[3:8, :] += 70
# image_model[12:17,:]-=20
# image_model[13:17,:]-=50
image_model[12:17, :] -= 70
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

parallel_ccd = arcticpy.CCD(
    full_well_depth=1000,
    well_fill_power=1.0,
    well_notch_depth=0.0,
    first_electron_fill=0.0,
)
parallel_roe = arcticpy.ROE(
    empty_traps_between_columns=True,
    empty_traps_for_first_transfers=True,
    overscan_start=1990,
    prescan_offset=10,
)
# parallel_roe = arcticpy.ROEChargeInjection()


#
# Noisy image
#
image_pre_cti_noisy = image_model  # + np.random.normal(0,4.5,image_model.shape)
# image_pre_cti = np.maximum(image_pre_cti,np.zeros(image_pre_cti.shape));
# print(image_pre_cti[0:10,0])
parallel_ccd_alt = arcticpy.CCD(
    full_well_depth=1000,
    well_fill_power=1.0,
    well_notch_depth=-200,
    first_electron_fill=0.0,
)

start = time.time_ns()

image_post_cti_noisy = arcticpy.add_cti(
    image=image_pre_cti_noisy,
    parallel_traps=parallel_traps,
    parallel_ccd=parallel_ccd_alt,
    parallel_roe=parallel_roe,
    parallel_express=parallel_express,
    parallel_prune_n_electrons=parallel_prune_n_electrons,
    parallel_prune_frequency=parallel_prune_frequency,
    verbosity=0,
)

print(f"Clocking Time Noisy = {((time.time_ns() - start)/1e9)} s")

#
# Noise-free image
#

image_pre_cti = image_model
# print(image_pre_cti[0:10,0])

start = time.time_ns()

image_post_cti_nonoise = arcticpy.add_cti(
    image=image_pre_cti,
    parallel_traps=parallel_traps,
    parallel_ccd=parallel_ccd,
    parallel_roe=parallel_roe,
    parallel_express=1,
    parallel_prune_n_electrons=parallel_prune_n_electrons,
    parallel_prune_frequency=0,
    allow_negative_pixels=1,
    verbosity=0,
)

print(f"Clocking Time No Noise = {((time.time_ns() - start)/1e9)} s")
print(np.sum(image_model), image_model[0:49, 0])
print(np.sum(image_post_cti_noisy), image_post_cti_noisy[0:49, 0])
print(np.sum(image_post_cti_nonoise), image_post_cti_nonoise[0:49, 0])


#
# Correct CTI
#
# start = time.time_ns()
# image_corrected_nonoise = arcticpy.remove_cti(
#    image=image_post_cti_nonoise, n_iterations=5,
#    parallel_traps=parallel_traps,
#    parallel_ccd=parallel_ccd,
#    parallel_roe=parallel_roe,
#    parallel_express=parallel_express,
#    parallel_prune_n_electrons=parallel_prune_n_electrons,
#    parallel_prune_frequency=parallel_prune_frequency,
#    allow_negative_pixels=1,
#    verbosity=0
# )
# print(f"Correction Time No Noise = {((time.time_ns() - start)/1e9)} s")


#
# Plot
#
n_rows_in_image, n_columns_in_image = image_post_cti_nonoise.shape
pixels = np.arange(n_rows_in_image)
colours = ["#1199ff", "#ee4400", "#7711dd", "#44dd44", "#775533"]
fig, ax = plt.subplots()
ax.plot(pixels[0:50], image_model[0:50, 0], alpha=0.8, label="Input")
# ax.plot(pixels[0:50], image_post_cti_nonoise[0:50,0]-image_model[0,0:50], alpha=0.8, label="%d")
# ax.plot(pixels[0:50], image_post_cti[0:50,0], alpha=0.8, label="%d")
ax.plot(pixels[0:50], image_post_cti_nonoise[0:50, 0], alpha=0.8, label="Trailed")
ax.plot(pixels[0:50], image_post_cti_noisy[0:50, 0], alpha=0.8, label="Alt CCD")
# ax.plot(pixels[0:50], image_corrected_nonoise[0:50,0], alpha=0.8, label="%d")

ax.set(xlabel="pixel", ylabel="offset bias [n_e]", title="Effect of CTI")
ax.grid()
ax.legend()
plt.tight_layout()
# plt.ioff()
# plt.pause(0.01)
plt.show(block=False)
# plt.gcf().canvas.draw_idle()
# plt.gcf().canvas.start_event_loop(0.3)

# import time
# time.sleep(2)
# Press enter to continue
input("Press Enter to continue...")
# Press any key to continue
# print("Press any key to continue...")
# orig_settings = termios.tcgetattr(sys.stdin)
# tty.setcbreak(sys.stdin)
# x = 0
# while x == 0:
#    x=sys.stdin.read(1)[0]
# termios.tcsetattr(sys.stdin, termios.TCSADRAIN, orig_settings)

import numpy as np
import arcticpy
import time
import matplotlib.pyplot as plt

np.set_printoptions(linewidth=205, edgeitems=56, suppress=True)


#
# Test different values of pruning (edit here)
#
parallel_prune_n_electrons = 1e-6
parallel_prune_frequency = 10
parallel_express = 2
stats_range = np.arange(500, 1900)
plot_range = np.arange(1200, 1400)


#
# Set up test image
#
dark_level = 300
read_noise = 4.5

image_model = np.zeros((2000, 100))
# image_model += dark_level
image_model += np.random.poisson(dark_level, image_model.shape)
# image_model[20:1900,:]+=dark_level
# image_model += np.random.normal(dark_level,np.sqrt(dark_level)/1000,image_model.shape)


parallel_traps = [
    arcticpy.TrapInstantCapture(density=1.0, release_timescale=(-1 / np.log(0.5))),
    # arcticpy.TrapInstantCapture(density=10.0, release_timescale=100),
    # arcticpy.TrapSlowCapture(density=10.0, release_timescale=(-1/np.log(0.5)), capture_timescale=0.001),
    # arcticpy.TrapSlowCapture(density=10.0, release_timescale=(4), capture_timescale=0.001),
    # arcticpy.TrapInstantCaptureContinuum(density=10.0, release_timescale=(1.), release_timescale_sigma=0.1),
    # arcticpy.TrapSlowCaptureContinuum(density=10.0, release_timescale=(1.), capture_timescale=1, release_timescale_sigma=0.1),
]

parallel_ccd = arcticpy.CCD(full_well_depth=10000, well_fill_power=1.0)
parallel_roe = arcticpy.ROE(
    empty_traps_between_columns=True,
    empty_traps_for_first_transfers=False,
    overscan_start=1900,
    prescan_offset=50,
)


#
# Add CTI
#

start = time.time_ns()

image_post_cti = arcticpy.add_cti(
    image=image_model,
    parallel_traps=parallel_traps,
    parallel_ccd=parallel_ccd,
    parallel_roe=parallel_roe,
    parallel_express=parallel_express,
    parallel_prune_n_electrons=parallel_prune_n_electrons,
    parallel_prune_frequency=parallel_prune_frequency,
    allow_negative_pixels=1,
    verbosity=0,
)

print(f"Clocking Time Noisy = {((time.time_ns() - start)/1e9)} s")
print("mean level (true)    ", np.mean(image_model[stats_range, :]))
print("mean level (trailed) ", np.mean(image_post_cti[stats_range, :]))


#
# Add read noise
#
read_noise_image = np.random.normal(0, read_noise, image_model.shape)
image_pre_cti_noisy = image_model + read_noise_image
image_post_cti_noisy = image_post_cti + read_noise_image

read_noise_obj = arcticpy.ReadNoise(read_noise)
read_noise_obj.set_arctic_parameters(
    parallel_traps=parallel_traps,
    parallel_ccd=parallel_ccd,
    parallel_roe=parallel_roe,
    parallel_express=parallel_express,
    parallel_prune_n_electrons=parallel_prune_n_electrons,
    parallel_prune_frequency=parallel_prune_frequency,
)
read_noise_obj.optimise_SR_fraction_from_image(image_post_cti_noisy)
print("Optimum S+R fraction: ", read_noise_obj.SRfrac_optimised)
read_noise_obj.SRfrac_optimised = 0.3

#
# Correct CTI
#
start = time.time_ns()
image_corrected_noisy = arcticpy.remove_cti(
    image=image_post_cti_noisy,
    n_iterations=19,
    parallel_traps=parallel_traps,
    parallel_ccd=parallel_ccd,
    parallel_roe=parallel_roe,
    parallel_express=parallel_express,
    parallel_prune_n_electrons=parallel_prune_n_electrons,
    parallel_prune_frequency=parallel_prune_frequency,
    allow_negative_pixels=1,
    read_noise=read_noise_obj,
    verbosity=0,
)
print(f"Correction Time Noisy = {((time.time_ns() - start)/1e9)} s")
print(
    "mean level (correct) ",
    np.mean(image_corrected_noisy[stats_range, :]),
    " +/-",
    np.std(image_corrected_noisy[stats_range, :])
    / np.sqrt(np.size(image_corrected_noisy[stats_range, :])),
)


#
# Plot
#
n_rows_in_image, n_columns_in_image = image_model.shape
pixels = np.arange(n_rows_in_image)
colours = ["#1199ff", "#ee4400", "#7711dd", "#44dd44", "#775533"]
fig, ax = plt.subplots()
ax.plot(
    pixels[plot_range], image_model[plot_range, 0], alpha=0.8, label="No read noise"
)
# ax.plot(pixels[0:50], image_post_cti_nonoise[0:50,0]-image_model[0,0:50], alpha=0.8, label="%d")
ax.plot(
    pixels[plot_range],
    image_pre_cti_noisy[plot_range, 0],
    alpha=0.8,
    label="Ideal correction",
)
ax.plot(
    pixels[plot_range],
    image_corrected_noisy[plot_range, 0],
    alpha=0.8,
    label="Achieved correction",
)
ax.plot(
    pixels[plot_range],
    image_post_cti_noisy[plot_range, 0],
    alpha=0.8,
    label="Downloaded",
)

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

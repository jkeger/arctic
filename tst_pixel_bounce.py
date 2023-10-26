import numpy as np
import arcticpy
import time
import matplotlib.pyplot as plt
import tty
import sys
import termios

np.set_printoptions(linewidth=205, edgeitems=56, suppress=True)


#
# Test pixel bounce models
#
pixel_bounce_kA = -0.1
pixel_bounce_kv = 0
pixel_bounce_omega = 10
pixel_bounce_gamma = 0.9

# pixel_bounce_omega=8
# pixel_bounce_gamma=0.8

# pixel_bounce_kA=2e-1
# pixel_bounce_kv=-1e-1
# pixel_bounce_omega=1.2
# pixel_bounce_gamma=2

#
# Set up test image
#
image_pre_cti = np.zeros((10, 50)) + 200
image_pre_cti[:, 5:40] += 700
start = time.time_ns()


#
# Add pixel bounce
#
pixel_bounce = arcticpy.PixelBounce(kA=-0.1, kv=0, omega=10, gamma=0.9)
image_post_cti = arcticpy.add_pixel_bounce(
    image=image_pre_cti,
    parallel_window_stop=10,
    serial_window_stop=50,
    pixel_bounce=pixel_bounce,
)
#
# Remove pixel bounce
#
image_corrected = arcticpy.remove_pixel_bounce(
    image=image_post_cti,
    n_iterations=5,
    parallel_window_stop=10,
    serial_window_stop=50,
    pixel_bounce=pixel_bounce,
)

print(f"Time taken = {((time.time_ns() - start)/1e9)} s")


#
# Plot
#
n_rows_in_image, n_columns_in_image = image_post_cti.shape
pixels = np.arange(n_columns_in_image)
colours = ["#1199ff", "#ee4400", "#7711dd", "#44dd44", "#775533"]
fig, ax = plt.subplots()
ax.plot(
    pixels[0:50],
    image_post_cti[0, 0:50] - image_pre_cti[0, 0:50],
    alpha=0.8,
    label="%d",
)
ax.plot(pixels[0:50], image_post_cti[0, 0:50] / 10 - 20, alpha=0.8, label="%d")
ax.plot(pixels[0:50], image_corrected[0, 0:50] / 10 - 20, alpha=0.8, label="%d")

ax.set(xlabel="pixel", ylabel="offset bias [n_e]", title="Effect of pixel bounce")
ax.grid()
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

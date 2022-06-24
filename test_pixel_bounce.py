import numpy as np
import arcticpy
import time
import matplotlib.pyplot as plt
import tty
import sys
import termios
np.set_printoptions(linewidth=205,edgeitems=56,suppress=True)


#
# Test pixel bounce models
#
pixel_bounce_kA=-0.1
pixel_bounce_kv=0#4e-1
pixel_bounce_omega=-10
pixel_bounce_gamma=10#.9

#pixel_bounce_omega=8
#pixel_bounce_gamma=0.8

#pixel_bounce_kA=2e-1
#pixel_bounce_kv=-1e-1
#pixel_bounce_omega=1.2
#pixel_bounce_gamma=2

#
# Set up test image
#
image_model = np.zeros((1,50))+200
image_model[:,10:40]+=700
#image_model[500:505,:]+=700
#image_model[1000:1005,:]+=700
#image_model[1500:1505,:]+=700

serial_traps = [
    arcticpy.TrapInstantCapture(density=0.0, release_timescale=(-1/np.log(0.5))),
    #arcticpy.TrapInstantCapture(density=10.0, release_timescale=100),
    #arcticpy.TrapSlowCapture(density=10.0, release_timescale=(-1/np.log(0.5)), capture_timescale=0.001),
    #arcticpy.TrapSlowCapture(density=10.0, release_timescale=(4), capture_timescale=0.001),
    #arcticpy.TrapInstantCaptureContinuum(density=10.0, release_timescale=(1.), release_timescale_sigma=0.1),
    #arcticpy.TrapSlowCaptureContinuum(density=10.0, release_timescale=(1.), capture_timescale=1, release_timescale_sigma=0.1),
]

serial_ccd = arcticpy.CCD(full_well_depth=1000, well_fill_power=1.0)
serial_roe = arcticpy.ROE(
    empty_traps_between_columns=True,
    empty_traps_for_first_transfers=False,
    pixel_bounce_kA=pixel_bounce_kA,
    pixel_bounce_kv=pixel_bounce_kv,
    pixel_bounce_omega=pixel_bounce_omega,
    pixel_bounce_gamma=pixel_bounce_gamma
    #overscan_start=1990
)
#parallel_roe.prescan_offset=10



#
# Noise-free image
#

image_pre_cti = image_model
#print(image_pre_cti[0:10,0])

start = time.time_ns()

image_post_cti = arcticpy.add_cti(
    image=image_pre_cti,
    serial_traps=serial_traps,
    serial_ccd=serial_ccd,
    serial_roe=serial_roe,
    serial_express=1,
    verbosity=0
)
#image_post_cti = arcticpy.add_cti(
#    image=image_pre_cti,
#    serial_ccd=serial_ccd,
#    serial_roe=serial_roe,
#    serial_express=1,
#    verbosity=0
#s)

print(f"Clocking Time No Noise = {((time.time_ns() - start)/1e9)} s")
print(image_post_cti[0,:])

n_rows_in_image, n_columns_in_image = image_post_cti.shape
pixels = np.arange(n_columns_in_image)
colours = ["#1199ff", "#ee4400", "#7711dd", "#44dd44", "#775533"]
fig, ax = plt.subplots()
ax.plot(pixels, image_post_cti[0,:]-image_pre_cti[0,:], alpha=0.8, label="%d")
ax.plot(pixels, image_post_cti[0,:]/10-20, alpha=0.8, label="%d")

ax.set(xlabel='pixel', ylabel='offset bias [n_e]',
       title='Effect of pixel bounce')
ax.grid()
plt.tight_layout()
#plt.ioff()
#plt.pause(0.01)
plt.show(block=False)   
#plt.gcf().canvas.draw_idle()
#plt.gcf().canvas.start_event_loop(0.3)

#import time
#time.sleep(2)
# Press enter to continue
input("Press Enter to continue...")
# Press any key to continue
#print("Press any key to continue...")
#orig_settings = termios.tcgetattr(sys.stdin)
#tty.setcbreak(sys.stdin)
#x = 0
#while x == 0:
#    x=sys.stdin.read(1)[0]
#termios.tcsetattr(sys.stdin, termios.TCSADRAIN, orig_settings)  

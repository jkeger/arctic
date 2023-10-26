import arcticpy as arctic
import autoarray as aa
import time
import numpy as np

data_path = "/Users/rjm/GitHub/testdata/je0o30lzq"

# Load each quadrant of the image  (see pypi.org/project/autoarray)
image_A, image_B, image_C, image_D = [
    aa.acs.ImageACS.from_fits(
        file_path=data_path + "_raw.fits",
        quadrant_letter=quadrant,
        bias_subtract_via_bias_file=True,
        bias_subtract_via_prescan=True,
    ).native
    for quadrant in ["A", "B", "C", "D"]
]

# Automatic CTI model  (see CTI_model_for_HST_ACS() in arcticpy/src/cti.py)
date = 2400000.5 + image_A.header.modified_julian_date
roe, ccd, traps = arctic.CTI_model_for_HST_ACS(date)


noise = arctic.ReadNoise(sigma_readnoise=4.5)


background_level = 2.5 * np.median(image_A) - 1.5 * np.mean(image_A)
background_level = np.max(np.median(image_A), 0)

print(background_level)

noise.optimise_parameters(
    background_level,
    n_pixels=2066,
    parallel_traps=traps,
    parallel_ccd=ccd,
    parallel_roe=roe,
)  # ,
#        serial_traps=traps,
#        serial_ccd=ccd,
#        serial_roe=roe)


# Or manual CTI model  (see class docstrings in src/<traps,roe,ccd>.cpp)
# traps = [
#    arctic.TrapInstantCapture(density=0.6, release_timescale=0.74),
#    arctic.TrapInstantCapture(density=1.6, release_timescale=7.70),
#    arctic.TrapInstantCapture(density=1.4, release_timescale=37.0),
# ]
# roe = arctic.ROE()
# ccd = arctic.CCD(full_well_depth=84700, well_fill_power=0.478)


# Remove CTI  (see remove_cti() in src/cti.cpp)
# start = time.time_ns()
# image_out_A, image_out_B, image_out_C, image_out_D = [
#    arctic.remove_cti(
#           image=image,
#           n_iterations=5,
#           parallel_roe=roe,
#           parallel_ccd=ccd,
#           parallel_traps=traps,
#           parallel_express=5,
#           verbosity=1,
#    )
#    for image in [image_A, image_B, image_C, image_D]
# ]
# print(f"Time taken = {((time.time_ns() - start)/1e9)} s")

# Save the corrected image
# aa.acs.output_quadrants_to_fits(
#    file_path=data_path + "_out.fits",
#    quadrant_a=image_out_A,
#    quadrant_b=image_out_B,
#    quadrant_c=image_out_C,
#    quadrant_d=image_out_D,
#    header_a=image_A.header,
#    header_b=image_B.header,
#    header_c=image_C.header,
#    header_d=image_D.header,
#    overwrite=True,
# )

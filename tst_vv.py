import arcticpy as arctic
import autoarray as aa
import time

image_path = "/Users/rjm/GitHub/testdata/je0o30lzq"

# Load each quadrant of the image  (see pypi.org/project/autoarray)
image_A_raw, image_B_raw, image_C_raw, image_D_raw = [
    aa.acs.ImageACS.from_fits(
        file_path=image_path + "_raw.fits",
        quadrant_letter=quadrant,
        bias_subtract_via_bias_file=True,
        bias_subtract_via_prescan=True,
    ).native
    for quadrant in ["A", "B", "C", "D"]
]



# Automatic CTI model  (see CTI_model_for_HST_ACS() in arcticpy/src/cti.py)
date = 2400000.5 + image_A_raw.header.modified_julian_date
parallel_roe, parallel_ccd, parallel_traps, serial_roe, serial_ccd, serial_traps = arctic.CTI_model_for_HST_ACS(date)

print("Correcting one CCD quadrant on the fly")
image_A_corrected_otf = arctic.remove_cti(image_A_raw,
    n_iterations=3,
    parallel_ccd=parallel_ccd,
    parallel_roe=parallel_roe,
    parallel_traps=parallel_traps,
    parallel_express=5,
    #read_noise=4.0,
    serial_ccd=serial_ccd,
    serial_roe=serial_roe,
    serial_traps=serial_traps,
    verbosity=1)
   
print("Here's one I made earlier")
image_A_corrected, image_B_corrected, image_C_corrected, image_D_corrected = [
    aa.acs.ImageACS.from_fits(
        file_path=image_path + "_out.fits",
        quadrant_letter=quadrant,
        bias_subtract_via_bias_file=True,
        bias_subtract_via_prescan=True,
    ).native
    for quadrant in ["A", "B", "C", "D"]
]


# Set up V&V test
vv=arctic.VVTestBench(
    parallel_roe=parallel_roe, 
    parallel_ccd=parallel_ccd, 
    parallel_traps=parallel_traps, 
    serial_roe=serial_roe, 
    serial_ccd=serial_ccd, 
    serial_traps=serial_traps, 
    sum_of_exponentials=True,
    verbose=True
)
before=vv.test(image_A_raw)
after=vv.test(image_A_corrected,
          parallel_valid_columns = before.parallel.valid_columns,
          parallel_pixels_pre_cti = before.parallel.pixels_pre_cti,
          parallel_fit_bias = True, 
          parallel_model_bias = before.parallel.best_fit_bias)
before.parallel.plot()

import arcticpy as arctic
import autoarray as aa
import time

image_path = "/Users/rjm/GitHub/testdata/je0o30lzq"

# Load each quadrant of the image  (see pypi.org/project/autoarray)
image_A, image_B, image_C, image_D = [
    aa.acs.ImageACS.from_fits(
        file_path=image_path + "_raw.fits",
        quadrant_letter=quadrant,
        bias_subtract_via_bias_file=True,
        bias_subtract_via_prescan=True,
    ).native
    for quadrant in ["A", "B", "C", "D"]
]

# Automatic CTI model  (see CTI_model_for_HST_ACS() in arcticpy/src/cti.py)
date = 2400000.5 + image_A.header.modified_julian_date
roe, ccd, traps = arctic.CTI_model_for_HST_ACS(date)

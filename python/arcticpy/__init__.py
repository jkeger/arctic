from arcticpy.cti import add_cti, remove_cti, CTI_model_for_HST_ACS
from arcticpy.pixel_bounce import PixelBounce, add_pixel_bounce, remove_pixel_bounce
from arcticpy.ccd import CCDPhase, CCD
from arcticpy.roe import ROE, ROEChargeInjection, ROETrapPumping
from arcticpy.traps import (
    TrapInstantCapture,
    TrapSlowCapture,
    TrapInstantCaptureContinuum,
    TrapSlowCaptureContinuum,
)
from arcticpy.read_noise import ReadNoise

try:
    from arcticpy.wrapper import (
        cy_print_array as print_array,
        cy_print_array_2D as print_array_2D,
    )
except ModuleNotFoundError:
    from wrapper import (
        cy_print_array as print_array,
        cy_print_array_2D as print_array_2D,
    )

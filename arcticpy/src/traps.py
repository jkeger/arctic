import numpy as np


class AbstractTrap:

    def __init__(
        self,
        density=1.0,
        release_timescale=1.0,
    ):
        self.density = density
        self.release_timescale = release_timescale

class TrapInstantCapture(AbstractTrap):
    def __init__(
        self,
        density=1.0,
        release_timescale=1.0,
    #    fractional_volume_none_exposed=0.0,
    #    fractional_volume_full_exposed=0.0,
    ):

        super().__init__(density=density, release_timescale=release_timescale)
        # self.fractional_volume_none_exposed = fractional_volume_none_exposed
        # self.fractional_volume_full_exposed = fractional_volume_full_exposed

        self.fractional_volume_none_exposed = 0.0
        self.fractional_volume_full_exposed = 0.0

class TrapSlowCapture(TrapInstantCapture):
    def __init__(self, density=1.0, release_timescale=1.0, capture_timescale=0.0):
        super().__init__(density, release_timescale)

        self.capture_timescale = capture_timescale


class TrapInstantCaptureContinuum(TrapInstantCapture):
    def __init__(self, density=1.0, release_timescale=1.0, release_timescale_sigma=0.0):
        super().__init__(density, release_timescale)

        self.release_timescale_sigma = release_timescale_sigma


class TrapSlowCaptureContinuum(TrapSlowCapture):
    def __init__(
        self,
        density=1.0,
        release_timescale=1.0,
        release_timescale_sigma=0.0,
        capture_timescale=0.0,
    ):
        super().__init__(density, release_timescale)

        self.release_timescale_sigma = release_timescale_sigma
        self.capture_timescale = capture_timescale

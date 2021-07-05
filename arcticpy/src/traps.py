import numpy as np


class TrapInstantCapture(object):
    def __init__(
        self,
        density=1.0,
        release_timescale=1.0,
    ):
        self.density = density
        self.release_timescale = release_timescale


class TrapSlowCapture(TrapInstantCapture):
    def __init__(
        self,
        density=1.0,
        release_timescale=1.0,
        capture_timescale=0.0,
    ):
        super().__init__(density, release_timescale)

        self.capture_timescale = capture_timescale


class TrapInstantCaptureContinuum(TrapInstantCapture):
    def __init__(
        self,
        density=1.0,
        release_timescale=1.0,
        release_timescale_sigma=0.0,
    ):
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

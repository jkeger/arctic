import numpy as np


class Trap(object):
    def __init__(
        self,
        density,
        release_timescale,
        capture_timescale,
    ):
        self.density = density
        self.release_timescale = release_timescale
        self.capture_timescale = capture_timescale


class TrapInstantCapture(Trap):
    def __init__(
        self,
        density,
        release_timescale,
    ):
        super().__init__(density, release_timescale, 0.0)

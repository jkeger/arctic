import numpy as np


class Trap(object):
    def __init__(
        self,
        density=1.0,
        release_timescale=1.0,
        capture_timescale=0.0,
    ):
        self.density = density
        self.release_timescale = release_timescale
        self.capture_timescale = capture_timescale


class TrapInstantCapture(Trap):
    def __init__(
        self,
        density=1.0,
        release_timescale=1.0,
    ):
        super().__init__(density, release_timescale, 0.0)

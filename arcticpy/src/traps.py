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

    def poisson_density_from(self, total_pixels, seed=-1):

        if seed == -1:
            seed = np.random.randint(
                0, int(1e9)
            )

        np.random.seed(seed)

        density_pixels = self.density * total_pixels
        poisson_density_pixels = np.random.poisson(density_pixels)
        poisson_density_per_pixel = poisson_density_pixels / total_pixels

        return TrapInstantCapture(density=poisson_density_per_pixel, release_timescale=self.release_timescale)

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

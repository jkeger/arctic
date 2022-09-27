import numpy as np

from autoconf.dictable import Dictable

class AbstractTrap(Dictable):
    def __init__(self, density=1.0, release_timescale=1.0):
        self.density = density
        self.release_timescale = release_timescale

    @property
    def delta_ellipticity(self):
        raise NotImplementedError

class TrapInstantCapture(AbstractTrap):
    def __init__(
        self,
        density:float=1.0,
        release_timescale=1.0,
        fractional_volume_none_exposed=0.0,
        fractional_volume_full_exposed=0.0,
    ):
        super().__init__(density, release_timescale)

        self.fractional_volume_none_exposed = fractional_volume_none_exposed
        self.fractional_volume_full_exposed = fractional_volume_full_exposed

    @property
    def delta_ellipticity(self):

        a = 0.05333
        d_a = -0.03357
        d_p = 1.628
        d_w = 0.2951
        g_a = 0.09901
        g_p = 0.4553
        g_w = 0.4132

        return 4.0 * self.density * (
            a
            + d_a * (np.arctan((np.log10(self.release_timescale) - d_p) / d_w))
            + (
                g_a
                * np.exp(
                    -((np.log10(self.release_timescale) - g_p) ** 2.0) / (2 * g_w ** 2.0)
                )
            )
        )

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

class TrapSlowCapture(AbstractTrap):
    def __init__(self, density=1.0, release_timescale=1.0, capture_timescale=0.0):
        super().__init__(density, release_timescale)

        self.capture_timescale = capture_timescale


class TrapInstantCaptureContinuum(AbstractTrap):
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

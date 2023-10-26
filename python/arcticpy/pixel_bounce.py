import numpy as np


class PixelBounce:

    """
    In flat fields acquired during ground testing of Euclid CCDs, the serial
    overscan shows spurious features that may be a combination of CTI and pixel
    bounce. In particular, the spurious signal dies away over several pixels but
    is not monotonic. An increase between pixels 2 and 3 of the overscan region
    cannot be due to CTI.

    To model pixel bounce, we regard the reference voltage as receiving an
    impulse when the signal voltage changes suddenly, then returning to ground
    as a heavily damped harmonic oscillator. Correlated double sampling then
    produces a spurious, oscillating bias offset in the first few pixels after
    the change in signal.

    Parameters of a damped harmonic oscillator are
    * the natural frequency and damping - both of which presumably depend on
      capacitance between the sampling phase and ground.
    * an initial amplitude and velocity of oscillation - which could depend on
      the coupling between the signal and reference voltage, and the clocking time,
      during which the spurious bias might start to oscillate unnoticed.


    Parameters
    ----------
    kA : float
        Initial condition of reference volatage in the pixel after a change in
        signal voltage.
    kv : float
        Initial condition of rate of change of reference volatage in the pixel
        after a change in signal voltage.
    gamma : float
        Damping coefficicent of oscillations in the reference voltage, expressed
        as a ~half life in units of the time between clocks.
    omega : float
        Frequency of oscillations in the reference voltage, in units (per pixel)
        i.e. freq in Hz * clock speed.
    """

    def __init__(self, kA=0, kv=0, gamma=1.0, omega=1.0):
        if gamma < 0:
            raise Exception("Damping factor gamma cannot be negative")
        if omega < 0:
            raise Exception("Oscillation frequency omega should not be negative")
        self.kA = kA
        self.kv = kv
        self.gamma = gamma
        self.omega = omega

    @property
    def omega0(self):
        return np.sqrt(omega**2 + gamma**2)  # natural frequency of oscillator

    def add_pixel_bounce(
        self,
        image,
        parallel_window_start=0,
        parallel_window_stop=-1,
        serial_window_start=0,
        serial_window_stop=-1,
        verbosity=1,
    ):
        """
        Add pixel bounce to an image, modelled as Damped Harmonic Oscillations (DHO)
        in a CCD's reference voltage, driven by sudden changes in the signal. This
        creates spurious features in the serial (same row) direction away from any
        gradient in the image.

        Parameters
        ----------
        image : [[float]]
            The input array of pixel values, assumed to be in units of electrons.

            The first dimension is the "row" (y) index, the second is the "column"
            (x) index. Pixel bounce is only ever added in the x direction (ie during
            serial readout for images oriented as usual in ArCTIc).

        parallel_window_start/stop : int
            First and last row of pixels (in the y direction) to process, for speed.
            Default is to process the entire image.

        serial_window_start/stop : int
            First and last column of pixels (in the x direction) to process, for speed.
            Default is to process the entire image.

        Returns
        -------
        image : [[float]]
            The output array of pixel values.
        """

        # Parse inputs needed to process only a subset of the image
        image = np.copy(image).astype(np.double)
        if (self.kA == 0) and (self.kv == 0):
            return image
        n_y, n_x = image.shape
        if parallel_window_stop == -1:
            parallel_window_stop = n_y
        if serial_window_stop == -1:
            serial_window_stop = n_x
        image_subarray = image[
            parallel_window_start:parallel_window_stop,
            serial_window_start:serial_window_stop,
        ]
        n_y, n_x = image_subarray.shape

        # Pre-calcualte useful quantities from eqn (43) of
        # Cieslinski & Ratkiewicz (2005) https://arxiv.org/abs/physics/0507182
        epsilon = 1
        coeffA = 2 * np.exp(-1 * self.gamma * epsilon) * np.cos(self.omega * epsilon)
        coeffB = np.exp(-2 * self.gamma * epsilon)

        # Initialise bias offset voltage, which should settle during prescan
        # of each row
        bias = np.zeros(image_subarray.shape)
        biasm1 = np.zeros(n_y)

        # Read out (a column of) pixels along a row, starting at second pixel (this
        # assumes the first pixel in each row cannot be affected by pixel bounce, as
        # the electronics have been reset and stabilised during prescan).
        for i in range(1, n_x):
            # Store previous values of bias, so difference equation can
            # compute rates of change
            biasm2 = biasm1.copy()
            biasm1 = bias[:, i - 1].copy()

            # What electronic impulse is being experienced?
            delta = image_subarray[:, i] - image_subarray[:, i - 1]

            # Impose this (linearly) on the difference equation,
            # as one term that creates a bias offset (propto pixel_bounce_kA)
            # and one that creates a rate of change of bias (propto pixel_bounce_kV)
            biasm1 += (self.kA - self.kv) * delta
            biasm2 += (self.kA - 2 * self.kv) * delta

            # DHO difference equation, Cieslinski & Ratkiewicz (2005) eqn (43)
            bias[:, i] = coeffA * biasm1 - coeffB * biasm2

        # Spurious bias caused by correlated double sampling
        image[
            parallel_window_start:parallel_window_stop,
            serial_window_start:serial_window_stop,
        ] -= bias[:, :]

        return image

    def remove_pixel_bounce(
        self,
        image,
        n_iterations,
        parallel_window_start=0,
        parallel_window_stop=-1,
        serial_window_start=0,
        serial_window_stop=-1,
        verbosity=1,
    ):
        """
        Remove the effect of pixel bounce, by iterating towards an image that, when
        pixel bounce is added to it, recovers the input.
        """
        image = np.copy(image).astype(np.double)
        image_remove_pixel_bounce = np.copy(image).astype(np.double)
        for iteration in range(1, n_iterations + 1):
            if verbosity >= 1:
                print("Iter %d: " % iteration, end="", flush=True)

            # Iteratively add pixel bounce to a model of the corrected image
            image_add_pixel_bounce = self.add_pixel_bounce(
                image_remove_pixel_bounce,
                parallel_window_start=parallel_window_start,
                parallel_window_stop=parallel_window_stop,
                serial_window_start=serial_window_start,
                serial_window_stop=serial_window_stop,
                verbosity=verbosity,
            )

            # Improve the estimate of the image with pixel bounce removed
            image_remove_pixel_bounce += image - image_add_pixel_bounce

        return image_remove_pixel_bounce

    def add_pixel_bounce_slow(self, image, do_Plot=False):
        """
        C++ style version, looping over each row one at a time
        """
        # Pre-calcualte (once) useful quantities from eqn (43) of
        # Cieslinski & Ratkiewicz (2005) https://arxiv.org/abs/physics/0507182
        epsilon = 1
        # omega = np.sqrt(self.omegaO**2 - self.gamma**2)
        coeffA = 2 * np.exp(-1 * self.gamma * epsilon) * np.cos(self.omega * epsilon)
        coeffB = np.exp(-2 * self.gamma * epsilon)
        biasm1 = 0.0

        # Read out one column of pixels through the (column of) traps
        n_rows_in_image, n_columns_in_image = image.shape
        import copy

        for row_index in range(n_rows_in_image):
            print("Bouncing, one row at a time")

            # Initialise bias offset voltage, which should settle during prescan
            # of each row
            bias = np.zeros((1, n_columns_in_image))

            # Each pixel
            # for row_index in window_row_range:
            for column_index in range(1, n_columns_in_image):
                # Store previous values of bias, so difference equation can
                # compute rates of change
                biasm2 = copy.copy(biasm1)
                biasm1 = bias[row_index, column_index - 1]

                # What electronic impulse is being experienced?
                delta = (
                    image[row_index, column_index] - image[row_index, column_index - 1]
                )

                # Impose this (linearly) on the difference equation,
                # as one term that creates a bias offset (propto pixel_bounce_kA)
                # and one that creates a rate of change of bias (propto pixel_bounce_kV)
                biasm1 += (self.kA - self.kv) * delta
                biasm2 += (self.kA - 2 * self.kv) * delta

                # DHO difference equation, Cieslinski & Ratkiewicz (2005) eqn (43)
                bias[0, column_index] = coeffA * biasm1 - coeffB * biasm2

            image[row_index : row_index + 1, :] -= bias

        return image


"""
Standalone functions to call the above, but mirroring syntax of add_cti() and remove_cti()
"""


def add_pixel_bounce(
    image,
    pixel_bounce=None,
    parallel_window_start=0,
    parallel_window_stop=-1,
    serial_window_start=0,
    serial_window_stop=-1,
    verbosity=1,
):
    if pixel_bounce is not None:
        image = pixel_bounce.add_pixel_bounce(
            image,
            parallel_window_start=parallel_window_start,
            parallel_window_stop=parallel_window_stop,
            serial_window_start=serial_window_start,
            serial_window_stop=serial_window_stop,
            verbosity=verbosity,
        )
    return image


def remove_pixel_bounce(
    image,
    n_iterations,
    pixel_bounce=None,
    parallel_window_start=0,
    parallel_window_stop=-1,
    serial_window_start=0,
    serial_window_stop=-1,
    verbosity=1,
):
    if pixel_bounce is not None:
        image = pixel_bounce.remove_pixel_bounce(
            image,
            n_iterations,
            parallel_window_start=parallel_window_start,
            parallel_window_stop=parallel_window_stop,
            serial_window_start=serial_window_start,
            serial_window_stop=serial_window_stop,
            verbosity=verbosity,
        )
    return image

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
    * an initial amplitude and phase of oscillation - which could depend on
      the coupling between the signal and reference voltage, and the clocking time,
      during which the spurious bias might start to oscillate unnoticed.


    Parameters
    ----------
    xi : float
        Strength of pixel bounce. A change in signal of delta / xi_units will induce 
        oscillations of initial amplitude ( delta * xi )^mu.
    xi_units : float
    	Rescale the number of electrons producing a given amplitude of oscillations, for
    	better convergence of a fit in which mu is a free parameter.
    phi : float
        Starting phase of the oscillations in the reference voltage.
    gamma : float
        Damping coefficicent of oscillations in the reference voltage, expressed
        as a ~half life in units of the time between clocks.
    omega : float
        Frequency of oscillations in the reference voltage, in units (per pixel)
        i.e. freq in Hz * clock speed.
    mu : float
    	Power to which delta is scaled when computing a nonlinear bounce response.
    oversample : int
    	Factor by which to oversample the pixels, when calculating the oscillations. High
    	values make the calculation more accurate. By default will aim for ~1% precision.
    """

    def __init__(self, xi=0.0, phi=0.0, gamma=0.0, omega=1.0, mu=1.0, xi_units=1.0):
        if gamma < 0:
            raise Exception("Damping factor gamma cannot be negative")
        if omega < 0:
            raise Exception("Oscillation frequency omega should not be negative")
        self.xi = xi
        self.phi = phi
        self.gamma = gamma
        self.omega = omega
        self.mu = mu
        self.xi_units = xi_units

    @property
    def omega0(self):
        """
        The natural frequency the oscillator would have had in the absence of damping.
        """
        return np.sqrt(self.omega**2 + self.gamma**2)
    
    @property
    def period(self):
        return 2 * np.pi / self.omega
        
    def impulse(self, delta, epsilon):
        """
        The impulse on the electronic bias caused by a change in the signal level
        on the readout node. impulse and phi could be any function of delta
        """
        impulse = np.sign(delta) * ( np.abs(delta) / self.xi_units ) ** self.mu
        phi = self.phi
        # Convert to initial offset and initial velocity form
        J_A = impulse * np.cos(phi)
        J_v = impulse * np.sin(phi) * self.omega * epsilon
        return J_A, J_v

    def perturb_bias(
            self,
            image,
            oversample,
            parallel_window_start,
            parallel_window_stop,
            serial_window_start,
            serial_window_stop,
            verbosity
    ):
        """
        Applies pixel bounce to the effective bias level against which an image is
        compared during readout.

        This function is separate from `add_pixel_bounce` to allow the bias to be 
        calculated multiple times (e.g. for multiple pixel bounce classes) and then 
        all added to the same image, so their order has no effect.

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
        oversample : int
    	    Factor by which to oversample the pixels, when calculating the oscillations. 
    	    High values make the calculation more accurate.
        
        Returns
        -------
        bias : [[float]]
            The output bias image array of pixel values.
        """
        if (self.xi == 0):
            return np.zeros(image.shape)
        
        # Parse inputs needed to process only a subset of the image
        n_y, n_x = image.shape
        if parallel_window_stop == -1: parallel_window_stop = n_y
        if serial_window_stop == -1: serial_window_stop = n_x

        # Supersample image for numerical stability (prevent aliasing)
        epsilon = 1 # Evaluate result every 1 pixel
        if (oversample is None): # Calculate a sensible default oversampling factor
        	if (self.omega > 0):
	        	# 500 samples per oscillation empirically gives ~1% precision
	        	desired_samples_per_oscillation = 500
	        	oversample = desired_samples_per_oscillation / self.period
        		# Limit on oversampling to prevent excessive runtime
	        	max_oversample = 200
        		oversample = min( oversample, max_oversample )
        	else:
        		oversample = 1
        if (oversample > 1): # Oversample the image 
        	i_oversample = int(np.ceil( oversample ))
        	image = np.repeat(image, i_oversample, axis=1)
        	serial_window_start *= i_oversample
        	serial_window_stop *= i_oversample
        	epsilon /= i_oversample
        #print("Oversampling factor",i_oversample)

        # Pre-calcualte useful quantities from eqn (43) of
        # Cieslinski & Ratkiewicz (2005) https://arxiv.org/abs/physics/0507182        
        coeffA = 2 * np.exp(-1 * self.gamma * epsilon) * np.cos(self.omega * epsilon)
        coeffB = np.exp(-2 * self.gamma * epsilon)
        cos_phi = np.cos(self.phi)
        omega_sin_phi = np.sin(self.phi) * self.omega * epsilon
        xi_sin_minus_cos = self.xi / self.xi_units * ( omega_sin_phi - cos_phi )
        xi_2sin_minus_cos = self.xi / self.xi_units * ( 2 * omega_sin_phi - cos_phi ) * (
    		1 + self.gamma * epsilon )           
        # Not sure why, but empirically the 1+gamma term improves convergence.
        # There must be second-order terms that I haven't worked out analytically.
        #bias_im2 += self.xi * ( (2 * J_v - J_A ) 
        #	+ ( 1 - self.omega * epsilon ) * self.gamma * epsilon * ( 4 * J_v - J_A ) 
        #)
        
        # Initialise bias offset voltage, which should settle during prescan
        # of each row
        bias = np.zeros(image.shape)
        bias_im1 = np.zeros(parallel_window_stop - parallel_window_start)

        # Read out (a column of) pixels along a row, starting at second pixel (this
        # assumes the first pixel in each row cannot be affected by pixel bounce, as
        # the electronics have been reset and stabilised during prescan).
        for i in range(serial_window_start + 1, serial_window_stop):
            
            # Store previous values of bias, so difference equation can
            # compute rates of change
            bias_im2 = bias_im1.copy() # Could have been bias[...,i-2] but this works
            bias_im1 = bias[parallel_window_start:parallel_window_stop, i - 1].copy()

            # Calculate the change of charge now at the sense node
            delta = (
                image[parallel_window_start:parallel_window_stop, i] -
                image[parallel_window_start:parallel_window_stop, i - 1]
            )
            
            # What electronic impulse is being experienced?
            if (self.mu != 1.0): # General, potentially nonlinear version
            	J_A, J_v = self.impulse(delta, epsilon)
            	
            	# Impose this (linearly) on the difference equation,
            	# as one term that creates a bias offset (propto pixel_bounce_kA)
            	# and one that creates a rate of change of bias (propto pixel_bounce_kV)
            	bias_im1 += self.xi * ( J_v - J_A )
            	bias_im2 += self.xi * ( 2 * J_v - J_A ) * ( 1 + self.gamma * epsilon )
            	# Not sure why, but empirically the 1+gamma term improves convergence.
            	# There must be second-order terms that I haven't worked out analytically.
            	# Empirically, they look something like
            	#bias_im2 += self.xi * ( (2 * J_v - J_A ) 
            	#	+ ( 1 - self.omega * epsilon ) * self.gamma * epsilon * ( 4 * J_v - J_A ) 
            	#)     
            else: # 2xfaster equivalent of above, when phi is fixed & response is linear
            	bias_im1 += delta * xi_sin_minus_cos
            	bias_im2 += delta * xi_2sin_minus_cos
            		
            # DHO difference equation, Cieslinski & Ratkiewicz (2005) eqn (43)
            bias[parallel_window_start:parallel_window_stop, i] = \
                coeffA * bias_im1 - coeffB * bias_im2

        # Undo the supersampling of the image
        if (oversample > 1):
        	#bias = bias.reshape(n_y, n_x, i_oversample).mean(axis=2) # Mean of values
        	#bias = bias[:, i_oversample-1::i_oversample] # End of each oversampled bin
        	bias = bias[:, ::i_oversample] # Take every nth value

        return bias

    def add_pixel_bounce(
        self,
        image,
        oversample=None,
        parallel_window_start=0,
        parallel_window_stop=-1,
        serial_window_start=0,
        serial_window_stop=-1,
        verbosity=1
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

        # Correlated double sampling
        image_bounced = np.copy(image).astype(np.double)
        image_bounced -= self.perturb_bias(
            image,
            oversample=oversample,
            parallel_window_start=parallel_window_start,
            parallel_window_stop=parallel_window_stop,
            serial_window_start=serial_window_start,
            serial_window_stop=serial_window_stop,
            verbosity=verbosity,
        )

        return image_bounced

    def remove_pixel_bounce(
        self,
        image,
        n_iterations,
        oversample=None,
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
                oversample=oversample,
				parallel_window_start=parallel_window_start,
                parallel_window_stop=parallel_window_stop,
                serial_window_start=serial_window_start,
                serial_window_stop=serial_window_stop,
                verbosity=verbosity,
            )

            # Improve the estimate of the image with pixel bounce removed
            image_remove_pixel_bounce += image - image_add_pixel_bounce

        return image_remove_pixel_bounce


"""
Standalone functions to call the above, but mirroring syntax of add_cti() and remove_cti()
These also enable the processing of several pixel bounces, in serial, in case that ever 
becomes a plausible thing.
"""

def add_pixel_bounce(
    image,
    pixel_bounce_list=None,
    oversample=None,
    parallel_window_start=0,
    parallel_window_stop=-1,
    serial_window_start=0,
    serial_window_stop=-1,
    verbosity=1,
):
    if pixel_bounce_list is None:
        raise Exception("Must provide a list of pixel bounce objects")

    # Idea bias level
    bias = np.zeros(image.shape)

    # Compute bias after it has been perturbed by capacitance with the image signal
    # Apply each pixel bounce capaitance separately, so their order has no effect
    for pixel_bounce in pixel_bounce_list:
        bias += pixel_bounce.perturb_bias(
            image,
            oversample=oversample,
            parallel_window_start=parallel_window_start,
            parallel_window_stop=parallel_window_stop,
            serial_window_start=serial_window_start,
            serial_window_stop=serial_window_stop,
            verbosity=verbosity,
        )

    # The effect of correlated double sampling (avoid overwriting input image)
    image_bounced = np.copy(image).astype(np.double) - bias

    return image_bounced



def remove_pixel_bounce(
    image,
    n_iterations,
    pixel_bounce_list=None,
    oversample=None,
    parallel_window_start=0,
    parallel_window_stop=-1,
    serial_window_start=0,
    serial_window_stop=-1,
    verbosity=1,
):
    if pixel_bounce_list is None:
        raise Exception("Must provide a list of pixel bounce objects")

    # Prepare output array (avoid overwriting input image)
    image_remove_pixel_bounce = np.copy(image).astype(np.double)

    # Iteratively add pixel bounce to a model of the corrected image
    for iteration in range(1, n_iterations + 1):

        if verbosity >= 1: print("Iter %d: " % iteration, end="", flush=True)
        image_add_pixel_bounce = add_pixel_bounce(
            image_remove_pixel_bounce,
            pixel_bounce_list=pixel_bounce_list,
            oversample=oversample,
            parallel_window_start=parallel_window_start,
            parallel_window_stop=parallel_window_stop,
            serial_window_start=serial_window_start,
            serial_window_stop=serial_window_stop,
            verbosity=verbosity,
        )
        
        # Improve the estimate of the image with pixel bounce removed
        image_remove_pixel_bounce += image - image_add_pixel_bounce

    return image_remove_pixel_bounce

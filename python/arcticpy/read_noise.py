import numpy as np

"""
CTI correction moves trailed electrons back to their proper location, but also
moves read noise - which was not trailed in the first place. The net effect is to
amplify read noise, and hide the difference in the (anti)correlation between
adjacent pixels. 

This class provides a set of routines to predict this effect, and counteract it.
"""

class ReadNoise:
    def __init__(
        self, 
        sigma=0.0, 
        adjacency=0.3, 
        blah=0.75, 
        amplitude_scale=1.0,
        n_iter=100
    ):
        """
        David, call these variables whatever you like
        """
        self.sigma = sigma
        self.adjacency = adjacency
        self.blah = blah
        self.amplitude_scale = amplitude_scale
        self.n_iter = n_iter

    @property
    def sigma(self):
        """
        RMS value of read noise, in units of electrons
        """
        return self._sigma

    @sigma.setter
    def sigma(self, value):
        if value < 0:
            value = 0.
        self._sigma = value

    def estimate_read_noise_image_from_image(self, image):
        #raise NotImplementedError
        return image

    def estimate_residual_covariance(
        self, 
        background_level,
        # Parallel
        parallel_ccd=None, #can we do this with **kwargs ? I'm not sure in python
        parallel_roe=None,
        parallel_traps=None,
        parallel_express=0,
        parallel_window_offset=0,
        parallel_window_start=0,
        parallel_window_stop=-1,
        parallel_time_start=0,
        parallel_time_stop=-1,
        parallel_prune_n_electrons=1e-10, 
        parallel_prune_frequency=20,
        # Serial
        serial_ccd=None,
        serial_roe=None,
        serial_traps=None,
        serial_express=0,
        serial_window_offset=0,
        serial_window_start=0,
        serial_window_stop=-1,
        serial_time_start=0,
        serial_time_stop=-1,
        serial_prune_n_electrons=1e-10, 
        serial_prune_frequency=20,
        # Pixel bounce
        pixel_bounce=None,
        # Output
        verbosity=1,
    ):
        raise NotImplementedError

    def optimise_parameters(
        self,
        background_level,
        #figure_of_merit=figure_of_merit(), # should probably pass a function here
        **kwargs 
        ## Parallel
        #parallel_ccd=None, #again, should do this with **kwargs 
        #parallel_roe=None,
        #parallel_traps=None,
        #parallel_express=0,
        #parallel_window_offset=0,
        #parallel_window_start=0,
        #parallel_window_stop=-1,
        #parallel_time_start=0,
        #parallel_time_stop=-1,
        #parallel_prune_n_electrons=1e-10, 
        #parallel_prune_frequency=20,
        ## Serial
        #serial_ccd=None,
        #serial_roe=None,
        #serial_traps=None,
        #serial_express=0,
        #serial_window_offset=0,
        #serial_window_start=0,
        #serial_window_stop=-1,
        #serial_time_start=0,
        #serial_time_stop=-1,
        #serial_prune_n_electrons=1e-10, 
        #serial_prune_frequency=20,
        ## Pixel bounce
        #pixel_bounce=None,
        ## Output
        #verbosity=1,
    ):
        raise NotImplementedError
        
    def figure_of_merit(covariance_matrix):
        raise NotImplementedError
        return 0.0
        
    def covariance_matrix_from_image(image, n_pixels=5):
        raise NotImplementedError
        covariance_matrix = np.zeros(n_pixels,n_pixels)
        return covariance_matrix


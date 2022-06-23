import numpy as np
import matplotlib.pyplot as plt
import copy

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
    omega : float
        Natural frequency of oscillations in the reference voltage, in units
        (per pixel) i.e. freq in Hz * clock speed
    gamma : float
        Damping coefficicent of oscillations in the reference voltage
    """
    
    def __init__(
        self,
        kA=0., 
        kv=0., 
        omega=1.,
        gamma=1. 
    ):

        self.kA = kA
        self.kv = kv
        self.omega = omega
        self.gamma = gamma

    
    
    def add_pixel_bounce(
        self,
        image,
        do_Plot = False
    ):
        #,
        #window_row_range,
        #window_column_range
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
            
        window_row_range : range
            The subset of row pixels to model, to save time when only a specific 
            region of the image is of interest. Defaults to range(0, n_pixels) for 
            the full image.
        
        window_column_range : range
            The subset of column pixels to model, to save time when only a specific 
            region of the image is of interest. Defaults to range(0, n_columns) for 
            the full image.
    
        Returns
        -------
        image : [[float]]
            The output array of pixel values.
        """
        
        # Pre-calcualte (once) useful quantities from eqn (43) of
        # Cieslinski &Ratkiewicz (2005) https://arxiv.org/abs/physics/0507182
        epsilon = 1
        #omega = np.sqrt(self.omegaO**2 - self.gamma**2)
        coeffA = 2 * np.exp(-1 * self.gamma * epsilon) * np.cos(self.omega * epsilon)
        coeffB = np.exp(-2 * self.gamma * epsilon)
        biasm1 = 0.
        
        # Read out one column of pixels through the (column of) traps
        n_rows_in_image, n_columns_in_image = image.shape
        for row_index in range(n_rows_in_image):
            
            print("Bouncing")
            
            # Initialise bias offset voltage, which should settle during prescan
            # of each row
            bias = np.zeros((1,n_columns_in_image))
            print(bias.shape)
            
            # Each pixel
            #for row_index in window_row_range:
            for column_index in range(1, n_columns_in_image):
                
                # Store previous values of bias, so difference equation can 
                # compute rates of change 
                biasm2 = copy.copy(biasm1)
                biasm1 = bias[row_index,column_index - 1]
    
                # What electronic impulse is being experienced?
                delta = image[row_index, column_index] - image[row_index, column_index - 1] 
                
                # Impose this (linearly) on the difference equation,
                # as one term that creates a bias offset (propto pixel_bounce_kA)
                # and one that creates a rate of change of bias (propto pixel_bounce_kV)
                biasm1 += (self.kA - self.kv) * delta
                biasm2 += (self.kA - 2 * self.kv) * delta
                
                # DHO difference equation, Cieslinski &Ratkiewicz (2005) eqn (43)
                bias[0,column_index] = coeffA * biasm1 - coeffB * biasm2
                
            print(bias.shape,image[:,:].shape)
            image[:, :] -= bias
            
            """
            if do_plot:
                pixels = np.arange(n_rows_in_image)
                colours = ["#1199ff", "#ee4400", "#7711dd", "#44dd44", "#775533"]
                plt.figure(figsize=(10, 6))
                ax1 = plt.gca()
                ax2 = ax1.twinx()
                ax1.legend(title="express", loc="lower left")
                ax1.set_yscale("log")
                ax1.set_xlabel("Pixel")
                ax1.set_ylabel("Counts")
                ax2.set_ylabel("Fractional Difference (dotted)")
                plt.tight_layout()
                plt.show()        
            """
    
        return image

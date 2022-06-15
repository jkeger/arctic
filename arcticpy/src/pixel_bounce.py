import numpy as np
import matplotlib.pyplot as plt
import copy

def add_pixel_bounce(
    image,
    roe
):
    #,
    #window_row_range,
    #window_column_range
    """
    Add pixel bounce to an image, modelled as Damped Harmonic Oscillations (DHO)
    in a CCD's reference voltage, driven by sudden changes in the signal.
    
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
    #omega = np.sqrt(roe.pixel_bounce_omegaO**2 - roe.pixel_bounce_gamma**2)
    coeffA = 2 * np.exp(-1 * roe.pixel_bounce_gamma * epsilon) * np.cos(roe.pixel_bounce_omega * epsilon)
    coeffB = np.exp(-2 * roe.pixel_bounce_gamma * epsilon)
    biasm1 = 0.
    do_plot = True
    
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
            biasm1 += (roe.pixel_bounce_kA - roe.pixel_bounce_kv) * delta
            biasm2 += (roe.pixel_bounce_kA - 2 * roe.pixel_bounce_kv) * delta
            
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

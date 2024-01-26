import numpy as np
import matplotlib.pyplot as plt
import arcticpy as arctic
import copy

#def frac_of_pixel_volume(n_e, ccd):
#    # Volume-driven model used by ArCTIc ccd.cpp for the fraction of a pixel volume filled by n_e electrons
#    well_range = ccd.full_well_depths[0] - ccd.well_notch_depths[0]
#    volume = min(max(((n_e - ccd.well_notch_depths[0]) / well_range ), 0), 1) ** ccd.well_fill_powers[0]
#    return ccd.first_electron_fills[0] + (1 - ccd.first_electron_fills[0]) * volume  
        
class VVTestBench:

    """
    Set up and carry out a 'Verification and Validation' EPER test of the level
    of CTI trailing in an image, by fitting the trailing of sky background into 
    the overscan regions. Comparing the output before and after CTI correction
    can test that the CTI correction has worked (or at least improved) an image.
    The whole trail profile is not fitted (e.g. optimising the release times),
    only the amplitude of a trail with profile defined by an instance of _traps. 
    
    Note that this test operates at the very, very bottom of a CCD pixel's 
    potential well - which is never used in an image with sky background, not
    necessarily suited to a volume-driven model such as arCTIc, and far from 
    the regime where arCTIc has been calibrated (at least for HST). Results 
    may therefore vary and correction may not be perfect.
    
    Example use
    -----------
    Can either (1) set kwarg VVTest=True in add_cti(), (2) run the standalone routine
    vv_test(image, parallel_roe=parallel_roe, parallel_ccd=parallel_ccd,... ) or
    (3) define a test bench once, then call it multiple times (see below).
    
    vv=arctic.VVTestBench(
        parallel_roe=parallel_roe, 
        parallel_ccd=parallel_ccd, 
        parallel_traps=parallel_traps, 
        serial_roe=serial_roe, 
        serial_ccd=serial_ccd, 
        serial_traps=serial_traps, 
        sum_of_exponentials=True,
        verbose=True
    )
    before=vv.test(image_A_raw)
    after=vv.test(image_A_corrected,
              parallel_valid_columns = before.parallel.valid_columns,
              parallel_pixels_pre_cti = before.parallel.pixels_pre_cti,
              parallel_fit_bias = True, 
              parallel_model_bias = before.parallel.best_fit_bias)
    before.parallel.plot()


    Parameters
    ----------
    parallel_ccd   :} ArCTIc model parameters
    parallel_roe   :} V&V test will be performed in the parallel (serial) direction if
    parallel_traps :} all three parallel (serial) inputs are set, _roe describes an 
    serial_ccd     :} overscan region, and _traps defines a trail model with >0 traps. 
    serial_roe     :} It can still use serial (parallel) _roe to identify perpendicular
    serial_traps   :} prescan/overscan regions to ignore, because these have no trails.

    sum_of_exponentials : bool
        Fit the EPER trail in an image using an analytic model, rather than arCTIc itself.
        An arCTIc model will look like this if the trap density is very low. As the trap 
        density rises, the trail flattens because trailed electrons can be recaptured and
        retrailed. The arCTIc trail shape also changes depending on the background level.
        The fitter in this algorithm is not designed to be robust, so it is simpler and
        faster with the analytic model.
    verbose : bool
        Print a running commentary to stdout.
    """
    
    def __init__(
        self,
        parallel_ccd = None,
        parallel_roe = None,
        parallel_traps = None,
        serial_ccd = None,
        serial_roe = None,
        serial_traps = None,
        verbose = False, 
        sum_of_exponentials = True, # generate model trail analytically, rather than arCTIc
        nrows_for_background = 10, # image region before overscan used to estimate background
        ncols_min_required = 4, # minmum number of columns (without stars) needed to bother
        sigma_clipping_threshold = 3., # used while calculating which columns contain stars 
        sigma_clipping_niter = 10., # used while calculating which columns contain stars 
    ):
        self.parallel_ccd   = parallel_ccd
        self.parallel_roe   = parallel_roe
        self.parallel_traps = parallel_traps
        self.serial_ccd     = serial_ccd
        self.serial_roe     = serial_roe
        self.serial_traps   = serial_traps
        self.verbose                  = verbose
        self.sum_of_exponentials      = sum_of_exponentials
        self.nrows_for_background     = nrows_for_background
        self.ncols_min_required       = ncols_min_required
        self.sigma_clipping_threshold = sigma_clipping_threshold
        self.sigma_clipping_niter     = sigma_clipping_niter
        self.results = []

    def print(self, *args, **kwargs): 
        if self.verbose: print(*args, **kwargs) # don't print if not verbose
    
    def test(
        self,
        image,
        parallel_valid_columns = None,
        parallel_pixels_pre_cti = None,
        parallel_model_bias = None,
        parallel_fit_bias = None,
        parallel_iterate_bias = None,
        serial_valid_columns = None,
        serial_pixels_pre_cti = None,
        serial_model_bias = None,
        serial_fit_bias = None,
        serial_iterate_bias = None
    ):
        """
        Fit the EPER trails into overscan regions in both parallel and serial directions 
        (or in whichever directions overscan regions and a CTI model exist).
        For example, this can be run before - then again after - CTI correction.

        Parameters
        ----------
        image: numpy.ndarray
            The 2D image (with a single float for each pixel) to be tested

        Parameters (all duplicated for parallel & serial directions)
        ------------------------------------------------------------
        valid_columns: [int]
            A list of columns free from astronomical sources near the overscan region.
            If not defined, this will be estimated automatically from image.
            Will generally be defined only on a second call, e.g. for a corrected image.
        pixels_pre_cti: numpy.ndarray
            A 1D array of the mean value of an image column, to be used instead of the 
            image itself, e.g. during a second call, to compare apples with apples.
        model_bias: float
            Initial guess at the level of residual DC bias in the overscan region.
        fit_bias : bool
            Assume that bias correction is imperfect, and also fit a DC bias offset in the 
            overscan region, which is free to take any value.
        iterate_bias : bool
            Iterantively improve the model of the DC bias offset.
            Does nothing if fit_bias is False
        """

        # Carry out V&V test in the parallel direction (if parallel overscan exists)
        vv_parallel = VVResult1D()
        if (
            self.parallel_ccd is not None and 
            self.parallel_roe is not None and
            self.parallel_traps is not None
        ):
            if (self.parallel_roe.overscan_start >= 0):
                self.print("Parallel V&V test")
                vv_parallel = self._test1d(
                    image, 
                    self.parallel_ccd,
                    self.parallel_roe,
                    self.parallel_traps,
                    x_roe = self.serial_roe,
                    valid_columns = parallel_valid_columns,
                    pixels_pre_cti = parallel_pixels_pre_cti,
                    model_bias = parallel_model_bias,
                    fit_bias = parallel_fit_bias,
                    iterate_bias = parallel_iterate_bias
                )
 
        # Carry out V&V test in the serial direction (if v overscan exists)
        vv_serial = VVResult1D()
        if (
            self.serial_ccd is not None and 
            self.serial_roe is not None and
            self.serial_traps is not None
        ):
            if (self.serial_roe.overscan_start >= 0):
                self.print("Serial V&V test")
                vv_serial = self._test1d(
                    image.T, 
                    self.serial_ccd,
                    self.serial_roe,
                    self.serial_traps,
                    x_roe = self.parallel_roe,
                    valid_columns = serial_valid_columns,
                    pixels_pre_cti = serial_pixels_pre_cti,
                    model_bias = serial_model_bias,
                    fit_bias = serial_fit_bias,
                    iterate_bias = serial_iterate_bias
                )
        
        # Save to history of results within the testbench object, as well as returning
        result = VVResult(vv_parallel,vv_serial)
        self.results.append(result)
        
        return result

    def _test1d(
        self,
        image,
        y_ccd,
        y_roe,
        y_traps,
        x_roe = None,
        valid_columns = None,
        pixels_pre_cti = None,
        model_bias = None,
        fit_bias = None,
        iterate_bias = None,
    ):
        """
        Fit the EPER trail into an overscan regions in a single (the y) direction of an image.
        """
        if y_ccd is None: raise Exception("CCD well filling model required to interpret V&V!")
        if y_roe is None: raise Exception("ROE model required to define overscan region!")
        if y_roe.overscan_start < 0: raise Exception("No overscan region is defined!")
        if y_traps is None: raise Exception("CTI model required to define test trail profile!")
        if model_bias is None: model_bias = 0.
        if fit_bias is None: fit_bias = True
        if iterate_bias is None: iterate_bias = False
        
        # Coordinates of overscan region and a place to measure the background
        y_max, x_max = image.shape
        # Exclude prescan/overscan regions in the transverse direction
        x_min = 0
        if x_roe is not None:
            x_min = x_roe.prescan_length
            if x_roe.overscan_start >= 0:
                x_max = x_roe.overscan_start
            if(x_max<x_min): raise Exception("No pixel areas recognised")
        
        # Exclude columns containing objects
        if valid_columns is None:
            self.print("Finding columns containing (only) background")
            # Extract representative chunk of image to calculate background,
            y_min = np.min(
                y_roe.overscan_start - self.nrows_for_background, y_roe.prescan_length
            )
            background_image = image[ y_min:y_roe.overscan_start, x_min:x_max ]
            # Iteratively find pixels with values away from the median
            valid_pixels = np.ones_like(background_image, dtype=bool)
            for iteration in np.flip(np.arange(self.sigma_clipping_niter)):
                background_level = ( np.median(background_image[valid_pixels]) )
                background_rms = np.sqrt(np.mean(
                    (background_image[valid_pixels] - background_level)**2  
                ))
                #tolerance = background_rms * ( self.sigma_clipping_threshold + iteration )
                tolerance = background_rms * ( self.sigma_clipping_threshold )
                valid_pixels = np.abs(background_image - background_level) <= tolerance
            valid_columns = np.squeeze(np.all(valid_pixels, axis=0))
            if np.sum(valid_columns) < self.ncols_min_required: 
                raise Exception("Background region could not be defined!")
             
        # Extract overscan region
        overscan_image = (image[ y_roe.overscan_start: , x_min:x_max ])[:,valid_columns]
        overscan = np.mean(overscan_image,axis=1)
        overscan_sigma = np.std(overscan_image,axis=1)/np.sqrt(np.sum(valid_columns))
        weight = 1.0 / overscan_sigma ** 2

        # Model EPER data before CTI was applied
        if pixels_pre_cti is None: 
            self.print("Making pre-CTI image")
            pixels_pre_cti = (image[0:y_roe.overscan_start,x_min:x_max])[:,valid_columns]
            pixels_pre_cti = np.mean(pixels_pre_cti,axis=1)
            # Ignore that electrons in the overscan should be pushed back to the image 
            # array, because there might also be unaccounted for bias in every pixel
            #pixels_pre_cti[y_roe.overscan_start-1] += np.sum(overscan)
        model_pre_cti = np.concatenate(
            (pixels_pre_cti, np.zeros(len(overscan)) + model_bias)
        )
        
        # Model EPER data after the nominal CTI model has been applied
        # First catch errors that will lead to a degenerate fit
        model_trap_density=sum(trap.density for trap in y_traps)
        if model_trap_density <= 0: raise Exception("No traps in CTI model!")
        if model_pre_cti[y_roe.overscan_start-1] <= y_ccd.well_notch_depths[0]:
            raise Exception("CTI model will not result in any trailing!")
        self.print("Generating model of EPER (bias: ",model_bias,", trap density: ",model_trap_density,")")
        if self.sum_of_exponentials:
            # Simple, approximate analytic model of trail
            self.print("Generating sum-of-exponentials model of EPER trail")
            traps_effect_on_final_pixel = y_roe.overscan_start * (
                y_ccd.phases[0].volume(model_pre_cti[y_roe.overscan_start-1]) - y_ccd.phases[0].volume(model_bias)
            )
            model_overscan = np.full( len(overscan), model_bias )
            for trap in y_traps:
                model_overscan += ( trap.density * traps_effect_on_final_pixel
                    * np.exp((-np.arange(len(overscan))) / trap.release_timescale)
                    * (1 - np.exp(-1 / trap.release_timescale))
                )
        else:
            # Use arCTIc to trail the pre-CTI data
            # Start trailing a few pixels early, to reach stead state with traps filled
            self.print("Using arCTIc to generate model of EPER trail")
            n_traps_seen_by_final_pixel = ( y_roe.overscan_start * model_trap_density
                * y_ccd.phases[0].volume(model_pre_cti[y_roe.overscan_start-1]) 
            )
            nrows_buffer = 10 # Minimum number of pixels in buffer 
            while np.sum(model_pre_cti[y_roe.overscan_start-nrows_buffer:y_roe.overscan_start]) < ( 
                n_traps_seen_by_final_pixel * 3 #Increase to engineering standard 3x safety margin 
            ): nrows_buffer += 1
            self.print("arCTIc pre-trailing buffer: ",nrows_buffer," pixels")
            # Run pre-CTI data model through arCTIc
            model_post_cti = arctic.add_cti(
                model_pre_cti.reshape(-1, 1),
                parallel_roe=y_roe,
                parallel_ccd=y_ccd,
                parallel_traps=y_traps,
                parallel_express=0,
                parallel_window_start=np.min(y_roe.overscan_start-nrows_buffer,0)
            )
            model_overscan = (model_post_cti[y_roe.overscan_start:]).flatten()
               
        # Find the best-fit DC bias and trap density using linear algebra
        # M is the Vandermonde matrix (for a quadratic polynomial it would be (1, x, x^2))
        if fit_bias:
            M = np.vstack((model_overscan - model_bias,np.ones_like(model_overscan))).T
            best_fit_coefficients, residuals, rank, singular_values = np.linalg.lstsq(
                M * weight[:, np.newaxis], overscan * weight, rcond=None
            )
        else:
            M = np.vstack((model_overscan - model_bias,np.zeros_like(model_overscan))).T
            best_fit_coefficients, residuals, rank, singular_values = np.linalg.lstsq(
                M * weight[:, np.newaxis], (overscan - model_bias) * weight, rcond=None
            )
            best_fit_coefficients[1] = model_bias
        best_fit_overscan = best_fit_coefficients[1] + best_fit_coefficients[0] * (model_overscan - model_bias)
        self.print("Best-fit coefficients: ",best_fit_coefficients)
       
        # Interpret the best-fit coefficients properly, assuming volume-driven CTI
        best_fit_bias = best_fit_coefficients[1] #+ best_fit_coefficients[1] * model_bias 
        model_volume_filled = y_ccd.phases[0].volume(model_pre_cti[y_roe.overscan_start-1])
        model_volume_active = model_volume_filled - y_ccd.phases[0].volume(model_bias) 
        best_fit_volume_active = model_volume_filled - y_ccd.phases[0].volume(best_fit_bias)
        # Account for fitted trail amplitude and fitted volume of pixel that it came from
        best_fit_trap_density = model_trap_density * (
            best_fit_coefficients[0] * ( model_volume_active / best_fit_volume_active )
        )
        self.print("Inferred best-fit model (bias: ", best_fit_bias,
            ", trap density: ",best_fit_trap_density,")")
        
        # Store results in a handy format for posterity
        result = VVResult1D(
            valid_columns = valid_columns,
            pixels_pre_cti = pixels_pre_cti,
            overscan = overscan,
            overscan_sigma = overscan_sigma,
            model_overscan = model_overscan,
            model_bias = model_bias,
            model_trap_density = model_trap_density,
            best_fit_overscan = best_fit_overscan,
            best_fit_bias = best_fit_bias,
            best_fit_trap_density = best_fit_trap_density
        )
        
        if iterate_bias:
            while (np.abs(result.best_fit_bias - result.model_bias) > 0.0001):
                self.print("Iterating to optimise model bias")
                result = self._test1d(
                    image,
                    y_ccd,
                    y_roe,
                    y_traps,
                    x_roe = x_roe,
                    valid_columns = valid_columns,
                    pixels_pre_cti = pixels_pre_cti,
                    model_bias = result.best_fit_bias,
                    fit_bias = True,
                    iterate_bias = False
                )

        return result


class VVResult1D(object):
    def __init__(
        self,
        valid_columns = None,
        pixels_pre_cti = None,
        overscan = None,
        overscan_sigma = None,
        model_overscan = None,
        model_bias = None,
        model_trap_density = None,
        best_fit_overscan = None,
        best_fit_bias = None,
        best_fit_trap_density = None
    ):
        """
        A simple class merely containing the result of a V&V test,
        and a function to plot the EPER trail with fit
        """
        self.valid_columns = valid_columns
        self.pixels_pre_cti = pixels_pre_cti
        self.overscan = overscan
        self.overscan_sigma = overscan_sigma
        self.model_overscan = model_overscan
        self.model_bias = model_bias
        self.model_trap_density = model_trap_density
        self.best_fit_overscan = best_fit_overscan
        self.best_fit_bias = best_fit_bias
        self.best_fit_trap_density = best_fit_trap_density
    
    def __str__(self):
        return str(self.best_fit_trap_density)
    
    def plot(self):
        x=np.arange(len(self.overscan))
        plt.plot(x,self.best_fit_overscan,"-")
        plt.plot(x,self.model_overscan+self.best_fit_bias-self.model_bias,".-")
        plt.plot(x,self.model_overscan,":")
        plt.errorbar(x,self.overscan,self.overscan_sigma,fmt="o")
        plt.yscale("log")
        plt.show()

class VVResult(object):
    def __init__(self, parallel, serial):
        self.parallel = parallel
        self.serial = serial
        


"""
Standalone functions to call the above, but mirroring syntax of add_cti() and remove_cti()
"""
def vv_test(
    image,
    vv_testbench = None,
    parallel_ccd = None,
    parallel_roe = None,
    parallel_traps = None,
    serial_ccd = None,
    serial_roe = None,
    serial_traps = None,
    verbose = False, 
    sum_of_exponentials = True, # generate model trail analytically, rather than arCTIc
    nrows_for_background = 10, # image region before overscan used to estimate background
    ncols_min_required = 4, # minmum number of columns (without stars) needed to bother
    sigma_clipping_threshold = 3., # used while calculating which columns contain stars 
    sigma_clipping_niter = 10., # used while calculating which columns contain stars 
    parallel_valid_columns = None,
    parallel_pixels_pre_cti = None,
    parallel_model_bias = None,
    parallel_fit_bias = None,
    parallel_iterate_bias = None,
    serial_valid_columns = None,
    serial_pixels_pre_cti = None,
    serial_model_bias = None,
    serial_fit_bias = None,
    serial_iterate_bias = None
):
    if vv_testbench is None:
        vv_testbench=arctic.VVTestBench(
            parallel_roe=parallel_roe, 
            parallel_ccd=parallel_ccd, 
            parallel_traps=parallel_traps, 
            serial_roe=serial_roe, 
            serial_ccd=serial_ccd, 
            serial_traps=serial_traps, 
            sum_of_exponentials=sum_of_exponentials,
            verbose=verbose
        )
    result=vv.test(image,
        parallel_valid_columns  = parallel_valid_columns,
        parallel_pixels_pre_cti = parallel_pixels_pre_cti,
        parallel_model_bias     = parallel_model_bias,
        parallel_fit_bias       = parallel_fit_bias,
        parallel_iterate_bias   = parallel_iterate_bias,
        serial_valid_columns    = serial_valid_columns,
        serial_pixels_pre_cti   = serial_pixels_pre_cti,
        serial_model_bias       = serial_model_bias,
        serial_fit_bias         = serial_fit_bias,
        serial_iterate_bias     = serial_iterate_bias
    )  
    return result

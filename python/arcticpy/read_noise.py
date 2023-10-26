import numpy as np
import arcticpy as ac
import matplotlib as mpl
from scipy.optimize import curve_fit

"""
CTI correction moves trailed electrons back to their proper location, but also
moves read noise - which was not trailed in the first place. The net effect is to
amplify read noise, and hide the difference in the (anti)correlation between
adjacent pixels. 

This class provides a set of routines to predict this effect, and counteract it.

#################
SAMPLE USAGE CASE
#################

If you have an image as a 2D numpy array and an arctic model (with parameters such as parallel_roe, 
parallel_ccd, parallel_traps, etc...) run the following:

1.) readnoise = arctic.ReadNoise(4.5) 
  The argument sets the level of readnoise (in units of electrons)

2.) [optional] readnoise.set_arctic_parameters(**kwargs)
  Input each arctic parameter as a keyword argument. 
  These will then be passed to subsequent function calls automatically
  (Note: these choices can always be overridden in functions, if desired)

3.) [optional] readnoise.optimise_SR_fraction_from_image(image)
  Input your data image. The function will determine the optimum
  fraction of S+R separation to apply to minimise CTI covariance

4.) generate_SR_frames_from_image(image)
  Do the actual S+R splitting on your science image. If the SR
  optimisation routine is not run, assume a correction level of 100%

5.) [optional]  readnoise.measure_simulated_covariance_corners()
  estimate the resulting covariance matrices in each ccd corner, using the optimised simulation

######POSSIBLE ALTERNATIVE OPTION?#####
5.) [optional] readnoise.calculate_covariance_corners_from image(image)
  measure covariance matrices using the real image data
  (not yet implemented)
#######################################

6.) [optional] readnoise.plot_covariance_matrices()
  plot the covariance matrices of all four corners
  (not fully implemented)


#################
EXAMPLE COMMANDS
#################
skyImage, readNoiseImage = readnoise.generate_SR_frames_from_image(image, sr_fraction=None,
                  parallel_roe, parallel_ccd, parallel_traps)

image_corrected = arctic.cti_correct(skyImage,
                  parallel_roe, parallel_ccd, parallel_traps)
                  
image_corrected += readNoiseImage         


#to be updated

readnoise.covariance

covariance = readnoise.etimate_residual_covariance_from_image(image, 
                  matrix_size=5, fprSize=5, 
                  parallel_roe, parallel_ccd, parallel_traps)

covariance = readnoise.etimate_residual_covariance(sky_level, sky_sigma, 
                  matrix_size=5, fprSize=5, 
                  parallel_roe, parallel_ccd, parallel_traps)



"""


class ReadNoise:
    def __init__(
        self,
        sigma_readnoise=0.0,
        adjacency=0.3,
        noise_model_scaling=1.0,  # originally 0.75
        amplitude_scale=0.2,  # originally 0.33
        n_iter=200,
        sr_fraction=None,
        serial=True,
        covariance_matrices=None,
        **kwargs
    ):
        self.sigmaRN = sigma_readnoise
        self.adjacency = adjacency
        self.ampScale = amplitude_scale
        self.outScale = noise_model_scaling
        self.n_iter = n_iter
        self.smoothCol = serial
        self.sigmaSky = None
        self.skyFrameSim = None
        self.readnoiseFrameSim = None
        self.SRfrac_optimised = None
        self.arcKwargs = {}
        self.covarianceMatrices = {}
        self.dataFrames = {}

    #        self._sr_fraction = sr_fraction
    #        self._sr_fraction_optimised = None

    #    @property
    #    def sr_fraction(self):
    #        """
    #        RMS value of read noise, in units of electrons
    #        """
    #        if self._sr_fraction is not None: return self._sr_fraction
    #        #if self._sr_fraction_optimised is None: self.optimise_sr_fraction()
    #        return self._sr_fraction_optimised
    #
    #    @sr_fraction.setter
    #    def sr_fraction(self, value):
    #        self._sr_fraction = value
    #
    #    @property
    #    def sr_fraction_optimised(self, **kwargs):
    #        """
    #        Value of S+R splitting fraction that will minimise pixel-to-pixel covariance
    #        after CTI correction
    #        """
    #        if self._sr_fraction_optimised is None: self.optimise_sr_fraction(**kwargs)
    #        return self._sr_fraction_optimised
    #
    #    @sr_fraction_optimised.setter
    #    def sr_fraction_optimised(self, value):
    #        self._sr_fraction_optimised = value

    @property
    def sigma(self):
        """
        RMS value of read noise, in units of electrons
        """
        return self._sigma

    @sigma.setter
    def sigma(self, value):
        if value < 0:
            value = 0.0
        self._sigma = value

    ###############
    #
    # USER-FACING FUNCTIONS
    #
    ###############
    def set_arctic_parameters(self, **kwargs):
        """
        Function for pre-setting arCTIc parameters (such as CCD, trap density, and ROE)
        This is a convenience function to eliminate multiple, bulky keyword argument calls in other functions
        If desired, the user can still specify or overwrite these values (on the fly) in the other functions
        by explicitly calling the keyword arguments there.

        Parameters
        ----------
        **kwargs : variables
            Keyword arguments specifying arCTIc parameters
        """
        self.arcKwargs = kwargs

    ###############
    ###############
    def generate_SR_frames_from_image(self, image, sr_fraction=None):
        """
        Function for generating a model of read noise in an image, using the level specified in the
        initial ReadNoise call. Once modeled, the read noise component is separated from the data,
        leaving a smooth "S" component and a read-noise-only "R" component. The S somponent can then
        be CTI-corrected and recombined with the R model to minimise residual covariance in the image.

        Parameters
        ----------

        image: 2D numpy aray
            This can be a real science image or a simulation generated by <optimise_SR_fraction>

        sr_fraction: float
            scale fraction specifying what percentage of read noise should be separated into the R
            frame. Default is set to the optimised value measured in <optimise_SR_frac>, or 100%
            if <optimise_SR_frac> has not been previously run.


        """
        ampReadNoise = self.sigmaRN

        # if sr_fraction is unset, use system value
        if sr_fraction == None:
            sr_fraction = self.SRfrac_optimised
            # if optimisation is not set, default to 100%
            if sr_fraction == None:
                sr_fraction = 1.0

        imageIn = image
        imageAdj = np.zeros_like(imageIn)  # adjustment image (for read noise modeling)
        imageOut = imageIn.copy()  # output ("smoothed") image

        nrows = imageIn.shape[0]
        ncols = imageIn.shape[1]

        rmsGlobal = 0.0
        nrmsGlobal = 0.0

        smoother = 1
        print("iter", "model_rms", "target_rms", "residual")
        for s in range(self.n_iter):
            oldchk = ampReadNoise - rmsGlobal
            imageAdj = self._determine_noise_model(imageIn, imageOut)

            if (ampReadNoise - rmsGlobal) > 0:
                imageOut += imageAdj * self.outScale / smoother
                noiseImage = imageIn - imageOut
            else:
                imageOut -= imageAdj * self.outScale / smoother
                noiseImage = imageIn - imageOut

            cond = abs(imageIn > 0.1) | abs(imageOut > 0.1)
            rmsGlobal = np.sum(noiseImage[cond] ** 2)
            nrmsGlobal = noiseImage[cond].size

            rmsGlobal = np.sqrt(rmsGlobal / nrmsGlobal)
            print("\033[K", end="\r")
            print(
                "%4d  %f    %5.2f    %f"
                % (s, rmsGlobal, self.sigmaRN, (ampReadNoise - rmsGlobal)),
                end="",
            )

            chk = ampReadNoise - rmsGlobal
            if chk * oldchk < 0:
                smoother += 1

            if type(ampReadNoise) == np.ndarray:
                if np.max(ampReadNoise - rmsGlobal) < 0.0001:
                    break
            else:
                if abs(ampReadNoise - rmsGlobal) < 0.0001:
                    break

        # scale the smooth and noise frames if <sr_fraction> != 1
        if sr_fraction != 1:
            imageOut += noiseImage
            noiseImage *= sr_fraction
            imageOut -= noiseImage

        return imageOut, noiseImage  # These are the "S" and "R" frames, respectively

    ###############
    ###############
    def optimise_SR_fraction(
        self,
        background_level,
        background_sigma=None,
        n_pixels=2048,
        subchip_size=500,
        matrix_size=5,
        fom_method="box",
        **kwargs
    ):
        """
        Adjust parameters of the S+R algorithm that estimates the read noise in an image.
        The image itself is not required; merely its dimensions and sky level parameters.

        It works by simulating a subchip_size x subchip_size regions of the image, one
        at each corner of the CCD, then tries to minimise the pixel-to-pixel covariance
        after CTI correction in the most distant corner.

        This merely adjusts the self parameters contained within the ReadNoise instance.
        The final optimised S+R value is stored as the <optimised_SR_fraction> class variable

        Parameters
        ----------
        background_level : Float
            The mean (sky) background level across the image.

        background_sigma : Float
            The rms variation per pixel around the background level.
            If not specified, it will assume sqrt(background_level) for shot noise.

        n_pixels : Float or (Float, Float)
            The number of pixels in the CCD. If a single number is supplied, it assumes
            the CCD is square. If two numbers are supplied, the assumed order is (n_y,n_x).

        subchip_size : Float
            size of the cutout region, to test CTI behaviour in different regimes (e.g. far from readout, close to readout, etc.)

        matrix_size : Int
            Size of covariance matrix used to estimate covariance. By default the program creates a 5x5 matrix

        fom_method : String
            Method used to estimate the mean value of a covariance matrix, for figure-of-merit purposes. Can be:
                box : take a square (3x3) region around the central pixel [DEFAULT]
                row : take all cells along the central row
                column : take all cells along the central column

        **kwargs : parameters that characterise CTI features in arCTIc (trap density, CCD, read out electronics (ROE) descriptions)
        """

        # Parse inputs
        if type(n_pixels) == int:
            n_pixels = (n_pixels, n_pixels)
        chip_size = n_pixels
        if background_sigma == None:
            background_sigma = np.sqrt(background_level)
        self.sigmaSky = background_sigma

        # set prescan regions (this simulates the maximum possible CTI trailing, averaged at the far edge of the real CCD)
        if "parallel_roe" in kwargs:
            kwargs["parallel_roe"].prescan_offset = n_pixels[0] + (subchip_size // 2)
        if "serial_roe" in kwargs:
            kwargs["serial_roe"].prescan_offset = n_pixels[1] + (subchip_size // 2)

        # container for optimised values
        self.optVals = {}

        # if they don't exist, make the sky background and readnoise images for the simulations
        if (self.skyFrameSim is None) & (self.readnoiseFrameSim is None):
            self._create_initial_sim_images(
                background_level, background_sigma, image_size=subchip_size
            )

        # create "simulation noise" covariance matrix (estimated from the sky+readnoise image)
        # in this case, the fpr decrement is set to zero, because there is no CTI trailing
        sim_matrix = self.covariance_matrix_from_image(
            self.skyFrameSim + self.readnoiseFrameSim,
            matrix_size=matrix_size,
            fprSize=5,
        )

        # ideally, the matrix should be all zeros except for the centre square (== sigmaSky**2 + sigmaReadnoise**2)
        # therefore, subtracting this from sim_matrix will provide the simulation noise covariance
        sim_matrix[matrix_size // 2, matrix_size // 2] -= (
            self.sigmaRN**2 + self.sigmaSky**2
        )

        self.optVals["pre-benchmark"] = sim_matrix

        # run S+R routine at 0% and 100% S+R fraction
        result_array = np.array([])
        matrix_array = np.zeros((2, matrix_size, matrix_size))
        sr_frac = np.array([0.0, 1.0])
        for frac in sr_frac:
            print("\nestimating S+R covariance at %d percent level" % (100 * frac))
            raw_matrix, correction_matrix = self._estimate_residual_covariance(
                self.skyFrameSim,
                self.readnoiseFrameSim,
                frac,
                matrix_size=matrix_size,
                **self.arcKwargs,
                **kwargs
            )
            matrix_array[int(frac)] = correction_matrix
            result = self.figure_of_merit(
                correction_matrix, fom_method=fom_method
            )  # TODO: check to see if changing subgrid_size actually matters
            result_array = np.append(result_array, result)
        # interpolate between the results to determine the point of minimum residual covariance
        a_fit, cov = curve_fit(
            self._fitter_function, sr_frac, result_array, absolute_sigma=True
        )
        m = a_fit[1]
        b = a_fit[0]
        xint = (
            -b / m
        )  # this intercept will be the optimised S+R fraction (where mean covariance=0)

        # store final optimised fraction as a class variable
        self.SRfrac_optimised = xint
        # self.optVals['optSRfrac'] = xint

    ###############
    ###############
    def optimise_SR_fraction_from_image(
        self,
        image,
        subchip_size=500,  # size of the cutout region, to test CTI behaviour (a subset of pixels far form the readout)
        matrix_size=5,  # covariance matrix size (NxN array)
        fom_method="box",
        **kwargs
    ):
        """
        A wrapper function for running the S+R optimisation routine using a real astronomical image rather than theoretical values.
        This routine still calls <optimise_SR_fraction> but the sky paramerters and image size are determined from the data itself.

        Parameters
        ----------
        image : numpy array
            2D numpy array containing the pixel values of the image

        subchip_size : Float
            size of the cutout region, to test CTI behaviour in different regimes (e.g. far from readout, close to readout, etc.)

        matrix_size : Int
            Size of covariance matrix used to estimate covariance. By default the program creates a 5x5 matrix

        fom_method : String
            Method used to estimate the mean value of a covariance matrix, for figure-of-merit purposes. Can be:
                box : take a square (3x3) region around the central pixel [DEFAULT]
                row : take all cells along the central row
                column : take all cells along the central column

        **kwargs : parameters that characterise CTI features in arCTIc (trap density, CCD, read out electronics (ROE) descriptions)

        """

        image_shape = image.shape
        sky_level = np.median(image)
        sky_back = np.sqrt(sky_level)  # assume pure shot noise for now

        self.optimise_SR_fraction(
            sky_level,
            sky_back,
            image_shape,
            subchip_size,
            matrix_size,
            fom_method,
            **kwargs
        )

    ###############
    ###############
    def calculate_covariance_corners_from_image(
        self,
        image,
    ):
        raise NotImplementedError

    ###############
    ###############
    def plot_matrix(
        self,
        covariance_matrix,
        supressCentralPix=True,
        title="MyPlotMatrix",
        clearFrame=True,
        **kwargs
    ):
        if supressCentralPix == True:
            matrix = covariance_matrix.copy()
            matrix_size = matrix.shape[0]
            matrix[
                matrix_size // 2, matrix_size // 2
            ] = 0  # set central pixel to zero, to better see correlation dynamic range
        else:
            matrix = covariance_matrix.copy()

        if clearFrame:
            mpl.pyplot.clf()
        mpl.pyplot.imshow(matrix, cmap="bwr", **kwargs)
        mpl.pyplot.colorbar()
        mpl.pyplot.title(title)
        mpl.pyplot.show()

    ###############
    ###############
    def plot_optimised_covariance_matrices(self):
        mpl.pyplot.figure()
        mpl.pyplot.subplot(221)
        self.plot_matrix(
            self.optVals["noCTIQuadMatrix_diff"],
            title="Readout Corner",
            clearFrame=False,
            vmin=-0.01,
            vmax=0.01,
        )
        mpl.pyplot.subplot(222)
        self.plot_matrix(
            self.optVals["serialQuadMatrix_diff"],
            title="Serial Corner",
            clearFrame=False,
            vmin=-0.01,
            vmax=0.01,
        )
        mpl.pyplot.subplot(223)
        self.plot_matrix(
            self.optVals["parallelQuadMatrix_diff"],
            title="Parallel Corner",
            clearFrame=False,
            vmin=-0.01,
            vmax=0.01,
        )
        mpl.pyplot.subplot(224)
        self.plot_matrix(
            self.optVals["combinedQuadMatrix_diff"],
            title="Combined Corner",
            clearFrame=False,
            vmin=-0.01,
            vmax=0.01,
        )

    ###############

    ###############
    #
    # BACKGROUND FUNCTIONS
    #
    ###############
    def _determine_noise_model(self, imageIn, imageOut):
        """
        Method for estimating readnoise on image (in S+R fashion)
        assumes parallel+serial CTI trailing by default
        """
        readNoiseAmp = self.sigmaRN

        dval0 = imageIn - imageOut
        dval0u = dval0.copy()

        dval0u[dval0u > 1] = 1
        dval0u[dval0u < -1] = -1

        dval9 = imageIn * 0
        dcount = imageIn * 0

        dval9[:-1, :-1] += imageIn[1:, 1:] - imageOut[1:, 1:]
        dcount[:-1, :-1] += 1
        dval9[:-1, :] += imageIn[1:, :] - imageOut[1:, :]
        dcount[:-1, :] += 1
        dval9[:-1, 1:] += imageIn[1:, :-1] - imageOut[1:, :-1]
        dcount[:-1, 1:] += 1
        dval9[:, :-1] += imageIn[:, 1:] - imageOut[:, 1:]
        dcount[:, :-1] += 1
        dval9[:, :] += imageIn[:, :] - imageOut[:, :]
        dcount[:, :] += 1
        dval9[:, 1:] += imageIn[:, :-1] - imageOut[:, :-1]
        dcount[:, 1:] += 1
        dval9[1:, :-1] += imageIn[:-1, 1:] - imageOut[:-1, 1:]
        dcount[1:, :-1] += 1
        dval9[1:, :] += imageIn[:-1, :] - imageOut[:-1, :]
        dcount[1:, :] += 1
        dval9[1:, 1:] += imageIn[:-1, :-1] - imageOut[:-1, :-1]
        dcount[1:, 1:] += 1

        dval9 /= dcount

        readNoiseAmpFraction = self.ampScale
        dval9u = dval9.copy()
        if type(readNoiseAmp) == np.ndarray:
            dval9u = dval9u.flatten()
            readNoiseAmp = readNoiseAmp.flatten()
            dval9u[dval9u > readNoiseAmp * readNoiseAmpFraction] = (
                readNoiseAmp * readNoiseAmpFraction
            )[dval9u > readNoiseAmp * readNoiseAmpFraction]
            dval9u[dval9u < readNoiseAmp * -readNoiseAmpFraction] = (
                readNoiseAmp * -readNoiseAmpFraction
            )[dval9u < readNoiseAmp * -readNoiseAmpFraction]
            dval9u = dval9u.reshape(imageIn.shape)
            readNoiseAmp = readNoiseAmp.reshape(imageIn.shape)
        else:
            dval9u[dval9u > readNoiseAmp * readNoiseAmpFraction] = (
                readNoiseAmp * readNoiseAmpFraction
            )
            dval9u[dval9u < readNoiseAmp * -readNoiseAmpFraction] = (
                readNoiseAmp * -readNoiseAmpFraction
            )

        dmod1 = imageIn * 0
        dmod1[1:, :] = imageOut[:-1, :] - imageOut[1:, :]
        dmod2 = imageIn * 0
        dmod2[:-1, :] = imageOut[1:, :] - imageOut[:-1, :]

        if self.smoothCol:
            cmod1 = imageIn * 0
            cmod1[:, 1:] = imageOut[:, :-1] - imageOut[:, 1:]
            cmod2 = imageIn * 0
            cmod2[:, :-1] = imageOut[:, 1:] - imageOut[:, :-1]

        dmod1u = dmod1.copy()
        if type(readNoiseAmp) == np.ndarray:
            dmod1u = dmod1u.flatten()
            readNoiseAmp = readNoiseAmp.flatten()
            dmod1u[dmod1u > readNoiseAmp * readNoiseAmpFraction] = (
                readNoiseAmp * readNoiseAmpFraction
            )[dmod1u > readNoiseAmp * readNoiseAmpFraction]
            dmod1u[dmod1u < readNoiseAmp * -readNoiseAmpFraction] = (
                readNoiseAmp * -readNoiseAmpFraction
            )[dmod1u < readNoiseAmp * -readNoiseAmpFraction]
            dmod1u = dmod1u.reshape(imageIn.shape)
        else:
            dmod1u[dmod1u > readNoiseAmp * readNoiseAmpFraction] = (
                readNoiseAmp * readNoiseAmpFraction
            )
            dmod1u[dmod1u < readNoiseAmp * -readNoiseAmpFraction] = (
                readNoiseAmp * -readNoiseAmpFraction
            )

        dmod2u = dmod2.copy()
        if type(readNoiseAmp) == np.ndarray:
            dmod2u = dmod2u.flatten()
            readNoiseAmp = readNoiseAmp.flatten()
            dmod2u[dmod2u > readNoiseAmp * readNoiseAmpFraction] = (
                readNoiseAmp * readNoiseAmpFraction
            )[dmod2u > readNoiseAmp * readNoiseAmpFraction]
            dmod2u[dmod2u < readNoiseAmp * -readNoiseAmpFraction] = (
                readNoiseAmp * -readNoiseAmpFraction
            )[dmod2u < readNoiseAmp * -readNoiseAmpFraction]
            dmod2u = dmod2u.reshape(imageIn.shape)
            readNoiseAmp = readNoiseAmp.reshape(imageIn.shape)
        else:
            dmod2u[dmod2u > readNoiseAmp * readNoiseAmpFraction] = (
                readNoiseAmp * readNoiseAmpFraction
            )
            dmod2u[dmod2u < readNoiseAmp * -readNoiseAmpFraction] = (
                readNoiseAmp * -readNoiseAmpFraction
            )

        if self.smoothCol:
            cmod1u = cmod1.copy()
            if type(readNoiseAmp) == np.ndarray:
                cmod1u = cmod1u.flatten()
                readNoiseAmp = readNoiseAmp.flatten()
                cmod1u[cmod1u > readNoiseAmp * readNoiseAmpFraction] = (
                    readNoiseAmp * readNoiseAmpFraction
                )[cmod1u > readNoiseAmp * readNoiseAmpFraction]
                cmod1u[cmod1u < readNoiseAmp * -readNoiseAmpFraction] = (
                    readNoiseAmp * -readNoiseAmpFraction
                )[cmod1u < readNoiseAmp * -readNoiseAmpFraction]
                cmod1u = cmod1u.reshape(imageIn.shape)
                readNoiseAmp = readNoiseAmp.reshape(imageIn.shape)
            else:
                cmod1u[cmod1u > readNoiseAmp * readNoiseAmpFraction] = (
                    readNoiseAmp * readNoiseAmpFraction
                )
                cmod1u[cmod1u < readNoiseAmp * -readNoiseAmpFraction] = (
                    readNoiseAmp * -readNoiseAmpFraction
                )

            cmod2u = cmod2.copy()
            if type(readNoiseAmp) == np.ndarray:
                cmod2u = cmod2u.flatten()
                readNoiseAmp = readNoiseAmp.flatten()
                cmod2u[cmod2u > readNoiseAmp * readNoiseAmpFraction] = (
                    readNoiseAmp * readNoiseAmpFraction
                )[cmod2u > readNoiseAmp * readNoiseAmpFraction]
                cmod2u[cmod2u < readNoiseAmp * -readNoiseAmpFraction] = (
                    readNoiseAmp * -readNoiseAmpFraction
                )[cmod2u < readNoiseAmp * -readNoiseAmpFraction]
                cmod2u = cmod2u.reshape(imageIn.shape)
                readNoiseAmp = readNoiseAmp.reshape(imageIn.shape)
            else:
                cmod2u[cmod2u > readNoiseAmp * readNoiseAmpFraction] = (
                    readNoiseAmp * readNoiseAmpFraction
                )
                cmod2u[cmod2u < readNoiseAmp * -readNoiseAmpFraction] = (
                    readNoiseAmp * -readNoiseAmpFraction
                )

        readNoiseAmp2 = readNoiseAmp**2
        w0 = dval0 * dval0 / (dval0 * dval0 + 4.0 * readNoiseAmp2)
        w9 = dval9 * dval9 / (dval9 * dval9 + 18.0 * readNoiseAmp2)
        w1 = 4 * readNoiseAmp2 / (dmod1 * dmod1 + 4.0 * readNoiseAmp2)
        w2 = 4 * readNoiseAmp2 / (dmod2 * dmod2 + 4.0 * readNoiseAmp2)
        if self.smoothCol:
            wc1 = 4 * readNoiseAmp2 / (cmod1 * cmod1 + 4.0 * readNoiseAmp2)
            wc2 = 4 * readNoiseAmp2 / (cmod2 * cmod2 + 4.0 * readNoiseAmp2)

        if self.smoothCol:
            return (
                (dval0u * w0 / 6)
                + (dval9u * w9 / 6)
                + (dmod1u * w1 / 6)
                + (dmod2u * w2 / 6)
                + (cmod1u * wc1 / 6)
                + (cmod2u * wc2 / 6)
            )
        else:
            return (
                (dval0u * w0 * 0.25)
                + (dval9u * w9 * 0.25)
                + (dmod1u * w1 * 0.25)
                + (dmod2u * w2 * 0.25)
            )

    ###############
    ###############
    def _create_initial_sim_images(
        self,
        background_level,
        background_sigma,
        image_size=500,
        overwrite=False,  # figure out how to implement this (it fails currently)
    ):
        """
        A subroutine to generate sky_background and readnoise frames for S+R simulations

        These components will create a baseline "clean" image (sky + readnoise) that we
        would expect to observe in the absence of CTI smearing effects
        in each simulated corner. Keeping this fixed will control random noise fluctuations
        ("simulation noise") when estimating covariances throughout the simulation.

        Note: This routine will be normally be called in <optimise_sr_fraction>
              rather than through direct user input
        """

        # create sky frame
        skyFrame = np.random.normal(
            background_level, background_sigma, (image_size, image_size)
        )
        self.skyFrameSim = skyFrame

        # create readnoise_frame
        noiseFrame = np.random.normal(0, self.sigmaRN, (image_size, image_size))
        self.readnoiseFrameSim = noiseFrame

        # generate covariance matrix from clean image

        return

    ###############
    ###############
    def _estimate_residual_covariance(
        self,
        skyFrame,
        noiseFrame,
        sr_fraction,
        matrix_size=5,  # size of covariance grid, in pixels (also assumes square)
        **kwargs
    ):
        """
        Estimate a covariance matrix for an optimising simulation, assuming the sky/noise frames have already been created
        """
        # add CTI effects to skyFrame
        ctiTrailedFrame = ac.add_cti(skyFrame, **kwargs)
        # add read noise to CTI-trailed image
        readNoiseAddedFrame = ctiTrailedFrame + noiseFrame
        # do S+R routine
        sFrame, rFrame = self.generate_SR_frames_from_image(
            readNoiseAddedFrame, sr_fraction
        )
        ###rescale S+R correction to a fractional value (for optimization)
        ##sFrame+=rFrame
        ##rFrame*=sr_fraction
        ##sFrame-=rFrame
        # clean CTI from S frame
        ctiCorrectedFrame = ac.remove_cti(sFrame, 1, **kwargs)
        # re-add R frame to correction
        outputFrame = ctiCorrectedFrame + rFrame

        # determine covariance FOMs
        fomSR = self.covariance_matrix_from_image(outputFrame, matrix_size=matrix_size)
        fomBenchmark = self.covariance_matrix_from_image(
            skyFrame + noiseFrame, matrix_size=matrix_size
        )
        fomDiff = fomSR - fomBenchmark

        return fomSR, fomDiff

    ###############
    ###############
    def _fitter_function(self, x, intercept, slope):
        """
        simple functional form to optimize a (linear) S+R fit using covariance matrix data
        passed to python optiser in optimise_SR_fraction, not needed by the user
        """
        y = intercept + slope * x
        return y

    ###############
    ###############
    def figure_of_merit(self, covariance_matrix, fom_method="box", subgrid_size=None):
        """
        function for estimating the covariance figure of merit: the average value over some section of a covariance matrix
        if method == 'box' take a square region around the central pixel; 'row' is along the central row; column is along the central column
        if subgrid size is selected, use only an NxN subset of pixels in the matrix, starting from the center outward
        """
        xlen = covariance_matrix.shape[1]
        ylen = covariance_matrix.shape[0]
        xcenter = xlen // 2
        ycenter = ylen // 2
        xstart, xend = 0, xlen
        ystart, yend = 0, ylen
        if subgrid_size is not None:
            xdiff = (xlen - subgrid_size) // 2
            ydiff = (ylen - subgrid_size) // 2
            xstart += xdiff
            xend -= xdiff
            ystart += ydiff
            yend -= ydiff

        covariance_matrix[ycenter, xcenter] = 0  # supress autocorrelation pixel

        if fom_method == "box":
            result_mean = (
                np.sum(covariance_matrix[ystart:yend, xstart:xend])
                - covariance_matrix[ycenter, xcenter]
            ) / 8  # average over central box
        if fom_method == "row":
            result_mean = (
                np.sum(covariance_matrix[ycenter, xstart:xend])
                - covariance_matrix[ycenter, xcenter]
            ) / 4  # average over central row (for serial CTI)
        if fom_method == "column":
            result_mean = (
                np.sum(covariance_matrix[ystart:yend, xcenter])
                - covariance_matrix[ycenter, xcenter]
            ) / 4  # sum over central column (for parallel CTI)
        # raise NotImplementedError
        return result_mean

    ###############
    ###############
    def covariance_matrix_from_image(self, image, matrix_size=5, fprSize=5):
        matRange = (
            matrix_size // 2
        )  # set positions of correlation pixels and set limits to remove "roll-over" region

        covariance_matrix = np.zeros((matrix_size, matrix_size))
        # calcluate mean stats on image
        image2 = image[fprSize:, fprSize:]  # remove FPR decrement
        x = image2[
            matRange:-matRange, matRange:-matRange
        ].flatten()  # remove possible roll-over region
        xbar = np.mean(x)

        # roll image to get cross correlation
        for i in range(-matRange, matRange + 1, 1):
            for j in range(-matRange, matRange + 1, 1):
                y = np.roll(image2, (-j, -i), axis=(0, 1))[2:-2, 2:-2].flatten()
                ybar = np.mean(y)

                # calculate covariance
                covar = (
                    np.sum((x - xbar) * (y - ybar)) / x.size
                )  # /(np.std(x-xbar)*np.std(y-ybar))/(x.size) #removing noise-level normalization
                # covar = np.sum((x-xbar)*(y-ybar))/(np.std(x-xbar)*np.std(y-ybar))/(x.size)
                # populate matrix cells
                covariance_matrix[i + matRange, j + matRange] = covar
        # covariance_matrix[matRange,matRange] = 0 #switch this normalization to plotting

        return covariance_matrix

    ###############

    ###############
    #
    # FUNCTIONS TO BE REFORMATTED / RETHOUGHT? (WORK IN PROGRESS)
    #
    ###############
    def measure_simulated_covariance_corners(
        self,
        n_pixels=None,  # the size of the entire CCD over which the procedure will be run
        subchip_size=500,  # size of the cutout region, to test CTI behaviour (a subset of pixels far form the readout)
        matrix_size=5,  # covariance matrix size (NxN array)
        fom_method="box",
        **kwargs
    ):
        """
        Apply optimised S+R correction to the four corners of the CCD (close to readout, far from parallel register, far from serial register, far from both)
        and estimate a covariance matrix for each corner. These matrices can be used to correct any residual covariance in weak lensing shapes.
        """

        # update any keyword arguments
        allKwargs = kwargs | self.arcKwargs

        frac_opt = (
            self.SRfrac_optimised
        )  # self.optVals['optSRfrac'] #set the optimized S+R fraction
        if frac_opt == None:
            frac_opt = 1.0

        if type(n_pixels) == int:
            n_pixels = (n_pixels, n_pixels)
        chip_size = n_pixels

        ##set prescan regions (this simulates the maximum possible CTI trailing, averaged at the far edge of the real CCD)
        # if 'parallel_roe' in allKwargs:
        #    kwargs['parallel_roe'].prescan_offset = n_pixels[0]+(subchip_size//2)
        # if 'serial_roe' in allKwargs:
        #    kwargs['serial_roe'].prescan_offset = n_pixels[1]+(subchip_size//2)

        # re-run the S+R routine with the optimised level
        raw_matrix_opt, correction_matrix_opt = self._estimate_residual_covariance(
            self.skyFrameSim,
            self.readnoiseFrameSim,
            frac_opt,
            matrix_size=matrix_size,
            **self.arcKwargs,
            **kwargs
        )
        (
            raw_matrix_opt_noSR,
            correction_matrix_opt_noSR,
        ) = self._estimate_residual_covariance(
            self.skyFrameSim,
            self.readnoiseFrameSim,
            0,
            matrix_size=matrix_size,
            **self.arcKwargs,
            **kwargs
        )  # this is dumb...do it better

        # run a second S+R routine in the region close to the readout registers, to identify/remove stochastic simulation noise
        kwargs_noCTI = kwargs | self.arcKwargs
        kwargs_noCTI["parallel_traps"] = None
        kwargs_noCTI["serial_traps"] = None
        # effectively, we are removing the effects of CTI, but still calculating a covariance matrix
        (
            raw_matrix_opt_noCTI,
            correction_matrix_opt_noCTI,
        ) = self._estimate_residual_covariance(
            self.skyFrameSim,
            self.readnoiseFrameSim,
            frac_opt,
            matrix_size=matrix_size,
            **kwargs_noCTI
        )
        (
            raw_matrix_opt_noCTI_noSR,
            correction_matrix_opt_noCTI_noSR,
        ) = self._estimate_residual_covariance(
            self.skyFrameSim,
            self.readnoiseFrameSim,
            0,
            matrix_size=matrix_size,
            **kwargs_noCTI
        )

        # create the "benchmark" covariance matrix: a purely non-correlated array with the central pixel equal to (readnoise_sigma**2 + background_sigma**2)
        # Any difference between this and "raw_matrix_opt_noCTI" is the simulation noise, and can be eliminated
        benchmark_matrix = raw_matrix_opt_noCTI * 0
        centrePix = matrix_size // 2
        benchmark_matrix[centrePix, centrePix] = self.sigmaRN**2 + self.sigmaSky**2
        # calculate the simulation-noise residual
        residual_benchmark_matrix = raw_matrix_opt_noCTI - benchmark_matrix

        # Finally, if both parallel and serial CTI are present, calculate covariances of the "cross-region" chip corners (far from serial readout but close to parallel readout, and vice-versa)
        if ("parallel_traps" in allKwargs) & ("serial_traps" in allKwargs):
            if (allKwargs["parallel_traps"] is not None) & (
                allKwargs["serial_traps"] is not None
            ):
                kwargs_parallelOnly = kwargs | self.arcKwargs
                kwargs_parallelOnly[
                    "serial_traps"
                ] = None  # make a parallel-only corner
                kwargs_serialOnly = kwargs | self.arcKwargs
                kwargs_serialOnly["parallel_traps"] = None  # make a serial-only corner

                (
                    raw_matrix_opt_parallel,
                    correction_matrix_opt_parallel,
                ) = self._estimate_residual_covariance(
                    self.skyFrameSim,
                    self.readnoiseFrameSim,
                    frac_opt,
                    matrix_size=matrix_size,
                    **kwargs_parallelOnly
                )
                (
                    raw_matrix_opt_serial,
                    correction_matrix_opt_serial,
                ) = self._estimate_residual_covariance(
                    self.skyFrameSim,
                    self.readnoiseFrameSim,
                    frac_opt,
                    matrix_size=matrix_size,
                    **kwargs_serialOnly
                )
                (
                    raw_matrix_opt_parallel_noSR,
                    correction_matrix_opt_parallel_noSR,
                ) = self._estimate_residual_covariance(
                    self.skyFrameSim,
                    self.readnoiseFrameSim,
                    0,
                    matrix_size=matrix_size,
                    **kwargs_parallelOnly
                )
                (
                    raw_matrix_opt_serial_noSR,
                    correction_matrix_opt_serial_noSR,
                ) = self._estimate_residual_covariance(
                    self.skyFrameSim,
                    self.readnoiseFrameSim,
                    0,
                    matrix_size=matrix_size,
                    **kwargs_serialOnly
                )

                # package all relevant quantities into optVals and return
                self.optVals["combinedQuadMatrix"] = raw_matrix_opt
                self.optVals["combinedQuadMatrix_diff"] = correction_matrix_opt
                self.optVals["combinedQuadMatrix_noSR"] = raw_matrix_opt_noSR
                self.optVals[
                    "combinedQuadMatrix_noSR_diff"
                ] = correction_matrix_opt_noSR
                self.optVals["parallelQuadMatrix"] = raw_matrix_opt_parallel
                self.optVals["parallelQuadMatrix_diff"] = correction_matrix_opt_parallel
                self.optVals["parallelQuadMatrix_noSR"] = raw_matrix_opt_parallel_noSR
                self.optVals[
                    "parallelQuadMatrix_noSR_diff"
                ] = correction_matrix_opt_parallel_noSR
                self.optVals["serialQuadMatrix"] = raw_matrix_opt_serial
                self.optVals["serialQuadMatrix_diff"] = correction_matrix_opt_serial
                self.optVals["serialQuadMatrix_noSR"] = raw_matrix_opt_serial_noSR
                self.optVals[
                    "serialQuadMatrix_noSR_diff"
                ] = correction_matrix_opt_serial_noSR
                self.optVals["noCTIQuadMatrix"] = raw_matrix_opt_noCTI
                self.optVals["noCTIQuadMatrix_diff"] = correction_matrix_opt_noCTI
                self.optVals["noCTIQuadMatrix_noSR"] = raw_matrix_opt_noCTI
                self.optVals["noCTIQuadMatrix_diff_noSR"] = correction_matrix_opt_noCTI
                self.optVals["benchmark"] = benchmark_matrix
                self.optVals["benchmark_resid"] = residual_benchmark_matrix

    ###############

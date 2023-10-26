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
        sigma_readnoise=0.0,
        adjacency=0.3,
        noise_model_scaling=1.0,  # originally 0.75
        amplitude_scale=0.2,  # originally 0.33
        n_iter=200,
        serial=True,
    ):
        self.sigmaRN = sigma_readnoise
        self.adjacency = adjacency
        self.ampScale = amplitude_scale
        self.outScale = noise_model_scaling
        self.n_iter = n_iter
        self.smoothCol = serial

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
    ###############
    def determine_noise_model(self, imageIn, imageOut):
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
    def estimate_read_noise_model_from_image(self, image):
        ampReadNoise = self.sigmaRN

        imageIn = image
        imageAdj = np.zeros_like(imageIn)  # adjustment image (for read noise modeling)
        imageOut = imageIn.copy()  # output ("smoothed") image

        nrows = imageIn.shape[0]
        ncols = imageIn.shape[1]

        rmsGlobal = 0.0
        nrmsGlobal = 0.0

        smoother = 1
        for s in range(self.n_iter):
            oldchk = ampReadNoise - rmsGlobal
            imageAdj = self.determine_noise_model(imageIn, imageOut)

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
            print(s, rmsGlobal, (ampReadNoise - rmsGlobal))

            chk = ampReadNoise - rmsGlobal
            if chk * oldchk < 0:
                smoother += 1

            if type(ampReadNoise) == np.ndarray:
                if np.max(ampReadNoise - rmsGlobal) < 0.0001:
                    break
            else:
                if abs(ampReadNoise - rmsGlobal) < 0.0001:
                    break
        return imageOut, noiseImage
        # raise NotImplementedError
        # return image

    ###############
    ###############
    def estimate_residual_covariance(
        self,
        background_level,
        # Parallel
        parallel_ccd=None,  # can we do this with **kwargs ? I'm not sure in python
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
    ):
        # Serial
        if self.smoothCol:
            # should be more agnostic about direction or rows/cols (for variable names...)
            serial_ccd = (None,)
            serial_roe = (None,)
            serial_traps = (None,)
            serial_express = (0,)
            serial_window_offset = (0,)
            serial_window_start = (0,)
            serial_window_stop = (-1,)
            serial_time_start = (0,)
            serial_time_stop = (-1,)
            serial_prune_n_electrons = (1e-10,)
            serial_prune_frequency = (20,)
            # Pixel bounce
            pixel_bounce = (None,)
            # Output
            verbosity = 1

        raise NotImplementedError

    ###############
    ###############
    def optimise_parameters(
        self,
        image,
        background_level,
        # figure_of_merit=figure_of_merit(), # should probably pass a function here
        **kwargs
        ## Parallel
        # parallel_ccd=None, #again, should do this with **kwargs
        # parallel_roe=None,
        # parallel_traps=None,
        # parallel_express=0,
        # parallel_window_offset=0,
        # parallel_window_start=0,
        # parallel_window_stop=-1,
        # parallel_time_start=0,
        # parallel_time_stop=-1,
        # parallel_prune_n_electrons=1e-10,
        # parallel_prune_frequency=20,
        ## Serial
        # serial_ccd=None,
        # serial_roe=None,
        # serial_traps=None,
        # serial_express=0,
        # serial_window_offset=0,
        # serial_window_start=0,
        # serial_window_stop=-1,
        # serial_time_start=0,
        # serial_time_stop=-1,
        # serial_prune_n_electrons=1e-10,
        # serial_prune_frequency=20,
        ## Pixel bounce
        # pixel_bounce=None,
        ## Output
        # verbosity=1,
    ):
        # things to compare over iterations
        outputFrame, noiseFrame = self.estimate_read_noise_model_from_image(image)
        covar = self.covariance_matrix_from_image(image)
        raise NotImplementedError

    ###############
    ###############
    def figure_of_merit(self, covariance_matrix):
        xlen = covariance_matrix.shape[1] // 2
        ylen = covariance_matrix.shape[0] // 2
        fullsum = np.sum(covariance_matrix)  # sum over all cells
        rowsum = np.sum(
            covariance_matrix[ylen, :]
        )  # sum over central row (for serial CTI)
        colsum = np.sum(
            covariance_matrix[:, xlen]
        )  # sum over central column (for parallel CTI)
        # raise NotImplementedError
        return fullsum, rowsum, colsum

    ###############
    ###############
    def covariance_matrix_from_image(self, image, n_pixels=5):
        # raise NotImplementedError
        covariance_matrix = np.zeros(n_pixels, n_pixels)
        # calcluate mean stats on image
        x = image.flatten()
        xbar = np.mean(x)

        # roll image to get cross correlation
        matRange = n_pixels // 2
        for i in range(-matRange, matRange + 1, 1):
            for j in range(-matRange, matRange + 1, 1):
                y = np.roll(img, (-j, -i), axis=(0, 1)).flatten()
                ybar = np.mean(y)

                # calculate covariance
                covar = (
                    np.sum((x - xbar) * (y - ybar))
                    / (pl.std(x - xbar) * pl.std(y - ybar))
                    / (x.size)
                )

                # populate matrix cells
                covariance_matrix[i + 2, j + 2] = covar

        return covariance_matrix

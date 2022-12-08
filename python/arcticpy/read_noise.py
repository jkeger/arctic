import numpy as np
import arcticpy as ac
from scipy.optimize import curve_fit

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
            noise_model_scaling=1.0, # originally 0.75 
            amplitude_scale=0.2,     # originally 0.33
            n_iter=200,
            serial=True,
            **kwargs
            
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
            value = 0.
        self._sigma = value
    ###############
    ###############
    def determine_noise_model(self, imageIn, imageOut):
        '''
        Method for estimating readnoise on image (in S+R fashion)
        assumes parallel+serial by default
        '''
        readNoiseAmp = self.sigmaRN
        
        dval0 = imageIn - imageOut
        dval0u = dval0.copy()
        
        dval0u[dval0u>1] = 1
        dval0u[dval0u<-1] = -1
        
        dval9 = imageIn*0
        dcount = imageIn*0
        
        dval9[:-1,:-1]+=(imageIn[1: ,1: ]-imageOut[1: ,1: ]); dcount[:-1,:-1]+=1
        dval9[:-1,:  ]+=(imageIn[1: ,:  ]-imageOut[1: ,:  ]); dcount[:-1,:  ]+=1
        dval9[:-1,1: ]+=(imageIn[1: ,:-1]-imageOut[1: ,:-1]); dcount[:-1,1: ]+=1
        dval9[:  ,:-1]+=(imageIn[:  ,1: ]-imageOut[:  ,1: ]); dcount[:  ,:-1]+=1
        dval9[:  ,:  ]+=(imageIn[:  ,:  ]-imageOut[:  ,:  ]); dcount[:  ,:  ]+=1
        dval9[:  ,1: ]+=(imageIn[:  ,:-1]-imageOut[:  ,:-1]); dcount[:  ,1: ]+=1
        dval9[1: ,:-1]+=(imageIn[:-1,1: ]-imageOut[:-1,1: ]); dcount[1: ,:-1]+=1
        dval9[1: ,:  ]+=(imageIn[:-1,:  ]-imageOut[:-1,:  ]); dcount[1: ,:  ]+=1
        dval9[1: ,1: ]+=(imageIn[:-1,:-1]-imageOut[:-1,:-1]); dcount[1: ,1: ]+=1
        
        dval9/=dcount
        
        readNoiseAmpFraction = self.ampScale
        dval9u = dval9.copy()
        if type(readNoiseAmp)==np.ndarray:
            dval9u = dval9u.flatten()
            readNoiseAmp = readNoiseAmp.flatten()
            dval9u[dval9u > readNoiseAmp*readNoiseAmpFraction] = (readNoiseAmp*readNoiseAmpFraction)[dval9u > readNoiseAmp*readNoiseAmpFraction]
            dval9u[dval9u < readNoiseAmp*-readNoiseAmpFraction] = (readNoiseAmp*-readNoiseAmpFraction)[dval9u < readNoiseAmp*-readNoiseAmpFraction]
            dval9u = dval9u.reshape(imageIn.shape)
            readNoiseAmp = readNoiseAmp.reshape(imageIn.shape)        
        else:
            dval9u[dval9u > readNoiseAmp*readNoiseAmpFraction] = readNoiseAmp*readNoiseAmpFraction
            dval9u[dval9u < readNoiseAmp*-readNoiseAmpFraction] = readNoiseAmp*-readNoiseAmpFraction
            
        dmod1 = imageIn*0
        dmod1[1:,:] = imageOut[:-1,:] - imageOut[1:,:]
        dmod2 = imageIn*0
        dmod2[:-1,:] = imageOut[1:,:] - imageOut[:-1,:]
        
        if self.smoothCol:
            cmod1 = imageIn*0
            cmod1[:,1:] = imageOut[:,:-1] - imageOut[:,1:]
            cmod2 = imageIn*0
            cmod2[:,:-1] = imageOut[:,1:] - imageOut[:,:-1]
            
        dmod1u = dmod1.copy()
        if type(readNoiseAmp)==np.ndarray:
            dmod1u = dmod1u.flatten()
            readNoiseAmp = readNoiseAmp.flatten()
            dmod1u[dmod1u>readNoiseAmp*readNoiseAmpFraction] = (readNoiseAmp*readNoiseAmpFraction)[dmod1u>readNoiseAmp*readNoiseAmpFraction]
            dmod1u[dmod1u<readNoiseAmp*-readNoiseAmpFraction] = (readNoiseAmp*-readNoiseAmpFraction)[dmod1u<readNoiseAmp*-readNoiseAmpFraction]
            dmod1u = dmod1u.reshape(imageIn.shape)       
        else:
            dmod1u[dmod1u>readNoiseAmp*readNoiseAmpFraction] = readNoiseAmp*readNoiseAmpFraction
            dmod1u[dmod1u<readNoiseAmp*-readNoiseAmpFraction] = readNoiseAmp*-readNoiseAmpFraction
            
        dmod2u = dmod2.copy()
        if type(readNoiseAmp)==np.ndarray:
            dmod2u = dmod2u.flatten()
            readNoiseAmp = readNoiseAmp.flatten()
            dmod2u[dmod2u>readNoiseAmp*readNoiseAmpFraction] = (readNoiseAmp*readNoiseAmpFraction)[dmod2u>readNoiseAmp*readNoiseAmpFraction]
            dmod2u[dmod2u<readNoiseAmp*-readNoiseAmpFraction] = (readNoiseAmp*-readNoiseAmpFraction)[dmod2u<readNoiseAmp*-readNoiseAmpFraction]
            dmod2u = dmod2u.reshape(imageIn.shape)
            readNoiseAmp = readNoiseAmp.reshape(imageIn.shape) 
        else:
            dmod2u[dmod2u>readNoiseAmp*readNoiseAmpFraction] = readNoiseAmp*readNoiseAmpFraction
            dmod2u[dmod2u<readNoiseAmp*-readNoiseAmpFraction] = readNoiseAmp*-readNoiseAmpFraction
            
        if self.smoothCol:
            cmod1u = cmod1.copy()
            if type(readNoiseAmp)==np.ndarray:
                cmod1u = cmod1u.flatten()
                readNoiseAmp = readNoiseAmp.flatten()
                cmod1u[cmod1u>readNoiseAmp*readNoiseAmpFraction] = (readNoiseAmp*readNoiseAmpFraction)[cmod1u>readNoiseAmp*readNoiseAmpFraction]
                cmod1u[cmod1u<readNoiseAmp*-readNoiseAmpFraction] = (readNoiseAmp*-readNoiseAmpFraction)[cmod1u<readNoiseAmp*-readNoiseAmpFraction]
                cmod1u = cmod1u.reshape(imageIn.shape)
                readNoiseAmp = readNoiseAmp.reshape(imageIn.shape)
            else:
                cmod1u[cmod1u>readNoiseAmp*readNoiseAmpFraction] = readNoiseAmp*readNoiseAmpFraction
                cmod1u[cmod1u<readNoiseAmp*-readNoiseAmpFraction] = readNoiseAmp*-readNoiseAmpFraction
                
            cmod2u = cmod2.copy()
            if type(readNoiseAmp)==np.ndarray:
                cmod2u = cmod2u.flatten()
                readNoiseAmp = readNoiseAmp.flatten()
                cmod2u[cmod2u>readNoiseAmp*readNoiseAmpFraction] = (readNoiseAmp*readNoiseAmpFraction)[cmod2u>readNoiseAmp*readNoiseAmpFraction]
                cmod2u[cmod2u<readNoiseAmp*-readNoiseAmpFraction] = (readNoiseAmp*-readNoiseAmpFraction)[cmod2u<readNoiseAmp*-readNoiseAmpFraction]
                cmod2u = cmod2u.reshape(imageIn.shape)
                readNoiseAmp = readNoiseAmp.reshape(imageIn.shape)          
            else:
                cmod2u[cmod2u>readNoiseAmp*readNoiseAmpFraction] = readNoiseAmp*readNoiseAmpFraction
                cmod2u[cmod2u<readNoiseAmp*-readNoiseAmpFraction] = readNoiseAmp*-readNoiseAmpFraction
                
        readNoiseAmp2 = readNoiseAmp**2
        w0 =     dval0 * dval0 / (dval0 * dval0 + 4.0 * readNoiseAmp2)
        w9 =     dval9 * dval9 / (dval9 * dval9 + 18.0 * readNoiseAmp2)
        w1 = 4 * readNoiseAmp2 / (dmod1 * dmod1 + 4.0 * readNoiseAmp2)
        w2 = 4 * readNoiseAmp2 / (dmod2 * dmod2 + 4.0 * readNoiseAmp2)
        if self.smoothCol:
            wc1 = 4 * readNoiseAmp2 / (cmod1 * cmod1 + 4.0 * readNoiseAmp2)
            wc2 = 4 * readNoiseAmp2 / (cmod2 * cmod2 + 4.0 * readNoiseAmp2)    
            
        if self.smoothCol:
            return  (dval0u * w0 / 6) + (dval9u * w9 / 6) +(dmod1u * w1 / 6) + (dmod2u * w2 / 6) +(cmod1u * wc1 / 6) + (cmod2u * wc2 / 6)
        else:
            return  (dval0u * w0 * 0.25) + (dval9u * w9 * 0.25) +(dmod1u * w1 * 0.25) + (dmod2u * w2 * 0.25)
    ###############
    ###############
    def estimate_read_noise_model_from_image(self, image):
        ampReadNoise = self.sigmaRN

        imageIn = image
        imageAdj = np.zeros_like(imageIn) #adjustment image (for read noise modeling)
        imageOut = imageIn.copy()         #output ("smoothed") image
        
        nrows = imageIn.shape[0]
        ncols = imageIn.shape[1]
        
        rmsGlobal = 0.
        nrmsGlobal = 0.
        
        smoother = 1
        print('iter','model_rms','target_rms','residual')
        for s in range(self.n_iter):
            oldchk = ampReadNoise-rmsGlobal
            imageAdj= self.determine_noise_model(imageIn,imageOut)

            if (ampReadNoise-rmsGlobal) > 0:
                imageOut += imageAdj * self.outScale / smoother
                noiseImage = imageIn - imageOut
            else:
                imageOut -= imageAdj * self.outScale / smoother
                noiseImage = imageIn - imageOut
        
            cond = abs(imageIn > 0.1)|abs(imageOut > 0.1)
            rmsGlobal = np.sum(noiseImage[cond]**2)
            nrmsGlobal = noiseImage[cond].size

            rmsGlobal = np.sqrt(rmsGlobal/nrmsGlobal)
            print("\033[K",end='\r')
            print('%4d  %f    %5.2f    %f'%(s,rmsGlobal, self.sigmaRN, (ampReadNoise-rmsGlobal)),end='')
            
            chk = ampReadNoise-rmsGlobal
            if chk*oldchk < 0:
                smoother+=1
                
            if type(ampReadNoise)==np.ndarray:
                if np.max(ampReadNoise - rmsGlobal) < 0.0001:
                    break
            else:
                if abs(ampReadNoise - rmsGlobal) < 0.0001:
                    break
        return imageOut,noiseImage
        #raise NotImplementedError
        #return image
    ###############
    ###############
    def estimate_residual_covariance(
            self, 
            background_level,
            background_sigma=None,
            chip_size=500,  #size, in pixels, of CCD (assumes square)
            matrix_size=5, #size of covariance grid, in pixels (also assumes square)
            **kwargs
    ):
        if background_sigma==None:
            background_sigma = np.sqrt(background_level) #if sky sigma is not set, assume sqrt(N)
            
        #create sky frame    
        skyFrame = np.random.normal(background_level, background_sigma, (chip_size,chip_size))        
        #add CTI effects
        ctiTrailedFrame = ac.add_cti(skyFrame,**kwargs)
        #add read noise
        noiseFrame = np.random.normal(0, self.sigmaRN, (chip_size,chip_size))
        readNoiseAddedFrame = ctiTrailedFrame + noiseFrame
        #do S+R routine
        sFrame, rFrame = self.estimate_read_noise_model_from_image(readNoiseAddedFrame)
        #clean CTI from S frame
        ctiCorrectedFrame = ac.remove_cti(sFrame,1,**kwargs)
        #re-add R frame to correction
        outputFrame = ctiCorrectedFrame + rFrame

        #determine covariance FOMs
        fomSR = self.covariance_matrix_from_image(outputFrame,matrix_size=matrix_size)
        fomBenchmark = self.covariance_matrix_from_image(skyFrame+noiseFrame,matrix_size=matrix_size)
        fomDiff = fomSR - fomBenchmark
        
        return fomSR,fomDiff
        
        #raise NotImplementedError
        #return np.array([[0,0,0,0,0],[0,0,0,0,0],[0,0,1,0,0],[0,0,0,0,0],[0,0,0,0,0]])

    ###############
    ###############
    def _estimate_residual_covariance_for_opt(
            self, 
            background_level,
            sr_fraction,
            background_sigma=None,
            chip_size=500,  #size, in pixels, of CCD (assumes square)
            matrix_size=5, #size of covariance grid, in pixels (also assumes square)
            **kwargs
    ):
        if background_sigma==None:
            background_sigma = np.sqrt(background_level) #if sky sigma is not set, assume sqrt(N)
        
        #create sky frame    
        skyFrame = np.random.normal(background_level, background_sigma, (chip_size,chip_size))        
        #add CTI effects
        ctiTrailedFrame = ac.add_cti(skyFrame,**kwargs)
        #add read noise
        noiseFrame = np.random.normal(0, self.sigmaRN, (chip_size,chip_size))
        readNoiseAddedFrame = ctiTrailedFrame + noiseFrame
        #do S+R routine
        sFrame, rFrame = self.estimate_read_noise_model_from_image(readNoiseAddedFrame)
        #rescale S+R correction to a fractional value (for optimization)
        sFrame+=rFrame
        rFrame*=sr_fraction
        sFrame-=rFrame 
        #clean CTI from S frame
        ctiCorrectedFrame = ac.remove_cti(sFrame,1,**kwargs)
        #re-add R frame to correction
        outputFrame = ctiCorrectedFrame + rFrame

        #determine covariance FOMs
        fomSR = self.covariance_matrix_from_image(outputFrame,matrix_size=matrix_size)
        fomBenchmark = self.covariance_matrix_from_image(skyFrame+noiseFrame,matrix_size=matrix_size)
        fomDiff = fomSR - fomBenchmark

        return fomSR,fomDiff
    ###############
    ###############
    def _create_initial_sim_images(
            self,
            background_level,
            background_sigma,
            image_size=500
    ):
        '''
        subroutine to generate sky_background and readnoise frames for S+R simulations
        These will stay fixed in each simulation, to control random noise fluctuations
        '''
        
        #if background_sigma==None:
        #    background_sigma = np.sqrt(background_level) #if sky sigma is not set, assume sqrt(N)

        #create sky frame    
        skyFrame = np.random.normal(background_level, background_sigma, (image_size,image_size))
        #create readnoise_frame
        noiseFrame = np.random.normal(0, self.sigmaRN, (image_size,image_size))

        return skyFrame,noiseFrame                                    
    
    ###############    
    def _estimate_residual_covariance_for_opt_premade(
            self,
            skyFrame,
            noiseFrame,
            sr_fraction,
            matrix_size=5, #size of covariance grid, in pixels (also assumes square)
            **kwargs
    ):
        '''
        Estimate a covariance matrix for an optimising simulation, assuming the sky/noise frames have already been created
        '''
        #add CTI effects to skyFrame
        ctiTrailedFrame = ac.add_cti(skyFrame,**kwargs)
        #add read noise to STI-trailed image
        readNoiseAddedFrame = ctiTrailedFrame + noiseFrame
        #do S+R routine
        sFrame, rFrame = self.estimate_read_noise_model_from_image(readNoiseAddedFrame)
        #rescale S+R correction to a fractional value (for optimization)
        sFrame+=rFrame
        rFrame*=sr_fraction
        sFrame-=rFrame 
        #clean CTI from S frame
        ctiCorrectedFrame = ac.remove_cti(sFrame,1,**kwargs)
        #re-add R frame to correction
        outputFrame = ctiCorrectedFrame + rFrame

        #determine covariance FOMs
        fomSR = self.covariance_matrix_from_image(outputFrame,matrix_size=matrix_size)
        fomBenchmark = self.covariance_matrix_from_image(skyFrame+noiseFrame,matrix_size=matrix_size)
        fomDiff = fomSR - fomBenchmark

        return fomSR,fomDiff    
    ###############
    def _fitter_function(self,x,intercept,slope):
        y = intercept + slope * x
        return y
    ###############
    def optimise_parameters(
            self,
            background_level,
            background_sigma=None,
            chip_size=2048,   #the size of the entire CCD over which the procedure will be run
            subchip_size=500, #size of the cutout region, to test CTI behaviour in different regimes (e.g. far from readout, close to readout, etc.) 
            matrix_size=5,    #covariance matrix size (NxN array)
            **kwargs
    ):
        '''
        S+R optimisation routine. Given a specified set of CTI trap parameters, this routine calculates the optimum S+R fraction to minimise image correlations during correction
        '''

        #if sky sigma is not set, assume sqrt(N)
        if background_sigma==None:
            background_sigma = np.sqrt(background_level)  
        
        #set prescan regions (this simulates the maximum possible CTI trailing, averaged at the far edge of the real CCD)
        if 'parallel_roe' in kwargs:
            kwargs['parallel_roe'].prescan_offset = chip_size+(subchip_size//2)
        if 'serial_roe' in kwargs:
            kwargs['serial_roe'].prescan_offset = chip_size+(subchip_size//2)

        #container for optimised values
        optVals = {}

        #pre-make the sky background and readnoise images, keep these constant over all quadrants
        skyFrame, readNoiseFrame = self._create_initial_sim_images(background_level,background_sigma,image_size=subchip_size)

        #run S+R routine at 0% and 100% S+R fraction
        result_array = np.array([])
        matrix_array = np.zeros((2,matrix_size,matrix_size))
        sr_frac = np.array([0.0,1.0])
        for frac in sr_frac:
            raw_matrix,correction_matrix = self._estimate_residual_covariance_for_opt_premade(skyFrame,readNoiseFrame,frac,matrix_size=matrix_size,**kwargs)
            matrix_array[int(frac)] = correction_matrix
            result = self.figure_of_merit(correction_matrix) #TODO: check to see if changing subgrid_size actually matters
            result_array = np.append(result_array,result)
            print('both progression:',result_array)
        #interpolate between the results to determine the point of minimum residual covariance
        a_fit,cov=curve_fit(self._fitter_function,sr_frac,result_array,absolute_sigma=True)
        m = a_fit[1]
        b = a_fit[0]
        xint = -b/m

        frac_opt = xint #set the optimized S+R fraction

        #re-run the S+R routine with the optimised level
        raw_matrix_opt,correction_matrix_opt = self._estimate_residual_covariance_for_opt_premade(skyFrame,readNoiseFrame,frac_opt,matrix_size=matrix_size,**kwargs)

        #run a second S+R routine in the region close to the readout registers, to identify/remove stochastic simulation noise
        kwargs_noCTI = kwargs.copy()
        kwargs_noCTI['parallel_traps']=None 
        kwargs_noCTI['serial_traps']=None
        #effectively, we are removing the effects of CTI, but still calculating a covariance matrix
        raw_matrix_opt_noCTI,correction_matrix_opt_noCTI = self._estimate_residual_covariance_for_opt_premade(skyFrame,readNoiseFrame,frac_opt,matrix_size=matrix_size,**kwargs_noCTI)

        #create the "benchmark" covariance matrix: a purely non-correlated array with the central pixel equal to (readnoise_sigma**2 + background_sigma**2)
        #Any difference between this and "raw_matrix_opt_noCTI" is the simulation noise, and can be eliminated
        benchmark_matrix = raw_matrix_opt_noCTI*0
        centrePix = matrix_size//2
        benchmark_matrix[centrePix,centrePix] = self.sigmaRN**2 + background_sigma**2
        #calculate the simulation-noise residual
        residual_benchmark_matrix = raw_matrix_opt_noCTI - benchmark_matrix

        #Finally, if both parallel and serial CTI are present, calculate covariances of the "cross-region" chip quadrants (far from serial readout but close to parallel readout, and vice-versa)
        if ('parallel_traps' in kwargs)&('serial_traps' in kwargs):
            if (kwargs['parallel_traps'] is not None)&(kwargs['serial_traps'] is not None):
                kwargs_parallelOnly = kwargs.copy()
                kwargs_parallelOnly['serial_traps']=None #make a parallel-only quadrant
                kwargs_serialOnly = kwargs.copy()
                kwargs_serialOnly['parallel_traps']=None #make a serial-only quadrant

                raw_matrix_opt_parallel,correction_matrix_opt_parallel = self._estimate_residual_covariance_for_opt_premade(skyFrame,readNoiseFrame,frac_opt,matrix_size=matrix_size,**kwargs_parallelOnly)
                raw_matrix_opt_serial,correction_matrix_opt_serial = self._estimate_residual_covariance_for_opt_premade(skyFrame,readNoiseFrame,frac_opt,matrix_size=matrix_size,**kwargs_serialOnly)

                #package all relevant quantities into optVals and return
                optVals['optSRfrac'] = frac_opt
                optVals['combinedQuadMatrix'] = raw_matrix_opt
                optVals['combinedQuadMatrix_diff'] = correction_matrix_opt
                optVals['parallelQuadMatrix'] = raw_matrix_opt_parallel
                optVals['parallelQuadMatrix_diff'] = correction_matrix_opt_parallel
                optVals['serialQuadMatrix'] = raw_matrix_opt_serial
                optVals['serialQuadMatrix_diff'] = correction_matrix_opt_serial
                optVals['noCTIQuadMatrix'] = raw_matrix_opt_noCTI
                optVals['noCTIQuadMatrix_diff'] = correction_matrix_opt_noCTI
                optVals['benchmark'] = benchmark_matrix
                optVals['benchmark_resid'] = residual_benchmark_matrix

        return optVals                 
    ###############        
#    ###############
#    def optimise_parameters(
#        self,
#        background_level,
#        chip_size=500,
#        matrix_size=5,
#        **kwargs    
#        ##figure_of_merit=figure_of_merit(), # should probably pass a function here
#    ):
#        '''
#        S+R optimisation routine. Given a specified set of CTI trap parameters, this routine calculates the optimum S+R fraction to minimise image correlations during correction
#        '''
#        optVals = {}
#        
#        kwargs['parallel_roe'].prescan_offset = 1900
#        kwargs['serial_roe'].prescan_offset = 1900
#        
#        if 'parallel_traps' in kwargs:
#            if kwargs['parallel_traps'] is not None:
#                kwargs_parallel = kwargs.copy()
#                kwargs_parallel['serial_traps']=None #make a parallel-only quadrant
#                #things to compare over iterations
#                result_array = np.array([])
#                matrix_array = np.zeros((11,matrix_size,matrix_size))
#                sr_frac = np.arange(0,1.01,0.1)##np.array([0.0,0.5,1.0])##
#                for frac in sr_frac:
#                    raw_matrix,correction_matrix = self._estimate_residual_covariance_for_opt(background_level,frac,chip_size=chip_size,matrix_size=matrix_size,**kwargs_parallel)
#                    matrix_array[int(frac*10)] = correction_matrix
#                    result = self.figure_of_merit(correction_matrix) #see if changing subgrid_size actually matters
#                    result_array = np.append(result_array,result)
#                    print('parallel progression:',result_array) 
#                    
#                a_fit,cov=curve_fit(self._fitter_function,sr_frac,result_array,absolute_sigma=True)
#                m = a_fit[1]
#                b = a_fit[0]
#                xint = -b/m
#
#                lo = np.floor(xint*10).astype(int)
#                hi = np.ceil(xint*10).astype(int)
#                mean_grid = (matrix_array[lo]+matrix_array[hi])/2
#                
#                optVals['parallelQuadMatrix'] = mean_grid
#                optVals['parallelQuadFrac'] = xint
#        
#        if 'serial_traps' in kwargs:
#            if kwargs['serial_traps'] is not None:
#                kwargs_serial = kwargs.copy()
#                kwargs_serial['parallel_traps']=None #make a serial-only quadrant
#                #things to compare over iterations
#                result_array = np.array([])
#                matrix_array = np.zeros((11,matrix_size,matrix_size))
#                sr_frac = np.arange(0,1.01,0.1)##np.array([0.0,0.5,1.0])##
#                for frac in sr_frac:
#                    raw_matrix,correction_matrix = self._estimate_residual_covariance_for_opt(background_level,frac,chip_size=chip_size,matrix_size=matrix_size,**kwargs_serial)
#                    matrix_array[int(frac*10)] = correction_matrix
#                    result = self.figure_of_merit(correction_matrix) #see if changing subgrid_size actually matters
#                    result_array = np.append(result_array,result)
#                    print('serial progression:',result_array) 
#                    
#                a_fit,cov=curve_fit(self._fitter_function,sr_frac,result_array,absolute_sigma=True)
#                m = a_fit[1]
#                b = a_fit[0]
#                xint = -b/m
#
#                lo = np.floor(xint*10).astype(int)
#                hi = np.ceil(xint*10).astype(int)
#                mean_grid = (matrix_array[lo]+matrix_array[hi])/2
#
#                optVals['serialQuadMatrix'] = mean_grid
#                optVals['serialQuadFrac'] = xint
#        
#        if ('parallel_traps' in kwargs)&('serial_traps' in kwargs):
#            if (kwargs['parallel_traps'] is not None)&(kwargs['serial_traps'] is not None):
#                #things to compare over iterations
#                result_array = np.array([])
#                matrix_array = np.zeros((11,matrix_size,matrix_size))
#                sr_frac = np.arange(0,1.01,0.1)##np.array([0.0,0.5,1.0])##
#                for frac in sr_frac:
#                    raw_matrix,correction_matrix = self._estimate_residual_covariance_for_opt(background_level,frac,chip_size=chip_size,matrix_size=matrix_size,**kwargs)
#                    matrix_array[int(frac*10)] = correction_matrix
#                    result = self.figure_of_merit(correction_matrix) #see if changing subgrid_size actually matters
#                    result_array = np.append(result_array,result)
#                    print('both progression:',result_array) 
#                    
#                a_fit,cov=curve_fit(self._fitter_function,sr_frac,result_array,absolute_sigma=True)
#                m = a_fit[1]
#                b = a_fit[0]
#                xint = -b/m
#
#                lo = np.floor(xint*10).astype(int)
#                hi = np.ceil(xint*10).astype(int)
#                mean_grid = (matrix_array[lo]+matrix_array[hi])/2
#
#                optVals['combinedQuadMatrix'] = mean_grid
#                optVals['combinedQuadFrac'] = xint
#            
#        
#        return optVals
#            
#        ##outputFrame,noiseFrame = self.estimate_read_noise_model_from_image(image)
#        ##covar = self.covariance_matrix_from_image(image)
#
#    ###############
    ###############
    def optimise_parameters_from_image(
        self,
        image,
        background_level=None,
        chip_size=500,
        matrix_size=5,
        **kwargs    
    ):
        '''
        version of the S+R optimiser routine, but parameters can be determined from a supplied image rather than manually specified
        '''
        optVals = {}
        image_nrows = image.shape[0]
        image_ncols = image.shape[1]

        #use CTI prescan offset to ensure sample region is at the far edge of the full chip
        kwargs['parallel_roe'].prescan_offset = kwargs['parallel_roe'].prescan_offset + image_nrows - chip_size//2
        kwargs['serial_roe'].prescan_offset = kwargs['serial_roe'].prescan_offset + image_ncols - chip_size//2

        #if background nevel is not specified, estimate using the median pixel value
        if background_level == None:
            background_level = np.median(image)
            
        
        if 'parallel_traps' in kwargs:
            if kwargs['parallel_traps'] is not None:
                kwargs_parallel = kwargs.copy()
                kwargs_parallel['serial_traps']=None #make a parallel-only quadrant
                ###use CTI prescan offset to ensure sample region is at the far edge of the full chip
                ##kwargs_parallel['parallel_roe'].prescan_offset = kwargs_parallel['parallel_roe'].prescan_offset + image_nrows - chip_size//2
                #things to compare over iterations
                result_array = np.array([])
                matrix_array = np.zeros((11,matrix_size,matrix_size))
                sr_frac =  np.array([0.0,0.5,1.0])##np.arange(0,1.01,0.1)
                for frac in sr_frac:
                    correction_matrix = self._estimate_residual_covariance_for_opt(background_level,frac,**kwargs_parallel)
                    matrix_array[int(frac*10)] = correction_matrix
                    result = self.figure_of_merit(correction_matrix) #see if changing subgrid_size actually matters
                    result_array = np.append(result_array,result)
                    
                a_fit,cov=curve_fit(self._fitter_function,sr_frac,result_array,absolute_sigma=True)
                m = a_fit[1]
                b = a_fit[0]
                xint = -b/m

                lo = np.floor(xint*10).astype(int)
                hi = np.ceil(xint*10).astype(int)
                mean_grid = (matrix_array[lo]+matrix_array[hi])/2
                
                optVals['parallelQuadMatrix'] = mean_grid
                optVals['parallelQuadFrac'] = xint
        
        if 'serial_traps' in kwargs:
            if kwargs['serial_traps'] is not None:
                kwargs_serial = kwargs.copy()
                kwargs_serial['parallel_traps']=None #make a serial-only quadrant
                ###use CTI prescan offset to ensure sample region is at the far edge of the full chip
                ##kwargs_serial['serial_roe'].prescan_offset = kwargs_serial['serial_roe'].prescan_offset + image_ncols - chip_size//2
                #things to compare over iterations
                result_array = np.array([])
                matrix_array = np.zeros((11,matrix_size,matrix_size))
                sr_frac = np.array([0.0,0.5,1.0])##np.arange(0,1.01,0.1)
                for frac in sr_frac:
                    correction_matrix = self._estimate_residual_covariance_for_opt(background_level,frac,**kwargs_serial)
                    matrix_array[int(frac*10)] = correction_matrix
                    result = self.figure_of_merit(correction_matrix) #see if changing subgrid_size actually matters
                    result_array = np.append(result_array,result)
                    
                a_fit,cov=curve_fit(self._fitter_function,sr_frac,result_array,absolute_sigma=True)
                m = a_fit[1]
                b = a_fit[0]
                xint = -b/m

                lo = np.floor(xint*10).astype(int)
                hi = np.ceil(xint*10).astype(int)
                mean_grid = (matrix_array[lo]+matrix_array[hi])/2

                optVals['serialQuadMatrix'] = mean_grid
                optVals['serialQuadFrac'] = xint
        
        if ('parallel_traps' in kwargs)&('serial_traps' in kwargs):
            if (kwargs['parallel_traps'] is not None)&(kwargs['serial_traps'] is not None):
                ###use CTI prescan offset to ensure sample region is at the far edge of the full chip
                ##kwargs_parallel['parallel_roe'].prescan_offset = kwargs_parallel['parallel_roe'].prescan_offset + image_nrows - chip_size//2
                ##kwargs_serial['serial_roe'].prescan_offset = kwargs_serial['serial_roe'].prescan_offset + image_ncols - chip_size//2
                #things to compare over iterations
                result_array = np.array([])
                matrix_array = np.zeros((11,matrix_size,matrix_size))
                sr_frac =  np.array([0.0,0.5,1.0])##np.arange(0,1.01,0.1)
                for frac in sr_frac:
                    correction_matrix = self._estimate_residual_covariance_for_opt(background_level,frac,**kwargs)
                    matrix_array[int(frac*10)] = correction_matrix
                    result = self.figure_of_merit(correction_matrix) #see if changing subgrid_size actually matters
                    result_array = np.append(result_array,result)
                    
                a_fit,cov=curve_fit(self._fitter_function,sr_frac,result_array,absolute_sigma=True)
                m = a_fit[1]
                b = a_fit[0]
                xint = -b/m

                lo = np.floor(xint*10).astype(int)
                hi = np.ceil(xint*10).astype(int)
                mean_grid = (matrix_array[lo]+matrix_array[hi])/2

                optVals['combinedQuadMatrix'] = mean_grid
                optVals['combinedQuadFrac'] = xint
            
        
        return optVals
            
        ##outputFrame,noiseFrame = self.estimate_read_noise_model_from_image(image)
        ##covar = self.covariance_matrix_from_image(image)

    ###############    
    ###############
    def figure_of_merit(self, covariance_matrix, fom_method='box',subgrid_size=None):
        '''
        function for estimating the covariance figure of merit: the average value over some section of a covariance matrix
        if method == 'box' take a square region around the central pixel; 'row' is along the central row; column is along the central column
        if subgrid size is selected, use only an NxN subset of pixels in the matrix, starting from the center outward
        '''
        xlen = covariance_matrix.shape[1]
        ylen = covariance_matrix.shape[0]
        xcenter = xlen//2
        ycenter = ylen//2
        xstart,xend = 0,xlen
        ystart,yend = 0,ylen
        if subgrid_size is not None:
            xdiff = (xlen-subgrid_size)//2
            ydiff = (ylen-subgrid_size)//2
            xstart+=xdiff
            xend-=xdiff
            ystart+=ydiff
            yend-=ydiff

        covariance_matrix[ycenter,xcenter] = 0 #supress autocorrelation pixel    
            
        if fom_method == 'box':
            result_mean = (np.sum(covariance_matrix[ystart:yend,xstart:xend]) - covariance_matrix[ycenter,xcenter])/8   #average over central box
        if fom_method == 'row':
            result_mean = (np.sum(covariance_matrix[ycenter,xstart:xend]) - covariance_matrix[ycenter,xcenter])/4  #average over central row (for serial CTI)
        if fom_method == 'column':
            result_mean = (np.sum(covariance_matrix[ystart:yend,xcenter]) - covariance_matrix[ycenter,xcenter])/4  #sum over central column (for parallel CTI)
        #raise NotImplementedError
        return result_mean

    ###############
    ###############
    def covariance_matrix_from_image(self, image, matrix_size=5):
        
        covariance_matrix = np.zeros((matrix_size,matrix_size))
        #calcluate mean stats on image
        image2 = image[5:,5:] #remove FPR decrement
        x = image2.flatten()
        xbar = np.mean(x)

        #roll image to get cross correlation
        matRange= matrix_size//2
        for i in range(-matRange,matRange+1,1):
            for j in range(-matRange,matRange+1,1):
                y = np.roll(image2,(-j,-i),axis=(0,1)).flatten()
                ybar = np.mean(y)

                #calculate covariance
                covar = np.sum((x-xbar)*(y-ybar))/x.size #/(np.std(x-xbar)*np.std(y-ybar))/(x.size) #removing noise-level normalization
                #covar = np.sum((x-xbar)*(y-ybar))/(np.std(x-xbar)*np.std(y-ybar))/(x.size)
                #populate matrix cells
                covariance_matrix[i+matRange,j+matRange] = covar
        #covariance_matrix[matRange,matRange] = 0 #switch this normalization to plotting
                
        return covariance_matrix


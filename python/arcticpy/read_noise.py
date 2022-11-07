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
            chip_size=2048,  #size, in pixels, of CCD (assumes square)
            matrix_size=5, #size of covariance grid, in pixels (also assumes square)
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
        if background_sigma==None:
            background_sigma = np.sqrt(background_level) #if sky sigma is not set, assume sqrt(N)
            
        #create sky frame    
        skyFrame = np.random.normal(background_level, background_sigma, (chip_size,chip_size))        
        #add CTI effects
        ctiTrailedFrame = ac.add_cti(skyFrame,parallel_roe=parallel_roe,parallel_ccd=parallel_ccd,parallel_traps=parallel_traps,parallel_express=parallel_express,serial_roe=serial_roe,serial_ccd=serial_ccd,serial_traps=serial_traps,serial_express=serial_express)
        #add read noise
        noiseFrame = np.random.normal(0, self.sigmaRN, (chip_size,chip_size))
        readNoiseAddedFrame = ctiTrailedFrame + noiseFrame
        #do S+R routine
        sFrame, rFrame = self.estimate_read_noise_model_from_image(readNoiseAddedFrame)
        #clean CTI from S frame
        ctiCorrectedFrame = ac.remove_cti(sFrame,1,parallel_roe=parallel_roe,parallel_ccd=parallel_ccd,parallel_traps=parallel_traps,parallel_express=parallel_express,serial_roe=serial_roe,serial_ccd=serial_ccd,serial_traps=serial_traps,serial_express=serial_express)
        #re-add R frame to correction
        outputFrame = ctiCorrectedFrame + rFrame

        #determine covariance FOMs
        fomSR = self.covariance_matrix_from_image(outputFrame,matrix_size=matrix_size)
        fomBenchmark = self.covariance_matrix_from_image(skyFrame+noiseFrame,matrix_size=matrix_size)
        fomDiff = fomSR - fomBenchmark
        
        return fomDiff
        
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
            ### Parallel
            ##parallel_ccd=None, #can we do this with **kwargs ? I'm not sure in python
            ##parallel_roe=None,
            ##parallel_traps=None,
            ##parallel_express=0,
            ##parallel_window_offset=0,
            ##parallel_window_start=0,
            ##parallel_window_stop=-1,
            ##parallel_time_start=0,
            ##parallel_time_stop=-1,
            ##parallel_prune_n_electrons=1e-10, 
            ##parallel_prune_frequency=20,
            ### Serial  
            ##serial_ccd=None,
            ##serial_roe=None,
            ##serial_traps=None,
            ##serial_express=0,
            ##serial_window_offset=0,
            ##serial_window_start=0,
            ##serial_window_stop=-1,
            ##serial_time_start=0,
            ##serial_time_stop=-1,
            ##serial_prune_n_electrons=1e-10, 
            ##serial_prune_frequency=20,
            ### Pixel bounce
            ##pixel_bounce=None,
            ### Output
            ##verbosity=1,
    ):
        if background_sigma==None:
            background_sigma = np.sqrt(background_level) #if sky sigma is not set, assume sqrt(N)

        if 'parallel_roe' in kwargs:
            parallel_roe = kwargs['parallel_roe']
        else:
            parallel_roe = None
            
        if 'parallel_ccd' in kwargs:
            parallel_ccd = kwargs['parallel_ccd']
        else:
            parallel_ccd = None
            
        if 'parallel_traps' in kwargs:
            parallel_traps = kwargs['parallel_traps']
        else:
            parallel_traps = None
            
        if 'parallel_express' in kwargs:
            parallel_express = kwargs['parallel_express']
        else:
            parallel_express = 0

        if 'serial_roe' in kwargs:
            serial_roe = kwargs['serial_roe']
        else:
            serial_roe = None
            
        if 'serial_ccd' in kwargs:
            serial_ccd = kwargs['serial_ccd']
        else:
            serial_ccd = None
            
        if 'serial_traps' in kwargs:
            serial_traps = kwargs['serial_traps']
        else:
            serial_traps = None
            
        if 'serial_express' in kwargs:
            serial_express = kwargs['serial_express']
        else:
            serial_express = 0        
            
            
        #create sky frame    
        skyFrame = np.random.normal(background_level, background_sigma, (chip_size,chip_size))        
        #add CTI effects
        ctiTrailedFrame = ac.add_cti(skyFrame,parallel_roe=parallel_roe,parallel_ccd=parallel_ccd,parallel_traps=parallel_traps,parallel_express=parallel_express,serial_roe=serial_roe,serial_ccd=serial_ccd,serial_traps=serial_traps,serial_express=serial_express)
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
        ctiCorrectedFrame = ac.remove_cti(sFrame,1,parallel_roe=parallel_roe,parallel_ccd=parallel_ccd,parallel_traps=parallel_traps,parallel_express=parallel_express,serial_roe=serial_roe,serial_ccd=serial_ccd,serial_traps=serial_traps,serial_express=serial_express)
        #re-add R frame to correction
        outputFrame = ctiCorrectedFrame + rFrame

        #determine covariance FOMs
        fomSR = self.covariance_matrix_from_image(outputFrame,matrix_size=matrix_size)
        fomBenchmark = self.covariance_matrix_from_image(skyFrame+noiseFrame,matrix_size=matrix_size)
        fomDiff = fomSR - fomBenchmark

        return fomDiff
    ###############
    ###############
    def _fitter_function(self,x,intercept,slope):
        y = intercept + slope * x
        return y
    ###############
    ###############
    def optimise_parameters(
        self,
        background_level,
        **kwargs    
        #chip_size=500,
        #matrix_size=5,
        ##figure_of_merit=figure_of_merit(), # should probably pass a function here
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
        matrix_size = kwargs['matrix_size']
        optVals = {}
        
        if 'parallel_traps' in kwargs:
            if kwargs['parallel_traps'] is not None:
                kwargs_parallel = kwargs.copy()
                kwargs_parallel['serial_traps']=None #make a parallel-only quadrant
                #things to compare over iterations
                result_array = np.array([])
                matrix_array = np.zeros((11,matrix_size,matrix_size))
                sr_frac = np.arange(0,1.01,0.1)
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
                
                optVals['parallelQuadMatrix'] = mean_grid
                optVals['parallelQuadFrac'] = xint
        
        if 'serial_traps' in kwargs:
            if kwargs['serial_traps'] is not None:
                kwargs_serial = kwargs.copy()
                kwargs_serial['parallel_traps']=None #make a seriial-only quadrant
                #things to compare over iterations
                result_array = np.array([])
                matrix_array = np.zeros((11,matrix_size,matrix_size))
                sr_frac = np.arange(0,1.01,0.1)
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
                #things to compare over iterations
                result_array = np.array([])
                matrix_array = np.zeros((11,matrix_size,matrix_size))
                sr_frac = np.arange(0,1.01,0.1)
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
               
        if fom_method == 'box':
            result_sum = np.sum(covariance_matrix[ystart:yend,xstart:xend])         #sum over central box
        if fom_method == 'row':
            result_sum = np.sum(covariance_matrix[ycenter,xstart:xend])  #sum over central row (for serial CTI)
        if fom_method == 'column':
            result_sum = np.sum(covariance_matrix[ystart:yend,xcenter])  #sum over central column (for parallel CTI)
        #raise NotImplementedError
        return result_sum

    ###############
    ###############
    def covariance_matrix_from_image(self, image, matrix_size=5):
        
        covariance_matrix = np.zeros((matrix_size,matrix_size))
        #calcluate mean stats on image
        x = image.flatten()
        xbar = np.mean(x)

        #roll image to get cross correlation
        matRange= matrix_size//2
        for i in range(-matRange,matRange+1,1):
            for j in range(-matRange,matRange+1,1):
                y = np.roll(image,(-j,-i),axis=(0,1)).flatten()
                ybar = np.mean(y)

                #calculate covariance
                covar = np.sum((x-xbar)*(y-ybar))/(np.std(x-xbar)*np.std(y-ybar))/(x.size)

                #populate matrix cells
                covariance_matrix[i+matRange,j+matRange] = covar
        covariance_matrix[matRange,matRange] = 0
                
        return covariance_matrix


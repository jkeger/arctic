import numpy as np
from typing import List, Optional

try:
    from arcticpy import wrapper as w
except ImportError:
    import wrapper as w

from arcticpy.ccd import CCDPhase, CCD
from arcticpy.roe import ROE
from arcticpy.traps import (
    TrapInstantCapture,
    TrapSlowCapture,
    TrapInstantCaptureContinuum,
    TrapSlowCaptureContinuum,
)
from arcticpy.pixel_bounce import PixelBounce, add_pixel_bounce
from arcticpy.vv_test import VVTestBench
from arcticpy.read_noise import ReadNoise


class ndarray_plus(np.ndarray):
    """
    A class that looks and feels like a numpy.ndarry, but contains extra information.
    The image (a 2D floating point array) will be returned as one of these, because
    it can also include e.g. a Verification and Validation test that CTI correction
    has reduced trailing, or an estimate of the covariance between adjacent pixels
    that was induced by the correction.
    """
    def __new__(
        cls, 
        values: np.ndarray,
        covariance: np.ndarray=None,
        vv_test: bool=None,
        *args,
        **kwargs
    ):
        obj = values.view(cls)
        if vv_test is None:
            if hasattr(obj, "vv_test"):
                vv_test = obj.vv_test
            else: vv_test = VVTestBench()
        obj.vv_test = vv_test
        if covariance is None:
            covariance = np.zeros((2,2,5,5))
            #covariance[:,:,2,2]=1.
        obj.covariance = covariance
        return obj
        
    def __array_finalize__(self, obj):
        if hasattr(obj, "covariance"):
            self.covariance = obj.covariance
    
        if hasattr(obj, "vv_test"):
            self.vv_test = obj.vv_test
        else:
            self.vv_test = None
  
def add_cti(
    image,
    header=None,
    # Parallel
    parallel_ccd=None,
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
    # Combined
    allow_negative_pixels=1,
    # Pixel bounce
    pixel_bounce_list : Optional[List[PixelBounce]] = None,
    # Output
    vv_test=False,
    verbosity=1,
    iteration=0,
):
    """
    Wrapper for arctic's add_cti() in src/cti.cpp, see its documentation.

    Add CTI trails to an image by trapping, releasing, and moving electrons
    along their independent columns, for parallel and/or serial clocking.

    This wrapper extracts individual numbers and arrays from the user-input
    objects to pass to the C++ via Cython. See cy_add_cti() in wrapper.pyx and
    add_cti() in interface.cpp.

    Parameters (where different to add_cti() in src/cti.cpp)
    ----------
    parallel_traps : [Trap]
    serial_traps : [Trap]
        The 1D arrays of all trap species objects, for parallel and serial
        clocking. The core arctic's add_cti() requires the different types of
        traps to be provided in separate arrays. Here, mutliple trap types can
        be passed in a single array, which will be separated by the wrapper.

    verbosity : int (opt.)
        The verbosity parameter to control the amount of printed information:
            0   No printing (except errors etc).
            1   Standard.
            2   Extra details. For some reason, this makes things go VERY slow.

    vv_test : Bool
        If True, run a "Verification & Validation" test by fitting exponential
        curves to pixels in overscan regions (if available). The results can 
        be accessed as image.vv_test.results[-1].parallel.best_fit_trap_density
    
    Inputs
    ------
    image : 2D numpy.ndarray of pixel values
    
    Outputs
    -------
    image : 
        is not just a numpy.ndarray, but has additional properties vv_test and
        covariance
    """
    # ========
    # Extract inputs and/or set dummy variables to pass to the wrapper
    # ========
    # Parallel
    if parallel_traps is not None:
        (
            parallel_trap_densities,
            parallel_trap_release_timescales,
            parallel_trap_third_params,
            parallel_trap_fourth_params,
            parallel_n_traps_ic,
            parallel_n_traps_sc,
            parallel_n_traps_ic_co,
            parallel_n_traps_sc_co,
        ) = _extract_trap_parameters(parallel_traps)
    else:
        # No parallel clocking, set dummy variables instead
        (
            parallel_roe,
            parallel_ccd,
            parallel_trap_densities,
            parallel_trap_release_timescales,
            parallel_trap_third_params,
            parallel_trap_fourth_params,
            parallel_n_traps_ic,
            parallel_n_traps_sc,
            parallel_n_traps_ic_co,
            parallel_n_traps_sc_co,
        ) = _set_dummy_parameters()
    parallel_prune_n_es = np.array([parallel_prune_n_electrons], dtype=np.double)

    # Serial
    if serial_traps is not None:
        (
            serial_trap_densities,
            serial_trap_release_timescales,
            serial_trap_third_params,
            serial_trap_fourth_params,
            serial_n_traps_ic,
            serial_n_traps_sc,
            serial_n_traps_ic_co,
            serial_n_traps_sc_co,
        ) = _extract_trap_parameters(serial_traps)
    else:
        # No serial clocking, set dummy variables instead
        (
            serial_roe,
            serial_ccd,
            serial_trap_densities,
            serial_trap_release_timescales,
            serial_trap_third_params,
            serial_trap_fourth_params,
            serial_n_traps_ic,
            serial_n_traps_sc,
            serial_n_traps_ic_co,
            serial_n_traps_sc_co,
        ) = _set_dummy_parameters()
    serial_prune_n_es = np.array([serial_prune_n_electrons], dtype=np.double)
    
    image = np.copy(image).astype(np.double)


    # ========
    # V&V test
    # ========
    # Measure level of trailing into overscan regions of input image
    vv=VVTestBench(
        parallel_roe=parallel_roe, 
        parallel_ccd=parallel_ccd, 
        parallel_traps=parallel_traps, 
        serial_roe=serial_roe, 
        serial_ccd=serial_ccd, 
        serial_traps=serial_traps, 
        sum_of_exponentials=True,
        verbose=(verbosity >= 1)
    )
    if vv_test:
        vv_test_before=vv.test(image)

    # ========
    # Add CTI
    # ========
    # Pass the extracted inputs to C++ via the cython wrapper
    image_trailed = w.cy_add_cti(
        image,
        # ========
        # Parallel
        # ========
        # ROE
        parallel_roe.dwell_times,
        parallel_roe.prescan_offset,
        parallel_roe.overscan_start,
        parallel_roe.empty_traps_between_columns,
        parallel_roe.empty_traps_for_first_transfers,
        parallel_roe.force_release_away_from_readout,
        parallel_roe.use_integer_express_matrix,
        parallel_roe.n_pumps,
        parallel_roe.type,
        # CCD
        parallel_ccd.fraction_of_traps_per_phase,
        parallel_ccd.full_well_depths,
        parallel_ccd.well_notch_depths,
        parallel_ccd.well_fill_powers,
        parallel_ccd.first_electron_fills,
        # Traps
        parallel_trap_densities,
        parallel_trap_release_timescales,
        parallel_trap_third_params,
        parallel_trap_fourth_params,
        parallel_n_traps_ic,
        parallel_n_traps_sc,
        parallel_n_traps_ic_co,
        parallel_n_traps_sc_co,
        # Misc
        parallel_express,
        parallel_window_offset,
        parallel_window_start,
        parallel_window_stop,
        parallel_time_start,
        parallel_time_stop,
        parallel_prune_n_es,
        parallel_prune_frequency,
        # ========
        # Serial
        # ========
        # ROE
        serial_roe.dwell_times,
        serial_roe.prescan_offset,
        serial_roe.overscan_start,
        serial_roe.empty_traps_between_columns,
        serial_roe.empty_traps_for_first_transfers,
        serial_roe.force_release_away_from_readout,
        serial_roe.use_integer_express_matrix,
        serial_roe.n_pumps,
        serial_roe.type,
        # CCD
        serial_ccd.fraction_of_traps_per_phase,
        serial_ccd.full_well_depths,
        serial_ccd.well_notch_depths,
        serial_ccd.well_fill_powers,
        serial_ccd.first_electron_fills,
        # Traps
        serial_trap_densities,
        serial_trap_release_timescales,
        serial_trap_third_params,
        serial_trap_fourth_params,
        serial_n_traps_ic,
        serial_n_traps_sc,
        serial_n_traps_ic_co,
        serial_n_traps_sc_co,
        # Misc
        serial_express,
        serial_window_offset,
        serial_window_start,
        serial_window_stop,
        serial_time_start,
        serial_time_stop,
        serial_prune_n_es,
        serial_prune_frequency,
        # ========
        # Combined
        # ========
        allow_negative_pixels,
        # ========
        # Output
        # ========
        verbosity,
        iteration,
    )

    # ================
    # Add pixel bounce
    # ================
    if pixel_bounce_list is not None:

        bias_total = np.zeros(image.shape)
        
        for pixel_bounce in pixel_bounce_list:
            bias = pixel_bounce.bias_from(
                image_trailed,
                parallel_window_start=parallel_window_start,
                parallel_window_stop=parallel_window_stop,
                serial_window_start=serial_window_start,
                serial_window_stop=serial_window_stop,
                verbosity=verbosity,
            )
            bias_total += bias

        image_trailed -= bias_total[:, :]

    # ========
    # V&V test
    # ========
    # Re-measure level of trailing into overscan regions of output image
    if vv_test:
        vv_test_after=vv.test(image_trailed,
                              parallel_valid_columns = vv_test_before.parallel.valid_columns,
                              parallel_pixels_pre_cti = vv_test_before.parallel.pixels_pre_cti,
                              parallel_fit_bias = True, 
                              parallel_model_bias = vv_test_before.parallel.best_fit_bias)

    # ===================
    # Update image header
    # ===================
    if header is not None:
        header.set(
            "cticor",
            "ArCTIc",
            "CTI addition performed using ArCTIc v" + w.cy_version_arctic(),
        )
        header.set(
            "ctipar",
            "ArCTIc",
            "CTI addition performed using ArCTIc v" + w.cy_version_arctic(),
        )

    return ndarray_plus(image_trailed, vv_test = vv)


def remove_cti(
    image,
    n_iterations,
    header=None,
    # Parallel
    parallel_ccd=None,
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
    # Combined
    allow_negative_pixels=1,
    # Pixel bounce
    pixel_bounce_list : Optional[List[PixelBounce]] = None,
    # Optional: read noise de-amplification
    remove_read_noise=False,
    # Optional: perform Validation & Verification test
    vv_test=False,
    # Output
    verbosity=1,
):
    """
    Wrapper for arctic's remove_cti() in src/cti.cpp, see its documentation.

    Remove CTI trails from an image by first modelling the addition of CTI, for
    parallel and/or serial clocking, using the add_cti() wrapper.

    Parameters (where different to remove_cti() in src/cti.cpp)
    ----------
    parallel_traps : [Trap]
    serial_traps : [Trap]
        The 1D arrays of all trap species objects, for parallel and serial
        clocking. The core arctic's add_cti() requires the different types of
        traps to be provided in separate arrays. Here, mutliple trap types can
        be passed in a single array, which will be separated by the wrapper.

    verbosity : int (opt.)
        The verbosity parameter to control the amount of printed information:
            0   No printing (except errors etc).
            1   Standard.
            2   Extra details. For some reason, this makes things go VERY slow.

    vv_test : Bool
        If True, run a "Verification & Validation" test by fitting exponential
        curves to pixels in overscan regions (if available). The results can 
        be accessed as image.vv_test.results[-1].parallel.best_fit_trap_density
    
    remove_read_noise : Bool
        If True, estimate and remove the white readout noise in the image
        before doing CTI correction, to prevent its being amplified. 
        Add the noise back afterwards.
        
    Inputs
    ------
    image : 2D numpy.ndarray of pixel values
    
    Outputs
    -------
    image : 
        is not just a numpy.ndarray, but has additional properties vv_test and
        covariance
    """
    image = np.copy(image).astype(np.double)
    image_remove_cti = np.copy(image).astype(np.double)

    if verbosity >= 1:
        w.cy_print_version()

    # ========
    # V&V test
    # ========
    # Measure level of trailing into overscan regions of input image
    covariance = None
    vv=VVTestBench(
        parallel_roe=parallel_roe, 
        parallel_ccd=parallel_ccd, 
        parallel_traps=parallel_traps, 
        serial_roe=serial_roe, 
        serial_ccd=serial_ccd, 
        serial_traps=serial_traps, 
        sum_of_exponentials=True,
        verbose=(verbosity >= 1)
    )
    if vv_test:
       vv_test_before=vv.test(image)
    
    # =======================
    # Attempt to estimate and remove read noise, so it it not amplified
    # =======================
    if remove_read_noise > 0:
        sigma_readnoise = 1. * remove_read_noise
        read_noise = ReadNoise(sigma_readnoise=sigma_readnoise)
        print(read_noise.sigmaRN)
        image_remove_cti,image_read_noise = read_noise.generate_SR_frames_from_image(image_remove_cti)
        print("\nMean and rms of modelled read noise:",np.mean(image_read_noise),np.std(image_read_noise))        

    # =======================
    # Estimate the image with removed CTI more accurately each iteration
    # =======================
    for iteration in range(1, n_iterations + 1):
        if verbosity >= 1:
            print("Iter %d: " % iteration, end="", flush=True)

        # Model the effect of adding CTI trails
        image_add_cti = add_cti(
            image=image_remove_cti,
            header=header,
            # Parallel
            parallel_ccd=parallel_ccd,
            parallel_roe=parallel_roe,
            parallel_traps=parallel_traps,
            parallel_express=parallel_express,
            parallel_window_offset=parallel_window_offset,
            parallel_window_start=parallel_window_start,
            parallel_window_stop=parallel_window_stop,
            parallel_time_start=parallel_time_start,
            parallel_time_stop=parallel_time_stop,
            parallel_prune_n_electrons=parallel_prune_n_electrons,
            parallel_prune_frequency=parallel_prune_frequency,
            # Serial
            serial_ccd=serial_ccd,
            serial_roe=serial_roe,
            serial_traps=serial_traps,
            serial_express=serial_express,
            serial_window_offset=serial_window_offset,
            serial_window_start=serial_window_start,
            serial_window_stop=serial_window_stop,
            serial_time_start=serial_time_start,
            serial_time_stop=serial_time_stop,
            serial_prune_n_electrons=serial_prune_n_electrons,
            serial_prune_frequency=serial_prune_frequency,
            # Combined
            allow_negative_pixels=allow_negative_pixels,
            # Pixel bounce
            pixel_bounce_list=pixel_bounce_list,
            # Output
            verbosity=verbosity,
            vv_test=False,
            iteration=iteration
        )

        # Improve the estimate of the image with CTI trails removed
        delta = image - image_add_cti
        if remove_read_noise > 0:
            delta -= image_read_noise
            # Doing the following ought to be right, but turns out to bias the
            # mean of the output image
            #delta_squared = delta ** 2
            #delta *= delta_squared / ( delta_squared + read_noise.sigmaRN ** 2 )
        image_remove_cti += delta
        
        # Prevent unphysical, negative image values
        # Hack to get long iteractions to converge faster
        # Warning: this can introduce biases in e.g. dark exposures
        if not allow_negative_pixels:
            if iteration == 1:
                image_remove_cti[image_remove_cti < 0.0] = 0.0
            
    # =======================
    # Add back the read noise, if it had been removed
    # =======================
    if remove_read_noise > 0:
        image_remove_cti += image_read_noise
        #
        # TO DO: Estimate residual covariance due to read noise
        #        This will eventually be calculated via a call like
        #
        #        covariance = read_noise.measure_simulated_covariance_corners()
        #
        # But that's not yet finished. For now...
        covariance = np.zeros((2,2,5,5))
        covariance[:,:,2,2]=read_noise.sigmaRN
           
    # ========
    # V&V test
    # ========
    # Re-measure level of trailing into overscan regions of output image
    if vv_test:
        vv_test_after=vv.test(image_remove_cti,
                              parallel_valid_columns = vv_test_before.parallel.valid_columns,
                              parallel_pixels_pre_cti = vv_test_before.parallel.pixels_pre_cti,
                              parallel_fit_bias = True, 
                              parallel_model_bias = vv_test_before.parallel.best_fit_bias)
        
    # ===================
    # Update image header
    # ===================
    if header is not None:
        header.set(
            "cticor",
            "ArCTIc",
            "CTI correction performed using ArCTIc v" + w.cy_version_arctic(),
        )
        header.set(
            "ctipar",
            "ArCTIc",
            "CTI correction performed using ArCTIc v" + w.cy_version_arctic(),
        )
        
    return ndarray_plus(image_remove_cti, vv_test = vv, covariance = covariance)



def CTI_model_for_HST_ACS(date):
    """
    Return arcticpy objects that provide a preset CTI model for the Hubble Space
    Telescope (HST) Advanced Camera for Surveys (ACS).

    The returned objects are ready to be passed to add_cti() or remove_cti(),
    for parallel clocking.

    See Massey et al. (2014). Updated model and references coming soon.

    Parameters
    ----------
    date : float
        The Julian date. Should not be before the ACS launch date.

    Returns
    -------
    roe : ROE
        The ROE object that describes the read-out electronics.

    ccd : CCD
        The CCD object that describes how electrons fill the volume.

    traps : [Trap]
        A list of trap objects that set the parameters for each trap species.
    """
    # Julian dates
    date_acs_launch = 2452334.5  # ACS launched, SM3B, 01 March 2002
    date_T_change = 2453920.0  # Temperature changed, 03 July 2006
    date_sm4_repair = 2454968.0  # ACS repaired, SM4, 16 May 2009

    assert date >= date_acs_launch, "Date must be after ACS launch (2002/03/01)"

    # Trap species
    relative_densities = np.array([0.17, 0.45, 0.38])
    if date < date_T_change:
        release_times = np.array([0.48, 4.86, 20.6])
    else:
        release_times = np.array([0.74, 7.70, 37.0])

    # Density evolution
    if date < date_sm4_repair:
        initial_total_trap_density = 0.017845
        trap_growth_rate = 3.5488e-4
    else:
        initial_total_trap_density = -0.246591 * 1.011
        trap_growth_rate = 0.000558980 * 1.011
    total_trap_density = initial_total_trap_density + trap_growth_rate * (
        date - date_acs_launch
    )
    trap_densities = relative_densities * total_trap_density

    # arctic objects
    # There is CTI only in the parallel direction, so don't worry about e.g. serial prescan
    parallel_roe = ROE(
        dwell_times=[1.0],
        empty_traps_between_columns=True,
        empty_traps_for_first_transfers=False,
        force_release_away_from_readout=True,
        use_integer_express_matrix=False,
        overscan_start=2048
    )
    serial_roe = ROE(
        dwell_times=[1.0],
        empty_traps_between_columns=True,
        empty_traps_for_first_transfers=False,
        force_release_away_from_readout=True,
        use_integer_express_matrix=False,
        prescan_length=24,
        read_noise=4.0
    )

    # Single-phase CCD
    parallel_ccd = CCD(full_well_depth=84700, well_notch_depth=0.0, well_fill_power=0.478)
    serial_ccd = CCD(full_well_depth=84700, well_notch_depth=0.0, well_fill_power=0.478)

    # Instant-capture traps
    parallel_traps = [
        TrapInstantCapture(
            density=trap_densities[i], release_timescale=release_times[i]
        )
        for i in range(len(trap_densities))
    ]
    serial_traps = None

    return parallel_roe, parallel_ccd, parallel_traps, serial_roe, serial_ccd, serial_traps

################################################ INTERNAL FUNCTIONS

def _extract_trap_parameters(traps):
    """Extract trap parameters for add/remove_cti() to pass to the wrapper.

    Returns the converted arguments in the formats and types required by the
    cython wrapper's cy_add/remove_cti().
    """
    # Extract trap inputs
    traps_ic = [trap for trap in traps if type(trap) == TrapInstantCapture]
    traps_sc = [trap for trap in traps if type(trap) == TrapSlowCapture]
    traps_ic_co = [trap for trap in traps if type(trap) == TrapInstantCaptureContinuum]
    traps_sc_co = [trap for trap in traps if type(trap) == TrapSlowCaptureContinuum]
    n_traps_ic = len(traps_ic)
    n_traps_sc = len(traps_sc)
    n_traps_ic_co = len(traps_ic_co)
    n_traps_sc_co = len(traps_sc_co)
    if n_traps_sc + n_traps_ic + n_traps_ic_co + n_traps_sc_co != len(traps):
        raise Exception(
            "Not all traps extracted successfully (%d instant capture, %d slow capture, %d continuum, %d slow_capture_continuum, %d total)"
            % (n_traps_ic, n_traps_sc, n_traps_ic_co, n_traps_sc_co, len(traps))
        )

    # Make sure the order is correct
    traps = traps_ic + traps_sc + traps_ic_co + traps_sc_co
    trap_densities = np.array([trap.density for trap in traps], dtype=np.double)
    trap_release_timescales = np.array(
        [trap.release_timescale for trap in traps], dtype=np.double
    )
    # Third parameter for some trap types
    trap_third_params = []
    for trap in traps:
        if type(trap) == TrapInstantCapture:
            trap_third_params.append(trap.fractional_volume_none_exposed)
        elif type(trap) == TrapSlowCapture:
            trap_third_params.append(trap.capture_timescale)
        elif type(trap) == TrapInstantCaptureContinuum:
            trap_third_params.append(trap.release_timescale_sigma)
        elif type(trap) == TrapSlowCaptureContinuum:
            trap_third_params.append(trap.release_timescale_sigma)
    trap_third_params = np.array(trap_third_params, dtype=np.double)
    # Fourth parameter for some trap types
    trap_fourth_params = []
    for trap in traps:
        if type(trap) == TrapInstantCapture:
            trap_fourth_params.append(trap.fractional_volume_full_exposed)
        elif type(trap) == TrapSlowCapture:
            trap_fourth_params.append(0.0)
        elif type(trap) == TrapInstantCaptureContinuum:
            trap_fourth_params.append(0.0)
        elif type(trap) == TrapSlowCaptureContinuum:
            trap_fourth_params.append(trap.capture_timescale)
    trap_fourth_params = np.array(trap_fourth_params, dtype=np.double)

    return (
        trap_densities,
        trap_release_timescales,
        trap_third_params,
        trap_fourth_params,
        n_traps_ic,
        n_traps_sc,
        n_traps_ic_co,
        n_traps_sc_co,
    )


def _set_dummy_parameters():
    """Set dummy variables for add/remove_cti() to pass to the wrapper.

    Returns placeholder arguments in the formats and types required by the
    cython wrapper's cy_add/remove_cti() for when one of parallel or serial
    clocking is not being used.
    """
    roe = ROE()
    ccd = CCD([CCDPhase(0.0, 0.0, 0.0, 0.0)], [0.0])
    trap_densities = np.array([0.0], dtype=np.double)
    trap_release_timescales = np.array([0.0], dtype=np.double)
    trap_third_params = np.array([0.0], dtype=np.double)
    trap_fourth_params = np.array([0.0], dtype=np.double)
    n_traps_ic = 0
    n_traps_sc = 0
    n_traps_ic_co = 0
    n_traps_sc_co = 0

    return (
        roe,
        ccd,
        trap_densities,
        trap_release_timescales,
        trap_third_params,
        trap_fourth_params,
        n_traps_ic,
        n_traps_sc,
        n_traps_ic_co,
        n_traps_sc_co,
    )


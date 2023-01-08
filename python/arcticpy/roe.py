""" 
The properties of readout electronics (ROE) used to operate a CCD.
(Works equally well for clocking electrons in an n-type CCD or holes in a p-type 
CCD.)
Three different clocking modes are available:
1) Standard readout, in which photoelectrons are created during an exposure, and
   read out (through different numbers of intervening pixels) to the readout
   electronics.
2) Charge Injection Line, in which electrons are injected at the far end of the
   CCD, by a charge injection structure. All travel the same distance to the
   readout electronics.
3) Trap pumping, in which electrons are shuffled backwards and forwards many 
   times, but end up in the same place as they began.
   
By default, or if the dwell_times variable has only one element, the pixel-to-
pixel transfers are assumed to happen instantly, in one step. This recovers the 
behaviour of earlier versions of ArCTIC (written in java, IDL, or C++). If 
instead a list of n dwell_times is provided, it is assumed that each pixel 
contains n phases in which electrons are stored during intermediate steps of the 
readout sequence. The number of phases should match that in the instance of a 
CCD class, and the units of dwell_times should match those in the instance of a 
Traps class.
Unlike CCD pixels, where row 1 is closer to the readout register than row 2, 
phase 2 is closer than phase 1:
+-----------
| Pixel n:  
|   Phase 0 
|   Phase 1 
|   Phase 2 
+-----------
|    .       
     .
|    .       
+-----------
| Pixel 1:  
|   Phase 0 
|   Phase 1 
|   Phase 2 
+-----------
| Pixel 0:  
|   Phase 0 
|   Phase 1 
|   Phase 2 
+-----------
| Readout   
+----------- 
"""
import numpy as np

#from .dictable import Dictable

roe_type_standard = 0
roe_type_charge_injection = 1
roe_type_trap_pumping = 2


class ROE:
    def __init__(
        self,
        dwell_times=[1.0],
        prescan_offset=0,
        overscan_start=-1,
        empty_traps_between_columns=True,
        empty_traps_for_first_transfers=False,
        force_release_away_from_readout=True,
        use_integer_express_matrix=False,
        pixel_bounce_kA=0.,
        pixel_bounce_kv=0.,
        pixel_bounce_omega=1.,
        pixel_bounce_gamma=1.
    ):
        self.dwell_times = np.array(dwell_times, dtype=np.double)
        self.prescan_offset = prescan_offset
        self.overscan_start = overscan_start
        self.empty_traps_between_columns = empty_traps_between_columns
        self.empty_traps_for_first_transfers = empty_traps_for_first_transfers
        self.force_release_away_from_readout = force_release_away_from_readout
        self.use_integer_express_matrix = use_integer_express_matrix
        self.pixel_bounce_kA    = pixel_bounce_kA
        self.pixel_bounce_kv    = pixel_bounce_kv
        self.pixel_bounce_omega = pixel_bounce_omega
        self.pixel_bounce_gamma = pixel_bounce_gamma
        self.n_pumps = -1  # Dummy value

        self.type = roe_type_standard
  


class ROEChargeInjection(ROE):
    def __init__(
        self,
        dwell_times=[1.0],
        prescan_offset=0,
        overscan_start=-1,
        empty_traps_between_columns=True,
        force_release_away_from_readout=True,
        use_integer_express_matrix=False,
        pixel_bounce_kA=0.,
        pixel_bounce_kv=0.,
        pixel_bounce_omega=1.,
        pixel_bounce_gamma=1.
        
    ):
        ROE.__init__(
            self,
            dwell_times=dwell_times,
            prescan_offset=prescan_offset,
            overscan_start=overscan_start,
            empty_traps_between_columns=empty_traps_between_columns,
            empty_traps_for_first_transfers=False,
            force_release_away_from_readout=force_release_away_from_readout,
            use_integer_express_matrix=use_integer_express_matrix,
            pixel_bounce_kA=pixel_bounce_kA,
            pixel_bounce_kv=pixel_bounce_kv,
            pixel_bounce_omega=pixel_bounce_omega,
            pixel_bounce_gamma=pixel_bounce_gamma
        )

        self.type = roe_type_charge_injection
    
    def from_normal_roe(roe, n_pixels_in_image):
        """
        Convert a normal ROE sequence, with all its options, to one modelling
        charge injection readout.
        Must specify the number of physical pixels between the charge injection
        register and the readout node.
        """
        return ROEChargeInjection(
            dwell_times=roe.dwell_times,
            prescan_offset=roe.prescan_offset + n_pixels_in_image,
            overscan_start=roe.overscan_start,
            empty_traps_between_columns=roe.empty_traps_between_columns,
            force_release_away_from_readout=roe.force_release_away_from_readout,
            use_integer_express_matrix=roe.use_integer_express_matrix,
            pixel_bounce_kA=roe.pixel_bounce_kA,
            pixel_bounce_kv=roe.pixel_bounce_kv,
            pixel_bounce_omega=roe.pixel_bounce_omega,
            pixel_bounce_gamma=roe.pixel_bounce_gamma
        )


class ROETrapPumping(ROE):
    def __init__(
        self,
        dwell_times=[0.5, 0.5],
        prescan_offset=0,
        overscan_start=-1,
        n_pumps=1,
        empty_traps_for_first_transfers=False,
        use_integer_express_matrix=False,
        pixel_bounce_kA=0.,
        pixel_bounce_kv=0.,
        pixel_bounce_omega=1.,
        pixel_bounce_gamma=1.
     ):
        ROE.__init__(
            self,
            dwell_times=dwell_times,
            prescan_offset=prescan_offset,
            overscan_start=overscan_start,
            empty_traps_between_columns=True,
            empty_traps_for_first_transfers=empty_traps_for_first_transfers,
            force_release_away_from_readout=False,
            use_integer_express_matrix=use_integer_express_matrix,
            pixel_bounce_kA=pixel_bounce_kA,
            pixel_bounce_kv=pixel_bounce_kv,
            pixel_bounce_omega=pixel_bounce_omega,
            pixel_bounce_gamma=pixel_bounce_gamma
        )
        self.n_pumps = n_pumps

        self.type = roe_type_trap_pumping

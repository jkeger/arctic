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

import numpy as np


class ROE(object):
    def __init__(
        self,
        dwell_times=[1],
        empty_traps_between_columns=True,
        empty_traps_for_first_transfers=True,
        force_release_away_from_readout=True,
        use_integer_express_matrix=False,
    ):
        self.dwell_times = np.array(dwell_times)
        self.empty_traps_between_columns = empty_traps_between_columns
        self.empty_traps_for_first_transfers = empty_traps_for_first_transfers
        self.force_release_away_from_readout = force_release_away_from_readout
        self.use_integer_express_matrix = use_integer_express_matrix


class CCDPhase(object):
    def __init__(
        self,
        full_well_depth,
        well_notch_depth,
        well_fill_power,
    ):
        self.full_well_depth = full_well_depth
        self.well_notch_depth = well_notch_depth
        self.well_fill_power = well_fill_power


class CCD(object):
    def __init__(
        self,
        phases,
        fraction_of_traps_per_phase,
    ):
        self.phases = phases
        self.fraction_of_traps_per_phase = np.array(fraction_of_traps_per_phase)

        # Extract convenient arrays of the parameters for each phases
        self.full_well_depths = np.array(
            [phase.full_well_depth for phase in self.phases]
        )
        self.well_notch_depths = np.array(
            [phase.well_notch_depth for phase in self.phases]
        )
        self.well_fill_powers = np.array(
            [phase.well_fill_power for phase in self.phases]
        )

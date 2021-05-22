import numpy as np


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

        # Extract convenient arrays
        self.full_well_depths = np.array(
            [phase.full_well_depth for phase in self.phases]
        )
        self.well_notch_depths = np.array(
            [phase.well_notch_depth for phase in self.phases]
        )
        self.well_fill_powers = np.array(
            [phase.well_fill_power for phase in self.phases]
        )

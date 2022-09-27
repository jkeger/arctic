import numpy as np

from autoconf.dictable import Dictable

class CCDPhase(Dictable):
    def __init__(self, full_well_depth=1e4, well_notch_depth=0.0, well_fill_power=1.0):
        self.full_well_depth = full_well_depth
        self.well_notch_depth = well_notch_depth
        self.well_fill_power = well_fill_power


class CCD(object):
    def __init__(
        self,
        phases=[CCDPhase()],
        fraction_of_traps_per_phase=[1.0],
        full_well_depth=None,
        well_notch_depth=None,
        well_fill_power=None,
    ):
        """For convenience, the CCDPhase parameters can be passed directly to
        this CCD object to override self.phases with an automatic single phase
        with those parameters.
        """
        if full_well_depth is not None:
            if well_notch_depth is None:
                well_notch_depth = 0.0
            if well_fill_power is None:
                well_fill_power = 1.0

            self.phases = [
                CCDPhase(
                    full_well_depth=full_well_depth,
                    well_notch_depth=well_notch_depth,
                    well_fill_power=well_fill_power,
                )
            ]
        else:
            self.phases = phases
        self.fraction_of_traps_per_phase = np.array(
            fraction_of_traps_per_phase, dtype=np.double
        )

        # Extract convenient arrays
        self.full_well_depths = np.array(
            [phase.full_well_depth for phase in self.phases], dtype=np.double
        )
        self.well_notch_depths = np.array(
            [phase.well_notch_depth for phase in self.phases], dtype=np.double
        )
        self.well_fill_powers = np.array(
            [phase.well_fill_power for phase in self.phases], dtype=np.double
        )

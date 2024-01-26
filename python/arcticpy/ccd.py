import numpy as np


class CCDPhase:
    def __init__(
        self,
        full_well_depth=1e4,
        well_notch_depth=0.0,
        well_fill_power=1.0,
        first_electron_fill=0.0,
    ):
        self.full_well_depth = full_well_depth
        self.well_notch_depth = well_notch_depth
        self.well_fill_power = well_fill_power
        self.first_electron_fill = first_electron_fill

    def volume(self, n_e):
        # Fraction of a pixel volume that is occupied by n_e electrons
        # Mirrors the ("volume-driven") model used by ArCTIc in ccd.cpp
        # Provided here for convenience
        well_range = self.full_well_depth - self.well_notch_depth
        volume = ( self.first_electron_fill 
            + (1 - self.first_electron_fill) 
            * np.clip((n_e - self.well_notch_depth) / well_range ,
            0, 1) ** self.well_fill_power
        )
        return volume  


class CCD(object):
    def __init__(
        self,
        phases=[CCDPhase()],
        fraction_of_traps_per_phase=[1.0],
        full_well_depth=None,
        well_notch_depth=None,
        well_fill_power=None,
        first_electron_fill=None,
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
            if first_electron_fill is None:
                first_electron_fill = 0.0

            self.phases = [
                CCDPhase(
                    full_well_depth=full_well_depth,
                    well_notch_depth=well_notch_depth,
                    well_fill_power=well_fill_power,
                    first_electron_fill=first_electron_fill,
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
        self.first_electron_fills = np.array(
            [phase.first_electron_fill for phase in self.phases], dtype=np.double
        )

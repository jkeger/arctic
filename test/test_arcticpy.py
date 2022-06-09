import os
import sys
from urllib.request import urlretrieve

path = os.path.dirname(os.path.realpath(__file__))
sys.path.append(os.path.join(path, ".."))
import arcticpy as cti
import json
import matplotlib.pyplot as plt
import numpy as np
import copy
import pytest

from os import path

class TestCompareOldArCTIC:
    def test__add_cti__single_pixel__vary_express__compare_old_arctic(self):

        # Manually set True to make the plot
        do_plot = False
        # do_plot = True

        image_pre_cti = np.zeros((20, 1))
        image_pre_cti[2, 0] = 800

        # Nice numbers for easy manual checking
        traps = [cti.TrapInstantCapture(density=10, release_timescale=-1 / np.log(0.5))]
        ccd = cti.CCD(
            phases=[
                cti.CCDPhase(
                    well_fill_power=1, full_well_depth=1000, well_notch_depth=0
                )
            ]
        )
        roe = cti.ROE(
            empty_traps_between_columns=True,
            empty_traps_for_first_transfers=False,
            use_integer_express_matrix=True,
        )

        if do_plot:
            pixels = np.arange(len(image_pre_cti))
            colours = ["#1199ff", "#ee4400", "#7711dd", "#44dd44", "#775533"]
            plt.figure(figsize=(10, 6))
            ax1 = plt.gca()
            ax2 = ax1.twinx()

        for i, (express, image_idl) in enumerate(
            zip(
                [1, 2, 5, 10, 20],
                [
                    [
                        0.00000,
                        0.00000,
                        776.000,
                        15.3718,
                        9.65316,
                        5.81950,
                        3.41087,
                        1.95889,
                        1.10817,
                        0.619169,
                        0.342489,
                        0.187879,
                        0.102351,
                        0.0554257,
                        0.0298603,
                        0.0160170,
                        0.00855758,
                        0.00455620,
                        0.00241824,
                        0.00128579,
                    ],
                    [
                        0.00000,
                        0.00000,
                        776.000,
                        15.3718,
                        9.65316,
                        5.81950,
                        3.41087,
                        1.95889,
                        1.10817,
                        0.619169,
                        0.348832,
                        0.196128,
                        0.109984,
                        0.0614910,
                        0.0340331,
                        0.0187090,
                        0.0102421,
                        0.00558406,
                        0.00303254,
                        0.00164384,
                    ],
                    [
                        0.00000,
                        0.00000,
                        776.000,
                        15.3718,
                        9.59381,
                        5.80216,
                        3.43231,
                        1.99611,
                        1.15104,
                        0.658983,
                        0.374685,
                        0.211807,
                        0.119441,
                        0.0670274,
                        0.0373170,
                        0.0205845,
                        0.0113179,
                        0.00621127,
                        0.00341018,
                        0.00187955,
                    ],
                    [
                        0.00000,
                        0.00000,
                        776.160,
                        15.1432,
                        9.51562,
                        5.78087,
                        3.43630,
                        2.01144,
                        1.16452,
                        0.668743,
                        0.381432,
                        0.216600,
                        0.122556,
                        0.0689036,
                        0.0383241,
                        0.0211914,
                        0.0116758,
                        0.00641045,
                        0.00352960,
                        0.00195050,
                    ],
                    [
                        0.00000,
                        0.00000,
                        776.239,
                        15.0315,
                        9.47714,
                        5.77145,
                        3.43952,
                        2.01754,
                        1.17049,
                        0.673351,
                        0.384773,
                        0.218860,
                        0.124046,
                        0.0697859,
                        0.0388253,
                        0.0214799,
                        0.0118373,
                        0.00650488,
                        0.00358827,
                        0.00198517,
                    ],
                ],
            )
        ):
            image_post_cti = cti.add_cti(
                image=image_pre_cti,
                parallel_roe=roe,
                parallel_ccd=ccd,
                parallel_traps=traps,
                parallel_express=express,
                verbosity=0,
            ).T[0]

            image_idl = np.array(image_idl)

            if do_plot:
                c = colours[i]

                if i == 0:
                    ax1.plot(
                        pixels,
                        image_post_cti,
                        alpha=0.8,
                        c=c,
                        label="%d (py)" % express,
                    )
                    ax1.plot(
                        pixels,
                        image_idl,
                        ls="--",
                        alpha=0.8,
                        c=c,
                        label="%d (idl)" % express,
                    )
                    ax2.plot(
                        pixels,
                        (image_post_cti - image_idl) / image_idl,
                        alpha=0.8,
                        ls=":",
                        c=c,
                    )
                else:
                    ax1.plot(
                        pixels, image_post_cti, alpha=0.8, c=c, label="%d" % express
                    )
                    ax1.plot(pixels, image_idl, alpha=0.8, c=c, ls="--")
                    ax2.plot(
                        pixels,
                        (image_post_cti - image_idl) / image_idl,
                        alpha=0.8,
                        ls=":",
                        c=c,
                    )

            assert image_post_cti == pytest.approx(image_idl, rel=0.05)

        if do_plot:
            ax1.legend(title="express", loc="lower left")
            ax1.set_yscale("log")
            ax1.set_xlabel("Pixel")
            ax1.set_ylabel("Counts")
            ax2.set_ylabel("Fractional Difference (dotted)")
            plt.tight_layout()
            plt.show()

    def test__add_cti__single_pixel__vary_express_2__compare_old_arctic(self):

        # Manually set True to make the plot
        do_plot = False
        # do_plot = True

        image_pre_cti = np.zeros((120, 1))
        image_pre_cti[102, 0] = 800

        traps = [cti.TrapInstantCapture(density=10, release_timescale=-1 / np.log(0.5))]
        ccd = cti.CCD(
            phases=[
                cti.CCDPhase(
                    well_fill_power=0.5, full_well_depth=1000, well_notch_depth=0
                )
            ]
        )
        roe = cti.ROE(
            empty_traps_between_columns=True,
            empty_traps_for_first_transfers=False,
            use_integer_express_matrix=True,
        )

        if do_plot:
            pixels = np.arange(len(image_pre_cti))
            colours = ["#1199ff", "#ee4400", "#7711dd", "#44dd44", "#775533"]
            plt.figure(figsize=(10, 6))
            ax1 = plt.gca()
            ax2 = ax1.twinx()

        for i, (express, image_idl) in enumerate(
            zip(
                [2, 20],
                [
                    [
                        0.00000,
                        0.00000,
                        42.6722,
                        250.952,
                        161.782,
                        107.450,
                        73.0897,
                        50.6914,
                        35.6441,
                        25.3839,
                        18.2665,
                        13.2970,
                        9.79078,
                        7.30555,
                        5.52511,
                        4.24364,
                        3.30829,
                        2.61813,
                        2.09710,
                        1.70752,
                    ],
                    [
                        0.00000,
                        0.00000,
                        134.103,
                        163.783,
                        117.887,
                        85.8632,
                        63.6406,
                        47.9437,
                        36.6625,
                        28.4648,
                        22.4259,
                        17.9131,
                        14.4976,
                        11.8789,
                        9.84568,
                        8.25520,
                        6.98939,
                        5.97310,
                        5.14856,
                        4.47386,
                    ],
                ],
            )
        ):
            image_post_cti = cti.add_cti(
                image=image_pre_cti,
                parallel_traps=traps,
                parallel_ccd=ccd,
                parallel_roe=roe,
                parallel_express=express,
                verbosity=0,
            ).T[0]

            image_idl = np.append(np.zeros(100), image_idl)

            if do_plot:
                c = colours[i]

                if i == 0:
                    ax1.plot(
                        pixels,
                        image_post_cti,
                        alpha=0.8,
                        c=c,
                        label="%d (py)" % express,
                    )
                    ax1.plot(
                        pixels,
                        image_idl,
                        ls="--",
                        alpha=0.8,
                        c=c,
                        label="%d (idl)" % express,
                    )
                    ax2.plot(
                        pixels,
                        (image_post_cti - image_idl) / image_idl,
                        alpha=0.8,
                        ls=":",
                        c=c,
                    )
                else:
                    ax1.plot(
                        pixels, image_post_cti, alpha=0.8, c=c, label="%d" % express
                    )
                    ax1.plot(pixels, image_idl, alpha=0.8, c=c, ls="--")
                    ax2.plot(
                        pixels,
                        (image_post_cti - image_idl) / image_idl,
                        alpha=0.8,
                        ls=":",
                        c=c,
                    )

            assert image_post_cti == pytest.approx(image_idl, rel=0.02)

        if do_plot:
            ax1.legend(title="express", loc="lower left")
            ax1.set_yscale("log")
            ax1.set_xlabel("Pixel")
            ax1.set_xlim(100, 121)
            ax1.set_ylabel("Counts")
            ax2.set_ylabel("Fractional Difference (dotted)")
            plt.tight_layout()
            plt.show()

    def test__add_cti__single_pixel__vary_express_3__compare_old_arctic(self):

        # Manually set True to make the plot
        do_plot = False
        # do_plot = True

        image_pre_cti = np.zeros((40, 1))
        image_pre_cti[2, 0] = 800

        traps = [cti.TrapInstantCapture(density=10, release_timescale=5)]
        ccd = cti.CCD(
            phases=[
                cti.CCDPhase(
                    well_fill_power=0.5, full_well_depth=1000, well_notch_depth=0
                )
            ]
        )
        roe = cti.ROE(
            empty_traps_between_columns=True,
            empty_traps_for_first_transfers=False,
            use_integer_express_matrix=True,
        )

        if do_plot:
            pixels = np.arange(len(image_pre_cti))
            colours = ["#1199ff", "#ee4400", "#7711dd", "#44dd44", "#775533"]
            plt.figure(figsize=(10, 6))
            ax1 = plt.gca()
            ax2 = ax1.twinx()

        for i, (express, image_idl) in enumerate(
            zip(
                # [2, 40],
                [40],
                [
                    # [ # express=2 but different save/restore trap approach
                    #     0.0000000,
                    #     0.0000000,
                    #     773.16687,
                    #     6.1919436,
                    #     6.3684354,
                    #     6.2979879,
                    #     6.0507884,
                    #     5.7028670,
                    #     5.2915287,
                    #     4.8539200,
                    #     4.4115725,
                    #     3.9793868,
                    #     3.5681694,
                    #     3.1855009,
                    #     2.8297379,
                    #     2.5070403,
                    #     2.2149866,
                    #     1.9524645,
                    #     1.7182100,
                    #     1.5103321,
                    #     1.3410047,
                    #     1.1967528,
                    #     1.0735434,
                    #     0.96801299,
                    #     0.87580109,
                    #     0.79527515,
                    #     0.72667038,
                    #     0.66463155,
                    #     0.61054391,
                    #     0.56260395,
                    #     0.51870286,
                    #     0.47962612,
                    #     0.44496229,
                    #     0.41361856,
                    #     0.38440439,
                    #     0.35855818,
                    #     0.33410615,
                    #     0.31309450,
                    #     0.29213923,
                    #     0.27346680,
                    # ],
                    [
                        0.00000,
                        0.00000,
                        773.317,
                        5.98876,
                        6.13135,
                        6.05125,
                        5.81397,
                        5.49105,
                        5.11484,
                        4.71890,
                        4.32139,
                        3.93756,
                        3.57154,
                        3.23464,
                        2.91884,
                        2.63640,
                        2.37872,
                        2.14545,
                        1.93805,
                        1.75299,
                        1.58590,
                        1.43964,
                        1.30883,
                        1.19327,
                        1.09000,
                        0.996036,
                        0.915593,
                        0.841285,
                        0.775049,
                        0.718157,
                        0.664892,
                        0.617069,
                        0.574792,
                        0.537046,
                        0.502112,
                        0.471202,
                        0.442614,
                        0.417600,
                        0.394439,
                        0.373072,
                    ]
                ],
            )
        ):
            image_post_cti = cti.add_cti(
                image=image_pre_cti,
                parallel_traps=traps,
                parallel_ccd=ccd,
                parallel_roe=roe,
                parallel_express=express,
                verbosity=0,
            ).T[0]

            image_idl = np.array(image_idl)

            if do_plot:
                c = colours[i]

                if i == 0:
                    ax1.plot(
                        pixels,
                        image_post_cti,
                        alpha=0.8,
                        c=c,
                        label="%d (py)" % express,
                    )
                    ax1.plot(
                        pixels,
                        image_idl,
                        ls="--",
                        alpha=0.8,
                        c=c,
                        label="%d (idl)" % express,
                    )
                    ax2.plot(
                        pixels,
                        (image_post_cti - image_idl) / image_idl,
                        alpha=0.8,
                        ls=":",
                        c=c,
                    )
                else:
                    ax1.plot(
                        pixels, image_post_cti, alpha=0.8, c=c, label="%d" % express
                    )
                    ax1.plot(pixels, image_idl, alpha=0.8, c=c, ls="--")
                    ax2.plot(
                        pixels,
                        (image_post_cti - image_idl) / image_idl,
                        alpha=0.8,
                        ls=":",
                        c=c,
                    )

            assert image_post_cti == pytest.approx(image_idl, rel=0.03)

        if do_plot:
            ax1.legend(title="express", loc="lower left")
            ax1.set_yscale("log")
            ax1.set_xlabel("Pixel")
            ax1.set_ylabel("Counts")
            ax2.set_ylabel("Fractional Difference (dotted)")
            plt.tight_layout()
            plt.show()

class TestCompareTrapTypes:
    def test__add_cti__all_trap_types_broadly_similar_results(self):

        # Manually set True to make the plot
        do_plot = False
        # do_plot = True

        image_pre_cti = np.zeros((20, 1))
        image_pre_cti[2, 0] = 800

        trap_ic = cti.TrapInstantCapture(density=10, release_timescale=-1 / np.log(0.5))
        trap_sc = cti.TrapSlowCapture(
            density=10, release_timescale=-1 / np.log(0.5), capture_timescale=0.1
        )
        trap_ic_co = cti.TrapInstantCaptureContinuum(
            density=10, release_timescale=-1 / np.log(0.5), release_timescale_sigma=0.05
        )
        trap_sc_co = cti.TrapSlowCaptureContinuum(
            density=10,
            release_timescale=-1 / np.log(0.5),
            release_timescale_sigma=0.05,
            capture_timescale=0.1,
        )

        ccd = cti.CCD(
            phases=[
                cti.CCDPhase(
                    well_fill_power=1, full_well_depth=1000, well_notch_depth=0
                )
            ]
        )
        roe = cti.ROE(
            empty_traps_between_columns=True,
            empty_traps_for_first_transfers=False,
            use_integer_express_matrix=True,
        )
        express = 5

        # Standard instant-capture traps
        image_post_cti_ic = cti.add_cti(
            image=image_pre_cti,
            parallel_roe=roe,
            parallel_ccd=ccd,
            parallel_traps=[trap_ic],
            parallel_express=express,
            verbosity=0,
        ).T[0]

        if do_plot:
            pixels = np.arange(len(image_pre_cti))
            colours = ["#1199ff", "#ee4400", "#7711dd", "#44dd44"]
            plt.figure(figsize=(10, 6))
            ax1 = plt.gca()
            ax2 = ax1.twinx()

            ax1.plot(
                pixels,
                image_post_cti_ic,
                alpha=0.8,
                c=colours[0],
                label="Instant Capture",
            )

        # Other trap types
        for i, (trap, label) in enumerate(
            zip(
                [trap_sc, trap_ic_co, trap_sc_co],
                ["Slow Capture", "Instant Capture Continuum", "Slow Capture Continuum"],
            )
        ):
            image_post_cti = cti.add_cti(
                image=image_pre_cti,
                parallel_roe=roe,
                parallel_ccd=ccd,
                parallel_traps=[trap],
                parallel_express=express,
                verbosity=0,
            ).T[0]

            if do_plot:
                c = colours[i + 1]

                ax1.plot(pixels, image_post_cti, alpha=0.8, c=c, label=label)
                ax2.plot(
                    pixels,
                    (image_post_cti - image_post_cti_ic) / image_post_cti_ic,
                    alpha=0.8,
                    ls=":",
                    c=c,
                )

            assert image_post_cti == pytest.approx(image_post_cti_ic, rel=0.1)

        if do_plot:
            ax1.legend(loc="lower left")
            ax1.set_yscale("log")
            ax1.set_xlabel("Pixel")
            ax1.set_ylabel("Counts")
            ax2.set_ylabel("Fractional Difference (dotted)")
            plt.tight_layout()
            plt.show()



class TestMultipleTrapTypes:
    def test__add_cti__multiple_trap_types_together_or_consecutively(self):

        # Manually set True to make the plot
        do_plot = False
        # do_plot = True

        image_pre_cti = np.zeros((20, 1))
        image_pre_cti[2, 0] = 800

        trap_ic = cti.TrapInstantCapture(density=10, release_timescale=-1 / np.log(0.5))
        trap_sc = cti.TrapSlowCapture(
            density=10, release_timescale=-1 / np.log(0.5), capture_timescale=0.1
        )
        trap_ic_co = cti.TrapInstantCaptureContinuum(
            density=10, release_timescale=-1 / np.log(0.5), release_timescale_sigma=0.05
        )
        trap_sc_co = cti.TrapSlowCaptureContinuum(
            density=10,
            release_timescale=-1 / np.log(0.5),
            release_timescale_sigma=0.05,
            capture_timescale=0.1,
        )

        ccd = cti.CCD(
            phases=[
                cti.CCDPhase(
                    well_fill_power=1, full_well_depth=1000, well_notch_depth=0
                )
            ]
        )
        roe = cti.ROE(
            empty_traps_between_columns=True,
            empty_traps_for_first_transfers=False,
            use_integer_express_matrix=True,
        )
        express = 5


        # Implement all types of traps simultaneously
        image_post_cti_simultaneously = cti.add_cti(
            image=image_pre_cti,
            parallel_roe=roe,
            parallel_ccd=ccd,
            parallel_traps=[trap_ic,trap_sc,trap_ic_co,trap_sc_co],
            parallel_express=express,
            verbosity=0,
        ).T[0]
        
        if do_plot:
            pixels = np.arange(len(image_pre_cti))
            colours = ["#1199ff", "#ee4400", "#7711dd", "#44dd44", "#666666"]
            plt.figure(figsize=(10, 6))
            ax1 = plt.gca()
            ax2 = ax1.twinx()

            ax1.plot(
                pixels,
                image_post_cti_simultaneously,
                alpha=0.8,
                c=colours[0],
                label="All types simultaneously",
            )

        # Implement trap types one after another
        image_post_cti_separately = image_pre_cti
        for i, (trap, label) in enumerate(
            zip(
                [trap_ic, trap_sc, trap_ic_co, trap_sc_co],
                ["After Instant Capture", "After Slow Capture", "After Instant Capture Continuum", "After Slow Capture Continuum"],
            )
        ):
            image_pre_cti = image_post_cti_separately
            image_post_cti_separately = cti.add_cti(
                image=image_pre_cti,
                parallel_roe=roe,
                parallel_ccd=ccd,
                parallel_traps=[trap],
                parallel_express=express,
                verbosity=0,
            )
            
            if do_plot:
                c = colours[i + 1]

                ax1.plot(pixels, image_post_cti_separately.T[0], alpha=0.8, c=c, label=label)
                ax2.plot(
                    pixels,
                    (image_post_cti_separately.T[0] - image_post_cti_simultaneously) / image_post_cti_simultaneously,
                    alpha=0.8,
                    ls=":",
                    c=c,
                )

            assert 1 == pytest.approx(1.0001, rel=0.1)
        assert image_post_cti_separately.T[0] == pytest.approx(image_post_cti_simultaneously, rel=0.1)

        if do_plot:
            ax1.legend(loc="lower left")
            ax1.set_yscale("log")
            ax1.set_xlabel("Pixel")
            ax1.set_ylabel("Counts")
            ax2.set_ylabel("Fractional Difference (dotted)")
            plt.tight_layout()
            plt.show()


class TestRemoveCTI:
    def test__remove_cti__better_removal_with_more_iterations(self):
        # Add CTI to a test image, then remove it
        image_pre_cti = np.array(
            [
                [0.0, 0.0, 0.0, 0.0],
                [200.0, 0.0, 0.0, 0.0],
                [0.0, 200.0, 0.0, 0.0],
                [0.0, 0.0, 200.0, 0.0],
                [0.0, 0.0, 0.0, 0.0],
                [0.0, 0.0, 0.0, 0.0],
            ]
        )

        roe = cti.ROE(
            dwell_times=[1.0],
            empty_traps_between_columns=True,
            empty_traps_for_first_transfers=False,
            force_release_away_from_readout=True,
            use_integer_express_matrix=False,
        )
        ccd = cti.CCD(
            phases=[
                cti.CCDPhase(
                    full_well_depth=1e3, well_notch_depth=0.0, well_fill_power=1.0
                )
            ],
            fraction_of_traps_per_phase=[1.0],
        )
        traps = [
            cti.TrapInstantCapture(density=10.0, release_timescale=-1.0 / np.log(0.5))
        ]
        express = 0
        offset = 0
        start = 0
        stop = -1

        # Add CTI
        image_add_cti = cti.add_cti(
            image=image_pre_cti,
            parallel_roe=roe,
            parallel_ccd=ccd,
            parallel_traps=traps,
            parallel_express=express,
            parallel_window_offset=offset,
            parallel_window_start=start,
            parallel_window_stop=stop,
            serial_roe=roe,
            serial_ccd=ccd,
            serial_traps=traps,
            serial_express=express,
            serial_window_offset=offset,
            serial_window_start=start,
            serial_window_stop=stop,
            verbosity=0,
        )

        # Remove CTI
        for n_iterations in range(2, 7):
            image_remove_cti = cti.remove_cti(
                image=image_add_cti,
                n_iterations=n_iterations,
                parallel_roe=roe,
                parallel_ccd=ccd,
                parallel_traps=traps,
                parallel_express=express,
                parallel_window_offset=offset,
                parallel_window_start=start,
                parallel_window_stop=stop,
                serial_roe=roe,
                serial_ccd=ccd,
                serial_traps=traps,
                serial_express=express,
                serial_window_offset=offset,
                serial_window_start=start,
                serial_window_stop=stop,
                verbosity=0,
            )

            # Expect better results with more iterations
            tolerance = 10 ** (1 - n_iterations)
            assert image_remove_cti == pytest.approx(image_pre_cti, abs=tolerance)


class TestCTIModelForHSTACS:
    def test__CTI_model_for_HST_ACS(self):
        # Julian dates
        date_acs_launch = 2452334.5  # ACS launched, SM3B, 01 March 2002
        date_T_change = 2453920.0  # Temperature changed, 03 July 2006
        date_sm4_repair = 2454968.0  # ACS repaired, SM4, 16 May 2009

        # Before the temperature change
        date_1 = date_T_change - 246
        date_2 = date_T_change - 123
        roe_1, ccd_1, traps_1 = cti.CTI_model_for_HST_ACS(date_1)
        roe_2, ccd_2, traps_2 = cti.CTI_model_for_HST_ACS(date_2)

        # Trap density grows with time
        total_density_1 = np.sum([trap.density for trap in traps_1])
        total_density_2 = np.sum([trap.density for trap in traps_2])
        assert total_density_1 < total_density_2

        # After the SM4 repair
        date_3 = date_sm4_repair + 123
        date_4 = date_sm4_repair + 246
        roe_3, ccd_3, traps_3 = cti.CTI_model_for_HST_ACS(date_3)
        roe_4, ccd_4, traps_4 = cti.CTI_model_for_HST_ACS(date_4)

        # Trap density grows with time
        total_density_3 = np.sum([trap.density for trap in traps_3])
        total_density_4 = np.sum([trap.density for trap in traps_4])
        assert total_density_3 < total_density_4

        # Constant ROE and CCD
        for roe in [roe_1, roe_2, roe_3, roe_4]:
            assert len(roe.dwell_times) == 1
            assert roe.dwell_times[0] == 1.0
            assert roe.empty_traps_between_columns == True
            assert roe.empty_traps_for_first_transfers == False
            assert roe.force_release_away_from_readout == True
            assert roe.use_integer_express_matrix == False
        for ccd in [ccd_1, ccd_2, ccd_3, ccd_4]:
            assert len(ccd.phases) == 1
            assert ccd.fraction_of_traps_per_phase[0] == 1.0
            assert ccd.phases[0].full_well_depth == 84700
            assert ccd.phases[0].well_notch_depth == 0.0
            assert ccd.phases[0].well_fill_power == 0.478


class TestDictable:


    def test__ccd_is_dictable(self):

        json_file = path.join(
            "{}".format(path.dirname(path.realpath(__file__))), "files", "ccd.json"
        )

        ccd = cti.CCDPhase(full_well_depth=1.0, well_notch_depth=2.0, well_fill_power=3.0)

        with open(json_file, "w+") as f:
            json.dump(ccd.dict(), f, indent=4)

        with open(json_file, "r+") as f:
            ccd_load_dict = json.load(f)

        ccd_load = cti.CCDPhase.from_dict(ccd_load_dict)

        assert ccd_load.full_well_depth == 1.0
        assert ccd_load.well_notch_depth == 2.0
        assert ccd_load.well_fill_power == 3.0

    def test__trap_is_dictable(self):

        json_file = path.join(
            "{}".format(path.dirname(path.realpath(__file__))), "files", "trap.json"
        )

        trap = cti.TrapInstantCapture(density=1.0, release_timescale=2.0)

        with open(json_file, "w+") as f:
            json.dump(trap.dict(), f, indent=4)

        with open(json_file, "r+") as f:
            trap_load_dict = json.load(f)

        trap_load = cti.TrapInstantCapture.from_dict(trap_load_dict)

        assert trap_load.density == 1.0
        assert trap_load.release_timescale == 2.0


def run_demo():
    # Add CTI to a test image, then remove it
    image_pre_cti = np.array(
        [
            [0.0, 0.0, 0.0, 0.0],
            [200.0, 0.0, 0.0, 0.0],
            [0.0, 200.0, 0.0, 0.0],
            [0.0, 0.0, 200.0, 0.0],
            [0.0, 0.0, 0.0, 0.0],
            [0.0, 0.0, 0.0, 0.0],
        ]
    )

    roe = cti.ROE(
        dwell_times=[1.0],
        empty_traps_between_columns=True,
        empty_traps_for_first_transfers=False,
        force_release_away_from_readout=True,
        use_integer_express_matrix=False,
    )
    ccd = cti.CCD(
        phases=[
            cti.CCDPhase(full_well_depth=1e3, well_notch_depth=0.0, well_fill_power=1.0)
        ],
        fraction_of_traps_per_phase=[1.0],
    )
    traps = [cti.TrapInstantCapture(density=10.0, release_timescale=-1.0 / np.log(0.5))]
    express = 0
    offset = 0
    start = 0
    stop = -1

    print("\n# Test image:")
    cti.print_array_2D(image_pre_cti)

    print("\n# Add CTI")
    image_post_cti = cti.add_cti(
        image=image_pre_cti,
        parallel_roe=roe,
        parallel_ccd=ccd,
        parallel_traps=traps,
        parallel_express=express,
        parallel_offset=offset,
        parallel_window_start=start,
        parallel_window_stop=stop,
        serial_roe=roe,
        serial_ccd=ccd,
        serial_traps=traps,
        serial_express=express,
        serial_offset=offset,
        serial_window_start=start,
        serial_window_stop=stop,
        verbosity=1,
    )

    print("\n# Image with CTI added:")
    cti.print_array_2D(image_post_cti)

    print("\n# Remove CTI")
    image_removed_cti = cti.remove_cti(
        image=image_post_cti,
        n_iterations=4,
        parallel_roe=roe,
        parallel_ccd=ccd,
        parallel_traps=traps,
        parallel_express=express,
        parallel_offset=offset,
        parallel_window_start=start,
        parallel_window_stop=stop,
        serial_roe=roe,
        serial_ccd=ccd,
        serial_traps=traps,
        serial_express=express,
        serial_offset=offset,
        serial_window_start=start,
        serial_window_stop=stop,
        verbosity=1,
    )

    print("\n# Image with CTI removed:")
    cti.print_array_2D(image_removed_cti)


def run_benchmark():
    # Download the test image
    filename = os.path.join(os.path.join(path, ".."), "hst_acs_10_col.txt")
    if not os.path.isfile(filename):
        url_path = "http://astro.dur.cti.uk/~cklv53/files/hst_acs_10_col.txt"
        urlretrieve(url_path, filename)

    # Load the image
    image_pre_cti = np.loadtxt(filename, skiprows=1)

    # CTI model parameters
    roe = cti.ROE(
        dwell_times=[1.0],
        empty_traps_between_columns=True,
        empty_traps_for_first_transfers=False,
        force_release_away_from_readout=True,
        use_integer_express_matrix=False,
    )
    ccd = cti.CCD(
        phases=[
            cti.CCDPhase(full_well_depth=1e4, well_notch_depth=0.0, well_fill_power=1.0)
        ],
        fraction_of_traps_per_phase=[1.0],
    )
    traps = [cti.TrapInstantCapture(density=10.0, release_timescale=-1.0 / np.log(0.5))]
    express = 5
    offset = 0
    start = 0
    stop = -1

    image_post_cti = cti.add_cti(
        image=image_pre_cti,
        parallel_roe=roe,
        parallel_ccd=ccd,
        parallel_traps=traps,
        parallel_express=express,
        parallel_offset=offset,
        parallel_window_start=start,
        parallel_window_stop=stop,
    )


def print_help():
    print(
        "ArCTICpy \n"
        "======== \n"
        "AlgoRithm for Charge Transfer Inefficiency (CTI) Correction: python wrapper \n"
        "--------------------------------------------------------------------------- \n"
        "Add or remove image trails due to charge transfer inefficiency in CCD "
        "detectors by modelling the trapping, releasing, and moving of charge along "
        "pixels. \n"
        "\n"
        "Run with pytest to perform some unit tests using the python wrapper. \n"
        "\n"
        "-d, --demo \n"
        "    Execute the demo code in the run_demo() function in this file, which adds \n"
        "    then removes CTI from a test image. \n"
        "-b, --benchmark \n"
        "    Execute the run_benchmark() function in this file, e.g. for profiling. \n"
        "\n"
        "See README.md for more information.  https://github.com/jkeger/arctic \n\n"
    )


if __name__ == "__main__":
    try:
        if sys.argv[1] in ["-d", "--demo"]:
            print("# Running demo code!")
            run_demo()
        elif sys.argv[1] in ["-b", "--benchmark"]:
            print("# Running benchmark code")
            run_benchmark()
        else:
            print_help()
    except IndexError:
        print_help()

import os
import sys

sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)), ".."))
import arcticpy as ac
import numpy as np
import pytest
import matplotlib.pyplot as plt


class TestCompareOldArCTIC:
    def test__add_cti__single_pixel__vary_express__compare_old_arctic(self):

        # Manually set True to make the plot
        do_plot = False
        # do_plot = True

        image_pre_cti = np.zeros((20, 1))
        image_pre_cti[2, 0] = 800

        # Nice numbers for easy manual checking
        traps = [ac.TrapInstantCapture(density=10, release_timescale=-1 / np.log(0.5))]
        ccd = ac.CCD(
            phases=[
                ac.CCDPhase(well_fill_power=1, full_well_depth=1000, well_notch_depth=0)
            ]
        )
        roe = ac.ROE(
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
            image_post_cti = ac.add_cti(
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
                        pixels,
                        image_post_cti,
                        alpha=0.8,
                        c=c,
                        label="%d" % express,
                    )
                    ax1.plot(
                        pixels,
                        image_idl,
                        alpha=0.8,
                        c=c,
                        ls="--",
                    )
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

        traps = [ac.TrapInstantCapture(density=10, release_timescale=-1 / np.log(0.5))]
        ccd = ac.CCD(
            phases=[
                ac.CCDPhase(
                    well_fill_power=0.5, full_well_depth=1000, well_notch_depth=0
                )
            ]
        )
        roe = ac.ROE(
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
            image_post_cti = ac.add_cti(
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
                        pixels,
                        image_post_cti,
                        alpha=0.8,
                        c=c,
                        label="%d" % express,
                    )
                    ax1.plot(
                        pixels,
                        image_idl,
                        alpha=0.8,
                        c=c,
                        ls="--",
                    )
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

        traps = [ac.TrapInstantCapture(density=10, release_timescale=5)]
        ccd = ac.CCD(
            phases=[
                ac.CCDPhase(
                    well_fill_power=0.5, full_well_depth=1000, well_notch_depth=0
                )
            ]
        )
        roe = ac.ROE(
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
                    ],
                ],
            )
        ):
            image_post_cti = ac.add_cti(
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
                        pixels,
                        image_post_cti,
                        alpha=0.8,
                        c=c,
                        label="%d" % express,
                    )
                    ax1.plot(
                        pixels,
                        image_idl,
                        alpha=0.8,
                        c=c,
                        ls="--",
                    )
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


if __name__ == "__main__":
    # Add CTI to a test image
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
    
    roe = ac.ROE(
        dwell_times=[1.0],
        empty_traps_between_columns=True,
        empty_traps_for_first_transfers=False,
        force_release_away_from_readout=True,
        use_integer_express_matrix=False,
    )
    ccd = ac.CCD(
        phases=[
            ac.CCDPhase(full_well_depth=1e3, well_notch_depth=0.0, well_fill_power=1.0)
        ],
        fraction_of_traps_per_phase=[1.0],
    )
    traps = [ac.TrapInstantCapture(density=10.0, release_timescale=-1.0 / np.log(0.5))]
    express = 0
    offset = 0
    start = 0
    stop = -1
    
    ac.print_array_2D(image_pre_cti)
    
    image_post_cti = ac.add_cti(
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
    
    ac.print_array_2D(image_post_cti)

ArCTIC
======

AlgoRithm for Charge Transfer Inefficiency (CTI) Correction
-----------------------------------------------------------

Add or remove image trails due to charge transfer inefficiency in CCD detectors
by modelling the trapping, releasing, and moving of charge along pixels.

https://github.com/jkeger/arctic

Jacob Kegerreis: jacob.kegerreis@durham.ac.uk  
Richard Massey  
James Nightingale  

This file contains general documentation and some examples. See also the
docstrings and comments throughout the code for further details, and the unit
tests for more examples and tests.


\
Contents
--------
+ Installation
    + Python wrapper
    + GSL
+ Usage
    + Python example
    + C++
+ Unit Tests
+ Files
+ Documentation
    + Add/remove CTI
    + CCD
    + ROE
    + Trap species
    + Trap managers
    + Python wrapper


\
Compilation
===========

<!-- 
Download
--------
+ Run `git clone https://github.com/jkeger/arctic.git` and `cd arctic`. For this branch, also `git checkout dev`.
-->

Step 1 GNU Scientific Library
-----------------------------
+ Run `make gsl` to download and install the GNU Scientific Library to local subdirectory gsl/ that contains bin/, include/, lib/, share/.
If your system already has GSL installed, you can skip this step and  DIR_GSL=/path/to/gsl make all

    **MacOS:** requires `sudo make gsl` to grant permission to also run ./configure in the middle of the `get_gsl.sh` script.
If you don't like doing this, you can cut and paste the few lines marked with comments in the middle of that script.

Step 2 arCTIc C++ core
----------------------
+ Run `make` to compile the C++ code into an `arctic` executable and `libarctic.so` dynamic library. <!-- + Add `/***current*directory***/arctic` to your `$PATH`. -->

    You should now get output from `./arctic --demo`. The makefile header describes additional options, including unit tests that can be compiled via `make all` but are only needed by developers.

    <!-- **MacOS:** cannot currently compile the unit tests. It gets confused because the Catch2 unit test framework uses a second main.c file. 
If you know how to circumvent this, please tell us! On the first build, mac users may also need to create an (empty) directory 
/sw/lib via `sudo mount -uw /` then `sudo mkdir -p /sw/lib`. -->

Step 3 arCTIc python wrapper
----------------------------
+ Run `make wrapper` to create `arcticpy/wrapper.cypython*.so`.
+ Add `/***current*directory***/arctic` to both your `$PYTHONPATH` and to another system variable `$DYLD_LIBRARY_PATH`.
+ Import the python module, e.g. `import arcticpy as cti.`.

    You should now get output from `import numpy, arcticpy ; test=arcticpy.add_cti(numpy.zeros((5,5)))` (in python).


\
Usage
=====
See the `run_demo()` functions in `src/main.cpp` and `test/test_arcticpy.py` for
more basic examples of using the main code or python wrapper to add and remove
CTI to a test image.


Python wrapper
--------------
Full example correction of CTI for a Hubble Space Telescope ACS image,
using the [https://pypi.org/project/autoarray/](autoarray) package
to load and save the fits image with correct units and quadrant rotations, etc:
```python
import arcticpy as cti
import autoarray as aa

image_path = "image_path/image_name"

# Load each quadrant of the image  (see pypi.org/project/autoarray)
image_A, image_B, image_C, image_D = [
    aa.acs.ImageACS.from_fits(
        file_path=image_path + ".fits",
        quadrant_letter=quadrant,
        bias_subtract_via_bias_file=True,
        bias_subtract_via_prescan=True,
    ).native
    for quadrant in ["A", "B", "C", "D"]
]

# Automatic CTI model  (see CTI_model_for_HST_ACS() in arcticpy/src/cti.py)
date = 2400000.5 + image_A.header.modified_julian_date
roe, ccd, traps = cti.CTI_model_for_HST_ACS(date)

# Or manual CTI model  (see class docstrings in src/<traps,roe,ccd>.cpp)
traps = [
    cti.TrapInstantCapture(density=0.6, release_timescale=0.74),
    cti.TrapInstantCapture(density=1.6, release_timescale=7.70),
    cti.TrapInstantCapture(density=1.4, release_timescale=37.0),
]
roe = cti.ROE()
ccd = cti.CCD(full_well_depth=84700, well_fill_power=0.478)

# Remove CTI  (see remove_cti() in src/cti.cpp)
image_out_A, image_out_B, image_out_C, image_out_D = [
    cti.remove_cti(
        image=image,
        n_iterations=5,
        parallel_roe=roe,
        parallel_ccd=ccd,
        parallel_traps=traps,
        parallel_express=5,
        verbosity=1,
    )
    for image in [image_A, image_B, image_C, image_D]
]

# Save the corrected image
aa.acs.output_quadrants_to_fits(
    file_path=image_path + "_out.fits",
    quadrant_a=image_out_A,
    quadrant_b=image_out_B,
    quadrant_c=image_out_C,
    quadrant_d=image_out_D,
    header_a=image_A.header,
    header_b=image_B.header,
    header_c=image_C.header,
    header_d=image_D.header,
    overwrite=True,
)
```


\
C++
---
ArCTIC will typically be used via the python wrapper module, but the code can be
run directly as `./arctic` with the following command-line options:

+ `-h`, `--help`  
    Print help information and exit.
+ `-v <int>`, `--verbosity=<int>`  
    The verbosity parameter to control the amount of printed information:
    + `0`   No printing (except errors etc).
    + `1`   Standard.
    + `2`   Extra details.
+ `-d`, `--demo`  
    Execute the editable demo code in the `run_demo()` function at the very top
    of `src/main.cpp`. A good place to run your own quick tests or use arctic
    without any wrappers. The demo version adds then removes CTI from a
    test image.
+ `-b`, `--benchmark`  
    Execute the simple test `run_benchmark()` function in `src/main.cpp`,
    e.g. for profiling.

\
The code can also be used as a library for other C++ programs, as in the
`lib_test` example.

The python wrapper also has demo and benchmark functions provided.
Run `python3 test/test_arcticpy.py` with `-d` or `-b` (or the long versions)
as above, or with anything else to print help information.



\
Unit Tests
==========
Tests are included for most individual parts of the code, organised with Catch2.

As well as making sure the code is working correctly, most tests are intended to
be relatively reader-friendly examples to help show how all the pieces of the
code work if you need to understand the internal details as a developer,
alongside the more user-focused documentation.

Compile the tests with `make test` (or `make all`) in the top directory, then
run with `./test_arctic`.

Add arguments to select which tests to run by their names, e.g:
+ `*'these ones'*`  All tests that contain 'these ones'.
+ `~*'not these'*`  All tests except those that contain 'not these'.
+ `-# [#filename]`  All tests in filename.cpp.

Compiling with `make lib_test` will create a simple example of using the shared
object library (`libarctic.so`), which is run with `./lib_test`.

A few python tests of the primary functions are included for the arcticpy
wrapper. Compile the wrapper with `make wrapper` (or `make all`) in the top
directory, then run with `pytest test/test_arcticpy.py`.



\
Files
=====
A quick summary of the code files and their contents:

+ `makefile`                The makefile for compiling the code. See its header.
    + `get_gsl.sh`          The script called by the makefile to install GSL.
+ `arctic`, `test_arctic`   The program and unit-test executables.
+ `libarctic.so`            The shared object library.
+ `src/`                    Source code files.
    + `main.cpp`  
        Main program. See above and its documentation for the command-line
        options, and see `run_demo()` for an example of running user-editable
        code directly.
    + `cti.cpp`  
        Contains the primary user-facing functions `add_cti()` and
        `remove_cti()`. These are wrappers for `clock_charge_in_one_direction()`,
        which contains the primary nested for loops over an image to add CTI to,
        in order: each column, each express pass (see below), and each row.
        (And then each step and each phase in the clocking sequence if doing
        multiphase clocking, see below.)
    + `ccd.cpp`  
        Defines the `CCD` classes that describe how electrons fill the volume
        inside each (phase of a) pixel in a CCD detector.
    + `roe.cpp`  
        Defines the `ROE` classes that describe the properties of readout
        electronics (ROE) used to operate a CCD detector.
    + `traps.cpp`  
        Defines the `Trap` classes that describe the properties of a single
        species of charge traps.    
    + `trap_managers.cpp`  
        Defines the internal `TrapManager` classes that organise one or many
        species of traps. Contains the core function
        `n_electrons_released_and_captured()`, called by
        `clock_charge_in_one_direction()` to model the capture and release of
        electrons and track the trapped electrons using the "watermarks".
    + `util.cpp`  
        Miscellaneous internal utilities.
+ `include/`                The `*.hpp` header files for each source code file.
+ `test/`                   Unit tests and examples.
+ `build/`                  Compiled object and dependency files.
+ `arcticpy/`               The python, Cython, and other files for the wrapper.
    + `setup.py`                The file for compiling the package.
    + `src/`                    Source files.
        + `cti.py`  
            The python versions of the primary user-facing functions `add_cti()`
            and `remove_cti()`.
        + `ccd.py`, `roe.py`, `traps.py`  
            The python versions of the `CCD`, `ROE`, and `Trap` classes that are
            needed as arguments for the primary CTI functions. These mirror the
            inputs for the corresponding same-name C++ classes documented below.
        + `wrapper.pyx`  
            The Cython wrapper, passes the python inputs to the C++ interface.
        + `interface.cpp`, `interface.hpp`  
            The source and header files for functions to cleanly interface
            between Cython and the main precompiled library. e.g. converts the
            image array and CTI model inputs into the required C++ objects.
        + `wrapper.cpp`, `../wrapper.cpython*.so`  
            Compiled Cython output files.



\
Documentation
=============
The code docstrings contain the full documentation for each class and function.

Most of the python wrapper code precisely mirrors the core C++ classes and
functions. The full docstrings are not duplicated in that case so please refer
to the C++ docstrings for the complete details.

This section provides an overview of the key contents and features. It is aimed
at general users plus a few extra details for anyone wanting to navigate or work
on the code itself.

The primary functions to add and remove CTI take as arguments custom objects
that describe the trap species, CCD properties, and ROE details (see below).
A core aspect of the code is that it includes several polymorphic versions of
each of these classes. These provide a variety of ways to model CTI, such as
different types of trap species, multiple phases in each pixel, or alternative
readout sequences for trap pumping, etc.


\
Add/remove CTI
--------------
### Add CTI
To add (and remove) CTI trails, the primary inputs are the initial image
followed by the properties of the CCD, readout electronics (ROE), and trap
species, for either or both parallel and serial clocking.

These parameters are set using the `CCD`, `ROE`, and `Trap` classes, as
described below.

See `add_cti()`'s docstring in `cti.cpp` for the full details, and
`clock_charge_in_one_direction()` for the inner code that loops over the image.

### Remove CTI
Removing CTI trails is done by iteratively modelling the addition of CTI, as
described in Massey et al. (2010) section 3.2 and Table 1.

The `remove_cti()` function takes all the same parameters as `add_cti()` plus
the number of iterations for the forward modelling.

More iterations provide higher accuracy at the cost of longer runtime. In
practice, 2 or 3 iterations are usually sufficient.

### Image
The input image should be a 2D array of charge values, where the first dimension
runs over the rows of pixels and the second inner dimension runs over the
separate columns, as in this example of an image before and after calling
`add_cti()` (with arbitrary trap parameters):

```C++
// Initial image with one bright pixel in the first three columns:
{{  0.0,     0.0,     0.0,     0.0  },
 {  200.0,   0.0,     0.0,     0.0  },
 {  0.0,     200.0,   0.0,     0.0  },
 {  0.0,     0.0,     200.0,   0.0  },
 {  0.0,     0.0,     0.0,     0.0  },
 {  0.0,     0.0,     0.0,     0.0  }}
// Image with parallel CTI trails:
{{  0.0,     0.0,     0.0,     0.0  },
 {  196.0,   0.0,     0.0,     0.0  },
 {  3.0,     194.1,   0.0,     0.0  },
 {  2.0,     3.9,     192.1,   0.0  },
 {  1.3,     2.5,     4.8,     0.0  },
 {  0.8,     1.5,     2.9,     0.0  }}
// Final image with parallel and serial CTI trails:
{{  0.0,     0.0,     0.0,     0.0  },
 {  194.1,   1.9,     1.5,     0.9  },
 {  2.9,     190.3,   2.9,     1.9  },
 {  1.9,     3.8,     186.5,   3.7  },
 {  1.2,     2.4,     4.7,     0.1  },
 {  0.7,     1.4,     2.8,     0.06 }}
```

As this illustrates, by default, charge is transferred "up" from row N to row 0
along each independent column, such that the charge in the first element/pixel 0
undergoes 1 transfer, and the final row N is furthest from the readout register
so undergoes N+1 transfers. The CTI trails appear behind bright pixels as the
traps capture electrons from their original pixels and release them at a later
time.

Parallel clocking is the transfer along each independent column, while serial
clocking is across the columns and is performed after parallel clocking, if the
arguments for each are not omitted.

Note that technically instead of actually moving the charges past the traps in
each pixel, as happens in the real hardware, the code tracks the occupancies of
the traps (see Watermarks below) and updates them by scanning over each pixel.
This simplifies the code structure and keeps the image array conveniently
static.

### Express
As described in more detail in Massey et al. (2014) section 2.1.5, the effects
of each individual pixel-to-pixel transfer can be very similar, so multiple
transfers can be computed at once for efficiency.

This allows much faster computation with a mild decrease in accuracy.

For example, the electrons in the pixel closest to the readout have only one
transfer, those in the 2nd pixel undergo 2, those in the 3rd have 3, and so on.
The `express` input sets the number of times the transfers are calculated.
`express = 1` is the fastest and least accurate, `express = 2` means the
transfers are re-computed half-way through the readout, up to `express = N`
where `N` is the total number of pixels, for the full computation of every step
without assumptions.

The default `express = 0` is a convenient input for automatic `express = N`.

### Offsets and windows
To account for additional transfers before the first image pixel reaches the
readout (e.g. a prescan region), the `offset` sets the number of extra pixels.

Somewhat similarly, instead of adding CTI to the entire supplied image, only a
subset of pixels can be selected using the `window_start` and `_stop` arguments.
Note that, because of edge effects, the range should be started several pixels
before the actual region of interest.


\
CCD
---
How electrons fill the volume inside each (phase of a) pixel in the
charged-coupled device (CCD) detector.

By default, charge is assumed to move instantly from one pixel to another, but
each pixel can be separated into multiple phases, in combination with a
multiphase ROE clock sequence.

See the `CCD` and `CCDPhase` class docstrings in `ccd.cpp` for the full
documentation.

### Multiple phases
The `CCD` object can be created either with a single `CCDPhase` or a list of
phases plus an array of the fraction of traps distributed in each phase, which
could be interpreted as the physical width of each phase in the pixel.

The number of phases must be the same for the CCD and ROE objects.


\
ROE
---
The properties of readout electronics (ROE) used to operate a CCD.

Three different modes are available:

+ Standard, in which charge is read out from the pixels in which they
    start to the readout register, so are transferred across a different number
    of pixels depending on their initial distance from readout.
+ Charge injection, in which the electrons are directly created at the far end
    of the CCD, then are all transferred the same number of times through the
    full image of pixels to the readout register.
+ Trap pumping (AKA pocket pumping), in which charge is transferred back and
    forth, to end up in the same place it began.

See the `ROE`, `set_clock_sequence()`, and child class docstrings in `roe.cpp`
for the full documentation, including illustrative diagrams of the multiphase
clocking sequences.

### Express matrix  
The `ROE` class also contains the `set_express_matrix_from_pixels_and_express()`
function used to generate the array of express multipliers that controls which
transfers are computed.

### Multiple phases
Like the CCD, the `ROE` object can model single or multiple steps in the clock
sequence for each transfer. The number of steps in a clocking sequence is
usually same as the number of phases, but not necessarily, as in the case for
trap pumping.

The number of phases must be the same for the CCD and ROE objects.


\
Trap species
------------
The parameters for a trap species.

See the `Trap*` class docstrings in `traps.cpp` for the full documentation.

### Instant capture
For the relatively simple algorithm of release first then instant capture. This
is the primary model used by previous versions of ArCTIC.

Optionally, these traps can be assigned a non-uniform distribution with volume
within the pixel, e.g. to model "surface" traps that are only reached by very
large charge clouds.

### Slow capture
For combined release and non-instant capture, following Lindegren (1998),
section 3.2.

### Continuum lifetime distribution (instant capture)
For a trap species with a continuum (log-normal distribution) of release
timescales, and instant capture.

### Continuum lifetime distribution (slow capture)
For a trap species with a continuum (log-normal distribution) of release
timescales, and non-instant capture.


\
Trap managers
-------------
This is not relevant for typical users, but is key to the internal structure of
the code and the "watermark" approach to tracking the trap occupancies (see
below).

The different trap manager child classes also implement the different algorithms
required for the corresponding types of trap species described above, primarily
in the `n_electrons_released_and_captured()` method.

See the `TrapManager*` class docstrings in `trap_managers.cpp` for the full
documentation.

To allow the options of multiple types of trap species and/or multiple phases in
each pixel, the code actually uses a top-level "trap-manager manager" to hold
the necessary multiple `TrapManager` objects. See the `TrapManagerManager` class
docstring for the details.

### Watermarks
The `watermark_volumes` and `watermark_fills` arrays track the states of the
charge traps and the number of electrons they have captured. The core release
and capture algorithms are based on these arrays.

`watermark_volumes` is a 1D array of the fractional volume each watermark
represents as a proportion of the pixel volume. These are set by the volume the
charge cloud reaches in the pixel when electrons are captured.

`watermark_fills` is a 2D-style array (stored as 1D internally) of the fraction
of traps within that watermark that are full, i.e. that have captured electrons,
for each trap species. For efficiency, these values are multiplied by the
density of that trap species (the number of traps per pixel).

In the standard case, *capture* of electrons creates a *new watermark* level
at the "height" of the charge cloud and can overwrite lower ones as the traps
are filled up, while *release* lowers the fraction of filled traps in each
watermark level, without changing the volumes. Note that the stored volumes give
the size of that level, not the cumulative total.

The details can vary for different trap managers.

The unit tests in `test/test_trap_managers.cpp`, especially those for release
and capture, contain simple-number examples to demonstrate how it all works for
developers.


\
Python wrapper
--------------
After compiling the Cython, the `arcticpy` python module can be imported and
used as normal. `test/test_arcticpy.py` contains some tests and a basic example,
and see the full example at the top of this file.

The majority of the python functions and classes (`arcticpy/src/*.py`) directly
mirror the core C++, so in those cases the full docstrings are not duplicated.

The wrapper is organised internally as follows:  
*python* --> *Cython* --> *C++ wrapper* --> *core library*.  
This multi-level structure is a bit more extensive then strictly necessary, but
this keeps each level much cleaner and with a single purpose.

The user-facing python functions take numpy arrays and custom input-parameter
objects as user-friendly arguments. These mirror exactly the custom C++ objects
used as arguments by the core C++ program described above. To convert cleanly
between the two, the individual arrays and numbers are extracted from the python
objects and are passed via the Cython wrapper to the C++ wrapper, which then
builds the C++ objects as arguments for the core library functions.





<!--
Dev notes
=========
+ Non-uniform volume (e.g. surface) traps are currently only implemented for
    instant-capture traps, but should be relatively simple to duplicate for the
    other base types.
+ The slow-capture algorithm is currently slow, primarily because unlike
    instant-capture where the watermarks are frequently overwritten, here the
    arrays grow very large. It should be possible to improve this by
    consolidating the watermarks periodically, since many levels end up
    extremely tiny. But it's pretty open how best to implement that.
+ The trap and trap_manager classes currently have a fair amount of duplicate or
    near-duplicate code, some of which could be abstracted to generic functions
    and/or tweaking the class inheritance structure now that we know what's
    needed. e.g. TrapManagerSlowCaptureContinuum's
    n_electrons_released_and_captured() is basically the same as
    TrapManagerSlowCapture's, aside from the time conversions etc within the
    same loops. Or TrapSlowCaptureContinuum's fill_fraction to/from time_elapsed
    functions including the tabulated versions are basically the same as
    TrapInstantCaptureContinuum's.
+ It would be good to quantify the speed effects of, well, everything, but
    especially things like the scaling with express or the number of traps, and
    the different ROE options like empty_traps_for_first_transfers that require
    extra steps to be modelled.
-->

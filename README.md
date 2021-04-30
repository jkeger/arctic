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



Contents
--------
+ Installation
+ Usage
+ Unit Tests
+ Documentation
    + Files
    + Add/remove CTI
    + CCD
    + ROE
    + Trap species
    + Trap managers
+ Examples
+ Python Wrapper



Installation
============
Compile the main code with `make` (or `make all`) in the root directory. 

See the `makefile` header documentation for all the options.

See the Wrapper section for the python wrapper.



Usage
=====
ArCTIC will normally be used via a wrapper, but the code can be executed 
directly as `./arctic` with the following command-line options:

+ `-h`, `--help`  
    Print help information and exit.
+ `-v <int>`, `--verbosity=<int>`  
    The verbosity parameter to control the amount of printed information:
    + `0`   No printing (except errors etc).
    + `1`   Standard.
    + `2`   Extra details.
+ `-c`, `--custom`  
    Execute the custom code in the `run_custom_code()` function at the very
    top of `src/main.cpp`. A good place to run your own quick tests or use 
    arctic without any wrappers. The demo version adds then removes CTI from a 
    test image.



Unit Tests
==========
Tests are included for most individual parts of the code, organised with Catch2.

As well as making sure the code is working correctly, most tests are intended to
be relatively reader-friendly examples to help show how all the pieces of the 
code work if you need to understand the inside details as a developer, alongside
the more user-focused documentation.

Compile the tests with `make test` (or `make all`) in the top directory, then 
run with `./test_arctic`.

Add arguments to select which tests to run by their names, e.g:
+ `*'these ones'*`  All tests that contain 'these ones'.
+ `~*'not these'*`  All tests except those that contain 'not these'.
+ `-# [#filename]`  All tests in filename.cpp.

Compiling with `make lib_test` will create a simple test of using the shared 
object library (`build/libarctic.so`), which is run with `./wrapper/lib_test`.



Documentation
=============
The code docstrings contain the full documentation for each class and function. 

This section provides an overview of the key contents and features. It is aimed 
at general users with a few extra details for anyone wanting to navigate or work
on the code itself.

The primary functions to add and remove CTI take as arguments custom objects 
that describe the trap species, CCD properties, and ROE details (see below). 
A core aspect of the code is that it includes several polymorphic versions of 
each of these classes. These provide a variety of ways to model CTI, such as 
different types of trap species, multiple phases in each pixel, or alternative 
readout sequences for trap pumping, etc.



Files
-----
A quick summary of the code files and their contents:

+ `arctic`, `test_arctic`   The program and unit-test executables.
+ `makefile`                The makefile for compiling the code. See its header.
+ `libarctic.so`            The shared object library.
+ `src/`                    Source code files.
    + `main.cpp`  
        Main program. See above and its documentation for the command-line 
        options, and see `run_custom_code()` for an example of running manually 
        editable code directly.
    + `cti.cpp`  
        Contains the primary user-facing functions `add_cti()` and 
        `remove_cti()`. These are wrappers for `clock_charge_in_one_direction()`
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
        `n_electrons_released_and_captured()`, called from 
        `clock_charge_in_one_direction()` to model the capture and release of
        electrons and track the trapped electrons using the "watermarks".
    + `util.cpp`  
        Miscellaneous internal utilities.
+ `include/`    The `*.hpp` header files for each source code file.
+ `test/`       Unit tests and examples.
+ `build/`      Compiled object and dependency files.
+ `wrapper/`    The python, Cython, and other files for the wrapper (see below).



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
practice, just 2 or 3 iterations are usually sufficient.

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
arguments for each are not provided (or `nullptr`).

Note that instead of actually moving the charges past the traps in each pixel,
as happens in the real hardware, the code tracks the occupancies of the traps 
(see Watermarks below) and updates them by scanning over each pixel. This 
simplifies the code structure and keeps the image array conveniently static.

### Express
As described in more detail in Massey et al. (2014) section 2.1.5, the effects 
of each individual pixel-to-pixel transfer can be very similar, so multiple 
transfers can be computed at once for efficiency.

This allows faster computation with a mild decrease in accuracy. 

For example, the electrons in the pixel closest to the readout have only one 
transfer, those in the 2nd pixel undergo 2, those in the 3rd have 3, and so on.
The `express` input sets the number of times the transfers are calculated. 
`express = 1` is the fastest and least accurate, `express = 2` means the 
transfers are re-computed half-way through the readout, up to `express = N` 
where `N` is the total number of pixels for the full computation of every step
without assumptions.

The default `express = 0` is a convenient input for automatic `express = N`.

### Offsets and windows
To account for additional transfers before the first image pixel reaches the 
readout (e.g. a prescan region), the `offset` sets the number of extra pixels.

Somewhat similarly, instead of adding CTI to the entire supplied image, only a 
subset of pixels can be selected using the `window_start` and `_stop` arguments.
Note that, because of edge effects, the range should be started several pixels 
before the actual region of interest.



CCD
---
How electrons fill the volume inside each (phase of a) pixel in the charged-
coupled device (CCD) detector.

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
for the full documentation, including illustrations of the multiphase clocking 
sequences.

### Express matrix  
The `ROE` class also contains the `set_express_matrix_from_pixels_and_express()` 
function used to generate the array of express multipliers.

### Multiple phases
Like the CCD, the `ROE` object can model single or multiple steps in the clock 
sequence for each transfer. The number of steps in a clocking sequence is 
usually same as the number of phases, but not necessarily, as is the case for 
trap pumping.

The number of phases must be the same for the CCD and ROE objects. 



Trap species
------------
The parameters for a trap species.

See the `Trap` and child class docstrings in `traps.cpp` for the full 
documentation.

### Standard (WIP)
Combined release and capture, allowing for instant or non-zero capture times,
following Lindegren (1998) section 3.2.

### Instant capture
For the simpler algorithm of release first then instant capture. This is the 
primary model used by previous versions of ArCTIC.



Trap managers
-------------
This is not relevant for typical users, but is key to the internal structure of 
the code and the "watermark" approach to tracking the trap occupancies.

The different trap manager child classes also implement the different algorithms
required for the corresponding types of trap species described above, primarily 
with the `n_electrons_released_and_captured()` method.

See the `TrapManager` and child class docstrings in `trap_managers.cpp` for the 
full documentation.

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
watermark level. Note that the stored volumes give the size of that level, not 
the cumulative total.

The details can vary for different trap managers.

The unit tests in `test/test_trap_managers.cpp`, especially those for release 
and capture, contain simple-number examples to demonstrate how it all works.



Examples
========
See `run_custom_code()` in `src/main.cpp` for a short and simple example of 
using the core features of arctic to add and then remove CTI from a test image.



Python Wrapper (WIP)
==============

The shared library must first be compiled with `make lib` (or `make all`).

Download the debug test image here:  
http://astro.dur.ac.uk/~cklv53/files/hst_acs_10_col.txt


Set up
------
+ `cd wrapper/`
+ `python3 setup.py build_ext --inplace`
+ Test: `python3 test_wrapper.py`


Files
-----
In `wrapper/`:

+ `lib_test.cpp`  
    A simple (just C++) test of using the library. Compile with 
    `make lib_test` and run with `./wrapper/lib_test`.
+ `test_wrapper.py`  
    A simple test and demo of using the compiled wrapper.
+ `setup.py`  
    The file for compiling the package.
+ `interface.cpp`, `interface.hpp`  
    The source and header files for functions to help interface between the 
    main C++ and Cython.
+ `wrapper.pyx`  
    The Cython wrappers for the C++ functions.
+ `build/`, `wrapper.cpp`, `wrapper.*.so`  
    Compiled output files.

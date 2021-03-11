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



Installation
============
Compile the main code with `make` (or `make arctic` or `make all`) in the top 
directory.

Run with `./arctic`.



Usage
=====
ArCTIC will normally be used via a wrapper, but the code can be executed 
directly with the following command-line options:

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
+ `-# [#filename]`  All tests from the file 'somefile.cpp'.



Documentation
=============
The code docstrings contain the full documentation for each class and function. 

This section provides an overview of the key contents and features. It is aimed 
at general users with a few extra details for anyone wanting to navigate or work
on the code itself.


Files
-----
A quick summary of the code files and their contents:

+ `arctic`, `test_arctic`   The program and unit-test executables.
+ `makefile`                The makefile for compiling the code.
+ `src/`                    Source code files.
    + `main.cpp`  
        Main program. See above and its documentation for the command-line 
        options, and see `run_custom_code()` for an example of running manually 
        editable code directly.
    + `cti.cpp`  
        Contains the primary user-facing functions `add_cti()` and 
        `remove_cti()`. These are wrappers for `clock_charge_in_one_direction()`, which contains the primary nested for loops over an image to add CTI to
        (in order) each column, each express pass (see below), and each row.
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
+ `include/`                The `*.hpp` header files for each source code file.
+ `test/`                   Unit tests and examples.



Add/remove CTI
--------------
### Add CTI
To add (and remove) CTI trails, the primary inputs are the initial image 
followed by the properties of the CCD, readout electronics (ROE), and trap 
species, for either or both parallel and serial clocking.

These parameters are set using the `CCD`, `ROE`, and `Trap` classes, as 
described below.

See `add_cti()`'s docstring in `main.cpp` for the full details.


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


### Remove CTI
Removing CTI trails is done by iteratively modelling the addition of CTI, as 
described in Massey et al. (2010) section 3.2 and Table 1.

The `remove_cti()` function takes all the same parameters as `add_cti()`
plus the number of iterations for the forward modelling.

More iterations provide higher accuracy at the cost of longer runtime. In 
practice, just 2 or 3 iterations are usually sufficient.



CCD
---
How electrons fill the volume inside each (phase of a) pixel.

See the `CCD` class docstring in `ccd.cpp` for the full documentation.

By default, charge is assumed to move instantly from one pixel to another.



ROE
---
The properties of readout electronics (ROE) used to operate a CCD.

See the `ROE` and child class docstrings in `roe.cpp` for the full documentation.


### Express matrix  
The `ROE` class also contains the `set_express_matrix_from_pixels_and_express()` 
function used to generate the array of express multipliers.



Trap species
------------
The parameters for a trap species.

See the `Trap` and child class docstrings in `traps.cpp` for the full documentation.


### Default trap species (WIP)
Combined release and capture, allowing for instant or non-zero capture times,
following Lindegren (1998) section 3.2.


### Instant capture
For the simpler algorithm of release first then instant-capture. This is the 
primary model used by previous versions of ArCTIC.



Trap managers
-------------
This is not relevant for typical users, but is key to the internal structure of 
the code and the "watermark" approach to tracking the trap occupancies.

The different trap managers also implement the different algorithms required for
the corresponding types of trap species described above, primarily with their 
`n_electrons_released_and_captured()` method.

See the `TrapManager` and child class docstrings in `trap_managers.cpp` for the 
full documentation.


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

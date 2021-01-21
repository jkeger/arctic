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
+ Documentation
    + Files
+ Examples


Installation
============
Compile the main code with `make` (or `make arctic` or `make all`) in the top 
directory.

Run with `./arctic`.


Unit Tests
==========
Tests are included for most individual pieces of the code, along with more 
example-style tests, organised with Catch2.

Compile the tests with `make test` (or `make all`) in the top directory, then 
run with `./test_arctic`.


Documentation
=============

Files
-----



Examples
========

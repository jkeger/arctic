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

Add tag arguments to select which tests to run, e.g:
+ `thisTestOnly`        Matches the test case called, 'thisTestOnly'
+ `"this test only"`    Matches the test case called, 'this test only'
+ `these*`              Matches all cases starting with 'these'
+ `exclude:notThis`     Matches all tests except, 'notThis'
+ `~notThis`            Matches all tests except, 'notThis'
+ `~*notThese*`         Matches all tests except those that contain 'notThese'
+ `a* ~ab* abc`         Matches all tests that start with 'a', except those that
                        start with 'ab', except 'abc', which is included
+ `-# [#somefile]`      Matches all tests from the file 'somefile.cpp'


Documentation
=============

Files
-----



Examples
========

Synopsis
========
This is a signal processing library written in c (hence csignal) in a cross-platform fashion.

The goal of this project is to provide signal processing functionality to external applications in a cross-platform (e.g. Windows, Mac OS X, iOS, and Android) and cross-architecture (e.g. x86, x64, ARM, ARM64) fashion.

It should be noted that while this project is marked as created and authored (at least initially) by Brent Carrara, most of the algorithms in this project are implemented from other sources, which are referenced in the header files where their API prototypes are defined.

Code Example
============

Motivation
==========

Installation
============
Cmake is required to build this project and furthermore the following other libraries are required as well:

- cpcommon
- darwinhelper

API Reference
=============
Using Cmake Doxygen style documentation can be generate by executing the 'make doc' command.

Tests
=====
Full API tests are available on all platforms and are supported using Python and the unittest framework. To execute tests run the 'make test' command in the build directory.

Contributors
============
Brent Carrara (bcarr092@uottawa.ca)

License
=======
A licensing model needs to be established for this library.

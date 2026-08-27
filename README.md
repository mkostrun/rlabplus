# lib.so

**_RLaB3_** is an interactive, interpreted scientific programming environment for Linux operating system. 
It allows fast prototyping and script development. 
Its syntax is a combination of c (curly brackets for blocks of statement, some statements) and M**LAB and fortran (e.g., indexing of arrays starts from 1).
The project 
[**_RLaB3_**](https://sourceforge.net/projects/rlabplus) 
provides a syntax update with some internal and organizational changes compared to 
[**_RLaB2_**](https://rlab.sourceforge.net), as developed by Ian Searle.
The projects support 32- and 64-bit linux systems, for AMD, Intel and arm/RaspberryPi.

The solvers that are part of **lib.so** sub-project are here because they require additional special topic shared libraries to be installed by the user. These special topic libraries need not be part of standard linux distribution (e.g., gpib), are too specific to be installed by default when installing linux (ngpsice, gts, gphoto2, glpk), or have a branch provided together with the rlab shared library (matio, kripto) where the library itself is built as .a and then embedded in the rlab's shared object library in the final form.

The following solver libraries are provided:

- _gpib_ - GPIB (Hewlett-Packard's General Purpose Interface Bus, for those who still use it and remember it)
for hardware and instrumentation control: **_RLaB2_**/**_RLaB3_** wrapper for [linux-gpib](https://sourceforge.net/projects/linux-gpib).

- _glpk_ - wrapper to [GNU linear programming kit](https://www.gnu.org/software/glpk) system library for linear programming;

- _gphoto2_ - wrapper to [gphoto2](http://gphoto.org) system library, to provide camera control and image download for usb connected cameras;

- _gts_ - an extension of [GNU triangulation library](https://gts.sourceforge.net), which provides mesh creation and computations.

- _kripto_ - provides cryptography functions from [libkripto](https://github.com/LightBit/libkripto).

- _matio_ - MATLAB /mostly input/only basic output/ library: if you are wondering about the asymmetry, well, since 2010's **_RLaB2_**/**_RLaB3_**
support input/output using HDF5 for all its data structures (except _cell_ that was added in 2018).
As MATLAB _mat_ files format is an abominable obfuscation of HDF5, advanced users should not have problems importing HDF5 files into it.

- _ngspice_ - access to modeling of electronic/thermal circuits using [_ngspice_](https://sourceforge.net/projects/ngspice) : create circuit scripts, executing them using a multiprocessor ngspice interpreter, with computation results returned as internal variables. The only caveat of the approach is that _ngspice_ has to be built as a shared library: Fear not, however, the library supports linux systems with _ngspice_ executable only, as well, but then its supports boils down to script creation, calling executable to execute the scripts and write the output in their _raw_ format, which can be post-processed by **_RLaB2_**/**_RLaB3_**.

- as in immutable - weights, this change was forked off official site and absorbed into **_RLaB3_**). In this particular case the home-brewed NN library I developed way back, was removed.

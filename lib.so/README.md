# about rlab

**_RLaB3_** is an interactive, interpreted scientific programming environment for Linux operating system. 
It allows fast prototyping and script development. 
Its syntax is a combination of c (curly brackets for blocks of statement, some statements) and M**LAB and fortran (e.g., indexing of arrays starts from 1).
The project 
[**_RLaB3_**](https://sourceforge.net/projects/rlabplus) 
provides a syntax update with some internal and organizational changes compared to 
[**_RLaB2_**](https://rlab.sourceforge.net), as developed by Ian Searle.
The projects support 32- and 64-bit linux systems, for AMD, Intel and arm/RaspberryPi.

# lib.so

The solvers that are part of **lib.so** sub-project are here because they require additional special topic shared libraries to be installed by the user. These special topic libraries need not be part of standard linux distribution (e.g., gpib), are too specific to be installed by default when installing linux (ngpsice, gts, gphoto2, glpk), or have a branch provided together with the rlab shared library (matio, kripto) where the library itself is built as .a and then embedded in the rlab's shared object library in the final form.

In its rlab form, each of the solver libraries comprise share object library, and a rlab script that loads the shared library and links new rlab functions to it. 

The following solver libraries are provided:

- [_gpib_](https://github.com/mkostrun/rlabplus/tree/main/lib.so/gpib) - GPIB (Hewlett-Packard's General Purpose Interface Bus, for those who still use it and remember it)
for hardware and instrumentation control: **_RLaB2_**/**_RLaB3_** wrapper for [linux-gpib](https://sourceforge.net/projects/linux-gpib).

- [_ngspice_](https://github.com/mkostrun/rlabplus/tree/main/lib.so/ngspice) - wrapper to  [_NGSPICE_](https://sourceforge.net/projects/ngspice)  shared library or executable, for solving electronic circuits. 

- [_grampc_](https://github.com/mkostrun/rlabplus/tree/main/lib.so/grampc) - wrapper to [grampc](https://github.com/grampc/grampc) library, to provide nonlinear model predictive control (MPC) framework for rlab;

- [_cfitsio_](https://github.com/mkostrun/rlabplus/tree/main/lib.so/cfitsio) - wrapper to [CFITSIO](https://heasarc.gsfc.nasa.gov/docs/software/fitsio/) system library, for loading and manipulations of images  using FITS file format;

- [_expat_](https://github.com/mkostrun/rlabplus/tree/main/lib.so/expat) - wrapper to [expat XML parser](https://libexpat.github.io/) sytem library, for loading XML structures into rlab variables;

- [_glpk_](https://github.com/mkostrun/rlabplus/tree/main/lib.so/glpk) - wrapper to [GNU linear programming kit](https://www.gnu.org/software/glpk) system library for linear programming;

- [_gphoto2_](https://github.com/mkostrun/rlabplus/tree/main/lib.so/gphoto2) - wrapper to [gphoto2](http://gphoto.org) system library, to provide camera control and image download for usb connected cameras;

- [_gts_](https://github.com/mkostrun/rlabplus/tree/main/lib.so/gts) - an extension of [GNU triangulation library](https://gts.sourceforge.net), which provides mesh creation and computations.

- [_kripto_](https://github.com/mkostrun/rlabplus/tree/main/lib.so/kripto) - provides cryptography functions from [libkripto](https://github.com/LightBit/libkripto).

- [_matio_](https://github.com/mkostrun/rlabplus/tree/main/lib.so/matio) - wrapper to [MATIO](https://sourceforge.net/projects/matio), MATLAB I/O library. For this library, user has to obtain their own copy of MATIO and build it, then provide *libmatio.a*  for inclusion in rlab shared object library.


 

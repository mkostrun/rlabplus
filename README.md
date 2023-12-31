# RLAB3/RLABPLUS

**_RLaB3_** is an interactive, interpreted scientific programming environment for Linux operating system. 
It allows fast prototyping and script development. 
Its syntax is a combination of c (curly brackets for blocks of statement, some statements) and M**LAB and fortran (e.g., indexing of arrays starts from 1).
The project 
[**_RLaB3_**](https://sourceforge.net/projects/rlabplus) 
provides a syntax update with some internal and organizational changes compared to 
[**_RLaB2_**](https://rlab.sourceforge.net), as developed by Ian Searle.
The projects support 32- and 64-bit linux systems, for AMD, Intel and arm/RaspberryPi.

The project **_RLABPLUS_** adds a number of solvers, input/output operations, and visualization features and connections to other specialized software,
in form of shared object libraries.
Basically, the solvers within the project are aimed at narrow technical field, and might not be of interest to wide audiences. 
For that reason, these solvers were not incorporated into **_RLaB3_** but are, so to speak, free standing.

The project comprise the following solver libraries:

- _gpib_ - GPIB (Hewlett-Packard's General Purpose Interface Bus, for those who still use it and remember it)
for hardware and instrumentation control: **_RLaB2_**/**_RLaB3_** wrapper for [linux-gpib](https://sourceforge.net/projects/linux-gpib).

- _glpk_ - provides solvers from [GNU linear programming kit](https://www.gnu.org/software/glpk).

- _gphoto2_ - provides camera control and image download for usb connected cameras using [gphoto2](http://gphoto.org).

- _gts_ - provides access to [GNU triangulation library](https://gts.sourceforge.net) for meshes creations and computations.

- _kripto_ - provides cryptography functions from [libkripto](https://github.com/LightBit/libkripto).

- _matio_ - MATLAB /mostly input/only basic output/ library: if you are wondering about the asymmetry, well, since 2010's **_RLaB2_**/**_RLaB3_**
support input/output using HDF5 for all its data structures (except _cell_ that was added in 2018).
As MATLAB _mat_ files format is an abominable obfuscation of HDF5, advanced users should not have problems importing HDF5 files into it.

- _ngspice_ - access to modeling of electronic/thermal circuits using [_ngspice_](https://sourceforge.net/projects/ngspice) : create circuit scripts, executing them using a multiprocessor ngspice interpreter, with computation results returned as internal variables. The only caveat of the approach is that _ngspice_ has to be built as a shared library: Fear not, however, the library supports linux systems with _ngspice_ executable only, as well, but then its supports boils down to script creation, calling executable to execute the scripts and write the output in their _raw_ format, which can be post-processed by **_RLaB2_**/**_RLaB3_**.
My doppelgaenger posted code model for [diode](https://github.com/xspice-code-model-diode/xspice-cmdiode) that was aimed at electro-thermal modeling of the component using **_RLaB2_**/**_RLaB3_**.

At random times it may happen that a library is gobbled up by **_RLaB3_**: If that is the case then it will dissappear from this list, and appear as a part of **_RLaB3_**, with the request that the shared library be available for linking.

At some other random times it may happen that the library and its shared library are both gobbled up by **_RLaB3_**: This happened to _fann_ library after I modified it to suit my needs (I allowed NN nodes to have fixed - as in immutable - weights, this change was forked off official site and absorbed into **_RLaB3_**). In this particular case the home-brewed NN library I developed way back, was removed.

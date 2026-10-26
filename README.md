# about rlab

**_RLaB3_** is an interactive, interpreted scientific programming environment for Linux operating system. 
It allows fast prototyping and script development. 
Its syntax is a combination of c (curly brackets for blocks of statement, some statements) and M**LAB and fortran (e.g., indexing of arrays starts from 1).
The project 
[**_RLaB3_**](https://sourceforge.net/projects/rlabplus) 
provides a syntax update with some internal and organizational changes compared to 
[**_RLaB2_**](https://rlab.sourceforge.net), as developed by Ian Searle.
The projects support 32- and 64-bit linux systems, for AMD, Intel and arm/RaspberryPi.

# [lib.so](https://github.com/mkostrun/rlabplus/tree/main/lib.so)

The solvers that are part of **lib.so** sub-project are here because they require additional special topic shared libraries to be installed by the user. These special topic libraries need not be part of standard linux distribution (e.g., gpib), are too specific to be installed by default when installing linux (ngpsice, gts, gphoto2, glpk), or have a branch provided together with the rlab shared library (matio, kripto) where the library itself is built as .a and then embedded in the rlab's shared object library in the final form.

In its rlab form, each of the solver libraries comprise share object library, and a rlab script that loads the shared library and links new rlab functions to it. 

# [lib.r](https://github.com/mkostrun/rlabplus/tree/main/lib.r)

The solvers that are part of **lib.r** sub-project are special topic libraries written using rlab script that need not be loaded at the start of rlab. While these may be installed in system-accessible location, rlab will only load them if specifically instructed to do so.




## ngspice

### 1. building shared object library for rlab

To use NGSPICE with rlab it is necessary to download NGSPICE source from sourceforge, and to build it
for the linux system. The first step in building the library, is to configure it. One has to execute configure
script in NGSPICE source directory. For the latest rlab, the configure script is called with the following parameters below:
```
# export CC="gcc-15 -std=gnu11"
# ./configure \
	--with-ngshared \
	--with-pic  \
	--with-gnu-ld   \
	--libdir=/usr/local/lib64 \
	--includedir=/usr/local/include \
	--enable-nobypass \
	--enable-xspice \
	--enable-cider \
	--enable-pss \
	--enable-ndev \
	--enable-openmp
# make -j12
# sudo make install
```
then go to the directory where the rlab library is being built (default $HOME/rlab/lib.so/ngspice), and there execute
```
# ./rmake
```
To use the library, put the line
```
rfile libngspice.so.r3
```
at the beginning of your rlab-3 script If one is to use rlab-2, then file extension would be
```
>> rfile libngspice.so.r
```
and the provided file need be renamed.

### 2. NGSPICE in rlab

Check installed library by typing
```
>> ngspice
ans = 
	circuit	cmd		exec	exit		fork		
	getvals	init	isrunning	runckt
>>
```
The functions from the list ngspice are discussed in the manual *ngspice-manual.pdf* in **doc/** subdirectory.

**rlab/** subdirectory contains number of scripts organized either as individual examples, or in further subdirectories like projects on their own.

### 3. Example Script

Consider an [electronic circuit](/art/isrc_1.png)


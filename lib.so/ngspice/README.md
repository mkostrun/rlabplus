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

![electronic circuit](/lib.so/ngspice/art/isrc_1.png)

Consider an electronic circuit shown in the figure above. To make things more
interesting, we assume that *i1* source is external.

An **rlab** script that would solve that circuit is as follows, see the file *rlab/eg_spice_external_src_2.r3* of which this is an excerpt not showing the plotting.
```
rfile libngspice.so

f = 50;
a = 0.001;
t_per = 1./f;
t_step = t_per / 200;
t_end  = 2 * t_per;

r1val = 10e3;
r2val = 1e3;

spicecir = [ ...
  "* test for external current sources i1 with rlab", ...
  ".parameter r1val=" + num2str(r1val,"%e"), ...
  ".parameter r2val=" + num2str(r2val,"%e"), ...
  "i1 0 1 dc 0 external", ...
  "r1 1 2 'r1val'", ...
  "r2 2 0 'r2val'", ...
  ".ic v(1)=0 v(2)=0", ...
  ".tran" + num2str(t_step," %e") + num2str(t_end," %e"), ...
  ".end", ...
[]];

fn_isrc = function(t,p)
{
  global(pi);
  rval = <<>>;
  rval.i1 = p.a .* sin(2 .* pi .* p.f .* t );
  return rval;
};

p = <<>>;
p.f = f;
p.a = a;

// setup ngspice for external voltage source
_ngspice_setup = <<>>;
_ngspice_setup.stdout = term();
_ngspice_setup.sync = <<>>;
_ngspice_setup.sync.isrc  = fn_isrc;
_ngspice_setup.sync.param = p;
ngspice.init(_ngspice_setup);

ngspice.runckt(spicecir);
s = ngspice.getvals();

```
For comparison, python version of this script with external voltage source is given [here](https://pyspice.fabrice-salvaire.fr/releases/v1.4/examples/ngspice-shared/external-source.html).


















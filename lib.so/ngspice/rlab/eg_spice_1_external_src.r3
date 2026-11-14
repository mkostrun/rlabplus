//
//
//
rfile libngspice.so

NITER=1;

f = 50;
a = 10;
t_per = 1./f;
t_step = t_per / 200;
t_end  = 2 * t_per;

r1val = 10e3;
r2val = 1e3;

spicecir = [ ...
  "* test for external voltage sources with rlab", ...
  ".parameter r1val=" + num2str(r1val,"%e"), ...
  ".parameter r2val=" + num2str(r2val,"%e"), ...
  "v1 1 0 dc 0 external", ...
  "r1 1 2 'r1val'", ...
  "r2 2 0 'r2val'", ...
  ".ic v(1)=0 v(2)=0", ...
  ".tran" + num2str(t_step," %e") + num2str(t_end," %e"), ...
  ".end", ...
[]];

spicecir' ?

fn_vsrc = function(t,p)
{
  global(pi);

  rval = <<>>;

  rval.v1 = p.a .* sin(2 .* pi .* p.f .* t );

  return rval;
};

p = <<>>;
p.f = f;
p.a = a;

//
// setup ngspice for external voltage source
//
_ngspice_setup = <<>>;
_ngspice_setup.stdout = term();
_ngspice_setup.sync = <<>>;
_ngspice_setup.sync.vsrc  = fn_vsrc;
_ngspice_setup.sync.param = p;

ngspice.init(_ngspice_setup);

printf("Running: ");
tic();
for (i in 1:NITER)
{
  spinner();
  ngspice.runckt(spicecir);
  s = ngspice.getvals();
}
printf("Done! Run %g times took %g seconds.\n", NITER, toc());
ngspice.exit();



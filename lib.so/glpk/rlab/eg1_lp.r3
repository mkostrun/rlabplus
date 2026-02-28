//
// lp_simplex.r: test of the GNU Linear Programming Kit, v.4.9
//
rfile libglpk.so

if (!exist(NITER))
{ NITER=1000; }

lpopts = <<>>;
// lpopts.stdout  = stderr();

//
// load problem from file and solve it
//
for (i in 1:NITER)
{
  y0 = _glpk_read ("plan.lpt");
}

for(i in 1:NITER)
{
  for (m in ["primal", "dual", "dualp", "interior"])
  {
    spinner();
    lpopts.method  = m;
    ys = _glpk_solve(y0, lpopts);
  }
}




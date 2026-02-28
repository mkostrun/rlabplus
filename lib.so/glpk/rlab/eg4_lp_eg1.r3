//
//
//
if (!exist(_glpk_solve))
{
  printf("Linear programming solvers not installed!\n");
  EOF
}

if (!exist(NITER))
{ NITER=100; }

//
// load problem from file and solve it
//
for (i in 1:NITER)
{
  y0 = lp("plan.lpt");
}


for(i in 1:NITER)
{
  for (m in ["primal", "dual", "dualp", "interior"])
  {
    spinner();
    ys = y0.solve(<<method=m>>);
  }
}




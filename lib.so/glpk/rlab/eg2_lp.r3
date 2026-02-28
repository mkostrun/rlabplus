//
// lp_simplex.r: test of the GNU Linear Programming Kit, v.4.9
//
if (!exist(_glpk_solve))
{
  printf("Linear programming solvers not installed!\n");
  EOF
}

if (!exist(NITER))
{ NITER=1000; }

lpopts = <<>>;
//lpopts.stdout = "./testme.txt";
lpopts.stdout = rconsole();

y0 = <<>>;
y0.objective    = [10,6,4];               // cost function
y0.constraints  = [1,1,1; 10,4,5; 2,2,6]; // constraint matrix
y0.bounds_col   = [0,inf(); 0,inf(); 0,inf()];           // column (structural) bounds
y0.bounds_row   = [-inf(),100; -inf(),600; -inf(),300];  // row (auxiliary) bounds
y0.opt_direction = "max";
y0.problem       = "lp";

s = <<>>;
for (m in ["primal", "dual", "dualp", "interior"])
{
  lpopts.method = m;
  tic();
  for(i in 1:NITER)
  {
    spinner();
    s.[ m ] = _glpk_solve(y0, lpopts);
  }
  printf("GLPK solver %10s : time needed for %s problems is %s sec.\n", ...
      "'" + m + "'", num2str(NITER,"%g"), num2str(toc(),"%g"));
}

printf("Solution =\n");

for (m in ["primal", "dual", "dualp", "interior"])
{
  printf("Method %10s solution = (%s)\n", "'" + m + "'", num2str(s.[m].coef_col,"%g",","));
}

_glpk_write(y0,"test_lp2.lpt");
_glpk_write(y0,"test_lp2.mps");



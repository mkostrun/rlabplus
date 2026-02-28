 
//
// Bryson :
//
//    dx/dt = ffct(t,x,u)
//
static(DEBUG);
DEBUG =1;
rfile libgrampc.so

//
// define list of functions for the optimization solver GRAMPC
//
optim_fns = <<>>;
optim_fns.ffct = function(t, x, u, s)
{
  if (DEBUG)
  {
    "ffct:\n"?
  }
  rval = [ ...
      x[2]; ...
      u; ...
  []];
  return rval;
};
// ODE: (df/dx)_{i,j} = d (f_i) / d(x_j)
optim_fns.dfdx = function(t, x, u, s)
{
  if (DEBUG)
  {
    "dfdx:\n"?
  }
  rval = [ ...
      0, 1; ...
      0, 0; ...
  []];
  return rval;
};
// ODE: (df/du)_{i,j} = d (f_i) / d(u_j)
optim_fns.dfdu = function(t, x, u, s)
{
  if (DEBUG)
  {
    "dfdu:\n"?
  }
  rval = [ ...
      0; ...
      1; ...
  []];
  return rval;
};
// COST: subintegral 
optim_fns.lfct = function(t, x, x_des, u, u_des, s)
{
  // s:
  //  s.x[1:2]
  //  s.u[1]
  //  s.v[1:2]
  //  s.h[1:4]
  rval = 0.5 .* (u).^2;
  return rval;
};
// COST: subintegral 
optim_fns.dldu = function(t, x, x_des, u, u_des, s)
{
  // s:
  //  s.x[1:2]
  //  s.u[1]
  //  s.v[1:2]
  //  s.h[1:4]
  rval = u;
  return rval;
};
// CONSTRAINT: inequalities
optim_fns.hfct = function(t, x, u, s)
{
  // s:
  //  s.h[1]
  rval = [ ...
      x[1] - s.h; ...
  []];
  return rval;
};
// CONSTRAINT: inequalities
optim_fns.dhdx = function(t, x, u, s)
{
  // s:
  //  s.h[1]
  //  s.gt[1]
  rval = [ ...
       1,  0; ...
      []];
  return rval;
};
// CONSTRAINT: inequalities
optim_fns.gtfct = function(T, x, s)
{
  // s:
  //  s.h[1]
  //  s.gt[1]
  rval = [ ...
      x[1]; ...
      x[2] - s.gt; ... 
  []];
  return rval;
};
// CONSTRAINT: inequalities
optim_fns.dgtdx = function(T, x, s)
{
  // s:
  //  s.h[1]
  //  s.gt[1]
  rval = [ ...
      1, 0; ...
      0, 1; ... 
  []];
  return rval;
};

// problem parameters:
//  passed directly to the functions in the list
s = <<>>;
s.h  = 0.1;
s.gt = -1;
// time
dt = 1/1024;
t0 = 0;
tf = 2;
tm = [t0:tf:dt];

// state
x0 = [0, 1];
// control:
u0 = [0];

// options for the solver
opts = <<>>;
opts.x0_des = [0, 0];
opts.u0_des = zeros(u0);
opts.Thor = 1;
opts.Nhor = 50;
opts.Tmax = 10;
opts.Tmin = 0.1;
opts.MaxGradIter = 250;    // Maximum number of gradient iterations
opts.MaxMultIter = 1;      // Maximum number of augmented Lagrangian iterations
opts.PenaltyIncreaseFactor = 1.5;
opts.PenaltyIncreaseThreshold = 0.75;
opts.PenaltyDecreaseFactor = 1.0;
opts.PenaltyMin = 2e4;
opts.PenaltyMax = 1e7;
opts.ConvergenceGradientRelTol = 1e-8;
opts.ConstraintsAbsTol = 1e-6;

tic();
y = grampc.solve(optim_fns, s, tm, x0, u0, opts);
printf("Optimization took %g sec\n", toc());







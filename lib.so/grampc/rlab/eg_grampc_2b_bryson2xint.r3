/* This file is part of rlabplus/GRAMPC - (https://www.github.com/mkostrun/rlabplus/lib.so/grampc/)
 *  grampc:   https://sourceforge.net/projects/grampc/
 *  rlabplus: https://sourceforge.net/projects/rlabplus/
 *
 * GRAMPC -- A software framework for embedded nonlinear model predictive
 * control using a gradient-based augmented Lagrangian approach
 *
 * Copyright 2014-2019 by Tobias Englert, Knut Graichen, Felix Mesmer,
 * Soenke Rhein, Andreas Voelz, Bartosz Kaepernick (<v2.0), Tilman Utz (<v2.0).
 * All rights reserved.
 *
 * GRAMPC is distributed under the BSD-3-Clause license, see LICENSE.txt
 *
 * This rlab script describes the crane 2D problem from
 * Kapernick, B., Graichen, K.: Model predictive control of an overhead
 * crane using constraint substitution. In: Proc. American Control
 * Conference (ACC), pp. 3973-3978, 2013.
 *
 *                                           _T
 *                                          /
 *      min    J(u,p,T;x0) = V(T,x(T),p) + / l(t,x(t),u(t),p) dt
 *   u(.),p,T                            _/
 *                                      0
 *             .
 *      s.t.   x(t) = f(t0+t,x(t),u(t),p), x(0) = x0
 *             h(x)  <= 0
 *             u_min <= u(t) <= u_max
 *
 */

static(DEBUG);
DEBUG=0;

rfile libgrampc.so

//
// define list of functions for the optimization solver GRAMPC
//
optim_fns = <<>>;
optim_fns.f = function(t, x, u, s)
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
// optim_fns.dfdx = function(t, x, u, s)
optim_fns.f_x = [ ...
      0, 1; ...
      0, 0 ];

// ODE: (df/du)_{i,j} = d (f_i) / d(u_j)
// optim_fns.dfdu = function(t, x, u, s)
optim_fns.f_u = [ ...
      0; ...
      1 ];

// COST: subintegral
optim_fns.l = function(t, x, x_des, u, u_des, s)
{
  if (DEBUG)
  {
    "lfct:\n"?
  }
  // s:
  //  s.x[1:2]
  //  s.u[1]
  //  s.v[1:2]
  //  s.h[1:4]
  rval = 0.5 .* (u).^2;
  return rval;
};

optim_fns.l_x = function(t, x, x_des, u, u_des, s)
{
  return zeros(1,2);
};

optim_fns.l_u = function(t, x, x_des, u, u_des, s)
{
  return (u);
};

// CONSTRAINT: inequalities
optim_fns.h = function(t, x, u, s)
{
  // s:
  //  s.h[1]
  rval = [ ...
      x[1] - s.h; ...
  []];
  return rval;
};

optim_fns.h_x = function(t, x, u, s)
{
  return [1,0];
};

optim_fns.h_u =  function(t, x, u, s)
{
  return 0;
};

// CONSTRAINT: equalities
optim_fns.gtfct = function(T, x, s)
{
  if (DEBUG)
  {
    "gtfct:\n"?
    [T,x]?
  }
  // s:
  //  s.h[1]
  //  s.gt[1]
  rval = [ ...
      x[1]; ...
      x[2] - s.gt; ...
  []];
  //rval?
  return rval;
};
// CONSTRAINT: equalities
//optim_fns.dgtdx = function(T, x, s)
optim_fns.dgtdx = [ ...
      1, 0; ...
      0, 1  ];

// problem parameters:
//  passed directly to the functions in the list
s = <<>>;
s.h  = 0.1;
s.gt = -1;

// time
dt = 1e-3;
t0 = 0;
// horizon:
thor = 1;
tmax = 10;
tmin = 0.1;
nhor = 50;


// state
x0 = [0, 1];
// control:
u0 = [0];

// options for the solver
opts = <<>>;
opts.t0   = t0;
opts.dt   = dt;
opts.Thor = thor;
opts.Tmax = tmax;
opts.Tmin = tmin;
opts.Nhor = nhor;
opts.ShiftControl = 0;
opts.MaxGradIter = 250;     // Maximum number of gradient iterations
opts.MaxMultIter = 1;       // Maximum number of augmented Lagrangian iterations
opts.LineSearchInit = 5e-7; // Line search
opts.PenaltyIncreaseFactor = 1.5;       // Penalties
opts.PenaltyIncreaseThreshold = 0.75;
opts.PenaltyDecreaseFactor = 1.0;
opts.PenaltyMin = 2e4;
opts.PenaltyMax = 1e7;
opts.ConvergenceGradientRelTol = 1e-8; // non-zero value implies convergence check is 'on'
opts.ConstraintsAbsTol = 1e-6;
opts.MaxIter = 2000;

opts.estim_penmin = 1;
//opts.ScaleProblem = 1;
opts.stdout = term();

tic();
y = grampc.solve(optim_fns, s, x0, u0, opts);
printf("Optimization took %g sec\n", toc());


gnuwin(1);
gnulimits (0,thor,-1,1);
gnulimits2(0,thor,-8,2);
gnuxtics (0.1,10);
gnuytics (0.2,5);
gnuy2tics(1,5);
gnuxlabel("Time (s)");
gnuylabel("State Amplitude" ,"Control Amplitude");
gnucmd ("set grid xtics ytics");
gnulegend([...
  "State: x_1", ...
  "x_2", ...
  "Control: u", ...
[]]);
gnuformat([ ...
  "with lines lt 1 lw 2 lc rgb 'red' axes x1y1", ...
  "with lines lt 1 lw 2 lc rgb 'orange' axes x1y1", ...
  "with lines lt 1 lw 2 lc rgb 'blue' axes x1y2", ...
[]]);
gnuplot(<<...
  a1=y.pred.x[;1,2]; ...
  a2=y.pred.x[;1,3]; ...
  b1=y.pred.u; ...
 >>, "./fig/eg_grampc_2b.pdf");

gnuwin(2);
gnulimits (0,thor,,);
gnuxtics (0.1,10);
gnuytics (,);
gnuxlabel("Time (s)");
gnuylabel("Cost");
gnucmd ("set grid xtics ytics");
gnulegend([...
  "Cost", ...
  "Augmented Cost", ...
[]]);
gnuformat([ ...
  "with lines lt 1 lw 2 lc rgb 'red' axes x1y1", ...
  "with lines lt 1 lw 2 lc rgb 'orange' axes x1y1", ...
[]]);
gnuplot(a1=y.pred.j, "./fig/eg_grampc_2b_cost.pdf");









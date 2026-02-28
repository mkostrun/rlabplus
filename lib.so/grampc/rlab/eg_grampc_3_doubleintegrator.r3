//
// Double Integrator :
//
/* This file is part of GRAMPC/rlab interface
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
 *
 *
 *
 *
 *
 *
 *
 * This probfct-file describes a double integrator problem in the 
 * context of shrinking horizon MPC
 *
 *                                           _T
 *                                          /
 *      min    J(u,p,T;x0) = V(T,x(T),p) + / l(t,x(t),u(t),p) dt
 *   u(.),p,T                            _/
 *                                      0
 *             .
 *      s.t.   x_1(t) = x_2(t), x_1(0) = x_1k
 *             .
 *             x_2(t) = u(t),   x_2(0) = x_2k
 *             x(T)  <= x_des
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
optim_fns.ffct = function(t, x, u, s)
{
  // s:
  //  s.l
  //  s.v
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
  // s:
  //  s.l
  //  s.v
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
  // s:
  //  s.l
  //  s.v
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
  //  s.l
  //  s.v
  if (DEBUG)
  {
    "lfct:\n"?
  }
  rval = s.l .* (u - u_des).^2;
  return rval;
};
// COST: subintegral
optim_fns.dldu = function(t, x, x_des, u, u_des, s)
{
  // s:
  //  s.l
  //  s.v
  if (DEBUG)
  {
    "dldu:\n"?
  }
  rval = 2 .* s.l .* (u - u_des);
  return rval;
};
optim_fns.vfct = function(t, x, u, s)
{
  // s:
  //  s.l
  //  s.v
  if (DEBUG)
  {
    "vfct:\n"?
  }
  rval = s.v .* t;
  return rval;
};
optim_fns.dvdt = function(t, x, u, s)
{
  // s:
  //  s.l
  //  s.v
  if (DEBUG)
  {
    "dvdt:\n"?
  }
  rval = s.v;
  return rval;
};
// CONSTRAINT: equalities
optim_fns.gtfct = function(T, x, s)
{
  // s:
  //  s.l
  //  s.v
  rval = [ ...
      x[1]; ...
      x[2]; ...
  []];
  if (DEBUG)
  {
    "gtfct:\n[T,x]="?
    [T,x]?
    "rval="?
    rval?
  }
  return rval;
};
// CONSTRAINT: equalities
optim_fns.dgtdx = function(T, x, s)
{
  // s:
  //  s.l
  //  s.v
  if (DEBUG)
  {
    "dgtdx:\n"?
  }
  rval = [ ...
      1, 0; ...
      0, 1; ... 
  []];
  return rval;
};

// problem parameters:
//  passed directly to the functions in the list
s = <<>>;
s.l = 0.1;
s.v = 1.0;

// state
x0 = [-1, -1];
// control:
u0 = [0];

// options for the solver
opts = <<>>;
opts.xdes = [0, 0];
opts.udes = zeros(u0);
opts.umax =  1;
opts.umin = -1;
opts.Thor = 6.00;
opts.Tmax = 20;

opts.Tmin = 0.001;
opts.Nhor = 50;

opts.t0   = 0;
opts.dt   = 0.01;
opts.Tsim = 10;

// Penalties
opts.MaxGradIter  = 200;    // Maximum number of gradient iterations
opts.MaxMultIter  = 1;      // Maximum number of augmented Lagrangian iterations
opts.ShiftControl = "off";
opts.ConstraintsAbsTol = 1e-3;

// Line search
opts.LineSearchMax = 1e2;
// Input and or parameter optimization
//opts.OptimTime = "on";
opts.OptimTimeLineSearchFactor = 0.35;
// Penalties
opts.PenaltyMin = 1e1;
opts.PenaltyIncreaseFactor = 1.25;
opts.PenaltyDecreaseFactor = 1.0;

// Convergence test
opts.ConvergenceCheck = "on";
opts.ConstraintsAbsTol = 1e-3;
opts.ConvergenceGradientRelTol = 1e-9;

opts.CountTnextLessThanTmin = 2;

opts.MaxIter = 100;
opts.estim_penmin = 1;
//opts.ScaleProblem = 1;
opts.stdout = rconsole();


tic();
y = grampc.solve(optim_fns, s, x0, u0, opts);
printf("Optimization took %g sec\n", toc());

tf = max(y.sol.x[;1]);


gnuwin(1);
gnulimits (0,tf,-2,2);
//gnulimits2(0,tf);
gnuxtics (0.5,5);
gnuytics (0.5,5);
gnuy2tics(0.5,5);
gnuxlabel("Time (s)");
gnuylabel("State Amplitude" ,"Control Amplitude");
gnuylabel("Amplitude");
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
  a1=y.sol.x[;1,2]; ...
  a2=y.sol.x[;1,3]; ...
  b1=y.sol.u; ...
 >>, "./eg_grampc_3.pdf");









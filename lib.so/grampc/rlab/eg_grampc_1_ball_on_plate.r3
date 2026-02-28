//
// Ball on the plate:
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
 * This file describes the single-axis ball on plate problem from
 * Richter, S.: Computational complexity certifcation of gradient methods
 * for real-time model predictive control. Ph.D. thesis, ETH Zürich (2012).
 *  
 *                                           _T
 *                                          /
 *      min    J(u,p,T;x0) = V(T,x(T),p) + / l(t,x(t),u(t),p) dt
 *   u(.),p,T                            _/
 *                                      0
 *             .
 *      s.t.   x(t) = f(t0+t,x(t),u(t),p),
 *             x(0) = x0
 *             x_min <= x(t) <= x_max
 *             u_min <= u(t) <= u_max
 *
 */

rfile libgrampc.so

//
// define list of functions for the optimization solver GRAMPC
//
optim_fns = <<>>;
// ODE: RHS function FFCT
optim_fns.ffct = function(t, x, u, s)
{
  rval = [ ...
      x[2] - 0.04 * u; ...
      -7.01 * u; ...
  []];
  return rval;
};
// ODE: (df/dx)_{i,j} = d (f_i) / d(x_j)
optim_fns.dfdx = function(t, x, u, s)
{
  rval = [ ...
      0, 1; ...
      0, 0; ...
  []];
  return rval;
};
// ODE: (df/du)_{i,j} = d (f_i) / d(u_j)
optim_fns.dfdu = function(t, x, u, s)
{
  rval = [ ...
      -0.04; ...
      -7.01; ...
  []];
  return rval;
};

// COST: subintegral 
optim_fns.lfct = function(t, x, xdes, u, udes, s)
{
  // s:
  //  s.x[1:2]
  //  s.u[1]
  //  s.v[1:2]
  //  s.h[1:4]
  rval = 0.5 .* (sum((x-xdes).^2 .* s.x) + sum(s.u *(u-udes).^2));
  return rval;
};
// COST: subintegral 
optim_fns.dldx = function(t, x, xdes, u, udes, s)
{
  // s:
  //  s.x[1:2]
  //  s.u[1]
  //  s.v[1:2]
  //  s.h[1:4]
  rval = (x-xdes).* s.x;
  return rval;
};
// COST: subintegral 
optim_fns.dldu = function(t, x, xdes, u, udes, s)
{
  // s:
  //  s.x[1:2]
  //  s.u[1]
  //  s.v[1:2]
  //  s.h[1:4]
  rval = s.u *(u-udes);
  return rval;
};

// COST: terminal
optim_fns.vfct = function(t, x, xdes, s)
{
  // s:
  //  s.x[1:2]
  //  s.u[1]
  //  s.v[1:2]
  //  s.h[1:4]
  rval = sum(s.v .* (x-xdes).^2);
  return rval;
};
// COST: terminal
optim_fns.dvdx = function(t, x, xdes, s)
{
  // s:
  //  s.x[1:2]
  //  s.u[1]
  //  s.v[1:2]
  //  s.h[1:4]
  rval = 2 .* (x-xdes) .* s.v;
  return rval;
};

// CONSTRAINT: inequalities
optim_fns.hfct = function(t, x, u, s)
{
  // s:
  //  s.x[1:2]
  //  s.u[1]
  //  s.v[1:2]
  //  s.h[1:4]
  rval = [ ...
      s.h[1] - x[1]; ...
     -s.h[2] + x[1]; ...
      s.h[3] - x[2]; ...
     -s.h[4] + x[2]; ...
  []];
  return rval;
};
// CONSTRAINT: inequalities
optim_fns.dhdx = function(t, x, u, s)
{
  // s:
  //  s.x[1:2]
  //  s.u[1]
  //  s.v[1:2]
  //  s.h[1:4]
  rval = [ ...
      -1,  0; ...
       1,  0; ...
       0, -1; ...
       0,  1; ...
      []];
  return rval;
};

// problem parameters:
//  passed directly to the functions in the list
s = <<>>;
s.x = [100, 10];
s.u =  180;
s.v = [100, 10];
s.h = [-0.2, 0.2, -0.1, 0.1];

// state
x0 = [0.1, 0.01];
// control:
u0 = [0];

// options for the solver
opts = <<>>;
opts.t0 = 0;
opts.dt = 1e-2;
opts.Tsim = 8;
opts.xdes = [-0.2, 0];
opts.udes = zeros(u0);
opts.umax   =  0.0524 .* ones(u0);
opts.umin   = -0.0524 .* ones(u0);
opts.Thor   = 0.3;
opts.Nhor   = 10;
opts.MaxMultIter  = 3;
opts.AugLagUpdateGradientRelTol = 1;
opts.ConstraintsAbsTol = 1.e-3; // dim(hfct), or 1 then applies to each constraint;
opts.PenaltyMin = 0.1;
opts.estim_penmin = 1;
opts.stdout = rconsole();

tic();
y = grampc.solve(optim_fns, s, x0, u0, opts);

printf("Optimization took %g sec\n", toc());

gnuwin(1);
gnulimits (0,tf,-0.6,0.6);
gnulimits2(0,tf,-0.06,0.06);
gnuxtics (1,10);
gnuytics (0.1,5);
gnuy2tics(0.01,5);
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
  a1=y.sol.x[;1,2]; ...
  a2=y.sol.x[;1,3]; ...
  b1=y.sol.u; ...
>>, "./eg_grampc_1.pdf");


gnuwin(2);
gnulimits (0,tf);
gnuxtics (1,10);
gnuytics (0.5,5);
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
gnuplot(a1=y.sol.j, "./eg_grampc_1_cost.pdf");












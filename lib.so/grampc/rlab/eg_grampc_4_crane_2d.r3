//
// Crane 2D :
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
 * This probfct-file describes the crane 2D problem from
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
DEBUG = 0;
rfile libgrampc.so

EG = 4;

static(G);
G = mks.g;

//
// define list of functions for the optimization solver GRAMPC
//
optim_fns = <<>>;
optim_fns.ffct = function(t, x, u, s)
{
  rval = [ ...
      x[2]; ...
      u[1]; ...
      x[4]; ...
      u[2]; ...
      x[6]; ...
      -((G * sin(x[5]) + cos(x[5])*u[1] + 2 * x[4] * x[6]) / x[3]);
  []];
  if (DEBUG)
  {
    "ffct:\n"?
    rval?
  }
  return rval;
};
// ODE: (df/dx)_{i,j} = d (f_i) / d(x_j)
optim_fns.dfdx = function(t, x, u, s)
{
  sinx5 = sin(x[5]);
  cosx5 = cos(x[5]);
  rval = [ ...
      0, 1, 0, 0, 0, 0; ...
      0, 0, 0, 0, 0, 0; ...
      0, 0, 0, 1, 0, 0; ...
      0, 0, 0, 0, 0, 0; ...
      0, 0, 0, 0, 0, 1; ...
      0, 0, ...
        (G*sinx5 + cosx5*u[1] + 2 * x[4] * x[6]) / (x[3].^2), ...
          2 * x[6] ./ x[3], ...
            -((G * cosx5 - sinx5 * u[1]) / x[3]), ...
              -2 * x[4] ./ x[3]; ...
  []];
  if (DEBUG)
  {
    "dfdx:\n"?
    rval ?
  }
  return rval;
};
// ODE: (df/du)_{i,j} = d (f_i) / d(u_j)
optim_fns.dfdu = function(t, x, u, s)
{
  rval = [ ...
      0, 0; ...
      1, 0; ...
      0, 0; ...
      0, 1; ...
      0, 0; ...
      -cos(x[5]) ./ x[3], 0; ...
  []];
  if (DEBUG)
  {
    "dfdu:\n"?
    rval?
  }
  return rval;
};
// COST: subintegral 
optim_fns.lfct = function(t, x, x_des, u, u_des, s)
{
  // s:
  //  s.cost_x[1:6]
  //  s.cost_u[1:2]
  //  s.h[1:3]
  rval = sum(s.cost_x .* (x - x_des).^2) + sum(s.cost_u .* (u - u_des).^2);
  if (DEBUG)
  {
    "lfct:\n"?
    rval?
  }
  return rval;
};
// COST: subintegral 
optim_fns.dldx = function(t, x, x_des, u, u_des, s)
{
  // s:
  //  s.cost_x[1:6]
  //  s.cost_u[1:2]
  //  s.h[1:3]
  rval = 2 .* s.cost_x .* (x - x_des);
  if (DEBUG)
  {
    "dldx:\n"?
    rval?
  }
  return rval;
};
// COST: subintegral 
optim_fns.dldu = function(t, x, x_des, u, u_des, s)
{
  // s:
  //  s.cost_x[1:6]
  //  s.cost_u[1:2]
  //  s.h[1:3]
  rval = 2 .* s.cost_u .* (u - u_des);
  if (DEBUG)
  {
    "dldu:\n"?
    rval?
  }
  return rval;
};
// CONSTRAINT: inequalities
optim_fns.hfct = function(t, x, u, s)
{
  // s:
  //  s.cost_x[1:6]
  //  s.cost_u[1:2]
  //  s.h[1:3]
  rval = [ ...
      cos(x[5])*x[3] - s.h[1]*(x[1]+sin(x[5])*x[3]).^2 - s.h[2]; ...
       x[6] - s.h[3]; ...
      -x[6] - s.h[3]; ...
  []];
  if (DEBUG)
  {
    "hfct:\n"?
    rval?
  }
  return rval;
};
// CONSTRAINT: inequalities
optim_fns.dhdx = function(t, x, u, s)
{
  // s:
  //  s.cost_x[1:6]
  //  s.cost_u[1:2]
  //  s.h[1:3]
  sinx5 = sin(x[5]);
  cosx5 = cos(x[5]);
  rval = [ ...
      -2 .* s.h[1]*(x[1]+sinx5)*x[3], ...
          0, ...
            cosx5 - 2 .* sinx5 .* s.h[1]*(x[1]+sinx5*x[3]), ...
                0, ...
                    -sinx5*x[3] - 2 .* s.h[1]*(x[1]+sinx5*x[3]) .* x[3] .* cosx5, ...
                        0; ...
      0, 0, 0, 0, 0,  1; ...
      0, 0, 0, 0, 0, -1; ...
  []];
  if (DEBUG)
  {
    "hfct:\n"?
    rval?
  }
  return rval;
};
// CONSTRAINT: inequalities
optim_fns.dhdu = function(t, x, u, s)
{
  // s:
  //  s.cost_x[1:6]
  //  s.cost_u[1:2]
  //  s.h[1:3]
  rval = [ ...
      0, 0; ...
      0, 0; ...
      0, 0; ...
  []];
  if (DEBUG)
  {
    "dhdu:\n"?
    rval?
  }
  return rval;
};

// problem parameters:
Tsim = 10;
//  passed directly to the functions in the list
s = <<>>;
s.cost_x = [1,2,2,1,1,4];
s.cost_u = [0.05, 0.05];
s.h      = [0.2, 1.25, 0.3];

// state
x0 = [-2.0, 0.0, 2.0, 0.0, 0.0, 0.0];
// control:
u0 = [0, 0];

// options for the solver
opts = <<>>;
opts.xdes = [2.0, 0.0, 2.0, 0.0, 0.0, 0.0];
opts.udes = [0, 0];
opts.umax   = [2, 2];
opts.umin   = [-2,-2];
opts.Thor   = 2.00;
opts.Nhor   = 20;
opts.t0     = 0;
opts.dt     = 0.002;
opts.Tsim   = Tsim;

opts.ConstraintsAbsTol = [1e-4, 1e-3, 1e-3];
opts.estim_penmin = 1;

opts.stdout = rconsole();

tic();
y = grampc.solve(optim_fns, s, x0, u0, opts);
printf("Optimization took %g sec\n", toc());

rfile module_plot_sol




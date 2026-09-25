/* This file is part of rlabplus/GRAMPC - (https://www.github.com/mkostrun/rlabplus/lib.so/grampc/)
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
 * This rlab script describes the helicopter problem from
 * Tondel, P., Johansen, T.A.: Complexity Reduction in Explicit Linear
 * Model Predictive  Control. IFAC Proceedings Volumes 35(1), 189-194 (2002)
 *
 *                                           _T
 *                                          /
 *      min    J(u,p,T;x0) = V(T,x(T),p) + / l(t,x(t),u(t),p) dt
 *   u(.),p,T                            _/
 *                                      0
 *             .
 *      s.t.   x(t) = f(t0+t,x(t),u(t),p), x(0) = x0
 *             x_min <= x(t) <= x_max
 *             u_min <= u(t) <= u_max
 *
 */

static(DEBUG);
DEBUG =1;
rfile libgrampc.so

EG = 7;

// problem parameters:
//  passed directly to the functions in the list
pcost = [0.44, 0.6, 100, 100, 10, 10, 400, 200, 0.001, 0.001, 100, 100, 10, 10, 400, 200];
s = <<>>;
s.x = pcost[11:16];
s.u = pcost[9:10];
s.h = pcost[1,2];

//
// define list of functions for the optimization solver GRAMPC
//
optim_fns = <<>>;
optim_fns.f = <<>>;
optim_fns.f.f_x = [ ...
  0, 0, 1, 0, 0, 0; ...
  0, 0, 0, 1, 0, 0; ...
  0, 0, 0, 0, 0, 0; ...
  0, 0, 0, 0, 0, 0; ...
  1, 0, 0, 0, 0, 0; ...
  1, 0, 0, 0, 0, 0 ];
optim_fns.f.f_u = [ ...
     0,     0; ...
  0.01, -0.01; ...
  0.19,  0.19; ...
  1.32, -1.32; ...
     0,     0; ...
     0,     0 ];

// COST: subintegral
optim_fns.l = <<>>;
optim_fns.l.l_xx = s.x;
optim_fns.l.l_uu = s.u;

// COST: terminal
optim_fns.v = <<>>;
optim_fns.v.v_xx = pcost[3:8];


// CONSTRAINT: inequalities
optim_fns.h = function(t, x, u, s)
{
  // s:
  //  s.h
  rval = [ ...
      -s.h[1].^2 + x[3].^2; ...
      -s.h[2].^2 + x[4].^2; ...
  []];
  return rval;
};
// CONSTRAINT: inequalities
optim_fns.h_x = function(t, x, u, s)
{
  // s:
  //  s.h[1]
  //  s.gt[1]
  rval = [ ...
    0,  0, 2.*x[3],       0, 0, 0; ...
    0,  0,       0, 2.*x[4], 0, 0 ];
  return rval;
};

// time
t0 = 0;
dt = 1/128;
Tsim = 11;
Thor = 3.0;

// state
x0 = [0.5, 0.5, 0, 0, 0, 0];
// control:
u0 = [0, 0];

// options for the solver
opts = <<>>;
opts.t0 = t0;
opts.dt = dt;
opts.Tsim = Tsim;
opts.xdes = zeros(x0);
opts.udes = zeros(u0);
opts.umax =  3.0 .* ones(u0);
opts.umin = -1.0 .* ones(u0);
opts.Thor = Thor;
opts.Nhor = 10;
opts.ConstraintsAbsTol = [1e-3, 1e-3];
opts.stdout = term();

tic();
y = grampc.solve(optim_fns, s, x0, u0, opts);
printf("Optimization took %g sec\n", toc());

rfile module_plot_sol






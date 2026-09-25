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
 * This rlab script describes the mass-spring-damper problem from
 * Kapernick, B.: Gradient-Based Nonlinear Model Predictive Control With
 * Constraint Transformation for Fast Dynamical Systems. Dissertation,
 * Ulm University. Shaker, Aachen, Germany (2016)
 *
 *                                           _T
 *		                            /
 *      min    J(u,p,T;x0) = V(T,x(T),p) + / l(t,x(t),u(t),p) dt
 *   u(.),p,T                            _/
 *                                      0
 *             .
 *      s.t.   x(t) = f(t0+t,x(t),u(t),p), x(0) = x0
 *             u_min <= u(t) <= u_max
 *
 */

static(DEBUG);
DEBUG =1;
rfile libgrampc.so

EG = 8;

// problem parameters:
//  passed directly to the functions in the list
m = 1;
c = 1;
d = 0.002;
psys= [ m, c, d, ...
  1.0,  1.0,  1.0,  1.0,  1.0,  1.0, 1.0, ...
  1.0,  1.0,  1.0, 0.01, 0.01,  1.0, 1.0, ...
  1.0,  1.0,  1.0,  1.0,  1.0,  1.0, 1.0, 1.0 ];
s = <<>>;
s.lx = psys[4:13];
s.lu = psys[14:15];
s.vx = psys[16:25];

//
// define list of functions for the optimization solver GRAMPC
//
optim_fns = <<>>;
optim_fns.f = <<>>;
optim_fns.f.f_x = [ ...
  0, 0, 0, 0, 0, 1, 0, 0, 0, 0; ...
  0, 0, 0, 0, 0, 0, 1, 0, 0, 0; ...
  0, 0, 0, 0, 0, 0, 0, 1, 0, 0; ...
  0, 0, 0, 0, 0, 0, 0, 0, 1, 0; ...
  0, 0, 0, 0, 0, 0, 0, 0, 0, 1; ...
  -2*c/m,    c/m,      0,      0,      0, -2*d/m,    d/m,      0,      0,      0; ...
     c/m, -2*c/m,    c/m,      0,      0,    d/m, -2*d/m,    d/m,      0,      0; ...
       0,    c/m, -2*c/m,    c/m,      0,      0,    d/m, -2*d/m,    d/m,      0; ...
       0,      0,    c/m, -2*c/m,    c/m,      0,      0,    d/m, -2*d/m,    d/m; ...
       0,      0,      0,    c/m, -2*c/m,      0,      0,      0,    d/m, -2*d/m ];
optim_fns.f.f_u = [ ...
     0,     0; ...
     0,     0; ...
     0,     0; ...
     0,     0; ...
     0,     0; ...
   1/m,     0; ...
     0,     0; ...
     0,     0; ...
     0,     0; ...
     0,  -1/m ];

// COST: subintegral
optim_fns.l = <<>>;
optim_fns.l.l_xx = s.lx;
optim_fns.l.l_uu = s.lu;

// COST: terminal
optim_fns.v = <<>>;
optim_fns.v.v_xx = s.vx;

// time
t0 = 0;
dt = 1/256;
Tsim = 22;
Thor = 10.0;

// state
x0 = [ 1, 0, 0, 0, 1, 0, 0, 0, 0, 0 ];
// control:
u0 = [0, 0];

// options for the solver
opts = <<>>;
opts.t0 = t0;
opts.dt = dt;
opts.Tsim = Tsim;
opts.xdes = zeros(x0);
opts.udes = zeros(u0);
opts.umax =  1.0 .* ones(u0);
opts.umin = -1.0 .* ones(u0);
opts.Thor = Thor;
opts.Nhor = 10;
opts.MaxGradIter = 5;
opts.stdout = term();

tic();
y = grampc.solve(optim_fns, s, x0, u0, opts);
printf("Optimization took %g sec\n", toc());

rfile module_plot_sol






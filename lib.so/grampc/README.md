# grampc

GRAMPC stands for "Gradient Method for Model Predictive Control," check 
web site for references and documentation download. 
The rlab implementation follows naming convention of the GRAMPC-software
manual.

The difference between MATLAB wrapper provided with the library and rlab implementation is that here all the functions are rlab-scripted.

## 1. building shared object library for rlab

To use GRAMPC with rlab it is necessary to download its source from github, and to put it into grampc subdirectory in the directory containing rlab sources.
The provided Makefile should be put in grampc subdirectory, and it
contains modification by which the built library _libgrampc.a_ is
moved to rlab source directory (one level up).
For the latest rlab, the build of grampc is done through rmake script which basically does the following:
```
# export CC="gcc-15 -std=gnu11"
# cd grampc
# make clean
# make -j4
# cp 
# cd ..
```
The then from the rlab source directory execute
```
# ./rmake
```
To use the library, put the line
```
rfile libgrampc.so.r3
```
at the beginning of your rlab-3 script If one is to use rlab-2, then file extension would be
```
>> rfile libngspice.so.r
```
and the provided file extension need be renamed (from .r3 to .r)


## 2. GRAMPC in rlab

Check installed library by typing
```
>> grampc
ans = 
	solve
>>
```
For now, the library provides a single function. Maybe in the future, there may be consistency check for various input options, and if needed, the solvers split in two classes: initial value problems, and boundary value problems.

## 3. What does GRAMPC library for rlab offer that other wrappers do not?

## 3.1 Shorthand for functions

Any user of GRAMPC in rlab scripting environment would eventually notice that the solver is slow. This is not suprise, because there are many functions, and each of them is scripted - meaning that the rlab interpreter is quite busy, well, interpreting the scripted functions and calculating the their numerical responses.

But there is better way.

Many functions, particularly their derivatives may happen to be either constants, or linear function(al)s of involved variables. In such cases the library allows user to specify particular entry as a real matrix rather than function. 

Consider an example from _eg_grampc_1_ball_on_plate.r3_, that is, "ball on the plate" problem, and take a look at list entries _hfct_, _dhdx_ as functions, with _dhdu_ being zero:
````
...
// CONSTRAINT: inequalities as functions: GRAMPC formulation
optim_fns.h = function(t, x, u, s)
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
optim_fns.h_x = function(t, x, u, s)
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
...
````
Notice that in this particular case extended to _u_'s, as well, the left-hand-side of the inequality constraints can be written using constant vector for _hfct_ and matrix for _dhdx_ and _dhdu_:
````
    (LHS)_i := (h)_i + (h_x * x)_i + (h_u * u)_i <= 0, for i=1, ... Nh
````
If this is indeed the case for the problem at hand, then the rlab library allow user to specify inequality constraints as matrices rather than as functions:
````
...
// CONSTRAINT: inequalities: GRAMPC for RLAB 
optim_fns.h = <<>>
optim_fns.h.h_0 = [ ...
    s.h[1]; ...
   -s.h[2]; ...
    s.h[3]; ...
   -s.h[4]; ...
[]];
optim_fns.h.h_x = [ ...
    -1,  0; ...
     1,  0; ...
     0, -1; ...
     0,  1; ...
[]];
...
````

This approach works even if the problem statement is reduced in that _h_ is a function, but _h_x_ or _h_u_ are constant. Then, _h_ is defined as a function, but _h_x_ or _h_u_ are provided as matrices. 

As of now, the following shorthands for functions are permitted (Entry can be either function or matrix):

- _f_u_, _f_x_, or  _f=<<f_0, f_x, f_u>>_ with all entries in the sublist constant matrices
- _g_x_, or  _g=<<g_0, g_x, g_u>>_ with all entries in the sublist constant matrices
- _gt_x_, or  _gt=<<gt_0, gt_x, gt_u>>_ with all entries in the sublist constant matrices
- _h_, _h_x_, _h_u_, or _h=<<h_0, h_x, h_u>>_ with all entries in the sublist constant matrices
- _m_ 

### 3.1.1 Cost Function

The cost function _l_ has special treatment. Consider the script _eg_grampc_4_crane_2d.r3_, in which transiliterated from c-lang the functions _l_ and its partial derivatives _l_x_ and _l_u_ read:
````
...
// COST: subintegral
optim_fns.l = function(t, x, x_des, u, u_des, s)
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
optim_fns.l_x = function(t, x, x_des, u, u_des, s)
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
optim_fns.l_u = function(t, x, x_des, u, u_des, s)
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
...
````
Notice that the cost function _lfct_ here may be represented  through two (symmetric and positive definite) matrices, call them _l_xx_ and _l_uu_, of dimensions dim(x) and dim(u), and the mixed derivatives matrix  _l_ux_ respectively,
````
    lfct := 0.5 * (x - xdes)^T * diag(l_xx) * (x - xdes) + 0.5 * (u - udes)^T * diag(l_uu) * (u - udes) 
            + 0.5 * ( (u - udes)^T * l_ux * (x - xdes) + Transpose )
````
Per example above, Rlab allows user to specify the cost and its partial derivatives using only constant matrices,
````
optim_fns.l = <<>>;
optim_fns.l.l_xx = s.cost_x;
optim_fns.l.l_uu = s.cost_u;
````
From this information, Rlab will internally calculate the scalar _lfct_  and the matrices _dldx_ and _dldu_ without need to call the scripted functions.
Rlab allows _l_ux_ to be provided, as well. It will affect the partial derivatives per formula above.

### 3.1.2 Summary of Speed Improvements

Through this flexibility, we report the following speed improvements of example scripts
|  script |  only functions |  matrices were possible |
|---|---|---|
|  eg1 |  0.36 |  0.03 |
|  eg2 |  0.021 |  0.016 |
|  eg3 |  13.0 |  1.0 |
|  eg4 |  8.1 |  7.2 |
|  eg6 |  54.6 |  2.8 |

These timings were obtained on AMD Ryzen 7 5800X 8-Core Processor. User is advised to check the scripts for details.

## 3.2 _xdes_ flexibility

GRAMPC solves optimization problem assuming _xdes_ a constant. rlab allows _xdes_, ahem, to change but only at discrete times. 

Rlab allows user to change desired state of the system _xdes_, where the change occurs at predefined number of breakpoints.
One needs to specify two entries in _opt_ list: _xdes_ for desired states and _bkpt_ for time breakpoints when desired state changes.
These are defined as per following example:
````
// xdes changes at discrete times:
// if t <= bkpt[1]         : xdes = [0.0, 1.0, 1.0]
//    else if t <= bkpt[2] : xdes = [0.5, 0.5, 1.0]
//        else             : xdes = [1.0, 0.0, 1.0]   /* t > bkpt[2]*/
opts.xdes = [ ...
    0.0, 1.0, 1.0; ...
    0.5, 0.5, 1.0; ...
    1.0, 0.0, 1.0 ];
opts.bkpt = [1.0, 2.0];
````
Rlab checks matrix  _xdes_ and vector _bkpt_ for consistency: NR(xdes)=LENGTH(bkpt)+1 is required for calculation to proceed.


## 4. One Example Script


![GRAMPC solution](/lib.so/grampc/art/eg_grampc_1_cost.png)


Consider an example problem from GRAMPC package "Ball on the Plate", which solves an optimization problem of a single-axis ball on plate problem from Richter, S.: Computational complexity certification of gradient methods for real-time model predictive control. Ph.D. thesis, ETH Zürich (2012).

An rlab script that would solve that circuit is as follows, see the file *rlab/eg_grampc_1b_ball_on_plate.r3* of which this is an excerpt not showing the plotting.

```
rfile libgrampc.so
//
// define list of functions for the optimization solver GRAMPC
//
optim_fns = <<>>;
// ODE: RHS function FFCT
optim_fns.f = function(t, x, u, s)
{
  rval = [ ...
      x[2] - 0.04 * u; ...
      -7.01 * u; ...
  []];
  return rval;
};
// ODE: (df/dx)_{i,j} = d (f_i) / d(x_j)
optim_fns.f_x = function(t, x, u, s)
{
  rval = [ ...
      0, 1; ...
      0, 0; ...
  []];
  return rval;
};
// ODE: (df/du)_{i,j} = d (f_i) / d(u_j)
optim_fns.f_u = function(t, x, u, s)
{
  rval = [ ...
      -0.04; ...
      -7.01; ...
  []];
  return rval;
};

// COST: subintegral
optim_fns.l = function(t, x, xdes, u, udes, s)
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
optim_fns.l_x = function(t, x, xdes, u, udes, s)
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
optim_fns.l_u = function(t, x, xdes, u, udes, s)
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
optim_fns.v = function(t, x, xdes, s)
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
optim_fns.v_x = function(t, x, xdes, s)
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
optim_fns.h = function(t, x, u, s)
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
optim_fns.h_x = function(t, x, u, s)
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
opts.stdout = term();

tic();
y = grampc.solve(optim_fns, s, x0, u0, opts);
printf("Optimization took %g sec\n", toc());

```
Above script does not use any shorthand for functions. User is referred to the file *rlab/eg_grampc_1b_ball_on_plate.r3*  which uses all possible shorthands for functions. Results in Summary section confer substantial increase in speed of calculation (9x).


















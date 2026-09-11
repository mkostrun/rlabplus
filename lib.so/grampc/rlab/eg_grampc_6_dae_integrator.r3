// This rlab-file solves the DAE Integrator problem.
//
//                                           _T
//                                          /
//      min    J(u,p,T;x0) = V(T,x(T),p) + / l(t,x(t),u(t),p) dt
//   u(.),p,T                            _/
//                                      0
//             .
//      s.t.   x(t) = f(t0+t,x(t),u(t),p), x(0) = x0
//             h(x)  <= 0
//             u_min <= u(t) <= u_max
//
EG = 6;
rfile libgrampc.so

//
// define list of functions for the optimization solver GRAMPC
//
optim_fns = <<>>;
optim_fns.ffct = function(t, x, u, s)
{
  rval = [ u[1]; u[2]; x[1] + x[2] - x[3] ];  
  return rval;
};

// ODE: (df/dx)_{i,j} = d (f_i) / d(x_j)
optim_fns.dfdx = function(t, x, u, s)
{
  rval = zeros(3,3);
  rval[3;] = [1, 1, -1];
  return rval;
};

// ODE: (df/du)_{i,j} = d (f_i) / d(u_j)
optim_fns.dfdu = function(t, x, u, s)
{
  rval = [...
    1, 0; ...
    0, 1; ...
    0, 0 ];
  return rval;
};

// ODE: (df/dt) = 0
optim_fns.dfdt = function(t, x, u, s)
{
  retrun zero(3,1);
};



// COST: subintegral 
optim_fns.lfct = function(t, x, x_des, u, u_des, s)
{
  // s: (unoptimizable) parameter array
  rval = 0.5*(s[3]*u[1].*u[1] + s[3]*u[2]*u[2] + s[1]*(x[1]-x_des[1]).*(x[1] - x_des[1]) ...
      + s[2]*(x[2] - x_des[2])*(x[2] - x_des[2]) );
  return rval;
};

// COST: subintegral: jacobian x
optim_fns.dldx = function(t, x, x_des, u, u_des, s)
{
  // s: (unoptimizable) parameter array
  rval = [ s[1] * (x[1] - x_des[1]), s[2] * (x[2] - x_des[2]), 0];
  return rval;
};

// COST: subintegral : jacobian u
optim_fns.dldu = function(t, x, x_des, u, u_des, s)
{
  // s: (unoptimizable) parameter array
  rval = [s[3] * u[1], s[3] * u[2]];
  return rval;
};

// TERMINAL COST: 
optim_fns.vfct = function(T, x, x_des, s)
{
  // s: (unoptimizable) parameter array
  rval = 0.5*( s[1]*(x[1]-x_des[1]).^2  + s[2]*(x[2]-x_des[2]).^2 );
  return rval;
};

// TERMINAL COST: jacobian x
optim_fns.dvdx = function(T, x, x_des, s)
{
  // s: (unoptimizable) parameter array
  rval = [ s[1]*(x[1]-x_des[1]), s[2]*(x[2]-x_des[2]), 0];
  return rval;
};

// EQUALITY constraints
optim_fns.gfct = function(t, x, u, s)
{
  // s: (unoptimizable) parameter array
  return (-1 + x[3]);  
};

// EQUALITY constraints: jacobian x
optim_fns.dgdx = function(t, x, u, s)
{
  // s: (unoptimizable) parameter array
  return [0, 0, 1];  
};

// EQUALITY constraints: jacobian u
optim_fns.dgdu = function(t, x, u, s)
{
  // s: (unoptimizable) parameter array
  return [0, 0];
};

optim_fns.mass = function(s)
{
  rval = [...
    1,0,0; ...
    0,1,0; ...
    0,0,0 ];
  return rval;
};

// problem parameters:

s = [500, 0, 1];   
// time
dt = 1/128;
t0 = 0;
tf = 2;
tm = [t0:tf:dt];

// state
x0 = [ 1, 0, 1];
// control:
u0 = [-2, 2];

// options for the solver
opts = <<>>;
opts.stdout = term();

// time:
opts.t0 = 0;
opts.dt = dt;
opts.Tsim = 2.9;


// xdes is a function of time:
// t <= bkpt[1]       : xdes = [0.0, 1.0, 1.0]
//    t <= bkpt[2]    : xdes = [0.5, 0.5, 1.0]
//        t > bkpt[2] : xdes = [1.0, 0.0, 1.0]
opts.xdes = [ ...
    0.0, 1.0, 1.0; ...
    0.5, 0.5, 1.0; ...
    1.0, 0.0, 1.0 ];
opts.bkpt = [1.0, 2.0];

opts.udes = zeros(u0);
opts.umax = [ 2,  2];
opts.umin = [-2, -2];
opts.Thor = 0.4;
opts.Nhor = 30;

opts.MaxGradIter = 10;  // Maximum number of gradient iterations
opts.MaxMultIter = 3;   // Maximum number of augmented Lagrangian iterations
opts.ConstraintsAbsTol = 1e-4;
opts.PenaltyIncreaseFactor = 1.1;
opts.LineSearchMax = 1e-1;

opts.integrator = "rodas";
opts.erel = 1e-4;
opts.eabs = 1e-5;
IFCN = 0;     // 0 --> right hand side independent of time t  
IDFX = 0;     // 0 --> DF/Dt is numerically computed 
IJAC = 0;     // 1(0) -> analytical (numerical) jacobian (partial derivatives of right hand side w.r.t. state) 
IMAS = 1;     // 1 --> mass matrix is supplied 
MLJAC = length(x0); // no. of lower diagonals of jacobian ~	value must be between 1 and NX: NX->full matrix
MUJAC = length(x0); // no. of upper diagonals of jacobian ~	value must be between 1 and NX 
MLMAS = length(x0); // no. of lower diagonals of mass matrix 
MUMAS = length(x0); // no. of upper diagonals of mass matrix 
//opts.flags = [IFCN, IDFX, IJAC, IMAS, MLJAC, MUJAC, MLMAS, MUMAS];
opts.flags = [IFCN, IDFX, IJAC];


tic();
y = grampc.solve(optim_fns, s, x0, u0, opts);
printf("Optimization took %g sec\n", toc());







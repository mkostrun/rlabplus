/* rlabplus_GRAMPC.h: optimal control */

/* This file is a part of rlabplus
   Copyright (C) 2025 M. Kostrun

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   See the file ./COPYING
   ********************************************************************** */
#ifndef RLABPLUS_GRAMPC_H
#define RLABPLUS_GRAMPC_H

//
// GRAMPC:
//
// the following functions can be provided
#define RLAB_NAME_GRAMPC_F      "ffct"
#define RLAB_NAME_GRAMPC_DFDX   "dfdx"
#define RLAB_NAME_GRAMPC_DFDU   "dfdu"
#define RLAB_NAME_GRAMPC_DFDP   "dfdp"
#define RLAB_NAME_GRAMPC_DFDT   "dfdt"
#define RLAB_NAME_GRAMPC_LFCT   "lfct"
#define RLAB_NAME_GRAMPC_DLDX   "dldx"
#define RLAB_NAME_GRAMPC_DLDU   "dldu"
#define RLAB_NAME_GRAMPC_DLDP   "dldp"
#define RLAB_NAME_GRAMPC_VFCT   "vfct"
#define RLAB_NAME_GRAMPC_DVDX   "dvdx"
#define RLAB_NAME_GRAMPC_DVDP   "dvdp"
#define RLAB_NAME_GRAMPC_DVDT   "dvdt"
#define RLAB_NAME_GRAMPC_G      "gfct"
#define RLAB_NAME_GRAMPC_DGDX   "dgdx"
#define RLAB_NAME_GRAMPC_DGDU   "dgdu"
#define RLAB_NAME_GRAMPC_DGDP   "dgdp"
#define RLAB_NAME_GRAMPC_H      "hfct"
#define RLAB_NAME_GRAMPC_DHDX   "dhdx"
#define RLAB_NAME_GRAMPC_DHDU   "dhdu"
#define RLAB_NAME_GRAMPC_DHDP   "dhdp"
#define RLAB_NAME_GRAMPC_GT     "gtfct"
#define RLAB_NAME_GRAMPC_DGTDX  "dgtdx"
#define RLAB_NAME_GRAMPC_DGTDP  "dgtdp"
#define RLAB_NAME_GRAMPC_DGTDT  "dgtdt"
#define RLAB_NAME_GRAMPC_HT     "htfct"
#define RLAB_NAME_GRAMPC_DHTDX  "dhtdx"
#define RLAB_NAME_GRAMPC_DHTDP  "dhtdp"
#define RLAB_NAME_GRAMPC_DHTDT  "dhtdt"
#define RLAB_NAME_GRAMPC_M      "mass"
#define RLAB_NAME_GRAMPC_MTR    "masstr"
// options
#define RLAB_NAME_GRAMPC_OPTS_PARAMS  "p"
#define RLAB_NAME_GRAMPC_OPTS_PARAMS_MIN  "pmin"
#define RLAB_NAME_GRAMPC_OPTS_PARAMS_MAX  "pmax"
#define RLAB_NAME_GRAMPC_OPTS_X0DES   "xdes"
#define RLAB_NAME_GRAMPC_OPTS_U0DES   "udes"
#define RLAB_NAME_GRAMPC_OPTS_UMAX    "umax"
#define RLAB_NAME_GRAMPC_OPTS_UMIN    "umin"
#define RLAB_NAME_GRAMPC_OPTS_THOR    "Thor"
#define RLAB_NAME_GRAMPC_OPTS_THORMIN "Tmin"
#define RLAB_NAME_GRAMPC_OPTS_THORMAX "Tmax"
#define RLAB_NAME_GRAMPC_OPTS_NHOR    "Nhor"
#define RLAB_NAME_GRAMPC_OPTS_MAXMULTITER "MaxMultIter"
#define RLAB_NAME_GRAMPC_OPTS_MAXGRADITER "MaxGradIter"
#define RLAB_NAME_GRAMPC_OPTS_AUGLAGUPDATEGRADIENTRELTOL "AugLagUpdateGradientRelTol"
#define RLAB_NAME_GRAMPC_OPTS_CONSTRAINTSABSTOL   "ConstraintsAbsTol"
#define RLAB_NAME_GRAMPC_OPTS_ESTIMATE_PENMIN     "estim_penmin"
#define RLAB_NAME_GRAMPC_OPTS_PENMIN              "PenaltyMin"
#define RLAB_NAME_GRAMPC_OPTS_PENMAX              "PenaltyMax"
#define RLAB_NAME_GRAMPC_OPTS_PENINCFAC           "PenaltyIncreaseFactor"
#define RLAB_NAME_GRAMPC_OPTS_PENDECFAC           "PenaltyDecreaseFactor"
#define RLAB_NAME_GRAMPC_OPTS_PENTHR              "PenaltyIncreaseThreshold"
#define RLAB_NAME_GRAMPC_OPTS_GRT_CONV            "ConvergenceGradientRelTol"
#define RLAB_NAME_GRAMPC_OPTS_INT_RELTOL          RLAB_NAME_GEN_EREL
#define RLAB_NAME_GRAMPC_OPTS_INT_ABSTOL          RLAB_NAME_GEN_EABS
#define RLAB_NAME_GRAMPC_OPTS_INT_STEP_MINSIZE    "minstep"
#define RLAB_NAME_GRAMPC_OPTS_INT_STEP_MAXNUMBER  "maxi"
#define RLAB_NAME_GRAMPC_OPTS_OPTIMCTRL           "OptimControl"
#define RLAB_NAME_GRAMPC_OPTS_OPTIMTIME           "OptimTime"
#define RLAB_NAME_GRAMPC_OPTS_OPTIMTIME_LSF       "OptimTimeLineSearchFactor"
#define RLAB_NAME_GRAMPC_OPTS_SHIFTCTRL           "ShiftControl"
#define RLAB_NAME_GRAMPC_OPTS_MAXMULTITER         "MaxMultIter"
//
#define RLAB_NAME_GRAMPC_OPTS_LINESEARCH_INIT     "LineSearchInit"
#define RLAB_NAME_GRAMPC_OPTS_LINESEARCH_MAX      "LineSearchMax"
#define RLAB_NAME_GRAMPC_OPTS_LINESEARCH_MIN      "LineSearchMin"
#define RLAB_NAME_GRAMPC_OPTS_LINESEARCH_AAT      "LineSearchAdaptAbsTol"
#define RLAB_NAME_GRAMPC_OPTS_LINESEARCH_IT       "LineSearchIntervalTol"
#define RLAB_NAME_GRAMPC_OPTS_LINESEARCH_IF       "LineSearchIntervalFactor"
// scale
#define RLAB_NAME_GRAMPC_OPTS_SCALEPROBLEM        "ScaleProblem"
// rlab parameters
#define RLAB_NAME_GRAMPC_OPTS_T0                  "t0"
#define RLAB_NAME_GRAMPC_OPTS_DT                  "dt"
#define RLAB_NAME_GRAMPC_OPTS_TSIM                "Tsim"
#define RLAB_NAME_GRAMPC_OPTS_MAXITER             "MaxIter"
#define RLAB_NAME_GRAMPC_OPTS_COUNTTNEXTTMIN      "CountTnextLessThanTmin"

// output
#define RLAB_NAME_GRAMPC_OUTPUT_STATE   "x"
#define RLAB_NAME_GRAMPC_OUTPUT_CONTROL "u"
#define RLAB_NAME_GRAMPC_OUTPUT_COST    "j"

// rlab headers, located in variable $RLAB_SDK
#include <rlab/rlab.h>
#include <rlab/ent.h>
#include <rlab/class.h>
#include <rlab/symbol.h>
#include <rlab/mem.h>
#include <rlab/mdr.h>
#include <rlab/mdrf1.h>
#include <rlab/mds.h>
#include <rlab/mdc.h>
#include <rlab/msr.h>
#include <rlab/list.h>
#include <rlab/btree.h>
#include <rlab/bltin.h>
#include <rlab/util.h>
#include <rlab/mathl.h>
#include <rlab/function.h>

extern FILE *RLAB_STDERR_DS;
extern double gslrngf_ (void);

// naming convention for the solver parameters and names
#include <rlab/rlab_solver_parameters_names.h>
#include <rlab/rlab_macros.h>
#include <rlab/rlab_macros_code.h>

// standard libraries
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "grampc.h"

struct _grampc_table
{
  int     Nx;   // size of X
  int     Np;   // size of p - "optimizible parameters"
  int     Nu;   // size of u - control inputs
  int     Nh;   // dimension of equality constraints
  int     Ng;   // dimension of inequality constraints
  int     Nht;  // dimension of equality constraints at T
  int     Ngt;  // dimension of inequality constraints at T
  int     Nc;   // number of constraints
  // system functions
  bool    have_ffct;
  bool    have_dfdu;
  bool    have_dfdp;
  //
  bool    have_lfct;
  bool    have_dldx;
  bool    have_dldu;
  bool    have_dldp;
  //
  bool    have_Vfct;
  bool    have_dVdx;
  bool    have_dVdp;
  bool    have_dVdT;
  // equality constraints: on [0,T>
  bool    have_gfct;
  bool    have_dgdx;
  bool    have_dgdu;
  bool    have_dgdp;
  // inequality constraints on [0,T>
  bool    have_hfct;
  bool    have_dhdx;
  bool    have_dhdu;
  bool    have_dhdp;
  // equality constraints: at T
  bool    have_gTfct;
  bool    have_dgTdx;
  bool    have_dgTdp;
  bool    have_dgTdT;
  // inequality constraints at T
  bool    have_hTfct;
  bool    have_dhTdx;
  bool    have_dhTdp;
  bool    have_dhTdT;
  // additional functions required for semi-implicit systems
  bool    have_dfdx;
  bool    have_dfdxtrans;
  bool    have_dfdt;
  bool    have_dHdxdt;
  bool    have_Mfct;
  bool    have_Mtrans;

  // rlab starts here
  r_extern_func_args ext_f_args;
  // f - ODE function
  Ent * ent_fn_ffct;
  Ent * ent_fn_dfdx;
  Ent * ent_fn_dfdu;
  Ent * ent_fn_dfdp;
  Ent * ent_fn_dfdt;
  // l - Cost function - integral
  Ent * ent_fn_lfct;
  Ent * ent_fn_dldx;
  Ent * ent_fn_dldu;
  Ent * ent_fn_dldp;
  // V - cost function - terminal
  Ent * ent_fn_Vfct;
  Ent * ent_fn_dVdx;
  Ent * ent_fn_dVdT;
  Ent * ent_fn_dVdp;
  // g - equality function
  Ent * ent_fn_gfct;
  Ent * ent_fn_dgdx;
  Ent * ent_fn_dgdu;
  Ent * ent_fn_dgdp;
  // h - inequality function
  Ent * ent_fn_hfct;
  Ent * ent_fn_dhdx;
  Ent * ent_fn_dhdu;
  Ent * ent_fn_dhdp;
  // gT - equality function, terminal
  Ent * ent_fn_gTfct;
  Ent * ent_fn_dgTdx;
  Ent * ent_fn_dgTdp;
  Ent * ent_fn_dgTdT;
  // hT - inequality function, terminal
  Ent * ent_fn_hTfct;
  Ent * ent_fn_dhTdx;
  Ent * ent_fn_dhTdp;
  Ent * ent_fn_dhTdT;
  Ent * ent_fn_dHdxdt;
  // mass function, for some problems
  Ent * ent_fn_Mfct;
  Ent * ent_fn_Mtrans;
};

typedef struct _grampc_table GRAMPC_TABLE;


// rlabplus extensions: GRAMPC_simplex.c
extern Ent * ent_grampc(int nargs, Datum args[]);

void dfdu(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u,
          ctypeRNum *p, typeUSERPARAM *userparam);
void dfdp(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u,
          ctypeRNum *p, typeUSERPARAM *userparam);


#endif
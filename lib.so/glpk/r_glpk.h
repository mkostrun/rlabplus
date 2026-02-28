/* rlabplus_glpk.h: Gnu Programming Kit for rlabplus */

/* This file is a part of rlabplus
   Copyright (C) 2017 M. Kostrun

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

#ifndef RLABPLUS_GLPK_H
#define RLABPLUS_GLPK_H


//
// glpk:
//
//  input
#define RLAB_NAME_GLPK_STRUCT_OPTDIR      "opt_direction"
#define RLAB_NAME_GLPK_STRUCT_OBJECTIVE   "objective"
#define RLAB_NAME_GLPK_STRUCT_CONSTRAINT   RLAB_NAME_GEN_CONSTRAINTS
#define RLAB_NAME_GLPK_STRUCT_BOUNDS_AUX  "bounds_row"
#define RLAB_NAME_GLPK_STRUCT_BOUNDS_STR  "bounds_col"
#define RLAB_NAME_GLPK_STRUCT_CLASS       "problem"
#define RLAB_NAME_GLPK_STRUCT_COLS_BIN    "col_bin"
#define RLAB_NAME_GLPK_STRUCT_COLS_INT    "col_int"
#define RLAB_NAME_GLPK_STRUCT_C0          "c0"
// input
#define RLAB_NAME_GLPK_PRINT_SOL  "print_sol"
#define RLAB_NAME_GLPK_METHOD      RLAB_NAME_GEN_METHOD
#define RLAB_NAME_GLPK_ORDER      "ordering"
#define RLAB_NAME_GLPK_OBJMAX     "obj_max"
#define RLAB_NAME_GLPK_OBJMIN     "obj_min"
#define RLAB_NAME_GLPK_SMPRE      "presolve"
#define RLAB_NAME_GLPK_MIPBT      "mip_btrack"
#define RLAB_NAME_GLPK_MAXI        RLAB_NAME_GEN_MAXITER
#define RLAB_NAME_GLPK_PRICE      "pricing"
#define RLAB_NAME_GLPK_RATIO      "r_test"
#define RLAB_NAME_GLPK_TOLBND     "tol_bnd"
#define RLAB_NAME_GLPK_TOLDJ      "tol_dj"
#define RLAB_NAME_GLPK_TOLPIV     "tol_piv"
#define RLAB_NAME_GLPK_STDOUT      RLAB_NAME_GEN_STDOUT
#define RLAB_NAME_GLPK_MIP_TOLINT   "tol_int"
#define RLAB_NAME_GLPK_MIP_TOLOBJ   "tol_obj"
#define RLAB_NAME_GLPK_MIP_BRA      "branch"
#define RLAB_NAME_GLPK_MIP_BTR      "backtrack"
#define RLAB_NAME_GLPK_MIP_PP       "preprocess"
#define RLAB_NAME_GLPK_MIP_FP       "feas_pump"
#define RLAB_NAME_GLPK_MIP_PS       "proximity_search"
#define RLAB_NAME_GLPK_MIP_GMI      "gmi_cuts"
#define RLAB_NAME_GLPK_MIP_MIR      "mir_cuts"
#define RLAB_NAME_GLPK_MIP_COV      "cov_cuts"
#define RLAB_NAME_GLPK_MIP_CLQ      "clq_cuts"
#define RLAB_NAME_GLPK_MIP_MIPGAP   "mip_gap"
#define RLAB_NAME_GLPK_MIP_CB       "cb_size"
#define RLAB_NAME_GLPK_MIP_PRESOLVE "presolve"
#define RLAB_NAME_GLPK_MIP_BINARIZE "binarize"

#define RLAB_NAME_GLPK_COEF_COL       "coef_col"
#define RLAB_NAME_GLPK_COEF_ROW       "coef_row"
#define RLAB_NAME_GLPK_OBJECTIVE      "objective"
#define RLAB_NAME_GLPK_STATUS         "status"
#define RLAB_NAME_GLPK_STATUS_PRIMAL  "status_primal"
#define RLAB_NAME_GLPK_STATUS_DUAL    "dual_status"
#define RLAB_NAME_GLPK_COEF_COL_DUAL  "dual_coef_col"
#define RLAB_NAME_GLPK_COEF_ROW_DUAL  "dual_coef_row"
#define RLAB_NAME_GLPK_UNBOUND_ROW    "unbound_row"
#define RLAB_NAME_GLPK_UNBOUND_COL    "unbound_col"


// rlabplus extensions: glpk_simplex.c
extern Ent * ent_glpk_read_file (int nargs, Datum args[]);
extern Ent * ent_glpk_write_file (int nargs, Datum args[]);
extern Ent * ent_glpk_solve_lp  (int nargs, Datum args[]);

#endif
//
// rlabplus (C) 2002-2013 Marijan Kostrun.
//
// GLPK - Gnu Linear programming kit
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
// See the file ./COPYING
// **********************************************************************

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

// standard libraries
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>

// glpk
#include "glpk.h"
#include "r_glpk.h"

// naming convention for the solver parameters and names
#include <rlab/rlab_solver_parameters_names.h>
#include <rlab/rlab_macros.h>
#include <rlab/rlab_macros_code.h>

//
// read: user given or from file in a certain format
//
#undef THIS_SOLVER
#define THIS_SOLVER RLAB_NAME_EMBED_GLPK ".read"
Ent * ent_glpk_read_file (int nargs, Datum args[])
{
  Ent *e1=0, *e2=0, *E;
  int i, j, k;
  MDR *x1, *x2, *x3, *x4=0, *x5=0;
  MSR *sret;
  char *s1 = 0, *s2 = 0;
  Btree *bw;

  glp_prob *lpx_lp = 0;
  int nr, nc;

  FILE *fptr=0;

  if (nargs > 2 || nargs == 0)
  { rerror (THIS_SOLVER ": requires one or two arguments"); }

  if (nargs >= 1)
  {
    // file name
    RLABCODE_PROCESS_ARG1_S(THIS_SOLVER,0,e1,s1,1);

    // does the file exist?
    fptr = fopen (s1, "r");
    if (!fptr)
      rerror (THIS_SOLVER ": Input file '%s' does not exist. Cannot continue!");
    fclose (fptr);
  }

  if (nargs == 2)
  {
    // second argument is the type of the file: may be omitted thus NOERROR macro
    RLABCODE_PROCESS_ARG1_S_NOERRORS(THIS_SOLVER,1,e2,s2,-1);
  }

  // find last dot in filename to figure out the extension
  if (!s2)
  {
    s2 = strrchr(s1, '.');
    s2++;
  }

  // For loading problem we do not need terminal printouts
  glp_term_out(GLP_OFF);

  // create a problem
  lpx_lp = glp_create_prob();

  if (!strcmp (s2, "mps") || !strcmp (s2, "MPS"))
  {
    if (glp_read_mps (lpx_lp,GLP_MPS_DECK,NULL,s1))
      if (glp_read_mps (lpx_lp,GLP_MPS_FILE,NULL,s1))
        rerror(THIS_SOLVER ": File not in MPS format !");

  }
  else if (!strcmp (s2, "lpt") || !strcmp (s2, "LPT"))
  {
    // I hate this: redirecting printf's
    if (glp_read_lp (lpx_lp,NULL,s1))
    {
      if (lpx_lp)
        glp_delete_prob (lpx_lp);
      rerror(THIS_SOLVER ": File not in CPX format !");
    }
  }
  else if (!strcmp (s2, "mod") || !strcmp (s2, "MOD") || !strcmp (s2, "gz") || !strcmp (s2, "GZ"))
  {
    // I hate this: redirecting printf's
    if (glp_read_prob (lpx_lp, 0, s1))
    {
      if (lpx_lp)
        glp_delete_prob (lpx_lp);
      rerror(THIS_SOLVER ": File not in GLPK format !");
    }
  }
  else
  {
    if (lpx_lp)
      glp_delete_prob (lpx_lp);
    rerror(THIS_SOLVER ": Unsupported file format");
  }

  // enable printouts
  glp_term_out(GLP_OFF);

  if (!lpx_lp)
    rerror(THIS_SOLVER ": Unsuccesful read from file");

  // clean
  ent_Clean (e1);
  ent_Clean (e2);

  bw = btree_Create ();

  //
  // return a list with the relevant variables from the file
  //
  nr = glp_get_num_rows (lpx_lp);
  nc = glp_get_num_cols (lpx_lp);

  // cost linear functional
  x1 = mdr_Create (1, nc);
  for (j = 1; j <= nc; j++)
    MdrV1 (x1, j) = glp_get_obj_coef (lpx_lp, j);
  install  (bw, RLAB_NAME_GLPK_STRUCT_OBJECTIVE, ent_Assign_Rlab_MDR(x1));

  if (glp_get_obj_coef (lpx_lp, 0))
  {
    install  (bw, RLAB_NAME_GLPK_STRUCT_C0,
              ent_Create_Rlab_Double(glp_get_obj_coef (lpx_lp, 0)) );
  }

  if (glp_get_obj_dir (lpx_lp) == GLP_MIN)
    install (bw, RLAB_NAME_GLPK_STRUCT_OPTDIR, ent_Create_Rlab_String("min"));
  else if (glp_get_obj_dir (lpx_lp) == GLP_MAX)
    install (bw, RLAB_NAME_GLPK_STRUCT_OPTDIR, ent_Create_Rlab_String("max"));
  else
    install (bw, RLAB_NAME_GLPK_STRUCT_OPTDIR, ent_Create_Rlab_String("undef"));

  //
  // use sparse storage for constraint matrix A
  //
  int len, nnz = 0;
  nnz  = glp_get_num_nz (lpx_lp);
  sret = msr_Create (nr, nc);
  msr_Setup (sret, nnz);
  k = 0;
  for (i = 1; i <= nr; i++)
  {
    sret->ia[i - 1] = k + 1;
    len = glp_get_mat_row (lpx_lp, i, &sret->ja[k-1], &sret->d[k-1]);
    k  += len;
  }
  sret->ia[nr] = k + 1;

  E = ent_Create ();
  ent_type (E) = MATRIX_SPARSE_REAL;
  ent_data (E) = sret;
  install  (bw, RLAB_NAME_GLPK_STRUCT_CONSTRAINT, E);

  // bounds: auxiliary variables
  x2 = mdr_Create (nr, 2);
  for (i = 1; i <= nr; i++)
  {
    //
    Mdr1 (x2, i, 1) = glp_get_row_lb(lpx_lp,i);
    if (Mdr1 (x2, i, 1) == -DBL_MAX)
      Mdr1 (x2, i, 1) = -create_inf ();
    //
    Mdr1 (x2, i, 2) = glp_get_row_ub(lpx_lp,i);
    if (Mdr1 (x2, i, 2) ==  DBL_MAX)
      Mdr1 (x2, i, 2) =  create_inf ();
  }
  install  (bw, RLAB_NAME_GLPK_STRUCT_BOUNDS_AUX, ent_Assign_Rlab_MDR(x2));

  // bounds: structural variables
  x3 = mdr_Create (nc, 2);
  for (j = 1; j <= nc; j++)
  {
    //
    Mdr1 (x3, j, 1) = glp_get_col_lb (lpx_lp, j);
    if (Mdr1 (x3, j, 1) == -DBL_MAX)
      Mdr1 (x3, j, 1) = -create_inf ();
    //
    Mdr1 (x3, j, 2) = glp_get_col_ub (lpx_lp, j);
    if (Mdr1 (x3, j, 2) == DBL_MAX)
      Mdr1 (x3, j, 2) =  create_inf ();
  }
  install  (bw, RLAB_NAME_GLPK_STRUCT_BOUNDS_STR, ent_Assign_Rlab_MDR(x3));

  //
  // check whether the problem is MIP or LP
  //
  if (glp_get_num_int(lpx_lp) + glp_get_num_bin(lpx_lp))
    install  (bw, RLAB_NAME_GLPK_STRUCT_CLASS, ent_Create_Rlab_String("mip"));
  else
    install  (bw, RLAB_NAME_GLPK_STRUCT_CLASS, ent_Create_Rlab_String("lp"));

  //
  // if the problem is MIP and there are integer variables add isint array
  //
  if (glp_get_num_int(lpx_lp))
  {
    x4 = mdr_Create(1,nc);
    for (i=1; i <= nc; i++)
    {
      MdrV1 (x4,i) = 0;
      if (glp_get_col_kind(lpx_lp,i) == GLP_IV)
        MdrV1 (x4,i) = 1;
    }
    install  (bw, RLAB_NAME_GLPK_STRUCT_COLS_INT, ent_Assign_Rlab_MDR(x4));
  }
  if (glp_get_num_bin(lpx_lp))
  {
    x5 = mdr_Create(1,nc);
    for (i=1; i <= nc; i++)
    {
      MdrV1 (x5,i) = 0;
      if (glp_get_col_kind(lpx_lp,i) == GLP_BV)
        MdrV1 (x5,i) = 1;
    }
    install  (bw, RLAB_NAME_GLPK_STRUCT_COLS_BIN, ent_Assign_Rlab_MDR(x5));
  }

  //
  // clean lp from the memory
  //
  if (lpx_lp)
    glp_delete_prob (lpx_lp);

  return ent_Assign_Rlab_BTREE(bw);
}

//
// lp write
//
#undef THIS_SOLVER
#define THIS_SOLVER RLAB_NAME_EMBED_GLPK ".write"
Ent * ent_glpk_write_file (int nargs, Datum args[])
{
  Ent *e1=0, *e2=0, *e3=0;
  int nc=0, nr=0;
  int i, j, k, istatus=0;
  char res[16] = { '\0' }, *s1=0, *s2=0;
  MDR *x1=0, *x2=0, *x3=0, *x4=0, *x5=0, *x6=0;
  MSR *x2s=0;

  FILE *fptr = NULL;

  int    * ilpx_bound=0;
  double * left_bound=0, * right_bound=0, c0=0;

  int    lpx_class_mip=0, lpx_opt_dir=0;

  ListNode * node;

  glp_prob *lpx_lp = 0;

  if (nargs <= 1 || nargs > 3)
    rerror (THIS_SOLVER ": requires two or three arguments");

  //
  // e1: list contining all the entries
  //
  e1 = bltin_get_ent (args[0]);
  if (ent_type (e1) != BTREE)
    rerror (THIS_SOLVER ": First argument 'lp' must be list !");

  //
  // objective
  //
  node = btree_FindNode (ent_data (e1), RLAB_NAME_GLPK_STRUCT_OBJECTIVE);
  if (!node)
    rerror (THIS_SOLVER ": First argument 'lp' must have entry '"
        RLAB_NAME_GLPK_STRUCT_OBJECTIVE "' real vector !");
  x1 = class_matrix_real (var_ent (node));
  if (!x1)
    rerror (THIS_SOLVER ": First argument 'lp' must have entry '"
        RLAB_NAME_GLPK_STRUCT_OBJECTIVE "' real vector !");
  nc = MNC (x1) * MNR (x1);
  if (!nc)
    rerror (THIS_SOLVER ": First argument 'lp' must have entry '"
        RLAB_NAME_GLPK_STRUCT_OBJECTIVE "' real vector !");

  //
  // objective - constant term
  //
  node = btree_FindNode (ent_data (e1), RLAB_NAME_GLPK_STRUCT_C0);
  if (node)
    if (ent_type(var_ent (node)) == MATRIX_DENSE_REAL)
      c0 = class_double(var_ent (node));

  //
  // constraint
  //
  node = btree_FindNode (ent_data (e1), RLAB_NAME_GLPK_STRUCT_CONSTRAINT);
  if (!node)
    rerror (THIS_SOLVER ": First argument 'lp' must have entry '"
        RLAB_NAME_GLPK_STRUCT_CONSTRAINT "' real dense or sparse matrix !");
  if (    ent_type(var_ent (node)) != MATRIX_DENSE_REAL
      &&  ent_type(var_ent (node)) != MATRIX_SPARSE_REAL    )
    rerror (THIS_SOLVER ": First argument 'lp' must have entry '"
        RLAB_NAME_GLPK_STRUCT_CONSTRAINT "' real dense or sparse matrix !");
  // check entry and process accordingly
  if (ent_type(var_ent (node)) == MATRIX_DENSE_REAL)
  {
    x2 = class_matrix_real (var_ent (node));
    if (!x2)
      rerror (THIS_SOLVER ": First argument 'lp' must have entry '"
          RLAB_NAME_GLPK_STRUCT_CONSTRAINT "' real matrix !");
    nr = MNR (x2);
    if (!nr)
      rerror (THIS_SOLVER ": First argument 'lp' must have entry '"
          RLAB_NAME_GLPK_STRUCT_CONSTRAINT "' real matrix !");
    if (nc != MNC (x2))
      rerror (THIS_SOLVER ": First argument 'lp' : '"
          RLAB_NAME_GLPK_STRUCT_CONSTRAINT"' and '"
          RLAB_NAME_GLPK_STRUCT_OBJECTIVE"' dimension mismatch !");
  }
  else
  {
    x2s = ent_data (var_ent (node));
    if (!x2s)
      rerror (THIS_SOLVER ": First argument 'lp' must have entry '"
          RLAB_NAME_GLPK_STRUCT_CONSTRAINT "' real matrix !");
    nr = MNR (x2s);
    if (!nr)
      rerror (THIS_SOLVER ": First argument 'lp' must have entry '"
          RLAB_NAME_GLPK_STRUCT_CONSTRAINT "' real matrix !");
    if (nc != MNC (x2s))
      rerror (THIS_SOLVER ": First argument 'lp' : '"
          RLAB_NAME_GLPK_STRUCT_CONSTRAINT"' and '"
          RLAB_NAME_GLPK_STRUCT_OBJECTIVE"' dimension mismatch !");
  }

  //
  // bounds_aux
  //
  node = btree_FindNode (ent_data (e1), RLAB_NAME_GLPK_STRUCT_BOUNDS_AUX);
  if (!node)
    rerror (THIS_SOLVER ": First argument 'lp' must have entry '"
        RLAB_NAME_GLPK_STRUCT_BOUNDS_AUX "' real 2-column matrix !");
  if (ent_type(var_ent (node)) != MATRIX_DENSE_REAL)
    rerror (THIS_SOLVER ": First argument 'lp' must have entry '"
        RLAB_NAME_GLPK_STRUCT_BOUNDS_AUX "' real 2-column matrix !");
  x3 = class_matrix_real (var_ent (node));
  if (!x3)
    rerror (THIS_SOLVER ": First argument 'lp' must have entry '"
        RLAB_NAME_GLPK_STRUCT_BOUNDS_AUX "' real 2-column matrix !");
  if (nr != MNR (x3))
    rerror (THIS_SOLVER ": First argument 'lp' : '"
        RLAB_NAME_GLPK_STRUCT_CONSTRAINT"' and '"
        RLAB_NAME_GLPK_STRUCT_BOUNDS_AUX"' dimension mismatch !");

  //
  // bounds_struct
  //
  node = btree_FindNode (ent_data (e1), RLAB_NAME_GLPK_STRUCT_BOUNDS_STR);
  if (!node)
    rerror (THIS_SOLVER ": First argument 'lp' must have entry '"
        RLAB_NAME_GLPK_STRUCT_BOUNDS_STR "' real 2-column matrix !");
  if (ent_type(var_ent (node)) != MATRIX_DENSE_REAL)
    rerror (THIS_SOLVER ": First argument 'lp' must have entry '"
        RLAB_NAME_GLPK_STRUCT_BOUNDS_STR "' real 2-column matrix !");
  x4 = class_matrix_real (var_ent (node));
  if (nc != MNR (x4))
    rerror (THIS_SOLVER ": First argument 'lp' : '"
        RLAB_NAME_GLPK_STRUCT_CONSTRAINT"' and '"
        RLAB_NAME_GLPK_STRUCT_BOUNDS_STR"' dimension mismatch !");

  //
  // problem
  //
  node = btree_FindNode (ent_data (e1), RLAB_NAME_GLPK_STRUCT_CLASS);
  if (!node)
    rerror (THIS_SOLVER ": First argument 'lp' must have entry '"
        RLAB_NAME_GLPK_STRUCT_CLASS "' string scalar !");
  if (ent_type(var_ent (node)) != MATRIX_DENSE_STRING)
    rerror (THIS_SOLVER ": First argument 'lp' must have entry '"
        RLAB_NAME_GLPK_STRUCT_BOUNDS_STR "' string scalar !");
  s1 = class_char_pointer (var_ent (node));
  if (!s1)
    rerror (THIS_SOLVER ": First argument 'lp' must have entry '"
        RLAB_NAME_GLPK_STRUCT_CLASS "' string scalar !");
  if (!strlen(s1))
    rerror (THIS_SOLVER ": First argument 'lp' must have entry '"
        RLAB_NAME_GLPK_STRUCT_CLASS "' string scalar !");
  if (!strcmp (s1, "lp") || !strcmp (s1, "LP"))
    lpx_class_mip = 0;
  else if (!strcmp (s1, "mip") || !strcmp (s1, "MIP"))
    lpx_class_mip = 1;
  else
    rerror (THIS_SOLVER ": First argument 'lp' must have entry '"
        RLAB_NAME_GLPK_STRUCT_CLASS "' string scalar !");

  //
  // opt_direction
  //
  node = btree_FindNode (ent_data (e1), RLAB_NAME_GLPK_STRUCT_OPTDIR);
  if (ent_type(var_ent (node)) != MATRIX_DENSE_STRING)
    rerror (THIS_SOLVER ": First argument 'lp' must have entry '"
        RLAB_NAME_GLPK_STRUCT_OPTDIR "' string scalar !");
  s1 = class_char_pointer (var_ent (node));
  if (!s1)
    rerror (THIS_SOLVER ": First argument 'lp' must have entry '"
        RLAB_NAME_GLPK_STRUCT_OPTDIR "' string scalar !");
  if (!strlen(s1))
    rerror (THIS_SOLVER ": First argument 'lp' must have entry '"
        RLAB_NAME_GLPK_STRUCT_OPTDIR "' string scalar !");

  if (!strcmp (s1, "min") || !strcmp (s1, "MIN"))
    lpx_opt_dir = GLP_MIN;
  else if (!strcmp (s1, "max") || !strcmp (s1, "MAX"))
    lpx_opt_dir = GLP_MAX;
  else
    rerror (THIS_SOLVER ": First argument 'lp' : Unknown option for '"
        RLAB_NAME_GLPK_STRUCT_OPTDIR "' !");

  //
  // isint
  //
  if (lpx_class_mip)
  {
    node = btree_FindNode (ent_data (e1), RLAB_NAME_GLPK_STRUCT_COLS_INT);
    if (node)
      x5 = class_matrix_real (var_ent (node));
    node = btree_FindNode (ent_data (e1), RLAB_NAME_GLPK_STRUCT_COLS_BIN);
    if (node)
      x6 = class_matrix_real (var_ent (node));
    if (!x5 && !x6)
      rerror (THIS_SOLVER ": First argument 'lp' must have entry '"
          RLAB_NAME_GLPK_STRUCT_COLS_INT "'  or '"
          RLAB_NAME_GLPK_STRUCT_COLS_BIN "'  if classified as mixed-integer-problem !");
    if (MNR(x5)*MNC(x5) != nc || MNR(x6)*MNC(x6) != nc)
      rerror (THIS_SOLVER ": First argument 'lp': '"
          RLAB_NAME_GLPK_STRUCT_COLS_INT "'  or '"
          RLAB_NAME_GLPK_STRUCT_COLS_BIN "'  dimension mismatch !");
  }

  // file name
  e2 = bltin_get_ent (args[1]);
  if (ent_type (e2) != MATRIX_DENSE_STRING)
    rerror (THIS_SOLVER ": Second argument 'filename' must be string scalar !");
  s1 = class_char_pointer (e2);
  if (!s1)
    rerror (THIS_SOLVER ": Second argument 'filename' must be string scalar !");
  if (!strlen(s1))
    rerror (THIS_SOLVER ": Second argument 'filename' must be string scalar !");

  // user can specify class additionally
  if (nargs == 3)
  {
    // type
    e3 = bltin_get_ent (args[2]);
    if (ent_type (e3) == MATRIX_DENSE_STRING)
      s2 = class_char_pointer (e3);
  }

  // get the data
  fptr = fopen (s1, "w");
  if (!fptr)
    rerror (THIS_SOLVER ": cannot open file for writing!");

  //
  // prepare bounds for the lp structure
  //
  ilpx_bound  = (int *)    GC_MALLOC( (nr + nc + 1) * sizeof(int) );
  left_bound  = (double *) GC_MALLOC( (nr + nc + 1) * sizeof(double) );
  right_bound = (double *) GC_MALLOC( (nr + nc + 1) * sizeof(double) );
  for (i = 1; i <= nr; i++)
  {
    if (Mdr1 (x3, i, 1) == -create_inf () && Mdr1 (x3, i, 2) == create_inf ())
    {
      ilpx_bound[i]  = GLP_FR;
      left_bound[i]  = 0;
      right_bound[i] = 0;
      continue;
    }
    else if (Mdr1 (x3, i, 1) == -create_inf () && Mdr1 (x3, i, 2) != create_inf ())
    {
      ilpx_bound[i]  = GLP_UP;
      left_bound[i]  = 0;
      right_bound[i] = Mdr1 (x3, i, 2);
    }
    else if (Mdr1 (x3, i, 1) != -create_inf () && Mdr1 (x3, i, 2) == create_inf ())
    {
      ilpx_bound[i]  = GLP_LO;
      left_bound[i]  = Mdr1 (x3, i, 1);
      right_bound[i] = 0;
    }
    else if (Mdr1 (x3, i, 1) < Mdr1 (x3, i, 2))
    {
      ilpx_bound[i]  = GLP_DB;
      left_bound[i]  = Mdr1 (x3, i, 1);
      right_bound[i] = Mdr1 (x3, i, 2);
    }
    else if (Mdr1 (x3, i, 1) == Mdr1 (x3, i, 2))
    {
      ilpx_bound[i]  = GLP_FX;
      left_bound[i]  = Mdr1 (x3, i, 1);
      right_bound[i] = Mdr1 (x3, i, 2);
    }
    else
    {
      GC_FREE(ilpx_bound);
      GC_FREE(left_bound);
      GC_FREE(right_bound);
      rerror (THIS_SOLVER ": First argument 'lp' : '"
          RLAB_NAME_GLPK_STRUCT_BOUNDS_AUX "' is improper");
    }
    continue;

  }
  for (i = 1; i <= nc; i++)
  {
    if (Mdr1 (x4, i, 1) == -create_inf () && Mdr1 (x4, i, 2) == create_inf ())
    {
      ilpx_bound[i + nr]  = GLP_FR;
      left_bound[i + nr]  = 0;
      right_bound[i + nr] = 0;
    }
    else if (Mdr1 (x4, i, 1) == -create_inf () && Mdr1 (x4, i, 2) != create_inf ())
    {
      ilpx_bound[i + nr]  = GLP_UP;
      left_bound[i + nr]  = 0;
      right_bound[i + nr] = Mdr1 (x4, i, 2);
    }
    else if (Mdr1 (x4, i, 1) != -create_inf () && Mdr1 (x4, i, 2) == create_inf ())
    {
      ilpx_bound[i + nr]  = GLP_LO;
      left_bound[i + nr]  = Mdr1 (x4, i, 1);
      right_bound[i + nr] = 0;
    }
    else if (Mdr1 (x4, i, 1) < Mdr1 (x4, i, 2))
    {
      ilpx_bound[i + nr]  = GLP_DB;
      left_bound[i + nr]  = Mdr1 (x4, i, 1);
      right_bound[i + nr] = Mdr1 (x4, i, 2);
    }
    else if (Mdr1 (x4, i, 1) == Mdr1 (x4, i, 2))
    {
      ilpx_bound[i + nr]  = GLP_FX;
      left_bound[i + nr]  = Mdr1 (x4, i, 1);
      right_bound[i + nr] = Mdr1 (x4, i, 2);
    }
    else
    {
      GC_FREE(ilpx_bound);
      GC_FREE(left_bound);
      GC_FREE(right_bound);
      rerror (THIS_SOLVER ": First argument 'lp' : '"
          RLAB_NAME_GLPK_STRUCT_BOUNDS_STR "' is improper");
    }
    continue;
  }

  //
  // create/reset a LP solver structure for others to use
  //
  lpx_lp = glp_create_prob ();
  if (!lpx_lp)
    rerror (THIS_SOLVER ": Terrible internal error : Cannot assign memory !");

  glp_set_prob_name (lpx_lp, NULL);         // no name
  glp_set_obj_dir   (lpx_lp, lpx_opt_dir);  // min or max

  // lp: rows are dummy variables
  glp_add_rows (lpx_lp, nr);
  for (i = 1; i <= nr; i++)
  {
    sprintf (res, "r[%i]", i);
    glp_set_row_name (lpx_lp, i, res);
    glp_set_row_bnds (lpx_lp, i, ilpx_bound[i], left_bound[i], right_bound[i]);
  }

  // lp: columns are the variables of interest
  glp_add_cols (lpx_lp, nc);
  for (i = nr + 1; i <= nr + nc; i++)
  {
    sprintf (res, "x[%i]", i - nr);
    glp_set_col_name (lpx_lp, i - nr, res);
    glp_set_col_bnds (lpx_lp, i - nr, ilpx_bound[i], left_bound[i],
                      right_bound[i]);
    glp_set_obj_coef (lpx_lp, i - nr, MdrV1 (x1, i - nr));
  }
  if (c0)
    glp_set_obj_coef (lpx_lp, 0, c0);


  // clean up
  GC_FREE ( ilpx_bound );
  GC_FREE ( left_bound );
  GC_FREE ( right_bound );

  // constraint matrix A declared row-wise
  if (x2)
  {
    int    * cidxs, ncnz;
    double * cvals;

    cidxs = (int *)    GC_MALLOC( (nc + 1) * sizeof(int) );
    cvals = (double *) GC_MALLOC( (nc + 1) * sizeof(double) );
    for (i = 1; i <= nr; i++)
    {
        // count the number of nonzeros' in a row.
        // glpk dumps core for zeros in constraint matrix.
      ncnz = 0;
      for (j = 1; j <= nc; j++)
        if (Mdr1 (x2, i, j) != 0)
          ncnz++;
      k = 0;
      for (j = 1; j <= nc; j++)
        if (Mdr1 (x2, i, j) != 0)
      {
        k++;
        cidxs[k] = j;
        cvals[k] = Mdr1 (x2, i, j);
      }
      glp_set_mat_row (lpx_lp, i, ncnz, cidxs, cvals);
    }
    GC_FREE (cidxs);
    GC_FREE (cvals);
  }
  else if (x2s)
  {
    int    * cidxs, len, tlen = 0;
    double * cvals;

    cidxs = (int *)    GC_MALLOC( (nc + 1) * sizeof(int) );
    cvals = (double *) GC_MALLOC( (nc + 1) * sizeof(double) );

    for (i = 1; i <= nr; i++)
    {
      len = x2s->ia[i] - x2s->ia[i - 1];
      for (j = 1; j <= len; j++)
      {
        cidxs[j] = x2s->ja[tlen + j - 1];
        cvals[j] = x2s->d[tlen + j - 1];
      }
      glp_set_mat_row (lpx_lp, i, len, cidxs, cvals);
      tlen += len;
    }
    GC_FREE (cidxs);
    GC_FREE (cvals);
  }

  for (j=1; j<=nc; j++)
  {
    // set all columns to continuous
    glp_set_col_kind(lpx_lp,j,GLP_CV);

    // x5: cols_int
    if (x5)
      if (mdiV1(x5,j))
        glp_set_col_kind(lpx_lp,j,GLP_IV);
    if (x6)
      if (mdiV1(x6,j))
        glp_set_col_kind(lpx_lp,j,GLP_BV);
  }

  // find last dot in filename to figure out the extension
  if (!s2)
  {
    s2 = strrchr(s1, '.');
    s2++;
  }

  // For loading problem we do not need terminal printouts
  glp_term_out(GLP_OFF);

  if ( !strcmp (s2, "mps") || !strcmp (s2, "MPS") )
  {
    istatus = glp_write_mps (lpx_lp, GLP_MPS_FILE, NULL, s1);
  }
  else if (!strcmp (s2, "lpt") || !strcmp (s2, "LPT"))
  {
    istatus = glp_write_lp (lpx_lp, NULL, s1);
  }
  else
    rerror (THIS_SOLVER  ": unsupported file format");

  // For loading problem we do not need terminal printouts
  glp_term_out(GLP_ON);

  //
  // cleanup
  //
  ent_Clean (e1);
  ent_Clean (e2);
  ent_Clean (e3);

  //
  // clean lp from the memory
  //
  if (lpx_lp)
    glp_delete_prob (lpx_lp);

  return ent_Create_Rlab_Int(istatus);
}


static int lpx_terminal_printout(void *info, const char *s)
{
  FILE *fptr = info;
  fputs ("RLaB: ", fptr);
  fputs (s, fptr);
  return 1;
}

//
// lp solve
//
#undef THIS_SOLVER
#define THIS_SOLVER RLAB_NAME_EMBED_GLPK ".solve"
Ent *
ent_glpk_solve_lp (int nargs, Datum args[])
{
  Ent *e1=0, *e2=0;
  int nc=0, nr=0;
  int i, j, k, istatus=0, isuccess=GLP_UNDEF;
  char res[16] = { '\0' }, *s1=0, *c=0, *outs=0, *print_sol=0;
  MDR *x1=0, *x2=0, *x3=0, *x4=0, *x5=0, *x6=0;
  MSR *x2s=0;

  FILE *fptr = NULL;

  int    * ilpx_bound=0;
  double * left_bound=0, * right_bound=0, ddummy, c0=0;

  int    lpx_class_mip=0, lpx_opt_dir=0, idummy;

  ListNode * node;

  //
  // rlabplus parameters
  //
  int lpx_r_imethod = -1;

  // lp problem structures:
  glp_prob  *lpx_lp = 0;
  glp_smcp  lpx_sx_params; // simplex
  glp_iptcp lpx_ip_params; // interior point
  glp_iocp  lpx_mi_params; // mixed integer

  if (nargs != 2)
    rerror (THIS_SOLVER ": requires two arguments");

  // initialize default settings for all solver parameters
  glp_init_smcp (&lpx_sx_params);
  glp_init_iptcp(&lpx_ip_params);
  glp_init_iocp (&lpx_mi_params);

  //
  // e1: list containing all the entries
  //
  e1 = bltin_get_ent (args[0]);
  if (ent_type (e1) != BTREE)
    rerror (THIS_SOLVER ": First argument 'lp' must be list !");

  //
  // objective
  //
  node = btree_FindNode (ent_data (e1), RLAB_NAME_GLPK_STRUCT_OBJECTIVE);
  if (!node)
    rerror (THIS_SOLVER ": First argument 'lp' must have entry '"
        RLAB_NAME_GLPK_STRUCT_OBJECTIVE "' real vector !");
  x1 = class_matrix_real (var_ent (node));
  if (!x1)
    rerror (THIS_SOLVER ": First argument 'lp' must have entry '"
        RLAB_NAME_GLPK_STRUCT_OBJECTIVE "' real vector !");
  nc = MNC (x1) * MNR (x1);
  if (!nc)
    rerror (THIS_SOLVER ": First argument 'lp' must have entry '"
        RLAB_NAME_GLPK_STRUCT_OBJECTIVE "' real vector !");

  //
  // objective - constant term
  //
  node = btree_FindNode (ent_data (e1), RLAB_NAME_GLPK_STRUCT_C0);
  if (node)
    if (ent_type(var_ent (node)) == MATRIX_DENSE_REAL)
      c0 = class_double(var_ent (node));

  //
  // constraint
  //
  node = btree_FindNode (ent_data (e1), RLAB_NAME_GLPK_STRUCT_CONSTRAINT);
  if (!node)
    rerror (THIS_SOLVER ": First argument 'lp' must have entry '"
        RLAB_NAME_GLPK_STRUCT_CONSTRAINT "' real dense or sparse matrix !");
  if (    ent_type(var_ent (node)) != MATRIX_DENSE_REAL
      &&  ent_type(var_ent (node)) != MATRIX_SPARSE_REAL    )
    rerror (THIS_SOLVER ": First argument 'lp' must have entry '"
        RLAB_NAME_GLPK_STRUCT_CONSTRAINT "' real dense or sparse matrix !");
  // check entry and process accordingly
  if (ent_type(var_ent (node)) == MATRIX_DENSE_REAL)
  {
    x2 = class_matrix_real (var_ent (node));
    if (!x2)
      rerror (THIS_SOLVER ": First argument 'lp' must have entry '"
          RLAB_NAME_GLPK_STRUCT_CONSTRAINT "' real matrix !");
    nr = MNR (x2);
    if (!nr)
      rerror (THIS_SOLVER ": First argument 'lp' must have entry '"
          RLAB_NAME_GLPK_STRUCT_CONSTRAINT "' real matrix !");
    if (nc != MNC (x2))
      rerror (THIS_SOLVER ": First argument 'lp' : '"
          RLAB_NAME_GLPK_STRUCT_CONSTRAINT"' and '"
          RLAB_NAME_GLPK_STRUCT_OBJECTIVE"' dimension mismatch !");
  }
  else
  {
    x2s = ent_data (var_ent (node));
    if (!x2s)
      rerror (THIS_SOLVER ": First argument 'lp' must have entry '"
          RLAB_NAME_GLPK_STRUCT_CONSTRAINT "' real matrix !");
    nr = MNR (x2s);
    if (!nr)
      rerror (THIS_SOLVER ": First argument 'lp' must have entry '"
          RLAB_NAME_GLPK_STRUCT_CONSTRAINT "' real matrix !");
    if (nc != MNC (x2s))
      rerror (THIS_SOLVER ": First argument 'lp' : '"
          RLAB_NAME_GLPK_STRUCT_CONSTRAINT"' and '"
          RLAB_NAME_GLPK_STRUCT_OBJECTIVE"' dimension mismatch !");
  }

  //
  // bounds_aux
  //
  node = btree_FindNode (ent_data (e1), RLAB_NAME_GLPK_STRUCT_BOUNDS_AUX);
  if (!node)
    rerror (THIS_SOLVER ": First argument 'lp' must have entry '"
        RLAB_NAME_GLPK_STRUCT_BOUNDS_AUX "' real 2-column matrix !");
  if (ent_type(var_ent (node)) != MATRIX_DENSE_REAL)
    rerror (THIS_SOLVER ": First argument 'lp' must have entry '"
        RLAB_NAME_GLPK_STRUCT_BOUNDS_AUX "' real 2-column matrix !");
  x3 = class_matrix_real (var_ent (node));
  if (!x3)
    rerror (THIS_SOLVER ": First argument 'lp' must have entry '"
        RLAB_NAME_GLPK_STRUCT_BOUNDS_AUX "' real 2-column matrix !");
  if (nr != MNR (x3))
    rerror (THIS_SOLVER ": First argument 'lp' : '"
        RLAB_NAME_GLPK_STRUCT_CONSTRAINT"' and '"
        RLAB_NAME_GLPK_STRUCT_BOUNDS_AUX"' dimension mismatch !");

  //
  // bounds_struct
  //
  node = btree_FindNode (ent_data (e1), RLAB_NAME_GLPK_STRUCT_BOUNDS_STR);
  if (!node)
    rerror (THIS_SOLVER ": First argument 'lp' must have entry '"
        RLAB_NAME_GLPK_STRUCT_BOUNDS_STR "' real 2-column matrix !");
  if (ent_type(var_ent (node)) != MATRIX_DENSE_REAL)
    rerror (THIS_SOLVER ": First argument 'lp' must have entry '"
        RLAB_NAME_GLPK_STRUCT_BOUNDS_STR "' real 2-column matrix !");
  x4 = class_matrix_real (var_ent (node));
  if (nc != MNR (x4))
    rerror (THIS_SOLVER ": First argument 'lp' : '"
        RLAB_NAME_GLPK_STRUCT_CONSTRAINT"' and '"
        RLAB_NAME_GLPK_STRUCT_BOUNDS_STR"' dimension mismatch !");

  //
  // problem
  //
  node = btree_FindNode (ent_data (e1), RLAB_NAME_GLPK_STRUCT_CLASS);
  if (!node)
    rerror (THIS_SOLVER ": First argument 'lp' must have entry '"
        RLAB_NAME_GLPK_STRUCT_CLASS "' string scalar !");
  if (ent_type(var_ent (node)) != MATRIX_DENSE_STRING)
    rerror (THIS_SOLVER ": First argument 'lp' must have entry '"
        RLAB_NAME_GLPK_STRUCT_BOUNDS_STR "' string scalar !");
  s1 = class_char_pointer (var_ent (node));
  if (!s1)
    rerror (THIS_SOLVER ": First argument 'lp' must have entry '"
        RLAB_NAME_GLPK_STRUCT_CLASS "' string scalar !");
  if (!strlen(s1))
    rerror (THIS_SOLVER ": First argument 'lp' must have entry '"
        RLAB_NAME_GLPK_STRUCT_CLASS "' string scalar !");
  if (!strcmp (s1, "lp") || !strcmp (s1, "LP"))
    lpx_class_mip = 0;
  else if (!strcmp (s1, "mip") || !strcmp (s1, "MIP"))
    lpx_class_mip = 1;
  else
    rerror (THIS_SOLVER ": First argument 'lp' must have entry '"
        RLAB_NAME_GLPK_STRUCT_CLASS "' string scalar !");

  //
  // opt_direction
  //
  node = btree_FindNode (ent_data (e1), RLAB_NAME_GLPK_STRUCT_OPTDIR);
  if (ent_type(var_ent (node)) != MATRIX_DENSE_STRING)
    rerror (THIS_SOLVER ": First argument 'lp' must have entry '"
        RLAB_NAME_GLPK_STRUCT_OPTDIR "' string scalar !");
  s1 = class_char_pointer (var_ent (node));
  if (!s1)
    rerror (THIS_SOLVER ": First argument 'lp' must have entry '"
        RLAB_NAME_GLPK_STRUCT_OPTDIR "' string scalar !");
  if (!strlen(s1))
    rerror (THIS_SOLVER ": First argument 'lp' must have entry '"
        RLAB_NAME_GLPK_STRUCT_OPTDIR "' string scalar !");

  if (!strcmp (s1, "min") || !strcmp (s1, "MIN"))
    lpx_opt_dir = GLP_MIN;
  else if (!strcmp (s1, "max") || !strcmp (s1, "MAX"))
    lpx_opt_dir = GLP_MAX;
  else
    rerror (THIS_SOLVER ": First argument 'lp' : Unknown option for '"
        RLAB_NAME_GLPK_STRUCT_OPTDIR "' !");

  //
  // isint
  //
  if (lpx_class_mip)
  {
    node = btree_FindNode (ent_data (e1), RLAB_NAME_GLPK_STRUCT_COLS_INT);
    if (node)
      x5 = class_matrix_real (var_ent (node));
    node = btree_FindNode (ent_data (e1), RLAB_NAME_GLPK_STRUCT_COLS_BIN);
    if (node)
      x6 = class_matrix_real (var_ent (node));
    if (!x5 && !x6)
      rerror (THIS_SOLVER ": First argument 'lp' must have entry '"
          RLAB_NAME_GLPK_STRUCT_COLS_INT "'  or '"
          RLAB_NAME_GLPK_STRUCT_COLS_BIN "'  if classified as mixed-integer-problem !");
    if (MNR(x5)*MNC(x5) != nc || MNR(x6)*MNC(x6) != nc)
      rerror (THIS_SOLVER ": First argument 'lp': '"
          RLAB_NAME_GLPK_STRUCT_COLS_INT "'  or '"
          RLAB_NAME_GLPK_STRUCT_COLS_BIN "'  dimension mismatch !");
  }

  //
  // mandatory options for the solver
  //
  e2 = bltin_get_ent (args[1]);
  if (ent_type (e2) != BTREE)
    rerror (THIS_SOLVER ": Second argument 'options' must be list !");

  if (!lpx_class_mip)
  {
    // print solution to file?
    node = btree_FindNode (ent_data (e2), RLAB_NAME_GLPK_PRINT_SOL);
    if (node)
    {
      if (ent_type(var_ent (node)) == MATRIX_DENSE_STRING)
      {
        print_sol = class_char_pointer (var_ent (node));
      }
    }

    //
    // SIMPLEX OR INTERIOR POINT PROBLEM
    //
    // method: 0 - simplex or 1 - interior
    lpx_r_imethod = 0;
    node = btree_FindNode (ent_data (e2), RLAB_NAME_GLPK_METHOD);
    if (node)
    {
      if (ent_type(var_ent (node)) == MATRIX_DENSE_STRING)
      {
        c = class_char_pointer (var_ent (node));
        if (c)
          if (strlen(c))
          {
            if (!strcmp ("dual", c))
            {
              lpx_sx_params.meth = GLP_DUAL;
            }
            else if (!strcmp ("dualp", c))
            {
              lpx_sx_params.meth = GLP_DUALP;
            }
            else if (!strcmp ("primal", c))
            {
              // initialize parameter structure for the solver
              lpx_sx_params.meth = GLP_PRIMAL;
            }
            else if (!strcmp ("interior", c))
            {
              lpx_r_imethod = 1;
            }
          }
      }
    }

    // stdout
    if (lpx_r_imethod)
      lpx_ip_params.msg_lev = GLP_MSG_OFF;
    else
      lpx_sx_params.msg_lev = GLP_MSG_OFF;
    node = btree_FindNode (ent_data (e2), RLAB_NAME_GLPK_STDOUT);
    if (node)
    {
      if (ent_type(var_ent (node))== MATRIX_DENSE_STRING)
      {
        outs = class_char_pointer (var_ent (node));
        if (outs)
          if (strlen(outs))
          {
            if (lpx_r_imethod)
              lpx_ip_params.msg_lev = GLP_MSG_ALL;
            else
              lpx_sx_params.msg_lev = GLP_MSG_ALL;
          }
      }
    }

    if (lpx_r_imethod == 0)
    {
      //
      // S I M P L E X   M E T H O D S
      //
      // pricing
      lpx_sx_params.pricing = GLP_PT_STD;
      node = btree_FindNode (ent_data (e2), RLAB_NAME_GLPK_PRICE);
      if (node)
        if (ent_type(var_ent (node)) == MATRIX_DENSE_STRING)
        {
          c = class_char_pointer (var_ent (node));
          if (c)
            if (strlen(c))
            {
              if (!strcmp ("pse", c))
                lpx_sx_params.pricing = GLP_PT_PSE;
            }
        }
      // ratio test
      lpx_sx_params.pricing = GLP_RT_STD;
      node = btree_FindNode (ent_data (e2), RLAB_NAME_GLPK_RATIO);
      if (node)
      {
        if (ent_type(var_ent (node)) == MATRIX_DENSE_STRING)
        {
          c = class_char_pointer (var_ent (node));
          if (c)
            if (strlen(c))
            {
              if (!strcmp ("harris", c))
                lpx_sx_params.pricing = GLP_RT_HAR;
            }
        }
      }
      // tolbnd
      lpx_sx_params.tol_bnd = 1e-7;
      node = btree_FindNode (ent_data (e2), RLAB_NAME_GLPK_TOLBND);
      if (node)
      {
        if (ent_type(var_ent (node)) == MATRIX_DENSE_REAL)
        {
          ddummy = class_double ( var_ent (node) );
          if (ddummy > 0.0 && ddummy < 1)
            lpx_sx_params.tol_bnd = ddummy;
        }
      }
      // toldj
      lpx_sx_params.tol_dj = 1e-7;
      node = btree_FindNode (ent_data (e2), RLAB_NAME_GLPK_TOLDJ);
      if (node)
      {
        if (ent_type(var_ent (node)) == MATRIX_DENSE_REAL)
        {
          ddummy = class_double ( var_ent (node) );
          if (ddummy > 0.0 && ddummy < 1)
            lpx_sx_params.tol_dj = ddummy;
        }
      }
      // tolpiv
      lpx_sx_params.tol_piv = 1e-10;
      node = btree_FindNode (ent_data (e2), RLAB_NAME_GLPK_TOLPIV);
      if (node)
      {
        if (ent_type(var_ent (node)) == MATRIX_DENSE_REAL)
        {
          ddummy = class_double ( var_ent (node) );
          if (ddummy > 0.0 && ddummy < 1)
            lpx_sx_params.tol_piv = ddummy;
        }
      }
      // objll
      lpx_sx_params.obj_ll = -DBL_MAX;
      node = btree_FindNode (ent_data (e2), RLAB_NAME_GLPK_OBJMIN);
      if (node)
      {
        if (ent_type(var_ent (node)) == MATRIX_DENSE_REAL)
        {
          lpx_sx_params.obj_ll = class_double ( var_ent (node) );
        }
      }
      // objul
      lpx_sx_params.obj_ul = DBL_MAX;
      node = btree_FindNode (ent_data (e2), RLAB_NAME_GLPK_OBJMAX);
      if (node)
      {
        if (ent_type(var_ent (node)) == MATRIX_DENSE_REAL)
        {
          lpx_sx_params.obj_ul = class_double ( var_ent (node) );
        }
      }
      // iterations
      lpx_sx_params.it_lim = INT_MAX;
      node = btree_FindNode (ent_data (e2), RLAB_NAME_GLPK_MAXI);
      if (node)
      {
        if (ent_type(var_ent (node)) == MATRIX_DENSE_REAL)
        {
          idummy = (int) class_double (var_ent (node));
          if (idummy >= -1)
            lpx_sx_params.it_lim = idummy;
        }
      }
      // presol
      lpx_sx_params.presolve = GLP_ON;
      node = btree_FindNode (ent_data (e2), RLAB_NAME_GLPK_SMPRE);
      if (node)
      {
        if (ent_type(var_ent (node)) == MATRIX_DENSE_REAL)
        {
          idummy = (int) class_double (var_ent (node));
          if (!idummy)
            lpx_sx_params.presolve = GLP_OFF;
        }
      }
    }
    else if (lpx_r_imethod == 1)
    {
      //
      // I N T E R I O R   P O I N T   M E T H O D
      //
      // ordering algorithm
      lpx_ip_params.ord_alg = GLP_ORD_NONE;
      node = btree_FindNode (ent_data (e2), RLAB_NAME_GLPK_ORDER);
      if (node)
      {
        if (ent_type(var_ent (node)) == MATRIX_DENSE_STRING)
        {
          c = class_char_pointer (var_ent (node));
          if (c)
            if (strlen(c))
            {
              if (!strcmp ("qmd", c))
                lpx_ip_params.ord_alg = GLP_ORD_QMD;
              else if (!strcmp ("amd", c))
                lpx_ip_params.ord_alg = GLP_ORD_AMD;
              else if (!strcmp ("symamd", c))
                lpx_ip_params.ord_alg = GLP_ORD_SYMAMD;
            }
        }
      }
    }

  } // if (!lpx_class_mip)
  else
  {
    //
    // M I X E D   I N T E G E R   P R O B L E M
    //
    // stdout
    lpx_mi_params.msg_lev = GLP_MSG_OFF;
    node = btree_FindNode (ent_data (e2), RLAB_NAME_GLPK_STDOUT);
    if (node)
    {
      if (ent_type(var_ent (node))== MATRIX_DENSE_STRING)
      {
        outs = class_char_pointer (var_ent (node));
        if (outs)
          if (strlen(outs))
            lpx_mi_params.msg_lev = GLP_MSG_ALL;
      }
    }

    // tolint
    lpx_mi_params.tol_int = 1e-5;
    node = btree_FindNode (ent_data (e2), RLAB_NAME_GLPK_MIP_TOLINT);
    if (node)
    {
      if (ent_type(var_ent (node)) == MATRIX_DENSE_REAL)
      {
        ddummy = class_double (var_ent (node));
        if (ddummy > 0.0 && ddummy < 1)
          lpx_mi_params.tol_int = ddummy;
      }
    }
    // branch
    lpx_mi_params.br_tech = GLP_BR_DTH;
    node = btree_FindNode (ent_data (e2), RLAB_NAME_GLPK_MIP_BRA);
    if (node)
    {
      if (ent_type(var_ent (node)) == MATRIX_DENSE_STRING)
      {
        c = class_char_pointer (var_ent (node));
        if (c)
          if (strlen(c))
          {
            if (!strcmp ("ffv", c))
              lpx_mi_params.br_tech = GLP_BR_FFV;
            else if (!strcmp ("lfv", c))
              lpx_mi_params.br_tech = GLP_BR_LFV;
            else if (!strcmp ("mfv", c))
              lpx_mi_params.br_tech = GLP_BR_MFV;
            else if (!strcmp ("pch", c))
              lpx_mi_params.br_tech = GLP_BR_PCH;
          }
      }
    }
    // backtrack
    lpx_mi_params.bt_tech = GLP_BT_BLB;
    node = btree_FindNode (ent_data (e2), RLAB_NAME_GLPK_MIP_BTR);
    if (node)
    {
      if (ent_type(var_ent (node)) == MATRIX_DENSE_STRING)
      {
        c = class_char_pointer (var_ent (node));
        if (c)
          if (strlen(c))
          {
            if (!strcmp ("dfs", c))
              lpx_mi_params.bt_tech = GLP_BT_DFS;
            else if (!strcmp ("bfs", c))
              lpx_mi_params.bt_tech = GLP_BT_BFS;
            else if (!strcmp ("bph", c))
              lpx_mi_params.bt_tech = GLP_BT_BPH;
          }
      }
    }
    // preprocess
    lpx_mi_params.pp_tech = GLP_PP_ALL;
    node = btree_FindNode (ent_data (e2), RLAB_NAME_GLPK_MIP_PP);
    if (node)
    {
      if (ent_type(var_ent (node)) == MATRIX_DENSE_STRING)
      {
        c = class_char_pointer (var_ent (node));
        if (c)
          if (strlen(c))
          {
            if (!strcmp ("none", c))
              lpx_mi_params.pp_tech = GLP_PP_NONE;
            else if (!strcmp ("root", c))
              lpx_mi_params.pp_tech = GLP_PP_ROOT;
          }
      }
    }
    // feasibility pump
    lpx_mi_params.fp_heur = GLP_OFF;
    node = btree_FindNode (ent_data (e2), RLAB_NAME_GLPK_MIP_FP);
    if (node)
    {
      if (ent_type(var_ent (node)) == MATRIX_DENSE_REAL)
      {
        idummy = (int) class_double (var_ent (node));
        if (idummy)
          lpx_mi_params.fp_heur = GLP_ON;
      }
    }
#if (GLP_MAJOR_VERSION > 4) || ((GLP_MAJOR_VERSION==4) && (GLP_MINOR_VERSION > 51))
    // proximity search
    lpx_mi_params.ps_heur   = GLP_OFF;
    lpx_mi_params.ps_tm_lim = 0;
    node = btree_FindNode (ent_data (e2), RLAB_NAME_GLPK_MIP_PS);
    if (node)
    {
      if (ent_type(var_ent (node)) == MATRIX_DENSE_REAL)
      {
        idummy = (int) class_double (var_ent (node));
        if (idummy)
        {
          lpx_mi_params.ps_heur = GLP_ON;
          lpx_mi_params.ps_tm_lim = idummy;
        }
      }
    }
#endif
    // gomory cuts
    lpx_mi_params.gmi_cuts = GLP_OFF;
    node = btree_FindNode (ent_data (e2), RLAB_NAME_GLPK_MIP_GMI);
    if (node)
    {
      if (ent_type(var_ent (node)) == MATRIX_DENSE_REAL)
      {
        idummy = (int) class_double (var_ent (node));
        if (idummy)
          lpx_mi_params.gmi_cuts = GLP_ON;
      }
    }
    // mixed integer cuts
    lpx_mi_params.mir_cuts = GLP_OFF;
    node = btree_FindNode (ent_data (e2), RLAB_NAME_GLPK_MIP_MIR);
    if (node)
    {
      if (ent_type(var_ent (node)) == MATRIX_DENSE_REAL)
      {
        idummy = (int) class_double (var_ent (node));
        if (idummy)
          lpx_mi_params.mir_cuts = GLP_ON;
      }
    }
    // mixed cover cuts
    lpx_mi_params.cov_cuts = GLP_OFF;
    node = btree_FindNode (ent_data (e2), RLAB_NAME_GLPK_MIP_COV);
    if (node)
    {
      if (ent_type(var_ent (node)) == MATRIX_DENSE_REAL)
      {
        idummy = (int) class_double (var_ent (node));
        if (idummy)
          lpx_mi_params.cov_cuts = GLP_ON;
      }
    }
    // clique cuts
    lpx_mi_params.clq_cuts = GLP_OFF;
    node = btree_FindNode (ent_data (e2), RLAB_NAME_GLPK_MIP_CLQ);
    if (node)
    {
      if (ent_type(var_ent (node)) == MATRIX_DENSE_REAL)
      {
        idummy = (int) class_double (var_ent (node));
        if (idummy)
          lpx_mi_params.clq_cuts = GLP_ON;
      }
    }
    // tolobj
    lpx_mi_params.tol_obj = 1e-7;
    node = btree_FindNode (ent_data (e2), RLAB_NAME_GLPK_MIP_TOLOBJ);
    if (node)
    {
      if (ent_type(var_ent (node)) == MATRIX_DENSE_REAL)
      {
        ddummy = class_double (var_ent (node));
        if (ddummy > 0.0 && ddummy < 1)
          lpx_mi_params.tol_obj = ddummy;
      }
    }
    // mip gap
    lpx_mi_params.mip_gap = 0.0;
    node = btree_FindNode (ent_data (e2), RLAB_NAME_GLPK_MIP_MIPGAP);
    if (node)
    {
      if (ent_type(var_ent (node)) == MATRIX_DENSE_REAL)
      {
        ddummy = class_double (var_ent (node));
        if (ddummy > 0.0 && ddummy < 1)
          lpx_mi_params.mip_gap = ddummy;
      }
    }
    // count of extra bytes
    lpx_mi_params.cb_size = 0;
    node = btree_FindNode (ent_data (e2), RLAB_NAME_GLPK_MIP_CB);
    if (node)
    {
      if (ent_type(var_ent (node)) == MATRIX_DENSE_REAL)
      {
        idummy = (int) class_double (var_ent (node));
        if (idummy>0 && idummy<257)
        {
          lpx_mi_params.cb_size = idummy;
        }
      }
    }
    // presolve
    lpx_mi_params.presolve = GLP_OFF;
    node = btree_FindNode (ent_data (e2), RLAB_NAME_GLPK_MIP_PRESOLVE);
    if (node)
    {
      if (ent_type(var_ent (node)) == MATRIX_DENSE_REAL)
      {
        idummy = (int) class_double (var_ent (node));
        if (idummy)
          lpx_mi_params.presolve = GLP_ON;
      }
    }
    // binarize
    lpx_mi_params.binarize = GLP_OFF;
    node = btree_FindNode (ent_data (e2), RLAB_NAME_GLPK_MIP_BINARIZE);
    if (node)
    {
      if (ent_type(var_ent (node)) == MATRIX_DENSE_REAL)
      {
        idummy = (int) class_double (var_ent (node));
        if (idummy)
          lpx_mi_params.binarize = GLP_ON;
      }
    }

  }

  //
  // prepare bounds for the lp structure
  //
  ilpx_bound  = (int *)    GC_MALLOC( (nr + nc + 1) * sizeof(int) );
  left_bound  = (double *) GC_MALLOC( (nr + nc + 1) * sizeof(double) );
  right_bound = (double *) GC_MALLOC( (nr + nc + 1) * sizeof(double) );
  for (i = 1; i <= nr; i++)
  {
    if (Mdr1 (x3, i, 1) == -create_inf () && Mdr1 (x3, i, 2) == create_inf ())
    {
      ilpx_bound[i]  = GLP_FR;
      left_bound[i]  = 0;
      right_bound[i] = 0;
      continue;
    }
    else if (Mdr1 (x3, i, 1) == -create_inf () && Mdr1 (x3, i, 2) != create_inf ())
    {
      ilpx_bound[i]  = GLP_UP;
      left_bound[i]  = 0;
      right_bound[i] = Mdr1 (x3, i, 2);
    }
    else if (Mdr1 (x3, i, 1) != -create_inf () && Mdr1 (x3, i, 2) == create_inf ())
    {
      ilpx_bound[i]  = GLP_LO;
      left_bound[i]  = Mdr1 (x3, i, 1);
      right_bound[i] = 0;
    }
    else if (Mdr1 (x3, i, 1) < Mdr1 (x3, i, 2))
    {
      ilpx_bound[i]  = GLP_DB;
      left_bound[i]  = Mdr1 (x3, i, 1);
      right_bound[i] = Mdr1 (x3, i, 2);
    }
    else if (Mdr1 (x3, i, 1) == Mdr1 (x3, i, 2))
    {
      ilpx_bound[i]  = GLP_FX;
      left_bound[i]  = Mdr1 (x3, i, 1);
      right_bound[i] = Mdr1 (x3, i, 2);
    }
    else
    {
      GC_FREE(ilpx_bound);
      GC_FREE(left_bound);
      GC_FREE(right_bound);
      rerror (THIS_SOLVER ": First argument 'lp' : '"
          RLAB_NAME_GLPK_STRUCT_BOUNDS_AUX "' is improper");
    }
    continue;

  }
  for (i = 1; i <= nc; i++)
  {
    if (Mdr1 (x4, i, 1) == -create_inf () && Mdr1 (x4, i, 2) == create_inf ())
    {
      ilpx_bound[i + nr]  = GLP_FR;
      left_bound[i + nr]  = 0;
      right_bound[i + nr] = 0;
    }
    else if (Mdr1 (x4, i, 1) == -create_inf () && Mdr1 (x4, i, 2) != create_inf ())
    {
      ilpx_bound[i + nr]  = GLP_UP;
      left_bound[i + nr]  = 0;
      right_bound[i + nr] = Mdr1 (x4, i, 2);
    }
    else if (Mdr1 (x4, i, 1) != -create_inf () && Mdr1 (x4, i, 2) == create_inf ())
    {
      ilpx_bound[i + nr]  = GLP_LO;
      left_bound[i + nr]  = Mdr1 (x4, i, 1);
      right_bound[i + nr] = 0;
    }
    else if (Mdr1 (x4, i, 1) < Mdr1 (x4, i, 2))
    {
      ilpx_bound[i + nr]  = GLP_DB;
      left_bound[i + nr]  = Mdr1 (x4, i, 1);
      right_bound[i + nr] = Mdr1 (x4, i, 2);
    }
    else if (Mdr1 (x4, i, 1) == Mdr1 (x4, i, 2))
    {
      ilpx_bound[i + nr]  = GLP_FX;
      left_bound[i + nr]  = Mdr1 (x4, i, 1);
      right_bound[i + nr] = Mdr1 (x4, i, 2);
    }
    else
    {
      GC_FREE(ilpx_bound);
      GC_FREE(left_bound);
      GC_FREE(right_bound);
      rerror (THIS_SOLVER ": First argument 'lp' : '"
          RLAB_NAME_GLPK_STRUCT_BOUNDS_STR "' is improper");
    }
    continue;
  }

  //
  // create/reset a LP solver structure for others to use
  //
  lpx_lp = glp_create_prob ();
  if (!lpx_lp)
    rerror (THIS_SOLVER ": Terrible internal error : Cannot assign memory !");

  glp_set_prob_name (lpx_lp, NULL);         // no name
  glp_set_obj_dir   (lpx_lp, lpx_opt_dir);  // min or max

  // lp: rows are dummy variables
  glp_add_rows (lpx_lp, nr);
  for (i = 1; i <= nr; i++)
  {
    sprintf (res, "r[%i]", i);
    glp_set_row_name (lpx_lp, i, res);
    glp_set_row_bnds (lpx_lp, i, ilpx_bound[i], left_bound[i], right_bound[i]);
  }

  // lp: columns are the variables of interest
  glp_add_cols (lpx_lp, nc);
  for (i = nr + 1; i <= nr + nc; i++)
  {
    sprintf (res, "x[%i]", i - nr);
    glp_set_col_name (lpx_lp, i - nr, res);
    glp_set_col_bnds (lpx_lp, i - nr, ilpx_bound[i], left_bound[i],
                      right_bound[i]);
    glp_set_obj_coef (lpx_lp, i - nr, MdrV1 (x1, i - nr));
  }
  if (c0)
    glp_set_obj_coef (lpx_lp, 0, c0);

  // clean up
  GC_FREE ( ilpx_bound );
  GC_FREE ( left_bound );
  GC_FREE ( right_bound );

  // constraint matrix A declared row-wise
  if (x2)
  {
    int    * cidxs, ncnz;
    double * cvals;

    cidxs = (int *)    GC_MALLOC( (nc + 1) * sizeof(int) );
    cvals = (double *) GC_MALLOC( (nc + 1) * sizeof(double) );
    for (i = 1; i <= nr; i++)
    {
        // count the number of nonzeros' in a row.
        // glpk dumps core for zeros in constraint matrix.
      ncnz = 0;
      for (j = 1; j <= nc; j++)
        if (Mdr1 (x2, i, j) != 0)
          ncnz++;
      k = 0;
      for (j = 1; j <= nc; j++)
        if (Mdr1 (x2, i, j) != 0)
      {
        k++;
        cidxs[k] = j;
        cvals[k] = Mdr1 (x2, i, j);
      }
      glp_set_mat_row (lpx_lp, i, ncnz, cidxs, cvals);
    }
    GC_FREE (cidxs);
    GC_FREE (cvals);
  }
  else if (x2s)
  {
    int    * cidxs, len, tlen = 0;
    double * cvals;

    cidxs = (int *)    GC_MALLOC( (nc + 1) * sizeof(int) );
    cvals = (double *) GC_MALLOC( (nc + 1) * sizeof(double) );

    for (i = 1; i <= nr; i++)
    {
      len = x2s->ia[i] - x2s->ia[i - 1];
      for (j = 1; j <= len; j++)
      {
        cidxs[j] = x2s->ja[tlen + j - 1];
        cvals[j] = x2s->d[tlen + j - 1];
      }
      glp_set_mat_row (lpx_lp, i, len, cidxs, cvals);
      tlen += len;
    }
    GC_FREE (cidxs);
    GC_FREE (cvals);
  }

  for (j=1; j<=nc; j++)
  {
    // set all columns to continuous
    glp_set_col_kind(lpx_lp,j,GLP_CV);

    // x5: cols_int
    if (x5)
      if (mdiV1(x5,j))
        glp_set_col_kind(lpx_lp,j,GLP_IV);
    if (x6)
      if (mdiV1(x6,j))
        glp_set_col_kind(lpx_lp,j,GLP_BV);
  }

  // print management
  if (outs)
    fptr = fopen (outs, "w");

  if (fptr)
    glp_term_hook(lpx_terminal_printout,fptr);
  else
    glp_term_out(GLP_OFF);

  //
  //  S O L V E   L P   P R O B L E M
  //
  Btree *bw = btree_Create ();
  MDR *r1=0, *r2=0, *r3=0, *r4=0;

  if (!lpx_class_mip && !lpx_r_imethod)
  {
    //
    // S I M P L E X   M E T H O D S
    //

    // solve:
    isuccess = glp_simplex(lpx_lp, &lpx_sx_params);

    // success:
    install (bw, "fail", ent_Create_Rlab_Success());

    // status
    if (isuccess)
      goto lpx_get_me_out_of_here;
    istatus = glp_get_status(lpx_lp);
    switch (istatus)
    {
      case GLP_OPT:
        install (bw, RLAB_NAME_GLPK_STATUS, ent_Create_Rlab_String("optimal"));
        break;

      case GLP_FEAS:
        install (bw, RLAB_NAME_GLPK_STATUS, ent_Create_Rlab_String("feasible"));
        break;

      case GLP_INFEAS:
        install (bw, RLAB_NAME_GLPK_STATUS, ent_Create_Rlab_String("infeasible"));
        break;

      case GLP_UNBND:
        install (bw, RLAB_NAME_GLPK_STATUS, ent_Create_Rlab_String("unbounded"));
        break;

      default:
        install (bw, RLAB_NAME_GLPK_STATUS, ent_Create_Rlab_String("undef"));
    }

    // primal:
    //  status
    istatus = glp_get_prim_stat(lpx_lp);
    switch (istatus)
    {
      case GLP_FEAS:
        install (bw, RLAB_NAME_GLPK_STATUS_PRIMAL, ent_Create_Rlab_String("feasible"));
        break;

      case GLP_INFEAS:
        install (bw, RLAB_NAME_GLPK_STATUS_PRIMAL, ent_Create_Rlab_String("infeasible"));
        break;

      case GLP_NOFEAS:
        install (bw, RLAB_NAME_GLPK_STATUS_PRIMAL, ent_Create_Rlab_String("none"));
        break;

      default:
        install (bw, RLAB_NAME_GLPK_STATUS_PRIMAL, ent_Create_Rlab_String("undef"));
    }

    //  row solution
    if (istatus == GLP_FEAS)
    {
      r1 = mdr_Create (1, nr);
      for (j = 1; j <= nr; j++)
        MdrV1 (r1, j) = glp_get_row_prim (lpx_lp, j);
    }
    else
      r1 = mdr_Create(0,0);
    install (bw, RLAB_NAME_GLPK_COEF_ROW, ent_Assign_Rlab_MDR(r1));

    // col solution
    if (istatus == GLP_FEAS)
    {
      r2 = mdr_Create (1, nc);
      for (j = 1; j <= nc; j++)
        MdrV1 (r2, j) = glp_get_col_prim (lpx_lp, j);
    }
    else
      r2 = mdr_Create(0,0);
    install (bw, RLAB_NAME_GLPK_COEF_COL, ent_Assign_Rlab_MDR(r2));

    // status dual
    istatus = glp_get_dual_stat(lpx_lp);
    switch (istatus)
    {
      case GLP_FEAS:
        install (bw, RLAB_NAME_GLPK_STATUS_DUAL, ent_Create_Rlab_String("feasible"));
        break;

      case GLP_INFEAS:
        install (bw, RLAB_NAME_GLPK_STATUS_DUAL, ent_Create_Rlab_String("infeasible"));
        break;

      case GLP_NOFEAS:
        install (bw, RLAB_NAME_GLPK_STATUS_DUAL, ent_Create_Rlab_String("none"));
        break;

      default:
        install (bw, RLAB_NAME_GLPK_STATUS_DUAL, ent_Create_Rlab_String("undef"));
    }

    //  row dual solution
    if (istatus == GLP_FEAS)
    {
      r3 = mdr_Create (1, nr);
      for (j = 1; j <= nr; j++)
        MdrV1 (r3, j) = glp_get_row_dual (lpx_lp, j);
    }
    else
      r3 = mdr_Create(0,0);
    install (bw, RLAB_NAME_GLPK_COEF_ROW_DUAL, ent_Assign_Rlab_MDR(r3));

    //  col dual solution
    if (istatus == GLP_FEAS)
    {
      r4 = mdr_Create (1, nc);
      for (j = 1; j <= nc; j++)
        MdrV1 (r4, j) = glp_get_col_dual (lpx_lp, j);
    }
    else
      r4 = mdr_Create(0,0);
    install (bw, RLAB_NAME_GLPK_COEF_COL_DUAL, ent_Assign_Rlab_MDR(r4));

    // objective value
    install (bw, RLAB_NAME_GLPK_OBJECTIVE, ent_Create_Rlab_Double(glp_get_obj_val(lpx_lp)));

    if ((i = glp_get_unbnd_ray(lpx_lp)))
    {
      if (i<=nr)
        install (bw, RLAB_NAME_GLPK_UNBOUND_ROW, ent_Create_Rlab_Double(i));
      else
        install (bw, RLAB_NAME_GLPK_UNBOUND_COL, ent_Create_Rlab_Double(i - nr));
    }

    if (print_sol)
      if (strlen(print_sol))
        glp_write_sol(lpx_lp, print_sol);

    if (outs)
    {
      if (fptr)
        fflush(fptr);

      glp_print_ranges(lpx_lp, 0, NULL, 0,outs);
    }


  } // if (!lpx_class_mip && !lpx_r_imethod)
  else if (!lpx_class_mip && lpx_r_imethod)
  {
    //
    // I N T E R I O R - P O I N T   M E T H O D
    //

    // solve:
    isuccess = glp_interior(lpx_lp, &lpx_ip_params);

    // success:
    install (bw, "fail", ent_Create_Rlab_Double(isuccess));

    // status
    if (isuccess)
      goto lpx_get_me_out_of_here;
    istatus = glp_ipt_status(lpx_lp);
    switch (istatus)
    {
      case GLP_OPT:
        install (bw, RLAB_NAME_GLPK_STATUS, ent_Create_Rlab_String("optimal"));
        break;

      case GLP_INFEAS:
        install (bw, RLAB_NAME_GLPK_STATUS, ent_Create_Rlab_String("infeasible"));
        break;

      case GLP_NOFEAS:
        install (bw, RLAB_NAME_GLPK_STATUS, ent_Create_Rlab_String("none"));
        break;

      default:
        install (bw, RLAB_NAME_GLPK_STATUS, ent_Create_Rlab_String("undef"));
    }

    // primal:
    //  row solution
    if (istatus == GLP_OPT)
    {
      r1 = mdr_Create (1, nr);
      for (j = 1; j <= nr; j++)
        MdrV1 (r1, j) = glp_ipt_row_prim (lpx_lp, j);
    }
    else
      r1 = mdr_Create(0,0);
    install (bw, RLAB_NAME_GLPK_COEF_ROW, ent_Assign_Rlab_MDR(r1));

    // col solution
    if (istatus == GLP_OPT)
    {
      r2 = mdr_Create (1, nc);
      for (j = 1; j <= nc; j++)
        MdrV1 (r2, j) = glp_ipt_col_prim (lpx_lp, j);
    }
    else
      r2 = mdr_Create(0,0);
    install (bw, RLAB_NAME_GLPK_COEF_COL, ent_Assign_Rlab_MDR(r2));

    //  row dual solution
    if (istatus == GLP_OPT)
    {
      r3 = mdr_Create (1, nr);
      for (j = 1; j <= nr; j++)
        MdrV1 (r3, j) = glp_ipt_row_dual (lpx_lp, j);
    }
    else
      r3 = mdr_Create(0,0);
    install (bw, RLAB_NAME_GLPK_COEF_ROW_DUAL, ent_Assign_Rlab_MDR(r3));

    //  col dual solution
    if (istatus == GLP_OPT)
    {
      r4 = mdr_Create (1, nc);
      for (j = 1; j <= nc; j++)
        MdrV1 (r4, j) = glp_ipt_col_dual (lpx_lp, j);
    }
    else
      r4 = mdr_Create(0,0);
    install (bw, RLAB_NAME_GLPK_COEF_COL_DUAL, ent_Assign_Rlab_MDR(r4));

    // objective value
    install (bw, RLAB_NAME_GLPK_OBJECTIVE, ent_Create_Rlab_Double(glp_ipt_obj_val(lpx_lp)));

    if (isvalidstring(print_sol)>0)
    {
      glp_write_ipt(lpx_lp, print_sol);
    }

  } // if (!lpx_class_mip && lpx_r_imethod)
  else if (lpx_class_mip)
  {
    //
    // M I X E D - I N T E G E R   M E T H O D
    //

    // solve:
    isuccess = glp_intopt(lpx_lp, &lpx_mi_params);

    // success:
    install (bw, "fail", ent_Create_Rlab_Double(isuccess));

    // status
    if (isuccess)
      goto lpx_get_me_out_of_here;
    istatus = glp_mip_status(lpx_lp);
    switch (istatus)
    {
      case GLP_OPT:
        install (bw, RLAB_NAME_GLPK_STATUS, ent_Create_Rlab_String("optimal"));
        break;

      case GLP_FEAS:
        install (bw, RLAB_NAME_GLPK_STATUS, ent_Create_Rlab_String("feasible"));
        break;

      case GLP_NOFEAS:
        install (bw, RLAB_NAME_GLPK_STATUS, ent_Create_Rlab_String("none"));
        break;

      default:
        install (bw, RLAB_NAME_GLPK_STATUS, ent_Create_Rlab_String("undef"));
    }

    //  row solution
    if (istatus == GLP_OPT)
    {
      r1 = mdr_Create (1, nr);
      for (j = 1; j <= nr; j++)
        MdrV1 (r1, j) = glp_mip_row_val (lpx_lp, j);
    }
    else
      r1 = mdr_Create(0,0);
    install (bw, RLAB_NAME_GLPK_COEF_ROW, ent_Assign_Rlab_MDR(r1));

    // col solution
    if (istatus == GLP_OPT)
    {
      r2 = mdr_Create (1, nc);
      for (j = 1; j <= nc; j++)
        MdrV1 (r2, j) = glp_mip_col_val (lpx_lp, j);
    }
    else
      r2 = mdr_Create(0,0);
    install (bw, RLAB_NAME_GLPK_COEF_COL, ent_Assign_Rlab_MDR(r2));

    // objective value
    install (bw, RLAB_NAME_GLPK_OBJECTIVE, ent_Create_Rlab_Double(glp_mip_obj_val(lpx_lp)));

    if (print_sol)
      if (strlen(print_sol))
        glp_write_mip(lpx_lp, print_sol);
  } // if (lpx_sx_params)

lpx_get_me_out_of_here:

  //
  if (fptr)
  {
    fflush(fptr);
    fclose(fptr);
  }

  // turn on printing before exiting
  glp_term_out(GLP_ON);
  glp_term_hook(NULL,NULL);

  //
  // cleanup
  //
  ent_Clean (e1);
  ent_Clean (e2);
  if (lpx_lp)
    glp_delete_prob (lpx_lp);

  return ent_Assign_Rlab_BTREE(bw);
}



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


// glpk
#include "r_grampc.h"

//
//
// read: user given or from file in a certain format
//
#undef  THIS_SOLVER
#define THIS_SOLVER "grampc"

/** OCP dimensions: states (Nx), controls (Nu), parameters (Np), equalities (Ng),
    inequalities (Nh), terminal equalities (NgT), terminal inequalities (NhT) **/
void ocp_dim(typeInt *Nx, typeInt *Nu, typeInt *Np, typeInt *Ng,
             typeInt *Nh, typeInt *NgT, typeInt *NhT, typeUSERPARAM *userparam)
{
  GRAMPC_TABLE *gdata=(GRAMPC_TABLE *) userparam;
  *Nx = gdata->Nx;
  *Nu = gdata->Nu;
  *Np = gdata->Np;
  *Nh = gdata->Nh;
  *Ng = gdata->Ng;
  *NgT = gdata->Ngt;
  *NhT = gdata->Nht;
}


#include "fn_ffct.c"
#include "fn_lfct.c"
#include "fn_vfct.c"
#include "fn_gfct.c"
#include "fn_hfct.c"
#include "fn_gtfct.c"
#include "fn_htfct.c"
#include "fn_mass.c"

Ent * ent_grampc(int nargs, Datum args[])
{
  Ent *e1=0, *e2=0, *e3=0, *e4=0, *e5=0, *ex=0, *rent=0;

  MDR *x0=0, *u0=0, *xdes_mdr=0, *udes_mdr=0, *umax=0, *umin=0, *cat=0, *cat_mdr=0;
  MDR *x_sol_mdr=0, *u_sol_mdr=0, *c_sol_mdr=0, *p_mdr_max=0, *p_mdr_min=0;
  MDR *x_pred_mdr=0, *u_pred_mdr=0, *c_pred_mdr=0;

  double penmin=1, penmax=1e6, *p_save=0;
  double reltol=1e-6, abstol=1e-8, minstep=1e-16, penincfac=1.05, pendecfac=0.95, penthr=1;
  double aur=1e-2, cgr=0;
  double optimtime_lsf=1, linesearchinit=0.0, linesearchmin=0.0, linesearchmax=0.0,
            linesearchaat=0.0, linesearchit=0.0, linesearchif=0;
  double thor=create_nan(), tmin=create_nan(), tmax=create_nan();
  double dt=create_nan(), t0=create_nan(), tsim=create_nan(), t;


  Btree *bw=btree_Create(), *bw_sol=NULL, *bw_pred=NULL;
  int Nx=0, Nu=0, Nh=0, Ng=0, Nht=0, Ngt=0, Ntm=0, Np=0, Nc=0, i, j, k;
  int Nhor=-1, MaxMultIter=-1, MaxGradIter=-1, maxi=10000000, optimtime=0, estpenmin=0;
  int scaleproblem=0, ShiftControl=1, OptimControl=1, OptimParam=0, OptimTime=0, MaxIter=-1;
  int i_counttnexttmin=0, icount=0;
  char *outs=0, method=2;
  ListNode *node;

  MDR *t_mdr=0, *x_mdr=0, *x_des_mdr=0, *u_mdr=0, *u_des_mdr=0, *p_mdr=0;
  Ent *t_ent=0, *x_ent=0, *x_des_ent=0, *u_ent=0, *u_des_ent=0, *p_ent=0, *ep=0;

  FILE *fptr = NULL;
  FILE *rlab_stderr = (!RLAB_STDERR_DS) ? stderr : RLAB_STDERR_DS;
  time_t timer=0;

  TYPE_GRAMPC_POINTER(grampc);
  grampc = NULL;

  GRAMPC_TABLE gdata;
  gdata.ent_fn_hfct = NULL;
  gdata.ent_fn_hTfct = NULL;
  gdata.ent_fn_gfct = NULL;
  gdata.ent_fn_gTfct = NULL;
  gdata.have_hfct = 0;
  gdata.have_hTfct = 0;
  gdata.have_gfct = 0;
  gdata.have_gTfct = 0;
  gdata.Nh = 0;
  gdata.Nht = 0;
  gdata.Ng = 0;
  gdata.Ngt = 0;
  gdata.Nc  = 0;
  if (nargs<5)
  {
    rerror (THIS_SOLVER ": requires at least five arguments");
  }

  // function list
  e1 = bltin_get_ent (args[0]);
  if (ent_type (e1) != BTREE)
  {
    goto _exit;
  }

  // process functions from the list in logical order
  // dVdT:
  gdata.ent_fn_dVdT = NULL;
  gdata.have_dVdT = 0;
  node = btree_FindNode(ent_data(e1), RLAB_NAME_GRAMPC_DVDT);
  if (node)
  {
    ex = var_ent(node);
    if (!isfuncent(ex))
    {
      printf (THIS_SOLVER ": " RLAB_ERROR_ARG1_FUNC_VAR "\n");
    }
    gdata.ent_fn_dVdT = ex;
    gdata.have_dVdT = 1;
  }
  // dVdp:
  gdata.ent_fn_dVdp = NULL;
  gdata.have_dVdp = 0;
  node = btree_FindNode(ent_data(e1), RLAB_NAME_GRAMPC_DVDP);
  if (node)
  {
    ex = var_ent(node);
    if (!isfuncent(ex))
    {
      printf (THIS_SOLVER ": " RLAB_ERROR_ARG1_FUNC_VAR "\n");
    }
    gdata.ent_fn_dVdp = ex;
    gdata.have_dVdp = 1;
  }
  // dVdx:
  gdata.ent_fn_dVdx = NULL;
  gdata.have_dVdx = 0;
  node = btree_FindNode(ent_data(e1), RLAB_NAME_GRAMPC_DVDX);
  if (node)
  {
    ex = var_ent(node);
    if (!isfuncent(ex))
    {
      printf (THIS_SOLVER ": " RLAB_ERROR_ARG1_FUNC_VAR "\n");
    }
    gdata.ent_fn_dVdx = ex;
    gdata.have_dVdx = 1;
  }
  // Vfct:
  gdata.ent_fn_Vfct = NULL;
  gdata.have_Vfct = 0;
  node = btree_FindNode(ent_data(e1), RLAB_NAME_GRAMPC_VFCT);
  if (node)
  {
    ex = var_ent(node);
    if (!isfuncent(ex))
    {
      printf (THIS_SOLVER ": " RLAB_ERROR_ARG1_FUNC_VAR "\n");
    }
    gdata.ent_fn_Vfct = ex;
    gdata.have_Vfct = 1;
  }
  // dldp:
  gdata.ent_fn_dldp = NULL;
  gdata.have_dldp = 0;
  node = btree_FindNode(ent_data(e1), RLAB_NAME_GRAMPC_DLDP);
  if (node)
  {
    ex = var_ent(node);
    if (!isfuncent(ex))
    {
      printf (THIS_SOLVER ": " RLAB_ERROR_ARG1_FUNC_VAR "\n");
    }
    gdata.ent_fn_dldp = ex;
    gdata.have_dldp = 1;
  }
  // dldu:
  gdata.ent_fn_dldu = NULL;
  gdata.have_dldu = 0;
  node = btree_FindNode(ent_data(e1), RLAB_NAME_GRAMPC_DLDU);
  if (node)
  {
    ex = var_ent(node);
    if (!isfuncent(ex))
    {
      printf (THIS_SOLVER ": " RLAB_ERROR_ARG1_FUNC_VAR "\n");
    }
    gdata.ent_fn_dldu = ex;
    gdata.have_dldu = 1;
  }
  // dldx:
  gdata.ent_fn_dldx = NULL;
  gdata.have_dldx = 0;
  node = btree_FindNode(ent_data(e1), RLAB_NAME_GRAMPC_DLDX);
  if (node)
  {
    ex = var_ent(node);
    if (!isfuncent(ex))
    {
      printf (THIS_SOLVER ": " RLAB_ERROR_ARG1_FUNC_VAR "\n");
    }
    gdata.ent_fn_dldx = ex;
    gdata.have_dldx = 1;
  }
  // lfct:
  gdata.ent_fn_lfct = NULL;
  gdata.have_lfct = 0;
  node = btree_FindNode(ent_data(e1), RLAB_NAME_GRAMPC_LFCT );
  if (node)
  {
    ex = var_ent(node);
    if (!isfuncent(ex))
    {
      printf (THIS_SOLVER ": " RLAB_ERROR_ARG1_FUNC_VAR "\n");
    }
    gdata.ent_fn_lfct = ex;
    gdata.have_lfct = 1;
  }
  // ffct:
  gdata.ent_fn_ffct = NULL;
  gdata.have_ffct = 0;
  node = btree_FindNode(ent_data(e1), RLAB_NAME_GRAMPC_F );
  if (node)
  {
    ex = var_ent(node);
    if (!isfuncent(ex))
    {
      printf (THIS_SOLVER ": " RLAB_ERROR_ARG1_FUNC_VAR "\n");
    }
    gdata.ent_fn_ffct = ex;
    gdata.have_ffct = 1;
  }
  // dfdx:
  gdata.ent_fn_dfdx = NULL;
  gdata.have_dfdx = 0;
  node = btree_FindNode(ent_data(e1), RLAB_NAME_GRAMPC_DFDX );
  if (node)
  {
    ex = var_ent(node);
    if (!isfuncent(ex))
    {
      printf (THIS_SOLVER ": " RLAB_ERROR_ARG1_FUNC_VAR "\n");
    }
    gdata.ent_fn_dfdx = ex;
    gdata.have_dfdx = 1;
  }
  // dfdu:
  gdata.ent_fn_dfdu = NULL;
  gdata.have_dfdu = 0;
  node = btree_FindNode(ent_data(e1), RLAB_NAME_GRAMPC_DFDU );
  if (node)
  {
    ex = var_ent(node);
    if (!isfuncent(ex))
    {
      printf (THIS_SOLVER ": " RLAB_ERROR_ARG1_FUNC_VAR "\n");
    }
    gdata.ent_fn_dfdu = ex;
    gdata.have_dfdu = 1;
  }
  // dfdt:
  gdata.ent_fn_dfdt = NULL;
  gdata.have_dfdt = 0;
  node = btree_FindNode(ent_data(e1), RLAB_NAME_GRAMPC_DFDT );
  if (node)
  {
    ex = var_ent(node);
    if (!isfuncent(ex))
    {
      printf (THIS_SOLVER ": " RLAB_ERROR_ARG1_FUNC_VAR "\n");
    }
    gdata.ent_fn_dfdt = ex;
    gdata.have_dfdt = 1;
  }
  // hfct:
  gdata.ent_fn_hfct = NULL;
  gdata.have_hfct = 0;
  gdata.Nh = 0;
  node = btree_FindNode(ent_data(e1), RLAB_NAME_GRAMPC_H );
  if (node)
  {
    ex = var_ent(node);
    if (!isfuncent(ex))
    {
      printf (THIS_SOLVER ": " RLAB_ERROR_ARG1_FUNC_VAR "\n");
    }
    gdata.ent_fn_hfct = ex;
    gdata.have_hfct = 1;
  }
  // dhdx:
  gdata.ent_fn_dhdx = NULL;
  gdata.have_dhdx = 0;
  node = btree_FindNode(ent_data(e1), RLAB_NAME_GRAMPC_DHDX );
  if (node)
  {
    ex = var_ent(node);
    if (!isfuncent(ex))
    {
      printf (THIS_SOLVER ": " RLAB_ERROR_ARG1_FUNC_VAR "\n");
    }
    gdata.ent_fn_dhdx = ex;
    gdata.have_dhdx = 1;
  }
  // dhdu:
  gdata.ent_fn_dhdu = NULL;
  gdata.have_dhdu = 0;
  node = btree_FindNode(ent_data(e1), RLAB_NAME_GRAMPC_DHDU );
  if (node)
  {
    ex = var_ent(node);
    if (!isfuncent(ex))
    {
      printf (THIS_SOLVER ": " RLAB_ERROR_ARG1_FUNC_VAR "\n");
    }
    gdata.ent_fn_dhdu = ex;
    gdata.have_dhdu = 1;
  }
  // hTfct:
  gdata.ent_fn_hTfct = NULL;
  gdata.have_hTfct = 0;
  gdata.Nht = 0;
  node = btree_FindNode(ent_data(e1), RLAB_NAME_GRAMPC_HT );
  if (node)
  {
    ex = var_ent(node);
    if (!isfuncent(ex))
    {
      printf (THIS_SOLVER ": " RLAB_ERROR_ARG1_FUNC_VAR "\n");
    }
    gdata.ent_fn_hTfct = ex;
    gdata.have_hTfct = 1;
  }
  // dhTdx:
  gdata.ent_fn_dhTdx = NULL;
  gdata.have_dhTdx = 0;
  node = btree_FindNode(ent_data(e1), RLAB_NAME_GRAMPC_DHTDX );
  if (node)
  {
    ex = var_ent(node);
    if (!isfuncent(ex))
    {
      printf (THIS_SOLVER ": " RLAB_ERROR_ARG1_FUNC_VAR "\n");
    }
    gdata.ent_fn_dhTdx = ex;
    gdata.have_dhTdx = 1;
  }
  // dhTdp:
  gdata.ent_fn_dhTdp = NULL;
  gdata.have_dhTdp = 0;
  node = btree_FindNode(ent_data(e1), RLAB_NAME_GRAMPC_DHTDP );
  if (node)
  {
    ex = var_ent(node);
    if (!isfuncent(ex))
    {
      printf (THIS_SOLVER ": " RLAB_ERROR_ARG1_FUNC_VAR "\n");
    }
    gdata.ent_fn_dhTdp = ex;
    gdata.have_dhTdp = 1;
  }
  // dhTdT:
  gdata.ent_fn_dhTdT = NULL;
  gdata.have_dhTdT = 0;
  node = btree_FindNode(ent_data(e1), RLAB_NAME_GRAMPC_DHTDT );
  if (node)
  {
    ex = var_ent(node);
    if (!isfuncent(ex))
    {
      printf (THIS_SOLVER ": " RLAB_ERROR_ARG1_FUNC_VAR "\n");
    }
    gdata.ent_fn_dhTdT = ex;
    gdata.have_dhTdT = 1;
  }
  // gfct:
  gdata.ent_fn_gfct = NULL;
  gdata.have_gfct = 0;
  gdata.Ng = 0;
  node = btree_FindNode(ent_data(e1), RLAB_NAME_GRAMPC_G );
  if (node)
  {
    ex = var_ent(node);
    if (!isfuncent(ex))
    {
      printf (THIS_SOLVER ": " RLAB_ERROR_ARG1_FUNC_VAR "\n");
    }
    gdata.ent_fn_gfct = ex;
    gdata.have_gfct = 1;
  }
  // gTfct:
  gdata.ent_fn_gTfct = NULL;
  gdata.have_gTfct = 0;
  node = btree_FindNode(ent_data(e1), RLAB_NAME_GRAMPC_GT );
  if (node)
  {
    ex = var_ent(node);
    if (!isfuncent(ex))
    {
      printf (THIS_SOLVER ": " RLAB_ERROR_ARG1_FUNC_VAR "\n");
    }
    gdata.ent_fn_gTfct = ex;
    gdata.have_gTfct = 1;
  }
  // dgTdx:
  gdata.ent_fn_dgTdx = NULL;
  gdata.have_dgTdx = 0;
  node = btree_FindNode(ent_data(e1), RLAB_NAME_GRAMPC_DGTDX );
  if (node)
  {
    ex = var_ent(node);
    if (!isfuncent(ex))
    {
      printf (THIS_SOLVER ": " RLAB_ERROR_ARG1_FUNC_VAR "\n");
    }
    gdata.ent_fn_dgTdx = ex;
    gdata.have_dgTdx = 1;
  }
  // dgTdp:
  gdata.ent_fn_dgTdp = NULL;
  gdata.have_dgTdp = 0;
  node = btree_FindNode(ent_data(e1), RLAB_NAME_GRAMPC_DGTDP );
  if (node)
  {
    ex = var_ent(node);
    if (!isfuncent(ex))
    {
      printf (THIS_SOLVER ": " RLAB_ERROR_ARG1_FUNC_VAR "\n");
    }
    gdata.ent_fn_dgTdp = ex;
    gdata.have_dgTdp = 1;
  }
  // dgTdT:
  gdata.ent_fn_dgTdT = NULL;
  gdata.have_dgTdT = 0;
  node = btree_FindNode(ent_data(e1), RLAB_NAME_GRAMPC_DGTDT );
  if (node)
  {
    ex = var_ent(node);
    if (!isfuncent(ex))
    {
      printf (THIS_SOLVER ": " RLAB_ERROR_ARG1_FUNC_VAR "\n");
    }
    gdata.ent_fn_dgTdT = ex;
    gdata.have_dgTdT = 1;
  }

  // parameters for the functions in the list
  e2 = bltin_get_ent (args[1]);

  // initial state of the system: x0
  e3 = bltin_get_ent (args[2]);
  if (ent_type (e3) != MATRIX_DENSE_REAL)
  {
    goto _exit;
  }
  x0 = ent_data(e3);
  if (!EQVECT(x0))
  {
    goto _exit;
  }
  Nx = gdata.Nx = SIZE(x0);

  // initial system control: u0
  e4 = bltin_get_ent (args[3]);
  if (ent_type (e4) != MATRIX_DENSE_REAL)
  {
    goto _exit;
  }
  u0 = ent_data(e4);
  if (!EQVECT(u0))
  {
    goto _exit;
  }
  Nu = gdata.Nu = SIZE(u0);
  udes_mdr = mdr_Create_SameSize(u0);
  mdr_Zero(udes_mdr);

  // this we need to create because user can provide a scalar applicable to all u's, or
  // individual value for each entry in the u-array
  umax = mdr_Create_SameSize(u0);
  for (i=0; i<SIZE(u0); i++)
    MdrV0(umax,i) = create_inf();
  umin = mdr_Create_SameSize(u0);
  for (i=0; i<SIZE(u0); i++)
    MdrV0(umin,i) = -create_inf();

  // options for the solver

  Np = gdata.Np = 0;
  e5 = bltin_get_ent (args[4]);
  if (ent_type (e5) == BTREE)
  {

    // method for intergration, once interpolated solution is known:
    node = btree_FindNode(ent_data(e5), RLAB_NAME_GEN_IMETHOD);
    if (node)
    {
      p_ent = var_ent(node);
      if (ent_type (p_ent) == MATRIX_DENSE_REAL)
      {
        method = 2; /* set default value: 2 - 2nd order scheme: Heun */
        int idummy = class_double(p_ent);
        if (idummy >=1 && idummy <=4)
        {
          method = idummy;
        }
      }
    }

    // Convergence Criterion:
    // 1.  when
    //      Tnext - dt <= Tmin
    //   stop time optimization
    // 2. when reached
    //      Tnext - dt >= Tmin
    // stop the computation.
    node = btree_FindNode(ent_data(e5), RLAB_NAME_GRAMPC_OPTS_COUNTTNEXTTMIN);
    if (node)
    {
      p_ent = var_ent(node);
      if (ent_type (p_ent) == MATRIX_DENSE_REAL)
      {
        i_counttnexttmin = (class_double(p_ent) > 0);
      }
      if (i_counttnexttmin < 0)
      {
        i_counttnexttmin = 0;
      }
    }

    // OptimControl
    node = btree_FindNode(ent_data(e5), RLAB_NAME_GRAMPC_OPTS_OPTIMCTRL);
    if (node)
    {
      p_ent = var_ent(node);
      if (ent_type (p_ent) == MATRIX_DENSE_REAL)
      {
        OptimControl = (class_double(p_ent) > 0);
      }
    }

    // 'ShiftControl'
    node = btree_FindNode(ent_data(e5), RLAB_NAME_GRAMPC_OPTS_SHIFTCTRL);
    if (node)
    {
      p_ent = var_ent(node);
      if (ent_type (p_ent) == MATRIX_DENSE_REAL)
      {
        ShiftControl = (class_double(p_ent) > 0);
      }
    }

    // 'dt'
    node = btree_FindNode(ent_data(e5), RLAB_NAME_GRAMPC_OPTS_DT);
    if (node)
    {
      p_ent = var_ent(node);
      if (ent_type (p_ent) == MATRIX_DENSE_REAL)
      {
        dt = class_double(p_ent);
      }
    }

    // 't0'
    node = btree_FindNode(ent_data(e5), RLAB_NAME_GRAMPC_OPTS_T0);
    if (node)
    {
      p_ent = var_ent(node);
      if (ent_type (p_ent) == MATRIX_DENSE_REAL)
      {
        t0 = class_double(p_ent);
      }
    }

    // 'tsim'
    node = btree_FindNode(ent_data(e5), RLAB_NAME_GRAMPC_OPTS_TSIM);
    if (node)
    {
      p_ent = var_ent(node);
      if (ent_type (p_ent) == MATRIX_DENSE_REAL)
      {
        tsim = class_double(p_ent);
      }
    }

    // 'ScaleProblem'
    node = btree_FindNode(ent_data(e5), RLAB_NAME_GRAMPC_OPTS_SCALEPROBLEM);
    if (node)
    {
      p_ent = var_ent(node);
      if (ent_type (p_ent) == MATRIX_DENSE_REAL)
      {
        scaleproblem = (class_double(p_ent) > 0);
      }
    }

    // Do something with the options
    // 'p' optimizable parameters
    node = btree_FindNode(ent_data(e5), RLAB_NAME_GRAMPC_OPTS_PARAMS );
    if (node)
    {
      p_ent = var_ent(node);
      if (ent_type (p_ent) == MATRIX_DENSE_REAL)
      {
        p_mdr = ent_data(p_ent);
        Np = gdata.Np = SIZE(p_mdr);
        OptimParam = 1;
        if (SIZE(p_mdr) < 1)
        {
          Np = gdata.Np = 0;
          p_mdr  = NULL;
          p_ent = NULL;
          OptimParam = 0;
        }
      }
    }
    // 'p_min' lower bound on optimizable parameters
    node = btree_FindNode(ent_data(e5), RLAB_NAME_GRAMPC_OPTS_PARAMS_MIN );
    if (node)
    {
      ep = var_ent(node);
      if (ent_type (ep) == MATRIX_DENSE_REAL)
      {
        p_mdr_min = ent_data(ep);
        if (SIZE(p_mdr_min) < 1)
        {
          p_mdr_min = NULL;
        }
      }
    }

    // 'p_max' upper bound on optimizable parameters
    node = btree_FindNode(ent_data(e5), RLAB_NAME_GRAMPC_OPTS_PARAMS_MAX );
    if (node)
    {
      ep  = var_ent(node);
      if (ent_type (ep) == MATRIX_DENSE_REAL)
      {
        p_mdr_max = ent_data(ep);
        if (SIZE(p_mdr_max) < 1)
        {
          p_mdr_max = NULL;
        }
      }
    }

    // 'xdes_mdr'
    node = btree_FindNode(ent_data(e5), RLAB_NAME_GRAMPC_OPTS_X0DES);
    if (node)
    {
      ep = var_ent(node);
      if (ent_type (ep) == MATRIX_DENSE_REAL)
      {
        xdes_mdr = ent_data(ep);
        if (SIZE(xdes_mdr) != SIZE(x0))
        {
          xdes_mdr=NULL;
        }
      }
    }

    // 'udes_mdr'
    node = btree_FindNode(ent_data(e5), RLAB_NAME_GRAMPC_OPTS_U0DES);
    if (node)
    {
      ep = var_ent(node);
      if (ent_type (ep) == MATRIX_DENSE_REAL)
      {
        udes_mdr = ent_data(ep);
        if (SIZE(udes_mdr) != SIZE(u0))
        {
          udes_mdr = NULL;
        }
      }
    }

    // 'umax'
    node = btree_FindNode(ent_data(e5), RLAB_NAME_GRAMPC_OPTS_UMAX);
    if (node)
    {
      ep = var_ent(node);
      if (ent_type (ep) == MATRIX_DENSE_REAL)
      {
        MDR *up = ent_data(ep);
        if (SIZE(up) > 0)
        {
          for (i=0; i<SIZE(u0); i++)
          {
            MdrV0(umax,i) = mdrV0_safe(up,i);
          }
        }
      }
    }

    // 'umin'
    node = btree_FindNode(ent_data(e5), RLAB_NAME_GRAMPC_OPTS_UMIN);
    if (node)
    {
      ep = var_ent(node);
      if (ent_type (ep) == MATRIX_DENSE_REAL)
      {
        MDR *up = ent_data(ep);
        if (SIZE(up) > 0)
        {
          for (i=0; i<SIZE(u0); i++)
          {
            MdrV0(umin,i) = mdrV0_safe(up,i);
          }
        }
      }
    }

    // 'thor'
    node = btree_FindNode(ent_data(e5), RLAB_NAME_GRAMPC_OPTS_THOR);
    if (node)
    {
      ep = var_ent(node);
      if (ent_type (ep) == MATRIX_DENSE_REAL)
      {
        thor = class_double(ep);
      }
    }

    // 'tmin'
    node = btree_FindNode(ent_data(e5), RLAB_NAME_GRAMPC_OPTS_THORMIN);
    if (node)
    {
      ep = var_ent(node);
      if (ent_type (ep) == MATRIX_DENSE_REAL)
      {
        tmin = class_double(ep);
      }
    }

    // 'tmax'
    node = btree_FindNode(ent_data(e5), RLAB_NAME_GRAMPC_OPTS_THORMAX);
    if (node)
    {
      ep = var_ent(node);
      if (ent_type (ep) == MATRIX_DENSE_REAL)
      {
        tmax = class_double(ep);
      }
    }

    // 'N_hor'
    node = btree_FindNode(ent_data(e5), RLAB_NAME_GRAMPC_OPTS_NHOR);
    if (node)
    {
      ep = var_ent(node);
      if (ent_type (ep) == MATRIX_DENSE_REAL)
      {
        Nhor = class_int(ep);
      }
    }

    // 'MaxIter'
    node = btree_FindNode(ent_data(e5), RLAB_NAME_GRAMPC_OPTS_MAXITER);
    if (node)
    {
      ep = var_ent(node);
      if (ent_type (ep) == MATRIX_DENSE_REAL)
      {
        MaxIter = class_int(ep);
        if (MaxIter < 1)
        {
          MaxIter = 1;
        }
      }
    }

    // 'MaxMultIter'
    node = btree_FindNode(ent_data(e5), RLAB_NAME_GRAMPC_OPTS_MAXMULTITER);
    if (node)
    {
      ep = var_ent(node);
      if (ent_type (ep) == MATRIX_DENSE_REAL)
      {
        MaxMultIter = class_int(ep);
        if (MaxMultIter < 1)
        {
          MaxMultIter = 1;
        }
      }
    }

    // 'MaxGradIter'
    node = btree_FindNode(ent_data(e5), RLAB_NAME_GRAMPC_OPTS_MAXGRADITER);
    if (node)
    {
      ep = var_ent(node);
      if (ent_type (ep) == MATRIX_DENSE_REAL)
      {
        MaxGradIter = class_int(ep);
        if (MaxGradIter < 1)
        {
          MaxGradIter = 1;
        }
      }
    }

    // 'ConstraintsAbsTol'
    node = btree_FindNode(ent_data(e5), RLAB_NAME_GRAMPC_OPTS_CONSTRAINTSABSTOL);
    if (node)
    {
      ep = var_ent(node);
      if (ent_type (ep) == MATRIX_DENSE_REAL)
      {
        cat_mdr = ent_data(ep);
      }
    }

    // 'PenaltyMin'
    node = btree_FindNode(ent_data(e5), RLAB_NAME_GRAMPC_OPTS_PENMIN);
    if (node)
    {
      ep = var_ent(node);
      if (ent_type (ep) == MATRIX_DENSE_REAL)
      {
        penmin = class_double(ep);
      }
    }

    // 'PenaltyMax'
    node = btree_FindNode(ent_data(e5), RLAB_NAME_GRAMPC_OPTS_PENMAX);
    if (node)
    {
      ep = var_ent(node);
      if (ent_type (ep) == MATRIX_DENSE_REAL)
      {
        penmax = class_double(ep);
      }
    }

    // 'PenaltyIncreaseFactor'
    node = btree_FindNode(ent_data(e5), RLAB_NAME_GRAMPC_OPTS_PENINCFAC);
    if (node)
    {
      ep = var_ent(node);
      if (ent_type (ep) == MATRIX_DENSE_REAL)
      {
        penincfac = class_double(ep);
        if (penincfac < 1)
        {
          penincfac = 1;
        }
      }
    }

    // 'PenaltyDecreaseFactor'
    node = btree_FindNode(ent_data(e5), RLAB_NAME_GRAMPC_OPTS_PENDECFAC);
    if (node)
    {
      ep = var_ent(node);
      if (ent_type (ep) == MATRIX_DENSE_REAL)
      {
        pendecfac = class_double(ep);
        if (pendecfac > 1)
        {
          pendecfac = 1;
        }
      }
    }

    // 'PenaltyThreshold'
    node = btree_FindNode(ent_data(e5), RLAB_NAME_GRAMPC_OPTS_PENTHR);
    if (node)
    {
      ep = var_ent(node);
      if (ent_type (ep) == MATRIX_DENSE_REAL)
      {
        penthr = class_double(ep);
        if (penthr <  0)
        {
          penthr = 0;
        }
      }
    }

    // 'AugLagUpdateGradientRelTol'
    node = btree_FindNode(ent_data(e5), RLAB_NAME_GRAMPC_OPTS_AUGLAGUPDATEGRADIENTRELTOL);
    if (node)
    {
      ep = var_ent(node);
      if (ent_type (ep) == MATRIX_DENSE_REAL)
      {
        aur = class_double(ep);
        if (aur <  0)
        {
          aur = 1e-2;
        }
        else if (aur > 1)
        {
          aur = 1;
        }
      }
    }

    // 'ConvergenceGradientRelTol'
    node = btree_FindNode(ent_data(e5), RLAB_NAME_GRAMPC_OPTS_GRT_CONV);
    if (node)
    {
      ep = var_ent(node);
      if (ent_type (ep) == MATRIX_DENSE_REAL)
      {
        cgr = class_double(ep);
        if (cgr <  0)
        {
          cgr = 0;
        }
        else if (cgr > 1)
        {
          cgr = 1;
        }
      }
    }

    // 'IntegratorRelTol'
    node = btree_FindNode(ent_data(e5), RLAB_NAME_GRAMPC_OPTS_INT_RELTOL);
    if (node)
    {
      ep = var_ent(node);
      if (ent_type (ep) == MATRIX_DENSE_REAL)
      {
        reltol = class_double(ep);
      }
    }

    // 'IntegratorAbsTol'
    node = btree_FindNode(ent_data(e5), RLAB_NAME_GRAMPC_OPTS_INT_ABSTOL);
    if (node)
    {
      ep = var_ent(node);
      if (ent_type (ep) == MATRIX_DENSE_REAL)
      {
        abstol = class_double(ep);
      }
    }

    // 'IntegratorMinStepSize'
    node = btree_FindNode(ent_data(e5), RLAB_NAME_GRAMPC_OPTS_INT_STEP_MINSIZE);
    if (node)
    {
      ep = var_ent(node);
      if (ent_type (ep) == MATRIX_DENSE_REAL)
      {
        minstep = class_double(ep);
      }
    }

     // 'IntegratorMaxSteps'
    node = btree_FindNode(ent_data(e5), RLAB_NAME_GRAMPC_OPTS_INT_STEP_MAXNUMBER);
    if (node)
    {
      ep = var_ent(node);
      if (ent_type (ep) == MATRIX_DENSE_REAL)
      {
        maxi = class_double(ep);
      }
    }

     // 'OptimTime'
    node = btree_FindNode(ent_data(e5), RLAB_NAME_GRAMPC_OPTS_OPTIMTIME);
    if (node)
    {
      ep = var_ent(node);
      if (ent_type (ep) == MATRIX_DENSE_REAL)
      {
        optimtime = class_double(ep);
      }
      else if (ent_type (ep) == MATRIX_DENSE_STRING)
      {
        char *ans = class_char_pointer(ep);
        if ( !strcmp(ans, "on") || !strcmp(ans, "ON"))
        {
          optimtime = 1;
        }
      }
    }

     // 'OptimTimeLineSearchFactor'
    node = btree_FindNode(ent_data(e5), RLAB_NAME_GRAMPC_OPTS_OPTIMTIME_LSF);
    if (node)
    {
      ep = var_ent(node);
      if (ent_type (ep) == MATRIX_DENSE_REAL)
      {
        optimtime_lsf = class_double(ep);
        if (optimtime_lsf < 0 )
        {
          optimtime_lsf = 1.0;
        }
        else
        {
          optimtime = 1;
        }
      }
    }

     // 'LineSearchInit'
    node = btree_FindNode(ent_data(e5), RLAB_NAME_GRAMPC_OPTS_LINESEARCH_INIT);
    if (node)
    {
      ep = var_ent(node);
      if (ent_type (ep) == MATRIX_DENSE_REAL)
      {
        linesearchinit = class_double(ep);
      }
    }

    // 'LineSearchMin'
    node = btree_FindNode(ent_data(e5), RLAB_NAME_GRAMPC_OPTS_LINESEARCH_MIN);
    if (node)
    {
      ep = var_ent(node);
      if (ent_type (ep) == MATRIX_DENSE_REAL)
      {
        linesearchmin = class_double(ep);
      }
    }

       // 'LineSearchMax'
    node = btree_FindNode(ent_data(e5), RLAB_NAME_GRAMPC_OPTS_LINESEARCH_MAX);
    if (node)
    {
      ep = var_ent(node);
      if (ent_type (ep) == MATRIX_DENSE_REAL)
      {
        linesearchmax = class_double(ep);
      }
    }

    // 'LineSearchAdaptAbsTol'
    node = btree_FindNode(ent_data(e5), RLAB_NAME_GRAMPC_OPTS_LINESEARCH_AAT);
    if (node)
    {
      ep = var_ent(node);
      if (ent_type (ep) == MATRIX_DENSE_REAL)
      {
        linesearchaat = class_double(ep);
      }
    }

    // 'LineSearchIntervalTol'
    node = btree_FindNode(ent_data(e5), RLAB_NAME_GRAMPC_OPTS_LINESEARCH_IT);
    if (node)
    {
      ep = var_ent(node);
      if (ent_type (ep) == MATRIX_DENSE_REAL)
      {
        linesearchit = class_double(ep);
        if ((linesearchit <= 0) || (linesearchit >= 0.5))
        {
          linesearchit = 0.1;
        }
      }
    }

    // 'LineSearchIntervalFactor'
    node = btree_FindNode(ent_data(e5), RLAB_NAME_GRAMPC_OPTS_LINESEARCH_IF);
    if (node)
    {
      ep = var_ent(node);
      if (ent_type (ep) == MATRIX_DENSE_REAL)
      {
        linesearchif = class_double(ep);
        if ((linesearchif <= 0) || (linesearchif >= 1))
        {
          linesearchif = 0.85;
        }
      }
    }

    // 'estim_penmin'
    node = btree_FindNode(ent_data(e5), RLAB_NAME_GRAMPC_OPTS_ESTIMATE_PENMIN);
    if (node)
    {
      ep = var_ent(node);
      if (ent_type (ep) == MATRIX_DENSE_REAL)
      {
        estpenmin = (class_double(ep) > 0);
      }
    }

    // standard output
    RLABCODE_PROCESS_BTREE_ENTRY_S(e5,node,RLAB_NAME_GEN_STDOUT,outs,1,0);
  }

  //
  // check defaults:
  //
  if (isnand(dt) || (dt<=0.0))
  {
    printf(THIS_SOLVER ": entry 'dt=%g' is incorrect: Using default value 1e-3!", dt);
    dt = 1e-3;
  }

  if (isnand(t0))
  {
    printf(THIS_SOLVER ": entry 't0=%g' is incorrect: Using default value 0!", t0);
    t0 = 0;
  }

  if (isnand(thor) || (thor<=0.0))
  {
    thor = gslrngf_();
    printf(THIS_SOLVER ": entry 'thor=%g' is incorrect: "
            "Using random value from default RNG instead, and you go and read manual!\n"
                , thor);
  }

  // start populating gdata
  gdata.ext_f_args.f_func   = NULL;
  gdata.ext_f_args.df_func  = NULL;
  gdata.ext_f_args.arg1   = NULL;
  gdata.ext_f_args.arg2   = NULL;
  gdata.ext_f_args.arg3   = NULL;
  gdata.ext_f_args.arg4   = NULL;
  gdata.ext_f_args.arg5   = NULL;
  gdata.ext_f_args.arg6   = NULL;
  gdata.ext_f_args.arg7   = NULL;
  gdata.ext_f_args.arg8   = NULL;
  gdata.ext_f_args.n_args = 0;
  gdata.ext_f_args.d = NULL;

  // All functions calling structure:
  //
  //    rval = func(t,x,u/,p/)
  //
  // t
  t_mdr = mdr_CreateEmpty (1,1);
  t_ent = ent_Assign_Rlab_MDR(t_mdr);
  gdata.ext_f_args.arg1   = t_ent;
  gdata.ext_f_args.n_args = 1;
  //
  // x
  x_mdr = mdr_CreateEmpty(MNR(x0),MNC(x0));
  x_ent = ent_Assign_Rlab_MDR(x_mdr);
  gdata.ext_f_args.arg2   = x_ent;
  gdata.ext_f_args.n_args = 2;
  //
  // u
  u_mdr = mdr_CreateEmpty(MNR(u0),MNC(u0));
  u_ent = ent_Assign_Rlab_MDR(u_mdr);
  gdata.ext_f_args.arg3   = u_ent;
  gdata.ext_f_args.n_args = 3;
  //
  // p: optimizable parameters
  if (gdata.Np > 0)
  {
    gdata.ext_f_args.arg4 = p_ent;
    gdata.ext_f_args.n_args = gdata.ext_f_args.n_args + 1;
  }
  //
  // s: un-optimizable parameters
  if (ent_type(e2)!=UNDEF)
  {
    if (gdata.Np > 0)
    {
      gdata.ext_f_args.arg5   =   e2;
    }
    else
    {
      gdata.ext_f_args.arg4   =   e2;
    }
    gdata.ext_f_args.n_args = -(gdata.ext_f_args.n_args + 1);
  }

  // overall, the non-cost functions will have 3-5 parameters
  // the cost functions will have two additional parameters, which
  // we put in arg7 and arg8
  //
  // x_des
  x_des_mdr = mdr_CreateEmpty(MNR(x0),MNC(x0));
  x_des_ent = ent_Assign_Rlab_MDR(x_des_mdr);
  gdata.ext_f_args.arg7   = x_des_ent;
  // u_des
  u_des_mdr = mdr_CreateEmpty(MNR(u0),MNC(u0));
  u_des_ent = ent_Assign_Rlab_MDR(u_des_mdr);
  gdata.ext_f_args.arg8   = u_des_ent;

  // call functions h,hT,g,gT once
  // to find out the dimensions of equality and inequality constraints
  // h:
  if (gdata.ent_fn_hfct)
  {
    MDPTR(t_mdr) = &t0;
    ent_IncRef(gdata.ext_f_args.arg1);
    MDPTR(x_mdr) = MDPTR(x0);
    ent_IncRef(gdata.ext_f_args.arg2);
    MDPTR(u_mdr) = MDPTR(u0);
    ent_IncRef(gdata.ext_f_args.arg3);
    switch(gdata.ext_f_args.n_args)
    {
      case 3:
        // hfct = hfct(t, x, u)
        rent = ent_call_rlab_script_with_args(gdata.ent_fn_hfct, 3,
                                              gdata.ext_f_args.arg1,
                                              gdata.ext_f_args.arg2,
                                              gdata.ext_f_args.arg3);
        break;

      case -4:
        // hfct = hfct(t, x, u, s)
        ent_IncRef(gdata.ext_f_args.arg4);
        rent = ent_call_rlab_script_with_args(gdata.ent_fn_hfct, 4,
                                              gdata.ext_f_args.arg1,
                                              gdata.ext_f_args.arg2,
                                              gdata.ext_f_args.arg3,
                                              gdata.ext_f_args.arg4);
        ent_DecRef(gdata.ext_f_args.arg4);
        break;

      case  4:
        // hfct = hfct(t, x, u, p),
        ent_IncRef(gdata.ext_f_args.arg4);
        rent = ent_call_rlab_script_with_args(gdata.ent_fn_hfct, 4,
                                              gdata.ext_f_args.arg1,
                                              gdata.ext_f_args.arg2,
                                              gdata.ext_f_args.arg3,
                                              gdata.ext_f_args.arg4);
        ent_DecRef(gdata.ext_f_args.arg4);
        break;

      case -5:
        ent_IncRef(gdata.ext_f_args.arg4);
        ent_IncRef(gdata.ext_f_args.arg5);
        // hfct = hfct(t, x, u, p, s)
        rent = ent_call_rlab_script_with_args(gdata.ent_fn_hfct, 5,
                                              gdata.ext_f_args.arg1,
                                              gdata.ext_f_args.arg2,
                                              gdata.ext_f_args.arg3,
                                              gdata.ext_f_args.arg4,
                                              gdata.ext_f_args.arg5);
        ent_DecRef(gdata.ext_f_args.arg4);
        ent_DecRef(gdata.ext_f_args.arg5);
        break;
    }
    ent_DecRef(gdata.ext_f_args.arg1);
    ent_DecRef(gdata.ext_f_args.arg2);
    ent_DecRef(gdata.ext_f_args.arg3);
    if (rent)
    {
      if (ent_type(rent)==MATRIX_DENSE_REAL)
      {
        MDR *rval = ent_data(rent);
        Nh = gdata.Nh = SIZE(rval);
        if (gdata.Nh < 1)
        {
          Nh = gdata.Nh = 0;
          gdata.ent_fn_hfct = NULL;
          gdata.have_hfct = 0;
          printf(THIS_SOLVER ": Inconsistent problem: If dim(hfct)=0 it need not be specified\n");
        }
      }
      ent_Clean(rent);
    }
  }
  // hT:
  if (gdata.ent_fn_hTfct)
  {
    MDPTR(t_mdr) = &thor;
    ent_IncRef(gdata.ext_f_args.arg1);
    MDPTR(x_mdr) = MDPTR(x0);
    ent_IncRef(gdata.ext_f_args.arg2);
    switch(gdata.ext_f_args.n_args)
    {
      case 3:
        // hTfct = hTfct(T, x)
        rent = ent_call_rlab_script_with_args(gdata.ent_fn_hTfct, 2,
                                          gdata.ext_f_args.arg1,
                                          gdata.ext_f_args.arg2);
        break;

      case -4:
        // hTfct = hTfct(t, x, s)
        ent_IncRef(gdata.ext_f_args.arg4);
        rent = ent_call_rlab_script_with_args(gdata.ent_fn_hTfct, 3,
                                              gdata.ext_f_args.arg1,
                                              gdata.ext_f_args.arg2,
                                              gdata.ext_f_args.arg4);
        ent_DecRef(gdata.ext_f_args.arg4);
        break;

      case  4:
        // hTfct = hTfct(t, x, p)
        ent_IncRef(gdata.ext_f_args.arg4);
        rent = ent_call_rlab_script_with_args(gdata.ent_fn_hTfct, 3,
                                          gdata.ext_f_args.arg1,
                                          gdata.ext_f_args.arg2,
                                          gdata.ext_f_args.arg4);
        ent_DecRef(gdata.ext_f_args.arg4);
        break;

      case -5:
        // hTfct = hTfct(t, x, u, p, s)
        ent_IncRef(gdata.ext_f_args.arg4);
        ent_IncRef(gdata.ext_f_args.arg5);
        rent = ent_call_rlab_script_with_args(gdata.ent_fn_hTfct, 5,
                                          gdata.ext_f_args.arg1,
                                          gdata.ext_f_args.arg2,
                                          gdata.ext_f_args.arg3,
                                          gdata.ext_f_args.arg4,
                                          gdata.ext_f_args.arg5);
        ent_DecRef(gdata.ext_f_args.arg5);
        ent_DecRef(gdata.ext_f_args.arg4);
        break;
    }
    if (rent)
    {
      if (ent_type(rent)==MATRIX_DENSE_REAL)
      {
        MDR *rval = ent_data(rent);
        Nht = gdata.Nht = SIZE(rval);
        if (gdata.Nht < 1)
        {
          Nht = gdata.Nht = 0;
          gdata.ent_fn_hTfct = NULL;
          gdata.have_hTfct = 0;
          printf(THIS_SOLVER ": Inconsistent problem: If dim(hTfct)=0 it need not be specified!\n");
        }
      }
      ent_Clean(rent);
    }
  }
  // g:
  if (gdata.ent_fn_gfct)
  {
    MDPTR(t_mdr) = &t0;
    ent_IncRef(gdata.ext_f_args.arg1);
    MDPTR(x_mdr) = MDPTR(x0);
    ent_IncRef(gdata.ext_f_args.arg2);
    MDPTR(u_mdr) = MDPTR(u0);
    ent_IncRef(gdata.ext_f_args.arg3);
    switch(gdata.ext_f_args.n_args)
    {
      case 3:
        // gfct = gfct(t, x, u)
        rent = ent_call_rlab_script_with_args(gdata.ent_fn_gfct, 3,
                                          gdata.ext_f_args.arg1,
                                          gdata.ext_f_args.arg2,
                                          gdata.ext_f_args.arg3);
        break;

      case -4:
      case  4:
        // gfct = gfct(t, x, u, p), or
        // gfct = gfct(t, x, u, s)
        ent_IncRef(gdata.ext_f_args.arg4);
        rent = ent_call_rlab_script_with_args(gdata.ent_fn_gfct, 4,
                                          gdata.ext_f_args.arg1,
                                          gdata.ext_f_args.arg2,
                                          gdata.ext_f_args.arg3,
                                          gdata.ext_f_args.arg4);
        ent_DecRef(gdata.ext_f_args.arg4);
        break;

      case -5:
        // gfct = gfct(t, x, u, p, s)
        ent_IncRef(gdata.ext_f_args.arg4);
        ent_IncRef(gdata.ext_f_args.arg5);
        rent = ent_call_rlab_script_with_args(gdata.ent_fn_gfct, 5,
                                          gdata.ext_f_args.arg1,
                                          gdata.ext_f_args.arg2,
                                          gdata.ext_f_args.arg3,
                                          gdata.ext_f_args.arg4,
                                          gdata.ext_f_args.arg5);
        ent_DecRef(gdata.ext_f_args.arg4);
        ent_DecRef(gdata.ext_f_args.arg5);
        break;
    }
    ent_DecRef(gdata.ext_f_args.arg1);
    ent_DecRef(gdata.ext_f_args.arg2);
    ent_DecRef(gdata.ext_f_args.arg3);
    if (rent)
    {
      if (ent_type(rent)==MATRIX_DENSE_REAL)
      {
        MDR *rval = ent_data(rent);
        Ng = gdata.Ng = SIZE(rval);
        if (gdata.Ng < 1)
        {
          Ng = gdata.Ng = 0;
          gdata.ent_fn_gfct = NULL;
          gdata.have_gfct = 0;
          printf(THIS_SOLVER ": Inconsistent problem: If dim(gfct)=0 it need not be specified\n");
        }
      }
    }
    ent_Clean(rent);
  }
  // gT:
  if (gdata.ent_fn_gTfct)
  {
    MDPTR(t_mdr) = &thor;
    ent_IncRef(gdata.ext_f_args.arg1);
    MDPTR(x_mdr) = MDPTR(x0);
    ent_IncRef(gdata.ext_f_args.arg2);
    switch(gdata.ext_f_args.n_args)
    {
      case 3:
        // gTfct = gTfct(T, x)
        rent = ent_call_rlab_script_with_args(gdata.ent_fn_gTfct, 2,
                                          gdata.ext_f_args.arg1,
                                          gdata.ext_f_args.arg2);
        break;

      case -4:
      case  4:
        // gTfct = gTfct(t, x, p), or
        // gTfct = gTfct(t, x, s)
        ent_IncRef(gdata.ext_f_args.arg4);
        rent = ent_call_rlab_script_with_args(gdata.ent_fn_gTfct, 3,
                                          gdata.ext_f_args.arg1,
                                          gdata.ext_f_args.arg2,
                                          gdata.ext_f_args.arg4);
        ent_DecRef(gdata.ext_f_args.arg4);
        break;

      case -5:
        // gTfct = gTfct(t, x, p, s)
        ent_IncRef(gdata.ext_f_args.arg4);
        ent_IncRef(gdata.ext_f_args.arg5);
        rent = ent_call_rlab_script_with_args(gdata.ent_fn_gTfct, 4,
                                          gdata.ext_f_args.arg1,
                                          gdata.ext_f_args.arg2,
                                          gdata.ext_f_args.arg4,
                                          gdata.ext_f_args.arg5);
        ent_DecRef(gdata.ext_f_args.arg4);
        ent_DecRef(gdata.ext_f_args.arg5);
        break;
    }
    ent_DecRef(gdata.ext_f_args.arg1);
    ent_DecRef(gdata.ext_f_args.arg2);
    if (rent)
    {
      if (ent_type(rent)==MATRIX_DENSE_REAL)
      {
        MDR *rval = ent_data(rent);
        Ngt = gdata.Ngt = SIZE(rval);
        if (gdata.Ngt < 1)
        {
          Ngt = gdata.Ngt = 0;
          gdata.ent_fn_gTfct = NULL;
          gdata.have_gTfct = 0;
          printf(THIS_SOLVER ": Inconsistent problem: If dim(gTfct)=0 it need not be specified\n");
        }
      }
      ent_Clean(rent);
    }
  }

  if (SIZE(cat_mdr) > 0)
  {
    Nc = gdata.Nc = Ng + Nh + Ngt + Nht;
    cat = mdr_Create(1,Nc);
    for (i=0; i<Nc; i++)
    {
      MdrV0(cat,i) = mdrV0_safe(cat_mdr, i);
    }
  }

  //
  // lets grampc-it!!!
  //
  grampc_init(&grampc, (typeUSERPARAM *) &gdata);

  grampc_setparam_real(grampc, "t0", t0);
  grampc_setparam_real(grampc, "dt", dt);
  grampc_setparam_real(grampc, "Thor", thor);
  grampc_setopt_int   (grampc, "Nhor", Nhor);

  // optimize end time?
  grampc_setopt_string(grampc, "OptimTime", "off");
  grampc_setparam_real(grampc, "Tmax", thor);
  grampc_setparam_real(grampc, "Tmin", thor);
  if (optimtime)
  {
    grampc_setopt_string(grampc, "OptimTime", "on");
    grampc_setopt_real(grampc, "OptimTimeLineSearchFactor", optimtime_lsf);
    if (!isnand(tmax))
      grampc_setparam_real(grampc, "Tmax", tmax);

    if (!isnand(tmin))
      grampc_setparam_real(grampc, "Tmin", tmin);
  }

  grampc_setopt_string(grampc, "ShiftControl", "off");
  if (ShiftControl)
  {
    grampc_setopt_string(grampc, "ShiftControl", "on");
  }

  grampc_setopt_string(grampc, "OptimControl", "on");
  if (!OptimControl)
  {
    grampc_setopt_string(grampc, "OptimControl", "off");
  }

  //
  // Initial values:
  //

  // initial state: X0
  grampc_setparam_real_vector(grampc, "x0",   MDPTR(x0));

  // desired state: X
  if (xdes_mdr)
    grampc_setparam_real_vector(grampc, "xdes", MDPTR(xdes_mdr));

  // initial control: U0
  grampc_setparam_real_vector(grampc, "u0",   MDPTR(u0));

  // desired control
  if (udes_mdr)
    grampc_setparam_real_vector(grampc, "udes", MDPTR(udes_mdr));

  // control limits: UMAX
  grampc_setparam_real_vector(grampc, "umax", MDPTR(umax));

  // control limit: UMIN
  grampc_setparam_real_vector(grampc, "umin", MDPTR(umin));

  // parameters: ones that can be optimized on if user wishes so
  grampc_setopt_string(grampc, "OptimParam", "off");
  if (Np > 0)
  {
    grampc_setopt_string(grampc, "OptimParam", "on");
    grampc_setparam_real_vector(grampc, "p0", MDPTR(p_mdr));
    if (p_mdr_min)
    {
      grampc_setparam_real_vector(grampc, "pmin", MDPTR(p_mdr_min));
    }
    if (p_mdr_max)
    {
      grampc_setparam_real_vector(grampc, "pmax", MDPTR(p_mdr_max));
    }
  }

  if (MaxMultIter > 0)
    grampc_setopt_int(grampc, "MaxMultIter", MaxMultIter);

  if (aur>0)
    grampc_setopt_real(grampc, "AugLagUpdateGradientRelTol", aur);

  if (cat != NULL)
  {
    grampc_setopt_real_vector(grampc, "ConstraintsAbsTol", MDPTR(cat));
  }

  if (MaxGradIter > 0)
    grampc_setopt_int(grampc, "MaxGradIter", MaxGradIter);

  grampc_setopt_real(grampc, "PenaltyMin", penmin);
  grampc_setopt_real(grampc, "PenaltyMax", penmax);
  grampc_setopt_real(grampc, "PenaltyIncreaseFactor", penincfac);
  grampc_setopt_real(grampc, "PenaltyDecreaseFactor", pendecfac);
  grampc_setopt_real(grampc, "PenaltyIncreaseThreshold", penthr);
  grampc_setopt_real(grampc, "IntegratorRelTol", reltol);
  grampc_setopt_real(grampc, "IntegratorAbsTol", abstol);
  grampc_setopt_real(grampc, "IntegratorMinStepSize", minstep);
  grampc_setopt_int (grampc, "IntegratorMaxSteps", maxi);

  //
  grampc_setopt_string(grampc, "ConvergenceCheck", "off");
  if (cgr > 0)
  {
    grampc_setopt_string(grampc, "ConvergenceCheck", "on");
    grampc_setopt_real(grampc, "ConvergenceGradientRelTol", cgr);
  }

  // about provided constraint functions
  grampc_setopt_string(grampc, "IntegralCost", "off");
  if (gdata.have_lfct)
  {
    grampc_setopt_string(grampc, "IntegralCost", "on");
  }
  grampc_setopt_string(grampc, "TerminalCost", "off");
  if (gdata.have_Vfct)
  {
    grampc_setopt_string(grampc, "TerminalCost", "on");
  }
  //
  grampc_setopt_string(grampc, "EqualityConstraints", "off");
  if (gdata.have_gfct)
  {
    grampc_setopt_string(grampc, "EqualityConstraints", "on");
  }
  //
  grampc_setopt_string(grampc, "InequalityConstraints", "off");
  if (gdata.have_hfct)
  {
    grampc_setopt_string(grampc, "InequalityConstraints", "on");
  }
  //
  grampc_setopt_string(grampc, "TerminalEqualityConstraints", "off");
  if (gdata.have_gTfct)
  {
    grampc_setopt_string(grampc, "TerminalEqualityConstraints", "on");
  }
  //
  grampc_setopt_string(grampc, "TerminalInequalityConstraints", "off");
  if (gdata.have_hTfct)
  {
    grampc_setopt_string(grampc, "TerminalInequalityConstraints", "on");
  }

  // line search
  if (linesearchinit>0)
  {
    grampc_setopt_real(grampc, "LineSearchInit", linesearchinit);
  }
  if (linesearchmin>0)
  {
    grampc_setopt_real(grampc, "LineSearchMin", linesearchmin);
  }
  if (linesearchmax>0)
  {
    grampc_setopt_real(grampc, "LineSearchMax", linesearchmax);
  }
  if (linesearchaat>0)
  {
    grampc_setopt_real(grampc, "LineSearchAdaptAbsTol", linesearchaat);
  }

  if (linesearchit>0)
  {
    grampc_setopt_real(grampc, "LineSearchIntervalTol", linesearchit);
  }
  if (linesearchif>0)
  {
    grampc_setopt_real(grampc, "LineSearchIntervalFactor", linesearchif);
  }

  grampc_setopt_string(grampc, "ScaleProblem", "off");
  if (scaleproblem)
  {
    grampc_setopt_string(grampc, "ScaleProblem", "on");
  }


  if (estpenmin)
  {
    grampc_estim_penmin(grampc, estpenmin);
  }

  if (outs)
  {
    fptr = fopen (outs, "a");
  }

  if (fptr)
  {
    // write down boring info
    fprintf (fptr, "RLaB: GRAMPC solver for MPC optimization\n");
    fprintf (fptr, "RLaB: Messages from the solver follow:\n");
    // start the timer
    timer = clock ();
    //
    grampc_fprintopt(fptr, grampc);
    grampc_fprintparam(fptr, grampc);
  }


  if (tsim > 0)
  {
    //
    // IVP: tsim is provided
    //
    Ntm = tsim / dt;

    // return values:
    //  solutions (Initial Value Problem):
    x_sol_mdr = mdr_Create(Ntm+1, SIZE(x0)+1);
    mdr_Nan(x_sol_mdr);
    u_sol_mdr = mdr_Create(Ntm+1, SIZE(u0)+1);
    mdr_Nan(u_sol_mdr);
    c_sol_mdr = mdr_Create(Ntm+1, 3);
    mdr_Nan(c_sol_mdr);

    double *rwsReferenceIntegration=GC_MALLOC(method * Nx * sizeof(double));

    // x(t0)
    Mdr0(x_sol_mdr,0,0) = t0;
    for (j=0; j<Nx; j++)
    {
      Mdr0(x_sol_mdr,0,1+j) = MdrV0(x0,j);
    }
    // u(t0)
    Mdr0(u_sol_mdr,0,0) = t0;
    for (j=0; j<Nu; j++)
    {
      Mdr0(u_sol_mdr,0,1+j) = MdrV0(u0,j);
    }
    // c(t0)
    Mdr0(c_sol_mdr,0,0) = t0;
    Mdr0(c_sol_mdr,0,1) = create_nan();
    Mdr0(c_sol_mdr,0,2) = create_nan();

    t = t0;
    for (i=0; i<Ntm; i++)
    {
      grampc_setparam_real(grampc, "t0", t);

      grampc_run(grampc);

      if (grampc->sol->status > 0)
      {
        if (fptr)
        {
          if (grampc_fprintstatus(fptr, grampc->sol->status, STATUS_LEVEL_ERROR))
          {
            fprintf(fptr, "at iteration %i:\n -----\n", i);
          }
        }
        else
        {
          if (grampc_printstatus(grampc->sol->status, STATUS_LEVEL_ERROR))
          {
            printf("at iteration %i:\n -----\n", i);
          }
        }
      }

      // x next
      //    reference integration of the system: since grampc->sol->xnext is only an interpolated value:
      if (method == 1 || method == 2)
      {
        // method = 1: aka Euler forward scheme
        // method = 2: aka Heun scheme
        ffct(rwsReferenceIntegration, t, grampc->param->x0, grampc->sol->unext,
             grampc->sol->pnext, grampc->userparam);
        for (j=0; j<Nx; j++)
        {
          grampc->sol->xnext[j] = grampc->param->x0[j] + dt * rwsReferenceIntegration[j];
        }
        // next t0: should be there anyway
        t = t + dt;
        if (method == 2)
        {
          // method = 2: aka Heun scheme
          ffct(rwsReferenceIntegration + Nx, t, grampc->sol->xnext, grampc->sol->unext,
               grampc->sol->pnext, grampc->userparam);
          for (j=0; j<Nx; j++)
          {
            grampc->sol->xnext[j] = grampc->param->x0[j] + 0.5 * dt * (rwsReferenceIntegration[j]
                + rwsReferenceIntegration[j+Nx]);
          }
        }
      }

      Mdr0(x_sol_mdr,i+1,0) = t;
      for (j=0; j<Nx; j++)
      {
        Mdr0(x_sol_mdr,i+1,j+1) = grampc->sol->xnext[j];
      }

      grampc_setparam_real_vector(grampc, "x0", grampc->sol->xnext);

      // u next
      Mdr0(u_sol_mdr,i+1,0) = t;
      for (j=0; j<gdata.Nu; j++)
      {
        Mdr0(u_sol_mdr,i+1,1+j) = grampc->sol->unext[j];
      }
      //grampc_setparam_real_vector(grampc, "u0", grampc->sol->unext);

      // J
      Mdr0(c_sol_mdr,i+1,0) = t;
      Mdr0(c_sol_mdr,i+1,1) = grampc->sol->J[0];  // cost
      Mdr0(c_sol_mdr,i+1,2) = grampc->sol->J[1];  // augmented cost

      if (fptr)
      {
        fprintf(fptr, "GRAMPC: t=%14.6g, ", t);
        for (j=0; j<Nx; j++)
        {
          fprintf(fptr, "x[%i]=%14.6g, ", j+1, grampc->sol->xnext[j]);
        }
        for (j=0; j<Nu; j++)
        {
          fprintf(fptr, "u[%i]=%14.6g, ", j+1, grampc->sol->unext[j]);
        }
        fprintf(fptr, "J[1]=%14.6g, " , grampc->sol->J[0]);
        fprintf(fptr, "J[2]=%14.6g\n", grampc->sol->J[1]);
      }

      if (i_counttnexttmin)
      {
        icount++;
        if ((grampc->sol->Tnext - grampc->param->dt) <= grampc->param->Tmin)
        {
          grampc_setopt_string(grampc, "OptimTime", "off");
        }
        if (!((grampc->sol->Tnext - grampc->param->dt) >= grampc->param->Tmin) && (icount >= 2))
        {
          break;
        }
      }

    } /* for (i=0; i<Ntm; i++) */

    if (fptr)
    {
      fprintf (fptr, "RLaB: MPC optimization solver GRAMPC reports status %i\n", grampc->sol->status);
      timer = clock() - timer;
      fprintf (fptr, "RLaB: MPC optimization lasted %g sec.\n", timer / 1e6);
      fclose (fptr);
    }

    GC_FREE(rwsReferenceIntegration);

    // for initial value problems:
    //  sol:
    //    x
    //    u
    //    c
    bw_sol = btree_Create();
    install(bw_sol, RLAB_NAME_GRAMPC_OUTPUT_STATE,    ent_Assign_Rlab_MDR(x_sol_mdr));
    install(bw_sol, RLAB_NAME_GRAMPC_OUTPUT_CONTROL,  ent_Assign_Rlab_MDR(u_sol_mdr));
    install(bw_sol, RLAB_NAME_GRAMPC_OUTPUT_COST,     ent_Assign_Rlab_MDR(c_sol_mdr));
    install(bw, "sol", ent_Assign_Rlab_BTREE(bw_sol));

    goto _exit;

  } /* if (tsim > 0) */

  //
  // BVP: tsim is NOT provided
  //
  if (MaxIter <= 1)
  {
    printf (THIS_SOLVER ": entry MaxIter=%i does not provide sufficient number of "
          "iterations to converge: Exiting!\n", MaxIter);
    goto _exit;
  }

  int converged;

  for (i=0; i<MaxIter; i++)
  {
    converged = 1;

    grampc_run(grampc);

    if (grampc->sol->status > 0)
    {
      if (fptr)
      {
        if (grampc_fprintstatus(fptr, grampc->sol->status, STATUS_LEVEL_ERROR))
        {
          fprintf(fptr, "at iteration %i:\n -----\n", i);
        }
      }
      else
      {
        if (grampc_printstatus(grampc->sol->status, STATUS_LEVEL_ERROR))
        {
          printf("at iteration %i:\n -----\n", i);
        }
      }
    }

    if (fptr)
    {
      fprintf(fptr, "GRAMPC: iter=%8i: Convergence: ", i);
    }

    // do we check for convergence
    if (grampc->opt->ConvergenceCheck)
    {
      double z0, z1, dz;

      if (grampc->opt->OptimControl)
      {
        z0 = z1 = dz = 0.0;
        for (k=0; k<Nu; k++)
        {
          z0 = z0 + (grampc->rws->u[k])     * (grampc->rws->u[k]);
          z1 = z1 + (grampc->rws->uprev[k]) * (grampc->rws->uprev[k]);
          dz = dz + (grampc->rws->u[k] - grampc->rws->uprev[k]) * (grampc->rws->u[k] - grampc->rws->uprev[k]);
        }
        dz = sqrt(dz);
        z1 = MAX(sqrt(z1),sqrt(z0));

        if (dz > cgr * z1)
        {
          converged = 0;
        }

        if (fptr)
        {
          fprintf(fptr, "Control: %g, %g (%i)", dz, z1, converged);
        }
      }

      if (grampc->opt->OptimParam)
      {
        z0 = z1 = dz = 0.0;
        for (k=0; k<Np; k++)
        {
          z0 = z0 + (grampc->rws->p[k])     * (grampc->rws->p[k]);
          z1 = z1 + (grampc->rws->pprev[k]) * (grampc->rws->pprev[k]);
          dz = dz + (grampc->rws->p[k] - grampc->rws->pprev[k]) * (grampc->rws->p[k] - grampc->rws->pprev[k]);
        }
        dz = sqrt(dz);
        z1 = MAX(sqrt(z1),sqrt(z0));

        if (dz > cgr * z1)
        {
          converged = 0;
        }

        if (fptr)
        {
          fprintf(fptr, "Parameter: %g, %g (%i)", dz, z1, converged);
        }
      }

      if (grampc->opt->OptimTime)
      {
        z1 = MAX(grampc->rws->Tprev,grampc->rws->T);
        dz = ABS(z1-z0);
        if (dz > cgr * z1)
        {
          converged = 0;
        }
        if (fptr)
        {
          fprintf(fptr, "Terminal Time: %g, %g (%i)", dz, z1, converged);
        }
      }
    }

    if (fptr)
    {
      fprintf(fptr, "\n");
    }

    if (converged)
    {
      break;
    }

  } /* for (i=0; i<MaxIter; i++) */

  if (!converged)
  {
    if (fptr)
    {
      fprintf (fptr, "RLaB: MPC optimization solver GRAMPC FAILED TO CONVERGE in %i iterations\n",
               MaxIter);
      timer = clock() - timer;
      fprintf (fptr, "RLaB: MPC optimization lasted %g sec.\n", timer / 1e6);
      fclose (fptr);
    }
  }

  //  predictions are stored in 'pred' list
  x_pred_mdr = mdr_Create(Nhor, SIZE(x0)+1);
  u_pred_mdr = mdr_Create(Nhor, SIZE(u0)+1);
  c_pred_mdr = mdr_Create(Nhor, 4);

  // at each time step copy predicted solution from RWS
  for (j=0; j<Nhor; j++)
  {
    Mdr0(x_pred_mdr,j,0) = Mdr0(c_pred_mdr,j,0) = Mdr0(u_pred_mdr,j,0) = grampc->rws->t[j];
    for (k=0; k<Nx; k++)
    {
      Mdr0(x_pred_mdr,j,k+1) = grampc->rws->x[k + Nx * j];
    }
    for (k=0; k<Nu; k++)
    {
      Mdr0(u_pred_mdr,j,k+1) = grampc->rws->u[k + Nu * j];
    }
    for (k=0; k<3; k++)
    {
      Mdr0(c_pred_mdr,j,k+1) = grampc->rws->cfct[k + 3 * j];
    }
  }

//
//     // copy the solution for return
//     // x next
//     Mdr0(x_sol_mdr,i+1,0) = MdrV0(t_i,i+1);
//     for (j=0; j<gdata.Nx; j++)
//     {
//       Mdr0(x_sol_mdr,i+1,1+j) = grampc->sol->xnext[j];
//     }
//     grampc_setparam_real_vector(grampc, "x0", grampc->sol->xnext);
//
//     // u next
//     Mdr0(u_sol_mdr,i+1,0) = MdrV0(t_i,i+1);
//     for (j=0; j<gdata.Nu; j++)
//     {
//       Mdr0(u_sol_mdr,i+1,1+j) = grampc->sol->unext[j];
//     }
//
//     // J
//     Mdr0(c_sol_mdr,i+1,0) = MdrV0(t_i,i+1);
//     Mdr0(c_sol_mdr,i+1,1) = grampc->sol->J[0];
//     Mdr0(c_sol_mdr,i+1,2) = grampc->sol->J[1];
//
//     grampc_setparam_real_vector(grampc, "u0", grampc->sol->unext);
//
//     if (fptr)
//     {
//       fprintf(fptr, "GRAMPC: t=%14.6g, ", MdrV0(t_i,i+1));
//       fprintf(fptr, "tnext=%14.6g, ", grampc->sol->Tnext);
//       for (j=0; j<gdata.Nx; j++)
//       {
//         fprintf(fptr, "x[%i]=%14.6g, ", j+1, grampc->sol->xnext[j]);
//       }
//       for (j=0; j<gdata.Nu; j++)
//       {
//         fprintf(fptr, "u[%i]=%14.6g, ", j+1, grampc->sol->unext[j]);
//       }
//       fprintf(fptr, "J[1]=%14.6g, " , grampc->sol->J[0]);
//       fprintf(fptr, "J[2]=%14.6g\n", grampc->sol->J[1]);
//     }
//
//   } /* for (i=0; i<Ntm-1; i++) */
//

  //  pred:
  //    x
  //    u
  //    c
  bw_pred = btree_Create();
  install(bw_pred, RLAB_NAME_GRAMPC_OUTPUT_STATE,    ent_Assign_Rlab_MDR(x_pred_mdr));
  install(bw_pred, RLAB_NAME_GRAMPC_OUTPUT_CONTROL,  ent_Assign_Rlab_MDR(u_pred_mdr));
  install(bw_pred, RLAB_NAME_GRAMPC_OUTPUT_COST,     ent_Assign_Rlab_MDR(c_pred_mdr));
  install(bw, "pred", ent_Assign_Rlab_BTREE(bw_pred));


_exit:

  if (grampc)
    grampc_free(&grampc);

  // Clean Up rlab function variables:
  //  t
  MDPTR(t_mdr) = 0;
  ent_Destroy (t_ent);
  // x:
  MDPTR(x_mdr) = 0;
  ent_Destroy (x_ent);
  // u:
  MDPTR(u_mdr) = 0;
  ent_Destroy (u_ent);
  // x_des:
  MDPTR(x_des_mdr) = 0;
  ent_Destroy (x_des_ent);
  // u_des:
  MDPTR(u_des_mdr) = 0;
  ent_Destroy (u_des_ent);
  // umax, umin
  mdr_Destroy(umax);
  mdr_Destroy(umin);
  if (cat)
    mdr_Destroy(cat);

  //
  // Clean Up rlab solver inputs:
  //
  ent_Clean (e1);
  ent_Clean (e2);
  ent_Clean (e3);
  ent_Clean (e4);
  ent_Clean (e5);
  return ent_Assign_Rlab_BTREE(bw);
}




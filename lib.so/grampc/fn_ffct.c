/** System function f(t,x,u,p,userparam)
    ------------------------------------ **/

void ffct(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u,
          ctypeRNum *p, typeUSERPARAM *userparam)
{
  Ent *rent=0;
  GRAMPC_TABLE *gdata=(GRAMPC_TABLE *) userparam;
  if (!gdata->have_ffct)
  {
    return;
  }


  // pointer shuffle:
  int i;
  void *p_save=NULL;
  MDR *p_mdr=NULL;

  r_extern_func_args ext_f_args = (r_extern_func_args) gdata->ext_f_args;
  // t:
  MDR *t_mdr = ent_data(ext_f_args.arg1);
  MDPTR(t_mdr) = (void *) &t;
  // x:
  MDR *x_mdr = ent_data(ext_f_args.arg2);
  MDPTR(x_mdr) = (void *) x;
  // u:
  MDR *u_mdr = ent_data(ext_f_args.arg3);
  MDPTR(u_mdr) = (void *) u;

  // incref, call, then decref
  ent_IncRef(ext_f_args.arg1);
  ent_IncRef(ext_f_args.arg2);
  ent_IncRef(ext_f_args.arg3);
  switch(ext_f_args.n_args)
  {
    case 3:
      // ffct(t,x,u)
      rent = ent_call_rlab_script_3args(gdata->ent_fn_ffct,
                                        ext_f_args.arg1,
                                        ext_f_args.arg2,
                                        ext_f_args.arg3);
      break;

    case -4:
      // ffct(t,x,u,s)
      ent_IncRef(ext_f_args.arg4);
      rent = ent_call_rlab_script_4args (gdata->ent_fn_ffct,
                                         ext_f_args.arg1,
                                         ext_f_args.arg2,
                                         ext_f_args.arg3,
                                         ext_f_args.arg4);
      ent_DecRef(ext_f_args.arg4);
      break;

    case 4:
      // ffct(t,x,u,p)
      // p:
      p_mdr   = ent_data(ext_f_args.arg4);
      p_save  = MDRPTR(p_mdr);
      MDPTR(p_mdr) = (void *) p;
      ent_IncRef(ext_f_args.arg4);
      rent = ent_call_rlab_script_4args (gdata->ent_fn_ffct,
                                         ext_f_args.arg1,
                                         ext_f_args.arg2,
                                         ext_f_args.arg3,
                                         ext_f_args.arg4);
      ent_DecRef(ext_f_args.arg4);
      MDPTR(p_mdr) = p_save;
      break;

    case -5:
      // ffct(t,x,u,p,s)
      // p:
      p_mdr   = ent_data(ext_f_args.arg4);
      p_save  = MDRPTR(p_mdr);
      MDPTR(p_mdr) = (void *) p;
      ent_IncRef(ext_f_args.arg4);
      ent_IncRef(ext_f_args.arg5);
      rent = ent_call_rlab_script_5args (gdata->ent_fn_ffct,
                                         ext_f_args.arg1,
                                         ext_f_args.arg2,
                                         ext_f_args.arg3,
                                         ext_f_args.arg4,
                                         ext_f_args.arg5);
      ent_DecRef(ext_f_args.arg4);
      ent_DecRef(ext_f_args.arg5);
      MDPTR(p_mdr) = p_save;
      break;
  }
  ent_DecRef(ext_f_args.arg1);
  ent_DecRef(ext_f_args.arg2);
  ent_DecRef(ext_f_args.arg3);

  if (ent_type(rent)!=MATRIX_DENSE_REAL)
  {
    rerror (RLAB_ERROR_RHS_FUNC_MUST_RETURN_MDR);
  }

  MDR *retm = ent_data(rent);
  if (SIZE(retm)!=gdata->Nx)
  {
    fprintf(stderr, THIS_SOLVER ": [ffct] " RLAB_ERROR_RHS_FUNC_INCORRECT_DIM "\n");
    fprintf(stderr, THIS_SOLVER ": [ffct] Expected dimension %i, RHS dimension %i\n", gdata->Nx, SIZE(retm));
    fprintf(stderr, THIS_SOLVER ": [ffct] Check the RHS function: Cannot continue!\n");
    rerror ("odeiv");
  }

  for (i=0; i < gdata->Nx; i++)
  {
    out[i] = (typeRNum) MdrV0 (retm, i);
  }

  ent_Clean(rent);
  return;
}

/** Jacobian df/dx in vector form (column-wise) **/
void dfdx(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u,
          ctypeRNum *p, typeUSERPARAM *userparam)
{
  GRAMPC_TABLE *gdata=(GRAMPC_TABLE *) userparam;
  if (!gdata->have_dfdx)
  {
    return;
  }


  Ent *rent=0;
  MDR *p_mdr=NULL;
  void *p_save=NULL;

  // pointer shuffle:
  int i, j;
  r_extern_func_args ext_f_args = (r_extern_func_args) gdata->ext_f_args;
  // t:
  MDR *t_mdr = ent_data(ext_f_args.arg1);
  MDPTR(t_mdr) = (void *) &t;
  // x:
  MDR *x_mdr = ent_data(ext_f_args.arg2);
  MDPTR(x_mdr) = (void *) x;
  // u:
  MDR *u_mdr = ent_data(ext_f_args.arg3);
  MDPTR(u_mdr) = (void *) u;

  // incref, call, then decref
  ent_IncRef(ext_f_args.arg1);
  ent_IncRef(ext_f_args.arg2);
  ent_IncRef(ext_f_args.arg3);
  switch(ext_f_args.n_args)
  {
    case 3:
      // ffct(t,x,u)
      rent = ent_call_rlab_script_with_args(gdata->ent_fn_dfdx, 3,
                                        ext_f_args.arg1,
                                        ext_f_args.arg2,
                                        ext_f_args.arg3);
      break;

    case -4:
      // ffct(t,x,u,s)
      ent_IncRef(ext_f_args.arg4);
      rent = ent_call_rlab_script_with_args (gdata->ent_fn_dfdx, 4,
                                         ext_f_args.arg1,
                                         ext_f_args.arg2,
                                         ext_f_args.arg3,
                                         ext_f_args.arg4);
      ent_DecRef(ext_f_args.arg4);
      break;

    case 4:
      // ffct(t,x,u,p)
      // p:
      p_mdr   = ent_data(ext_f_args.arg4);
      p_save  = MDRPTR(p_mdr);
      MDPTR(p_mdr) = (void *) p;
      ent_IncRef(ext_f_args.arg4);
      rent = ent_call_rlab_script_with_args (gdata->ent_fn_dfdx, 4,
                                         ext_f_args.arg1,
                                         ext_f_args.arg2,
                                         ext_f_args.arg3,
                                         ext_f_args.arg4);
      ent_DecRef(ext_f_args.arg4);
      MDPTR(p_mdr) = p_save;
      break;

    case -5:
      // ffct(t,x,u,p,s)
      // p:
      p_mdr   = ent_data(ext_f_args.arg4);
      p_save  = MDRPTR(p_mdr);
      MDPTR(p_mdr) = (void *) p;
      ent_IncRef(ext_f_args.arg4);
      ent_IncRef(ext_f_args.arg5);
      rent = ent_call_rlab_script_with_args (gdata->ent_fn_dfdx, 5,
                                         ext_f_args.arg1,
                                         ext_f_args.arg2,
                                         ext_f_args.arg3,
                                         ext_f_args.arg4,
                                         ext_f_args.arg5);
      ent_DecRef(ext_f_args.arg4);
      ent_DecRef(ext_f_args.arg5);
      MDPTR(p_mdr) = p_save;
      break;
  }
  ent_DecRef(ext_f_args.arg1);
  ent_DecRef(ext_f_args.arg2);
  ent_DecRef(ext_f_args.arg3);

  if (ent_type(rent)!=MATRIX_DENSE_REAL)
  {
    rerror (RLAB_ERROR_RHS_FUNC_MUST_RETURN_MDR);
  }

  MDR *retm = ent_data(rent);
  if ((MNR(retm)!=gdata->Nx)||(MNC(retm)!=gdata->Nx))
  {
    fprintf(stderr, THIS_SOLVER ": [dfdx] " RLAB_ERROR_RHS_FUNC_INCORRECT_DIM "\n");
    fprintf(stderr, THIS_SOLVER ": [dfdx] Expected dimension %i, RHS dimension %i\n", gdata->Nx, SIZE(retm));
    fprintf(stderr, THIS_SOLVER ": [dfdx] Check the RHS function: Cannot continue!\n");
    rerror (THIS_SOLVER);
  }

  for (i=0; i < gdata->Nx; i++)
  {
    for (j=0; j < gdata->Nx; j++)
    {
      out[i + j*(gdata->Nx)] = (typeRNum) Mdr0 (retm, i, j);
    }
  }

  ent_Clean(rent);
  return;
}

/** Jacobian df/dx multiplied by vector vec, i.e. (df/dx)^T*vec or vec^T*(df/dx) **/
void dfdx_vec(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *vec,
              ctypeRNum *u, ctypeRNum *p, typeUSERPARAM *userparam)
{
  // call dfdx_{i,j} = d( f_i (x) ) / dx_j
  GRAMPC_TABLE *gdata=(GRAMPC_TABLE *) userparam;
  if (!gdata->have_dfdx)
  {
    return;
  }

  int i, j;
  MDR *retm = mdr_Create(gdata->Nx,gdata->Nx);
  dfdx((typeRNum *) MDRPTR(retm), t, x, u, p, userparam);
  for (j=0; j<gdata->Nx; j++)
  {
    out[j] = 0;
    for (i=0; i<gdata->Nx; i++)
    {
      if (Mdr0 (retm, i, j))
        out[j] = out[j] + vec[i] * Mdr0 (retm, i, j);
    }
  }

  mdr_Destroy(retm);

  return;
}

/** Jacobian df/dx in vector form (column-wise) **/
void dfdxtrans(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u,
               ctypeRNum *p, typeUSERPARAM *userparam)
{
  GRAMPC_TABLE *gdata=(GRAMPC_TABLE *) userparam;
  if (!gdata->have_dfdx)
  {
    return;
  }
  dfdx(out, t, x, u, p, userparam);
  md_transpose_insitu((unsigned char *)out, gdata->Nx, gdata->Nx, sizeof(double));
  return;
}


/** Jacobian df/du multiplied by vector vec, i.e. (df/du)^T*vec or vec^T*(df/du) **/
void dfdu(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u,
          ctypeRNum *p, typeUSERPARAM *userparam)
{
  GRAMPC_TABLE *gdata=(GRAMPC_TABLE *) userparam;
  if (!gdata->have_dfdu)
  {
    return;
  }

  Ent *rent=0;
  MDR *p_mdr=NULL;
  void *p_save=NULL;

  // pointer shuffle:
  int i, j;
  r_extern_func_args ext_f_args = (r_extern_func_args) gdata->ext_f_args;
  // t:
  MDR *t_mdr = ent_data(ext_f_args.arg1);
  MDPTR(t_mdr) = (void *) &t;
  // x:
  MDR *x_mdr = ent_data(ext_f_args.arg2);
  MDPTR(x_mdr) = (void *) x;
  // u:
  MDR *u_mdr = ent_data(ext_f_args.arg3);
  MDPTR(u_mdr) = (void *) u;

  // incref, call, then decref
  ent_IncRef(ext_f_args.arg1);
  ent_IncRef(ext_f_args.arg2);
  ent_IncRef(ext_f_args.arg3);
  switch(ext_f_args.n_args)
  {
    case 3:
      // dfdu(t,x,u)
      rent = ent_call_rlab_script_3args(gdata->ent_fn_dfdu,
                                        ext_f_args.arg1,
                                        ext_f_args.arg2,
                                        ext_f_args.arg3);
      break;

    case -4:
      // dfdu(t,x,u,s)
      ent_IncRef(ext_f_args.arg4);
      rent = ent_call_rlab_script_4args (gdata->ent_fn_dfdu,
                                         ext_f_args.arg1,
                                         ext_f_args.arg2,
                                         ext_f_args.arg3,
                                         ext_f_args.arg4);
      ent_DecRef(ext_f_args.arg4);
      break;

    case 4:
      // dfdu(t,x,u,p)
      // p:
      p_mdr   = ent_data(ext_f_args.arg4);
      p_save  = MDRPTR(p_mdr);
      MDPTR(p_mdr) = (void *) p;
      ent_IncRef(ext_f_args.arg4);
      rent = ent_call_rlab_script_4args (gdata->ent_fn_dfdu,
                                         ext_f_args.arg1,
                                         ext_f_args.arg2,
                                         ext_f_args.arg3,
                                         ext_f_args.arg4);
      ent_DecRef(ext_f_args.arg4);
      MDPTR(p_mdr) = p_save;
      break;

    case -5:
      // dfdu(t,x,u,p,s)
      // p:
      p_mdr   = ent_data(ext_f_args.arg4);
      p_save  = MDRPTR(p_mdr);
      MDPTR(p_mdr) = (void *) p;
      ent_IncRef(ext_f_args.arg4);
      ent_IncRef(ext_f_args.arg5);
      rent = ent_call_rlab_script_5args (gdata->ent_fn_dfdu,
                                         ext_f_args.arg1,
                                         ext_f_args.arg2,
                                         ext_f_args.arg3,
                                         ext_f_args.arg4,
                                         ext_f_args.arg5);
      ent_DecRef(ext_f_args.arg4);
      ent_DecRef(ext_f_args.arg5);
      MDPTR(p_mdr) = p_save;
      break;
  }
  ent_DecRef(ext_f_args.arg1);
  ent_DecRef(ext_f_args.arg2);
  ent_DecRef(ext_f_args.arg3);

  if (ent_type(rent)!=MATRIX_DENSE_REAL)
  {
    rerror (RLAB_ERROR_RHS_FUNC_MUST_RETURN_MDR);
  }

  MDR *retm = ent_data(rent);
  if ((MNR(retm)!=gdata->Nx)||(MNC(retm)!=gdata->Nu))
  {
    fprintf(stderr, THIS_SOLVER ": [dfdu] " RLAB_ERROR_RHS_FUNC_INCORRECT_DIM "\n");
    fprintf(stderr, THIS_SOLVER ": [dfdu] Expected dimension (%i,%i), RHS dimension (%i,%i)\n",
        gdata->Nx,gdata->Nu, MNR(retm), MNC(retm));
    fprintf(stderr, THIS_SOLVER ": [dfdu] Check the RHS function: Cannot continue!\n");
    rerror (THIS_SOLVER);
  }

  for (i=0; i < gdata->Nx; i++)
  {
    for (j=0; j < gdata->Nu; j++)
    {
      out[i + j*(gdata->Nx)] = (typeRNum) Mdr0 (retm, i, j);
    }
  }

  ent_Clean(rent);
  return;
}

void dfdu_vec(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *vec,
              ctypeRNum *u, ctypeRNum *p, typeUSERPARAM *userparam)
{
  GRAMPC_TABLE *gdata=(GRAMPC_TABLE *) userparam;
  if (!gdata->have_dfdu)
  {
    return;
  }
  int i, j;
  MDR *retm = mdr_Create(gdata->Nx,gdata->Nu);
  dfdu((typeRNum *) MDRPTR(retm), t, x, u, p, userparam);
  for (j=0; j<gdata->Nu; j++)
  {
    out[j] = 0;
    for (i=0; i<gdata->Nx; i++)
    {
      if (Mdr0 (retm, i, j))
        out[j] = out[j] + vec[i] * Mdr0 (retm, i, j);
    }
  }
  mdr_Destroy(retm);
  return;
}
/** Jacobian df/dp multiplied by vector vec, i.e. (df/dp)^T*vec or vec^T*(df/dp) **/
/** Jacobian df/dp  **/
void dfdp (typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u,
              ctypeRNum *p, typeUSERPARAM *userparam)
{
  GRAMPC_TABLE *gdata=(GRAMPC_TABLE *) userparam;
  if (!gdata->have_dfdp)
  {
    return;
  }
  Ent *rent=0;
  MDR *p_mdr=NULL;
  void *p_save=NULL;

  // pointer shuffle:
  int i, j;
  r_extern_func_args ext_f_args = (r_extern_func_args) gdata->ext_f_args;
  // t:
  MDR *t_mdr = ent_data(ext_f_args.arg1);
  MDPTR(t_mdr) = (void *) &t;
  // x:
  MDR *x_mdr = ent_data(ext_f_args.arg2);
  MDPTR(x_mdr) = (void *) x;
  // u:
  MDR *u_mdr = ent_data(ext_f_args.arg3);
  MDPTR(u_mdr) = (void *) u;

  // incref, call, then decref
  ent_IncRef(ext_f_args.arg1);
  ent_IncRef(ext_f_args.arg2);
  ent_IncRef(ext_f_args.arg3);
  switch(ext_f_args.n_args)
  {
    case 4:
      // dfdp(t,x,u,p)
      // p:
      p_mdr   = ent_data(ext_f_args.arg4);
      p_save  = MDRPTR(p_mdr);
      MDPTR(p_mdr) = (void *) p;
      ent_IncRef(ext_f_args.arg4);
      rent = ent_call_rlab_script_with_args (gdata->ent_fn_dfdp, 4,
                                         ext_f_args.arg1,
                                         ext_f_args.arg2,
                                         ext_f_args.arg3,
                                         ext_f_args.arg4);
      ent_DecRef(ext_f_args.arg4);
      MDPTR(p_mdr) = p_save;
      break;

    case -5:
      // dfdp(t,x,u,p,s)
      // p:
      p_mdr   = ent_data(ext_f_args.arg4);
      p_save  = MDRPTR(p_mdr);
      MDPTR(p_mdr) = (void *) p;
      ent_IncRef(ext_f_args.arg4);
      ent_IncRef(ext_f_args.arg5);
      rent = ent_call_rlab_script_with_args (gdata->ent_fn_dfdp, 5,
                                         ext_f_args.arg1,
                                         ext_f_args.arg2,
                                         ext_f_args.arg3,
                                         ext_f_args.arg4,
                                         ext_f_args.arg5);
      ent_DecRef(ext_f_args.arg4);
      ent_DecRef(ext_f_args.arg5);
      MDPTR(p_mdr) = p_save;
      break;
  }
  ent_DecRef(ext_f_args.arg1);
  ent_DecRef(ext_f_args.arg2);
  ent_DecRef(ext_f_args.arg3);

  if (ent_type(rent)!=MATRIX_DENSE_REAL)
  {
    rerror (RLAB_ERROR_RHS_FUNC_MUST_RETURN_MDR);
  }

  MDR *retm = ent_data(rent);
  if ((MNR(retm)!=gdata->Nx)||(MNC(retm)!=gdata->Np))
  {
    fprintf(stderr, THIS_SOLVER ": [dfdp] " RLAB_ERROR_RHS_FUNC_INCORRECT_DIM "\n");
    fprintf(stderr, THIS_SOLVER ": [dfdp] Expected dimension (%i,%i), RHS dimension (%i,%i)\n",
            gdata->Nx,gdata->Np,MNR(retm),MNC(retm));
    fprintf(stderr, THIS_SOLVER ": [dfdp] Check the RHS function: Cannot continue!\n");
    rerror (THIS_SOLVER);
  }

  for (i=0; i < gdata->Nx; i++)
  {
    for (j=0; j < gdata->Np; j++)
    {
      out[i + j*(gdata->Nx)] = (typeRNum) Mdr0 (retm, i, j);
    }
  }

  ent_Clean(rent);
  return;
}

void dfdp_vec(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *vec,
              ctypeRNum *u, ctypeRNum *p, typeUSERPARAM *userparam)
{
  GRAMPC_TABLE *gdata=(GRAMPC_TABLE *) userparam;
  if (!gdata->have_dfdp)
  {
    return;
  }

  int i, j;
  MDR *retm = mdr_Create(gdata->Nx,gdata->Np);
  dfdp((typeRNum *) MDRPTR(retm), t, x, u, p, userparam);
  for (j=0; j<gdata->Np; j++)
  {
    out[j] = 0;
    for (i=0; i < gdata->Nx; i++)
    {
      if (Mdr0 (retm, i, j))
        out[j] = out[j] + vec[i] * Mdr0(retm, i, j);
    }
  }
  mdr_Destroy(retm);
  return;
}

/** Jacobian df/dt **/
void dfdt(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u,
          ctypeRNum *p, typeUSERPARAM *userparam)
{
  GRAMPC_TABLE *gdata=(GRAMPC_TABLE *) userparam;
  if (!gdata->have_dfdt)
  {
    return;
  }

  Ent *rent=0;
  MDR *p_mdr=NULL;
  void *p_save=NULL;

  // pointer shuffle:
  int i;
  r_extern_func_args ext_f_args = (r_extern_func_args) gdata->ext_f_args;
  // t:
  MDR *t_mdr = ent_data(ext_f_args.arg1);
  MDPTR(t_mdr) = (void *) &t;
  // x:
  MDR *x_mdr = ent_data(ext_f_args.arg2);
  MDPTR(x_mdr) = (void *) x;
  // u:
  MDR *u_mdr = ent_data(ext_f_args.arg3);
  MDPTR(u_mdr) = (void *) u;

  // incref, call, then decref
  ent_IncRef(ext_f_args.arg1);
  ent_IncRef(ext_f_args.arg2);
  ent_IncRef(ext_f_args.arg3);
  switch(ext_f_args.n_args)
  {
    case 3:
      // dfdu(t,x,u)
      rent = ent_call_rlab_script_3args(gdata->ent_fn_dfdt,
                                        ext_f_args.arg1,
                                        ext_f_args.arg2,
                                        ext_f_args.arg3);
      break;

    case -4:
      // dfdu(t,x,u,s)
      ent_IncRef(ext_f_args.arg4);
      rent = ent_call_rlab_script_4args (gdata->ent_fn_dfdt,
                                         ext_f_args.arg1,
                                         ext_f_args.arg2,
                                         ext_f_args.arg3,
                                         ext_f_args.arg4);
      ent_DecRef(ext_f_args.arg4);
      break;

    case 4:
      // dfdu(t,x,u,p)
      // p:
      p_mdr   = ent_data(ext_f_args.arg4);
      p_save  = MDRPTR(p_mdr);
      MDPTR(p_mdr) = (void *) p;
      ent_IncRef(ext_f_args.arg4);
      rent = ent_call_rlab_script_4args (gdata->ent_fn_dfdt,
                                         ext_f_args.arg1,
                                         ext_f_args.arg2,
                                         ext_f_args.arg3,
                                         ext_f_args.arg4);
      ent_DecRef(ext_f_args.arg4);
      MDPTR(p_mdr) = p_save;
      break;

    case -5:
      // dfdu(t,x,u,p,s)
      // p:
      p_mdr   = ent_data(ext_f_args.arg4);
      p_save  = MDRPTR(p_mdr);
      MDPTR(p_mdr) = (void *) p;
      ent_IncRef(ext_f_args.arg4);
      ent_IncRef(ext_f_args.arg5);
      rent = ent_call_rlab_script_5args (gdata->ent_fn_dfdt,
                                         ext_f_args.arg1,
                                         ext_f_args.arg2,
                                         ext_f_args.arg3,
                                         ext_f_args.arg4,
                                         ext_f_args.arg5);
      ent_DecRef(ext_f_args.arg4);
      ent_DecRef(ext_f_args.arg5);
      MDPTR(p_mdr) = p_save;
      break;
  }
  ent_DecRef(ext_f_args.arg1);
  ent_DecRef(ext_f_args.arg2);
  ent_DecRef(ext_f_args.arg3);

  if (ent_type(rent)!=MATRIX_DENSE_REAL)
  {
    rerror (RLAB_ERROR_RHS_FUNC_MUST_RETURN_MDR);
  }

  MDR *retm = ent_data(rent);
  if (SIZE(retm)!=gdata->Nx)
  {
    fprintf(stderr, THIS_SOLVER ": [dfdt] " RLAB_ERROR_RHS_FUNC_INCORRECT_DIM "\n");
    fprintf(stderr, THIS_SOLVER ": [dfdt] Expected dimension %i, RHS dimension %i\n",
        gdata->Nx, SIZE(retm));
    fprintf(stderr, THIS_SOLVER ": [dfdt] Check the RHS function: Cannot continue!\n");
    rerror (THIS_SOLVER);
  }

  for (i=0; i < gdata->Nx; i++)
  {
    out[i] = (typeRNum) MdrV0 (retm, i);
  }

  ent_Clean(rent);
  return;
}


/** Inequality constraints h(t,x(t),u(t),p,uperparam) <= 0
    ------------------------------------------------------ **/
void hfct(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u,
          ctypeRNum *p, typeUSERPARAM *userparam)
{
  GRAMPC_TABLE *gdata=(GRAMPC_TABLE *) userparam;
  if (!gdata->have_hfct)
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
      // hfct(t,x,u)
      rent = ent_call_rlab_script_with_args(gdata->ent_fn_hfct, 3,
                                            ext_f_args.arg1,
                                            ext_f_args.arg2,
                                            ext_f_args.arg3);
      break;

    case -4:
      // hfct(t,x,u,s)
      ent_IncRef(ext_f_args.arg4);
      rent = ent_call_rlab_script_with_args (gdata->ent_fn_hfct, 4,
                                             ext_f_args.arg1,
                                             ext_f_args.arg2,
                                             ext_f_args.arg3,
                                             ext_f_args.arg4);
      ent_DecRef(ext_f_args.arg4);
      break;

    case 4:
      // hfct(t,x,u,p)
      // p:
      p_mdr   = ent_data(ext_f_args.arg4);
      p_save  = MDRPTR(p_mdr);
      MDPTR(p_mdr) = (void *) p;
      ent_IncRef(ext_f_args.arg4);
      rent = ent_call_rlab_script_with_args (gdata->ent_fn_hfct, 4,
                                             ext_f_args.arg1,
                                             ext_f_args.arg2,
                                             ext_f_args.arg3,
                                             ext_f_args.arg4);
      ent_DecRef(ext_f_args.arg4);
      MDPTR(p_mdr) = p_save;
      break;

    case -5:
      // hfct(t,x,u,p,s)
      // p:
      p_mdr   = ent_data(ext_f_args.arg4);
      p_save  = MDRPTR(p_mdr);
      MDPTR(p_mdr) = (void *) p;
      ent_IncRef(ext_f_args.arg4);
      ent_IncRef(ext_f_args.arg5);
      rent = ent_call_rlab_script_with_args (gdata->ent_fn_hfct, 5,
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
  if (SIZE(retm)!=gdata->Nh)
  {
    fprintf(stderr, THIS_SOLVER ": [hfct] " RLAB_ERROR_RHS_FUNC_INCORRECT_DIM "\n");
    fprintf(stderr, THIS_SOLVER ": [hfct] Expected dimension (%i), RHS dimension (%i)\n",
            gdata->Nh,SIZE(retm));
    fprintf(stderr, THIS_SOLVER ": [hfct] Check the RHS function: Cannot continue!\n");
    rerror (THIS_SOLVER);
  }

  for (i=0; i < gdata->Nh; i++)
  {
    out[i] = (typeRNum) MdrV0 (retm, i);
  }

  ent_Clean(rent);

  return;
}

void dhdx(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u,
          ctypeRNum *p, typeUSERPARAM *userparam)
{
  GRAMPC_TABLE *gdata=(GRAMPC_TABLE *) userparam;
  if (!gdata->have_dhdx)
  {
    return;
  }

//   printf("c:dhdx:in\n");

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
      // dgdu(t,x,u)
      rent = ent_call_rlab_script_with_args(gdata->ent_fn_dhdx, 3,
                                            ext_f_args.arg1,
                                            ext_f_args.arg2,
                                            ext_f_args.arg3);
      break;

    case -4:
      // gfct(t,x,u,s)
      ent_IncRef(ext_f_args.arg4);
      rent = ent_call_rlab_script_with_args (gdata->ent_fn_dhdx, 4,
                                             ext_f_args.arg1,
                                             ext_f_args.arg2,
                                             ext_f_args.arg3,
                                             ext_f_args.arg4);
      ent_DecRef(ext_f_args.arg4);
      break;

    case 4:
      // gfct(t,x,u,p)
      // p:
      p_mdr   = ent_data(ext_f_args.arg4);
      p_save  = MDRPTR(p_mdr);
      MDPTR(p_mdr) = (void *) p;
      ent_IncRef(ext_f_args.arg4);
      rent = ent_call_rlab_script_with_args (gdata->ent_fn_dhdx, 4,
                                             ext_f_args.arg1,
                                             ext_f_args.arg2,
                                             ext_f_args.arg3,
                                             ext_f_args.arg4);
      ent_DecRef(ext_f_args.arg4);
      MDPTR(p_mdr) = p_save;
      break;

    case -5:
      // gfct(t,x,u,p,s)
      // p:
      p_mdr   = ent_data(ext_f_args.arg4);
      p_save  = MDRPTR(p_mdr);
      MDPTR(p_mdr) = (void *) p;
      ent_IncRef(ext_f_args.arg4);
      ent_IncRef(ext_f_args.arg5);
      rent = ent_call_rlab_script_with_args (gdata->ent_fn_dhdx, 5,
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
  if ((MNR(retm)!=gdata->Nh)||(MNC(retm)!=gdata->Nx))
  {
    fprintf(stderr, THIS_SOLVER ": [dhdx] " RLAB_ERROR_RHS_FUNC_INCORRECT_DIM "\n");
    fprintf(stderr, THIS_SOLVER ": [dhdx] Expected dimension (%i,%i), RHS dimension (%i,%i)\n",
            gdata->Nh,gdata->Nx,MNR(retm),MNC(retm));
    fprintf(stderr, THIS_SOLVER ": [dhdx] Check the RHS function: Cannot continue!\n");
    rerror (THIS_SOLVER);
  }

  for (i=0; i < gdata->Nh; i++)
  {
    for (j=0; j < gdata->Nx; j++)
    {
      out[i + j*(gdata->Nh)] = (typeRNum) Mdr0 (retm, i, j);
    }
  }

//   printf("c:dhdx:out\n");

  ent_Clean(rent);
  return;
}


/**
  * Jacobian dh/dx multiplied by vector vec, i.e. (dh/dx)^T*vec or vec^T*(dh/dx)
  * DH^T = [dh1, dh2, ....]
  * vec -> DH^T, and not DX
  **/
void dhdx_vec(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u,
              ctypeRNum *p, ctypeRNum *vec, typeUSERPARAM *userparam)
{
  GRAMPC_TABLE *gdata=(GRAMPC_TABLE *) userparam;
  if (!gdata->have_dhdx)
  {
    return;
  }

  int i,j;
  MDR *retm = mdr_Create(gdata->Nh,gdata->Nx);
  dhdx((typeRNum *) MDRPTR(retm), t, x, u, p, userparam);
  for (j=0; j<gdata->Nx; j++)
  {
    out[j] = 0;
    for (i=0; i<gdata->Nh; i++)
    {
      if (Mdr0 (retm, i, j))
        out[j] = out[j] + vec[i] * Mdr0 (retm, i, j);
    }
  }
  mdr_Destroy(retm);
  return;
}

/** Jacobian dh/du multiplied by vector vec, i.e. (dh/du)^T*vec or vec^T*(dg/du) **/
void dhdu(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u,
          ctypeRNum *p, typeUSERPARAM *userparam)
{
  GRAMPC_TABLE *gdata=(GRAMPC_TABLE *) userparam;
  if (!gdata->have_dhdu)
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
      // dgdu(t,x,u)
      rent = ent_call_rlab_script_with_args(gdata->ent_fn_dhdu, 3,
                                            ext_f_args.arg1,
                                            ext_f_args.arg2,
                                            ext_f_args.arg3);
      break;

    case -4:
      // gfct(t,x,u,s)
      ent_IncRef(ext_f_args.arg4);
      rent = ent_call_rlab_script_with_args (gdata->ent_fn_dhdu, 3,
                                             ext_f_args.arg1,
                                             ext_f_args.arg2,
                                             ext_f_args.arg3,
                                             ext_f_args.arg4);
      ent_DecRef(ext_f_args.arg4);
      break;

    case 4:
      // gfct(t,x,u,p)
      // p:
      p_mdr   = ent_data(ext_f_args.arg4);
      p_save  = MDRPTR(p_mdr);
      MDPTR(p_mdr) = (void *) p;
      ent_IncRef(ext_f_args.arg4);
      rent = ent_call_rlab_script_with_args (gdata->ent_fn_dhdu, 4,
                                             ext_f_args.arg1,
                                             ext_f_args.arg2,
                                             ext_f_args.arg3,
                                             ext_f_args.arg4);
      ent_DecRef(ext_f_args.arg4);
      MDPTR(p_mdr) = p_save;
      break;

    case -5:
      // gfct(t,x,u,p,s)
      // p:
      p_mdr   = ent_data(ext_f_args.arg4);
      p_save  = MDRPTR(p_mdr);
      MDPTR(p_mdr) = (void *) p;
      ent_IncRef(ext_f_args.arg4);
      ent_IncRef(ext_f_args.arg5);
      rent = ent_call_rlab_script_with_args (gdata->ent_fn_dhdu, 5,
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
  if ((MNR(retm)!=gdata->Nh)||(MNC(retm)!=gdata->Nu))
  {
    fprintf(stderr, THIS_SOLVER ": [dhdu] " RLAB_ERROR_RHS_FUNC_INCORRECT_DIM "\n");
    fprintf(stderr, THIS_SOLVER ": [dhdu] Expected dimension (%i,%i), RHS dimension (%i,%i)\n",
            gdata->Nh,gdata->Nu,MNR(retm),MNC(retm));
    fprintf(stderr, THIS_SOLVER ": [dhdu] Check the RHS function: Cannot continue!\n");
    rerror (THIS_SOLVER);
  }

  for (i=0; i < gdata->Nh; i++)
  {
    for (j=0; j < gdata->Nu; j++)
    {
      out[i + j*(gdata->Nh)] = (typeRNum) Mdr0 (retm, i, j);
    }
  }

  ent_Clean(rent);
  return;
}

void dhdu_vec(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u,
              ctypeRNum *p, ctypeRNum *vec, typeUSERPARAM *userparam)
{
  GRAMPC_TABLE *gdata=(GRAMPC_TABLE *) userparam;
  if (!gdata->have_dhdu)
  {
    return;
  }

  int i,j;
  MDR *retm = mdr_Create(gdata->Nh,gdata->Nu);
  dhdu((typeRNum *) MDRPTR(retm), t, x, u, p, userparam);
  for (j=0; j<gdata->Nu; j++)
  {
    out[j] = 0;
    for (i=0; i<gdata->Nh; i++)
    {
      if (Mdr0 (retm, i, j))
        out[j] = out[j] + vec[i] * Mdr0 (retm, i, j);
    }
  }

  mdr_Destroy(retm);

  return;
}

/** Jacobian dh/dp multiplied by vector vec, i.e. (dh/dp)^T*vec or vec^T*(dg/dp) **/
void dhdp(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u,
          ctypeRNum *p, typeUSERPARAM *userparam)
{
  GRAMPC_TABLE *gdata=(GRAMPC_TABLE *) userparam;
  if (!gdata->have_dhdp)
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
      // gfct(t,x,u,p)
      // p:
      p_mdr   = ent_data(ext_f_args.arg4);
      p_save  = MDRPTR(p_mdr);
      MDPTR(p_mdr) = (void *) p;
      ent_IncRef(ext_f_args.arg4);
      rent = ent_call_rlab_script_with_args (gdata->ent_fn_dhdp, 4,
                                             ext_f_args.arg1,
                                             ext_f_args.arg2,
                                             ext_f_args.arg3,
                                             ext_f_args.arg4);
      ent_DecRef(ext_f_args.arg4);
      MDPTR(p_mdr) = p_save;
      break;

    case -5:
      // gfct(t,x,u,p,s)
      // p:
      p_mdr   = ent_data(ext_f_args.arg4);
      p_save  = MDRPTR(p_mdr);
      MDPTR(p_mdr) = (void *) p;
      ent_IncRef(ext_f_args.arg4);
      ent_IncRef(ext_f_args.arg5);
      rent = ent_call_rlab_script_with_args (gdata->ent_fn_dhdp, 5,
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
  if ((MNR(retm)!=gdata->Ng)||(MNC(retm)!=gdata->Np))
  {
    fprintf(stderr, THIS_SOLVER ": [dhdp] " RLAB_ERROR_RHS_FUNC_INCORRECT_DIM "\n");
    fprintf(stderr, THIS_SOLVER ": [dhdp] Expected dimension (%i,%i), RHS dimension (%i,%i)\n",
            gdata->Nh,gdata->Np,MNR(retm),MNC(retm));
    fprintf(stderr, THIS_SOLVER ": [dhdp] Check the RHS function: Cannot continue!\n");
    rerror (THIS_SOLVER);
  }

  for (i=0; i < gdata->Nh; i++)
  {
    for (j=0; j < gdata->Np; j++)
    {
      out[i + j*(gdata->Nh)] = (typeRNum) Mdr0 (retm, i, j);
    }
  }

  ent_Clean(rent);
  return;
}


void dhdp_vec(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u,
              ctypeRNum *p, ctypeRNum *vec, typeUSERPARAM *userparam)
{
  GRAMPC_TABLE *gdata=(GRAMPC_TABLE *) userparam;
  if (!gdata->have_dhdp)
  {
    return;
  }

  int i, j;
  MDR *retm = mdr_Create(gdata->Nh,gdata->Np);
  dhdp((typeRNum *) MDRPTR(retm), t, x, u, p, userparam);
  for (j=0; j<gdata->Np; j++)
  {
    out[j] = 0;
    for (i=0; i<gdata->Nh; i++)
    {
      if (Mdr0 (retm, i, j))
        out[j] = out[j] + vec[i] * Mdr0 (retm, i, j);
    }
  }
  mdr_Destroy(retm);
  return;
}

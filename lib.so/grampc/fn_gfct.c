/** Equality constraints g(t,x(t),u(t),p,uperparam) = 0
    --------------------------------------------------- **/
void gfct(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u,
          ctypeRNum *p, typeUSERPARAM *userparam)
{
  GRAMPC_TABLE *gdata=(GRAMPC_TABLE *) userparam;
  if (!gdata->have_gfct)
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
      // gfct(t,x,u)
      rent = ent_call_rlab_script_3args(gdata->ent_fn_gfct,
                                        ext_f_args.arg1,
                                        ext_f_args.arg2,
                                        ext_f_args.arg3);
      break;

    case -4:
      // gfct(t,x,u,s)
      ent_IncRef(ext_f_args.arg4);
      rent = ent_call_rlab_script_4args (gdata->ent_fn_gfct,
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
      rent = ent_call_rlab_script_4args (gdata->ent_fn_gfct,
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
      rent = ent_call_rlab_script_5args (gdata->ent_fn_gfct,
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
  if (SIZE(retm)!=gdata->Ng)
  {
    fprintf(stderr, THIS_SOLVER ": [gfct] " RLAB_ERROR_RHS_FUNC_INCORRECT_DIM "\n");
    fprintf(stderr, THIS_SOLVER ": [gfct] Expected dimension %i, RHS dimension %i\n", gdata->Ng, SIZE(retm));
    fprintf(stderr, THIS_SOLVER ": [gfct] Check the RHS function: Cannot continue!\n");
    rerror (THIS_SOLVER);
  }

  for (i=0; i < gdata->Ng; i++)
  {
    out[i] = (typeRNum) MdrV0 (retm, i);
  }

  ent_Clean(rent);
  return;
}

/** Jacobian dg/dx  **/
void dgdx(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u,
          ctypeRNum *p, typeUSERPARAM *userparam)
{
  GRAMPC_TABLE *gdata=(GRAMPC_TABLE *) userparam;
  if (!gdata->have_dgdx)
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
      // gfct(t,x,u)
      rent = ent_call_rlab_script_3args(gdata->ent_fn_dgdx,
                                        ext_f_args.arg1,
                                        ext_f_args.arg2,
                                        ext_f_args.arg3);
      break;

    case -4:
      // gfct(t,x,u,s)
      ent_IncRef(ext_f_args.arg4);
      rent = ent_call_rlab_script_4args (gdata->ent_fn_dgdx,
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
      rent = ent_call_rlab_script_4args (gdata->ent_fn_dgdx,
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
      rent = ent_call_rlab_script_5args (gdata->ent_fn_dgdx,
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
  if ((MNR(retm)!=gdata->Ng)||(MNC(retm)!=gdata->Nx))
  {
    fprintf(stderr, THIS_SOLVER ": [dgdx] " RLAB_ERROR_RHS_FUNC_INCORRECT_DIM "\n");
    fprintf(stderr, THIS_SOLVER ": [dgdx] Expected dimension (%i,%i), RHS dimension (%i,%i)\n",
            gdata->Ng,gdata->Nx,MNR(retm),MNC(retm));
    fprintf(stderr, THIS_SOLVER ": [dgdx] Check the RHS function: Cannot continue!\n");
    rerror (THIS_SOLVER);
  }

  for (i=0; i < gdata->Ng; i++)
  {
    for (j=0; j < gdata->Nx; j++)
    {
      out[i + j*(gdata->Ng)] = (typeRNum) Mdr0 (retm, i, j);
    }
  }

  ent_Clean(rent);
  return;
}

/** Jacobian dg/dx multiplied by vector vec, i.e. (dg/dx)^T*vec or vec^T*(dg/dx) **/
void dgdx_vec(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u,
              ctypeRNum *p, ctypeRNum *vec, typeUSERPARAM *userparam)
{
  GRAMPC_TABLE *gdata=(GRAMPC_TABLE *) userparam;
  if (!gdata->have_dgdx)
  {
    return;
  }

  int i, j;
  MDR *retm = mdr_Create(gdata->Ng,gdata->Nx);

  dgdx((typeRNum *) MDPTR(retm), t, x, u, p, userparam);
  for (j=0; j<gdata->Nx; j++)
  {
    out[j] = 0;
    for (i=0; i<gdata->Ng; i++)
    {
      if (Mdr0 (retm, i, j))
        out[j] = out[j] + vec[i] * Mdr0 (retm, i, j);
    }
  }

  mdr_Destroy(retm);

  return;
}

/** Jacobian dg/du **/
void dgdu(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u,
              ctypeRNum *p, ctypeRNum *vec, typeUSERPARAM *userparam)
{
  GRAMPC_TABLE *gdata=(GRAMPC_TABLE *) userparam;
  if (!gdata->have_dgdu)
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
      rent = ent_call_rlab_script_with_args(gdata->ent_fn_dgdu, 3,
                                        ext_f_args.arg1,
                                        ext_f_args.arg2,
                                        ext_f_args.arg3);
      break;

    case -4:
      // gfct(t,x,u,s)
      ent_IncRef(ext_f_args.arg4);
      rent = ent_call_rlab_script_with_args (gdata->ent_fn_dgdu, 3,
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
      rent = ent_call_rlab_script_with_args (gdata->ent_fn_dgdu, 4,
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
      rent = ent_call_rlab_script_with_args (gdata->ent_fn_dgdu, 5,
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
  if ((MNR(retm)!=gdata->Ng)||(MNC(retm)!=gdata->Nu))
  {
    fprintf(stderr, THIS_SOLVER ": [dgdu] " RLAB_ERROR_RHS_FUNC_INCORRECT_DIM "\n");
    fprintf(stderr, THIS_SOLVER ": [dgdu] Expected dimension (%i,%i), RHS dimension (%i,%i)\n",
            gdata->Ng,gdata->Nu,MNR(retm),MNC(retm));
    fprintf(stderr, THIS_SOLVER ": [dgdu] Check the RHS function: Cannot continue!\n");
    rerror (THIS_SOLVER);
  }

  for (i=0; i < gdata->Ng; i++)
  {
    for (j=0; j < gdata->Nu; j++)
    {
      out[i + j * (gdata->Ng)] = (typeRNum) Mdr0 (retm, i, j);
    }
  }

  ent_Clean(rent);
  return;
}


/** Jacobian dg/du multiplied by vector vec, i.e. (dg/du)^T*vec or vec^T*(dg/du) **/
void dgdu_vec(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u,
              ctypeRNum *p, ctypeRNum *vec, typeUSERPARAM *userparam)
{
  GRAMPC_TABLE *gdata=(GRAMPC_TABLE *) userparam;
  if (!gdata->have_dgdu)
  {
    return;
  }

  int i, j;
  MDR *retm = mdr_Create(gdata->Ng,gdata->Nu);
  dgdu((typeRNum *) MDPTR(retm), t, x, u, p, vec, userparam);
  for (j=0; j<gdata->Nu; j++)
  {
    out[j] = 0;
    for (i=0; i<gdata->Ng; i++)
    {
      if (Mdr0 (retm, i, j))
        out[j] = out[j] + vec[i] * Mdr0 (retm, i, j);
    }
  }
  mdr_Destroy(retm);
  return;
}

/** Jacobian dg/dp  **/
void dgdp (typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u,
              ctypeRNum *p, ctypeRNum *vec, typeUSERPARAM *userparam)
{
  GRAMPC_TABLE *gdata=(GRAMPC_TABLE *) userparam;
  if (!gdata->have_dgdp)
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
      rent = ent_call_rlab_script_with_args (gdata->ent_fn_dgdp, 4,
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
      rent = ent_call_rlab_script_with_args (gdata->ent_fn_dgdp, 5,
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
    fprintf(stderr, THIS_SOLVER ": [dgdp] " RLAB_ERROR_RHS_FUNC_INCORRECT_DIM "\n");
    fprintf(stderr, THIS_SOLVER ": [dgdp] Expected dimension (%i,%i), RHS dimension (%i,%i)\n",
            gdata->Ng,gdata->Np,MNR(retm),MNC(retm));
    fprintf(stderr, THIS_SOLVER ": [dgdp] Check the RHS function: Cannot continue!\n");
    rerror (THIS_SOLVER);
  }

  for (i=0; i < gdata->Ng; i++)
  {
    for (j=0; j < gdata->Np; j++)
    {
      out[i + j*(gdata->Ng)] = (typeRNum) Mdr0 (retm, i, j);
    }
  }

  ent_Clean(rent);
  return;
}

void dgdp_vec(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u,
              ctypeRNum *p, ctypeRNum *vec, typeUSERPARAM *userparam)
{
  GRAMPC_TABLE *gdata=(GRAMPC_TABLE *) userparam;
  if (!gdata->have_dgdp)
  {
    return;
  }

  int i, j;
  MDR *retm = mdr_Create(gdata->Ng,gdata->Np);
  dgdp((typeRNum *) MDPTR(retm), t, x, u, p, vec, userparam);
  for (j=0; j<gdata->Np; j++)
  {
    out[j] = 0;
    for (i=0; i<gdata->Ng; i++)
    {
      if (Mdr0 (retm, i, j))
        out[j] = out[j] + vec[i] * Mdr0 (retm, i, j);
    }
  }
  mdr_Destroy(retm);
  return;
}

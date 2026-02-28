/** Terminal equality constraints gT(T,x(T),p,uperparam) = 0
    -------------------------------------------------------- **/
void gTfct(typeRNum *out, ctypeRNum T, ctypeRNum *x, ctypeRNum *p,
           typeUSERPARAM *userparam)
{
  GRAMPC_TABLE *gdata=(GRAMPC_TABLE *) userparam;
  if (!gdata->have_gTfct)
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
  MDPTR(t_mdr) = (void *) &T;
  // x:
  MDR *x_mdr = ent_data(ext_f_args.arg2);
  MDPTR(x_mdr) = (void *) x;

  // incref, call, then decref
  ent_IncRef(ext_f_args.arg1);
  ent_IncRef(ext_f_args.arg2);
  switch(ext_f_args.n_args)
  {
    case 3:
      // gTfct(T,x)
      rent = ent_call_rlab_script_with_args(gdata->ent_fn_gTfct, 2,
                                        ext_f_args.arg1,
                                        ext_f_args.arg2);
      break;

    case -4:
      // gTfct(T,x,s)
      ent_IncRef(ext_f_args.arg4);
      rent = ent_call_rlab_script_with_args (gdata->ent_fn_gTfct, 3,
                                         ext_f_args.arg1,
                                         ext_f_args.arg2,
                                         ext_f_args.arg4);
      ent_DecRef(ext_f_args.arg4);
      break;

    case 4:
      // gTfct(t,x,p)
      // p:
      p_mdr   = ent_data(ext_f_args.arg4);
      p_save  = MDRPTR(p_mdr);
      MDPTR(p_mdr) = (void *) p;
      ent_IncRef(ext_f_args.arg4);
      rent = ent_call_rlab_script_with_args (gdata->ent_fn_gTfct, 3,
                                         ext_f_args.arg1,
                                         ext_f_args.arg2,
                                         ext_f_args.arg4);
      ent_DecRef(ext_f_args.arg4);
      MDPTR(p_mdr) = p_save;
      break;

    case -5:
      // gTfct(t,x,p,s)
      // p:
      p_mdr   = ent_data(ext_f_args.arg4);
      p_save  = MDRPTR(p_mdr);
      MDPTR(p_mdr) = (void *) p;
      ent_IncRef(ext_f_args.arg4);
      ent_IncRef(ext_f_args.arg5);
      rent = ent_call_rlab_script_with_args (gdata->ent_fn_gTfct, 4,
                                         ext_f_args.arg1,
                                         ext_f_args.arg2,
                                         ext_f_args.arg4,
                                         ext_f_args.arg5);
      ent_DecRef(ext_f_args.arg4);
      ent_DecRef(ext_f_args.arg5);
      MDPTR(p_mdr) = p_save;
      break;
  }
  ent_DecRef(ext_f_args.arg1);
  ent_DecRef(ext_f_args.arg2);

  if (ent_type(rent)!=MATRIX_DENSE_REAL)
  {
    rerror (RLAB_ERROR_RHS_FUNC_MUST_RETURN_MDR);
  }

  MDR *retm = ent_data(rent);
  if (SIZE(retm)!=gdata->Ngt)
  {
    fprintf(stderr, THIS_SOLVER ": [gTfct] " RLAB_ERROR_RHS_FUNC_INCORRECT_DIM "\n");
    fprintf(stderr, THIS_SOLVER ": [gTfct] Expected dimension %i, RHS dimension %i\n",
            gdata->Ngt, SIZE(retm));
    fprintf(stderr, THIS_SOLVER ": [gTfct] Check the RHS function: Cannot continue!\n");
    rerror (THIS_SOLVER);
  }

  for (i=0; i < gdata->Ngt; i++)
  {
    out[i] = (typeRNum) MdrV0 (retm, i);
  }

  ent_Clean(rent);
  return;
}

/** Jacobian dg/dx  **/
void dgTdx(typeRNum *out, ctypeRNum T, ctypeRNum *x, ctypeRNum *p,
           typeUSERPARAM *userparam)
{
  GRAMPC_TABLE *gdata=(GRAMPC_TABLE *) userparam;
  if (!gdata->have_dgTdx)
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
  MDPTR(t_mdr) = (void *) &T;
  // x:
  MDR *x_mdr = ent_data(ext_f_args.arg2);
  MDPTR(x_mdr) = (void *) x;

  // incref, call, then decref
  ent_IncRef(ext_f_args.arg1);
  ent_IncRef(ext_f_args.arg2);
  switch(ext_f_args.n_args)
  {
    case 3:
      // dgTdx(t,x)
      rent = ent_call_rlab_script_with_args(gdata->ent_fn_dgTdx, 2,
                                        ext_f_args.arg1,
                                        ext_f_args.arg2);
      break;

    case -4:
      // dgTdx(t,x,s)
      ent_IncRef(ext_f_args.arg4);
      rent = ent_call_rlab_script_with_args (gdata->ent_fn_dgTdx, 3,
                                         ext_f_args.arg1,
                                         ext_f_args.arg2,
                                         ext_f_args.arg4);
      ent_DecRef(ext_f_args.arg4);
      break;

    case 4:
      // dgTdx(t,x,p)
      // p:
      p_mdr   = ent_data(ext_f_args.arg4);
      p_save  = MDRPTR(p_mdr);
      MDPTR(p_mdr) = (void *) p;
      ent_IncRef(ext_f_args.arg4);
      rent = ent_call_rlab_script_with_args (gdata->ent_fn_dgTdx, 3,
                                         ext_f_args.arg1,
                                         ext_f_args.arg2,
                                         ext_f_args.arg4);
      ent_DecRef(ext_f_args.arg4);
      MDPTR(p_mdr) = p_save;
      break;

    case -5:
      // dgTdx(t,x,p,s)
      // p:
      p_mdr   = ent_data(ext_f_args.arg4);
      p_save  = MDRPTR(p_mdr);
      MDPTR(p_mdr) = (void *) p;
      ent_IncRef(ext_f_args.arg4);
      ent_IncRef(ext_f_args.arg5);
      rent = ent_call_rlab_script_with_args (gdata->ent_fn_dgTdx, 4,
                                         ext_f_args.arg1,
                                         ext_f_args.arg2,
                                         ext_f_args.arg4,
                                         ext_f_args.arg5);
      ent_DecRef(ext_f_args.arg4);
      ent_DecRef(ext_f_args.arg5);
      MDPTR(p_mdr) = p_save;
      break;
  }
  ent_DecRef(ext_f_args.arg1);
  ent_DecRef(ext_f_args.arg2);

  if (ent_type(rent)!=MATRIX_DENSE_REAL)
  {
    rerror (RLAB_ERROR_RHS_FUNC_MUST_RETURN_MDR);
  }

  MDR *retm = ent_data(rent);
  if ((MNR(retm)!=gdata->Ngt)||(MNC(retm)!=gdata->Nx))
  {
    fprintf(stderr, THIS_SOLVER ": [dgTdx] " RLAB_ERROR_RHS_FUNC_INCORRECT_DIM "\n");
    fprintf(stderr, THIS_SOLVER ": [dgTdx] Expected dimension (%i,%i), "
        "RHS dimension (%i,%i)\n",
        gdata->Ngt,gdata->Nx,MNR(retm),MNC(retm));
    fprintf(stderr, THIS_SOLVER ": [dgTdx] Check the RHS function: Cannot continue!\n");
    rerror (THIS_SOLVER);
  }

  for (i=0; i < gdata->Ngt; i++)
  {
    for (j=0; j < gdata->Nx; j++)
    {
      out[i + j*(gdata->Ngt)] = (typeRNum) Mdr0 (retm, i, j);
    }
  }

  ent_Clean(rent);
  return;
}



/** Jacobian dgT/dx multiplied by vector vec, i.e. (dgT/dx)^T*vec or vec^T*(dgT/dx) **/
void dgTdx_vec(typeRNum *out, ctypeRNum T, ctypeRNum *x, ctypeRNum *p,
               ctypeRNum *vec, typeUSERPARAM *userparam)
{
  GRAMPC_TABLE *gdata=(GRAMPC_TABLE *) userparam;
  if (!gdata->have_dgTdx)
  {
    return;
  }

  int i, j;
  MDR *retm = mdr_Create(gdata->Ngt,gdata->Nx);
  dgTdx((typeRNum *) MDPTR(retm), T, x, p, userparam);
  for (j=0; j<gdata->Nx; j++)
  {
    out[j] = 0;
    for (i=0; i<gdata->Ngt; i++)
    {
      if (Mdr0 (retm, i, j))
        out[j] = out[j] + vec[i] * Mdr0 (retm, i, j);
    }
  }
  mdr_Destroy(retm);

  return;
}
/** Jacobian dgT/dp multiplied by vector vec, i.e. (dgT/dp)^T*vec or vec^T*(dgT/dp) **/
void dgTdp(typeRNum *out, ctypeRNum T, ctypeRNum *x, ctypeRNum *p,
           typeUSERPARAM *userparam)
{
  GRAMPC_TABLE *gdata=(GRAMPC_TABLE *) userparam;
  if (!gdata->have_dgTdp)
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
  MDPTR(t_mdr) = (void *) &T;
  // x:
  MDR *x_mdr = ent_data(ext_f_args.arg2);
  MDPTR(x_mdr) = (void *) x;

  // incref, call, then decref
  ent_IncRef(ext_f_args.arg1);
  ent_IncRef(ext_f_args.arg2);
  switch(ext_f_args.n_args)
  {
    case 3:
      // dgTdx(t,x)
      rent = ent_call_rlab_script_with_args(gdata->ent_fn_dgTdp, 2,
                                            ext_f_args.arg1,
                                            ext_f_args.arg2);
      break;

    case -4:
      // dgTdx(t,x,s)
      ent_IncRef(ext_f_args.arg4);
      rent = ent_call_rlab_script_with_args (gdata->ent_fn_dgTdp, 3,
                                             ext_f_args.arg1,
                                             ext_f_args.arg2,
                                             ext_f_args.arg4);
      ent_DecRef(ext_f_args.arg4);
      break;

    case 4:
      // dgTdx(t,x,p)
      // p:
      p_mdr   = ent_data(ext_f_args.arg4);
      p_save  = MDRPTR(p_mdr);
      MDPTR(p_mdr) = (void *) p;
      ent_IncRef(ext_f_args.arg4);
      rent = ent_call_rlab_script_with_args (gdata->ent_fn_dgTdp, 3,
                                             ext_f_args.arg1,
                                             ext_f_args.arg2,
                                             ext_f_args.arg4);
      ent_DecRef(ext_f_args.arg4);
      MDPTR(p_mdr) = p_save;
      break;

    case -5:
      // dgTdx(t,x,p,s)
      // p:
      p_mdr   = ent_data(ext_f_args.arg4);
      p_save  = MDRPTR(p_mdr);
      MDPTR(p_mdr) = (void *) p;
      ent_IncRef(ext_f_args.arg4);
      ent_IncRef(ext_f_args.arg5);
      rent = ent_call_rlab_script_with_args (gdata->ent_fn_dgTdp, 4,
                                             ext_f_args.arg1,
                                             ext_f_args.arg2,
                                             ext_f_args.arg4,
                                             ext_f_args.arg5);
      ent_DecRef(ext_f_args.arg4);
      ent_DecRef(ext_f_args.arg5);
      MDPTR(p_mdr) = p_save;
      break;
  }
  ent_DecRef(ext_f_args.arg1);
  ent_DecRef(ext_f_args.arg2);

  if (ent_type(rent)!=MATRIX_DENSE_REAL)
  {
    rerror (RLAB_ERROR_RHS_FUNC_MUST_RETURN_MDR);
  }

  MDR *retm = ent_data(rent);
  if ((MNR(retm)!=gdata->Ngt)||(MNC(retm)!=gdata->Np))
  {
    fprintf(stderr, THIS_SOLVER ": [dgTdx] " RLAB_ERROR_RHS_FUNC_INCORRECT_DIM "\n");
    fprintf(stderr, THIS_SOLVER ": [dgTdx] Expected dimension (%i,%i), RHS dimension (%i,%i)\n",
            gdata->Ngt,gdata->Np,MNR(retm),MNC(retm));
    fprintf(stderr, THIS_SOLVER ": [dgTdx] Check the RHS function: Cannot continue!\n");
    rerror (THIS_SOLVER);
  }

  for (i=0; i < gdata->Ngt; i++)
  {
    for (j=0; j < gdata->Np; j++)
    {
      out[i + j*(gdata->Ngt)] = (typeRNum) Mdr0 (retm, i, j);
    }
  }

  ent_Clean(rent);
  return;
}


void dgTdp_vec(typeRNum *out, ctypeRNum T, ctypeRNum *x, ctypeRNum *p,
               ctypeRNum *vec, typeUSERPARAM *userparam)
{
  GRAMPC_TABLE *gdata=(GRAMPC_TABLE *) userparam;
  if (!gdata->have_dgTdp)
  {
    return;
  }

  int i, j;
  MDR *retm = mdr_Create(gdata->Ngt,gdata->Np);
  dgTdp((typeRNum *) MDPTR(retm), T, x, p, userparam);
  for (j=0; j<gdata->Np; j++)
  {
    out[j] = 0;
    for (i=0; i<gdata->Ngt; i++)
    {
      if (Mdr0 (retm, i, j))
        out[j] = out[j] + vec[i] * Mdr0 (retm, i, j);
    }
  }
  mdr_Destroy(retm);
  return;
}

/** Jacobian dgT/dT multiplied by vector vec, i.e. (dgT/dT)^T*vec or vec^T*(dgT/dT) **/
void dgTdT(typeRNum *out, ctypeRNum T, ctypeRNum *x, ctypeRNum *p,
           typeUSERPARAM *userparam)
{
  GRAMPC_TABLE *gdata=(GRAMPC_TABLE *) userparam;
  if (!gdata->have_dgTdp)
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
  MDPTR(t_mdr) = (void *) &T;
  // x:
  MDR *x_mdr = ent_data(ext_f_args.arg2);
  MDPTR(x_mdr) = (void *) x;

  // incref, call, then decref
  ent_IncRef(ext_f_args.arg1);
  ent_IncRef(ext_f_args.arg2);
  switch(ext_f_args.n_args)
  {
    case 3:
      // dgTdx(t,x)
      rent = ent_call_rlab_script_with_args(gdata->ent_fn_dgTdT, 2,
                                            ext_f_args.arg1,
                                            ext_f_args.arg2);
      break;

    case -4:
      // dgTdx(t,x,s)
      ent_IncRef(ext_f_args.arg4);
      rent = ent_call_rlab_script_with_args (gdata->ent_fn_dgTdT, 3,
                                             ext_f_args.arg1,
                                             ext_f_args.arg2,
                                             ext_f_args.arg4);
      ent_DecRef(ext_f_args.arg4);
      break;

    case 4:
      // dgTdx(t,x,p)
      // p:
      p_mdr   = ent_data(ext_f_args.arg4);
      p_save  = MDRPTR(p_mdr);
      MDPTR(p_mdr) = (void *) p;
      ent_IncRef(ext_f_args.arg4);
      rent = ent_call_rlab_script_with_args (gdata->ent_fn_dgTdT, 3,
                                             ext_f_args.arg1,
                                             ext_f_args.arg2,
                                             ext_f_args.arg4);
      ent_DecRef(ext_f_args.arg4);
      MDPTR(p_mdr) = p_save;
      break;

    case -5:
      // dgTdx(t,x,p,s)
      // p:
      p_mdr   = ent_data(ext_f_args.arg4);
      p_save  = MDRPTR(p_mdr);
      MDPTR(p_mdr) = (void *) p;
      ent_IncRef(ext_f_args.arg4);
      ent_IncRef(ext_f_args.arg5);
      rent = ent_call_rlab_script_with_args (gdata->ent_fn_dgTdT, 4,
                                             ext_f_args.arg1,
                                             ext_f_args.arg2,
                                             ext_f_args.arg4,
                                             ext_f_args.arg5);
      ent_DecRef(ext_f_args.arg4);
      ent_DecRef(ext_f_args.arg5);
      MDPTR(p_mdr) = p_save;
      break;
  }
  ent_DecRef(ext_f_args.arg1);
  ent_DecRef(ext_f_args.arg2);

  if (ent_type(rent)!=MATRIX_DENSE_REAL)
  {
    rerror (RLAB_ERROR_RHS_FUNC_MUST_RETURN_MDR);
  }

  MDR *retm = ent_data(rent);
  if (SIZE(retm)!=gdata->Ngt)
  {
    fprintf(stderr, THIS_SOLVER ": [dgTdT] " RLAB_ERROR_RHS_FUNC_INCORRECT_DIM "\n");
    fprintf(stderr, THIS_SOLVER ": [dgTdT] Expected dimension (%i), RHS dimension (%i)\n",
            gdata->Ngt,SIZE(retm));
    fprintf(stderr, THIS_SOLVER ": [dgTdT] Check the RHS function: Cannot continue!\n");
    rerror (THIS_SOLVER);
  }

  for (i=0; i < gdata->Ngt; i++)
  {
      out[i] = (typeRNum) MdrV0 (retm, i);
  }

  ent_Clean(rent);
  return;
}


void dgTdT_vec(typeRNum *out, ctypeRNum T, ctypeRNum *x, ctypeRNum *p,
               ctypeRNum *vec, typeUSERPARAM *userparam)
{
  GRAMPC_TABLE *gdata=(GRAMPC_TABLE *) userparam;
  if (!gdata->have_dgTdT)
  {
    return;
  }

  int i;
  MDR *retm = mdr_Create(1,gdata->Ngt);
  dgTdT((typeRNum *) MDRPTR(retm), T, x, p, userparam);
  out[0] = 0;
  for (i=0; i < gdata->Ngt; i++)
  {
    out[0] = out[0] + MdrV0 (retm, i) * vec[i];
  }

  mdr_Destroy(retm);


  return;
}


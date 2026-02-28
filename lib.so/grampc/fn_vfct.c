/** Terminal cost V(T,x(T),p,xdes,userparam)
    ---------------------------------------- **/
void Vfct(typeRNum *out, ctypeRNum T, ctypeRNum *x, ctypeRNum *p,
          ctypeRNum *xdes, typeUSERPARAM *userparam)
{
  GRAMPC_TABLE *gdata=(GRAMPC_TABLE *) userparam;
  if (!gdata->have_Vfct)
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
  // x_des:
  MDR *x_des_mdr = ent_data(ext_f_args.arg7);
  MDPTR(x_mdr) = (void *) xdes;

  // incref, call, then decref
  ent_IncRef(ext_f_args.arg1);
  ent_IncRef(ext_f_args.arg2);
  ent_IncRef(ext_f_args.arg7);
  switch(ext_f_args.n_args)
  {
    case 3:
      // vfct(t,x,x_des,u)
      rent = ent_call_rlab_script_3args(gdata->ent_fn_Vfct,
                                        ext_f_args.arg1, /* t */
                                        ext_f_args.arg2, /* x */
                                        ext_f_args.arg7  /* x_des */
                                       );
      break;

    case -4:
      // vfct(t,x,x_des,s)
      ent_IncRef(ext_f_args.arg4);
      rent = ent_call_rlab_script_4args (gdata->ent_fn_Vfct,
                                         ext_f_args.arg1, /* t */
                                         ext_f_args.arg2, /* x */
                                         ext_f_args.arg7, /* x_des */
                                         ext_f_args.arg4  /* s */
                                        );
      ent_DecRef(ext_f_args.arg4);
      break;

    case 4:
      // vfct(t,x,x_des,p)
      // p:
      p_mdr   = ent_data(ext_f_args.arg4);
      p_save  = MDRPTR(p_mdr);
      MDPTR(p_mdr) = (void *) p;
      ent_IncRef(ext_f_args.arg4);
      rent = ent_call_rlab_script_4args (gdata->ent_fn_Vfct,
                                         ext_f_args.arg1, /* t */
                                         ext_f_args.arg2, /* x */
                                         ext_f_args.arg7, /* x_des */
                                         ext_f_args.arg4  /* p */
                                        );
      MDPTR(p_mdr) = p_save;
      ent_DecRef(ext_f_args.arg4);
      break;

    case -5:
      // vfct(t,x,u,p,s)
      // p:
      p_mdr   = ent_data(ext_f_args.arg4);
      p_save  = MDRPTR(p_mdr);
      MDPTR(p_mdr) = (void *) p;
      ent_IncRef(ext_f_args.arg4);
      ent_IncRef(ext_f_args.arg5);
      rent = ent_call_rlab_script_5args (gdata->ent_fn_Vfct,
                                         ext_f_args.arg1, /* t */
                                         ext_f_args.arg2, /* x */
                                         ext_f_args.arg7, /* x_des */
                                         ext_f_args.arg4, /* p */
                                         ext_f_args.arg5  /* s */
                                        );
      ent_DecRef(ext_f_args.arg4);
      ent_DecRef(ext_f_args.arg5);
      MDPTR(p_mdr) = p_save;
      break;
  }
  ent_DecRef(ext_f_args.arg1);
  ent_DecRef(ext_f_args.arg2);
  ent_DecRef(ext_f_args.arg7);

  if (ent_type(rent)!=MATRIX_DENSE_REAL)
  {
    rerror (RLAB_ERROR_RHS_FUNC_MUST_RETURN_MDR);
  }

  MDR *retm = ent_data(rent);
  if (SIZE(retm)!=1)
  {
    fprintf(stderr, THIS_SOLVER ": [vfct] " RLAB_ERROR_RHS_FUNC_INCORRECT_DIM "\n");
    fprintf(stderr, THIS_SOLVER ": [vfct] Expected dimension %i, RHS dimension %i\n", 1, SIZE(retm));
    fprintf(stderr, THIS_SOLVER ": [vfct] Check the RHS function: Cannot continue!\n");
    rerror (THIS_SOLVER);
  }

  out[0] = (typeRNum) MdrV0 (retm, 0);

  ent_Clean(rent);
  return;
}

/** Gradient dV/dx **/
void dVdx(typeRNum *out, ctypeRNum T, ctypeRNum *x, ctypeRNum *p,
          ctypeRNum *xdes, typeUSERPARAM *userparam)
{
  GRAMPC_TABLE *gdata=(GRAMPC_TABLE *) userparam;
  if (!gdata->have_dVdx)
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
  // x_des:
  MDR *x_des_mdr = ent_data(ext_f_args.arg7);
  MDPTR(x_des_mdr) = (void *) xdes;

  // incref, call, then decref
  ent_IncRef(ext_f_args.arg1);
  ent_IncRef(ext_f_args.arg2);
  ent_IncRef(ext_f_args.arg7);
  switch(ext_f_args.n_args)
  {
    case 3:
      // vfct(t,x,x_des,u)
      rent = ent_call_rlab_script_3args(gdata->ent_fn_dVdx,
                                        ext_f_args.arg1, /* t */
                                        ext_f_args.arg2, /* x */
                                        ext_f_args.arg7  /* x_des */
                                       );
      break;

    case -4:
      // vfct(t,x,x_des,s)
      ent_IncRef(ext_f_args.arg4);
      rent = ent_call_rlab_script_4args (gdata->ent_fn_dVdx,
                                         ext_f_args.arg1, /* t */
                                         ext_f_args.arg2, /* x */
                                         ext_f_args.arg7, /* x_des */
                                         ext_f_args.arg4  /* s */
                                        );
      ent_DecRef(ext_f_args.arg4);
      break;

    case 4:
      // vfct(t,x,x_des,p)
      // p:
      p_mdr   = ent_data(ext_f_args.arg4);
      p_save  = MDRPTR(p_mdr);
      MDPTR(p_mdr) = (void *) p;
      ent_IncRef(ext_f_args.arg4);
      rent = ent_call_rlab_script_4args (gdata->ent_fn_dVdx,
                                         ext_f_args.arg1, /* t */
                                         ext_f_args.arg2, /* x */
                                         ext_f_args.arg7, /* x_des */
                                         ext_f_args.arg4  /* p */
                                        );
      MDPTR(p_mdr) = p_save;
      ent_DecRef(ext_f_args.arg4);
      break;

    case -5:
      // vfct(t,x,u,p,s)
      // p:
      p_mdr   = ent_data(ext_f_args.arg4);
      p_save  = MDRPTR(p_mdr);
      MDPTR(p_mdr) = (void *) p;
      ent_IncRef(ext_f_args.arg4);
      ent_IncRef(ext_f_args.arg5);
      rent = ent_call_rlab_script_5args (gdata->ent_fn_dVdx,
                                         ext_f_args.arg1, /* t */
                                         ext_f_args.arg2, /* x */
                                         ext_f_args.arg7, /* x_des */
                                         ext_f_args.arg4, /* p */
                                         ext_f_args.arg5  /* s */
                                        );
      ent_DecRef(ext_f_args.arg4);
      ent_DecRef(ext_f_args.arg5);
      MDPTR(p_mdr) = p_save;
      break;
  }
  ent_DecRef(ext_f_args.arg1);
  ent_DecRef(ext_f_args.arg2);
  ent_DecRef(ext_f_args.arg7);

  if (ent_type(rent)!=MATRIX_DENSE_REAL)
  {
    rerror (RLAB_ERROR_RHS_FUNC_MUST_RETURN_MDR);
  }

  MDR *retm = ent_data(rent);
  if (SIZE(retm)!=gdata->Nx)
  {
    fprintf(stderr, THIS_SOLVER ": [dvdx] " RLAB_ERROR_RHS_FUNC_INCORRECT_DIM "\n");
    fprintf(stderr, THIS_SOLVER ": [dvdx] Expected dimension %i, RHS dimension %i\n", 1, SIZE(retm));
    fprintf(stderr, THIS_SOLVER ": [dvdx] Check the RHS function: Cannot continue!\n");
    rerror (THIS_SOLVER);
  }

  for (i=0; i<gdata->Nx; i++)
    out[i] = (typeRNum) MdrV0 (retm, i);

  ent_Clean(rent);
  return;
}
/** Gradient dV/dp **/
void dVdp(typeRNum *out, ctypeRNum T, ctypeRNum *x, ctypeRNum *p,
          ctypeRNum *xdes, typeUSERPARAM *userparam)
{
  GRAMPC_TABLE *gdata=(GRAMPC_TABLE *) userparam;
  if (!gdata->have_dVdp)
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
  // x_des:
  MDR *x_des_mdr = ent_data(ext_f_args.arg7);
  MDPTR(x_des_mdr) = (void *) xdes;

  // incref, call, then decref
  ent_IncRef(ext_f_args.arg1);
  ent_IncRef(ext_f_args.arg2);
  ent_IncRef(ext_f_args.arg7);
  switch(ext_f_args.n_args)
  {
    case 4:
      // vfct(t,x,x_des,p)
      // p:
      p_mdr   = ent_data(ext_f_args.arg4);
      p_save  = MDRPTR(p_mdr);
      MDPTR(p_mdr) = (void *) p;
      ent_IncRef(ext_f_args.arg4);
      rent = ent_call_rlab_script_4args (gdata->ent_fn_dVdp,
                                         ext_f_args.arg1, /* t */
                                         ext_f_args.arg2, /* x */
                                         ext_f_args.arg7, /* x_des */
                                         ext_f_args.arg4  /* p */
                                        );
      MDPTR(p_mdr) = p_save;
      ent_DecRef(ext_f_args.arg4);
      break;

    case -5:
      // vfct(t,x,u,p,s)
      // p:
      p_mdr   = ent_data(ext_f_args.arg4);
      p_save  = MDRPTR(p_mdr);
      MDPTR(p_mdr) = (void *) p;
      ent_IncRef(ext_f_args.arg4);
      ent_IncRef(ext_f_args.arg5);
      rent = ent_call_rlab_script_5args (gdata->ent_fn_dVdp,
                                         ext_f_args.arg1, /* t */
                                         ext_f_args.arg2, /* x */
                                         ext_f_args.arg7, /* x_des */
                                         ext_f_args.arg4, /* p */
                                         ext_f_args.arg5  /* s */
                                        );
      ent_DecRef(ext_f_args.arg4);
      ent_DecRef(ext_f_args.arg5);
      MDPTR(p_mdr) = p_save;
      break;
  }
  ent_DecRef(ext_f_args.arg1);
  ent_DecRef(ext_f_args.arg2);
  ent_DecRef(ext_f_args.arg7);

  if (ent_type(rent)!=MATRIX_DENSE_REAL)
  {
    rerror (RLAB_ERROR_RHS_FUNC_MUST_RETURN_MDR);
  }

  MDR *retm = ent_data(rent);
  if (SIZE(retm)!=1)
  {
    fprintf(stderr, THIS_SOLVER ": [dVdp] " RLAB_ERROR_RHS_FUNC_INCORRECT_DIM "\n");
    fprintf(stderr, THIS_SOLVER ": [dVdp] Expected dimension %i, RHS dimension %i\n", 1, SIZE(retm));
    fprintf(stderr, THIS_SOLVER ": [dVdp] Check the RHS function: Cannot continue!\n");
    rerror (THIS_SOLVER);
  }

  out[0] = (typeRNum) MdrV0 (retm, 0);

  ent_Clean(rent);
  return;
}

/** Gradient dV/dT **/
void dVdT(typeRNum *out, ctypeRNum T, ctypeRNum *x, ctypeRNum *p,
          ctypeRNum *xdes, typeUSERPARAM *userparam)
{
  GRAMPC_TABLE *gdata=(GRAMPC_TABLE *) userparam;
  if (!gdata->have_dVdT)
  {
    return;
  }
  Ent *rent=0;
  MDR *p_mdr=NULL;
  void *p_save=NULL;

  // pointer shuffle:
  r_extern_func_args ext_f_args = (r_extern_func_args) gdata->ext_f_args;
  // t:
  MDR *t_mdr = ent_data(ext_f_args.arg1);
  MDPTR(t_mdr) = (void *) &T;
  // x:
  MDR *x_mdr = ent_data(ext_f_args.arg2);
  MDPTR(x_mdr) = (void *) x;
  // x_des:
  MDR *x_des_mdr = ent_data(ext_f_args.arg7);
  MDPTR(x_des_mdr) = (void *) xdes;

  // incref, call, then decref
  ent_IncRef(ext_f_args.arg1);
  ent_IncRef(ext_f_args.arg2);
  ent_IncRef(ext_f_args.arg7);
  switch(ext_f_args.n_args)
  {
    case 3:
      // dVdT(t,x,x_des)
      rent = ent_call_rlab_script_3args(gdata->ent_fn_dVdT,
                                        ext_f_args.arg1, /* t */
                                        ext_f_args.arg2, /* x */
                                        ext_f_args.arg7  /* x_des */
                                       );
      break;

    case -4:
      // dVdT(t,x,x_des,s)
      ent_IncRef(ext_f_args.arg4);
      rent = ent_call_rlab_script_4args (gdata->ent_fn_dVdT,
                                         ext_f_args.arg1, /* t */
                                         ext_f_args.arg2, /* x */
                                         ext_f_args.arg7, /* x_des */
                                         ext_f_args.arg4  /* s */
                                        );
      ent_DecRef(ext_f_args.arg4);
      break;

    case 4:
      // dVdT(t,x,x_des,p)
      // p:
      p_mdr   = ent_data(ext_f_args.arg4);
      p_save  = MDRPTR(p_mdr);
      MDPTR(p_mdr) = (void *) p;
      ent_IncRef(ext_f_args.arg4);
      rent = ent_call_rlab_script_4args (gdata->ent_fn_dVdT,
                                         ext_f_args.arg1, /* t */
                                         ext_f_args.arg2, /* x */
                                         ext_f_args.arg7, /* x_des */
                                         ext_f_args.arg4  /* p */
                                        );
      MDPTR(p_mdr) = p_save;
      ent_DecRef(ext_f_args.arg4);
      break;

    case -5:
      // dVdT(t,x,u,p,s)
      // p:
      p_mdr   = ent_data(ext_f_args.arg4);
      p_save  = MDRPTR(p_mdr);
      MDPTR(p_mdr) = (void *) p;
      ent_IncRef(ext_f_args.arg4);
      ent_IncRef(ext_f_args.arg5);
      rent = ent_call_rlab_script_5args (gdata->ent_fn_dVdT,
                                         ext_f_args.arg1, /* t */
                                         ext_f_args.arg2, /* x */
                                         ext_f_args.arg7, /* x_des */
                                         ext_f_args.arg4, /* p */
                                         ext_f_args.arg5  /* s */
                                        );
      ent_DecRef(ext_f_args.arg4);
      ent_DecRef(ext_f_args.arg5);
      MDPTR(p_mdr) = p_save;
      break;
  }
  ent_DecRef(ext_f_args.arg1);
  ent_DecRef(ext_f_args.arg2);
  ent_DecRef(ext_f_args.arg7);

  if (ent_type(rent)!=MATRIX_DENSE_REAL)
  {
    rerror (RLAB_ERROR_RHS_FUNC_MUST_RETURN_MDR);
  }

  MDR *retm = ent_data(rent);
  if (SIZE(retm)!=1)
  {
    fprintf(stderr, THIS_SOLVER ": [dvdt] " RLAB_ERROR_RHS_FUNC_INCORRECT_DIM "\n");
    fprintf(stderr, THIS_SOLVER ": [dvdt] Expected dimension %i, RHS dimension %i\n", 1, SIZE(retm));
    fprintf(stderr, THIS_SOLVER ": [dvdt] Check the RHS function: Cannot continue!\n");
    rerror (THIS_SOLVER);
  }

  out[0] = (typeRNum) MdrV0 (retm, 0);

  ent_Clean(rent);
  return;
}

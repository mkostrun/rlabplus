/** Integral cost l(t,x(t),u(t),p,xdes,udes,userparam)
    -------------------------------------------------- **/
void lfct(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u,
          ctypeRNum *p, ctypeRNum *xdes, ctypeRNum *udes, typeUSERPARAM *userparam)
{
  GRAMPC_TABLE *gdata=(GRAMPC_TABLE *) userparam;
  if (!gdata->have_lfct)
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
  // x_des:
  MDR *x_des_mdr = ent_data(ext_f_args.arg7);
  MDPTR(x_des_mdr) = (void *) xdes;
  // u_des:
  MDR *u_des_mdr = ent_data(ext_f_args.arg8);
  MDPTR(u_des_mdr) = (void *) udes;

  // incref, call, then decref
  ent_IncRef(ext_f_args.arg1);
  ent_IncRef(ext_f_args.arg2);
  ent_IncRef(ext_f_args.arg3);
  ent_IncRef(ext_f_args.arg7);
  ent_IncRef(ext_f_args.arg8);
  switch(ext_f_args.n_args)
  {
    case 3:
      // lfct(t,x,x_des,u,u_des)
      rent = ent_call_rlab_script_5args(gdata->ent_fn_lfct,
                                        ext_f_args.arg1, /* t */
                                        ext_f_args.arg2, /* x */
                                        ext_f_args.arg7, /* x_des */
                                        ext_f_args.arg3, /* u */
                                        ext_f_args.arg8  /* u_des */
                                       );
      break;

    case -4:
      // lfct(t,x,x_des,u,u_des,s)
      ent_IncRef(ext_f_args.arg4);
      rent = ent_call_rlab_script_6args (gdata->ent_fn_lfct,
                                         ext_f_args.arg1, /* t */
                                         ext_f_args.arg2, /* x */
                                         ext_f_args.arg7, /* x_des */
                                         ext_f_args.arg3, /* u */
                                         ext_f_args.arg8, /* u_des */
                                         ext_f_args.arg4  /* s */
                                        );
      ent_DecRef(ext_f_args.arg4);
      break;

    case 4:
      // lfct(t,x,x_des,u,u_des,p)
      // p:
      p_mdr   = ent_data(ext_f_args.arg4);
      p_save  = MDRPTR(p_mdr);
      MDPTR(p_mdr) = (void *) p;
      ent_IncRef(ext_f_args.arg4);
      rent = ent_call_rlab_script_6args (gdata->ent_fn_lfct,
                                         ext_f_args.arg1, /* t */
                                         ext_f_args.arg2, /* x */
                                         ext_f_args.arg7, /* x_des */
                                         ext_f_args.arg3, /* u */
                                         ext_f_args.arg8, /* u_des */
                                         ext_f_args.arg4  /* p */
                                        );
      ent_DecRef(ext_f_args.arg4);
      MDPTR(p_mdr) = p_save;
      break;

    case -5:
      // lfct(t,x,u,p,s)
      // p:
      p_mdr   = ent_data(ext_f_args.arg4);
      p_save  = MDRPTR(p_mdr);
      MDPTR(p_mdr) = (void *) p;
      ent_IncRef(ext_f_args.arg4);
      ent_IncRef(ext_f_args.arg5);
      rent = ent_call_rlab_script_7args (gdata->ent_fn_lfct,
                                         ext_f_args.arg1, /* t */
                                         ext_f_args.arg2, /* x */
                                         ext_f_args.arg7, /* x_des */
                                         ext_f_args.arg3, /* u */
                                         ext_f_args.arg8, /* u_des */
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
  ent_DecRef(ext_f_args.arg3);
  ent_DecRef(ext_f_args.arg7);
  ent_DecRef(ext_f_args.arg8);

  if (ent_type(rent)!=MATRIX_DENSE_REAL)
  {
    rerror (RLAB_ERROR_RHS_FUNC_MUST_RETURN_MDR);
  }

  MDR *retm = ent_data(rent);
  if (SIZE(retm)!=1)
  {
    fprintf(stderr, THIS_SOLVER ": [lfct] " RLAB_ERROR_RHS_FUNC_INCORRECT_DIM "\n");
    fprintf(stderr, THIS_SOLVER ": [lfct] Expected dimension %i, RHS dimension %i\n", 1, SIZE(retm));
    fprintf(stderr, THIS_SOLVER ": [lfct] Check the RHS function: Cannot continue!\n");
    rerror (THIS_SOLVER);
  }

  out[0] = (typeRNum) MdrV0 (retm, 0);

  ent_Clean(rent);
  return;
}

/** Gradient dl/dx **/
void dldx(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u,
          ctypeRNum *p, ctypeRNum *xdes, ctypeRNum *udes, typeUSERPARAM *userparam)
{
  GRAMPC_TABLE *gdata=(GRAMPC_TABLE *) userparam;
  if (!gdata->have_dldx)
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
  // x_des:
  MDR *x_des_mdr = ent_data(ext_f_args.arg7);
  MDPTR(x_des_mdr) = (void *) xdes;
  // u_des:
  MDR *u_des_mdr = ent_data(ext_f_args.arg8);
  MDPTR(u_des_mdr) = (void *) udes;

  // incref, call, then decref
  ent_IncRef(ext_f_args.arg1);
  ent_IncRef(ext_f_args.arg2);
  ent_IncRef(ext_f_args.arg3);
  ent_IncRef(ext_f_args.arg7);
  ent_IncRef(ext_f_args.arg8);
  switch(ext_f_args.n_args)
  {
    case 3:
      // lfct(t,x,x_des,u,u_des)
      rent = ent_call_rlab_script_with_args(gdata->ent_fn_dldx, 5,
                                        ext_f_args.arg1, /* t */
                                        ext_f_args.arg2, /* x */
                                        ext_f_args.arg7, /* x_des */
                                        ext_f_args.arg3, /* u */
                                        ext_f_args.arg8  /* u_des */
                                       );
      break;

    case -4:
      // lfct(t,x,x_des,u,u_des,s)
      ent_IncRef(ext_f_args.arg4);
      rent = ent_call_rlab_script_with_args (gdata->ent_fn_dldx, 6,
                                         ext_f_args.arg1, /* t */
                                         ext_f_args.arg2, /* x */
                                         ext_f_args.arg7, /* x_des */
                                         ext_f_args.arg3, /* u */
                                         ext_f_args.arg8, /* u_des */
                                         ext_f_args.arg4  /* s */
                                        );
      ent_DecRef(ext_f_args.arg4);
      break;

    case 4:
      // lfct(t,x,x_des,u,u_des,p)
      // p:
      p_mdr   = ent_data(ext_f_args.arg4);
      p_save  = MDRPTR(p_mdr);
      MDPTR(p_mdr) = (void *) p;
      ent_IncRef(ext_f_args.arg4);
      rent = ent_call_rlab_script_with_args (gdata->ent_fn_dldx, 6,
                                         ext_f_args.arg1, /* t */
                                         ext_f_args.arg2, /* x */
                                         ext_f_args.arg7, /* x_des */
                                         ext_f_args.arg3, /* u */
                                         ext_f_args.arg8, /* u_des */
                                         ext_f_args.arg4  /* p */
                                        );
      ent_DecRef(ext_f_args.arg4);
      MDPTR(p_mdr) = p_save;
      break;

    case -5:
      // lfct(t,x,u,p,s)
      // p:
      p_mdr   = ent_data(ext_f_args.arg4);
      p_save  = MDRPTR(p_mdr);
      MDPTR(p_mdr) = (void *) p;
      ent_IncRef(ext_f_args.arg4);
      ent_IncRef(ext_f_args.arg5);
      rent = ent_call_rlab_script_with_args (gdata->ent_fn_dldx, 7,
                                         ext_f_args.arg1, /* t */
                                         ext_f_args.arg2, /* x */
                                         ext_f_args.arg7, /* x_des */
                                         ext_f_args.arg3, /* u */
                                         ext_f_args.arg8, /* u_des */
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
  ent_DecRef(ext_f_args.arg3);
  ent_DecRef(ext_f_args.arg7);
  ent_DecRef(ext_f_args.arg8);

  if (ent_type(rent)!=MATRIX_DENSE_REAL)
  {
    rerror (RLAB_ERROR_RHS_FUNC_MUST_RETURN_MDR);
  }

  MDR *retm = ent_data(rent);
  if (SIZE(retm)!=gdata->Nx)
  {
    fprintf(stderr, THIS_SOLVER ": [dldx] " RLAB_ERROR_RHS_FUNC_INCORRECT_DIM "\n");
    fprintf(stderr, THIS_SOLVER ": [dldx] Expected dimension %i, RHS dimension %i\n", gdata->Nx, SIZE(retm));
    fprintf(stderr, THIS_SOLVER ": [dldx] Check the RHS function: Cannot continue!\n");
    rerror (THIS_SOLVER);
  }

  for (i=0; i<gdata->Nx; i++)
  {
    out[i] = (typeRNum) MdrV0 (retm, i);
  }

  ent_Clean(rent);
  return;
}
/** Gradient dl/du **/
void dldu(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u,
          ctypeRNum *p, ctypeRNum *xdes, ctypeRNum *udes, typeUSERPARAM *userparam)
{
  GRAMPC_TABLE *gdata=(GRAMPC_TABLE *) userparam;
  if (!gdata->have_dldu)
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
  // x_des:
  MDR *x_des_mdr = ent_data(ext_f_args.arg7);
  MDPTR(x_des_mdr) = (void *) xdes;
  // u_des:
  MDR *u_des_mdr = ent_data(ext_f_args.arg8);
  MDPTR(u_des_mdr) = (void *) udes;

  // incref, call, then decref
  ent_IncRef(ext_f_args.arg1);
  ent_IncRef(ext_f_args.arg2);
  ent_IncRef(ext_f_args.arg3);
  ent_IncRef(ext_f_args.arg7);
  ent_IncRef(ext_f_args.arg8);
  switch(ext_f_args.n_args)
  {
    case 3:
      // lfct(t,x,x_des,u,u_des)
      rent = ent_call_rlab_script_5args(gdata->ent_fn_dldu,
                                        ext_f_args.arg1, /* t */
                                        ext_f_args.arg2, /* x */
                                        ext_f_args.arg7, /* x_des */
                                        ext_f_args.arg3, /* u */
                                        ext_f_args.arg8  /* u_des */
                                       );
      break;

    case -4:
      // lfct(t,x,x_des,u,u_des,s)
      ent_IncRef(ext_f_args.arg4);
      rent = ent_call_rlab_script_6args (gdata->ent_fn_dldu,
                                         ext_f_args.arg1, /* t */
                                         ext_f_args.arg2, /* x */
                                         ext_f_args.arg7, /* x_des */
                                         ext_f_args.arg3, /* u */
                                         ext_f_args.arg8, /* u_des */
                                         ext_f_args.arg4  /* s */
                                        );
      ent_DecRef(ext_f_args.arg4);
      break;

    case 4:
      // lfct(t,x,x_des,u,u_des,p)
      // p:
      p_mdr   = ent_data(ext_f_args.arg4);
      p_save  = MDRPTR(p_mdr);
      MDPTR(p_mdr) = (void *) p;
      ent_IncRef(ext_f_args.arg4);
      rent = ent_call_rlab_script_6args (gdata->ent_fn_dldu,
                                         ext_f_args.arg1, /* t */
                                         ext_f_args.arg2, /* x */
                                         ext_f_args.arg7, /* x_des */
                                         ext_f_args.arg3, /* u */
                                         ext_f_args.arg8, /* u_des */
                                         ext_f_args.arg4  /* p */
                                        );
      ent_DecRef(ext_f_args.arg4);
      MDPTR(p_mdr) = p_save;
      break;

    case -5:
      // lfct(t,x,u,p,s)
      // p:
      p_mdr   = ent_data(ext_f_args.arg4);
      p_save  = MDRPTR(p_mdr);
      MDPTR(p_mdr) = (void *) p;
      ent_IncRef(ext_f_args.arg4);
      ent_IncRef(ext_f_args.arg5);
      rent = ent_call_rlab_script_7args (gdata->ent_fn_dldu,
                                         ext_f_args.arg1, /* t */
                                         ext_f_args.arg2, /* x */
                                         ext_f_args.arg7, /* x_des */
                                         ext_f_args.arg3, /* u */
                                         ext_f_args.arg8, /* u_des */
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
  ent_DecRef(ext_f_args.arg3);
  ent_DecRef(ext_f_args.arg7);
  ent_DecRef(ext_f_args.arg8);

  if (ent_type(rent)!=MATRIX_DENSE_REAL)
  {
    rerror (RLAB_ERROR_RHS_FUNC_MUST_RETURN_MDR);
  }

  MDR *retm = ent_data(rent);
  if (SIZE(retm)!=gdata->Nu)
  {
    fprintf(stderr, THIS_SOLVER ": [dldu] " RLAB_ERROR_RHS_FUNC_INCORRECT_DIM "\n");
    fprintf(stderr, THIS_SOLVER ": [dldu] Expected dimension %i, RHS dimension %i\n", gdata->Nx, SIZE(retm));
    fprintf(stderr, THIS_SOLVER ": [dldu] Check the RHS function: Cannot continue!\n");
    rerror (THIS_SOLVER);
  }

  for (i=0; i<gdata->Nu; i++)
  {
    out[i] = (typeRNum) MdrV0 (retm, i);
  }

  ent_Clean(rent);
  return;
}

/** Gradient dl/dp **/
void dldp(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u,
          ctypeRNum *p, ctypeRNum *xdes, ctypeRNum *udes, typeUSERPARAM *userparam)
{
  GRAMPC_TABLE *gdata=(GRAMPC_TABLE *) userparam;
  if (!gdata->have_dldp)
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
  // x_des:
  MDR *x_des_mdr = ent_data(ext_f_args.arg7);
  MDPTR(x_des_mdr) = (void *) xdes;
  // u_des:
  MDR *u_des_mdr = ent_data(ext_f_args.arg8);
  MDPTR(u_des_mdr) = (void *) udes;

  // incref, call, then decref
  ent_IncRef(ext_f_args.arg1);
  ent_IncRef(ext_f_args.arg2);
  ent_IncRef(ext_f_args.arg3);
  ent_IncRef(ext_f_args.arg7);
  ent_IncRef(ext_f_args.arg8);
  switch(ext_f_args.n_args)
  {
    case 4:
      // lfct(t,x,x_des,u,u_des,p)
      // p:
      p_mdr   = ent_data(ext_f_args.arg4);
      p_save  = MDRPTR(p_mdr);
      MDPTR(p_mdr) = (void *) p;
      ent_IncRef(ext_f_args.arg4);
      rent = ent_call_rlab_script_6args (gdata->ent_fn_dldu,
                                         ext_f_args.arg1, /* t */
                                         ext_f_args.arg2, /* x */
                                         ext_f_args.arg7, /* x_des */
                                         ext_f_args.arg3, /* u */
                                         ext_f_args.arg8, /* u_des */
                                         ext_f_args.arg4  /* p */
                                        );
      ent_DecRef(ext_f_args.arg4);
      MDPTR(p_mdr) = p_save;
      break;

    case -5:
      // lfct(t,x,u,p,s)
      // p:
      p_mdr   = ent_data(ext_f_args.arg4);
      p_save  = MDRPTR(p_mdr);
      MDPTR(p_mdr) = (void *) p;
      ent_IncRef(ext_f_args.arg4);
      ent_IncRef(ext_f_args.arg5);
      rent = ent_call_rlab_script_7args (gdata->ent_fn_dldu,
                                         ext_f_args.arg1, /* t */
                                         ext_f_args.arg2, /* x */
                                         ext_f_args.arg7, /* x_des */
                                         ext_f_args.arg3, /* u */
                                         ext_f_args.arg8, /* u_des */
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
  ent_DecRef(ext_f_args.arg3);
  ent_DecRef(ext_f_args.arg7);
  ent_DecRef(ext_f_args.arg8);

  if (ent_type(rent)!=MATRIX_DENSE_REAL)
  {
    rerror (RLAB_ERROR_RHS_FUNC_MUST_RETURN_MDR);
  }

  MDR *retm = ent_data(rent);
  if (SIZE(retm)!=gdata->Nu)
  {
    fprintf(stderr, THIS_SOLVER ": [dldu] " RLAB_ERROR_RHS_FUNC_INCORRECT_DIM "\n");
    fprintf(stderr, THIS_SOLVER ": [dldu] Expected dimension %i, RHS dimension %i\n", gdata->Nx, SIZE(retm));
    fprintf(stderr, THIS_SOLVER ": [dldu] Check the RHS function: Cannot continue!\n");
    rerror (THIS_SOLVER);
  }

  for (i=0; i<gdata->Nu; i++)
  {
    out[i] = (typeRNum) MdrV0 (retm, i);
  }

  ent_Clean(rent);
  return;
}


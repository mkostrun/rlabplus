/**
  *
  * Additional functions required for semi-implicit systems
  *
  * M*dx/dt(t) = f(t0+t,x(t),u(t),p) using the solver RODAS
  *
  * ------------------------------------------------------- **/

/** Jacobian d(dH/dx)/dt  **/
void dHdxdt(typeRNum *out, ctypeRNum t, ctypeRNum *x, ctypeRNum *u,
            ctypeRNum *vec, ctypeRNum *p, typeUSERPARAM *userparam)
{
  GRAMPC_TABLE *gdata=(GRAMPC_TABLE *) userparam;
  if (!gdata->have_dHdxdt)
  {
    return;
  }
  return;
}

/** Mass matrix in vector form (column-wise, either banded or full matrix) **/
void Mfct(typeRNum *out, typeUSERPARAM *userparam)
{
  GRAMPC_TABLE *gdata=(GRAMPC_TABLE *) userparam;
  if (!gdata->have_Mfct)
  {
    return;
  }

  int i;
  Ent *rent=0;
  r_extern_func_args ext_f_args = (r_extern_func_args) gdata->ext_f_args;


  // incref, call, then decref
  switch(ext_f_args.n_args)
  {
    case 3:
      // mass()
      rent = ent_call_rlab_script_with_args (gdata->ent_fn_Mfct, 0);
      break;

    case -4:
      // mass(s)
      ent_IncRef(ext_f_args.arg4);
      rent = ent_call_rlab_script_with_args (gdata->ent_fn_Mfct, 1, ext_f_args.arg4);
      ent_DecRef(ext_f_args.arg4);
      break;
  }

  if (ent_type(rent)!=MATRIX_DENSE_REAL)
  {
    rerror (RLAB_ERROR_RHS_FUNC_MUST_RETURN_MDR);
  }

  MDR *retm = ent_data(rent);
  if ((MNR(retm)!=gdata->Nx)||(MNC(retm)!=gdata->Nx))
  {
    fprintf(stderr, THIS_SOLVER ": [mass] " RLAB_ERROR_RHS_FUNC_INCORRECT_DIM "\n");
    fprintf(stderr, THIS_SOLVER ": [mass] Expected dimension %i, RHS dimension %i\n", gdata->Nx, SIZE(retm));
    fprintf(stderr, THIS_SOLVER ": [mass] Check the RHS function: Cannot continue!\n");
    rerror ("odeiv");
  }

  for (i=0; i < (gdata->Nx)*(gdata->Nx); i++)
  {
    out[i] = (typeRNum) MdrV0 (retm, i);
  }

  ent_Clean(rent);
  return;
}

/** Transposed mass matrix in vector form (column-wise, either banded or full matrix) **/
void Mtrans(typeRNum *out, typeUSERPARAM *userparam)
{
  GRAMPC_TABLE *gdata=(GRAMPC_TABLE *) userparam;
  if (!gdata->have_Mfct)
  {
    return;
  }

  // Find MFCT
  Mfct((typeRNum *)out, (typeUSERPARAM *)userparam);

  // transpose it in place
  md_transpose_insitu((unsigned char *)out, gdata->Nx, gdata->Nx, sizeof(double));
  return;
}


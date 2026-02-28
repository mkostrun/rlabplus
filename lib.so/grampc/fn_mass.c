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
  return;
}

/** Transposed mass matrix in vector form (column-wise, either banded or full matrix) **/
void Mtrans(typeRNum *out, typeUSERPARAM *userparam)
{
  GRAMPC_TABLE *gdata=(GRAMPC_TABLE *) userparam;
  if (!gdata->have_Mtrans)
  {
    return;
  }
}


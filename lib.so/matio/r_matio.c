/* r_matio.c */

/*  This file is a part of RLaB2/rlabplus ("Our"-LaB)
   Copyright (C) 2005-2022 M. Kostrun

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   See the file ./COPYING
   ********************************************************************** */



#include "r_matio.h"

#undef  THIS_FILE
#define THIS_FILE "r_matio.c"
/*
 * Copyright (c) 2015-2022, The matio contributors
 * Copyright (c) 2005-2014, Christopher C. Hulbert
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
static enum mat_ft mat_file_ver = MAT_FT_DEFAULT;
static enum matio_compression compression = MAT_COMPRESSION_NONE;

const char * rlab_matio_default_header = "rlab was here: all your base are belong to us!";

static Mfile *mfile_list = 0;   // Ptr to 1st Mfile
static int nrfile = 0;          // Number of open M-files

//
// Create a new Mfile struct, and tack it on
// the front of the list. Return a ptr to the
// new Mfile struct. Make sure and bump the
// list element count.
//
static Mfile * mfile_Create (void)
{
  Mfile *new = (Mfile *) GC_MALLOC (sizeof (Rfile));
  if (new == 0)
    rerror ("out of memory");

  // finish
  new->next = mfile_list;
  nrfile++;
  mfile_list = new;
  return (new);
}

static int mfile_Destroy (const char *name)
{
  Mfile *rf, *next, *prev;

  rf = (Mfile *) mfile_list;
  next = rf->next;
  prev = rf;

  while (next)
  {
    const char *filename = Mat_GetFilename(rf->mat);
    if (!strcmp (filename, name))
    {
      // Found it!

      // Re-Hook up list
      prev->next = next;

      // Check two special cases
      if (prev == rf)     // 1st element
        mfile_list = next;
      else if (next == 0) // Last element
        prev->next = 0;

      // Now destroy the element
      Mat_Close((mat_t *)rf->mat);

      GC_FREE (rf);

      nrfile--;
      return (0);   /* Success: an rfile is destroyed */
    }

    prev = rf;
    rf = next;
    next = rf->next;
  }

  return (1);     /* Failure: no rfile with such name was found, so none was destroyed */
}


//
// Initialize the file list
//
static void init_mfile_list (void)
{
  mfile_list = mfile_Create ();
}

//
// Walk the list, destroying each node
//
static void destroy_mfile_list (void)
{
  Mfile *rf, *next;

  rf = mfile_list;
  next = rf->next;

  while (next)
  {
    const char *filename = Mat_GetFilename(rf->mat);
    mfile_Destroy (filename);
    rf = next;
    next = rf->next;
  }
}

//
// Walk the list, look for a particular node
//
static Mfile * mfile_find (char *name)
{
  Mfile *rf, *next;

  if (!mfile_list)
  {
    init_mfile_list();
    return (0);
  }

  rf = mfile_list;
  next = rf->next;

  while (next)
  {
    const char *filename = Mat_GetFilename(rf->mat);
    if (!strcmp (filename, name))
    {
      return (rf);
    }

    rf = next;
    next = rf->next;
  }

  return (0);
}

#undef  THIS_FUNCTION
#define THIS_FUNCTION "get_mfile_ds"
static Mfile * get_mfile_ds (char * name, int mmode, char *header)
{
  Mfile *rf=0;

  // does rfile with 'name' exists that is of desired type?
  rf = mfile_find (name);
  if (rf)
  {
    if (rf->mode == mmode)
    {
      return (rf);
    }
    fprintf(stderr, THIS_FUNCTION
        ": File %s is already opened. Conflicting file access mode: " );
    rerror("Cannot continue!\n" );
  }

  if ((mmode != MAT_ACC_RDONLY) && (mmode != MAT_ACC_RDWR))
  {
    fprintf(stderr, THIS_FUNCTION ": Error: File '%s' unsupported mode %i\n", name, mmode);
    return (0);
  }

  mat_t * mat;
  if (mmode == MAT_ACC_RDONLY)
  {
    mat = Mat_Open(name, MAT_ACC_RDONLY);
  }
  else
  {
    header = (char *) rlab_matio_default_header;
    mat = Mat_CreateVer(name, header,
                        (enum mat_ft)((mat_file_ver|mmode) & 0xfffffffe));
  }

  if (!mat)
  {
    fprintf(stderr, "Error: Failed to open file '%s' for mode %i\n", name, mmode);
    return (0);
  }

  // Make sure we don't try and open more files than the system will allow
  //   fprintf(stderr, "nrfile = %i, %s\n", nrfile, name);

  if (nrfile >= FOPEN_MAX - 3)
    rerror ("get_rfile_ds: Cannot open a new file. Too many files already opened");

  //
  // create new rfile and do all the necessary actions to be of specified type
  //
  rf = mfile_Create();
  rf->mat = mat;
  rf->mode = mmode;
  return rf;
}

/* Close the file asociated with the file name */
static int close_mfile_ds (char *name)
{
  /* Check list */
  if (isvalidstring(name) < 1 )
    return 1;

  return (mfile_Destroy (name));
}


static const char *mxclass[18] = {
  "mxUNKNOWN_CLASS",
  "mxCELL_CLASS",
  "mxSTRUCT_CLASS",
  "mxOBJECT_CLASS",
  "mxCHAR_CLASS",
  "mxSPARSE_CLASS",
  "mxDOUBLE_CLASS",
  "mxSINGLE_CLASS",
  "mxINT8_CLASS",
  "mxUINT8_CLASS",
  "mxINT16_CLASS",
  "mxUINT16_CLASS",
  "mxINT32_CLASS",
  "mxUINT32_CLASS",
  "mxINT64_CLASS",
  "mxUINT64_CLASS",
  "mxFUNCTION_CLASS",
  "mxOPAQUE_CLASS"
};

static int rlab_matio_is_double(enum matio_types type)
{
  int rval=0;
  switch ( type )
  {
    case MAT_T_DOUBLE:
    case MAT_T_SINGLE:
      rval = 1;
      break;

    default:
      break;
  }
  return rval;
}


static double rlab_matio_get_double(enum matio_types type, void *data)
{
  double rval=0/0;
  switch ( type )
  {
    case MAT_T_DOUBLE:
      rval = *(double *)data;
      break;
    case MAT_T_SINGLE:
      rval = *(float *)data;
      break;
    default:
      break;
  }
  return rval;
}

static int rlab_matio_get_int(enum matio_types type, void *data)
{
  int rval=0/0;
  switch ( type )
  {
    case MAT_T_INT64:
      rval = (long long)(*(mat_int64_t *)data);
      break;
    case MAT_T_UINT64:
      rval = (unsigned long long)(*(mat_uint64_t *)data);
      break;
    case MAT_T_INT32:
      rval = *(mat_int32_t *) data;
      break;
    case MAT_T_UINT32:
      rval = *(mat_uint32_t *) data;
      break;
    case MAT_T_INT16:
      rval =  *(mat_int16_t *) data;
      break;
    case MAT_T_UINT16:
      rval = *(mat_int16_t *) data;
      break;
    case MAT_T_INT8:
      rval = *(mat_int8_t *) data;
      break;
    case MAT_T_UINT8:
      rval = *(mat_uint8_t *) data;
      break;
    default:
      break;
  }
  return rval;
}

static int rlab_matio_is_int(enum matio_types type)
{
  int rval=0;
  switch ( type )
  {
    case MAT_T_INT64:
    case MAT_T_UINT64:
    case MAT_T_INT32:
    case MAT_T_UINT32:
    case MAT_T_INT16:
    case MAT_T_UINT16:
    case MAT_T_INT8:
    case MAT_T_UINT8:
      rval = 1;
      break;

    default:
      break;
  }
  return rval;
}

static MD * rlab_matio_get_number_matrix(matvar_t *matvar)
{
  MD *w=0;
  size_t i, j, stride, idx, len;
  int nr, nc;

  if ( !matvar->data )
  {
    return NULL;
  }

  // figure out the size of the array
  len = matvar->dims[0];
  nr  = len;
  if ( matvar->rank == 2 )
  {
    len *= matvar->dims[1];
    nc   = matvar->dims[1];
  }
  if ( matvar->rank == 3 )
  {
    len *= matvar->dims[2];
    nr   = 1;
    nc   = len;
  }

  stride = Mat_SizeOf(matvar->data_type);
  if ( matvar->isComplex )
  {
    mat_complex_split_t *complex_data = (mat_complex_split_t *)matvar->data;
    unsigned char *rp = (unsigned char *)complex_data->Re;
    unsigned char *ip = (unsigned char *)complex_data->Im;
    w = mdc_Create(nr, nc);
    if (rlab_matio_is_double(matvar->data_type))
    {
      for (i=0; i<len; i++)
      {
        MdcV0r(w,i) = rlab_matio_get_double(matvar->data_type, rp + i * stride);
        MdcV0i(w,i) = rlab_matio_get_double(matvar->data_type, ip + i * stride);
      }
    }
    else if (rlab_matio_is_int(matvar->data_type))
    {
      for (i=0; i<len; i++)
      {
        MdcV0r(w,i) = rlab_matio_get_int(matvar->data_type, rp + i * stride);
        MdcV0i(w,i) = rlab_matio_get_int(matvar->data_type, ip + i * stride);
      }
    }
  }
  else
  {
    unsigned char *data = (unsigned char *) matvar->data;
    if (rlab_matio_is_double(matvar->data_type))
    {
      w = mdr_Create(nr, nc);
      for (i=0; i<len; i++)
      {
        MdrV0(w,i) = rlab_matio_get_double(matvar->data_type, data + i*stride);
      }
    }
    else if (rlab_matio_is_int(matvar->data_type))
    {
      w = mdi_Create(nr, nc);
      for ( i=0; i<nr; i++ )
      {
        MdiV0(w,i) = rlab_matio_get_int(matvar->data_type, data + i * stride);
      }
    }
  }

  return (MD *) w;
}

static MD * rlab_matio_get_char_matrix(matvar_t *matvar)
{
  MD *w=0;
  size_t i, j, stride, idx, len;
  int nr, nc;

  if ( !matvar->data )
  {
    return NULL;
  }

  // figure out the size of the array
  nr = matvar->dims[0];
  stride = Mat_SizeOf(matvar->data_type);
  if ( matvar->rank == 2 )
  {
    stride *= matvar->dims[1];
  }
  if ( matvar->rank == 3 )
  {
    stride *= matvar->dims[2];
  }

  w = mds_Create(nr, 1);
  char * str_4_copy=0;
  unsigned char *data = (unsigned char *) matvar->data;
  for (i=0; i<nr; i++)
  {
    str_4_copy = GC_malloc((stride+1)*sizeof(unsigned char));
    for (j=0;j<stride;j++)
    {
      str_4_copy[j] = data[i+nr*j];
    }
    str_4_copy[stride] = 0;
    MdsV0(w,i) = str_4_copy;
  }

  return (MD *) w;
}

static void follow_struct_cell(matvar_t *matvar, void *parent_ptr, int bclass, int idx)
{
  MD *w=0;
  if ( NULL == matvar )
  {
    return;
  }

  if ( matvar->rank == 0 )
  {
    return;
  }

  if (matvar->data == 0)
  {
    return;
  }

  Ent *e=0;
  Btree *bnew=0;
  ListNode *node=0;
  MD *mdenew=0;
  int nfields, i, j, len, nr, nc;
  size_t nmemb, ncells;

 //printf("follow_struct_cell: In\n");

  switch ( matvar->class_type )
  {
    case MAT_C_DOUBLE:
    case MAT_C_SINGLE:
    case MAT_C_INT64:
    case MAT_C_UINT64:
    case MAT_C_INT32:
    case MAT_C_UINT32:
    case MAT_C_INT16:
    case MAT_C_UINT16:
    case MAT_C_INT8:
     //printf("MD num\n");
      w = rlab_matio_get_number_matrix(matvar);
      e = ent_Assign_Rlab_MD (w);
      if (bclass == BTREE)
      {
        node = btree_FindNode ((Btree *)parent_ptr, matvar->name);
        if (node)
        {
          Ent *E = var_ent(node->ent);
          ent_Destroy (E);
          listNode_AttachEnt(node, e);
          ent_IncRef (e);
        }
        else
          install((Btree *)parent_ptr, matvar->name, e);
      }
      else if (bclass == MATRIX_DENSE_ENTITY)
      {
        Ent *E = MdeV0((MDE*)parent_ptr, idx);
        if (E)
        {
          ent_Destroy (E);
        }
        MdeV0((MDE*)parent_ptr, idx) = e;
        ent_IncRef (e);
      }
      break;

    case MAT_C_CHAR:
     //printf("MD str\n");
      w = rlab_matio_get_char_matrix(matvar);
      e = ent_Assign_Rlab_MD (w);
      if (bclass == BTREE)
      {
        node = btree_FindNode ((Btree *)parent_ptr, matvar->name);
        if (node)
        {
          Ent *E = var_ent(node->ent);
          ent_Destroy (E);
          listNode_AttachEnt(node, e);
          ent_IncRef (e);
        }
        else
          install((Btree *)parent_ptr, matvar->name, e);
      }
      else if (bclass == MATRIX_DENSE_ENTITY)
      {
        Ent *E = MdeV0((MDE*)parent_ptr, idx);
        if (E)
        {
          ent_Destroy (E);
        }
        MdeV0((MDE*)parent_ptr, idx) = e;
        ent_IncRef (e);
      }
      break;

    case MAT_C_STRUCT:
      //printf("name: %p, %s\n", matvar->name, matvar->name);
      if (bclass == BTREE)
      {
        //printf("parent is btree\n");
        node = btree_FindNode ((Btree *)parent_ptr, matvar->name);
        if (!node)
        {
          bnew = btree_Create();
          install((Btree *)parent_ptr, matvar->name, ent_Assign_Rlab_BTREE (bnew));
        }
        else
        {
          bnew = ent_data(var_ent (node));
        }
       //printf("a\n");
        nfields = Mat_VarGetNumberOfFields(matvar);
       //printf("b\n");
        nmemb = matvar->dims[0];
       //printf("c\n");
        for ( i = 1; i < matvar->rank; i++ )
        {
          nmemb *= matvar->dims[i];
        }
       //printf("d\n");
        if ( nfields > 0 && nmemb > 0 )
        {
          matvar_t **fields = (matvar_t **)matvar->data;
          if ( NULL != fields )
          {
            for ( j = 0; j < nfields * nmemb; j++ )
              follow_struct_cell(fields[j],bnew,BTREE,0);
          }
        }
       //printf("e\n");
      }
      else if (bclass == MATRIX_DENSE_ENTITY)
      {
       //printf("parent is cell\n");
        bnew = btree_Create();
        e = ent_Assign_Rlab_BTREE (bnew);
       //printf("a: %p, %i: e=%p\n", parent_ptr, idx, e);
        MdeV0((MDE*)parent_ptr, idx) = e;
       //printf("b\n");
        nfields = Mat_VarGetNumberOfFields(matvar);
       //printf("c\n");
        nmemb = matvar->dims[0];
        for ( i = 1; i < matvar->rank; i++ )
        {
          nmemb *= matvar->dims[i];
        }
       //printf("d\n");
        if ( nfields > 0 && nmemb > 0 )
        {
          matvar_t **fields = (matvar_t **)matvar->data;
          if ( NULL != fields )
          {
            for ( j = 0; j < nfields * nmemb; j++ )
              follow_struct_cell(fields[j],bnew,BTREE,0);
          }
        }
       //printf("e\n");
      }
      break;

    case MAT_C_CELL:
     //printf("cell\n");
      len = matvar->dims[0];
      nr  = len;
      if ( matvar->rank == 2 )
      {
        len *= matvar->dims[1];
        nc   = matvar->dims[1];
      }
      if ( matvar->rank == 3 )
      {
        len *= matvar->dims[2];
        nr   = 1;
        nc   = len;
      }
      if (bclass == BTREE)
      {
        node = btree_FindNode ((Btree *)parent_ptr, matvar->name);
        // check the type of node:
        //printf("cell/btree: matvar name=%s: node=%p\n", matvar->name, node);

        if (!node)
        {
          // node does not exist: create it and move on
          mdenew =  mde_Create (nr, nc);
          install((Btree *)parent_ptr, matvar->name, ent_Assign_Rlab_MDE(mdenew));
        }
        else
        {
          // node already exists: It has an entity even if UNDEF attached to it
          Ent *exx = var_ent(node);
          if (ent_data(exx))
          {
            // node exists and has data attached to it
            mdenew = ent_data(exx);
          }
          else
          {
            // node exists and but no data is attached to it (UNDEF)
            if (exx->refc > 1)
            {
              ent_Clean (exx);
            }
            mdenew =  mde_Create (nr, nc);
            Ent *enew = ent_Assign_Rlab_MDE(mdenew);
            ent_IncRef(enew);
            listNode_AttachEnt   (node, enew);
          }
        }
        //printf("cell/btree: mdenew=%p\n", mdenew);
        matvar_t **cell = (matvar_t **)matvar->data;
        for (j=0; j<len; j++ )
        {
          follow_struct_cell(cell[j],mdenew,MATRIX_DENSE_ENTITY,j);
        }
      }
      else if (bclass == MATRIX_DENSE_ENTITY)
      {
        mdenew = mde_Create (nr, nc);
       //printf("cell/mde: mdenew=%p\n", mdenew);
        MdeV0((MDE*)parent_ptr, idx) = ent_Assign_Rlab_MDE(mdenew);
        matvar_t **cell = (matvar_t **)matvar->data;
        for (j=0; j<len; j++ )
          follow_struct_cell(cell[j], mdenew, MATRIX_DENSE_ENTITY, j);
      }
      break;
  }

 //printf("follow_struct_cell: Exit\n");
}

//  load data from matlab file 'fn' into a variable 'x' if given.
//  if 'x' is omitted then load all variables from 'fn' into symbol table.
//
#undef  THIS_SOLVER
#define THIS_SOLVER "matread"
Ent * ent_matio_read (int nargs, Datum args[])
{
  Ent *e1=0, *e2=0, *rent=0;
  char *filename=0;
  int i, type;
  int close_after_rw=0;
  Mfile *rf=0;
  FILE *rlab_stderr = (!RLAB_STDERR_DS) ? stderr : RLAB_STDERR_DS;
  Datum arg2;
  size_t stat;
  Btree *bnew=0, *symtab=0;
  ListNode *node=0;
  MD *w=0;
  void *m=0;
  double rval=RLAB_STATUS_FAILURE;

  /* Check n_args */
  if (nargs > 2 || nargs == 0)
    rerror (THIS_SOLVER ": One or two arguments required");

  /* get file name from argument list, always the 1st argument */
  RLABCODE_PROCESS_ARG1_S(THIS_SOLVER,0,e1,filename,1);

  // once we have a name,we can see if the url has been opened before
  rf = mfile_find(filename);
  if (!rf)
  {
    // now open it
    rf = get_mfile_ds (filename, MAT_ACC_RDONLY, NULL);
    if (!rf)
    {
      goto _exit;
    }
    close_after_rw = 1;
  }

 //printf("1\n");

  /* if second argument is given, this is a variable which will receive
     the content of the file, otherwise the content is written to $$
    */
  if (nargs == 1)
  {
    /* Use the global symbol table */
   symtab = get_symtab_ptr ();
   //printf("2\n");
  }
  else if (nargs > 1)
  {
    /* Use a user-supplied list */
    Datum arg2 = args[1];

    /* Check to see if variable exists, etc... */
    if (arg2.type != VAR)
      rerror (THIS_SOLVER ": 2nd argument must be a variable");

    // If the specified variable is already a list, then just
    // create (or overwrite) members. If it is not a list, then
    // destroy it, and create a new list.
    e2 = var_ent (arg2.u.ptr);
    if (ent_type (e2) == BTREE)
    {
      /* Create a new member. */
      symtab = ent_data (e2);
    }
    else
    {
      /* Destroy it, and create a list. */
      ent_Destroy (e2);

      e2 = ent_Create ();
      symtab = btree_Create ();
      ent_data (e2) = symtab;
      ent_SetType (e2, BTREE);
      ent_IncRef (e2);
      listNode_AttachEnt (arg2.u.ptr, e2);
    }
  }

 //printf("3\n");


  //mat_t *mat = Mat_Open(filename, MAT_ACC_RDONLY);

  if (rf->mat == NULL)
  {
    Mat_Critical(THIS_SOLVER ": Error opening file '%s'\n", filename);
    goto _exit;
  }

 //printf("4\n");
  char *address=0;
  matvar_t *matvar;
  while ( (matvar = Mat_VarReadNext(rf->mat)) != NULL )
  {
    switch ( matvar->class_type )
    {
      case MAT_C_DOUBLE:
      case MAT_C_SINGLE:
      case MAT_C_INT64:
      case MAT_C_UINT64:
      case MAT_C_INT32:
      case MAT_C_UINT32:
      case MAT_C_INT16:
      case MAT_C_UINT16:
      case MAT_C_INT8:
      case MAT_C_UINT8:
        w = rlab_matio_get_number_matrix(matvar);
        if (w)
        {
          node = btree_FindNode (symtab, matvar->name);
          if (node)
          {
            Ent *E = var_ent(node->ent);
            ent_Destroy (E);
            E = ent_Assign_Rlab_MD (w);
            listNode_AttachEnt(node, E);
            ent_IncRef (E);
          }
          else
            install(symtab, matvar->name, ent_Assign_Rlab_MD (w));
        }
        break;

      case MAT_C_CHAR:
        w = rlab_matio_get_char_matrix(matvar);
        if (w)
        {
          node = btree_FindNode (symtab, matvar->name);
          if (node)
          {
            Ent *E = var_ent(node->ent);
            ent_Destroy (E);
            E = ent_Assign_Rlab_MD (w);
            listNode_AttachEnt(node, E);
            ent_IncRef (E);
          }
          else
            install(symtab, matvar->name, ent_Assign_Rlab_MD (w));
        }
        break;

      case MAT_C_STRUCT:
      case MAT_C_CELL:
       //printf("4-1\n");
        follow_struct_cell(matvar, symtab, BTREE, 0);
        break;

      default:
        fprintf(stdout, "Matlab variable type %s not yet supported\n",
               mxclass[matvar->class_type]);
        break;
    }
    Mat_VarFree(matvar);
  }

 //printf("5\n");

  rval = RLAB_STATUS_SUCCESS;

_exit:

  if (close_after_rw)
  {
    mfile_Destroy(filename);
  }

  // clean-up
  ent_Clean (e1);
  ent_Clean (e2);

  //
  // write result
  //
  return ent_Create_Rlab_Double(rval);
}

#undef  THIS_SOLVER
#define THIS_SOLVER "matopen"
Ent * ent_matio_open (int nargs, Datum args[])
{
  ListNode *node;

  char *mode=0, *name=0, *header=0;
  int mmode = -1, i=0;
  Ent *e1=0, *e2=0, *e3=0, *rent=0;
  Mfile *rf = 0;

  int rval = RLAB_STATUS_FAILURE;

  if (nargs < 1)
    rerror (THIS_SOLVER ": requires at least 1 argument");

  // we force user to use URLs for the file names:
  // 1. filename
  // 2. serial port (e.g., serial:///dev/ttyS0)
  // 3. URL         (e.g., http://www.suse.de)
  // 4. tcp socket  (e.g., tcp://127.0.0.1:5555)
  // 5. hierarhical data format ver. 5 (e.g., h5://filename)
  // process filename
  RLABCODE_PROCESS_ARG1_S(THIS_SOLVER,0,e1,name,1);

  // once we have a name,we can see if the url has been opened before
  rf = mfile_find(name);

  if (rf)
    goto _exit;

  //
  // get file mode
  //
  if (nargs < 2)
    rerror(THIS_SOLVER ": Second argument 'file access mode' required for regular files");

  e2 = bltin_get_ent (args[1]);
  mode = class_char_pointer (e2);
  if (isvalidstring(mode)<1)
    rerror(THIS_SOLVER ": unknown file access mode");
  if (  mode[0]!='r' && mode[0]!='R' &&  mode[0]!='w' && mode[0]!='W'
      )
    rerror (THIS_SOLVER ": Unknown file access mode. Supported are (r)ead and (w)rite!\n");

  if (mode[0] == 'w' || mode[0] == 'W' )
  {
    //printf("mmode=%i\n", MAT_ACC_RDWR);
    mmode = MAT_ACC_RDWR;
  }
  else if (mode[0] == 'r' || mode[0] == 'R')
  {
    mmode = MAT_ACC_RDONLY;
  }

  if (mmode == -1)
    goto _exit;

  //
  // get options:
  //    header - string
  //    version - mat file format version 4,5,73
  if (nargs == 3)
  {
    e3 = bltin_get_ent (args[2]);
    if (ent_type (e3) == BTREE)
    {
      // buffer_size
      node = btree_FindNode (ent_data (e3), "version");
      if (node != 0)
      {
        if (ent_type(var_ent(node)) == MATRIX_DENSE_REAL)
        {
          i = (int) class_double (var_ent(node));
          if (i < 0)
            i = 0;
        }
      }

      // header
      node = btree_FindNode (ent_data (e3), "header");
      if (node != 0)
      {
        if (ent_type(var_ent(node)) == MATRIX_DENSE_STRING)
          header = class_char_pointer (var_ent(node));
      }
    }
  }

  switch (i)
  {
    case 4:
      mat_file_ver = MAT_FT_MAT4;
      break;

    case 5:
      mat_file_ver = MAT_FT_MAT5;
      break;

    default:
      mat_file_ver = MAT_FT_MAT73;
      break;
  }

  // now open it
  rf = get_mfile_ds (name, mmode, header);
  if (rf)
    rval = RLAB_STATUS_SUCCESS;

_exit:

  // clean up
  ent_Clean (e1);
  ent_Clean (e2);

  rent = ent_Create ();
  ent_data (rent) = mdr_CreateScalar (rval);
  ent_type(rent) = MATRIX_DENSE_REAL;
  return (rent);
}

//
// close rfile when needed
//
#undef  THIS_SOLVER
#define THIS_SOLVER "matclose"
Ent * ent_matio_close (int nargs, Datum args[])
{
  char *fname=0;
  Ent *e1=0, *rent=0;
  Mfile *rf=0;

  int rval = RLAB_STATUS_FAILURE;

  if (nargs != 1)
    rerror (THIS_SOLVER ": 1 argument allowed");

  // process filename
  RLABCODE_PROCESS_ARG1_S(THIS_SOLVER,0,e1,fname,1);

  // now check the name and decide how to close it
  rf = mfile_find (fname);
  if (rf)
  {
    rval = close_mfile_ds (fname);
  }

  // clean-up
  ent_Clean (e1);

  rent = ent_Create ();
  ent_data(rent) = mdi_CreateScalar(rval);
  ent_SetType (rent, MATRIX_DENSE_REAL);
  return (rent);
}

#undef  THIS_FUNCTION
#define THIS_FUNCTION "matio_write_rlab_atomic_md"
static int matio_write_rlab_atomic_md (Mfile *rf, char * var_name, MD * data)
{
  int size=SIZE(data), len, stride=-1;

  if (size < 1)
  {
    goto _exit;
  }

  int i,j,k,rval=1;
  size_t dims[2]={MNR(data),MNC(data)};
  matvar_t *matvar=0;

  switch (data->type)
  {
    case RLAB_TYPE_DOUBLE:
      matvar = Mat_VarCreate(var_name, MAT_C_DOUBLE, MAT_T_DOUBLE, 2, dims, (double *) MDRPTR(data), 0);
      break;

    case RLAB_TYPE_INT32:
      matvar = Mat_VarCreate(var_name, MAT_C_INT32, MAT_T_INT32, 2, dims, (int *) MDRPTR(data), 0);
      rval=0;
      break;

    case RLAB_TYPE_COMPLEX:
      matvar = Mat_VarCreate(var_name, MAT_C_DOUBLE, MAT_T_DOUBLE, 2, dims, (Complex *) MDRPTR(data),
                             MAT_F_COMPLEX);
      rval=0;
      break;

    case RLAB_TYPE_STRING:
      if (size < 1)
      {
        break;
      }
      for (i=0;i<size;i++)
      {
        len = isvalidstring(MdsV0(data,i));
        if (len>stride)
        {
          stride = len;
        }
      }
      if (!stride)
      {
        fprintf(stderr, THIS_FUNCTION ": Refusing to write empty string matrix to MAT FILE!\n");
        goto _exit;
      }
      char *char_data_ptr = GC_malloc(stride * size);
      for (i=0;i<MNR(data);i++)
      {
        for (j=0;j<MNC(data);j++)
        {
          for (k=0; k<isvalidstring(Mds0(data,i,j)); k++)
          {
            char *temp_char=Mds0(data,i,j);
            char_data_ptr[i*dims[0] + j + k] = temp_char[k];
          }
          for (   ; k<stride; k++)
          {
            char_data_ptr[i*dims[0] + j + k] = 0;
          }
        }
      }
      GC_free(char_data_ptr);
      rval = 0;
      break;

    default:
      break;
  }

  if (matvar)
  {
    rval = Mat_VarWrite(rf->mat, matvar, compression);
    Mat_VarFree(matvar);
  }

_exit:

  return rval;
}

#undef  THIS_SOLVER
#define THIS_SOLVER "matwritem"
Ent * ent_matio_writem (int nargs, Datum args[])
{
  ListNode *node=0;
  char *name, *var_name=0;
  int type;
  Mfile *rf;
  Ent *e1=0, *e2=0, *e3=0, *rent;
  int close_after_rw=0, rval=RLAB_STATUS_FAILURE;

  /* Check n_args */
  if (nargs < 2)
    rerror (THIS_SOLVER ": at least 2 arguments required");

  //
  // get the name of the stream
  //
  RLABCODE_PROCESS_ARG1_S(THIS_SOLVER,0,e1,name,1);

  // was the file previously accessed?
  rf = mfile_find(name);
  if (rf)
  {
    if (rf->mode == MAT_ACC_RDONLY)
    {
      fprintf(stderr, THIS_SOLVER
          ": Conflicting file type: Trying to write to file with READ access: Cannot continue!\n");
      goto _exit;
    }
  }
  if (!rf)
  {
    close_after_rw = 1;
    rf = get_mfile_ds (name, MAT_ACC_RDWR, NULL);
  }

  if (!rf)
  {
    fprintf (stderr, THIS_SOLVER ": Trying to write to matfile that is not accessible\n");
    rerror ("Cannot write to matfile");
  }

  //
  // now check the second argument: this is atomic content to be saved
  //
  e2 = bltin_get_ent (args[1]);
  type = ent_type (e2);
  if (type == UNDEF)
  {
    fprintf(stderr, THIS_SOLVER ": second argument 'data' is undefined: Nothing to do\n");
  }

  // check if the name under which the content will be written is provided as third entry
  // otherwise use its name
  if (nargs == 3)
  {
    e3 = bltin_get_ent (args[2]);
    if (ent_type(e3) == MATRIX_DENSE_STRING)
    {
      char *dummy = class_char_pointer(e3);
      if (isvalidstring(dummy)>1)
      {
        var_name = dummy;
      }
    }
  }
  else
  {
    if (args[1].type == VAR)
      var_name = var_key (args[1].u.ptr);
  }

  if (isvalidstring(var_name)<1)
  {
    fprintf (stderr, THIS_SOLVER ": Second argument is not variable. Cannot continue\n");
  }


  switch( type )
  {
    case MATRIX_DENSE_REAL:
    case MATRIX_DENSE_COMPLEX:
    case MATRIX_DENSE_STRING:
      matio_write_rlab_atomic_md (rf, var_name, ent_data(e2));
      break;

//
//     case MATRIX_SPARSE_REAL:
//       h5_write_rlab_atomic_msr (rf, obj_name, ent_data(e2));
//       break;
//
//     case MATRIX_SPARSE_COMPLEX:
//       h5_write_rlab_atomic_msc (rf, obj_name, ent_data(e2));
//       break;

    default:
      fprintf(stderr, THIS_SOLVER ": This data type cannot be saved (are you trying to save a list "
          "or a cell?)");
  }

  if (close_after_rw)
    mfile_Destroy(name);

_exit:

  // rlab stuff:
  ent_Clean (e1);
  ent_Clean (e2);

  rent = ent_Create();
  ent_data(rent) = mdr_CreateScalar(rval);
  ent_type(rent) = MATRIX_DENSE_REAL;
  return (rent);
}




//
// rcfitsio.c:  rlab's interface to CFITSIO library
// Marijan Kostrun, VI-2024
//
// This file is a part of RLaB + rlabplus
// Copyright (C) 2024  Marijan Kostrun
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
// See the file ../COPYING

#include "r_cfitsio.h"
#define THIS_LIBRARY "cfitsio"

static fits_rfile *fits_rfile_list = 0;   // Ptr to 1st Rfile
static int nrfile = 0;          // Number of open files

fits_rfile * create_fits_rfile (void)
{
  fits_rfile *new = (fits_rfile *) GC_MALLOC (sizeof (fits_rfile));
  if (new == 0)
    rerror ("out of memory");

  // zero the file structure
  new->name   = 0;
  new->fptr = 0;

  // finish
  new->next = fits_rfile_list;
  nrfile++;
  fits_rfile_list = new;
  return (new);
}

int destroy_fits_rfile (char *name)
{
  int status;

  fits_rfile *rf, *next, *prev;

  rf = fits_rfile_list;
  next = rf->next;
  prev = rf;

  while (next)
  {
    if (!strcmp (rf->name, name))
    {
      // Found it!

      // Re-Hook up list
      prev->next = next;

      // Check two special cases
      if (prev == rf)     // 1st element
        fits_rfile_list = next;
      else if (next == 0) // Last element
        prev->next = 0;

      // Close the file
      fits_close_file(rf->fptr, &status);

      // clean up all allocated data/pointers
      if (rf->name)
      {
        GC_FREE (rf->name);
        rf->name = 0;
      }
      rf->next = 0;
      rf->fptr = 0;
      GC_FREE (rf);
      rf = 0;

      nrfile--;
      return (0);   /* Success: an fitsio file was closed and its memory structure destroyed */
    }

    prev = rf;
    rf = next;
    next = rf->next;
  }

  return (1);     /* Failure: no fitsio file was found, so none was destroyed */
}

//
// Initialize the file list
//
void init_fits_rfile_list (void)
{
  fits_rfile_list = create_fits_rfile();
}

//
// Walk the list, destroying each node
//
void destroy_fits_rfile_list (void)
{
  fits_rfile *rf, *next;

  rf = fits_rfile_list;
  next = rf->next;

  while (next)
  {
    destroy_fits_rfile (rf->name);
    rf = next;
    next = rf->next;
  }
}

//
// Walk the list, look for a particular node
//
fits_rfile * find_fits_rfile (char *name)
{
  fits_rfile *rf, *next;

  rf = fits_rfile_list;
  if (rf)
  {
    next = rf->next;
    while (next)
    {
      if (!strcmp (rf->name, name))
      {
        return (rf);
      }

      rf = next;
      next = rf->next;
    }
  }

  return (0);
}

//
// Get the file/pipe descriptor asociated with the char string. This function does
// all the work, searches the list, uses popen() instead of fopen when necessary.
// It is used by other file manipulating codes that do not need full access to Rfiles.
//
fitsfile * get_fits_rfile_fptr (char *name)
{
  fits_rfile *rf, *next;

  rf = fits_rfile_list;
  next = rf->next;

  while (next)
  {
    if (!strcmp (rf->name, name))
    {
      return (rf->fptr);
    }
    rf = next;
    next = rf->next;
  }

  return (0);
}

//
// get rfile pointer with name and check if it is of proper type.
// if it does not exist, or the filetype is RFILE_NULL create a
// new object of desired type
//
fits_rfile * get_fits_rfile_ds (char * name, int mode)
{
  int status=0;
  fits_rfile *rf=0;
  fitsfile *fptr=0;         /* FITS file pointer, defined in fitsio.h */

  // does rfile with 'name' exists that is of desired type?
  rf = find_fits_rfile (name);
  if (rf)
  {
    return (rf);
  }

  // Make sure we don't try and open more files than the system will allow
  if (nrfile >= CFITSIO_MAX_FOPEN)
    rerror ("get_fits_rfile_ds: Cannot open a new file. Too many files already opened");

  if (fits_open_file(&fptr, name, mode, &status) == 0)
  {
    //
    // create new rfile and do all the necessary actions to be of specified type
    //
    rf = create_fits_rfile();
    rf->name = cpstr(name);
    rf->mode = mode;
    rf->fptr = fptr;
    return rf;
  }

  if (status)
    fits_report_error(stderr, status); /* print any error message */

  return (0);
}



#undef  THIS_SOLVER
#define THIS_SOLVER RLAB_NAME_FITSIO ".open"
//
// open fitsio file using library
//
Ent *
ent_cfitsio_open(int nargs, Datum args[])
{
  Ent *e1=0, *e2=0, *rent;
  char *name=0, *smode=0;
  int mode, rval=0;
  fits_rfile * rf=0;

  if (nargs > 2)
  {
    fprintf (stdout,
             THIS_SOLVER ": cfitsio wrapper library for RLaB\n");
    fprintf (stdout,
             THIS_SOLVER ": open an FITS file\n");
    fprintf (stdout,
             THIS_SOLVER ": Format:\n");
    fprintf (stdout,
             THIS_SOLVER ":   " THIS_SOLVER "(filename,access),\n");
    fprintf (stdout,
             THIS_SOLVER ": where\n");
    fprintf (stdout,
             THIS_SOLVER ":   'filename' is the fits file, and 'access' a string describing the access\n");
    fprintf (stdout,
             THIS_SOLVER ": See manual for details.\n");
    rerror ( THIS_SOLVER ": requires 2 arguments");
  }

  // we force user to use URLs for the file names:
  // process filename
  RLABCODE_PROCESS_ARG1_S(THIS_SOLVER,0,e1,name,1);

  //
  //    open(filename, mode)
  //
  if (nargs < 2)
    rerror(THIS_SOLVER ": Second argument 'access' is required for regular files");

  RLABCODE_PROCESS_ARG1_S(THIS_SOLVER,0,e2,smode,1);
  if (!smode)
  {
    rerror(THIS_SOLVER ": Second argument 'file access mode' required for regular files");
  }

  if (smode[0]=='a' || smode[0]=='A')
  {
    mode = READWRITE;
  }
  else if (smode[0]=='r' || smode[0]=='R')
  {
    mode = READONLY;
  }
  else if (smode[0]=='w' || smode[0]=='W')
  {
    mode = READWRITE;
  }
  else
  {
    rerror (THIS_SOLVER ": Unknown file access mode. Supported are 'a', 'w', and 'r'");
  }

  // once we have a name,we can see if the url has been opened before
  rf = get_fits_rfile_ds (name, mode);
  if (!rf)
  {
    rval = -1; // operation failed
    goto Open_clean_up;
  }

Open_clean_up:


  // clean up rlab
  ent_Clean(e1);
  ent_Clean(e2);
  return ent_Create_Rlab_Int (rval);
}


#undef  THIS_SOLVER
#define THIS_SOLVER RLAB_NAME_FITSIO ".close"
//
// open fitsio file using library
//
Ent * ent_cfitsio_close (int nargs, Datum args[])
{
  Ent *e1=0;
  char *name=0;
  int rval=0;
  fits_rfile * rf=0;

  if (nargs > 1)
  {
    fprintf (stdout,
             THIS_SOLVER ": cfitsio wrapper library for RLaB\n");
    fprintf (stdout,
             THIS_SOLVER ": open an FITS file\n");
    fprintf (stdout,
             THIS_SOLVER ": Format:\n");
    fprintf (stdout,
             THIS_SOLVER ":   " THIS_SOLVER "(filename,access),\n");
    fprintf (stdout,
             THIS_SOLVER ": where\n");
    fprintf (stdout,
             THIS_SOLVER ":   'filename' is the fits file, and 'access' a string describing the access\n");
    fprintf (stdout,
             THIS_SOLVER ": See manual for details.\n");
    rerror ( THIS_SOLVER ": requires 2 arguments");
  }

  // we force user to use URLs for the file names:
  // process filename
  //
  //    close(filename)
  //
  RLABCODE_PROCESS_ARG1_S(THIS_SOLVER,0,e1,name,1);

  // once we have a name,we can see if the url has been opened before
  rf = find_fits_rfile (name);
  if (!rf)
  {
    rval = -1; // operation failed: file does not exist in the rfile list, so nothing to close
    goto _exit;
  }

  fits_close_file(rf->fptr, &rval);

  destroy_fits_rfile (name);

_exit:

  // clean up rlab
  ent_Clean(e1);
  return ent_Create_Rlab_Int (rval);
}

#undef  THIS_SOLVER
#define THIS_SOLVER RLAB_NAME_FITSIO ".readheader"
//
// open fitsio file using library
//
Ent *
ent_cfitsio_readheader (int nargs, Datum args[])
{
  Ent *e1=0, *e2=0;
  char *name=0, *smode=0;
  int mode=READONLY, i, m;
  int ihdu=0, status=0, nkeys=0, keypos=0, hdutype=0, key=0;
  fits_rfile * rf=0;

  char card[FLEN_CARD];
  char s_ihdu[FLEN_CARD];

  MDS *kvc=0;
  MDR *idx_hdu=NULL;
  Btree *rval=btree_Create();

  if ((nargs<1) || (nargs > 3))
  {
    fprintf (stdout,
             THIS_SOLVER ": cfitsio wrapper library for RLaB\n");
    fprintf (stdout,
             THIS_SOLVER ": read headers from an FITS file\n");
    fprintf (stdout,
             THIS_SOLVER ": Format:\n");
    fprintf (stdout,
             THIS_SOLVER ":   " THIS_SOLVER "(filename,/hdu,/key//),\n");
    fprintf (stdout,
             THIS_SOLVER ": where\n");
    fprintf (stdout,
             THIS_SOLVER ":   'filename' is the fits file, 'hud' an index of the header, and 'key' the content being queried\n");
    fprintf (stdout,
             THIS_SOLVER ":   for specified 'hud' header.\n");
    fprintf (stdout,
             THIS_SOLVER ": See manual for details.\n");
    rerror ( THIS_SOLVER ": requires 1,2 or 3 arguments");
  }

  // we force user to use URLs for the file names:
  // process filename
  RLABCODE_PROCESS_ARG1_S(THIS_SOLVER,0,e1,name,1);

  rf = get_fits_rfile_ds (name, mode);
  if (!rf)
  {
    goto _exit;
  }

  // process hdu
  if (nargs>1)
  {
    e2 = bltin_get_ent( args[1] );
    if (ent_type(e2) == MATRIX_DENSE_REAL)
    {
      idx_hdu = ent_data(e2);
    }
  }

  if (!idx_hdu)
  {
    goto _loop;
  }

  // if 'name' does not contain index of an HDU, then start iterating over all HDUs
  for (m=0; m<SIZE(idx_hdu); m++)
  {
    ihdu = mdiV0(idx_hdu,m);
    if (fits_movabs_hdu(rf->fptr, ihdu, &hdutype, &status))
    {
      continue;
    }

    // get no. of keywords
    i = fits_get_hdrpos(rf->fptr, &nkeys, &keypos, &status);
    if (i)
    {
      continue; /* error occured: nothing to do */
    }

    // create index
    sprintf(s_ihdu,"%d",ihdu);
    if (nkeys < 1)
    {
      continue;
    }

    kvc = mds_Create(nkeys,1);
    for (key=1; key<=nkeys; key++)
    {
      if (fits_read_record(rf->fptr, key, card, &status))
      {
        continue;
      }

      if (isvalidstring(card)>0)
      {
        chomp_string(card);
        MdsV1(kvc,key) = cpstr(card);
      }
    }
    install(rval, s_ihdu, ent_Assign_Rlab_MDS(kvc));
  }

  goto _exit;

_loop:

  // if 'name' does not contain index of an HDU, then start iterating over all HDUs
  for (ihdu=1; !(fits_movabs_hdu(rf->fptr, ihdu, &hdutype, &status) ); ihdu++)
  {
    // get no. of keywords
    i = fits_get_hdrpos(rf->fptr, &nkeys, &keypos, &status);
    if (i)
    {
      continue; /* error occured: nothing to do */
    }

    // create index
    sprintf(s_ihdu,"%d",ihdu);
    if (nkeys < 1)
    {
      continue;
    }

    kvc = mds_Create(nkeys,1);
    for (key=1; key<=nkeys; key++)
    {
      if (fits_read_record(rf->fptr, key, card, &status))
      {
        continue;
      }

      if (isvalidstring(card)>0)
      {
        chomp_string(card);
        MdsV1(kvc,key) = cpstr(card);
      }
    }
    install(rval, s_ihdu, ent_Assign_Rlab_MDS(kvc));
  }

_exit:

  // clean up rlab
  ent_Clean(e1);
  ent_Clean(e2);

  return ent_Assign_Rlab_BTREE (rval);
}

#undef  THIS_SOLVER
#define THIS_SOLVER RLAB_NAME_FITSIO ".readimage"
//
// open fitsio file using library
//
Ent *
ent_cfitsio_readimage (int nargs, Datum args[])
{
  Ent *e1=0, *e2=0;
  char *name=0, *smode=0;
  int mode=READONLY, i, m;
  int ihdu=-1, status=0, hdutype=0, fpixel=1, nullval=0, nfound=0, anynull=0;
  long naxes[2];

  fits_rfile * rf=0;

  char s_ihdu[FLEN_CARD];

  MDR *dimage=0, *idx_hdu=NULL;
  Btree *rval=btree_Create();

  if ((nargs<1) || (nargs > 3))
  {
    fprintf (stdout,
             THIS_SOLVER ": cfitsio wrapper library for RLaB\n");
    fprintf (stdout,
             THIS_SOLVER ": read headers from an FITS file\n");
    fprintf (stdout,
             THIS_SOLVER ": Format:\n");
    fprintf (stdout,
             THIS_SOLVER ":   " THIS_SOLVER "(filename,/hdu,/key//),\n");
    fprintf (stdout,
             THIS_SOLVER ": where\n");
    fprintf (stdout,
             THIS_SOLVER ":   'filename' is the fits file, 'hud' an index of the header, and 'key' the content being queried\n");
    fprintf (stdout,
             THIS_SOLVER ":   for specified 'hud' header.\n");
    fprintf (stdout,
             THIS_SOLVER ": See manual for details.\n");
    rerror ( THIS_SOLVER ": requires 1,2 or 3 arguments");
  }

  // we force user to use URLs for the file names:

  // process filename
  RLABCODE_PROCESS_ARG1_S(THIS_SOLVER,0,e1,name,1);

  // process hdu
  if (nargs>1)
  {
    e2 = bltin_get_ent( args[1] );
    if (ent_type(e2) == MATRIX_DENSE_REAL)
    {
      idx_hdu = ent_data(e2);
    }
  }

  rf = get_fits_rfile_ds (name, mode);
  if (!rf)
  {
    goto _exit;
  }

  if (!idx_hdu)
  {
    goto _loop;
  }

  // if 'name' does not contain index of an HDU, then start iterating over all HDUs
  for (m=0; m<SIZE(idx_hdu); m++)
  {
    ihdu = mdiV0(idx_hdu,m);
    if (fits_movabs_hdu(rf->fptr, ihdu, &hdutype, &status))
    {
      continue;
    }
    // look for a table
    if ((hdutype!=ASCII_TBL) && (hdutype!=BINARY_TBL))
    {
      continue;
    }

    sprintf(s_ihdu,"%d",ihdu);

    if (fits_read_keys_lng(rf->fptr, "NAXIS", 1, 2, naxes, &nfound, &status) != 0)
    {
      fits_report_error(stderr, status); /* print error report */
      continue;
    }

    dimage = mdr_Create(naxes[0],naxes[1]);
    if (fits_read_img(rf->fptr, TDOUBLE, fpixel, naxes[0]*naxes[1], &nullval, (double *) MDRPTR(dimage),
        &anynull, &status) )
    {
      mdr_Destroy(dimage);
      fits_report_error(stderr, status); /* print error report */
      continue;
    }

    install(rval, s_ihdu, ent_Assign_Rlab_MDR(dimage));
  }

  goto _exit;

_loop:

  // if 'name' does not contain index of an HDU, then start iterating over all HDUs
  for (ihdu=1; !(fits_movabs_hdu(rf->fptr, ihdu, &hdutype, &status)); ihdu++)
  {
    // create index
    sprintf(s_ihdu,"%d",ihdu);

    if (fits_read_keys_lng(rf->fptr, "NAXIS", 1, 2, naxes, &nfound, &status) != 0)
    {
      fits_report_error(stderr, status); /* print error report */
      continue;
    }

    dimage = mdr_Create(naxes[0],naxes[1]);
    if (fits_read_img(rf->fptr, TDOUBLE, fpixel, naxes[0]*naxes[1], &nullval, (double *) MDRPTR(dimage),
                      &anynull, &status) )
    {
      mdr_Destroy(dimage);
      fits_report_error(stderr, status); /* print error report */
      continue;
    }

    install(rval, s_ihdu, ent_Assign_Rlab_MDR(dimage));
  }

_exit:

  // clean up rlab
  ent_Clean(e1);
  ent_Clean(e2);

  return ent_Assign_Rlab_BTREE (rval);
}

#undef  THIS_SOLVER
#define THIS_SOLVER RLAB_NAME_FITSIO ".readtable"
//
// open fitsio file using library
//
Ent * ent_cfitsio_readtable (int nargs, Datum args[])
{
  Ent *e1=0, *e2=0;
  char *name=0, *smode=0;
  int mode=READONLY, i, j, k, m;
  int ihdu=0, status=0, hdutype=0, fpixel=1, nullval=0, nfound=0, anynull=0, hdunum=0;
  long nrows, ncols;
  fits_rfile * rf=0;
  char colname[FLEN_CARD], comment[FLEN_CARD], s_ihdu[FLEN_CARD], s_colidx[FLEN_CARD], keyname[FLEN_CARD], **ctstr=0;

  MDS *col_label=0,  *col_fmt=0, *sdata=0;
  MDR *rdata=0, *idx_hdu=NULL;
  Btree *rval=btree_Create(), *rval1=NULL;

  if ((nargs<1) || (nargs > 3))
  {
    fprintf (stdout,
             THIS_SOLVER ": cfitsio wrapper library for RLaB\n");
    fprintf (stdout,
             THIS_SOLVER ": read headers from an FITS file\n");
    fprintf (stdout,
             THIS_SOLVER ": Format:\n");
    fprintf (stdout,
             THIS_SOLVER ":   " THIS_SOLVER "(filename,/hdu,/key//),\n");
    fprintf (stdout,
             THIS_SOLVER ": where\n");
    fprintf (stdout,
             THIS_SOLVER ":   'filename' is the fits file, 'hud' an index of the header, and 'key' the content being queried\n");
    fprintf (stdout,
             THIS_SOLVER ":   for specified 'hud' header.\n");
    fprintf (stdout,
             THIS_SOLVER ": See manual for details.\n");
    rerror ( THIS_SOLVER ": requires 1,2 or 3 arguments");
  }

  // we force user to use URLs for the file names:
  // process filename
  RLABCODE_PROCESS_ARG1_S(THIS_SOLVER,0,e1,name,1);

  // process hdu
  if (nargs>1)
  {
    e2 = bltin_get_ent( args[1] );
    if (ent_type(e2) == MATRIX_DENSE_REAL)
    {
      idx_hdu = ent_data(e2);
    }
  }

  rf = get_fits_rfile_ds (name, mode);
  if (!rf)
  {
    goto _exit;
  }

  if (!idx_hdu)
  {
    goto _loop;
  }

  // if 'name' does not contain index of an HDU, then start iterating over all HDUs
  for (m=0; m<SIZE(idx_hdu); m++)
  {
    ihdu = mdiV0(idx_hdu,m);
    if (fits_movabs_hdu(rf->fptr, ihdu, &hdutype, &status))
    {
      continue;
    }
    // look for a table
    if ((hdutype!=ASCII_TBL) && (hdutype!=BINARY_TBL))
    {
      continue;
    }

    sprintf(s_ihdu,"%d",ihdu);

    // read size of the table: columns
    if (fits_read_key_lng(rf->fptr, "TFIELDS", &ncols, comment, &status) != 0)
    {
      fits_report_error(stderr, status); /* print error report */
      continue;
    }

    // read size of the table: rows
    if (fits_read_key_lng(rf->fptr, "NAXIS2", &nrows, comment, &status) != 0)
    {
      fits_report_error(stderr, status); /* print error report */
      continue;
    }

    //
    // create tree for each table:
    //
    rval1 = btree_Create();

    //
    install(rval1, "ncols", ent_Create_Rlab_Int(ncols));

    //
    install(rval1, "nrows", ent_Create_Rlab_Int(nrows));

    //
    // create string array of column names:
    //
    col_label = mds_Create(1, ncols);
    for (i=1; i<=ncols; i++)
    {
      sprintf(keyname,"TTYPE%d",i);
      if (fits_read_key_str(rf->fptr, keyname, colname, comment, &status) != 0)
      {
        fits_report_error(stderr, status); /* print error report */
        continue;
      }

      MdsV1(col_label,i) = cpstr(colname);
    }
    install(rval1, "clabel", ent_Assign_Rlab_MDS(col_label));

    //
    // create string array of column formats:
    //
    col_fmt = mds_Create(1, ncols);
    for (i=1; i<=ncols; i++)
    {
      sprintf(keyname,"TFORM%d",i);
      if (fits_read_key_str(rf->fptr, keyname, colname, comment, &status) != 0)
      {
        fits_report_error(stderr, status); /* print error report */
        continue;
      }

      MdsV1(col_fmt,i) = cpstr(colname);
    }
    install(rval1, "cfmt", ent_Assign_Rlab_MDS(col_fmt));

    // read column data:
    for (i=1; i<=ncols; i++)
    {
      sprintf(keyname,"%d",i);

      if(!strcmp("1E", MdsV1(col_fmt,i)))
      {
        rdata = mdr_Create(nrows,1);

        fits_read_col(rf->fptr, TDOUBLE, i, 1, 1, nrows, NULL, MDRPTR(rdata), &anynull, &status);

        install(rval1, keyname, ent_Assign_Rlab_MDR(rdata));
      }
      else if(strstr(MdsV1(col_fmt,i), "A"))
      {
        int l=atol(MdsV1(col_fmt,i));
        ctstr = (char **) GC_malloc(nrows * sizeof(char*));
        for (j=0; j<nrows; j++)
        {
          ctstr[j] = (char *) GC_malloc(l * sizeof(char));
        }

        fits_read_col(rf->fptr, TSTRING, i, 1, 1, nrows, NULL,         ctstr, &anynull, &status);
        rdata = mdi_Create(nrows,l);
        for (j=0; j<nrows; j++)
        {
          for (k=0; k<l; k++)
          {
            Mdi0(rdata,j,k) = (ctstr[j][k] & 0xff);
          }
          GC_FREE(ctstr[j]);
        }

        GC_FREE(ctstr);

        install(rval1, keyname, ent_Assign_Rlab_MDR(rdata));
      }
    }

    install(rval, s_ihdu, ent_Assign_Rlab_BTREE(rval1));
  }

  goto _exit;

_loop:

  // if 'name' does not contain index of an HDU, then start iterating over all HDUs
  for (ihdu=1; !(fits_movabs_hdu(rf->fptr, ihdu, &hdutype, &status)); ihdu++)
  {
    // look for a table
    if ((hdutype!=ASCII_TBL) && (hdutype!=BINARY_TBL))
    {
      continue;
    }

    sprintf(s_ihdu,"%d",ihdu);

    // read size of the table: columns
    if (fits_read_key_lng(rf->fptr, "TFIELDS", &ncols, comment, &status) != 0)
    {
      fits_report_error(stderr, status); /* print error report */
      continue;
    }

    // read size of the table: rows
    if (fits_read_key_lng(rf->fptr, "NAXIS2", &nrows, comment, &status) != 0)
    {
      fits_report_error(stderr, status); /* print error report */
      continue;
    }

    //
    // create tree for each table:
    //
    rval1 = btree_Create();

    //
    install(rval1, "ncols", ent_Create_Rlab_Int(ncols));

    //
    install(rval1, "nrows", ent_Create_Rlab_Int(nrows));

    //
    // create string array of column names:
    //
    col_label = mds_Create(1, ncols);
    for (i=1; i<=ncols; i++)
    {
      sprintf(keyname,"TTYPE%d",i);
      if (fits_read_key_str(rf->fptr, keyname, colname, comment, &status) != 0)
      {
        fits_report_error(stderr, status); /* print error report */
        continue;
      }

      MdsV1(col_label,i) = cpstr(colname);
    }
    install(rval1, "clabel", ent_Assign_Rlab_MDS(col_label));

    //
    // create string array of column formats:
    //
    col_fmt = mds_Create(1, ncols);
    for (i=1; i<=ncols; i++)
    {
      sprintf(keyname,"TFORM%d",i);
      if (fits_read_key_str(rf->fptr, keyname, colname, comment, &status) != 0)
      {
        fits_report_error(stderr, status); /* print error report */
        continue;
      }

      MdsV1(col_fmt,i) = cpstr(colname);
    }
    install(rval1, "cfmt", ent_Assign_Rlab_MDS(col_fmt));

    // read column data:
    for (i=1; i<=ncols; i++)
    {
      sprintf(keyname,"%d",i);

      if(!strcmp("1E", MdsV1(col_fmt,i)))
      {
        rdata = mdr_Create(nrows,1);

        fits_read_col(rf->fptr, TDOUBLE, i, 1, 1, nrows, NULL, MDRPTR(rdata), &anynull, &status);

        install(rval1, keyname, ent_Assign_Rlab_MDR(rdata));
      }
      else if(strstr(MdsV1(col_fmt,i), "A"))
      {
        int l=atol(MdsV1(col_fmt,i));
        ctstr = (char **) GC_malloc(nrows * sizeof(char*));
        for (j=0; j<nrows; j++)
        {
          ctstr[j] = (char *) GC_malloc(l * sizeof(char));
        }

        fits_read_col(rf->fptr, TSTRING, i, 1, 1, nrows, NULL,         ctstr, &anynull, &status);
        rdata = mdi_Create(nrows,l);
        for (j=0; j<nrows; j++)
        {
          for (k=0; k<l; k++)
          {
            Mdi0(rdata,j,k) = (ctstr[j][k] & 0xff);
          }
          GC_FREE(ctstr[j]);
        }

        GC_FREE(ctstr);

        install(rval1, keyname, ent_Assign_Rlab_MDR(rdata));
      }
    }

    install(rval, s_ihdu, ent_Assign_Rlab_BTREE(rval1));
  }

_exit:

  // clean up rlab
  ent_Clean(e1);
  ent_Clean(e2);

  return ent_Assign_Rlab_BTREE (rval);
}




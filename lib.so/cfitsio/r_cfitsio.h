/* rgphoto2.h */

/* This file is a part of rlabplus
   Copyright (C) 2013  M. Kostrun

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

#include <stdio.h>
#include <fcntl.h>
#include "fitsio.h"

#include "rlab.h"
#include "ent.h"
#include "class.h"
#include "symbol.h"
#include "mem.h"
#include "mdr.h"
#include "mdrf1.h"
#include "mds.h"
#include "list.h"
#include "btree.h"
#include "bltin.h"
#include "util.h"
#include "mathl.h"
#include "function.h"
#include "lp.h"
#include "rfileio.h"
#include "rlab_solver_parameters_names.h"
#include "rlab_macros.h"
#include "rlab_macros_code.h"

#define CFITSIO_MAX_FOPEN 32

#define RLAB_NAME_FITSIO "fitsio"


struct _fitsio_file
{
  char            * name;       // The file/process name
  fitsfile        * fptr;       /* FITS file pointer, defined in fitsio.h */
  int               mode;       /* READONLY, */

  // The next FITSfile in the list
  struct _fitsio_file * next;
};
typedef struct _fitsio_file fits_rfile;


extern fits_rfile * create_fits_rfile (void);
extern fits_rfile * find_fits_rfile   (char *name);
extern fitsfile * get_fits_rfile_fptr (char *name);
extern int destroy_fits_rfile         (char *name);
extern void init_fits_rfile_list      (void);
extern void destroy_fits_rfile_list   (void);

extern Ent *ent_cfitsio_open  (int nargs, Datum args[]);
extern Ent *ent_cfitsio_close (int nargs, Datum args[]);
extern Ent *ent_cfitsio_readheader (int nargs, Datum args[]);
extern Ent *ent_cfitsio_readimage  (int nargs, Datum args[]);


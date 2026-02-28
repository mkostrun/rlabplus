/* r_fann.h */

/*  This file is a part of RLaB2/rlabplus ("Our"-LaB)
   Copyright (C) 2019 M. Kostrun

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
#ifndef RLAB_MATIO_H
#define RLAB_MATIO_H

//
// rlab headers
//
#include <rlab/rlab.h>
#include <rlab/mde.h>
#include <rlab/mdr.h>
#include <rlab/mdrf1.h>
#include <rlab/mdc.h>
#include <rlab/mdr_mdc.h>
#include <rlab/mdcf1.h>
#include <rlab/mdcf2.h>
#include <rlab/complex.h>
#include <rlab/ent.h>
#include <rlab/class.h>
#include <rlab/mem.h>
#include <rlab/bltin.h>
#include <rlab/util.h>
#include <rlab/mathl.h>
#include <rlab/rlab_solver_parameters_names.h>
#include <rlab/gc/gc.h>
#include <rlab/symbol.h>
#include <rlab/rfileio.h>
#include <rlab/rlab_macros.h>
#include <rlab/rlab_macros_code.h>

//
// standard headers
//
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <getopt.h>
#include <stdarg.h>
#include <math.h>

//
// headers from matio
//
#include "matio.h"
#include "matioConfig.h"

typedef struct _m_file
{
  mat_t *           mat;
  int               mode;
  enum mat_ft       ver;
  struct _m_file *  next;
} Mfile;


Ent * ent_matio_read  (int nargs, Datum args[]);
Ent * ent_matio_writem(int nargs, Datum args[]);
Ent * ent_matio_open  (int nargs, Datum args[]);
Ent * ent_matio_close (int nargs, Datum args[]);

#endif

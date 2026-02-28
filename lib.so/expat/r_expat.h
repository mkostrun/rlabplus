#ifndef RLABPLUS_EXPAT_H
#define RLABPLUS_EXPAT_H

//
// rlab headers
//
#include <rlab/rlab.h>
#include <rlab/mdr.h>
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
#include <rlab/btree.h>
#include <rlab/function.h>
#include <rlab/lp.h>
#include <rlab/list.h>
#include <rlab/symbol.h>
#include <rlab/ent.h>
#include <rlab/rlab_solver_parameters_names.h>
#include <rlab/rlab_macros_code.h>
#include <rlab/buffer.h>

//
// standard headers
//
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>

#include "expat.h"

#ifdef XML_LARGE_SIZE
#  define XML_FMT_INT_MOD "ll"
#else
#  define XML_FMT_INT_MOD "l"
#endif

#ifdef XML_UNICODE_WCHAR_T
#  define XML_FMT_STR "ls"
#else
#  define XML_FMT_STR "s"
#endif

#define MAX_DEPTH 100
#undef  _THIS_LIB
#define _THIS_LIB "expat"

#undef  DEBUG
// #define DEBUG

typedef struct _xml_rlab
{
  int min_depth;
  int max_depth;
  int depth;
  char  *node_name [MAX_DEPTH];
  Btree *node_btree[MAX_DEPTH];
} XML_rlab_list;

#endif
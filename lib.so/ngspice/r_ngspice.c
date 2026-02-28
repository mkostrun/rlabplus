// Copyright (C) 2003-2013 Marijan Kostrun
//   part of rlabplus for linux project on rlabplus.sourceforge.net
//
// ngspice wrapper for rlabplus
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
// See the file ./COPYING

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

#define _THIS_LIB "r_ngspice.c"

// ngspice specific
#include "r_ngspice.h"
typedef void *  funptr_t;

static int exit_status;

bool no_bg = true;
bool not_yet = true;
bool will_unload = false;

void * ngdllhandle = NULL;

static char *loadstring = "libngspice.so";
static int *ngspice_ret;
pthread_t mainthread;

/* callback functions used by ngspice */
// int ng_getchar     (char* outputreturn, void* userdata);
// int ng_getstat     (char* outputreturn, void* userdata);
// int ng_thread_runs (bool noruns, void* userdata);

// ControlledExit ng_exit;
// SendData ng_data;
// SendInitData ng_initdata;

static FILE *ngspice_fptr = NULL;

void alterp(int sig);

/* functions exported by ngspice */
funptr_t ngSpice_Init_handle = NULL;
funptr_t ngSpice_Command_handle = NULL;
funptr_t ngSpice_Circ_handle = NULL;
funptr_t ngSpice_CurPlot_handle = NULL;
funptr_t ngSpice_AllVecs_handle = NULL;
funptr_t ngSpice_GVI_handle = NULL;

//
// in place tolower
//
static char * mytolower(char *t)
{
  int i=-1;
  while (t[++i])
  { t[i] = tolower(t[i]); }
  return t;
}

static char * mycopyrename(char *name)
{
  char rname[256];
  int i=-1, j=-1;

//   fprintf(stderr, "name = %s\n", name);

  if (strstr(name, "#branch"))
  {
    rname[++i] = 'i';
    rname[++i] = '_';
//     rname[++i] = '(';
    while(name[++j]!='#')
    {
      ++i;
      if (name[j]!='.')
        rname[i] = name[j];
      else
        rname[i] = '_';
    }
//     rname[++i] = ')';
//     rname[++i] = '\0';
  }
  else if (strstr(name, "V(") || strstr(name, "v-"))
  {
    rname[++i] = name[++j];
    rname[++i] = '_';
    ++j;
    while(name[++j]!=')')
    {
      if (!name[j])
      { break; }
      ++i;
      if (name[j]!='.')
        rname[i] = name[j];
      else
        rname[i] = '_';
    }
//     rname[++i]='\0';
  }
  else if (strstr(name, "time"))
  {
    return cpstr(name);
  }
  else
  {
    // for subcircuits replace '.' with '_'
    rname[++i] = 'v';
    rname[++i] = '_';
    while (name[++j])
    {
      if (name[j]!='.')
        rname[++i] = name[j];
      else
        rname[++i] = '_';
    }
  }

  rname[++i]='\0';
  return cpstr(rname);
}

#include <time.h>
#include <sys/types.h>
#include <sys/times.h>
#include <sys/timeb.h>
#include <sys/resource.h>
#include <sys/param.h>
struct timeb timebegin;
void timediff(struct timeb *now, struct timeb *begin, int *sec, int *msec)
{
  *msec = now->millitm - begin->millitm;
  *sec = (int)(now->time - begin->time);
  if (*msec < 0) {
    *msec += 1000;
    (*sec)--;
  }
  return;
}

/* Case insensitive str eq. */
/* Like strcasecmp( ) XXX */
int cieq(register char *p, register char *s)
{
  while (*p) {
    if ((isupper(*p) ? tolower(*p) : *p) !=
      (isupper(*s) ? tolower(*s) : *s))
      return(false);
    p++;
    s++;
  }
  return (*s ? false : true);
}

/* Funcion called from main thread upon receiving signal SIGTERM */
void alterp(int sig)
{
  ((int * (*)(char*)) ngSpice_Command_handle)("bg_halt");
}

/* Callback function called from bg thread in ngspice to transfer
 *   any string created by printf or puts. Output to stdout in ngspice is
 *   preceded by token stdout, same with stderr.*/
static int ng_getchar(char* outputreturn, void* userdata)
{
  if (ngspice_fptr)
    fprintf(ngspice_fptr, "ng_getchar: %s\n", outputreturn);
  return 0;
}

/* Callback function called from bg thread in ngspice to transfer
 *   simulation status (type and progress in percent. */
static int ng_getstat(char* outputreturn, void* userdata)
{
  if (ngspice_fptr)
    fprintf(ngspice_fptr, "ng_getstat: %s\n", outputreturn);
  return 0;
}

/* Callback function called from ngspice upon starting (returns true) or
 *  leaving (returns false) the bg thread. */
int ng_thread_runs(bool noruns, void* userdata)
{
  no_bg = noruns;
  if (ngspice_fptr)
  {
    if (noruns)
      fprintf(ngspice_fptr, "ng_thread_runs: Spice not running!\n");
    else
      fprintf(ngspice_fptr, "ng_thread_runs: Spice running in the background!\n");
  }
  return 0;
}

/* Callback function called from bg thread in ngspice if fcn controlled_exit()
 *   is hit. Do not exit, but unload ngspice. */
int ng_exit(int exitstatus, bool immediate, bool quitexit, void* userdata)
{

  if(quitexit) {
    printf("DNote: Returned form quit with exit status %d\n", exitstatus);
  }
  if(immediate)
  {
    printf("DNote: Unload ngspice\n");
    ((int * (*)(char*)) ngSpice_Command_handle)("bg_pstop");
    dlclose(ngdllhandle);
  }

  else {
    printf("DNote: Prepare unloading ngspice\n");
    will_unload = true;
  }

  exit_status = exitstatus;

  return exitstatus;
}

/* Callback function called from bg thread in ngspice once per accepted data point */
// int
// ng_data(pvecvaluesall vdata, int numvecs, void* userdata)
// {
//   int *ret;
//
//   v2dat = vdata->vecsa[vecgetnumber]->creal;
//   if (!has_break && (v2dat > 0.5)) {
//     /* using signal SIGTERM by sending to main thread, alterp() then is run from the main thread,
//      *      (not on Windows though!)  */
//     #ifndef _MSC_VER
//     if (testnumber == 4)
//       pthread_kill(mainthread, SIGTERM);
//     #endif
//       has_break = true;
//       /* leave bg thread for a while to allow halting it from main */
//       #if defined(__MINGW32__) || defined(_MSC_VER)
//       Sleep (100);
//       #else
//       usleep (100000);
//       #endif
//       //        ret = ((int * (*)(char*)) ngSpice_Command_handle)("bg_halt");
//   }
//   return 0;
// }

/* Callback function called from bg thread in ngspice once upon intialization
 *   of the simulation vectors)*/
// static int ng_initdata(pvecinfoall intdata, void* userdata)
// {
//   int i;
//   int vn = intdata->veccount;
//   for (i = 0; i < vn; i++) {
//     printf("Vector: %s\n", intdata->vecs[i]->vecname);
//     /* find the location of V(2) */
//     if (cieq(intdata->vecs[i]->vecname, "V(2)"))
//       vecgetnumber = i;
//   }
//   return 0;
// }


Ent *
ent_ngspice_Initialize (int nargs, Datum args[])
{
  Ent *e1=0;
  char *s=NULL, *errmsg=NULL;
  int   rval=1;


  if (nargs>=1)
  {
    //
    // commands as a string vector
    //
    e1 = bltin_get_ent(args[0]);
    if (ent_type(e1) != MATRIX_DENSE_STRING)
      rerror("first argument has to be string vector!");
    s = class_char_pointer(e1);
  }

  // initialize output stream for all messages
  if (s)
  {
    if(!strcmp("stdout", s))
      ngspice_fptr = stdout;
    else if(!strcmp("stderr", s))
      ngspice_fptr = stderr;
    else
      ngspice_fptr = fopen (s, "a");
  }
  else
    ngspice_fptr = 0;

  if (!ngdllhandle)
  {
    rval = 0;

    ngdllhandle = dlopen(loadstring, RTLD_NOW);
    errmsg = dlerror();
    if (errmsg)
      fprintf(stderr, "%s\n", errmsg);
    if (!ngdllhandle)
      rerror("Failed to load libngspice.so !\n");
    if (!ngSpice_Init_handle)
    {
      ngSpice_Init_handle = dlsym(ngdllhandle, "ngSpice_Init");
      errmsg = dlerror();
      if (errmsg)
      {
        fprintf(stderr, "%s\n", errmsg);
        rerror("Horrible internal error: Failed to load libngspice.so !\n");
      }
      if (!ngSpice_Init_handle)
        rerror("Failed to load ngSpice_Init !\n");
    }

    if (!ngSpice_Command_handle)
    {
      ngSpice_Command_handle = dlsym(ngdllhandle, "ngSpice_Command");
      errmsg = dlerror();
      if (errmsg)
      {
        fprintf(stderr, "%s\n", errmsg);
        rerror("Horrible internal error: Failed to load libngspice.so !\n");
      }
      if (!ngSpice_Command_handle)
        rerror("Failed to load ngSpice_Command_handle !\n");
    }

    if (!ngSpice_Circ_handle)
    {
      ngSpice_Circ_handle = dlsym(ngdllhandle, "ngSpice_Circ");
      errmsg = dlerror();
      if (errmsg)
      {
        fprintf(stderr, "%s\n", errmsg);
        rerror("Horrible internal error: Failed to load libngspice.so !\n");
      }
      if (!ngSpice_Circ_handle)
        rerror("Failed to load ngSpice_Circ_handle !\n");
    }

    if (!ngSpice_CurPlot_handle)
    {
      ngSpice_CurPlot_handle = dlsym(ngdllhandle, "ngSpice_CurPlot");
      errmsg = dlerror();
      if (errmsg)
      {
        fprintf(stderr, "%s\n", errmsg);
        rerror("Horrible internal error: Failed to load libngspice.so !\n");
      }
      if (!ngSpice_CurPlot_handle)
        rerror("Failed to load ngSpice_CurPlot_handle !\n");
    }

    if (!ngSpice_AllVecs_handle)
    {
      ngSpice_AllVecs_handle = dlsym(ngdllhandle, "ngSpice_AllVecs");
      errmsg = dlerror();
      if (errmsg)
      {
        fprintf(stderr, "%s\n", errmsg);
        rerror("Horrible internal error: Failed to load libngspice.so !\n");
      }
      if (!ngSpice_AllVecs_handle)
        rerror("Failed to load ngSpice_AllVecs_handle !\n");
    }

    if (!ngSpice_GVI_handle)
    {
      ngSpice_GVI_handle = dlsym(ngdllhandle, "ngGet_Vec_Info");
      errmsg = dlerror();
      if (errmsg)
      {
        fprintf(stderr, "%s\n", errmsg);
        rerror("Horrible internal error: Failed to load libngspice.so !\n");
      }
      if (!ngSpice_GVI_handle)
        rerror("Failed to load ngSpice_GVI_handle !\n");
    }

    if (!ngspice_ret)
    {
      ngspice_ret = ((int * (*)
      (SendChar*, SendStat*, ControlledExit*, SendData*, SendInitData*, BGThreadRunning*, void*))
      ngSpice_Init_handle)
      (ng_getchar, ng_getstat, ng_exit, NULL, NULL, ng_thread_runs, NULL);
    }
  }

  ent_Clean (e1);

  return ent_Create_Rlab_Int(rval);
}


Ent *
ent_ngspice_Finalize (int nargs, Datum args[])
{
  int rval=1;

  if (ngdllhandle)
  {
    dlclose(ngdllhandle);
    rval=0;
  }
  if (ngspice_fptr)
  {
    fclose(ngspice_fptr);
    ngspice_fptr = 0;
  }

  // clean it up
  ngdllhandle = NULL;
  ngspice_ret = NULL;
  ngSpice_Init_handle = NULL;
  ngSpice_Command_handle = NULL;
  ngSpice_Circ_handle = NULL;
  ngSpice_CurPlot_handle = NULL;
  ngSpice_AllVecs_handle = NULL;
  ngSpice_GVI_handle = NULL;

  return ent_Create_Rlab_Double(rval);
}

Ent *
ent_ngspice_IsRunning (int nargs, Datum args[])
{
  int rval=1;

  if (!ngdllhandle)
    rerror("initialize ngspice first!");

  if (no_bg)
    rval = 0;

  return ent_Create_Rlab_Int(rval);
}

Ent *
ent_ngspice_Command (int nargs, Datum args[])
{
  Ent *e1=0;
  MDS *s1=0;

  int i,n,k=0;

  if (!ngdllhandle)
    rerror("initialize ngspice first!");

  //
  // commands as a string vector
  //
  e1 = bltin_get_ent(args[0]);
  if (ent_type(e1) != MATRIX_DENSE_STRING)
    rerror("first argument has to be string vector!");
  s1 = ent_data(e1);
  if (!s1)
    rerror("first argument has to be string vector!");
  if ((s1->nrow)!=1 && (s1->ncol)!=1)
    rerror("first argument has to be string vector!");

  // run simulation in the background
  n = (s1->nrow) * (s1->ncol);
  for (i=0; i<n; i++)
  {
    ngspice_ret = ((int * (*)(char*)) ngSpice_Command_handle)(MdsV0(s1,i));
    if (strstr(MdsV0(s1,i), "bg_run") || strstr(MdsV0(s1,i), "bg_resume"))
    {
      while(no_bg && (++k)<10)
      { usleep (1000); }
    }
  }

  ent_Clean(e1);

  return ent_Create_Rlab_Success();
}


Ent *
ent_ngspice_RunCkt (int nargs, Datum args[])
{
  Ent *e1=0;
  MDS *s1;
  char **circarray=0;

  int n, i, j;

  if (!ngdllhandle || !ngSpice_Circ_handle || !ngSpice_Command_handle)
    rerror("initialize ngspice first!");

  //
  // circuit as a string vector
  //
  e1 = bltin_get_ent(args[0]);
  if (ent_type(e1) != MATRIX_DENSE_STRING)
    rerror("first argument has to be string vector!");
  s1 = ent_data ( e1 );
  if (!s1)
    rerror("first argument has to be string vector!");
  if ((s1->nrow)!=1 && (s1->ncol)!=1)
    rerror("first argument has to be string vector!");

  // clear the old data in spice
  ngspice_ret = ((int * (*)(char*) ) ngSpice_Command_handle)("destroy all");

  // copy the pointers only, rlab keeps the data
  n  = (s1->nrow) * (s1->ncol);
  circarray = (char**) GC_MALLOC(sizeof(char*) * (n+1));
  j = 0;
  for (i=0; i<n; i++)
  {
    if (strlen(MdsV0(s1,i))>0)
      circarray[j++] = MdsV0(s1,i);
  }
  circarray[j] = NULL;  // add this per ngspice quirk

  ngspice_ret = ((int * (*)(char**)) ngSpice_Circ_handle)(circarray);

  ngspice_ret = ((int * (*)(char*) ) ngSpice_Command_handle)("bg_run");

  GC_FREE(circarray);

  // wait here until program starts
  i = 0;
  while(no_bg && (++i)<10)
  { usleep (1000); }

  exit_status = 0;

  ent_Clean(e1);

  return ent_Create_Rlab_Success();
}

Ent *
ent_ngspice_Circuit (int nargs, Datum args[])
{
  Ent *e1=0;
  MDS *s1;
  char **circarray=0;

  int n, i, j;

  if (!ngdllhandle || !ngSpice_Circ_handle || !ngSpice_Command_handle)
    rerror("initialize ngspice first!");

  //
  // circuit as a string vector
  //
  e1 = bltin_get_ent(args[0]);
  if (ent_type(e1) != MATRIX_DENSE_STRING)
    rerror("first argument has to be string vector!");
  s1 = ent_data ( e1 );

  if ((s1->nrow)!=1 && (s1->ncol)!=1)
    rerror("first argument has to be string vector!");

  // copy the pointers only, rlab keeps the data
  n  = (s1->nrow) * (s1->ncol);
  circarray = (char**) GC_MALLOC(sizeof(char*) * (n+1));
  j = 0;
  for (i=0; i<n; i++)
  {
    if (strlen(MdsV0(s1,i))>0)
      circarray[j++] = MdsV0(s1,i);
  }
  circarray[j] = NULL;  // add this per ngspice quirk

  ngspice_ret = ((int * (*)(char**)) ngSpice_Circ_handle)(circarray);

  GC_FREE(circarray);

  ent_Clean(e1);

  exit_status = 0;

  return ent_Create_Rlab_Success();
}

Ent *
ent_ngspice_GetVals (int nargs, Datum args[])
{
  char *curplot=0, **vecarray=0;
  char plotvec[256];
  int n=0, i, nvec, j;
  Btree *bw_spice=0, *bw_data=0;

  if (!ngdllhandle)
    rerror("initialize ngspice first!");

  curplot = ((char * (*)()) ngSpice_CurPlot_handle)();
  vecarray = ((char ** (*)(char*)) ngSpice_AllVecs_handle)(curplot);
  nvec = -1;
  while(vecarray[++nvec])
  {}

  // copy each vector from ngspice
  bw_spice = btree_Create ();
  ent_type (bw_spice) = BTREE;

  //
  // return the result as a list
  //
  bw_data = btree_Create ();
  ent_type (bw_data) = BTREE;
  for (i=0;i<nvec;i++)
  {
    pvector_info myvec;

    plotvec[0] = '\0';
//     fprintf(stderr,"vecarray = %s\n", vecarray[i]);
    sprintf(plotvec, "%s.%s", curplot, vecarray[i]);
    myvec = ((pvector_info (*)(char*)) ngSpice_GVI_handle)(plotvec);
    if(!myvec)
    {
      fprintf(stdout,"Uh-oh! What is going on?\n");
      continue;
    }

    // create the entity and copy this information in it
    Ent * ent_ngspice = ent_Create();
    MDR * rdata=0;
    MDC * cdata=0;
    if (myvec->v_compdata)
    {
      n = myvec->v_length;
      cdata = mdc_Create(myvec->v_length, 1);
      for (j=0; j<n; j++)
      {
        MdcV0r(cdata,j) = myvec->v_compdata[j].cx_real;
        MdcV0i(cdata,j) = myvec->v_compdata[j].cx_imag;
      }
      ent_data (ent_ngspice) = cdata;
      ent_type (ent_ngspice) = MATRIX_DENSE_COMPLEX;
      install  (bw_data, mytolower(mycopyrename(vecarray[i])), ent_ngspice);
    }
    else if (myvec->v_realdata)
    {
      n = myvec->v_length;
      rdata = mdr_Create(myvec->v_length, 1);
      for (j=0; j<n; j++)
      {
        MdrV0(rdata,j) = myvec->v_realdata[j];
      }
      ent_data (ent_ngspice) = rdata;
      ent_type (ent_ngspice) = MATRIX_DENSE_REAL;
      install  (bw_data, mytolower(mycopyrename(vecarray[i])), ent_ngspice);
    }
  }

  install (bw_spice, "data", ent_Assign_Rlab_BTREE(bw_data) );
  install (bw_spice, "variables", ent_Create_Rlab_Double(nvec));
  install (bw_spice, "points", ent_Create_Rlab_Double(n));
  install (bw_spice, "plot", ent_Create_Rlab_String(curplot));

  return ent_Assign_Rlab_BTREE(bw_spice);
}
















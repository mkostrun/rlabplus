// Copyright (C) 2003-2010 Marijan Kostrun
//   part of rlabplus for linux project on rlabplus.sourceforge.net
//
// linux-gpib wrapper for rlabplus
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

//
// standard headers
//
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdint.h>
#include <getopt.h>

#define _THIS_LIB "rlabplus_gpib.c"

#define DEBUG

//
// linux-gpib headers
//
#include <gpib/ib.h>
extern volatile int
    ibcnt;
extern volatile long
    ibcntl;
extern volatile int
    iberr;

//
// Library control variables:
//
// length of the character que
static int
    GPIB_MAX_NUM_BYTES = 524288;
// wait for operation to complete?
static int
    GPIB_WAIT_TO_COMPLETE = 1;
static int
    GPIB_PAUSE_WHILE_WAIT_TO_COMPLETE = 100;

#define GPIB_MAX_LEN_WRITE  64

// buffer for read operation:
static char
    *buffer=0;
static int
    buffer_len=0;


// catch ctrl-c, not to have to restart the gpib
//
static int keepRunning = 1;
void intHandler(int i)
{
  keepRunning = 0;
  return;
}

//
// ibrd: synchronous read from a single device
//
static int perform_ibrd(int iud, int inb)
{
  int i, j, k, ist;

  // expected number of bytes
  if (inb > 0)
    j = inb;
  else
    j = buffer_len;

  // user knows how many bytes to expect: read the bus
  // until desired number of bytes is received
  if (GPIB_WAIT_TO_COMPLETE)
  {
    usleep (GPIB_PAUSE_WHILE_WAIT_TO_COMPLETE);
    ibwait (iud, CMPL);
  }
  i = ibcnt;      // initial number of bytes, should be zero
  k = 0;          // received number of bytes
  ist = 0x0000;   // status, should not be TIMO - user must know what is she doing
  while (j>0 && !(ist&0x4000))
  {
    ist = ibrd(iud, &buffer[k], j);
    i = ibcnt;
    k = k + i;  // count
    j = j - i;
    if (!inb && (ist&0x2000))
      break;
  }

  if (GPIB_WAIT_TO_COMPLETE)
  {
    usleep (GPIB_PAUSE_WHILE_WAIT_TO_COMPLETE);
    ibwait (iud, CMPL);
  }

#ifdef DEBUG
  if (k!=inb && inb!=0)
  {
    fprintf(stderr, _THIS_LIB ": ibrd: (warning) expected %i bytes but only "
    "%i received\n", inb, k);
  }
#endif

  return (k);
}


//
// query read - my invention
//
Ent *
ent_gpib_ibqrd(int nargs, Datum args[])
{
  Ent *e1=0, *e2=0, *e3=0, *e4=0, *rent=0;
  int n=1, j=0, lenq=0, k=0;

  MDS *sq=0;
  MDR *w=0;
  MDS *ws=0;

  int iud, i, inb=0;

  //
  if (nargs != 3 && nargs != 4)
    rerror("ibqrd: three or four arguments required!");

  //
  // ud
  //
  e1 = bltin_get_ent(args[0]);
  if (ent_type(e1) != MATRIX_DENSE_REAL)
    rerror("ibqrd: first argument has to be an integer!");
  iud = (int) class_double ( e1 );

  // did user provide number of bytes to be read?
  //    if so, assume that the readout is a integer array
  //    otherwise assume a string
  e2 = bltin_get_ent(args[1]);
  if (ent_type(e2) == MATRIX_DENSE_REAL)
    inb = (int) class_double ( e2 );

  //
  // get query sequence
  //
  e3 = bltin_get_ent(args[2]);
  if (ent_type(e3) != MATRIX_DENSE_STRING)
    rerror("ibqrd: third argument has to be a string vector!");
  sq = ent_data ( e3 );
  if ((sq->nrow)!=1 && (sq->ncol)!=1)
    rerror("ibqrd: second argument has to be a string vector!");
  lenq  = (sq->nrow)*(sq->ncol);

  //
  // get number of queries
  //
  if (nargs > 3)
  {
    e4 = bltin_get_ent(args[3]);
    if (ent_type(e4) == MATRIX_DENSE_REAL)
    {
      n = (int) class_double ( e4 );
      if (n < 1)
        n=1;
    }
  }

  rent = ent_Create();

  // is there a buffer
  if (!buffer)
  {
    // no buffer. create new
    if (inb < GPIB_MAX_NUM_BYTES)
    {
      buffer = GC_malloc(GPIB_MAX_NUM_BYTES * sizeof(unsigned char));
      buffer_len = GPIB_MAX_NUM_BYTES;
    }
    else
    {
      buffer = GC_malloc(inb * sizeof(unsigned char));
      buffer_len = inb;
    }
  }
  else
  {
    // buffer exists adjust its size only if not big enough
    if (inb > buffer_len)
    {
      // buffer exists, extend it
      GC_realloc(buffer, inb - buffer_len);
      buffer_len = inb;
    }
  }

  struct sigaction act;
  act.sa_handler = intHandler;
  sigaction(SIGINT, &act, NULL);

  if (inb > 0)
  {
    w = mdi_Create(n,inb);

    for(i=0; i<n; i++)
    {
      // query
      for (j=0; (j<lenq) && keepRunning; j++)
      {
        ibwrt  (iud, MdsV0(sq,j), strlen(MdsV0(sq,j)));
        if (GPIB_WAIT_TO_COMPLETE)
        {
          usleep (GPIB_PAUSE_WHILE_WAIT_TO_COMPLETE);
          ibwait (iud, CMPL);
        }
      }

      k = perform_ibrd(iud, inb);
      for (j=0; j<k; j++)
      {
        Mdi0(w,i,j) = (int) buffer[j];
        buffer[j] = '\0';
      }
    }

    // return values:
    ent_data(rent) = w;
    ent_type(rent) = MATRIX_DENSE_REAL;
  }
  else
  {
    // reading the string from the device
    //
    ws = mds_Create(n, 1);

    for (i=0; i<n; i++)
    {
      // query
      for (j=0; (j<lenq) && keepRunning; j++)
      {
        ibwrt  (iud, MdsV0(sq,j), strlen(MdsV0(sq,j)));
        if (GPIB_WAIT_TO_COMPLETE)
        {
          usleep (GPIB_PAUSE_WHILE_WAIT_TO_COMPLETE);
          ibwait (iud, CMPL);
        }
      }

      k = perform_ibrd(iud, inb);
      if (k)
      {
        buffer[k] = '\0';
        MdsV0(ws,i) = cpstr( buffer );
      }
      else
        MdsV0(ws,i) = cpstr( "" );
    }

    // return values:
    ent_data(rent) = ws;
    ent_type(rent) = MATRIX_DENSE_STRING;
  }

  if (!keepRunning)
  {
    keepRunning=1;
    ibclr(iud);
    ibonl(iud,1);
    ibsic(0);
    ibloc(iud);
    rerror("ibwrta: operation interrupted\n");
  }

  ent_Clean(e1);
  ent_Clean(e2);
  ent_Clean(e3);
  ent_Clean(e4);

  return rent;
}


Ent *
ent_gpib_ibrd(int nargs, Datum args[])
{
  Ent *e1=0, *e2=0, *rent=0;
  rent = ent_Create();

  MDS *ws=0;
  MDR *w=0;

  int iud, inb=0;
  int i, k;

  //
  if (nargs != 1 && nargs != 2)
    rerror("ibrd: one or two arguments required!");

  //
  // ud
  //
  e1 = bltin_get_ent(args[0]);
  if (ent_type(e1) != MATRIX_DENSE_REAL)
    rerror("ibrd: first argument has to be an integer!");
  iud = (int) class_double ( e1 );

  // did user provide number of bytes to be read?
  //    if so, assume that the readout is a integer array
  //    otherwise assume a string
  if (nargs >= 2)
  {
    e2 = bltin_get_ent(args[1]);
    if (ent_type(e2) != MATRIX_DENSE_REAL)
      rerror("ibrd: second argument has to be an integer!");
    inb = (int) class_double ( e2 );
    if (inb < 1)
      inb = 1;
  }

  // is there a buffer
  if (!buffer)
  {
    // no buffer. create new
    if (inb <= GPIB_MAX_NUM_BYTES)
    {
      buffer = GC_malloc(GPIB_MAX_NUM_BYTES * sizeof(unsigned char));
      buffer_len = GPIB_MAX_NUM_BYTES;
    }
    else
    {
      buffer = GC_malloc(inb * sizeof(unsigned char));
      buffer_len = inb;
    }
  }
  else
  {
    // buffer exists adjust its size only if not bi enough
    if (inb > buffer_len)
    {
      // buffer exists, extend it
      GC_realloc(buffer, inb - buffer_len);
      buffer_len = inb;
    }
  }

  // do the read
  k = perform_ibrd(iud, inb);
  if (inb > 0)
  {
    // convert it to bytes
    if (k)
    {
      w = mdi_Create(1,k);
      for (i=0; i<k; i++)
      {
        MdiV0(w,i) = (int) buffer[i];
        buffer[i] = '\0';
      }
    }
    else
      w = mdi_Create(0,0);

    // return values:
    ent_data(rent) = w;
    ent_type(rent) = MATRIX_DENSE_REAL;
  }
  else
  {
    if (k)
    {
      // make a string out of it
      buffer[k] = '\0';
      ws = mds_CreateScalar( cpstr(buffer) );
    }
    else
      ws = mds_Create(0,0);

     // return values:
     ent_data(rent) = ws;
     ent_type(rent) = MATRIX_DENSE_STRING;
  }

  ent_Clean(e1);
  ent_Clean(e2);

  return rent;
}


Ent *
ent_gpib_ibdev(int nargs, Datum args[])
{
  Ent *e1=0, *e2=0, *e3=0, *e4=0, *rent=0;
  MDR *w;

  int iud, iminor;
  int ipad = 0;
  int isad = 0;
  int isend_eoi = 1;
  int eos = 0x140a; // set '\n' as read terminatiion and match all of its 8 bits
  int itimeout = T1s;

  //
  if (nargs <= 2)
    rerror("ibdev: at least three arguments required!");

  //
  // minor (board ID)
  //
  e1 = bltin_get_ent(args[0]);
  if (ent_type(e1) != MATRIX_DENSE_REAL)
    rerror("ibdev: first argument has to be an integer!");
  iminor = (int) class_double ( e1 );

  //
  // primary address
  //
  e2 = bltin_get_ent(args[1]);
  if (ent_type(e2) != MATRIX_DENSE_REAL)
    rerror("ibask: second argument has to be an integer!");
  ipad = (int) class_double ( e2 );

  //
  // secondary address
  //
  e3 = bltin_get_ent(args[2]);
  if (ent_type(e3) != MATRIX_DENSE_REAL)
    rerror("ibask: second argument has to be an integer!");
  isad = (int) class_double ( e3 );

  //
  // timeout
  //
  if (nargs > 3)
  {
    e4 = bltin_get_ent(args[3]);
    if (ent_type(e4) == BTREE)
    {
      ListNode *node;
      node = btree_FindNode (ent_data (e4), "timeout");
      if (node != 0)
      {
        char * stimeout = class_char_pointer (var_ent (node));
        if (!strcmp (stimeout, "TNONE"))
          itimeout = TNONE;     /* Infinite timeout (disabled)     */
        else if (!strcmp (stimeout, "T10us"))
          itimeout = T10us;     /* Timeout of 10 usec (ideal)      */
        else if (!strcmp (stimeout, "T30us"))
          itimeout = T30us;     /* Timeout of 30 usec (ideal)      */
        else if (!strcmp (stimeout, "T100us"))
          itimeout = T100us;    /* Timeout of 100 usec (ideal)     */
        else if (!strcmp (stimeout, "T300us"))
          itimeout = T300us;    /* Timeout of 300 usec (ideal)     */
        else if (!strcmp (stimeout, "T1ms"))
          itimeout = T1ms;      /* Timeout of 1 msec (ideal)       */
        else if (!strcmp (stimeout, "T3ms"))
          itimeout = T3ms;      /* Timeout of 3 msec (ideal)       */
        else if (!strcmp (stimeout, "T10ms"))
          itimeout = T10ms;     /* Timeout of 10 msec (ideal)      */
        else if (!strcmp (stimeout, "T30ms"))
          itimeout = T30ms;     /* Timeout of 30 msec (ideal)      */
        else if (!strcmp (stimeout, "T100ms"))
          itimeout = T100ms;    /* Timeout of 100 msec (ideal)     */
        else if (!strcmp (stimeout, "T300ms"))
          itimeout = T300ms;    /* Timeout of 300 msec (ideal)     */
        else if (!strcmp (stimeout, "T1s"))
          itimeout = T1s;       /* Timeout of 1 sec (ideal)        */
        else if (!strcmp (stimeout, "T3s"))
          itimeout = T3s;       /* Timeout of 3 sec (ideal)        */
        else if (!strcmp (stimeout, "T10s"))
          itimeout = T10s;      /* Timeout of 10 sec (ideal)       */
        else if (!strcmp (stimeout, "T30s"))
          itimeout = T30s;      /* Timeout of 30 sec (ideal)       */
        else if (!strcmp (stimeout, "T100s"))
          itimeout = T100s;     /* Timeout of 100 sec (ideal)      */
        else if (!strcmp (stimeout, "T300s"))
          itimeout = T300s;     /* Timeout of 300 sec (ideal)      */
        else if (!strcmp (stimeout, "T1000s"))
          itimeout = T1000s;   /* Timeout of 1000 sec (maximum)   */
        else
          itimeout = T1s;       /* Timeout of 1 sec (ideal)        */
      }

      //
      node = btree_FindNode (ent_data (e4), "send_eoi");
      if (node != 0)
        isend_eoi = (int) class_double (var_ent (node));

      // eos: integer or character
      node = btree_FindNode (ent_data (e4), "eos");
      if (node != 0)
      {
        if (ent_type(var_ent (node)) == MATRIX_DENSE_REAL)
          eos = (int) class_double (var_ent (node));
        else if (ent_type(var_ent (node)) == MATRIX_DENSE_STRING)
        {
          char *ex = class_char_pointer(var_ent (node));
          eos = (int) ex[0];
        }
        eos = eos & 0xff; // take only last two bytes of it
        // only if user supplied eos check the mode
        if (eos)
        {
          node = btree_FindNode (ent_data (e4), "eos_mode");
          if (node != 0)
          {
            char * eosmode = class_char_pointer (var_ent (node));
            if (strstr (eosmode, "REOS"))
              eos = eos | REOS;
            if (strstr (eosmode, "XEOS"))
              eos = eos | XEOS;
            if (strstr (eosmode, "BIN"))
              eos = eos | BIN;
          }
        }
      }
    }
  }
  iud = ibdev(iminor, ipad, isad, itimeout, isend_eoi, eos);

  if (iud < 0)
    w = mdr_Create(0,0);
  else
    w = mdr_CreateScalar( iud );

  if (e1)
    ent_Clean(e1);
  if (e2)
    ent_Clean(e2);
  if (e3)
    ent_Clean(e3);
  if (e4)
    ent_Clean(e4);

  rent = ent_Create();
  ent_data(rent) = w;
  ent_type(rent) = MATRIX_DENSE_REAL;

  return rent;
}

Ent * ent_gpib_ibfind(int nargs, Datum args[])
{
  Ent *e1=0, *rent=0;
  MDR *w;
  MDS *s1;

  //
  if (nargs != 1)
    rerror("ibfind: single argument required!");

  //
  // name of the device
  //
  e1 = bltin_get_ent(args[0]);
  if (ent_type(e1) != MATRIX_DENSE_STRING)
    rerror("ibfind: device name has to be a single string!");
  s1 = ent_data ( e1 );

  if (MNR(s1) * MNC(s1) != 1)
    rerror("ibfind: device name has to be a single string!");

  w = mdr_CreateScalar( ibfind( MdsV0(s1,0) ) );

  if (e1)
    ent_Clean(e1);

  rent = ent_Create();
  ent_data(rent) = w;
  ent_type(rent) = MATRIX_DENSE_REAL;

  return rent;
}

//
// ibonl: close or reinitialize board or device
//
Ent * ent_gpib_ibonl(int nargs, Datum args[])
{
  Ent *e1=0, *e2=0, *rent=0;
  MDR *w;

  int iud, ien = 0;

  //
  if (nargs != 1 && nargs != 2)
    rerror("ibonl: one or two arguments required!");

  //
  // ud
  //
  e1 = bltin_get_ent(args[0]);
  if (ent_type(e1) != MATRIX_DENSE_REAL)
    rerror("ibonl: first argument has to be an integer!");
  iud = (int) class_double ( e1 );

  if (nargs >= 2)
  {
    e2 = bltin_get_ent(args[1]);
    if (ent_type(e2) != MATRIX_DENSE_REAL)
      rerror("ibonl: second argument has to be an integer!");
    ien = (int) class_double ( e2 );
  }

  w  = mdr_CreateScalar( ibonl( iud, ien ) );

  if (e1)
    ent_Clean(e1);
  if (e2)
    ent_Clean(e2);

  rent = ent_Create();
  ent_data(rent) = w;
  ent_type(rent) = MATRIX_DENSE_REAL;

  return rent;
}

//
// ibclr: clear device
//
Ent * ent_gpib_ibclr(int nargs, Datum args[])
{
  Ent *e1=0, *rent=0;
  MDR *w;

  int iud;

  //
  if (nargs != 1)
    rerror("ibclr: single argument required!");

  //
  // ud
  //
  e1 = bltin_get_ent(args[0]);
  if (ent_type(e1) != MATRIX_DENSE_REAL)
    rerror("ibclr: first argument has to be an integer!");
  iud = (int) class_double ( e1 );

  w  = mdr_CreateScalar( ibclr( iud ) );

  if (e1)
    ent_Clean(e1);

  rent = ent_Create();
  ent_data(rent) = w;
  ent_type(rent) = MATRIX_DENSE_REAL;

  return rent;
}

//
// ibsre: set remote enable (board)
//
Ent * ent_gpib_ibsre(int nargs, Datum args[])
{
  Ent *e1=0, *e2=0, *rent=0;
  MDR *w;

  int iud, ien = 0;
  int ist=0;

  //
  if (nargs != 1 && nargs != 2)
    rerror("ibsre: one or two arguments required!");

  //
  // ud
  //
  e1 = bltin_get_ent(args[0]);
  if (ent_type(e1) != MATRIX_DENSE_REAL)
    rerror("ibsre: first argument has to be an integer!");
  iud = (int) class_double ( e1 );

  if (nargs >= 2)
  {
    e2 = bltin_get_ent(args[1]);
    if (ent_type(e2) != MATRIX_DENSE_REAL)
      rerror("ibsre: second argument has to be an integer!");
    ien = (int) class_double ( e2 );
  }

  ist = ibsre( iud, ien );

#ifdef DEBUG
  if (ist & 0x8000)
    fprintf(stdout,"ibsre: returned ERROR code %x\n", iberr);
#endif

  w  = mdr_CreateScalar( ist );

  ent_Clean(e1);
  ent_Clean(e2);

  rent = ent_Create();
  ent_data(rent) = w;
  ent_type(rent) = MATRIX_DENSE_REAL;

  return rent;
}

//
// ibsic: set interface clear (board)
//
Ent * ent_gpib_ibsic(int nargs, Datum args[])
{
  Ent *e1=0, *rent=0;
  MDR *w;

  int iud;

  //
  if (nargs != 1)
    rerror("ibsic: one argument required!");

  //
  // ud
  //
  e1 = bltin_get_ent(args[0]);
  if (ent_type(e1) != MATRIX_DENSE_REAL)
    rerror("ibsic: first argument has to be an integer!");
  iud = (int) class_double ( e1 );

  w  = mdr_CreateScalar( ibsic( iud ) );

  ent_Clean(e1);

  rent = ent_Create();
  ent_data(rent) = w;
  ent_type(rent) = MATRIX_DENSE_REAL;

  return rent;
}

//
// ibln: check if listener is present
//
Ent * ent_gpib_ibln(int nargs, Datum args[])
{
  Ent *e1=0, *e2=0, *e3=0, *rent=0;
  MDR *w;

  int iud, ipa, isa;
  short ir = 0, is;

  //
  if (nargs != 3)
    rerror("ibln: three arguments required!");

  //
  // ud
  //
  e1 = bltin_get_ent(args[0]);
  if (ent_type(e1) != MATRIX_DENSE_REAL)
    rerror("ibln: first argument has to be an integer!");
  iud = (int) class_double ( e1 );

  //
  // pad
  //
  e2 = bltin_get_ent(args[1]);
  if (ent_type(e2) != MATRIX_DENSE_REAL)
    rerror("ibln: second argument has to be an integer!");
  ipa = (int) class_double ( e2 );

  e3 = bltin_get_ent(args[2]);
  if (ent_type(e3) != MATRIX_DENSE_REAL)
    rerror("ibln: third argument has to be an integer!");
  isa = (int) class_double ( e3 );

  is = ibln( iud, ipa, isa, &ir );

  w  = mdr_CreateScalar( (double) ir );
  //w  = mdr_CreateScalar( (double) is );

  ent_Clean(e1);
	ent_Clean(e2);
  ent_Clean(e3);

  rent = ent_Create();
  ent_data(rent) = w;
  ent_type(rent) = MATRIX_DENSE_REAL;

  return rent;
}


//
// ibwrt: synchronous write to a single device
//
Ent * ent_gpib_ibwrt(int nargs, Datum args[])
{
  Ent *e1=0, *e2=0, *e3=0, *rent=0;
  MDR *w=0, *x=0;
  MDS *s2=0;

  int iud, n, i, ibe, ilen, ifmt = 1;
  char *type=0;

  //
  if (nargs != 2 && nargs != 3)
    rerror("ibwrt: two arguments required!");

  //
  // ud
  //
  e1 = bltin_get_ent(args[0]);
  if (ent_type(e1) != MATRIX_DENSE_REAL)
    rerror("ibwrt: first argument has to be an integer!");
  iud = (int) class_double ( e1 );

  struct sigaction act;
  act.sa_handler = intHandler;
  sigaction(SIGINT, &act, NULL);

  //
  // second argument: string vector of commands to be passed on
  //                  or numerical data that is casted in the form
  //                  described by argument 3
  //
  e2 = bltin_get_ent(args[1]);
  if (ent_type(e2) == MATRIX_DENSE_STRING)
  {
    //
    // commands as a string vector
    //

    s2 = ent_data ( e2 );
    n  = MNR (s2) * MNC (s2);
    w  = mdi_CreateScalar(0);

    for (i=0; (i<n) && keepRunning; i++)
    {
      ilen = strlen(MdsV0(s2,i));
      ibwrt(iud, MdsV0(s2,i), ilen);
      ibe = ThreadIberr();
      MdiV0(w,i) |= ibe;

#ifdef DEBUG
      if (ibe)
      {
        fprintf(stderr, "ibwrt: error %s (0x%x) occured\n", gpib_error_string(ibe), ibe);
        fprintf(stderr, "ibwrt: %s\n", MdsV0(s2,i));
      }
#endif

    }
  }
  else if (ent_type(e2) == MATRIX_DENSE_REAL)
  {
    x = ent_data(e2);
    n  = MNR (x) * MNC (x);

    if (nargs == 3)
    {
      e3 = bltin_get_ent(args[2]);
      if (ent_type(e3) == MATRIX_DENSE_STRING)
      {
        type = class_char_pointer(e3);
      }

      if (type)
      {
        if (!strcmp (type, "unsigned char") || !strcmp (type, "uchar") || !strcmp (type, "uint8_t"))
          ifmt =  1;
        else if (!strcmp (type, "char") || !strcmp (type, "schar") || !strcmp (type, "int8_t"))
          ifmt = -1;
        else if (!strcmp (type, "unsigned short int")  || !strcmp (type, "uint16_t") || !strcmp (type, "uint16"))
          ifmt =  2;
        else if (!strcmp (type, "short int")  || !strcmp (type, "int16_t") || !strcmp (type, "int16"))
          ifmt = -2;
        else if (!strcmp (type, "unsigned int")  || !strcmp (type, "uint32_t") || !strcmp (type, "uint32"))
          ifmt =  4;
        else if (!strcmp (type, "int")  || !strcmp (type, "int32_t") || !strcmp (type, "int32"))
          ifmt = -4;
        else if (!strcmp (type, "float") || !strcmp (type, "float32"))
          ifmt =  5;
        else if (!strcmp (type, "double") || !strcmp (type, "float64"))
          ifmt =  5;
      }
    }

    w  = mdi_CreateScalar(0);

    unsigned char uc;
    char c;
    unsigned short int usi;
    short int si;
    unsigned int ui;
    int k;
    float f;
    double d;

    for (i=0; (i<n) && keepRunning; i++)
    {
      switch(ifmt)
      {
        case  1:
          uc = (unsigned char) mdiV0(x,i);
          ibwrt(iud, (const void *) &uc, 1);
          break;

        case -1:
          c  = (char) mdiV0(x,i);
          ibwrt(iud, (const void *) &c, 1);
          break;

        case  2:
          usi = (unsigned short int) mdiV0(x,i);
          ibwrt(iud, (const void *) &usi, 2);
          break;

        case -2:
          si = (short int) mdiV0(x,i);
          ibwrt(iud, (const void *) &si, 2);
          break;

        case  4:
          ui = (unsigned int) mdrV0(x,i);
          ibwrt(iud, (const void *) &ui, 4);
          break;

        case -4:
          k = mdiV0(x,i);
          ibwrt(iud, (const void *) &k, 4);
          break;

        case 5:
          f = (float) mdrV0(x,i);
          ibwrt(iud,  (const void *) &f, 4);
          break;

        case 6:
          d = mdrV0(x,i);
          ibwrt(iud,  (const void *) &d, 8);
          break;

        default:
          continue;
      }
    }
    ibe = ThreadIberr();
    MdiV0(w,i) |= ibe;
  }

  if (!keepRunning)
  {
    keepRunning=1;
    ibclr(iud);
    ibonl(iud,1);
    ibsic(0);
    ibloc(iud);
    rerror("ibwrt: operation interrupted\n");
  }

  ent_Clean(e1);
  ent_Clean(e2);
  ent_Clean(e3);

  rent = ent_Create();
  ent_data(rent) = w;
  ent_type(rent) = MATRIX_DENSE_REAL;
  return rent;
}

//
// ibwrt: synchronous write to a single device from file
//
Ent * ent_gpib_ibwrtf(int nargs, Datum args[])
{
  Ent *e1=0, *e2=0, *rent=0;
  MDR *w;
  char *s2=0;

  int iud, n, i, ibe, ilen;

  //
  if (nargs != 2)
    rerror("ibwrt: two arguments required!");

  //
  // ud
  //
  e1 = bltin_get_ent(args[0]);
  if (ent_type(e1) != MATRIX_DENSE_REAL)
    rerror("ibwrt: first argument has to be an integer!");
  iud = (int) class_double ( e1 );

  //
  // commands as a string vector
  //
  e2 = bltin_get_ent(args[1]);
  if (ent_type(e2) != MATRIX_DENSE_STRING)
    rerror("ibwrt: second argument has to be a string vector!");
  s2 = class_char_pointer ( e2 );

  w  = mdr_Create(1,1);

  ibwrtf(iud, s2);

  ibe = ThreadIberr();
  MdrV0(w,i) = ibe;

#ifdef DEBUG
  if (ibe)
  {
    fprintf(stderr, "ibwrt: error %s (0x%x) occured while reading\n", gpib_error_string(ibe), ibe);
    fprintf(stderr, "ibwrt: from file '%s' to device!\n", s2);
  }
#endif

  ent_Clean(e1);
  ent_Clean(e2);

  rent = ent_Create();
  ent_data(rent) = w;
  ent_type(rent) = MATRIX_DENSE_REAL;
  return rent;
}

//
// ibwrta: asynchronous write to a single device
//
Ent * ent_gpib_ibwrta(int nargs, Datum args[])
{
  Ent *e1=0, *e2=0, *rent=0;
  MDR *w;
  MDS *s2;

  int iud, n, i;

  //
  if (nargs != 2)
    rerror("ibwrt: two arguments required!");

  //
  // ud
  //
  e1 = bltin_get_ent(args[0]);
  if (ent_type(e1) != MATRIX_DENSE_REAL)
    rerror("ibwrt: first argument has to be an integer!");
  iud = (int) class_double ( e1 );

  //
  // commands as a string vector
  //
  e2 = bltin_get_ent(args[1]);
  if (ent_type(e2) != MATRIX_DENSE_STRING)
    rerror("ibwrta: second argument has to be a string vector!");
  s2 = ent_data ( e2 );

  if (MNR(s2)!=1 && MNC(s2)!=1)
    rerror("ibwrta: second argument has to be a string vector!");

  n  = MNR (s2) * MNC (s2);

  w  = mdr_Create(n,1);

  struct sigaction act;
  act.sa_handler = intHandler;
  sigaction(SIGINT, &act, NULL);

  //
  // write asynchonously and wait till completed
  //
  for (i=0; (i<n) && keepRunning; i++)
  {
    MdrV0(w,i) = ibwrta(iud, MdsV0(s2,i), strlen(MdsV0(s2,i)));
  }
  if (keepRunning)
    ibwait(iud, CMPL);
  else
  {
    keepRunning=1;
    ibclr(iud);
    ibonl(iud,1);
    ibsic(0);
    ibloc(iud);
    rerror("ibwrta: operation interrupted\n");
  }

  ent_Clean(e1);
  ent_Clean(e2);

  rent = ent_Create();
  ent_data(rent) = w;
  ent_type(rent) = MATRIX_DENSE_REAL;
  return rent;
}

Ent * ent_gpib_ibask(int nargs, Datum args[])
{
  Ent *e1=0, *e2=0, *rent=0;
  MDR *w;

  int iud, iop, ires, istatus;

  //
  if (nargs != 2)
    rerror("ibask: two arguments required!");

  //
  // first argument: ud
  //
  e1 = bltin_get_ent(args[0]);
  if (ent_type(e1) != MATRIX_DENSE_REAL)
    rerror("ibask: first argument has to be an integer!");
  iud = (int) class_double ( e1 );

  //
  // second argument: option
  //
  e2 = bltin_get_ent(args[1]);
  if (ent_type(e2) != MATRIX_DENSE_REAL)
    rerror("ibask: second argument has to be an integer!");
  iop = (int) class_double ( e2 );

  istatus = ibask(iud, iop, &ires);

  if (istatus & ERR)
    w = mdr_Create(0,0);
  else
    w = mdr_CreateScalar( (double) ires );

  ent_Clean(e1);
  ent_Clean(e2);

  rent = ent_Create();
  ent_data(rent) = w;
  ent_type(rent) = MATRIX_DENSE_REAL;

  return rent;
}

Ent * ent_gpib_ibwait(int nargs, Datum args[])
{
  Ent *e1=0, *e2=0, *rent=0;
  MDR *w;

  int iud, ism, istatus;

  //
  if (nargs != 2)
		rerror("ibwait: two arguments required!");

  //
  // first argument: ud
  //
  e1 = bltin_get_ent(args[0]);
  if (ent_type(e1) != MATRIX_DENSE_REAL)
		rerror("ibwait: first argument has to be an integer!");
  iud = (int) class_double ( e1 );

  //
  // second argument: option
  //
  e2 = bltin_get_ent(args[1]);
  if (ent_type(e2) != MATRIX_DENSE_REAL)
		rerror("ibwait: second argument has to be an integer!");
  ism = (int) class_double ( e2 );

  istatus = ibwait(iud, ism);

  if (istatus & ERR)
    w = mdr_Create(0,0);
  else
    w = mdr_CreateScalar( (double) istatus );

  ent_Clean(e1);
  ent_Clean(e2);

  rent = ent_Create();
  ent_data(rent) = w;
  ent_type(rent) = MATRIX_DENSE_REAL;

  return rent;
}

Ent * ent_gpib_ibtrg(int nargs, Datum args[])
{
  Ent *e1=0, *rent=0;
  MDR *x, *w;

  int k;

  //
  if (nargs != 1)
    rerror("ibtrg: one argument required!");

  //
  // first argument: ud
  //
  e1 = bltin_get_ent(args[0]);
  if (ent_type(e1) != MATRIX_DENSE_REAL)
    rerror("ibtrg: first argument has to be an integer vector!");
  x = ent_data( e1 );

  if (! (MNR(x) * MNC(x)) )
    rerror("ibtrg: first argument has to be an integer vector!");

  w = mdr_Create(MNR(x), MNC(x));

  if(x->type == RLAB_TYPE_INT32)
  {
    for (k = 0; k < MNR(x) * MNC(x); k++)
      MdrV0(w,k) = ibtrg( MdiV0(x,k) );
  }
  else
  {
    for (k = 0; k < MNR(x) * MNC(x); k++)
      MdrV0(w,k) = ibtrg((int) MdrV0(x,k));
  }

  ent_Clean(e1);

  rent = ent_Create();
  ent_data(rent) = w;
  ent_type(rent) = MATRIX_DENSE_REAL;
  return rent;
}

Ent * ent_gpib_ibrsp (int nargs, Datum args[])
{
  Ent *e1=0, *rent=0;
  MDR *x, *w;

  int k;
  char res;

  //
  if (nargs != 1)
    rerror("ibrsp: one argument required!");

  //
  // first argument: ud
  //
  e1 = bltin_get_ent(args[0]);
  if (ent_type(e1) != MATRIX_DENSE_REAL)
    rerror("ibrsp: first argument has to be an integer vector!");
  x = ent_data( e1 );

  if (x->nrow * MNC(x) == 0)
    rerror("ibrsp: first argument has to be an integer vector!");

  w = mdi_Create(x->nrow, MNC(x));

  if(x->type == RLAB_TYPE_INT32)
  {
    for (k = 0; k < MNR(x) * MNC(x); k++)
    {
      ibrsp( MdiV0(x,k), &res);
      MdiV0(w,k) = (int) res;
    }
  }
  else
  {
    for (k = 0; k < MNR(x) * MNC(x); k++)
    {
      ibrsp((int) MdrV0(x,k), &res);
      MdiV0(w,k) = (int) res;
    }
  }

  ent_Clean(e1);

  rent = ent_Create();
  ent_data(rent) = w;
  ent_type(rent) = MATRIX_DENSE_REAL;
  return rent;
}

Ent * ent_gpib_ibrsv (int nargs, Datum args[])
{
  Ent *e1=0, *e2=0, *rent=0;
  MDR *w;

  int iud, ism, istatus;

  //
  if (nargs != 2)
    rerror("ibrsv: one argument required!");

  //
  // first argument: ud
  //
  e1 = bltin_get_ent(args[0]);
  if (ent_type(e1) != MATRIX_DENSE_REAL)
    rerror("ibrsv: first argument has to be an integer vector!");
  iud = (int) class_double ( e1 );

  //
  // second argument: option
  //
  e2 = bltin_get_ent(args[1]);
  if (ent_type(e2) != MATRIX_DENSE_REAL)
    rerror("ibrsv: second argument has to be an integer!");
  ism = (int) class_double ( e2 );

  istatus = ibrsv(iud, ism);

  if (istatus & ERR)
    w = mdr_Create(0,0);
  else
    w = mdr_CreateScalar( (double) istatus );

  ent_Clean(e1);
  ent_Clean(e2);

  rent = ent_Create();
  ent_data(rent) = w;
  ent_type(rent) = MATRIX_DENSE_REAL;
  return rent;
}

// go to local mode
Ent * ent_gpib_ibloc (int nargs, Datum args[])
{
  Ent *e1=0, *rent=0;
  MDR *w;

  int iud, istatus;

  //
  if (nargs != 1)
    rerror("ibloc: one argument required!");

  //
  // first argument: ud
  //
  e1 = bltin_get_ent(args[0]);
  if (ent_type(e1) != MATRIX_DENSE_REAL)
    rerror("ibloc: first argument has to be an integer vector!");
  iud = (int) class_double ( e1 );

  istatus = ibloc(iud);

  if (istatus & ERR)
    w = mdr_Create(0,0);
  else
    w = mdr_CreateScalar( (double) istatus );

  ent_Clean(e1);

  rent = ent_Create();
  ent_data(rent) = w;
  ent_type(rent) = MATRIX_DENSE_REAL;
  return rent;
}


//
// EnableRemote
//
Ent * ent_gpib_EnableRemote(int nargs, Datum args[])
{
  Ent *e1=0, *e2=0, *rent=0;
  MDR *mdr_addressList = 0;
  Addr4882_t *addressList=0;
  MDR *w;

  int board_desc, n, i;

  //
  if (nargs != 2)
    rerror("EnableRemote: two arguments required!");

  //
  // board descriptor
  //
  e1 = bltin_get_ent(args[0]);
  if (ent_type(e1) != MATRIX_DENSE_REAL)
    rerror("EnableRemote: first argument has to be an integer!");
  board_desc = (int) class_double ( e1 );

  //
  // addresses array
  //
  e2 = bltin_get_ent(args[1]);
  if (ent_type(e2) != MATRIX_DENSE_REAL)
    rerror("EnableRemote: second argument has to be an array of GPIB addresses!");
  mdr_addressList = ent_data( e2 );

  n = mdr_addressList->nrow *  mdr_addressList->ncol;

  addressList = GC_malloc((n+1) * sizeof(Addr4882_t));
  addressList[n] = NOADDR;
  for (i=0; i<n; i++)
  {
    if (mdr_addressList->type == RLAB_TYPE_INT32)
      addressList[i] = MakeAddr(board_desc, MdiV0(mdr_addressList,i));
    else if (mdr_addressList->type == RLAB_TYPE_DOUBLE)
      addressList[i] = MakeAddr(board_desc, MdrV0(mdr_addressList,i));
    else
      rerror ("unknown data type!");
  }

  EnableRemote (board_desc, addressList);

  w  = mdr_CreateScalar(0);

  if (e1)
    ent_Clean(e1);
  if (e2)
    ent_Clean(e2);
  if(addressList)
    GC_free (addressList);

  rent = ent_Create();
  ent_data(rent) = w;
  ent_type(rent) = MATRIX_DENSE_REAL;

  return rent;
}

Ent *
ent_gpib_ibcnt(int nargs, Datum args[])
{
  MDR *w = mdr_CreateScalar(ibcnt);

  Ent *rent = ent_Create();
  ent_data(rent) = w;
  ent_type(rent) = MATRIX_DENSE_REAL;
  return rent;
}

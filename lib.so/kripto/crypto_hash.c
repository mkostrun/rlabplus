/* crypto_hash.c */

/* Source code for the RLaB hash function
 *
 * Syntax: hash ( hf, x ), where 'hf' is the hash function and 'x' is a string matrix
 *
 */

/*  This file is a part of RLaB ("Our"-LaB), released as a part
    of project rlabplus.
    Copyright (C) 2008-2009 Marijan Kostrun

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
 ***********************************************************************/

#include "rlab.h"
#include "symbol.h"
#include "mem.h"
#include "listnode.h"
#include "btree.h"
#include "bltin.h"
#include "bltin1.h"
#include "util.h"
#include "scan.h"
#include "print.h"
#include "class.h"
#include "mds.h"
#include "mdr.h"
#include "rfileio.h"
#include "mdc.h"
#include "mdr_mdc.h"
#include "mdcf1.h"
#include "mdcf2.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <time.h>
#include <pty.h>


//
// crc32 calculator for rlab2.
// uses snippets of code by Andrew Pociu, as posted on
// http://www.geekpedia.com/code113_Checksum-CRC32-Calculator.html
//

// The CRC value table
static unsigned int CRCTable[] = {
  0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419,
  0x706af48f, 0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4,
  0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07,
  0x90bf1d91, 0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
  0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7, 0x136c9856,
  0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
  0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4,
  0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
  0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3,
  0x45df5c75, 0xdcd60dcf, 0xabd13d59, 0x26d930ac, 0x51de003a,
  0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599,
  0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
  0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190,
  0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f,
  0x9fbfe4a5, 0xe8b8d433, 0x7807c9a2, 0x0f00f934, 0x9609a88e,
  0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
  0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed,
  0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
  0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3,
  0xfbd44c65, 0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
  0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a,
  0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5,
  0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa, 0xbe0b1010,
  0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
  0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17,
  0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6,
  0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615,
  0x73dc1683, 0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
  0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1, 0xf00f9344,
  0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
  0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a,
  0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
  0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1,
  0xa6bc5767, 0x3fb506dd, 0x48b2364b, 0xd80d2bda, 0xaf0a1b4c,
  0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef,
  0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
  0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe,
  0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31,
  0x2cd99e8b, 0x5bdeae1d, 0x9b64c2b0, 0xec63f226, 0x756aa39c,
  0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
  0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b,
  0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
  0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1,
  0x18b74777, 0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
  0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45, 0xa00ae278,
  0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7,
  0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc, 0x40df0b66,
  0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
  0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605,
  0xcdd70693, 0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8,
  0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b,
  0x2d02ef8d
};

void crc32_DigestUpdate(unsigned int *crcval, char *m, int lm)
{
  int i;
  unsigned int val;

  for (i = 0; i < lm; i++)
  {
    val = (unsigned int) m[i];
    (*crcval) = ((*crcval) >> 8) ^ CRCTable[((*crcval) & 0xff) ^ val];
  }
  (*crcval) ^= 0xffffffff; // Toggle operation
}


//
// rlabplus (C) Marijan Kostrun, 2009
//
Ent *
ent_crc32 (int nargs, Datum args[])
{
  Ent *e1=0, *e2=0, *e3=0, *rent=0;
  MDS *m=0;
  char *mynewline=0;
  char newline = '\n';
  int nr, nc, i, j;
  char hex_output[9] = {0};
  unsigned int  myseed, CRCVal = 0xffffffff;

  //
  // get the string matrix to use
  //
  e1 = bltin_get_ent (args[0]);
  if (ent_type(e1) != MATRIX_DENSE_STRING)
    rerror("crc32: incorrect first argument");
  m  = ent_data (e1);
  nr = MNR(m);
  nc = MNC(m);
  if (SIZE(m)<1)
    rerror("crc32: null pointer as data");

  //
  // get the newline for joining the rows of the matrix
  //
  if (nargs > 1)
  {
    e2 = bltin_get_ent (args[1]);
    if (ent_type(e2) != MATRIX_DENSE_STRING)
      rerror("crc32: incorrect second argument");
    mynewline = class_char_pointer (e2);
  }
  else
    mynewline = 0;

  //
  // use different seed than the default one?
  //
  if (nargs > 2)
  {
    e3 = bltin_get_ent (args[2]);
    if (ent_type(e3) != MATRIX_DENSE_REAL)
      rerror("crc32: incorrect third argument");
    myseed = (unsigned int) class_double (e3);
  }
  else
    myseed = CRCVal;

  // just do it
  for (i = 0; i < nr; i++)
  {
    // columns of the string matrix are concatenated
    // rows of the string matrix are appended \newline (0x0a), so
    // that output of a hash of a string matrix is equivalent as
    // if that matrix were written in a file

    // concatenate columns of a single row
    for (j = 0; j < nc; j++)
      crc32_DigestUpdate(&myseed, Mds0(m,i,j), strlen(Mds0(m,i,j)));

    // add a \newline (0x0a, or '\n') between the rows but not after the last one
    if (i<nr-1)
    {
      if (mynewline)
        crc32_DigestUpdate(&myseed, mynewline, strlen(mynewline)) ;
      else
        crc32_DigestUpdate(&myseed, &newline, 1) ;
    }
  } // next i (row)

  // write things up
  sprintf(hex_output, "%08x", myseed);

  // clean up
  ent_Clean (e1);
  ent_Clean (e2);
  ent_Clean (e3);

  rent = ent_Create ();
  ent_data (rent) = mds_CreateScalar ( hex_output );
  ent_type (rent) = MATRIX_DENSE_STRING;
  return rent;
}

#ifdef HAVE_CRYPTO_HASH

#include <openssl/evp.h>
static int load_ssl=0;

//
// rlabplus (C) Marijan Kostrun, 2005-2009
//
Ent *
ent_openssl_hash (int nargs, Datum args[])
{
  Ent *e1=0, *e2=0, *e3=0, *rent;
  MDS *m;
  int i, j, nr, nc;

  char *hash_name=0;
  char *mynewline=0;

  char newline = '\n';

  //char mes1[]="USB disk";

  // openssl's stuff
  EVP_MD_CTX *mdctx=0;
  EVP_MD *md=0;
  unsigned char md_value[EVP_MAX_MD_SIZE];
  unsigned int md_len;

  /* Check number of arguments */
  if (nargs != 2 && nargs != 3)
  { rerror ("hash: two arguments required"); }

  //
  // what hash function to use
  //
  e1 = bltin_get_ent (args[0]);
  if (ent_type(e1) != MATRIX_DENSE_STRING)
    rerror("hash: incorrect first argument");
  hash_name = class_char_pointer (e1);

  // load ssl the first time the function is called
  if(!load_ssl)
  {
    load_ssl=1;
    OpenSSL_add_all_digests ();
  }

  if (hash_name)
  {
    if (!strcmp (hash_name, "sha1"))
    {
      md = (EVP_MD *) EVP_sha1();
    }
    else if (!strcmp (hash_name, "ripemd160"))
    {
      md = (EVP_MD *) EVP_ripemd160();
    }
    else
    {
      md = (EVP_MD *) EVP_md5();
    }
  }
  else
    rerror("hash: null pointer as hash name");

  //
  // get the string matrix to use
  //
  e2 = bltin_get_ent (args[1]);
  if (ent_type(e2) != MATRIX_DENSE_STRING)
    rerror("hash: incorrect second argument");
  m  = ent_data (e2);
  if (SIZE(m)<1)
    rerror("hash: null pointer as data");
  nr = MNR(m);
  nc = MNC(m);

  if (nargs > 2)
  {
    e3 = bltin_get_ent (args[2]);
    if (ent_type(e3) != MATRIX_DENSE_STRING)
      rerror("hash: incorrect second argument");
    mynewline = class_char_pointer (e3);
  }
  else
    mynewline = 0;


  //
  // hashing openssl's way
  //
  mdctx = EVP_MD_CTX_create();          // create
  EVP_DigestInit_ex(mdctx, md, NULL);   // initialize

  for (i = 0; i < nr; i++)
  {
    // columns of the string matrix are concatenated
    // rows of the string matrix are appended \newline (0x0a), so
    // that output of a hash of a string matrix is equivalent as
    // if that matrix were written in a file

    // concatenate columns of a single row
    for (j = 0; j < nc; j++)
      EVP_DigestUpdate(mdctx, Mds0(m,i,j), strlen(Mds0(m,i,j)));

    // add a \newline (0x0a, or '\n') between the rows but not after the last one
    if (i<nr-1)
    {
      if (mynewline)
        EVP_DigestUpdate(mdctx, mynewline, strlen(mynewline)) ;
      else
        EVP_DigestUpdate(mdctx, &newline, 1) ;
    }

  } // next i (row)

  // find out the value of the hash and its length
  EVP_DigestFinal_ex(mdctx, md_value, &md_len);

  char * hex_output = GC_malloc((md_len*2+1)*sizeof(char));
  for (i=0; i<md_len; i++)
    sprintf(hex_output + i*2, "%02x", md_value[i]);

  // clean-up
  EVP_MD_CTX_destroy(mdctx);

  if (e1)
    if (ent_type(e1) != UNDEF)
      ent_Clean (e1);
  if (e2)
    if (ent_type(e2) != UNDEF)
      ent_Clean (e2);
  if (e3)
    if (ent_type(e3) != UNDEF)
      ent_Clean (e3);


  rent = ent_Create ();
  ent_data (rent) = mds_CreateScalar ( hex_output );
  ent_type (rent) = MATRIX_DENSE_STRING;
  return rent;
}

#endif    /* HAVE_CRYPTO_HASH */



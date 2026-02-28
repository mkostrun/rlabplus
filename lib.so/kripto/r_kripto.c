/* r_fann.c */

/*  This file is a part of RLaB2/rlabplus ("Our"-LaB)
   Copyright (C) 2019-2021 M. Kostrun

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



#include "r_kripto.h"

#undef  THIS_FILE
#define THIS_FILE "r_kripto.c"

//
// rlabplus (C) Marijan Kostrun, 2005-2009, 2023
//
Ent * ent_kripto_block (int nargs, Datum args[])
{
  Ent *e1=0, *e2=0, *e3=0, *e4=0, *e5=0, *e6=0, *rent;
  MD *m=0, *s=0, *n=0, *q=0, *c=0;
  int r, i, j, nr, nc, mymsg_len, msg_len=0, undo_s=0, undo_iv=0, undo_m=0, niter=1;

  unsigned char *stream_crypt=0, *iv=0, *msg=0, *newline=0, *key=0, c3[3],   *hex_output=0;

  int is_decrypt=0, mrpt=1, myiv_len=0, iv_len=0, newline_len=0, mykey_len=0, key_len=0, rounds;
  size_t maxivlen, maxkeylen, block_size;

  //
  // initialize kripto stream function
  //
  const kripto_desc_block *b;
  kripto_block *k=0;

  /* Check number of arguments */
  if (nargs < 5)
  {
    rerror ("block: five or six arguments required\n");
  }

  //
  // block cipher function
  //
  e1 = bltin_get_ent (args[0]);
  if (ent_type(e1) != MATRIX_DENSE_STRING)
    rerror("stream: incorrect first argument 'hash function'");
  stream_crypt = (unsigned char*) class_char_pointer (e1);
  if (isvalidstring(stream_crypt) < 1)
    rerror("stream: incorrect first argument 'hash function'");

  //
  // rounds -> decrypt: is_descrypt + rounds
  //           encrypt: -1 - rounds;
  //
  // 0-encrypt/1-decrypt
  //
  e2 = bltin_get_ent (args[1]);
  if (ent_type(e2) == MATRIX_DENSE_REAL)
  {
    is_decrypt = class_int (e2);
    if (is_decrypt > 0)
    {
      rounds = is_decrypt - 1;
      is_decrypt = 1;
    }
    else if (is_decrypt < 0)
    {
      rounds = -is_decrypt - 1;
      is_decrypt = 0;
    }

  }

  //
  // tweak
  //
  e3 = bltin_get_ent (args[2]);
  if ((ent_type(e3) == MATRIX_DENSE_STRING) || (ent_type(e3) == MATRIX_DENSE_REAL))
  {
    q = ent_data(e3);
  }
  if (q)
  {
    if (q->type == RLAB_TYPE_INT32)
    {
    // meed to recast int32's as chars
      undo_iv = 1;
      iv  = (unsigned char *) MDRPTR(q);
      iv_len = SIZE(q);
      for (i = 0; i < iv_len; i++)
      {
        iv[i] = (unsigned char) MdiV0(q,i);
      }
    }
    else if (q->type == RLAB_TYPE_STRING)
    {
      iv = MdsV0(q,0);
      iv_len = isvalidstring(iv);
      if (iv_len < 1)
      {
        iv = NULL;
        iv_len = 0;
      }
    }
  }

  //
  // data
  //
  e4 = bltin_get_ent (args[3]);
  if ((ent_type(e4) == MATRIX_DENSE_STRING) || (ent_type(e4) == MATRIX_DENSE_REAL))
  {
    m = ent_data(e4);
  }
  if (!m)
  {
    fprintf(stderr, THIS_FILE ": Input 'msg' is needed: Cannot continue!\n");
    goto _exit;
  }
  if (m->type == RLAB_TYPE_INT32)
  {
    // meed to recast int32's as chars
    msg = (unsigned char *) MDRPTR(m);
    msg_len = SIZE(m);
    undo_m = 1;
    for (i = 0; i < msg_len; i++)
    {
      msg[i] = (unsigned char) MdiV0(m,i);
    }
  }
  else if (m->type == RLAB_TYPE_STRING)
  {
    msg = MdsV0(s,0);
    msg_len = isvalidstring(msg);
    if (msg_len < 1)
    {
      msg = NULL;
      msg_len = 0;
    }
  }
  if (msg_len < 1)
  {
    fprintf(stderr, THIS_FILE ": Input 'msg' is needed: Cannot continue!\n");
    goto _exit;
  }

  //
  // key
  //
  e5 = bltin_get_ent (args[4]);
  if ((ent_type(e5) == MATRIX_DENSE_STRING) || (ent_type(e5) == MATRIX_DENSE_REAL))
  {
    s = ent_data(e5);
  }
  if (!s)
  {
    fprintf(stderr, THIS_FILE ": Input 'key' is needed: Cannot continue!\n");
    goto _exit;
  }
  if (s->type == RLAB_TYPE_INT32)
  {
      // meed to recast int32's as chars
    key = (unsigned char *) MDRPTR(s);
    key_len = SIZE(s);
    undo_s = 1;
    for (i = 0; i < key_len; i++)
    {
      key[i] = (unsigned char) MdiV0(s,i);
    }
  }
  else if (s->type == RLAB_TYPE_STRING)
  {
    key = MdsV0(s,0);
    key_len = isvalidstring(key);
    if (key_len < 1)
    {
      key = NULL;
      key_len = 0;
    }
  }
  if (key_len < 1)
  {
    fprintf(stderr, THIS_FILE ": Input 'key' is needed: Cannot continue!\n");
    goto _exit;
  }

  //
  // iterations
  //
  if (nargs > 5)
  {
    e6 = bltin_get_ent (args[5]);
    if (ent_type(e6) == MATRIX_DENSE_REAL)
    {
      niter = class_int (e6);
      if (niter < 1)
      {
        niter = 1;
      }
    }
  }

  //
  //
  //
  // select the block cypher:
  //
  //
  if (strstr(stream_crypt, "aes"))
  {
    b = kripto_block_aes;
  }
  else if(strstr(stream_crypt, "anubis"))
  {
    b = kripto_block_anubis;
  }
  else if(strstr(stream_crypt, "aria"))
  {
    b = kripto_block_aria;
  }
  else if(strstr(stream_crypt, "blowfish"))
  {
    b = kripto_block_blowfish;
  }
  else if(strstr(stream_crypt, "camellia"))
  {
    b = kripto_block_camellia;
  }
  else if(strstr(stream_crypt, "cast5"))
  {
    b = kripto_block_cast5;
  }
  else if(strstr(stream_crypt, "craxs"))
  {
    b = kripto_block_crax_s;
  }
  else if(strstr(stream_crypt, "des"))
  {
    b = kripto_block_des;
  }
  else if(strstr(stream_crypt, "idea"))
  {
    b = kripto_block_idea;
  }
  else if(strstr(stream_crypt, "khazad"))
  {
    b = kripto_block_khazad;
  }
  else if(strstr(stream_crypt, "lea"))
  {
    b = kripto_block_lea;
  }
  else if(strstr(stream_crypt, "noekeon"))
  {
    b = kripto_block_noekeon;
  }
  else if(strstr(stream_crypt, "rc2"))
  {
    b = kripto_block_rc2;
  }
  else if(strstr(stream_crypt, "rc5"))
  {
    b = kripto_block_rc5;
  }
  else if(strstr(stream_crypt, "rc6"))
  {
    b = kripto_block_rc6;
  }
  else if(strstr(stream_crypt, "rect"))
  {
    b = kripto_block_rectangle;
  }
  else if(strstr(stream_crypt, "rijndael256"))
  {
    b = kripto_block_rijndael256;
  }
  else if(strstr(stream_crypt, "rijndael128"))
  {
    b = kripto_block_rijndael128;
  }
  else if(strstr(stream_crypt, "saferpp"))
  {
    b = kripto_block_saferpp;
  }
  else if(strstr(stream_crypt, "safersk"))
  {
    b = kripto_block_safer_sk;
  }
  else if(strstr(stream_crypt, "safer"))
  {
    b = kripto_block_safer;
  }
  else if(strstr(stream_crypt, "seed"))
  {
    b = kripto_block_seed;
  }
  else if(strstr(stream_crypt, "serpent"))
  {
    b = kripto_block_serpent;
  }
  else if(strstr(stream_crypt, "shacal2"))
  {
    b = kripto_block_shacal2;
  }
  else if(strstr(stream_crypt, "simon32"))
  {
    b = kripto_block_simon32;
  }
  else if(strstr(stream_crypt, "simon64"))
  {
    b = kripto_block_simon64;
  }
  else if(strstr(stream_crypt, "simon128"))
  {
    b = kripto_block_simon128;
  }
  else if(strstr(stream_crypt, "skipjack"))
  {
    b = kripto_block_skipjack;
  }
  else if(strstr(stream_crypt, "sm4"))
  {
    b = kripto_block_sm4;
  }
  else if(strstr(stream_crypt, "speck32"))
  {
    b = kripto_block_speck32;
  }
  else if(strstr(stream_crypt, "speck64"))
  {
    b = kripto_block_speck64;
  }
  else if(strstr(stream_crypt, "speck128"))
  {
    b = kripto_block_speck128;
  }
  else if(strstr(stream_crypt, "tea"))
  {
    b = kripto_block_tea;
  }
  else if(strstr(stream_crypt, "threefish256"))
  {
    b = kripto_block_threefish256;
  }
  else if(strstr(stream_crypt, "threefish512"))
  {
    b = kripto_block_threefish512;
  }
  else if(strstr(stream_crypt, "threefish1024"))
  {
    b = kripto_block_threefish1024;
  }
  else if(strstr(stream_crypt, "traxl"))
  {
    b = kripto_block_trax_l;
  }
  else if(strstr(stream_crypt, "traxm"))
  {
    b = kripto_block_trax_m;
  }
  else if(strstr(stream_crypt, "twofish"))
  {
    b = kripto_block_twofish;
  }
  else if(strstr(stream_crypt, "xtea"))
  {
    b = kripto_block_xtea;
  }

  if (!b)
  {
    fprintf(stderr, THIS_FILE ": Failed to create 'stream' function: Cannot continue!\n");
    goto _exit;
  }

  //
  // create block cipher with key
  //
  maxkeylen = kripto_block_maxkey(b);
  mykey_len = key_len;
  if (mykey_len > maxkeylen)
  {
    fprintf(stderr, THIS_FILE ": %s: 'key' length %i truncated to %i\n",
            stream_crypt, mykey_len, maxkeylen);
    mykey_len = maxkeylen;
  }

  k = kripto_block_create(b, rounds, key, mykey_len);

  if (!k)
  {
    fprintf(stderr, THIS_FILE ": Failed to create stream function: Cannot continue!\n");
    goto _exit;
  }


  //
  // update tweak
  //
  maxivlen = kripto_block_maxtweak(b);
  myiv_len = iv_len;
  if (myiv_len > maxivlen)
  {
    fprintf(stderr, THIS_FILE ": %s: 'iv' length %i truncated to %i\n",
            stream_crypt, myiv_len, maxivlen);
    myiv_len = maxivlen;
  }
  if(myiv_len > 0)
  {
    kripto_block_tweak(k, iv, myiv_len);
  }

  //
  // Make sure the messge length is the same as block size
  //
  block_size = kripto_block_size(b);
  mymsg_len = msg_len;
  if (mymsg_len < block_size)
  {
    fprintf(stderr, THIS_FILE ": %s: 'msg' length %i is smaller than block size %i:"
        " Cannot continue!\n",
        stream_crypt, mymsg_len, block_size);
    goto _exit;
  }
  if ( (mymsg_len > block_size) )
  {
    mymsg_len = block_size;
    fprintf(stderr, THIS_FILE ": %s: 'msg' length %i was truncated to block size %i!\n",
        stream_crypt, msg_len, mymsg_len);
  }
  c = mdi_Create(1, mymsg_len);
  hex_output = (unsigned char *) MDRPTR(c);

  //now copy msg to hex_output:
  for (i=0; i<mymsg_len; i++)
  {
    hex_output[i] = msg[i];
  }

  if (is_decrypt)
  {
    for(i=0; i<niter; i++)
    {
      kripto_block_decrypt(k, hex_output, hex_output);
    }
  }
  else
  {
    for(i=0; i<niter; i++)
    {
      kripto_block_encrypt(k, hex_output, hex_output);
    }
  }

  // clean-up
  if (undo_iv)
  {
    for (i=myiv_len-1; i>=0; i--)
    {
      MdiV0(q,i) = (iv[i] & 0xff);
    }
  }
  if (undo_m)
  {
    for (i=mymsg_len-1; i>=0; i--)
    {
      MdiV0(m,i) = (msg[i] & 0xff);
      MdiV0(c,i) = (hex_output[i] & 0xff);
    }
  }
  if (undo_s)
  {
    for (i=mykey_len-1; i>=0; i--)
    {
      MdiV0(s,i) = (key[i] & 0xff);
    }
  }

  kripto_block_destroy(k);

_exit:

  ent_Clean (e1);
  ent_Clean (e2);
  ent_Clean (e3);
  ent_Clean (e4);
  ent_Clean (e5);
  ent_Clean (e6);

  return ent_Assign_Rlab_MDR(c);
}

Ent * ent_kripto_stream (int nargs, Datum args[])
{
  Ent *e1=0, *e2=0, *e3=0, *e4=0, *e5=0, *e6=0, *rent;
  MD *m=0, *s=0, *n=0, *q=0, *c=0;
  int r, i, j, nr, nc, mymsg_len, msg_len=0, undo_s=0, undo_iv=0, undo_m=0;

  unsigned char *stream_crypt=0, *iv=0, *msg=0, *newline=0, *key=0, c3[3],   *hex_output=0;

  int is_decrypt=0, mrpt=1, myiv_len=0, iv_len=0, newline_len=0, mykey_len=0, key_len=0, rounds;
  size_t maxivlen, maxkeylen, mulof;

  //
  // initialize kripto stream function
  //
  kripto_stream *k=0;
  kripto_desc_stream *d=0;
  const kripto_desc_block *b;

  /* Check number of arguments */
  if (nargs != 5)
  {
    rerror ("stream: five arguments required\n");
  }

  //
  // stream function
  //
  e1 = bltin_get_ent (args[0]);
  if (ent_type(e1) != MATRIX_DENSE_STRING)
    rerror("stream: incorrect first argument 'hash function'");
  stream_crypt = (unsigned char*) class_char_pointer (e1);
  if (isvalidstring(stream_crypt) < 1)
    rerror("stream: incorrect first argument 'hash function'");

  //
  // 0-encrypt/1-decrypt
  //
  e2 = bltin_get_ent (args[1]);
  if (ent_type(e2) == MATRIX_DENSE_REAL)
  {
    is_decrypt = class_int (e2);
    if (is_decrypt > 0)
    {
      rounds = is_decrypt;
      is_decrypt = 1;
    }
    else if (is_decrypt < 0)
    {
      rounds = -is_decrypt;
      is_decrypt = 0;
    }

  }

  //
  // iv
  //
  e3 = bltin_get_ent (args[2]);
  if ((ent_type(e3) == MATRIX_DENSE_STRING) || (ent_type(e3) == MATRIX_DENSE_REAL))
  {
    q = ent_data(e3);
  }
  if (!q)
  {
    fprintf(stderr, THIS_FILE "[1]: Input 'iv' is needed: Cannot continue!\n");
    goto _exit;
  }
  if (q->type == RLAB_TYPE_INT32)
  {
    // meed to recast int32's as chars
    undo_iv = 1;
    iv  = (unsigned char *) MDRPTR(q);
    iv_len = SIZE(q);
    for (i = 0; i < iv_len; i++)
    {
      iv[i] = (unsigned char) MdiV0(q,i);
    }
  }
  else if (q->type == RLAB_TYPE_STRING)
  {
    iv = MdsV0(q,0);
    iv_len = isvalidstring(iv);
    if (iv_len < 1)
    {
      iv = NULL;
      iv_len = 0;
    }
  }
  if (iv_len < 1)
  {
    fprintf(stderr, THIS_FILE "[2]: Input 'iv' is needed: Cannot continue!\n");
    goto _exit;
  }

  //
  // data
  //
  e4 = bltin_get_ent (args[3]);
  if ((ent_type(e4) == MATRIX_DENSE_STRING) || (ent_type(e4) == MATRIX_DENSE_REAL))
  {
    m = ent_data(e4);
  }
  if (!m)
  {
    fprintf(stderr, THIS_FILE ": Input 'msg' is needed: Cannot continue!\n");
    goto _exit;
  }
  if (m->type == RLAB_TYPE_INT32)
  {
      // meed to recast int32's as chars
    msg = (unsigned char *) MDRPTR(m);
    msg_len = SIZE(m);
    undo_m = 1;
    for (i = 0; i < msg_len; i++)
    {
      msg[i] = (unsigned char) MdiV0(m,i);
    }
  }
  else if (m->type == RLAB_TYPE_STRING)
  {
    msg = MdsV0(s,0);
    msg_len = isvalidstring(msg);
    if (msg_len < 1)
    {
      msg = NULL;
      msg_len = 0;
    }
  }
  if (msg_len < 1)
  {
    fprintf(stderr, THIS_FILE ": Input 'msg' is needed: Cannot continue!\n");
    goto _exit;
  }

  //
  // key
  //
  e5 = bltin_get_ent (args[4]);
  if ((ent_type(e5) == MATRIX_DENSE_STRING) || (ent_type(e5) == MATRIX_DENSE_REAL))
  {
    s = ent_data(e5);
  }
  if (!s)
  {
    fprintf(stderr, THIS_FILE ": Input 'key' is needed: Cannot continue!\n");
    goto _exit;
  }
  if (s->type == RLAB_TYPE_INT32)
  {
      // meed to recast int32's as chars
    key = (unsigned char *) MDRPTR(s);
    key_len = SIZE(s);
    undo_s = 1;
    for (i = 0; i < key_len; i++)
    {
      key[i] = (unsigned char) MdiV0(s,i);
    }
  }
  else if (s->type == RLAB_TYPE_STRING)
  {
    key = MdsV0(s,0);
    key_len = isvalidstring(key);
    if (key_len < 1)
    {
      key = NULL;
      key_len = 0;
    }
  }
  if (key_len < 1)
  {
    fprintf(stderr, THIS_FILE ": Input 'key' is needed: Cannot continue!\n");
    goto _exit;
  }

  //
  //
  //
  // select the cypher and block now:
  //
  //
  if (!strcmp(stream_crypt, "chacha"))
  {
    maxivlen = kripto_stream_maxiv(kripto_stream_chacha);
    myiv_len = iv_len;
    if (myiv_len > maxivlen)
    {
      fprintf(stderr, THIS_FILE ": %s: 'iv' length %i truncated to %i\n",
              stream_crypt, myiv_len, maxivlen);
      myiv_len = maxivlen;
    }
    maxkeylen = kripto_stream_maxkey(kripto_stream_chacha);
    mykey_len = key_len;
    if (mykey_len > maxkeylen)
    {
      fprintf(stderr, THIS_FILE ": %s: 'key' length %i truncated to %i\n",
              stream_crypt, mykey_len, maxkeylen);
      mykey_len = maxkeylen;
    }
    k = kripto_stream_create(kripto_stream_chacha, rounds, key, mykey_len, iv, myiv_len);
  }
  else if (!strcmp(stream_crypt, "keccak800"))
  {
    maxivlen = kripto_stream_maxiv(kripto_stream_keccak800);
    myiv_len = iv_len;
    if (myiv_len > maxivlen)
    {
      fprintf(stderr, THIS_FILE ": %s: 'iv' length %i truncated to %i\n",
              stream_crypt, myiv_len, maxivlen);
      myiv_len = maxivlen;
    }
    maxkeylen = kripto_stream_maxkey(kripto_stream_keccak800);
    mykey_len = key_len;
    if (mykey_len > maxkeylen)
    {
      fprintf(stderr, THIS_FILE ": %s: 'key' length %i truncated to %i\n",
              stream_crypt, mykey_len, maxkeylen);
      mykey_len = maxkeylen;
    }
    k = kripto_stream_create(kripto_stream_keccak800, rounds, key, mykey_len, iv, myiv_len);
  }
  else if (!strcmp(stream_crypt, "keccak1600"))
  {
    maxivlen = kripto_stream_maxiv(kripto_stream_keccak1600);
    myiv_len = iv_len;
    if (myiv_len > maxivlen)
    {
      fprintf(stderr, THIS_FILE ": %s: 'iv' length %i truncated to %i\n",
              stream_crypt, myiv_len, maxivlen);
      myiv_len = maxivlen;
    }
    maxkeylen = kripto_stream_maxkey(kripto_stream_keccak1600);
    mykey_len = key_len;
    if (mykey_len > maxkeylen)
    {
      fprintf(stderr, THIS_FILE ": %s: 'key' length %i truncated to %i\n",
              stream_crypt, mykey_len, maxkeylen);
      mykey_len = maxkeylen;
    }
    k = kripto_stream_create(kripto_stream_keccak1600, rounds, key, mykey_len, iv, myiv_len);
  }
  else if (!strcmp(stream_crypt, "rc4"))
  {
    maxivlen = kripto_stream_maxiv(kripto_stream_rc4);
    myiv_len = iv_len;
    if (myiv_len > maxivlen)
    {
      fprintf(stderr, THIS_FILE ": %s: 'iv' length %i truncated to %i\n",
              stream_crypt, myiv_len, maxivlen);
      myiv_len = maxivlen;
    }
    maxkeylen = kripto_stream_maxkey(kripto_stream_rc4);
    mykey_len = key_len;
    if (mykey_len > maxkeylen)
    {
      fprintf(stderr, THIS_FILE ": %s: 'key' length %i truncated to %i\n",
              stream_crypt, mykey_len, maxkeylen);
      mykey_len = maxkeylen;
    }
    k = kripto_stream_create(kripto_stream_rc4, rounds, key, mykey_len, iv, myiv_len);
  }
  else if (!strcmp(stream_crypt, "salsa20"))
  {
    maxivlen = kripto_stream_maxiv(kripto_stream_salsa20);
    myiv_len = iv_len;
    if (myiv_len > maxivlen)
    {
      fprintf(stderr, THIS_FILE ": %s: 'iv' length %i truncated to %i\n",
              stream_crypt, myiv_len, maxivlen);
      myiv_len = maxivlen;
    }
    maxkeylen = kripto_stream_maxkey(kripto_stream_salsa20);
    mykey_len = key_len;
    if (mykey_len > maxkeylen)
    {
      fprintf(stderr, THIS_FILE ": %s: 'key' length %i truncated to %i\n",
              stream_crypt, mykey_len, maxkeylen);
      mykey_len = maxkeylen;
    }
    k = kripto_stream_create(kripto_stream_salsa20, rounds, key, mykey_len, iv, myiv_len);
  }
  else if (!strcmp(stream_crypt, "skein256"))
  {
    maxivlen = kripto_stream_maxiv(kripto_stream_skein256);
    myiv_len = iv_len;
    if (myiv_len > maxivlen)
    {
      fprintf(stderr, THIS_FILE ": %s: 'iv' length %i truncated to %i\n",
              stream_crypt, myiv_len, maxivlen);
      myiv_len = maxivlen;
    }
    maxkeylen = kripto_stream_maxkey(kripto_stream_skein256);
    mykey_len = key_len;
    if (mykey_len > maxkeylen)
    {
      fprintf(stderr, THIS_FILE ": %s: 'key' length %i truncated to %i\n",
              stream_crypt, mykey_len, maxkeylen);
      mykey_len = maxkeylen;
    }
    k = kripto_stream_create(kripto_stream_skein256, rounds, key, mykey_len, iv, myiv_len);
  }
  else if (!strcmp(stream_crypt, "skein512"))
  {
    maxivlen = kripto_stream_maxiv(kripto_stream_skein512);
    myiv_len = iv_len;
    if (myiv_len > maxivlen)
    {
      fprintf(stderr, THIS_FILE ": %s: 'iv' length %i truncated to %i\n",
              stream_crypt, myiv_len, maxivlen);
      myiv_len = maxivlen;
    }
    maxkeylen = kripto_stream_maxkey(kripto_stream_skein512);
    mykey_len = key_len;
    if (mykey_len > maxkeylen)
    {
      fprintf(stderr, THIS_FILE ": %s: 'key' length %i truncated to %i\n",
              stream_crypt, mykey_len, maxkeylen);
      mykey_len = maxkeylen;
    }
    k = kripto_stream_create(kripto_stream_skein512, rounds, key, mykey_len, iv, myiv_len);
  }
  else if (!strcmp(stream_crypt, "skein1024"))
  {
    maxivlen = kripto_stream_maxiv(kripto_stream_skein1024);
    myiv_len = iv_len;
    if (myiv_len > maxivlen)
    {
      fprintf(stderr, THIS_FILE ": %s: 'iv' length %i truncated to %i\n",
              stream_crypt, myiv_len, maxivlen);
      myiv_len = maxivlen;
    }
    maxkeylen = kripto_stream_maxkey(kripto_stream_skein1024);
    mykey_len = key_len;
    if (mykey_len > maxkeylen)
    {
      fprintf(stderr, THIS_FILE ": %s: 'key' length %i truncated to %i\n",
              stream_crypt, mykey_len, maxkeylen);
      mykey_len = maxkeylen;
    }
    k = kripto_stream_create(kripto_stream_skein1024, rounds, key, mykey_len, iv, myiv_len);
  }
  else
  {
    // get block description
    if (strstr(stream_crypt, "aes"))
    {
      b = kripto_block_aes;
    }
    else if(strstr(stream_crypt, "anubis"))
    {
      b = kripto_block_anubis;
    }
    else if(strstr(stream_crypt, "aria"))
    {
      b = kripto_block_aria;
    }
    else if(strstr(stream_crypt, "blowfish"))
    {
      b = kripto_block_blowfish;
    }
    else if(strstr(stream_crypt, "camellia"))
    {
      b = kripto_block_camellia;
    }
    else if(strstr(stream_crypt, "cast5"))
    {
      b = kripto_block_cast5;
    }
    else if(strstr(stream_crypt, "craxs"))
    {
      b = kripto_block_crax_s;
    }
    else if(strstr(stream_crypt, "des"))
    {
      b = kripto_block_des;
    }
    else if(strstr(stream_crypt, "idea"))
    {
      b = kripto_block_idea;
    }
    else if(strstr(stream_crypt, "khazad"))
    {
      b = kripto_block_khazad;
    }
    else if(strstr(stream_crypt, "lea"))
    {
      b = kripto_block_lea;
    }
    else if(strstr(stream_crypt, "noekeon"))
    {
      b = kripto_block_noekeon;
    }
    else if(strstr(stream_crypt, "rc2"))
    {
      b = kripto_block_rc2;
    }
    else if(strstr(stream_crypt, "rc5"))
    {
      b = kripto_block_rc5;
    }
    else if(strstr(stream_crypt, "rc6"))
    {
      b = kripto_block_rc6;
    }
    else if(strstr(stream_crypt, "rect"))
    {
      b = kripto_block_rectangle;
    }
    else if(strstr(stream_crypt, "rijndael256"))
    {
      b = kripto_block_rijndael256;
    }
    else if(strstr(stream_crypt, "rijndael128"))
    {
      b = kripto_block_rijndael128;
    }
    else if(strstr(stream_crypt, "saferpp"))
    {
      b = kripto_block_saferpp;
    }
    else if(strstr(stream_crypt, "safersk"))
    {
      b = kripto_block_safer_sk;
    }
    else if(strstr(stream_crypt, "safer"))
    {
      b = kripto_block_safer;
    }
    else if(strstr(stream_crypt, "seed"))
    {
      b = kripto_block_seed;
    }
    else if(strstr(stream_crypt, "serpent"))
    {
      b = kripto_block_serpent;
    }
    else if(strstr(stream_crypt, "shacal2"))
    {
      b = kripto_block_shacal2;
    }
    else if(strstr(stream_crypt, "simon32"))
    {
      b = kripto_block_simon32;
    }
    else if(strstr(stream_crypt, "simon64"))
    {
      b = kripto_block_simon64;
    }
    else if(strstr(stream_crypt, "simon128"))
    {
      b = kripto_block_simon128;
    }
    else if(strstr(stream_crypt, "skipjack"))
    {
      b = kripto_block_skipjack;
    }
    else if(strstr(stream_crypt, "sm4"))
    {
      b = kripto_block_sm4;
    }
    else if(strstr(stream_crypt, "speck32"))
    {
      b = kripto_block_speck32;
    }
    else if(strstr(stream_crypt, "speck64"))
    {
      b = kripto_block_speck64;
    }
    else if(strstr(stream_crypt, "speck128"))
    {
      b = kripto_block_speck128;
    }
    else if(strstr(stream_crypt, "tea"))
    {
      b = kripto_block_tea;
    }
    else if(strstr(stream_crypt, "threefish256"))
    {
      b = kripto_block_threefish256;
    }
    else if(strstr(stream_crypt, "threefish512"))
    {
      b = kripto_block_threefish512;
    }
    else if(strstr(stream_crypt, "threefish1024"))
    {
      b = kripto_block_threefish1024;
    }
    else if(strstr(stream_crypt, "traxl"))
    {
      b = kripto_block_trax_l;
    }
    else if(strstr(stream_crypt, "traxm"))
    {
      b = kripto_block_trax_m;
    }
    else if(strstr(stream_crypt, "twofish"))
    {
      b = kripto_block_twofish;
    }
    else if(strstr(stream_crypt, "xtea"))
    {
      b = kripto_block_xtea;
    }

    if (!b)
    {
      fprintf(stderr, THIS_FILE ": Failed to create 'stream' function: Cannot continue!\n");
      goto _exit;
    }

    // get stream description
    if (strstr(stream_crypt, "cbc"))
    {
      d = kripto_stream_cbc(b);
    }
    else if (strstr(stream_crypt, "cfb"))
    {
      d = kripto_stream_cfb(b);
    }
    else if (strstr(stream_crypt, "ecb"))
    {
      d = kripto_stream_ecb(b);
    }
    else if (strstr(stream_crypt, "ofb"))
    {
      d = kripto_stream_ofb(b);
    }

    maxivlen = kripto_stream_maxiv(d);
    myiv_len = iv_len;
    if (myiv_len > maxivlen)
    {
      fprintf(stderr, THIS_FILE ": %s: 'iv' length %i truncated to %i\n",
              stream_crypt, myiv_len, maxivlen);
      myiv_len = maxivlen;
    }
    maxkeylen = kripto_stream_maxkey(d);
    mykey_len = key_len;
    if (mykey_len > maxkeylen)
    {
      fprintf(stderr, THIS_FILE ": %s: 'key' length %i truncated to %i\n",
              stream_crypt, mykey_len, maxkeylen);
      mykey_len = maxkeylen;
    }
    k = kripto_stream_create(d, rounds, key, mykey_len, iv, myiv_len);
  }

  if (!k)
  {
    fprintf(stderr, THIS_FILE ": Failed to create stream function: Cannot continue!\n");
    goto _exit;
  }

  mulof = kripto_stream_multof(k);
  if (msg_len < mulof)
  {
    fprintf(stderr, THIS_FILE ": %s: 'msg' length %i is not multiple of %i: Cannot continue!\n",
            stream_crypt, msg_len, mulof);
    goto _exit;
  }
  mymsg_len = msg_len;
  if ( (mymsg_len % mulof) )
  {
    mymsg_len -= (mymsg_len % mulof);
    fprintf(stderr, THIS_FILE ": %s: 'msg' length %i is not multiple of %i:"
        " The message was truncated to !\n",
        stream_crypt, msg_len, mymsg_len);
    goto _exit;
  }

  c = mdi_Create(1, mymsg_len);
  hex_output = (unsigned char *) MDRPTR(c);

  if (is_decrypt)
  {
    kripto_stream_decrypt(k, msg, hex_output, mymsg_len);
  }
  else
  {
    kripto_stream_encrypt(k, msg, hex_output, mymsg_len);
  }

  // clean-up
  if (undo_iv)
  {
    for (i=iv_len-1; i>=0; i--)
    {
      MdiV0(q,i) = (iv[i] & 0xff);
    }
  }
  if (undo_m)
  {
    for (i=msg_len-1; i>=0; i--)
    {
      MdiV0(m,i) = (msg[i] & 0xff);
      MdiV0(c,i) = (hex_output[i] & 0xff);
    }
  }
  if (undo_s)
  {
    for (i=key_len-1; i>=0; i--)
    {
      MdiV0(s,i) = (key[i] & 0xff);
    }
  }

  kripto_stream_destroy(k);

_exit:

  ent_Clean (e1);
  ent_Clean (e2);
  ent_Clean (e3);
  ent_Clean (e4);
  ent_Clean (e5);

  return ent_Assign_Rlab_MDR(c);
}


Ent * ent_kripto_mac (int nargs, Datum args[])
{
  Ent *e1=0, *e2=0, *e3=0, *e4=0, *e5=0, *e6=0, *rent;
  MD *m=0, *s=0, *n=0;
  int r, i, j, nr, nc, mlen=0, undo_n=0, undo_s=0;

  unsigned char *stream_crypt=0, *newline=0, *key=0, c3[3], *hash_name=0, *hex_output=0;

  int hash_len=0, mrpt=1, newline_len=0, key_len=0;

  /* Check number of arguments */
  if (nargs != 6)
  {
    rerror ("hash: two arguments required");
  }

  //
  // hash function
  //
  e1 = bltin_get_ent (args[0]);
  if (ent_type(e1) != MATRIX_DENSE_STRING)
    rerror("hash: incorrect first argument 'hash function'");
  hash_name = (unsigned char*) class_char_pointer (e1);
  if (isvalidstring(hash_name) < 1)
    rerror("hash: incorrect first argument 'hash function'");

  //
  // hash length
  //
  e2 = bltin_get_ent (args[1]);
  if (ent_type(e2) == MATRIX_DENSE_REAL)
  {
    hash_len = class_int (e2);
  }

  //
  // data
  //
  e3 = bltin_get_ent (args[2]);
  if ((ent_type(e3) != MATRIX_DENSE_STRING) && (ent_type(e3) != MATRIX_DENSE_REAL))
  {
    rerror("hash: incorrect third argument 'data'");
  }
  m  = ent_data (e3);
  if (SIZE(m)<1)
    rerror("hash: 'data' has to be valid string");
  nr = MNR(m);
  nc = MNC(m);

  //
  // message repeat
  //
  e4 = bltin_get_ent (args[3]);
  if (ent_type(e4) == MATRIX_DENSE_REAL)
  {
    mrpt = class_int (e4);
    if (mrpt < 1)
    {
      mrpt = 1;
    }
  }

  //
  // salt
  //
  e5 = bltin_get_ent (args[4]);
  if ((ent_type(e5) == MATRIX_DENSE_STRING) || (ent_type(e5) == MATRIX_DENSE_REAL))
  {
    s = ent_data(e5);
  }

  //
  // newline
  //
  e6 = bltin_get_ent (args[5]);
  if ((ent_type(e6) == MATRIX_DENSE_STRING) || (ent_type(e6) == MATRIX_DENSE_REAL))
  {
    n = ent_data(e6);
    if (SIZE(n) < 1)
    {
      n = NULL;
    }
  }

  //
  // initialize key array
  //
  if (!s)
  {
    fprintf(stderr, THIS_FILE ": Input 'key' is needed: Cannot continue!\n");
    goto _exit;
  }

  if (s->type == RLAB_TYPE_INT32)
  {
      // meed to recast int32's as chars
    key = (unsigned char *) MDRPTR(s);
    key_len = SIZE(s);
    for (i = 0; i < key_len; i++)
    {
      key[i] = (unsigned char) MdiV0(s,i);
    }
  }
  else if (s->type == RLAB_TYPE_STRING)
  {
    key = MdsV0(s,0);
    key_len = isvalidstring(key);
    if (key_len < 1)
    {
      key = NULL;
      key_len = 0;
    }
  }

  if (key_len < 1)
  {
    fprintf(stderr, THIS_FILE ": Input 'key' is needed: Cannot continue!\n");
    goto _exit;
  }

  //
  // initialize kripto mac function
  //
  kripto_desc_mac *d=0;
  kripto_mac *k=0;

  if (!strcmp(hash_name, "sha512"))
  {
    // sha512, sha384
    if ((hash_len == 64) || (hash_len == 48))
    {
      d = kripto_mac_hmac(kripto_hash_sha2_512);
    }
    else if ((hash_len == 32) || (hash_len == 28))
    {
      d = kripto_mac_hmac(kripto_hash_sha2_256);
    }
  }
  else if (!strcmp(hash_name, "sha1"))
  {
    hash_len = 20;
    d = kripto_mac_hmac(kripto_hash_sha1);
  }
  else if (!strcmp(hash_name, "md5"))
  {
    hash_len = 16;
    d = kripto_mac_hmac(kripto_hash_md5);
  }
  else if (!strcmp(hash_name, "sha3"))
  {
    d = kripto_mac_hmac(kripto_hash_sha3);
  }
  else if (!strcmp(hash_name, "keccak1600"))
  {
    if (hash_len == 64 || hash_len == 48)
    {
      d = kripto_mac_hmac(kripto_hash_keccak1600);
    }
    else if (hash_len == 32 || hash_len == 28)
    {
      d = kripto_mac_hmac(kripto_hash_keccak800);
    }
  }
  else if (!strcmp(hash_name, "skein"))
  {
    if (hash_len == 128)
    {
      d = kripto_mac_hmac(kripto_hash_skein1024);
    }
    else if (hash_len == 64)
    {
      d = kripto_mac_hmac(kripto_hash_skein512);
    }
    else if (hash_len == 32)
    {
      d = kripto_mac_hmac(kripto_hash_skein256);
    }
  }
  else if (!strcmp(hash_name, "whirlpool"))
  {
    d = kripto_mac_hmac(kripto_hash_whirlpool);
  }
  else if (!strcmp(hash_name, "tiger"))
  {
    d = kripto_mac_hmac(kripto_hash_tiger);
  }
  else if (!strcmp(hash_name, "blake1"))
  {
    if (hash_len == 64)
    {
      d = kripto_mac_hmac(kripto_hash_blake512);
    }
    else if (hash_len == 32)
    {
      d = kripto_mac_hmac(kripto_hash_blake256);
    }
  }
  else if (!strcmp(hash_name, "blake2"))
  {
    if (hash_len == 64)
    {
      d = kripto_mac_hmac(kripto_hash_blake2b);
    }
    else if (hash_len == 32)
    {
      d = kripto_mac_hmac(kripto_hash_blake2s);
    }
  }
  k = kripto_mac_create(d, 0, key, key_len, hash_len);


  //
  //
  // check that input is consistent with the hash function
  //
  //
  if (!k)
  {
    fprintf(stderr, THIS_FILE ": Failed to create mac function!\n");
    goto _exit;
  }

  for (i = 0; i < nr; i++)
  {
    // columns of the string matrix are concatenated
    // rows of the string matrix are appended \newline (0x0a), so
    // that output of a hash of a string matrix is equivalent as
    // if that matrix were written in a file

    // concatenate columns of a single row
    for (j = 0; j< nc; j++)
    {
      mlen = isvalidstring(Mds0(m,i,j));
      if (mlen > 0)
      {
        kripto_mac_input(k, Mds0(m,i,j), mlen);
      }
    }

    // add a \newline (0x0a, or '\n') between the rows but not after the last one
    if (i<nr-1)
    {
      if (newline_len>0)
        kripto_mac_input(k, newline, newline_len);
    }

  } // next i (row)

  hex_output = (char *) GC_malloc((2*hash_len+1)*sizeof(char));

  kripto_mac_tag(k, hex_output, hash_len);

  // expand digest from uint8_t to \xNN
  for (i=hash_len-1; i>=0; i--)
  {
    sprintf(c3, "%02x", hex_output[i]);
    hex_output[2*i] = c3[0];
    hex_output[2*i+1] = c3[1];
  }
  hex_output[2*hash_len] = 0;

  // clean-up
  if (undo_n)
  {
    for (i=newline_len-1; i>=0; i--)
    {
      MdiV0(n,i) = (newline[i] & 0xff);
    }
  }
  kripto_mac_destroy(k);

_exit:

  ent_Clean (e1);
  ent_Clean (e2);
  ent_Clean (e3);
  ent_Clean (e4);
  ent_Clean (e5);
  ent_Clean (e6);

  rent = ent_Create ();
  ent_data (rent) = mds_CreateScalar ( hex_output );
  ent_type (rent) = MATRIX_DENSE_STRING;
  return rent;
}

Ent * ent_kripto_hash (int nargs, Datum args[])
{
  Ent *e1=0, *e2=0, *e3=0, *e4=0, *e5=0, *e6=0, *rent;
  MD *m=0, *s=0, *n=0;
  int i, j, nr, nc, mlen=0, undo_n=0, undo_s=0;

  unsigned char *hash_name=0, *newline=0, *salt=0, c3[3],   *hex_output=0;

  int hash_len=0, mrpt=1, newline_len=0, salt_len=0;

  /* Check number of arguments */
  if (nargs != 6)
  {
    rerror ("hash: two arguments required");
  }

  //
  // hash function
  //
  e1 = bltin_get_ent (args[0]);
  if (ent_type(e1) != MATRIX_DENSE_STRING)
    rerror("hash: incorrect first argument 'hash function'");
  hash_name = (unsigned char*) class_char_pointer (e1);
  if (isvalidstring(hash_name) < 1)
    rerror("hash: incorrect first argument 'hash function'");

  //
  // hash length
  //
  e2 = bltin_get_ent (args[1]);
  if (ent_type(e2) == MATRIX_DENSE_REAL)
  {
    hash_len = class_int (e2);
  }

  //
  // data
  //
  e3 = bltin_get_ent (args[2]);
  if ((ent_type(e3) != MATRIX_DENSE_STRING) && (ent_type(e3) != MATRIX_DENSE_REAL))
  {
    rerror("hash: incorrect third argument 'data'");
  }
  m  = ent_data (e3);
  if (SIZE(m)<1)
    rerror("hash: 'data' has to be valid string");
  nr = MNR(m);
  nc = MNC(m);

  //
  // message repeat
  //
  e4 = bltin_get_ent (args[3]);
  if (ent_type(e4) == MATRIX_DENSE_REAL)
  {
    mrpt = class_int (e4);
    if (mrpt < 1)
    {
      mrpt = 1;
    }
  }

  //
  // salt
  //
  e5 = bltin_get_ent (args[4]);
  if ((ent_type(e5) == MATRIX_DENSE_STRING) || (ent_type(e5) == MATRIX_DENSE_REAL))
  {
    s = ent_data(e5);
  }

  //
  // newline
  //
  e6 = bltin_get_ent (args[5]);
  if ((ent_type(e6) == MATRIX_DENSE_STRING) || (ent_type(e6) == MATRIX_DENSE_REAL))
  {
    n = ent_data(e6);
    if (SIZE(n) < 1)
    {
      n = NULL;
    }
  }

  //
  // initialize salt array
  //
  if (s)
  {
    if (s->type == RLAB_TYPE_INT32)
    {
      // meed to recast int32's as chars
      salt = (unsigned char *) MDRPTR(s);
      salt_len = SIZE(s);
      for (i = 0; i < salt_len; i++)
      {
        salt[i] = (unsigned char) MdiV0(s,i);
      }
    }
    else if (s->type == RLAB_TYPE_STRING)
    {
      salt = MdsV0(s,0);
      salt_len = isvalidstring(salt);
      if (salt_len < 1)
      {
        salt = NULL;
        salt_len = 0;
      }
    }
  }

  //
  // initialize newline array
  //
  if (n)
  {
    if (n->type == RLAB_TYPE_INT32)
    {
      // meed to recast int32's as chars
      undo_n = 1;
      newline = (unsigned char *) MDRPTR(n);
      newline_len = SIZE(n);
      for (i = 0; i < newline_len; i++)
      {
        newline[i] = (unsigned char) MdiV0(n,i);
      }
    }
    else if (n->type == RLAB_TYPE_STRING)
    {
      newline = MdsV0(n,0);
      newline_len = isvalidstring(newline);
      if (newline_len < 1)
      {
        newline = NULL;
        newline_len = 0;
      }
    }
  }


  //
  // initialize kripto hash function
  //
  kripto_hash *k=0;

  if (!strcmp(hash_name, "sha512"))
  {
    // sha512, sha384
    if ((hash_len == 64) || (hash_len == 48))
    {
      k = kripto_hash_create(kripto_hash_sha2_512, 0, salt, salt_len, hash_len);
    }
    else if ((hash_len == 32) || (hash_len == 28))
    {
      k = kripto_hash_create(kripto_hash_sha2_256, 0, salt, salt_len, hash_len);
    }
  }
  else if (!strcmp(hash_name, "sha1"))
  {
    hash_len = 20;
    k = kripto_hash_create(kripto_hash_sha1, 0, salt, salt_len, hash_len);
  }
  else if (!strcmp(hash_name, "md5"))
  {
    hash_len = 16;
    k = kripto_hash_create(kripto_hash_md5, 0, salt, salt_len, hash_len);
  }
  else if (!strcmp(hash_name, "sha3"))
  {
    k = kripto_hash_create(kripto_hash_sha3, 0, salt, salt_len, hash_len);
  }
  else if (!strcmp(hash_name, "keccak1600"))
  {
    if (hash_len == 64 || hash_len == 48)
    {
      k = kripto_hash_create(kripto_hash_keccak1600, 0, salt, salt_len, hash_len);
    }
    else if (hash_len == 32 || hash_len == 28)
    {
      k = kripto_hash_create(kripto_hash_keccak800, 0, salt, salt_len, hash_len);
    }
  }
  else if (!strcmp(hash_name, "skein"))
  {
    if (hash_len == 128)
    {
      k = kripto_hash_create(kripto_hash_skein1024, 0, salt, salt_len, hash_len);
    }
    else if (hash_len == 64)
    {
      k = kripto_hash_create(kripto_hash_skein512, 0, salt, salt_len, hash_len);
    }
    else if (hash_len == 32)
    {
      k = kripto_hash_create(kripto_hash_skein256, 0, salt, salt_len, hash_len);
    }
  }
  else if (!strcmp(hash_name, "whirlpool"))
  {
    k = kripto_hash_create(kripto_hash_whirlpool, 0, salt, salt_len, hash_len);
  }
  else if (!strcmp(hash_name, "tiger"))
  {
    k = kripto_hash_create(kripto_hash_tiger, 0, salt, salt_len, hash_len);
  }
  else if (!strcmp(hash_name, "blake1"))
  {
    if (hash_len == 64)
    {
      k = kripto_hash_create(kripto_hash_blake512, 0, salt, salt_len, hash_len);
    }
    else if (hash_len == 32)
    {
      k = kripto_hash_create(kripto_hash_blake256, 0, salt, salt_len, hash_len);
    }
  }
  else if (!strcmp(hash_name, "blake2"))
  {
    if (hash_len == 64)
    {
      k = kripto_hash_create(kripto_hash_blake2b, 0, salt, salt_len, hash_len);
    }
    else if (hash_len == 32)
    {
      k = kripto_hash_create(kripto_hash_blake2s, 0, salt, salt_len, hash_len);
    }
  }

  //
  //
  // check that input is consistent with the hash function
  //
  //
  if (!k)
  {
    fprintf(stderr, THIS_FILE ": Failed to create hash function!\n");
    goto _exit;
  }
  if (kripto_hash_maxsalt((const kripto_desc_hash *) k) < salt_len)
  {
    fprintf(stderr, THIS_FILE ": Provided 'salt' exceeds the capacity of the hash function!\n");
    goto _exit;
  }

  for (i = 0; i < nr; i++)
  {
    // columns of the string matrix are concatenated
    // rows of the string matrix are appended \newline (0x0a), so
    // that output of a hash of a string matrix is equivalent as
    // if that matrix were written in a file

    // concatenate columns of a single row
    for (j = 0; j< nc; j++)
    {
      mlen = isvalidstring(Mds0(m,i,j));
      if (mlen > 0)
      {
        kripto_hash_input(k, Mds0(m,i,j), mlen);
      }
    }

    // add a \newline (0x0a, or '\n') between the rows but not after the last one
    if (i<nr-1)
    {
      if (newline_len>0)
        kripto_hash_input(k, newline, newline_len);
    }

  } // next i (row)

  hex_output = (char *) GC_malloc((2*hash_len+1)*sizeof(char));

  kripto_hash_output(k, hex_output, hash_len);

  // expand digest from uint8_t to \xNN
  for (i=hash_len-1; i>=0; i--)
  {
    sprintf(c3, "%02x", hex_output[i]);
    hex_output[2*i] = c3[0];
    hex_output[2*i+1] = c3[1];
  }
  hex_output[2*hash_len] = 0;

  // clean-up
  if (undo_n)
  {
    for (i=newline_len-1; i>=0; i--)
    {
      MdiV0(n,i) = (newline[i] & 0xff);
    }
  }
  kripto_hash_destroy(k);

_exit:

  ent_Clean (e1);
  ent_Clean (e2);
  ent_Clean (e3);
  ent_Clean (e4);
  ent_Clean (e5);
  ent_Clean (e6);

  rent = ent_Create ();
  ent_data (rent) = mds_CreateScalar ( hex_output );
  ent_type (rent) = MATRIX_DENSE_STRING;
  return rent;
}


//
// crc32 calculator for rlab2/rlab3.
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
// rlabplus (C) Marijan Kostrun, 2009:
//  moved here from rlabplus source tree:
//    crypto_hash.c
Ent * ent_kripto_crc32 (int nargs, Datum args[])
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

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
#ifndef RLAB_FANN_H
#define RLAB_FANN_H

//
// rlab headers
//
#include <rlab/rlab.h>
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

#include "mac.h"
#include "hash.h"
#include "mac/hmac.h"

#include "hash/md5.h"
#include "hash/sha1.h"
#include "hash/sha2_256.h"
#include "hash/sha2_512.h"
#include "hash/sha3.h"
#include "hash/keccak800.h"
#include "hash/keccak1600.h"
#include "hash/skein1024.h"
#include "hash/skein512.h"
#include "hash/skein256.h"
#include "hash/blake2b.h"
#include "hash/blake2s.h"
#include "hash/blake512.h"
#include "hash/blake256.h"
#include "hash/tiger.h"
#include "hash/whirlpool.h"
#include "block.h"
#include "block/aes.h"
#include "block/anubis.h"
#include "block/des.h"
#include "block/aria.h"
#include "block/blowfish.h"
#include "block/camellia.h"
#include "block/cast5.h"
#include "block/crax_s.h"
#include "block/idea.h"
#include "block/khazad.h"
#include "block/lea.h"
#include "block/noekeon.h"
#include "block/rc2.h"
#include "block/rc5.h"
#include "block/rc6.h"
#include "block/rectangle.h"
#include "block/rijndael128.h"
#include "block/rijndael256.h"
#include "block/safer.h"
#include "block/saferpp.h"
#include "block/safer_sk.h"
#include "block/seed.h"
#include "block/serpent.h"
#include "block/shacal2.h"
#include "block/simon32.h"
#include "block/simon64.h"
#include "block/simon128.h"
#include "block/skipjack.h"
#include "block/sm4.h"
#include "block/speck128.h"
#include "block/speck32.h"
#include "block/speck64.h"
#include "block/tea.h"
#include "block/threefish1024.h"
#include "block/threefish256.h"
#include "block/threefish512.h"
#include "block/trax_l.h"
#include "block/trax_m.h"
#include "block/twofish.h"
#include "block/xtea.h"

#include "stream.h"
#include "stream/chacha.h"
#include "stream/ctr.h"
#include "stream/cbc.h"
#include "stream/cfb.h"
#include "stream/ecb.h"
#include "stream/keccak800.h"
#include "stream/keccak1600.h"
#include "stream/ofb.h"
#include "stream/rc4.h"
#include "stream/salsa20.h"
#include "stream/skein256.h"
#include "stream/skein512.h"
#include "stream/skein1024.h"



Ent * ent_kripto_crc32  (int nargs, Datum args[]);
Ent * ent_kripto_hash   (int nargs, Datum args[]);
Ent * ent_kripto_mac    (int nargs, Datum args[]);
Ent * ent_kripto_stream (int nargs, Datum args[]);
Ent * ent_kripto_block  (int nargs, Datum args[]);

#endif

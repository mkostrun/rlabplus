//
// libkripto.so.r
// loader for the crypto/hash functions
//
static(_LIB_NAME,fileaddr,_HOME_,_LIBD_, _hash, _hmac);

if (!exist(_LIB_NAME))
{
  _LIB_NAME = "kripto";

  _HOME_ = getenv("HOME");
  if(getenv("CPU") == "i686")
  {
    _LIBD_ = "/rlab/lib.so/"+_LIB_NAME+"/rlabplus_libkripto.so";
  }
  else
  {
    _LIBD_ = "/rlab/lib.so/"+_LIB_NAME+"/rlabplus_lib64kripto.so";
  }

  fileaddr = _HOME_ + _LIBD_ ;

  if (!exist(_hash))
  {
    _hash = dlopen(fileaddr, "ent_kripto_hash");
  }

  if (!exist(_hmac))
  {
    _hmac = dlopen(fileaddr, "ent_kripto_mac");
  }

  if (!exist(_stream))
  {
    _stream = dlopen(fileaddr, "ent_kripto_stream");
  }

  if (!exist(_block))
  {
    _block = dlopen(fileaddr, "ent_kripto_block");
  }

  //
  //
  // block encrypt/decrypt function
  //
  //
  block = <<>>;
  if (!exist(block.encrypt))
  {
    block.encrypt = function(bc, m, r, k, tw, it)
    {
      return _block(bc, -1-abs(r), tw, m, k, it);
    };
  }
  if (!exist(block.decrypt))
  {
    block.decrypt = function(bc, m, r, k, tw, it)
    {
      return _block(bc, 1+abs(r), tw, m, k, it);
    };
  }

  //
  //
  // encrypt/decrypt function
  //
  //
  if (!exist(encrypt))
  {
    encrypt = function(bc, m, r, k, iv)
    {
      return _stream(bc, -abs(r), iv, m, k);
    };
  }
  if (!exist(decrypt))
  {
    decrypt = function(bc, m, r, k, iv)
    {
      return _stream(bc, abs(r), iv, m, k);
    };
  }

  //
  //
  // HASH function
  //
  //
  hash = <<>>;

  if (!exist(hash.crc32))
  {
    hash.crc32 = dlopen(fileaddr, "ent_kripto_crc32");
  }

  if (!exist(hash.sha512))
  {
    hash.sha512 = function(d, drpt, s, n)
    {
      return _hash("sha512", 64, d, drpt, s, n);
    };
  }

  if (!exist(hash.sha384))
  {
    hash.sha384 = function(d, drpt, s, n)
    {
      return _hash("sha512", 48, d, drpt, s, n);
    };
  }

  if (!exist(hash.sha256))
  {
    hash.sha256 = function(d, drpt, s, n)
    {
      return _hash("sha512", 32, d, drpt, s, n);
    };
  }

  if (!exist(hash.sha224))
  {
    hash.sha224 = function(d, drpt, s, n)
    {
      return _hash("sha512", 28, d, drpt, s, n);
    };
  }

  if (!exist(hash.sha1))
  {
    hash.sha1 = function(d, drpt, s, n)
    {
      return _hash("sha1", , d, drpt, s, n);
    };
  }

  if (!exist(hash.md5))
  {
    hash.md5 = function(d, drpt, s, n)
    {
      if (exist(s))
      {
        printf("hash.md5: libkripto does not support 'salt': Ignored! Use 'newline' instead!\n" );
      }
      return _hash("md5", , d, drpt,, n);
    };
  }

  if (!exist(hash.sha3_512))
  {
    hash.sha3_512 = function(d, drpt, s, n)
    {
      return _hash("sha3", 64, d, drpt, s, n);
    };
  }

  if (!exist(hash.sha3_384))
  {
    hash.sha3_384 = function(d, drpt, s, n)
    {
      return _hash("sha3", 48, d, drpt, s, n);
    };
  }

  if (!exist(hash.sha3_256))
  {
    hash.sha3_256 = function(d, drpt, s, n)
    {
      return _hash("sha3", 32, d, drpt, s, n);
    };
  }

  if (!exist(hash.sha3_224))
  {
    hash.sha3_224 = function(d, drpt, s, n)
    {
      return _hash("sha3", 28, d, drpt, s, n);
    };
  }

  if (!exist(hash.keccak800))
  {
    hash.keccak800 = function(d, drpt, s, n)
    {
      return _hash("keccak800", 32, d, drpt, s, n);
    };
  }

  if (!exist(hash.keccak1600))
  {
    hash.keccak1600 = function(d, drpt, s, n)
    {
      return _hash("keccak1600", 64, d, drpt, s, n);
    };
  }

  if (!exist(hash.keccak512))
  {
    hash.keccak512 = function(d, drpt, s, n)
    {
      return _hash("keccak1600", 64, d, drpt, s, n);
    };
  }

  if (!exist(hash.keccak384))
  {
    hash.keccak384 = function(d, drpt, s, n)
    {
      return _hash("keccak1600", 48, d, drpt, s, n);
    };
  }

  if (!exist(hash.keccak256))
  {
    hash.keccak256 = function(d, drpt, s, n)
    {
      return _hash("keccak1600", 32, d, drpt, s, n);
    };
  }

  if (!exist(hash.skein1024))
  {
    hash.skein1024 = function(d, drpt, s, n)
    {
      return _hash("skein", 128, d, drpt, s, n);
    };
  }

  if (!exist(hash.skein512))
  {
    hash.skein512 = function(d, drpt, s, n)
    {
      return _hash("skein", 64, d, drpt, s, n);
    };
  }

  if (!exist(hash.skein256))
  {
    hash.skein256 = function(d, drpt, s, n)
    {
      return _hash("skein", 32, d, drpt, s, n);
    };
  }

  if (!exist(hash.tiger))
  {
    hash.tiger = function(d, drpt, s, n)
    {
      return _hash("tiger", 20, d, drpt, s, n);
    };
  }

  if (!exist(hash.whirlpool))
  {
    hash.whirlpool = function(d, drpt, s, n)
    {
      return _hash("whirlpool", 64, d, drpt, s, n);
    };
  }

  if (!exist(hash.blake1_512))
  {
    hash.blake1_512 = function(d, drpt, s, n)
    {
      return _hash("blake1", 64, d, drpt, s, n);
    };
  }

  if (!exist(hash.blake1_256))
  {
    hash.blake1_256 = function(d, drpt, s, n)
    {
      return _hash("blake1", 32, d, drpt, s, n);
    };
  }

  if (!exist(hash.blake2s))
  {
    hash.blake2s = function(d, drpt, s, n)
    {
      return _hash("blake2", 32, d, drpt, s, n);
    };
  }

  if (!exist(hash.blake2b))
  {
    hash.blake2b = function(d, drpt, s, n)
    {
      return _hash("blake2", 64, d, drpt, s, n);
    };
  }

  //
  //
  // MAC function
  //
  //
  hmac = <<>>;

  if (!exist(hmac.sha512))
  {
    hmac.sha512 = function(d, drpt, s, n)
    {
      return _hmac("sha512", 64, d, drpt, s, n);
    };
  }

  if (!exist(hmac.sha384))
  {
    hmac.sha384 = function(d, drpt, s, n)
    {
      return _hmac("sha512", 48, d, drpt, s, n);
    };
  }

  if (!exist(hmac.sha256))
  {
    hmac.sha256 = function(d, drpt, s, n)
    {
      return _hmac("sha512", 32, d, drpt, s, n);
    };
  }

  if (!exist(hmac.sha224))
  {
    hmac.sha224 = function(d, drpt, s, n)
    {
      return _hmac("sha512", 28, d, drpt, s, n);
    };
  }

  if (!exist(hmac.sha1))
  {
    hmac.sha1 = function(d, drpt, s, n)
    {
      return _hmac("sha1", , d, drpt, s, n);
    };
  }

  if (!exist(hmac.md5))
  {
    hmac.md5 = function(d, drpt, s, n)
    {
      return _hmac("md5", 32, d, drpt, s, n);
    };
  }

  if (!exist(hmac.sha3_512))
  {
    hmac.sha3_512 = function(d, drpt, s, n)
    {
      return _hmac("sha3", 64, d, drpt, s, n);
    };
  }

  if (!exist(hmac.sha3_384))
  {
    hmac.sha3_384 = function(d, drpt, s, n)
    {
      return _hmac("sha3", 48, d, drpt, s, n);
    };
  }

  if (!exist(hmac.sha3_256))
  {
    hmac.sha3_256 = function(d, drpt, s, n)
    {
      return _hmac("sha3", 32, d, drpt, s, n);
    };
  }

  if (!exist(hmac.sha3_224))
  {
    hmac.sha3_224 = function(d, drpt, s, n)
    {
      return _hmac("sha3", 28, d, drpt, s, n);
    };
  }

  if (!exist(hmac.keccak800))
  {
    hmac.keccak800 = function(d, drpt, s, n)
    {
      return _hmac("keccak800", 32, d, drpt, s, n);
    };
  }

  if (!exist(hmac.keccak1600))
  {
    hmac.keccak1600 = function(d, drpt, s, n)
    {
      return _hmac("keccak1600", 64, d, drpt, s, n);
    };
  }

  if (!exist(hmac.keccak512))
  {
    hmac.keccak512 = function(d, drpt, s, n)
    {
      return _hmac("keccak1600", 64, d, drpt, s, n);
    };
  }

  if (!exist(hmac.keccak384))
  {
    hmac.keccak384 = function(d, drpt, s, n)
    {
      return _hmac("keccak1600", 48, d, drpt, s, n);
    };
  }

  if (!exist(hmac.keccak256))
  {
    hmac.keccak256 = function(d, drpt, s, n)
    {
      return _hmac("keccak1600", 32, d, drpt, s, n);
    };
  }

  if (!exist(hmac.skein1024))
  {
    hmac.skein1024 = function(d, drpt, s, n)
    {
      return _hmac("skein", 128, d, drpt, s, n);
    };
  }

  if (!exist(hmac.skein512))
  {
    hmac.skein512 = function(d, drpt, s, n)
    {
      return _hmac("skein", 64, d, drpt, s, n);
    };
  }

  if (!exist(hmac.skein256))
  {
    hmac.skein256 = function(d, drpt, s, n)
    {
      return _hmac("skein", 32, d, drpt, s, n);
    };
  }

  if (!exist(hmac.tiger))
  {
    hmac.tiger = function(d, drpt, s, n)
    {
      return _hmac("tiger", 20, d, drpt, s, n);
    };
  }

  if (!exist(hmac.whirlpool))
  {
    hmac.whirlpool = function(d, drpt, s, n)
    {
      return _hmac("whirlpool", 64, d, drpt, s, n);
    };
  }

  if (!exist(hmac.blake1_512))
  {
    hmac.blake1_512 = function(d, drpt, s, n)
    {
      return _hmac("blake1", 64, d, drpt, s, n);
    };
  }

  if (!exist(hmac.blake1_256))
  {
    hmac.blake1_256 = function(d, drpt, s, n)
    {
      return _hmac("blake1", 32, d, drpt, s, n);
    };
  }

  if (!exist(hmac.blake2s))
  {
    hmac.blake2s = function(d, drpt, s, n)
    {
      return _hmac("blake2", 32, d, drpt, s, n);
    };
  }

  if (!exist(hmac.blake2b))
  {
    hmac.blake2b = function(d, drpt, s, n)
    {
      return _hmac("blake2", 64, d, drpt, s, n);
    };
  }

  clear(fileaddr,_HOME_,_LIBD_);

} // if (!exist(hash._LIB_NAME))



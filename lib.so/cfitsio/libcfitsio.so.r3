//
// libcfitsio.so.r3
// loader for the functions from cfitsio library for load/save and manipulations with fits files
//
static(_LIB_NAME, fileaddr, _HOME_, _LIBD_);

if (!exist(_LIB_NAME))
{
  _LIB_NAME = "cfitsio";

  _HOME_ = getenv("HOME");
  if(getenv("CPU") == "i686")
  {
    _LIBD_ = "/rlab/lib.so/"+_LIB_NAME+"/rlabplus_libcfitsio.so";
  }
  else
  {
    _LIBD_ = "/rlab/lib.so/"+_LIB_NAME+"/rlabplus_lib64cfitsio.so";
  }

  fileaddr = _HOME_ + _LIBD_ ;

  if (!exist(cfitsio))
  {
    cfitsio = <<>>;
  }

  if (!exist(cfitsio.open))
  {
    cfitsio.open = dlopen(fileaddr, "ent_cfitsio_open");
  }

  if (!exist(cfitsio.close))
  {
    cfitsio.close = dlopen(fileaddr, "ent_cfitsio_close");
  }

  if (!exist(cfitsio.readheader))
  {
    cfitsio.readheader = dlopen(fileaddr, "ent_cfitsio_readheader");
  }

  if (!exist(cfitsio.readimage))
  {
    cfitsio.readimage = dlopen(fileaddr, "ent_cfitsio_readimage");
  }

  if (!exist(cfitsio.readtable))
  {
    cfitsio.readtable = dlopen(fileaddr, "ent_cfitsio_readtable");
  }

  clear(fileaddr,_HOME_,_LIBD_);

} // if (!exist(hash._LIB_NAME))






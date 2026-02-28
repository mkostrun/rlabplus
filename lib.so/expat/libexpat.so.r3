//
// libgphoto.so.r3
// loader for the functions that communicate with gphoto2 camera interface
//
static(_LIB_NAME,fileaddr,_HOME_,_LIBD_, _hash, _hmac);

if (!exist(_LIB_NAME))
{
  _LIB_NAME = "expat";

  _HOME_ = getenv("HOME");
  if(getenv("CPU") == "i686")
  {
    _LIBD_ = "/rlab/lib.so/"+_LIB_NAME+"/rlabplus_libexpat.so";
  }
  else
  {
    _LIBD_ = "/rlab/lib.so/"+_LIB_NAME+"/rlabplus_lib64expat.so";
  }

  fileaddr = _HOME_ + _LIBD_ ;

  if (!exist(xml_parse))
  {
    xml_parse = dlopen(fileaddr, "ent_expat_parse");
  }

  clear(fileaddr,_HOME_,_LIBD_);

} // if (!exist(hash._LIB_NAME))






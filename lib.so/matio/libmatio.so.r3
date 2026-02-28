//
// libgpib.so.r
// loader for the functions that communicate with gpib card
//
static(_LIB_NAME,fileaddr,_HOME_,_LIBD_);

if (!exist(_LIB_NAME))
{
  _LIB_NAME = "matio";

  _HOME_ = getenv("HOME");
  if(getenv("CPU") == "i686")
  {
    _LIBD_ = "/rlab/lib.so/"+_LIB_NAME+"/rlabplus_libmatio.so";
  }
  else
  {
    _LIBD_ = "/rlab/lib.so/"+_LIB_NAME+"/rlabplus_lib64matio.so";
  }

  fileaddr = _HOME_ + _LIBD_ ;

  if (!exist(matread))
  {
    matread = dlopen(fileaddr, "ent_matio_read");
  }

  if (!exist(matwritem))
  {
    matwritem = dlopen(fileaddr, "ent_matio_writem");
  }


  if (!exist(matopen))
  {
    matopen = dlopen(fileaddr, "ent_matio_open");
  }

  if (!exist(matclose))
  {
    matclose = dlopen(fileaddr, "ent_matio_close");
  }

  clear(fileaddr,_HOME_,_LIBD_);

} // if (!exist(_LIB_NAME))




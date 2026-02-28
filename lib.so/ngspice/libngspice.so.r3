//
// libngspice.so.r3
// loader for the functions that communicate with ngspice circuit simulator
//
static(_LIB_NAME,fileaddr,_HOME_,_LIBD_);

if (!exist(_LIB_NAME))
{
  _LIB_NAME = "ngspice";

  _HOME_ = getenv("HOME");
  if(getenv("CPU") == "i686")
  {
    _LIBD_ = "/rlab/lib.so/"+_LIB_NAME+"/rlabplus_libngspice.so";
  }
  else
  {
    _LIBD_ = "/rlab/lib.so/"+_LIB_NAME+"/rlabplus_lib64ngspice.so";
  }

  fileaddr = _HOME_ + _LIBD_ ;

  if (!exist(ngspice))
  {
    ngspice = <<>>;
  }

  if (!exist(ngspice.init))
  {
    ngspice.init = dlopen(fileaddr, "ent_ngspice_Initialize");
  }

  if (!exist(ngspice.exit))
  {
    ngspice.exit = dlopen(fileaddr, "ent_ngspice_Finalize");
  }

  if (!exist(ngspice.cmd))
  {
    ngspice.cmd = dlopen(fileaddr, "ent_ngspice_Command");
  }

  if (!exist(ngspice.runckt))
  {
    ngspice.runckt = dlopen(fileaddr, "ent_ngspice_RunCkt");
  }

  if (!exist(ngspice.circuit))
  {
    ngspice.circuit = dlopen(fileaddr, "ent_ngspice_Circuit");
  }

  if (!exist(ngspice.getvals))
  {
    ngspice.getvals = dlopen(fileaddr, "ent_ngspice_GetVals");
  }

  if (!exist(ngspice.isrunning))
  {
    ngspice.isrunning = dlopen(fileaddr, "ent_ngspice_IsRunning");
  }

  clear(fileaddr,_HOME_,_LIBD_);

} // if (!exist(hash._LIB_NAME))






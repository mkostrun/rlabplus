//
// libgpib.so.r
// loader for the functions that communicate with gpib card
//
static(_LIBGPIB_INIT);

if (!exist(_LIBGPIB_INIT))
{
  _LIBGPIB_INIT = 1;

  _HOME_ = getenv("HOME");
  if(getenv("CPU") == "i686")
  {
    _LIBD_ = "/rlab/lib.so/gpib/rlabplus_libgpib.so";
  else
    _LIBD_ = "/rlab/lib.so/gpib/rlabplus_lib64gpib.so";
  }

  fileaddr = _HOME_ + _LIBD_ ;

  if (!exist(ibln))
  {
    ibln = dlopen(fileaddr, "ent_gpib_ibln");
  }

  if (!exist(ibsic))
  {
    ibsic = dlopen(fileaddr, "ent_gpib_ibsic");
  }

  if (!exist(ibonl))
  {
    ibonl = dlopen(fileaddr, "ent_gpib_ibonl");
  }

  if (!exist(ibsre))
  {
    ibsre = dlopen(fileaddr, "ent_gpib_ibsre");
  }

  if (!exist(ibclr))
  {
    ibclr = dlopen(fileaddr, "ent_gpib_ibclr");
  }

  if (!exist(ibfind))
  {
    ibfind = dlopen(fileaddr, "ent_gpib_ibfind");
  }

  if (!exist(ibdev))
  {
    ibdev = dlopen(fileaddr, "ent_gpib_ibdev");
  }

  if (!exist(ibwrt))
  {
    ibwrt = dlopen(fileaddr, "ent_gpib_ibwrt");
  }

  if (!exist(ibrd))
  {
    ibrd = dlopen(fileaddr, "ent_gpib_ibrd");
  }

  if (!exist(ibqrd))
  {
    ibqrd = dlopen(fileaddr, "ent_gpib_ibqrd");
  }

  if (!exist(ibwrta))
  {
    ibwrta = dlopen(fileaddr, "ent_gpib_ibwrta");
  }

  if (!exist(ibwrtf))
  {
    ibwrtf = dlopen(fileaddr, "ent_gpib_ibwrtf");
  }

  if (!exist(ibask))
  {
    ibask = dlopen(fileaddr, "ent_gpib_ibask");
  }

  if (!exist(ibwait))
  {
    ibwait = dlopen(fileaddr, "ent_gpib_ibwait");
  }

  if (!exist(ibtrg))
  {
    ibtrg = dlopen(fileaddr, "ent_gpib_ibtrg");
  }

  if (!exist(ibrsp))
  {
    ibrsp = dlopen(fileaddr, "ent_gpib_ibrsp");
  }

  if (!exist(ibrsv))
  {
    ibrsv = dlopen(fileaddr, "ent_gpib_ibrsv");
  }

  if (!exist(ibloc))
  {
    ibloc = dlopen(fileaddr, "ent_gpib_ibloc");
  }

  if (!exist(EnableRemote))
  {
    EnableRemote = dlopen(fileaddr, "ent_gpib_EnableRemote");
  }

  if (!exist(ibcnt))
  {
    ibcnt = dlopen(fileaddr, "ent_gpib_ibcnt");
  }


  clear(fileaddr,_HOME_,_LIBD_);
} // if (!exist(_LIBGPIB_INIT))


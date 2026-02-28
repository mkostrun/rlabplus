//
// libgpib.so.r3
// loader for the functions that communicate with gpib card
//
static(_LIBGPIB_INIT);

if (exist(_LIBGPIB_INIT))
{
  EOF
}

_HOME_ = getenv("HOME");

if(getenv("CPU") == "i686")
{
  _LIBD_ = "/rlab/lib.so/gpib/rlabplus_libgpib.so";
}

if(getenv("CPU") == "x86_64")
{
  _LIBD_ = "/rlab/lib.so/gpib/rlabplus_lib64gpib.so";
}

fileaddr = _HOME_ + _LIBD_ ;

ibln = dlopen(fileaddr, "ent_gpib_ibln");
ibsic = dlopen(fileaddr, "ent_gpib_ibsic");
ibonl = dlopen(fileaddr, "ent_gpib_ibonl");
ibsre = dlopen(fileaddr, "ent_gpib_ibsre");
ibclr = dlopen(fileaddr, "ent_gpib_ibclr");
ibfind = dlopen(fileaddr, "ent_gpib_ibfind");
ibdev = dlopen(fileaddr, "ent_gpib_ibdev");
ibwrt = dlopen(fileaddr, "ent_gpib_ibwrt");
ibrd = dlopen(fileaddr, "ent_gpib_ibrd");
ibqrd = dlopen(fileaddr, "ent_gpib_ibqrd");
ibwrta = dlopen(fileaddr, "ent_gpib_ibwrta");
ibwrtf = dlopen(fileaddr, "ent_gpib_ibwrtf");
ibask = dlopen(fileaddr, "ent_gpib_ibask");
ibwait = dlopen(fileaddr, "ent_gpib_ibwait");
ibtrg = dlopen(fileaddr, "ent_gpib_ibtrg");
ibrsp = dlopen(fileaddr, "ent_gpib_ibrsp");
ibrsv = dlopen(fileaddr, "ent_gpib_ibrsv");
ibloc = dlopen(fileaddr, "ent_gpib_ibloc");
EnableRemote = dlopen(fileaddr, "ent_gpib_EnableRemote");
ibcnt = dlopen(fileaddr, "ent_gpib_ibcnt");

clear(fileaddr,_HOME_,_LIBD_);

_LIBGPIB_INIT = 1;





//
// libgpib.so.r
// loader for the functions that communicate with gpib card
//
_HOME_ = getenv("HOME");
_LIB_NAME_ = "grampc";

if(getenv("CPU") == "i686")
{
  _LIBD_ = "/rlab/lib.so/"+_LIB_NAME_+"/rlabplus_grampc.so";
}
else
{
  _LIBD_ = "/rlab/lib.so/"+_LIB_NAME_+"/rlabplus_lib64grampc.so";
}

if (!exist(grampc))
{
  grampc = <<>>;
  grampc.solve = dlopen(_HOME_ + _LIBD_ , "ent_grampc");
}



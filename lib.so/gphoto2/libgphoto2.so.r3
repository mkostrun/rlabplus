//
// libgphoto.so.r3
// loader for the functions that communicate with gphoto2 camera interface
//
static(_LIB_NAME,fileaddr,_HOME_,_LIBD_, _hash, _hmac);

if (!exist(_LIB_NAME))
{
  _LIB_NAME = "gphoto2";

  _HOME_ = getenv("HOME");
  if(getenv("CPU") == "i686")
  {
    _LIBD_ = "/rlab/lib.so/"+_LIB_NAME+"/rlabplus_libgphoto2.so";
  }
  else
  {
    _LIBD_ = "/rlab/lib.so/"+_LIB_NAME+"/rlabplus_lib64gphoto2.so";
  }

  fileaddr = _HOME_ + _LIBD_ ;

  if (!exist(gp))
  {
    gp = <<>>;
  }

  if (!exist(gp.init))
  {
    gp.init = dlopen(fileaddr, "ent_gp_camera_init");
  }

  if (!exist(gp.exit))
  {
    gp.exit = dlopen(fileaddr, "ent_gp_camera_exit");
  }

  if (!exist(gp.config))
  {
    gp.config = dlopen(fileaddr, "ent_gp_camera_config");
  }

  if (!exist(gp.config_options))
  {
    gp.config_options = dlopen(fileaddr, "ent_gp_camera_config_options");
  }

  if (!exist(gp.info))
  {
    gp.info = dlopen(fileaddr, "ent_gp_camera_info");
  }

  if (!exist(gp.capture))
  {
    gp.capture = dlopen(fileaddr, "ent_gp_camera_capture");
  }

  if (!exist(gp.supported))
  {
    gp.supported = dlopen(fileaddr, "ent_gp_info_supported_cameras");
  }

  clear(fileaddr,_HOME_,_LIBD_);

} // if (!exist(hash._LIB_NAME))






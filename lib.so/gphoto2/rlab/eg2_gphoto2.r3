//
// Nikon Coolpix L22 operates as generic USB PTP camera
//
rfile libgphoto2.so
if (!exist(gp))
{
  stop("RLaB was not built with gphoto2 support");
}

gp.exit();
gp.init();
ci = gp.info();
ci.summary?
co = gp.config_options();

//
// cut to the chase and get photo
//
gp.capture("./testimage.jpg");




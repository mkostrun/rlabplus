//
//
//
rfile libgphoto2.so

mycamera = "D40";

if (!exist(gp))
{
  stop("RLaB was not built with gphoto2 support");
}

if (!exist(SUPPORTED_CAMERAS))
{
  SUPPORTED_CAMERAS = gp.supported();
}

"\nlooking for camera '" + mycamera + "' in a list of " ...
  + text(SUPPORTED_CAMERAS.n,"%.f models") + "\n"?

"\tmatching models: " + SUPPORTED_CAMERAS[ find(strindex(SUPPORTED_CAMERAS,mycamera)) ] ?

"\n\nlooking for camera (this time I chose my name better):\n"?
mycamera = "Nikon DSC D40 (PTP mode)";
"\tmatching models: " + SUPPORTED_CAMERAS[ find(strindex(SUPPORTED_CAMERAS,mycamera)) ] +"\n"?



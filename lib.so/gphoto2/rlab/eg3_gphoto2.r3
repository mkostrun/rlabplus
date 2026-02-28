//
//
//
rfile libgphoto2.so
if (!exist(gp))
{
  stop("RLaB was not built with gphoto2 support");
}


mycamera = "Nikon DSC D40 (PTP mode)";

gp.init(mycamera)

x = gp.info();

"Information about the driver for " + mycamera +  "\n"?
x.op?
x.file_op?
x.folder_op?
x.status?
x.summary?

opts=gp.config_options();

xopt = "shutterspeed";
yopt = "f-number";
"current value for '" + xopt + "' is '" + gp.config(xopt) + "'"
"values for '" + xopt + "' are"
x=opts.[xopt]?
"current value for '" + yopt + "' is '" + gp.config(yopt) + "'"
"values for '" + yopt + "' are"
y=opts.[yopt]?

c_speed = gp.config(xopt);
i_speed = find(x == c_speed);
c_fnum  = gp.config(yopt);
i_fnum  = find(y == c_fnum);

di_fnum  = 2;
di_speed = 3;

speed_Range = x[max(i_speed - di_speed,1):min(i_speed + di_speed,len(x))];
fnum_Range  = y[max(i_fnum - di_fnum,1):min(i_fnum + di_fnum,len(y))];

"\n\nSome features set in the camera are\n"?
"autoiso = " + gp.config("autoiso") +"\n"?
"batterylevel = " + gp.config("batterylevel")+"\n"?
"imagesize = " + gp.config("imagesize") +"\n"?
"iso = " + gp.config("iso") +"\n"?

"\n\nCapturing pictures now and transfering them to ./photo directory:" ?
mkdir("photo");
for (i in range(speed_Range))
{
  for (j in range(fnum_Range))
  {
    speed = speed_Range[i];
    fnum  = fnum_Range[j];
    gp.config(xopt,speed);
    gp.config(yopt,fnum);

    fnum2 = gsub("f","f/",fnum).string;
    if (!strindex(fnum2,"."))
    { fnum2 = fnum2 + ".0"; }
    fn = "photo/snap_" + speed + "_" + fnum2 + ".jpg";
    fn?
    gp.capture(fn);
  }
}

// put original values in, and leave
gp.config(xopt,c_speed);
gp.config(yopt,c_fnum);
gp.exit();





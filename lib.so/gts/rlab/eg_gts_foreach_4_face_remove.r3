//
//
//
rfile libgts.so geomview

DO_NOT_CLOSE=1;
DO_NOT_PAUSE=1;
// rfile module_geomview_scene

gts.surface.new("s", <<desc="sphere";level=4>>);
face_remove_fn = function(fc)
{
  static(count,count_rem);
  if (!exist(count))
  {
    count = 0;
  }
  if (!exist(count_rem))
  {
    count_rem = 0;
  }
  count ++;

  yes_no = all(fc[;1,4,7]<0);

  if (yes_no)
  {
    count_rem ++;
  }
r
  "\nface "?
  [count, yes_no, count_rem] ?
  fc?

  return yes_no;
};

geomview.plot_gts_surface("scene", "s");
pause();

v = gts.surface.vertex("s");
v?

gts.surface.for_each_face_remove("s", face_remove_fn);
geomview.plot_gts_surface("scene", "s");

pause();

geomview.snapshot("halfsphere.ps");
system("ps2pdf 2>/dev/null halfsphere.ps");








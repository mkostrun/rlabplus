//
//
//
rfile libgts.so

gts.surface.new("s", <<desc="sphere";level=2>>);

face_fn = function(fc)
{
  static(count);
  if (!exist(count))
  {
    count = 0;
  }
  count ++;

  "\nface "?
  count?
  fc?

  return count;
};


"Test 1\n"?
r = gts.surface.for_each_face("s", face_fn);
r?
"Test 1 - Completed\n\n"?

gts.surface.writem("./test.oogl", "s", "oogl");
//pause("after 1");

"Test 2\n"?
gts.surface.tessellate("s");
r = gts.surface.for_each_face("s", face_fn);
r?
if (all(r==-1))
{
  "Test 2 - Failed\n\n"?
  stop()
}
gts.surface.writem("./test.oogl", "s", "oogl");
//pause("after 2");


gts.surface.tessellate("s");
r = gts.surface.for_each_face("s", face_fn);
r?
gts.surface.writem("./test.oogl", "s", "oogl");
"done\n"?



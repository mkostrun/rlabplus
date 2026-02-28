//
//
//
rfile libgts.so

gts.surface.new("s", <<desc="sphere";level=2>>);

vertex_fn = function(vx)
{
  static(count);
  if (!exist(count))
  {
    count = 0;
  }
  count ++;

  rval = (uniform() < 0.9);

  [count, rval, vx]?

  return vx;
};

r = gts.surface.for_each_vertex("s", vertex_fn);
r?


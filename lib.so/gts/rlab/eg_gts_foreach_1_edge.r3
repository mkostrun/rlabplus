//
//
//
rfile libgts.so

gts.surface.new("s", <<desc="sphere";level=2>>);

edge_fn = function(ed)
{
  static(count);
  if (!exist(count))
  {
    count = 0;
  }
  count ++;

  rval = (uniform() < 0.9);

  ed?

  return ed;
};

r = gts.surface.for_each_edge("s", edge_fn);
r?


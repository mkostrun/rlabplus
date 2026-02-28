//
//
// 
rfile libgts.so

gnuwins(1);

//
// background
//
same_triangles = function(t1, t2)
{
  //
  v1 = t1[1:3];
  v2 = t1[4:6];
  v3 = t1[7:9];

  //
  u1 = t2[1:3];
  u2 = t2[4:6];
  u3 = t2[7:9];

  //
  d = [mnorm(u1 - v1,2),mnorm(u1 - v2,2),mnorm(u1 - v3,2)];
  //idx1 = find(d == min(d));
  if (min(d) > 1e-3)
  {
    return 0;
  }
  //
  d = [mnorm(u2 - v1,2),mnorm(u2 - v2,2),mnorm(u2 - v3,2)];
  //idx2 = find(d == min(d));
  if (min(d) > 1e-3)
  {
    return 0;
  }
  //
  d = [mnorm(u3 - v1,2),mnorm(u3 - v2,2),mnorm(u3 - v3,2)];
  //idx3 = find(d == min(d));
  if (min(d) > 1e-3)
  {
    return 0;
  }

//  idx = unique([idx1, idx2, idx3]);
//  if (length(idx)<3)
//  {
//    return 0;
//  }

  return 1;
};

get_indexed_triangle_from_surface = function (idx, h)
{
  e_idx = h.f[idx;];
  e     = h.e[e_idx;];
  // v = 
  //  [a1, a2; a3, a4; a5, a6]
  v     = [h.v[e[;1];],h.v[e[;2];]];
  // flatten to [v1,v2,v3]
  v12 = v[1;];
  v3a = v[2;1:3];
  v3b = v[2;4:6];
  if (all(v3a==v12[1:3]) || all(v3a==v12[4:6]))
  {
    rval = [v12, v3b];
  }
  else
  {
    rval = [v12, v3a];
  }
  return rval;
};

face_remove_fn = function(t1, h)
{
  global(get_indexed_triangle_from_surface,same_triangles);

  for(i in 1:h.f.nr)
  {
    t2 = get_indexed_triangle_from_surface(i,h);
    if (same_triangles(t1,t2))
    {
      return 1;
    }
  }

  return 0;
};

dim_scene = sqrt(10); //
color_bkg = uniform(1,3); // choose random color

//
// we try to remove a surface from "scene"
//
//_u = 1;
_u = uniform(4,3);
_v = (2.*uniform(1,3) -1) ;
vs = [...
    -1,-1, 0; ...
    -1, 1, 0; ...
     1, -1, 0; ...
     1,  1, 0 ...
];
gts.surface.from_vertices("scene", [_v + vs .* _u; dim_scene .* vs]);
s = gts.surface.export("scene");
rfile module_gnuplot_gts
sleep(1);

gts.surface.from_vertices("hole", _v + vs .* _u);
h = gts.surface.export("hole");

gts.surface.for_each_face_remove("scene", face_remove_fn, h);
s = gts.surface.export("scene");
rfile module_gnuplot_gts

















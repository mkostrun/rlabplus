//
// libgpib.so.r
// loader for the functions that communicate with gpib card
//
static(_LIB_NAME,fileaddr,_HOME_,_LIBD_);

if (!exist(_LIB_NAME))
{
  _LIB_NAME = "gts";

  _HOME_ = getenv("HOME");
  if(getenv("CPU") == "i686")
  {
    _LIBD_ = "/rlab/lib.so/"+_LIB_NAME+"/rlabplus_libgts.so";
  }
  else
  {
    _LIBD_ = "/rlab/lib.so/"+_LIB_NAME+"/rlabplus_lib64gts.so";
  }

  fileaddr = _HOME_ + _LIBD_ ;

  gts = <<>>;
  gts.surface = <<>>;
  gts.surface.calc = <<>>;
  if (!exist(gts.surface.ls))
  {
    gts.surface.ls = dlopen(fileaddr, "ent_gts_surface_list");
  }
  if (!exist(gts.surface.new))
  {
    gts.surface.new = dlopen(fileaddr, "ent_gts_surface_new");
  }
  if (!exist(gts.surface.new))
  {
    gts.surface.modify = dlopen(fileaddr, "ent_gts_surface_modify");
  }
  if (!exist(gts.surface.from_vertices))
  {
    gts.surface.from_vertices = dlopen(fileaddr, "ent_gts_surface_from_vertices");
  }
  if (!exist(gts.surface.vertex))
  {
    gts.surface.vertex = dlopen(fileaddr, "ent_gts_surface_vertices");
  }
  if (!exist(gts.surface.export))
  {
    gts.surface.export = dlopen(fileaddr, "ent_gts_surface_export");
  }
  if (!exist(gts.surface.add_face))
  {
    gts.surface.add_face = dlopen(fileaddr, "ent_gts_surface_add_face");
  }
  if (!exist(gts.surface.add_point))
  {
    gts.surface.add_point = dlopen(fileaddr, "ent_gts_surface_add_point");
  }
  if (!exist(gts.surface.remove_face))
  {
    gts.surface.remove_face = dlopen(fileaddr, "ent_gts_surface_remove_face");
  }
  if (!exist(gts.surface.remove_surface))
  {
    gts.surface.remove_surface = dlopen(fileaddr, "ent_gts_surface_remove_surface");
  }
  if (!exist(gts.surface.copy))
  {
    gts.surface.copy = dlopen(fileaddr, "ent_gts_surface_copy");
  }
  if (!exist(gts.surface.merge))
  {
    gts.surface.merge = dlopen(fileaddr, "ent_gts_surface_merge");
  }
  if (!exist(gts.surface.intersect))
  {
    gts.surface.intersect = dlopen(fileaddr, "ent_gts_surface_intersect");
  }
  if (!exist(gts.surface.segment))
  {
    gts.surface.segment = dlopen(fileaddr, "ent_gts_segment_intersect_surface");
  }
  if (!exist(gts.surface.boolean))
  {
    gts.surface.boolean = dlopen(fileaddr, "ent_gts_surface_boolean");
  }
  if (!exist(gts.surface.is))
  {
    gts.surface.is = dlopen(fileaddr, "ent_gts_surface_is");
  }
  if (!exist(gts.surface.info))
  {
    gts.surface.info = dlopen(fileaddr, "ent_gts_surface_info");
  }
  if (!exist(gts.surface.calc.area))
  {
    gts.surface.calc.area = dlopen(fileaddr, "ent_gts_surface_calc_area");
  }
  if (!exist(gts.surface.calc.vol))
  {
    gts.surface.calc.vol = dlopen(fileaddr, "ent_gts_surface_calc_volume");
  }
  if (!exist(gts.surface.calc.center_of_mass))
  {
    gts.surface.calc.center_of_mass = dlopen(fileaddr, "ent_gts_surface_calc_center_of_mass");
  }
  if (!exist(gts.surface.calc.center_of_area))
  {
    gts.surface.calc.center_of_area = dlopen(fileaddr, "ent_gts_surface_calc_center_of_area");
  }
  if (!exist(gts.surface.stat))
  {
    gts.surface.stat = dlopen(fileaddr, "ent_gts_surface_stats");
  }
  if (!exist(gts.surface.qstat))
  {
    gts.surface.qstat = dlopen(fileaddr, "ent_gts_surface_qstats");
  }
  if (!exist(gts.surface.writem))
  {
    gts.surface.writem = dlopen(fileaddr, "ent_gts_surface_writem");
  }
  if (!exist(gts.surface.for_each_vertex))
  {
    gts.surface.for_each_vertex = dlopen(fileaddr, "ent_gts_surface_foreach_vertex");
  }
  if (!exist(gts.surface.for_each_edge))
  {
    gts.surface.for_each_edge = dlopen(fileaddr, "ent_gts_surface_foreach_edge");
  }
  if (!exist(gts.surface.for_each_face))
  {
    gts.surface.for_each_face = dlopen(fileaddr, "ent_gts_surface_foreach_face");
  }
  if (!exist(gts.surface.for_each_face_remove))
  {
    gts.surface.for_each_face_remove = dlopen(fileaddr, "ent_gts_surface_foreach_face_remove");
  }
  if (!exist(gts.surface.tessellate))
  {
    gts.surface.tessellate = dlopen(fileaddr, "ent_gts_surface_tessellate");
  }
 if (!exist(gts.surface.readm))
  {
    gts.surface.readm = dlopen(fileaddr, "ent_gts_surface_readm");
  }
  if (!exist(gts.surface.threshold))
  {
    gts.surface.threshold = dlopen(fileaddr, "ent_gts_surface_threshold");
  }

  clear(fileaddr,_HOME_,_LIBD_);

} // if (!exist(_LIB_NAME))

//
//
//
static(_get_indexed_triangle_from_surface, _same_triangles, _face_remove_fn, _triange_in_polygon);

_triange_in_polygon = function(t, p)
{
  //
  // t = [u1, u2, u3]
  // p = [v1; v2; v3; ... vN]

  nv = zeros(1,3);
  for (k in 1:3)
  {
    u_k = t[(3*(k-1)+1):(3*k)];
    for (i in 1:p.nr)
    {
      v_i = p[i;];
      if (mnorm(u_k - v_i,2) < 1e-6)
      {
        nv[k] = i;
        break;
      }
    }
    if (nv[k]==0)
    {
      break;
    }
  }
  if (nv[k] == 0)
  {
    return 0;
  }

  return 1;
};

_get_indexed_triangle_from_surface = function (idx, h)
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

_same_triangles = function(t1, t2)
{
  //
  v1 = t1[1:3];
  v2 = t1[4:6];
  v3 = t1[7:9];

  //
  u1 = t2[1:3];
  u2 = t2[4:6];
  u3 = t2[7:9];

  "\nt1,t2=\n"?
  [t1;t2]?
  //"\nt1=\n"?
  //t1?

  //
  d = [mnorm(u1 - v1,2),mnorm(u1 - v2,2),mnorm(u1 - v3,2)];
  d?
  if (min(d) > 1e-3)
  {
    return 0;
  }
  //
  d = [mnorm(u2 - v1,2),mnorm(u2 - v2,2),mnorm(u2 - v3,2)];
  d?
  if (min(d) > 1e-3)
  {
    return 0;
  }
  //
  d = [mnorm(u3 - v1,2),mnorm(u3 - v2,2),mnorm(u3 - v3,2)];
  d?
  if (min(d) > 1e-3)
  {
    return 0;
  }

  "1\n"?
  return 1;
};

_face_remove_fn = function(t1, h)
{
  for(i in 1:h.f.nr)
  {
    t2 = _get_indexed_triangle_from_surface(i,h);
    if (_same_triangles(t1,t2))
    {
      return 1;
    }
  }

  return 0;
};

gts.surface.remove_surface2 = function(s1, sr)
{
  global(gts);
  h = gts.surface.export(sr);
  return gts.surface.for_each_face_remove(s1, _face_remove_fn, h);
};

gts.surface.remove_rectangle2 = function(s1, sr)
{
  global(gts);
  h = gts.surface.export(sr);
  return gts.surface.for_each_face_remove(s1, _triange_in_polygon, h.v);
};













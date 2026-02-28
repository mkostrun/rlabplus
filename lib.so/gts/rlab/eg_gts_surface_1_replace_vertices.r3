//
//
//
rfile libgts.so

surf_name = "deformed_ball";

gts.surface.new(surf_name, <<desc="sphere";level=4>>);

while (1)
{
  spinner();

  geomview.plot_gts_surface("scene", surf_name);

  v = gts.surface.vertex(surf_name);
  i = int(v.nr * uniform()) + 1;

  gts.surface.vertex(surf_name, v[i;], (1 + 0.01.*gaussian(1,3)) .* v[i;] + 0.01.*gaussian(1,3) );
}




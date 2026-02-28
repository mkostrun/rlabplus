rfile libgeomview
rfile libgts.so

gnuwins(2);

// ice cream cone parameters
b = 1;  // bottom radius - also radius of the ball
h = 4;  // height

A_0 = [0,0,h];


//
// construct half-sphere in space z<=0
//
// 1. create sphere of radius b
gts.surface.new("ice_cream_cone", <<desc="sphere";level=7;scale=b>>);
face_remove_fn = function(fc)
{
  yes_no = any(fc[;3,6,9]>0);
  return yes_no;
};
// remove all points which z<=0
gts.surface.for_each_face_remove("ice_cream_cone", face_remove_fn);
// find all points which are on the rim: z=0
ice_cream_cone = gts.surface.export("ice_cream_cone");
idx_vrim = find(ice_cream_cone.v[;3] == 0);
if (isempty(idx_vrim))
{
  stop("Calculation failed: change == to something more robust!\n");
}
fi = atan2(ice_cream_cone.v[idx_vrim;2],ice_cream_cone.v[idx_vrim;1]);
s_fi_idx = sort(fi).idx;
for (i in range(s_fi_idx))
{
  ip1 = ifelse(i>=length(s_fi_idx),i+1-length(s_fi_idx),i+1);
  // construct a triangle to be added to the surface:
  //  v[i], v[ip1], A_0
  i_1 = idx_vrim[s_fi_idx[i]];
  i_2 = idx_vrim[s_fi_idx[ip1]];
  gts.surface.add_face("ice_cream_cone",...
      [ice_cream_cone.v[i_1;],ice_cream_cone.v[i_2;],A_0] );
}

geomview.plot_gts_surface("scene","ice_cream_cone");
pause("Rotate 3D view to choosing, then press enter");
geomview.snapshot("ice_cream_cone.ps");

ice_cream_cone = gts.surface.export("ice_cream_cone");

//
// choose LOS n
//
phi_los_deg = 57;
psi_los_deg = 45;
rtm = eul2rotm(deg2rad.*[psi_los_deg,phi_los_deg],"zx");
nlos = (rtm * [0;0;1])';

v_rtm = zeros(ice_cream_cone.v);
for (i in 1:ice_cream_cone.v.nr)
{
  p0 = ice_cream_cone.v[i;];
  t  = -sum(p0 .* nlos);
  v_rtm[i;] = (p0 + t .* nlos) * rtm;
  v_rtm[i;3] = 0;
}

gnuwin (2);
gnuformat("with points pt 6 ps 1 lc rgb 'black'");
gnucmd ("set size ratio -1");
gnuplot(v_rtm[;1,2]);

area_gts_m2 = gts.surface.calc.area("ice_cream_cone");
area_gts_m2?


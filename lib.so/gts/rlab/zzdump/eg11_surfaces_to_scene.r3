//
//
//
rfile libgts.so

rfile libgeomview

//
// all units in meters
//
yaw_matrix = function(q_rad)
{
  rval = [ ...
       cos(q_rad), sin(q_rad), 0; ...
      -sin(q_rad), cos(q_rad), 0; ...
               0,           0, 1; ...
      []];
  return rval;
};

dim_scene = 15; //
target_position = [0, 0.5*dim_scene, 0];
yaw_deg = 45;
yaw_rad = deg2rad * yaw_deg;
color_bkg = uniform(1,3); // choose random color 
color_tgt = uniform(1,3); // choose random color 


gts.surface.new("kugla", <<desc="sphere";level=2>>);



//
// background
//
gts.surface.new("scene",  <<desc="rectangle";color=color_bkg;bbox_x=[-dim_scene,dim_scene];bbox_y=[-dim_scene,dim_scene];bbox_z=0>>);
s = gts.surface.export("scene");
rfile module_gnuplot_gts
sleep(0.3);

//
// target at:
//
gts.surface.new("s2",  <<desc="rectangle";color=color_tgt;bbox_x=[-1.5,1.5];bbox_y=[-3,3];bbox_z=0;transl=target_position;yaw=yaw_matrix(yaw_rad)>>);
s = gts.surface.export("s2");
rfile module_gnuplot_gts
sleep(0.3);

gts.surface.remove_surface("scene", "s2");
printf("remove_surface failed: face could not be removed!\n");
s = gts.surface.export("scene");
rfile module_gnuplot_gts
sleep(0.3);

//
//
//
s2 = gts.surface.export("s2");

gts.surface.add_point("scene", s2.v);
s = gts.surface.export("scene");
rfile module_gnuplot_gts
sleep(0.3);

if (!gts.surface.remove_face("scene", s2.v[2:4;]))
{
  printf("remove_face1 failed: face could not be removed!\n");
}
s = gts.surface.export("scene");
rfile module_gnuplot_gts

if (!gts.surface.remove_face("scene", s2.v[1,2,4;]))
{
  printf("remove_face2 failed: face could not be removed!\n");
}
s = gts.surface.export("scene");
rfile module_gnuplot_gts








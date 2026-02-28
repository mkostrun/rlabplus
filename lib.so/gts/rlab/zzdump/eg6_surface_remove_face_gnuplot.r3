//
//
// 
rfile libgts.so

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

q_rad = pi*uniform();
YM = yaw_matrix(q_rad);
Am = 0.1+0.9*uniform();

gnuwins(1);

//
// background
//

dim_scene = sqrt(10); //
color_bkg = uniform(1,3); // choose random color

"Generating scene\n" ?
gts.surface.new("scene",  <<desc="rectangle";...
    color=color_bkg;bbox_x=[-dim_scene,dim_scene];bbox_y=[-dim_scene,dim_scene];bbox_z=0>>);
bnew  = gts.surface.export("scene");

s = bnew;
rfile module_gnuplot_gts
sleep(0.5);

vs_offset = [2.*uniform(1,2)-1,0];

//
// we try to remove a surface from "scene"
//
vs = [...
    -1,-1, 0; ...
    -1, 1, 0; ...
     1, 1, 0];
gts.surface.add_point("scene", Am.*vs*YM + vs_offset);

s = bnew1;
rfile module_gnuplot_gts
sleep(0.5);

vs = [...
    -1, -1, 0; ...
     1, -1, 0; ...
     1,  1, 0];
gts.surface.add_point("scene", Am.*vs*YM + vs_offset);
bnew2  = gts.surface.export("scene");

s = bnew2;
rfile module_gnuplot_gts
sleep(0.5);

vs = [...
    -1, -1, 0; ...
    1, -1, 0; ...
    1,  1, 0];
gts.surface.remove_face("scene", Am.*vs*YM + vs_offset);
bnew3  = gts.surface.export("scene");

s = bnew3;
rfile module_gnuplot_gts
sleep(0.5);

    
vs = [...
    -1,-1, 0; ...
    -1, 1, 0; ...
    1, 1, 0];
gts.surface.remove_face("scene", Am.*vs*YM + vs_offset);
bnew4  = gts.surface.export("scene");

s = bnew4;
rfile module_gnuplot_gts

















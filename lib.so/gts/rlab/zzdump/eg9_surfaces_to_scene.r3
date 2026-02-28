//
//
//
rfile libgts.so

GEOMVIEW = "/usr/local/bin/geomview ";
geom_fn = "| " + GEOMVIEW + " -c -";
geom_gcl_script1= "(geometry scene { : foo } )\n";
geom_gcl_script1_start = "(read geometry { define foo\n";
geom_gcl_script1_end   = "})\n";
geom_gcl_script2= "(geometry scan { : bar } )\n";
geom_gcl_script2_start = "(read geometry { define bar\n";
geom_gcl_script2_end   = "})\n";

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

//
// background
//
gts.surface.new("scene",  <<desc="rectangle";color=color_bkg;bbox_x=[-dim_scene,dim_scene];...
    bbox_y=[-dim_scene,dim_scene];bbox_z=0>>);

if (0)
{
  //
  // target at:
  //
  gts.surface.new("target", <<desc="rectangle";color=color_tgt;bbox_x=[-1.5,1.5];bbox_y=[-3,3];bbox_z=0;transl=target_position;yaw=yaw_matrix(yaw_rad)>>);
  //
  gts.surface.new("s2", <<desc="rectangle";color=color_tgt;bbox_x=[-1.5,1.5];bbox_y=[-3,3];bbox_z=1.5;transl=target_position;yaw=yaw_matrix(yaw_rad)>>);
  gts.surface.merge("target", "s2");
  //
  gts.surface.new("s3", <<desc="rectangle";color=color_tgt;bbox_x=-1.5;bbox_y=[-3,3];bbox_z=[0,1.5];transl=target_position;yaw=yaw_matrix(yaw_rad)>>);
  gts.surface.merge("target", "s3");
  //
  gts.surface.new("s4", <<desc="rectangle";color=color_tgt;bbox_x=1.5;bbox_y=[-3,3];bbox_z=[0,1.5];transl=target_position;yaw=yaw_matrix(yaw_rad)>>);
  gts.surface.merge("target", "s4");
  //
  gts.surface.new("s5", <<desc="rectangle";color=color_tgt;bbox_x=[-1.5,1.5];bbox_y=-3;bbox_z=[0,1.5];transl=target_position;yaw=yaw_matrix(yaw_rad)>>);
  gts.surface.merge("target", "s5");
  //
  gts.surface.new("s6", <<desc="rectangle";color=color_tgt;bbox_x=[-1.5,1.5];bbox_y=3;bbox_z=[0,1.5];transl=target_position;yaw=yaw_matrix(yaw_rad)>>);
  gts.surface.merge("target", "s6");

  //stop()
  gts.surface.merge("scene", "target");
}

//
// next we try to find
//
H = 10;
theta_deg = 33;     // boresight slant
gamma_deg = 0;      // scan angle
phi_rad = 60e-3;    // sensor offset below boresight
fov_rad = 7.5e-3;   // sensor FOV
a_lo_rad = theta_deg * deg2rad - phi_rad - 0.5 * fov_rad;
a_hi_rad = theta_deg * deg2rad - phi_rad + 0.5 * fov_rad;

//scan_pattern_vx = [...
//    0, 0, H; ...
//    cos(gamma_deg*deg2rad) * tan(a_lo_rad) * H, sin(gamma_deg*deg2rad) * tan(a_lo_rad) * H, 0; ...
//    cos(gamma_deg*deg2rad) * tan(a_hi_rad) * H, sin(gamma_deg*deg2rad) * tan(a_hi_rad) * H, 0; ...
//     []];
scan_pattern_vx = [...
    0, 0, H; ...
    0.5*H, 0, -1; ...
    0, 0, -1; ...
    []];
gts.surface.from_vertices("scan", scan_pattern_vx, [1,2,3], <<color=[0,0,1]>>);

es = gts.surface.intersect("scan", "scene");
es.e?
all(abs(es.c - color_bkg) < 1e-4)?
es.c?

//gts.surface.merge("scene", "scan");

//
//
//
open (geom_fn, "w");
fprintf(geom_fn, geom_gcl_script1);
fprintf(geom_fn, geom_gcl_script2);

fprintf(geom_fn, geom_gcl_script1_start);
gts.surface.writem(geom_fn, "scene", "oogl");
fprintf(geom_fn, geom_gcl_script1_end);
fprintf(geom_fn, geom_gcl_script2_start);
gts.surface.writem(geom_fn, "scan", "oogl");
fprintf(geom_fn, geom_gcl_script2_end);

//v = gts.surface.vertex("scene");
//v?

pause();

fprintf(geom_fn, "(quit)");
close(geom_fn);






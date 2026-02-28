//
//
//
rfile libgts.so libgeomview

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
color_bkg = uniform(1,3); // choose random color
color_tgt = uniform(1,3); // choose random color

for (tp in 1:ceil(0.6*dim_scene))
{

  target_position = [0, tp, 0];

  tic();

  while (toc() < 600)
  {

    for (yaw_deg in [0:175:5])
    {

      spinner();

      //
      // background
      //
      gts.surface.new("scene",  <<desc="rectangle";color=color_bkg;bbox_x=[-dim_scene,dim_scene];bbox_y=[-dim_scene,dim_scene];bbox_z=0>>);

      yaw_rad = deg2rad * yaw_deg;

      //
      // target at:
      //
      gts.surface.new("t2",  <<desc="rectangle";color=color_tgt;bbox_x=[-1.5,1.5];bbox_y=[-3,3];bbox_z=0;transl=target_position;yaw=yaw_matrix(yaw_rad)>>);
      t2 = gts.surface.export("t2");

      gts.surface.add_point ("scene", t2.v);
      gts.surface.remove_rectangle2("scene", "t2");
      s = gts.surface.export("scene");
      rfile module_gnuplot_gts

      //
      // target without bottom
      //
      gts.surface.new("target", <<desc="rectangle";color=color_tgt;bbox_x=[-1.5,1.5];bbox_y=[-3,3];bbox_z=1.5;transl=target_position;yaw=yaw_matrix(yaw_rad)>>);
      //
      gts.surface.new("t3", <<desc="rectangle";color=color_tgt;bbox_x=-1.5;bbox_y=[-3,3];bbox_z=[0,1.5];transl=target_position;yaw=yaw_matrix(yaw_rad)>>);
      gts.surface.merge("target", "t3");
      //
      gts.surface.new("t4", <<desc="rectangle";color=color_tgt;bbox_x=1.5;bbox_y=[-3,3];bbox_z=[0,1.5];transl=target_position;yaw=yaw_matrix(yaw_rad)>>);
      gts.surface.merge("target", "t4");
      //
      gts.surface.new("t5", <<desc="rectangle";color=color_tgt;bbox_x=[-1.5,1.5];bbox_y=-3;bbox_z=[0,1.5];transl=target_position;yaw=yaw_matrix(yaw_rad)>>);
      gts.surface.merge("target", "t5");
      //
      gts.surface.new("t6", <<desc="rectangle";color=color_tgt;bbox_x=[-1.5,1.5];bbox_y=3;bbox_z=[0,1.5];transl=target_position;yaw=yaw_matrix(yaw_rad)>>);
      gts.surface.merge("target", "t6");

      //
      // scene
      //
      gts.surface.merge("scene", "target");
      geomview.plot_gts_surface("scene", "scene");

      sleep (0.3);

    } /*for (yaw_deg in [0:355:5])*/

  } /* while (toc() < 600) */

} /* for (tp in 1:ceil(0.6*dim_scene)) */







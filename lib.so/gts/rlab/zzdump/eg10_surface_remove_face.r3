//
//
// 
rfile libgts.so


//
// background
//

dim_scene = 5; //
color_bkg = uniform(1,3); // choose random color 

//
// background
//
gts.surface.new("bkg",  <<desc="rectangle";color=color_bkg;bbox_x=[-dim_scene,dim_scene];bbox_y=[-dim_scene,dim_scene];bbox_z=0>>);
b = gts.surface.export("bkg");

rng_tri = [...
    0,1,0; ...
    1,1,0; ...
    1,0,0; ...
    0,0,0];
gts.surface.from_vertices("hole", rng_tri, [1,2,3], <<color=[0,0,1]>>);

//gts.surface.new("hole",  <<desc="rectangle";color=[0,0,1];bbox_x=[-0.5*dim_scene,0.5*dim_scene];bbox_y=[-0.5*dim_scene,0.5*dim_scene];bbox_z=0>>);
//gts.surface.new("hole", <<...
//   desc="box";...
//   color=[0,0,1]; ...
//   bbox_x=[-1.5,1.5];bbox_y=[-3,3];bbox_z=[0,1.5]>>);


h = gts.surface.export("hole");


//if (!gts.surface.boolean("hole", "bkg", "scene", "2_OUT_1", <<color=color_bkg>>))
//if (!gts.surface.boolean("hole", "bkg", "scene", "2_IN_1", <<color=color_bkg>>))
if (!gts.surface.boolean("bkg", "hole", "scene", "1_OUT_2", <<color=color_bkg>>))
{
  s = gts.surface.export("scene");
  "showing scene !\n"?
  rfile module_geomview_scene
  stop()
}

stop()

if (!gts.surface.merge("scene", "bkg"))
{
  "showing bkg\n"?
  rfile module_geomview_scene
}
if (!gts.surface.merge("scene", "hole"))
{
  "showing bkg and hole\n"?
  rfile module_geomview_scene
}




















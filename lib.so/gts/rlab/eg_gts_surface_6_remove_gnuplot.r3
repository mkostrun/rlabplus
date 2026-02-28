//
//
// 
rfile libgts.so

clear(_plotname);

//
// background
//

dim_scene = 5; //
color_bkg = uniform(1,3); // choose random color 
rng(1,"uniform",[-0.1,0.1]);


//
// background
//
gts.surface.new("scene",  <<desc="rectangle";...
    color=color_bkg;bbox_x=[-dim_scene,dim_scene];bbox_y=[-dim_scene,dim_scene];bbox_z=0>>);
s = gts.surface.export("scene");
rfile module_gnuplot_gts
pause();

gts.surface.new("tgt",  <<desc="rectangle";...
    bbox_x=[-0.2*dim_scene,0.2*dim_scene];...
    bbox_y=[-0.2*dim_scene,0.2*dim_scene];...
    bbox_z=0>>);
t = gts.surface.export("tgt");
s = t;
rfile module_gnuplot_gts
pause();

//
// we try to remove a surface from "scene"
//
gts.surface.add_point("scene", t.v);
s = gts.surface.export("scene");
rfile module_gnuplot_gts
pause();

gts.surface.remove_surface2("scene", "tgt");
//gts.surface.remove_surface("scene", "tgt");
s = gts.surface.export("scene");
rfile module_gnuplot_gts





















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
gts.surface.new("scene",  <<desc="rectangle";...
    color=color_bkg;bbox_x=[-dim_scene,dim_scene];bbox_y=[-dim_scene,dim_scene];bbox_z=0>>);
b = gts.surface.export("scene");

// rfile module_geomview_scene

    
//
// we try to remove a surface from "scene"
//
vs1 = [...
    -1,-1,0; ...
    -1, 1, 0; ...
     1, 1, 0];

vs2 = [...
    -1, -1, 0; ...
    1, -1, 0; ...
    1,  1, 0];

vs3 = [...
    -dim_scene,    dim_scene, 0; ...
    -dim_scene,0.8*dim_scene, 0; ...
    -0.8*dim_scene,    dim_scene, 0; ...
    []];

vs4 = [...
    -0.8*dim_scene, 0.8*dim_scene, 0; ...
    -dim_scene,0.8*dim_scene, 0; ...
    -0.8*dim_scene,    dim_scene, 0; ...
    []];

gts.surface.add_point("scene", [vs1; vs2; vs3; vs4]);
bnew1 = gts.surface.export("scene");

s = bnew1;
rfile module_gnuplot_gts
pause();

//
gts.surface.remove_face("scene", vs1);

s = gts.surface.export("scene");
rfile module_gnuplot_gts
pause("Press 'enter'");

//
gts.surface.remove_face("scene", vs2);

s = gts.surface.export("scene");
rfile module_gnuplot_gts
pause("Press 'enter' once more!");

//
gts.surface.remove_face("scene", vs3);

s = gts.surface.export("scene");
rfile module_gnuplot_gts
pause("Press 'enter' once more please!");

//
gts.surface.remove_face("scene", vs4);

s = gts.surface.export("scene");
rfile module_gnuplot_gts
pause("Press 'enter' just once more! Pleeeeeese!");


























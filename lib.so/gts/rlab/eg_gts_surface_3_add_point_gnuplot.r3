//
//
// 
rfile libgts.so

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


//
// we try to remove a surface from "scene"
//
vs = [...
    -1,-1, 0; ...
    -1, 1, 0; ...
     1, -1, 0; ...
     1,  1, 0 ...
];
gts.surface.add_point("scene", vs);
bnew1 = gts.surface.export("scene");

s = bnew1;
rfile module_gnuplot_gts


















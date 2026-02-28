//
//
// 
rfile libgts.so

gnuwins(1);

dim_scene = sqrt(10); //
color_bkg = uniform(1,3); // choose random color

//
// we try to remove a surface from "scene"
//
//_u = 1;
_u = uniform(4,3);
_v = (2.*uniform(1,3) -1) ;
vs = [...
    -1,-1, 0; ...
    -1, 1, 0; ...
     1, -1, 0; ...
     1,  1, 0 ...
];
gts.surface.from_vertices("scene", [_v + vs .* _u; dim_scene .* vs]);
s = gts.surface.export("scene");
rfile module_gnuplot_gts
sleep(0.5);

gts.surface.from_vertices("hole", _v + vs .* _u);
s = gts.surface.export("hole");
rfile module_gnuplot_gts
sleep(0.5);

gts.surface.remove_surface2("scene", "hole");
s = gts.surface.export("scene");
rfile module_gnuplot_gts

















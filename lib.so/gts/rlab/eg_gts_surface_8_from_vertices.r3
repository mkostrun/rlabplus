//
//
//
rfile libgts.so

rfile libgeomview

GEOMVIEW = "/usr/local/bin/geomview ";
geom_fn = "| " + GEOMVIEW + " -c -";
geom_gcl_script1 = "(geometry cube { : foo })\n";
geom_gcl_script1_start = "(read geometry { define foo\n";
geom_gcl_script1_end   = "})\n";

//
//
//
cube_vx = [...
  0,0,0;...
  1,0,0;...
  1,1,0;...
  0,1,0;...
  0,0,1;...
  1,0,1;...
  1,1,1;...
  0,1,1;...
[]];
//idx_row = shuffle(1:cube_vx.nr);
//cube_vx = cube_vx[idx_row; ];

gts.surface.from_vertices("cube", cube_vx, ...
     [1,2,3; 1,3,4; 1,2,5; 2,5,6; 2,6,3; 3,6,7; 5,8,6; 6,7,8; 1,4,5; 5,4,8; 4,3,8; 3,8,7] );
geomview.plot_gts_surface("scene", "cube");

// rotate scene around its principal axes
geom_gcl_script2_template = "(transform scene scene focus rotate %f %f %f)";
for (i in 1:100)
{
  spinner();
  sprintf(geom_gcl_script2, geom_gcl_script2_template, 0.01 * gaussian(), 0.01 * gaussian(), 0.01 * gaussian());
  geomview.fprintf (geom_gcl_script2);
  sleep (0.05);
}

pause("Press enter to save the camera view to file, and exit geomview!");
geomview.snapshot("test.ps");
geomview.close();
system("(ps2pdf test.ps test.pdf; rm test.ps)&");










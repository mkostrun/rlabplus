//
//
//
rfile libgts.so

//
//
//
cube_vx = [...
  0,0,0;...
  0,0,1;...
  0,1,0;...
  1,0,0;...
  0,1,1;...
  1,0,1;...
  1,1,0;...
  1,1,1;...
[]];
//idx_row = shuffle(1:cube_vx.nr);
//cube_vx = cube_vx[idx_row; ];

//
gts.surface.new("s");
gts.surface.from_vertices("s", cube_vx, [1,3,4]);
gts.surface.writem("./test.oogl", "s");
pause();

gts.surface.new("s");
gts.surface.add_face("s",[0,0,0; 0,1,0; 1,0,0]);
gts.surface.add_face("s",[0,0,0; 0,1,0; 0,0,1]);
gts.surface.writem("./test.oogl", "s");
pause();

gts.surface.new("s");
gts.surface.add_face("s",[0.5,0.5,0], [0,0,0; 0,1,0; 1,1,0; 1,0,0]);
gts.surface.writem("./test.oogl", "s");
pause();



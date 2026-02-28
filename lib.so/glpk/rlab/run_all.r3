//
//
//

fns = ls("./eg*.r3");
for (_jj in 1:(int(1000*uniform()) + 1))
{
  spinner();
  fn = shuffle(fns,1);
  printf("%g: executing script %s\n", _jj, fn);
  NITER = int(10*uniform()) + 5;
  load(fn);
  printf("%g: Done\n\n", _jj);
}



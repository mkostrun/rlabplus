//
//
//
rfile libgts.so libgeomview

//sfn = ls("./data/*.gts");
sfn = "./data/horse4.gts";

for (i in range(sfn))
{
  surf_name = last(strsplt(sfn[i], "/"));
  surf_name = strsplt(surf_name, ".")[1];

  printf("Surface: %s\n", surf_name);
  gts.surface.readm(surf_name, sfn[i]);

  geomview.plot_gts_surface("scene", surf_name);

  pause("use mouse to rotate horse in position in which you want it saved to ps file, then press enter\n");

  if (isfile(surf_name + ".ps"))
  {
    rm(surf_name + ".ps");
  }
  geomview.snapshot(surf_name + ".ps");
  system("ps2pdf 2>/dev/null " + surf_name +".ps");
}




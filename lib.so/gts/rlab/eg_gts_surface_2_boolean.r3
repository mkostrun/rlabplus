//
//
//
rfile libgts.so

DO_NOT_PAUSE = 0;
DO_NOT_CLOSE = 0;
rfile module_geomview_scene

sfn = ls("./data/*.gts");
//[num2str([1:sfn.n]',"%.0f  ") + sfn]?

rsfn = [26, 29]; // prism \ sphere

//rsfn = [28, 29];

//pause()
surf_name = blank(rsfn);

for (i in range(rsfn))
{
  surf_name[i] = last(strsplt(sfn[rsfn[i]], "/"));
  surf_name[i] = strsplt(surf_name[i], ".")[1];

  printf("Surface: %s\n", surf_name[i]);

  gts.surface.readm(surf_name[i], sfn[rsfn[i]]);

  geomview(surf_name[i],geom_fn);

  if (!DO_NOT_PAUSE)
  {
    pause();
  }

}

gts.surface.boolean(surf_name[1],surf_name[2],"1/2","diff");
geomview("1/2",geom_fn);
printf("Prism without sphere:");
pause();

gts.surface.boolean(surf_name[1],surf_name[2],"1^2","inter");
geomview("1^2",geom_fn);
pause();

gts.surface.boolean(surf_name[1],surf_name[2],"1u2","union");
geomview("1u2",geom_fn);
pause();


if (!DO_NOT_CLOSE)
{
  fprintf(geom_fn, "(quit)");
  close(geom_fn);
}





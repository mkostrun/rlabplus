//
//
//
rfile libgts.so

clear(_plotname);

//sfn = ls("./data/*.gts");
//sfn = "./data/a10.gts";
//sfn = "./data/sponge.gts";
sfn = "./data/cutter.gts";
//sfn = "./data/sphere.gts";


rsfn = range(sfn);


for (i in rsfn)
{
  surf_name = last(strsplt(sfn[i], "/"));
  surf_name = strsplt(surf_name, ".")[1];

  printf("Surface: %s\n", surf_name);

  gts.surface.readm(surf_name, sfn[i]);

  s = gts.surface.export(surf_name);
  _plotname="cutter.pdf";
  rfile module_gnuplot_gts

  if ((length(rsfn)>1) && (i<length(rsfn)))
  {
    pause();
  }
}





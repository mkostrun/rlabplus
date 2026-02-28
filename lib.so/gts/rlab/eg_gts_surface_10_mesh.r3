//
//
//
rfile libgts.so

half_x = 2*pi;
NX = 50;
half_y = 2*pi;
NY = 50;

x = half_x ./ NX .* [-NX:NX];
y = half_y ./ NY .* [-NY:NY];
z = nan(length(x),length(y));
for (idx_x in range(x))
{
  x_i = x[idx_x];
  for (idx_y in range(y))
  {
    y_i = y[idx_y];
    r_i = sqrt(x_i.^2 + y_i.^2);
    if (r_i)
    {
      z[idx_x;idx_y] = sin(r_i .^ 2 ) ./ r_i .^ 2;
    }
    else
    {
      z[idx_x;idx_y] = 1;
    }
  }
}

gts.surface.new("s", <<desc="mesh";x=x;y=y;z=z>>);
geomview.plot_gts_surface("scene", "s");
pause("Press enter to save the camera view to file, and exit geomview!");
geomview.snapshot("mesh.ps");
geomview.close();
system("(ps2pdf mesh.ps mesh.pdf; rm mesh.ps)&");





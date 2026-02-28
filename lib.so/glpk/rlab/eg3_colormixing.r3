//
// lp_simplex.r: test of the GNU Linear Programming Kit, v.4.9
//
rfile libcolor libglpk.so


gnuwins (2);

// choose few light sources
N = 15;
NSOURCES = length(members(standard_illuminant));
printf("a");
i_sil = shuffle(1:NSOURCES)[1:N];
printf("b\n");
src = members(standard_illuminant)[i_sil];

xyY = ones(N,3);
XYZ = zeros(xyY);

for (i in 1:N)
{
  xyY[i;] = standard_illuminant.[src[i]].xy;
  xyY = xyY + 0;
  XYZ[i;] = xyz_from_xyY(xyY[i;]);
  XYZ = XYZ + 0;
}

// plot the colors
gnuwin (1);
gnuformat ("with points pt 6 ps 2");
gnuxlabel ("CIE1931-x");
gnuylabel ("CIE1931-y");
gnuplot (xyY[;1,2]);


// range x
min_x = min(xyY[;1]);
max_x = max(xyY[;1]);
ndx = 100;
dx  = (max_x - min_x) / ndx;

YT = 1;

// specify lp problem and solution
lpopts = <<>>;
lpopts.method = "primal";
# lpopts.stdout = stderr();
ylpx = <<>>;
ylpx.problem       = "lp";


data_xy1y2 = zeros(ndx+1,3);
for (i in 0:ndx)
{
  xt = min_x + dx * i;

  ylpx.objective   = (XYZ[;1] + XYZ[;3])';
  ylpx.constraints = [XYZ[;2], (xt-1) .* XYZ[;1] + xt .* XYZ[;3]]';
  ylpx.bounds_col  = [zeros(N,1), inf(N,1)];
  ylpx.bounds_row  = [YT, YT; -xt*YT, -xt*YT];

  ylpx.opt_direction = "max";
  s_max = _glpk_solve(ylpx, lpopts);

  ylpx.opt_direction = "min";
  s_min = _glpk_solve(ylpx, lpopts);

  data_xy1y2[i+1;] = [xt, YT/(YT+s_max.objective), YT/(YT+s_min.objective)];
}

gnuwin(2);
gnuformat ([...
  "with points pt 6 ps 2", ...
  "with lines lt 1 lw 1 lc @black@", ...
  "with lines lt 1 lw 1 lc @black@", ...
[]]);
gnuxlabel ("CIE1931-x");
gnuylabel ("CIE1931-y");
gnuplot (<<a1=xyY[;1,2];a2=data_xy1y2[;1,2];a3=data_xy1y2[;1,3]>>);







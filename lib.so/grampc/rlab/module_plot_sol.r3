//
//
//
_pfn = "./eg_grampc_" + num2str(EG,"%.0f");

gnuwins(1);

gnuwin (1);
gnulimits (0,Tsim,,);
gnulimits2(0,Tsim,,);
gnuxtics (1,10);
gnuytics (0.5,5);
gnuy2tics(0.5,5);
gnuxlabel("Time (s)");
gnuylabel("State Amplitude" ,"Control Amplitude");
gnucmd ("set grid xtics ytics");

NX = y.sol.x.nc - 1;
NU = y.sol.u.nc - 1;

_leg = [["State: ", ""] + num2str(1:NX,"x_%.0f"), ...
    ["Control: ", ""] + + num2str(1:NU,"u_%.0f")];
_fmt_x = [...
    "with lines lt 1 lw 2 lc rgb 'red' axes x1y1", ...
    "with lines lt 1 lw 2 lc rgb 'pink' axes x1y1", ...
    "with lines lt 1 lw 2 lc rgb 'orange' axes x1y1", ...
    "with lines lt 1 lw 2 lc rgb 'red' axes x1y1", ...
    "with lines lt 1 lw 2 lc rgb 'pink' axes x1y1", ...
    "with lines lt 1 lw 2 lc rgb 'orange' axes x1y1", ...
[]];
_fmt_u = [...
    "with lines lt 1 lw 2 lc rgb 'blue' axes x1y1", ...
    "with lines lt 1 lw 2 lc rgb 'purple' axes x1y2", ...
    "with lines lt 1 lw 2 lc rgb 'violet' axes x1y1", ...
    "with lines lt 1 lw 2 lc rgb 'purple' axes x1y2", ...
[]];

// assemble the plot data:
_plot_data = <<>>;
for (i in 1:NX)
{
  s_x = num2str(i, "a%02.0f");
  _plot_data.[s_x] = y.sol.x[;1,i+1];
}
for (i in 1:NU)
{
  s_u = num2str(i, "b%02.0f");
  _plot_data.[s_u] = y.sol.u[;1,i+1];
}

gnulegend(_leg);
gnuformat([_fmt_x[1:NX], _fmt_u[1:NU]]);
gnuplot(_plot_data, _pfn + ".pdf");

gnuwin(1);
gnulimits (0,Tsim,0,);
gnuxtics (1,10);
gnuxlabel("Time (s)");
gnuylabel("Cost");
gnucmd ("set grid xtics ytics");
gnulegend([...
    "Cost", ...
    "Augmented Cost", ...
    []]);
gnuformat([ ...
    "with lines lt 1 lw 2 lc rgb 'red' axes x1y1", ...
    "with lines lt 1 lw 2 lc rgb 'orange' axes x1y1", ...
    []]);
gnuplot(a1=y.sol.j, _pfn+"_cost.pdf");



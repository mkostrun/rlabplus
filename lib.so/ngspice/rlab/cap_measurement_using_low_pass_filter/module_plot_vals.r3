//
//
//

gnuwin (1);
gnulimits(cap_Range[1],last(cap_Range),0,3.3);
gnuxtics (10,2);
gnuytics (0.5,5);
gnuformat ([...
"with lines lt 1 lw 1 lc @black@", ...
[]]);
gnuxlabel("Sensor Capacitance / pF");
gnuylabel("Voltage C_3 / V");
gnulegend(members(mean_vcap));
gnuplot( mean_vcap );

if (range_vcap.nr > 1)
{
  gnuwin (2);
  gnulimits(,,,);
  gnuxtics (10,2);
  gnuytics (1,5);
  gnuformat ([...
  "with lines lt 1 lw 1 lc @black@", ...
  []]);
  gnuplot( range_vcap );
}


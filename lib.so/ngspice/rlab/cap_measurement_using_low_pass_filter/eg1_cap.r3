//
//
//
rfile libngspice.so

gnuwins (1);

fn = "./caplowpass1.cir";

ngspice.init();

spicecir = reads(fn);

ngspice.runckt(spicecir);
while(ngspice.isrunning())
{ sleep(0.1); }
s = ngspice.getvals();


gnuwin (1);
gnulimits(8,10,0,3.4);
gnuxtics (0.2,2);
gnuxlabel("Simulation Time (us)");
gnuytics (1,5);
gnuylabel("Voltage (V)");
gnulegend ([...
  "Generated Square-waves", ...
  "Voltage across C_1", ...
[]]);
gnuformat ([...
  "with lines lt 1 lw 1 lc @black@", ...
  "with lines lt 1 lw 2 lc @red@", ...
[]]);
gnuplot( << ...
  a=[s.data.time ./ 1e-6, s.data.v_1]; ...
  b=[s.data.time ./ 1e-6, s.data.v_2] ...
>> );


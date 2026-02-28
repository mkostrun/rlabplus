//
//
//
rfile libngspice.so

gnuwins (1);

fn = "./caplowpass2.cir";
ngspice.init();

spicecir0 = reads(fn);

tperiod = 1e-6;
cap1 = 7e-12;
res1 = 5e3;
duty = 1/8;
tperon = duty * tperiod;

rfile module_run_spice

if(exist(s.data))
{
  s.data.i_c1 = ndiff([s.data.time,s.data.v_2]) .* 7e-12;
  s.data.i_c2 = ndiff([s.data.time,s.data.v_3]) .* 1e-9;
}

"c_sensor = " + text(cap1*1e12,"%5.2f pF")
"v_3 = " + text(mean(s.data.v_3),"%5.2f +/-")+ text(var(s.data.v_3) ^ 0.5,"%.3f V")

rfile module_plot_vi

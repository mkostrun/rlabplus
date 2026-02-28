//
//
//
rfile libngspice.so

gnuwins (2);

fn = "./caplowpass2.cir";
tperiod = 1e-6;

ngspice.init();

range_vcap = <<>>;

spicecir0 = reads(fn);

cap_Range  = [7,70];
res_Range  = [2:10:0.5];
duty_Range = [1:7] ./ 8;

for (ires in range(res_Range))
{
  // set value R1
  spicecir = spicecir0;
  __i = find(strindex(spicecir,".param res1"));
  spicecir[__i] = ".param res1=" + text(res_Range[ires] * 1e3);

  sr = text(res_Range[ires],"%05.2f");

  range_vcap.[sr] = [];
  for (idty in range(duty_Range))
  {
    // set duty cycle
    tperon = duty_Range[idty] * tperiod;
    __i = find(strindex(spicecir,".param tperon"));
    spicecir[__i] = ".param tperon=" + text(tperon);

    data = [];
    for (icap in range(cap_Range))
    {
      // set value C1
      __i = find(strindex(spicecir,".param cap1"));
      spicecir[__i] = ".param cap1=" + text(cap_Range[icap] * 1e-12);

      //
      ngspice.runckt(spicecir);
      while(ngspice.isrunning())
      { spinner();sleep(0.1); }
      s = ngspice.getvals();

      data = [data, mean(s.data.v_3)];
    }
    range_vcap.[sr] = [range_vcap.[sr]; duty_Range[idty], max(data) - min(data)];
  }
}

rfile module_plot_dutycycle

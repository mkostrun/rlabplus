//
//
//
rfile libngspice.so

gnuwins (2);

fn = "./caplowpass2.cir";

ngspice.init();

mean_vcap = <<>>;
range_vcap = [];

spicecir0 = reads(fn);

// fixed values
tperiod = 1e-6;
duty = 1/8;
tperon = duty * tperiod;

cap_Range=7 * [1:10];
res_Range=5.4;

for (ires in range(res_Range))
{
  sr = text(res_Range[ires],"%05.2f");

  mean_vcap.[sr] = [];

  res1 = res_Range[ires] * 1e3;

  for (icap in range(cap_Range))
  {
    cap1 = cap_Range[icap] * 1e-12;

    rfile module_run_spice

    mean_vcap.[sr] = [mean_vcap.[sr]; cap_Range[icap], mean(s.data.v_3), var(s.data.v_3).^0.5];
  }

  range_vcap = [range_vcap; res_Range[ires], max(mean_vcap.[sr][;2]) - min(mean_vcap.[sr][;2])];
}

"R1 /kOhm    V_max-V_min /V"
range_vcap

rfile module_plot_vals

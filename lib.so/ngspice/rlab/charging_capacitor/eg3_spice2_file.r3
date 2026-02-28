//
// rlab and ngspice
//
if (!exist(spice))
{
  rfile ngspice
}
if (!exist(spice.raw))
{
  rfile ngspice
}

fn_ckt = "zz_eg3_c2.spc";
fn_raw = "mytestspice_eg3.raw";

// rising edge formula
vfunc = function(t,p)
{
  // p:
  //  p[1] -> barV
  //  p[2] -> t0
  //  p[3] -> tau
  // model
  //  V(t) = barV * (1 - exp(-(t-t0)/tau))
  //
  res = p[1].*(1 - exp(-(t-p[2])./p[3]));
  return res;
};


// constants
ns = 1e-9;
pF = 1e-12;
mohm = 1e6;

// circuit parameters - do not interfere with switching circuit
//  unless, ofcourse, you know what you are doing
// all units are SI
V1 = 5;
C1 = 125 * pF;
R1 = 100 * mohm;
R2 = 1200;

range_C2 = [0.1, 0.2, 0.5, 1, 2, 5, 10, 20, 50, 100, 200];

y = <<>>;
lgd_y = blank(0,0);

coef_fit = [];

for (k in 1:10)
{

tic();

for (i in range(range_C2))
{
  // label to avoid lexicographic ordering
  li = text(i,"%02.0f");

  C2 = range_C2[i] * pF;

  // create appropriate ngspice script:
  // write all data to file !!! can be improved upon
  spicecrkt= [ ...
    "* rc circuit: 2 rc circuits connected by a switch"; ...
    "v1 1 0 dc " + text(V1); ...
    "r1 1 2 " + text(R1); ...
    "c1 2 0 " + text(C1); ...
    "r2 4 5 " + text(R2); ...
    "c2 5 0 " + text(C2); ...
    "vc 3 0 0 pulse(0 1 0 1n 1n 5u 1s)";  ...
    "s1 2 4 3 0 switch"; ...
    ".model switch sw(vt=0.09 ron=0.01)"; ...
    ".ic v(2)=" + text(V1) + " v(4)=0 v(5)=0"; ...
    ".control"; ...
    "tran 0.05ns 200ns"; ...
    "write '" + fn_raw + "'"; ...
    ".endc"; ...
    ".end" ...
  ];

  // call ngspice and execute the script
  system("/bin/rm -rf " + [fn_raw, fn_ckt]);
  if (spice.exec(spicecrkt, fn_ckt, stderr(), stderr()))
  {
    stop();
  }

  // get the output:
  printf("Waiting ");
  while(!isfile(fn_raw))
  { sleep(0.01); }
  printf("Done!\n");
  s = spice.raw(fn_raw);

  lgd_y  = [lgd_y, ...
    text(R2, "R_2 = %g {/Symbol W}, ") + text(range_C2[i], "C_2 = %4.1f pf") ...
    ];

  y.[li] = [s.data.time, s.data.v_5] ./ [ns, 1];

  // fit this to the rising edge formula
  // find vmax
  _vmax = lastr(y.[li])[2];
  _dt = max(diff(y.[li][;1]));
  _ta = min(findroot(y.[li], 0.1 * _vmax));
  _tb = min(findroot(y.[li], 0.9 * _vmax));
  i4a = min(find(abs(y.[li][;1] - _ta) < _dt));
  i4b = min(find(abs(y.[li][;1] - _tb) < _dt));
  tf = y.[li][i4a:i4b;1];
  vf = y.[li][i4a:i4b;2];
  pf = zeros(1,3);
  pf[1] = _vmax;
  pf[2] = _ta;
  pf[3] = range_C2[i];

  //
  cf = odrfit (vf, tf, pf, vfunc, odropts);
  coef_fit = [coef_fit; [range_C2[i], cf.coef]];
}

printf("Calculation using files lasted %g sec!\n", toc());

}

// plot the quantity of interest
gnuwins (2);

gnuwin (1);
gnulimits (0, 50, 0, 5);
gnuxtics  (5, 5);
gnuxlabel ("Time / ns");
gnuytics  (0.5,5);
gnuylabel ("Voltage (V)");
gnulegend (lgd_y);
gnuplot (y, "r2b_vt.eps");

gnuwin (2);
gnulimits  (0.1, 200, 0.1, 200);
gnulimits2 (,,0, 1);
gnuscale  ("log","log");
gnuxtics  ([0.1,0.2,0.5,1,2,5,10,20,50,100,200]);
gnuxlabel ("Capacitor C_2 (pF)");
gnuytics  ([0.1,0.2,0.5,1,2,5,10,20,50,100,200],,,"nomirror");
gnuylabel ("Time / ns", "V/V_{max}");
gnuy2tics (0.1,1);
gnucmd    ("unset grid; set grid xtics ytics y2tics");
gnulegend ( ["Time constant for charging C_2 (ns)", "Final voltage ratio V(C_2)/V_{max}" ]);
gnuformat (["with lines lw 2 lc rgb 'green' axes x1y1", "with lines lw 2 lc rgb 'red' axes x1y2"]);
gnuplot   (<<a=coef_fit[;1,4];b=coef_fit[;1,2]./[1,V1]>>, "r2b_tc.eps");




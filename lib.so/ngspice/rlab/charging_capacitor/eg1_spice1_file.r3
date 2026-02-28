//
//
//
if (!exist(spice))
{
  rfile ngspice
}
if (!exist(spice.raw))
{
  rfile ngspice
}

fn_ckt = "zz_eg1_c2.spc";
fn_raw = "mytestspice_eg1.raw";

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
C2 = 5 * pF;
R2 = 1200;

// create appropriate ngspice script
spicecrkt= [ ...
  "* rc circuit: 2 rc circuits connected by a switch"; ...
  "v1 1 0 dc " + text(V1); ...
  "r1 1 2 " + text(R1); ...
  "c1 2 0 " + text(C1); ...
  "r2 4 5 " + text(R2); ...
  "c2 5 0 " + text(C2); ...
//  vc-voltage controlled switch between (r1,c1) and (r2,c2)"
//  the ciruit is switches at time ~1ns
  "vc 3 0 0 pulse(0 1 0 1n 1n 5u 1s)";  ...
  "s1 2 4 3 0 switch"; ...
  ".model switch sw(vt=0.09 ron=0.01)"; ...
//  transition
  ".ic v(2)=" + text(V1) + " v(4)=0 v(5)=0"; ...
//  do the output
  ".control"; ...
  "tran 0.01ns 50ns"; ...
  "write '" + fn_raw + "'"; ...
  ".endc"; ...
  ".end" ...
];

// call ngspice and execute the script
system("/bin/rm -rf " + [fn_raw, fn_ckt]);
spice.init();
spice.runckt(spicecrkt, fn_ckt, stderr(), stderr());

// get the output:
printf("Waiting ");
while(!isfile(fn_raw))
{ spinner();
  sleep(0.5);
}
printf("Done!\n");
s  = spice.raw(fn_raw);
s1 = spice.getvals();

y = <<>>;
y.["Current through R_2"] = [...
  s.data.time, (s.data.v_4 - s.data.v_5)/R2 ...
] ./ [ns, 1e-3];
y.["Voltage across capacitor C_2"] = [s.data.time, s.data.v_5] ./ [ns,1];
y.["Switching Voltage"] = [s.data.time, s.data.v_3] ./ [ns,1];

// plot
gnuwins (1);

gnulimits (0, 50, 0, 5);
gnuxtics  (5, 5);
gnuxlabel ("Time / ns");
gnuytics  (0.5,5);
gnuylabel ("Voltage (V), Current (mA)");
gnulegend (members(y));
gnuplot (y);




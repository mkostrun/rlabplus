//
//
//

// spice.init(stderr());
spice.init();

// create appropriate ngspice script
// start from the sample script
fn_cir = "./xp.cir";
spicecrkt0=reads(fn_cir);

// rfile ngspice
lamp = "xp-" + prompt("Which XLamp Chip (e/g) ", "g", ["e","g"]);

gnuwins (1);

theta_BJ = <<>>;
theta_BJ.["xp-e"] = 9;
theta_BJ.["xp-g"] = 4;

RSER_0 = <<>>;
RSER_0.["xp-e"] = 0.64221;
RSER_0.["xp-g"] = 0.28172;


// constants
usec = 1e-6;
nsec = 1e-9;
msec = 1e-3;
uF = 1e-6;
mohm = 1e-3;
Mohm = 1e6;

// DC sweep parameters
v0 = 1/128;
v1 = 4;
dv = v0;
V_range = [v0:v1:dv]';

tb = 25;
tj = tb;
DT_ABSERR = 0.01;

vi_temp = zeros(len(V_range),3);


// do everything at T=25 deg C:
V0 = v0;
V1 = v1;
DV = dv;
TJ = 25;
RPAR = 1e10;
rfile module_run_spice
vi_spice_25C = [v2, irs];
TJ = 150;
rfile module_run_spice
vi_spice_150C = [v2, irs];

for (i in range(V_range))
{
  spinner();

  V0 = V_range[i];
  V1 = V0;
  DV = dv;
  TJ = tb;
  rfile module_run_spice

  v_last = v2;
  i_last = irs;

  DT = theta_BJ.[lamp] * v_last * i_last;

  while (DT > DT_ABSERR)
  {
    smiley();
    DT_old = DT;
    TJ = tb + DT;
    rfile module_run_spice
    v_last = v2;
    i_last = irs;
    DT = theta_BJ.[lamp] * v_last * i_last;
    if (abs(DT-DT_old)<DT_ABSERR)
    { break; }
  }
  vi_temp[i;] = [v_last, i_last, TJ];
}
gnulegend(members(data));
gnuscale("lin", "log");
gnuplot( vi_temp[;1,2] );

spice.kill();


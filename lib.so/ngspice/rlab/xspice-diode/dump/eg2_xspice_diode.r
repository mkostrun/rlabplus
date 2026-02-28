//
//
//
gnuwins (1);

// spice.init(stderr());
spice.init();

fn_cir = "./xp_xspice.cir";
spicecrkt0=reads(fn_cir);

// constants
usec = 1e-6;
nsec = 1e-9;
msec = 1e-3;
uF = 1e-6;
mohm = 1e-3;
Mohm = 1e6;

tb=25;
RPAR = 1e10;

// DC sweep parameters
DV = 1/64;
v0 = dv;
v1 = 4;
V_range = [v0:v1:DV]';

vi = zeros(len(V_range),2);

TJA_range =[0:150:30];
// TJA_range =[0];

data = <<>>;

tic();
for (j in range(TJA_range))
{

TJA = TJA_range[j];
s_TJA = text(TJA, "%04.0fC");

spinner();
for (i in range(V_range))
{
  V0 = V_range[i];
  V1 = V0;
  TJ = tb;
  rfile module_run_xspice

  v_last = v1;
  i_last = irs;

  vi[i;] = [v_last, i_last];
}

data.[s_TJA] = vi;

}
printf("calculation lasted %g sec\n", toc());


gnulegend(members(data));
gnuscale("lin", "log");
gnuplot( data );

spice.kill();


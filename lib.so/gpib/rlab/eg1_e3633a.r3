//
//
//
ITEST = 0.1;
VSTART = 1.0;
rfile module_config_ps

NSTEPS = 4;
VTEST = 1.0;

// Rise the output voltage until
// ITEST is reached:
ps.v.level(0);   
ps.output.on();
for (i in 1:NSTEPS)
{
  ps.v.level(VTEST./NSTEPS .* i);    // volts
  ps.measure.i()?
}

t_f_s = 10;
idx_tic = 13;

data_tv = [];
tic(idx_tic);
do
{
  data1 = [toc(idx_tic), ps.v.level()];
  data_tv = [data_tv; data1];
  data1?
  sleep(0.1);
}
while(toc(idx_tic) < t_f_s);

ps.output.off();







 

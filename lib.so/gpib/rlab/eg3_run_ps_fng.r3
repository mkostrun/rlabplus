//
//
//
IMAX = 2.5;

ITEST = prompt("I_TEC (A) ", ITEST, [1,1.5,2,2.3]);
rfile module_config_ps

NSTEPS = 4;
VTEST = ITEST * 1.2;

// Rise the output voltage until
// ITEST is reached:
ps.v.level(0);

rfile module_config_fg

"sleeping 120s:"?
sleep(120);
"Done\n"?

T_S = 60;

//
// start camera triggers
//
fng.output.on();
fng.trigger.source.imm();

tic();
sleep(1);

//
// Turn on TEC
//
//
ps.v.level(0);
ps.output.on();
for (i in 1:NSTEPS)
{
  ps.v.level(VTEST./NSTEPS .* i);    // volts
  if (ps.measure.i()>IMAX)
  {
    ps.output.off();
    fng.output.off();
    stop("OOpsie!\n");
  }
}

while (toc() < 10)
{
  spinner();
  if (ps.measure.i() > IMAX)
  {
    fng.output.off();
    stop("OOpsie!\n");
  }
  sleep (0.1);
}

//
// Turn off TEC
//
ps.output.off();


//
//
//
while (toc() < T_S)
{
  spinner();
  fng.wait();
  sleep(0.1);
}

//
//
//



fng.output.off();






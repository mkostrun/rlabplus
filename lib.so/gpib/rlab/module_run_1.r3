//
//
//
tic(TIMER_TEC);
fng.trigger.now();


//
// Start the FNG to record the frames
//
"\tWaiting to start TEC: " ?
while (toc(TIMER_TEC) < T_TEC_ON)
{
  spinner();
}
"Done!\n"?

//
// Turn on TEC
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

//
// Wait with TEC running, then turn it off
//
"\tTEC run: " ?
while (toc(TIMER_TEC) < T_TEC_OFF)
{
  spinner();
}
ps.output.off();
"Completed!\n"?

//
// Record cooldown on the camera
//
"\tWaiting for cooldown: " ?
while (toc(TIMER_TEC) < ceil(N_CYCLES / F_BURST))
{
  spinner();
}
"Cool enough!\n\n"?










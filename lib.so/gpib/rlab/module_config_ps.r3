//
//
//
if (!exist(ibask))
{
  rfile libgpib.so
}

if (!exist(ibask))
{
  stop("GPIB library failed to load: Cannot continue!\n");
}

if (!exist(ps))
{
  rfile liblab_ps_e3613a_gpib
}

if (!exist(ITEST))
{
  stop("Need ITEST to continue");
}

ps.output.off();
ps.i.level(ITEST); // amps, the current in CC mode
ps.v.level(0);   // volts

printf("I_TEC (A) = %g\n", ITEST);














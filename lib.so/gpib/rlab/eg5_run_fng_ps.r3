//
//
//
IMAX = 2.5;

r_ITEST = prompt("I_TEC (A) ", r_ITEST, [0.5, 1, 1.5, 1.8, 2, 2.3]);

N_CYCLES = 200;
F_BURST = 10;
rfile module_config_fg_burst

printf("Set camera to collect %g frames and press record button now", N_CYCLES * length(r_ITEST));
pause();

TIMER_TEC = 13;
T_TEC_ON = 1;
T_TEC_OFF = 10;

for (ITEST in r_ITEST)
{
  // configure PS for ITEST in CC mode:
  NSTEPS = 4;
  VTEST = ITEST * 1.2;
  rfile module_config_ps

  // run one camera session
  rfile module_run_1

  //
  // Wait before starting the next measurement
  //
  if (ITEST !=last(r_ITEST))
  {
    sleep(floor(600*ITEST./max(r_ITEST)) + 60);
  }
}








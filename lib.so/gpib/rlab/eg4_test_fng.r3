//
//
//
N_CYCLES = 200;
F_BURST = 10;
rfile module_config_fg_burst

TIMER_TEC = 13;
T_TEC_ON = 1;
T_TEC_OFF = 10;

tic(TIMER_TEC);
fng.trigger.now();

"Waiting to start TEC: " ?
while (toc(TIMER_TEC) < T_TEC_ON)
{
  spinner();
}
"Done!\n"?

"TEC run: " ?
while (toc(TIMER_TEC) < T_TEC_OFF)
{
  spinner();
}
"Completed!\n"?

"Waiting for cooldown: " ?
while (toc(TIMER_TEC) < ceil(N_CYCLES / F_BURST))
{
  spinner();
}
"Cool enough!\n"?







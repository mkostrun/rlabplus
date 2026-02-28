//
//
//
rfile module_config_fg

T_S = 50;

fng.output.on();
//fng.trigger.source.imm();
tic();
while (toc() < T_S)
{
  spinner();
  fng.wait();
  sleep(0.5);
}

fng.output.off();






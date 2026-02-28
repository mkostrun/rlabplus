//
//
//
if (!exist(fng))
{
  rfile liblab2_fng_agilent_332XX_gpib

  fng = fng_agilent332XX_class(9);
}

if (!exist(N_CYCLES) || !exist(F_BURST))
{
  stop("Cannot continue with N_CYCLES\n");
}

V_ON  = 3.5;
V_OFF = 0;

//N_CYCLES = 5;
//F_BURST = 10;             // Hz, rate of the pulses for thermal camera

fng.rst();
sleep(0.5);


//
// pulse amplitude
//
fng.output.off();

fng.fn("SQU");
fng.freq(F_BURST);
fng.v.high(V_ON);
fng.v.low (V_OFF);

// burst mode
fng.burst.mode.trig();
fng.burst.ncycles(N_CYCLES);
fng.burst.period(ceil(N_CYCLES/F_BURST) + 1);
fng.trigger.source.bus();
fng.burst.state.on();

// get ready now:
sleep(0.5);
fng.output.on();

//
//
//






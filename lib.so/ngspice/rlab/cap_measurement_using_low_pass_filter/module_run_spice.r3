//
// requires:
//      spicecir0
//      tperiod
//      tperon
//      res1
//      cap1
//
// provides:
//      s = <<data;variables;points;plot>>
//      of which s.data is what we need
//

spicecir = spicecir0;

// set period
__i = find(strindex(spicecir,".param tperiod"));
spicecir[__i] = ".param tperiod=" + text(tperiod);

// set duty cycle
__i = find(strindex(spicecir,".param tperon"));
spicecir[__i] = ".param tperon=" + text(tperon);

// set value R1
__i = find(strindex(spicecir,".param res1"));
spicecir[__i] = ".param res1=" + text(res1);

// set value C1
__i = find(strindex(spicecir,".param cap1"));
spicecir[__i] = ".param cap1=" + text(cap1);

// run spice and wait
ngspice.runckt(spicecir);
while(ngspice.isrunning())
{
  spinner();
  sleep(0.01);
}

// get values
s = ngspice.getvals();

if(!exist(s.data))
{ stop("Uh-oh, something is not right here!"); }


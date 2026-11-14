//
//
//
rfile libngspice.so

i1src_0 = 2;
r1val = 1e1;
c1val = 1e-1;
vstat = 1;

t_step = 0.001;
t_end  = 100;

spicecir = [ ...
  "* solving thermal circuits with rlab", ...
  ".parameter r1val=" + num2str(r1val,"%e"), ...
  ".parameter c1val=" + num2str(c1val,"%e"), ...
  "i1 0 1	dc " + num2str(i1src_0,"%e"), ...
  "r1 1 0 'r1val'", ...
  "c1 1 0 'c1val'", ...
  "vstat 10 0 dc " + num2str(vstat,"%e"), ...
  ".ic v(1)=0", ...
  ".tran" + num2str(t_step," %e") + num2str(t_end," %e") , ...
  ".end", ...
[]];

_ngspice_setup = <<>>;
_ngspice_setup.stdout = term();


spicecir'?

//
//
//
ngspice.init(_ngspice_setup);
ngspice.command("reset");
ngspice.circuit(spicecir);
ngspice.command("stop when v(1) > 5 when 0.5 < v(10)");
ngspice.command("stop when v(1) < 3 when 0.5 > v(10)");

//
// run with heat engine for the first time:
//
ngspice.command("bg_run");
while(ngspice.isrunning())
{ spinner();sleep(0.01); }

// now keep cycling OFF/ON around desired set point
for (i in 1:3)
{
  //
  // run with heat engine off
  //
  ngspice.command("alter vstat=0");
  ngspice.command("alter i1=0");
  ngspice.command("bg_resume");
  while(ngspice.isrunning())
  { spinner();sleep(0.01); }

  //
  // run with heat engine on
  //
  ngspice.command("alter vstat=1");
  ngspice.command("alter i1="+num2str(i1src_0,"%e"));
  ngspice.command("bg_resume");
  while(ngspice.isrunning())
  { spinner();sleep(0.01); }
}

s = ngspice.getvals();

//
// plot it
//
gnuwins (1);
gnulimits (0,last(s.data.time));
gnuxlabel ("Time (s)");
gnuxtics  (0.5,5);
gnuytics  (1,5);
gnucmd    ("set grid xtics ytics");
gnuylabel ("Voltage (V)");
gnulegend ([...
  "PID Limits", ...
  "", ...
  "v(10): PID (I_1) STATE: 0-Off, 1-On", ...
  "v(1)", ...
[]]);
gnuformat ([...
  "with lines lt 1 lw 2 lc rgb 'dark-pink'", ...
  "with lines lt 1 lw 2 lc rgb 'dark-pink'", ...
  "with lines lt 1 lw 4 lc rgb 'gray50'", ...
  "with lines lt 1 lw 4 lc rgb 'blue'", ...
[]]);
gnuplot(<<...
  a1=[s.data.time,3]; ...
  a2=[s.data.time,5]; ...
  a3=[s.data.time,s.data.v_10]; ...
  b1=[s.data.time, s.data.v_1]; ...
>>, "fig-pid-stop.png");












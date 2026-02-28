//
//
//
if (!validgts(s))
{
  EOF
}

if (!exist(I))
{
  I = 1;
}

gnuwins(2);

gnuwin(I);

gnulimits(,,,,-1,1);
gnucmd ("set view 53,13" );
gnucmd ("set hidden" );
gnugts (s);
gnuformat ("with lines lt 1 lw 0.5 lc rgb 'black'");
if (strlen(_plotname)>0)
{
  gnusplot(s, _plotname);
}
else
{
  gnusplot(s);
}








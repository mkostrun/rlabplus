//
//
//

// fix parameters
spicecrkt = spicecrkt0;

if(exist(RSER))
{
  __i=find(strindex(spicecrkt,".param rser"));
  spicecrkt[__i] = ".param rser="+text(RSER);
}
if(exist(RPAR))
{
  __i=find(strindex(spicecrkt,".param rpar"));
  spicecrkt[__i] = ".param rpar="+text(RPAR);
}
if (exist(lamp))
{
  __i=find(strindex(spicecrkt,"'lamp'"));
  if (!isempty(__i))
  {
    if (lamp == "xp-e")
    {
      spicecrkt[__i] = gsub("XPE", "'lamp'", spicecrkt[__i]).string;
    else if (lamp == "xp-g")
    {
      spicecrkt[__i] = gsub("XPG", "'lamp'", spicecrkt[__i]).string;
    }}
  }
}
__i=find(strindex(spicecrkt,"'vstart'"));
spicecrkt[__i] = gsub(text(V0), "'vstart'", spicecrkt[__i]).string;
__i=find(strindex(spicecrkt,"'vend'"));
spicecrkt[__i] = gsub(text(V1), "'vend'", spicecrkt[__i]).string;
__i=find(strindex(spicecrkt,"'vdelta'"));
spicecrkt[__i] = gsub(text(DV), "'vdelta'", spicecrkt[__i]).string;

// fix initial junction temperature
__i=find(strindex(spicecrkt,"'temp'"));
spicecrkt[__i] = gsub(text(TJ), "'temp'", spicecrkt[__i]).string;


spice.runckt(spicecrkt);
while(spice.isrunning())
{ sleep(0.01); }
s = spice.getvals();


irs = -s.data.i_v1;
v1  =  s.data.v_1;
v2  =  s.data.v_2;

irp = v1 ./ RPAR;

eeQ_table = [irs, (irs - irp) .* v1 ./ irs ./ v2];


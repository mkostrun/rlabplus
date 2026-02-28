//
// liblab2_fng_agilent_332XX_gpib.r3
//

if (!exist(ibdev))
{
  rfile libgpib.so
}

static(_THIS_LIB, _LIB_DEBUG);
if (!exist(_THIS_LIB))
{
  _THIS_LIB = "liblab_fng_srs_ds345_gpib: ";
}
if (!exist(_LIB_DEBUG))
{
  _LIB_DEBUG = 0;
}
  
//
// local service functions
//
static(_read_from_device, _write_to_device, _write_raw);
_read_from_device = function (gid, n )
{
  _THIS_CMD = "_read_from_device";
  if (exist(n))
  {
    rval = ibrd(gid, n);
    if (_LIB_DEBUG)
    {
      printf("%s: %s: %s\n",_THIS_LIB, _THIS_CMD, ...
          "[" + num2str(rval, "%.0f",",") + "]");
    }
  }
  else
  {
    rval = ibrd(gid);
    if (isempty(rval))
    {rval="(null)";}
    chomp(rval);
    if (_LIB_DEBUG)
    {
      printf("%s: %s: %s\n",_THIS_LIB, _THIS_CMD, rval);
    }
  }
  return rval;
};

_write_to_device = function (gid, _cmd, _pause )
{
  rval = "";
  _THIS_CMD = "_write_to_device";

  if (!exist(_pause))
  { _pause = 0.0; }

  for (_c in _cmd)
  {
    rval = ibwrt(gid, _c);

    if (_LIB_DEBUG)
    { printf("%s: %s: %s\n",_THIS_LIB, _THIS_CMD, _c); }

    if (_pause)
    { sleep(_pause); }
  }

  return rval;
};

_write_raw = function (gid, x, fmt)
{
  rval = "";
  _THIS_CMD = "_write_raw";

  rval = ibwrt(gid, x, fmt);

  if (_LIB_DEBUG)
  {
    if (type(x)!="int")
    {
      printf("%s: %s: %s\n",_THIS_LIB, _THIS_CMD, num2str(x[:]',"%g",","));
    }
    else
    {
      printf("%s: %s: %s\n",_THIS_LIB, _THIS_CMD, num2str(x[:]',"%i",","));
    }
  }

  return rval;
};

static(_send_query_command_to_device_num,_send_query_command_to_device_char);
_send_query_command_to_device_num = function (gid, _cmd, x, fmt, csp )
{
  _this_solver = "_send_query_command_to_device";
  if (!exist(fmt))
  { fmt = "%g"; }
  if (!exist(csp))
  { csp = ","; }

  if (!exist(_cmd))
  { error(_this_solver + ": Missing 1st parameter string 'cmd'. Don't know what to do!"); }

  if (class(_cmd)!="string")
  { error(_this_solver + ": Missing 1st parameter string 'cmd'. Don't know what to do!"); }

  if (exist(x))
  {
    if (class(x) == "num")
    {
      _cmd = _cmd + " " + num2str(x,fmt,csp);
    }
    else if (class(x) == "string")
    {
      _cmd = _cmd + " " + x;
    }
    else
    {
      error("Don't know what to do!");
    }

    _write_to_device(gid,_cmd);

    return x;
  }

  _write_to_device(gid,_cmd + "?");
  x = _read_from_device(gid);
  chomp(x);
  return strtod(x);
};

_send_query_command_to_device_char = function (gid, _cmd, x, choices )
{
  _this_solver = "_send_query_command_to_device";

  if (strlen(_cmd)<1)
  {
    error(_this_solver + ": Missing 1st parameter string 'cmd'. Don't know what to do!");
  }

  if (strlen(x)>0)
  {
    x = toupper (x);

    if (!exist(choices))
    {
      choices = x;
    }

    if (choices.nc == 2)
    {
      shorthand_choice = choices[;1];
      fullname_choice = choices[;2];
    }
    else if (choices.nc == 1 || choices.nr == 1)
    {
      shorthand_choice = choices[:];
      fullname_choice  = choices[:];
    }
    else
    {
      error(_this_solver + ": 3rd parameter string array 'choices' must be vector, or two-column!");
    }

    found = 0;
    for (i in range(shorthand_choice))
    {
      if (strlen(shorthand_choice[i])>0)
      {
        if (strindex(x,fullname_choice[i])>0)
        {
          _cmd = _cmd + " " + fullname_choice[i];
          found = 1;
          break;
        }
      }
    }
    if (found)
    {
      _write_to_device(gid,_cmd);
      return x;
    }
  }

  _write_to_device(gid, _cmd + "?");
  x = _read_from_device(gid);
  chomp(x);
  return x;
};

fng_agilent332XX_class = classdef(gpib_major, gpib_opts)
{
  static(GPIB_ID, gopts);
  if (isnumber(gpib_major)<1)
  {
    EOF
  }
  if (class(gpib_opts)!="list")
  {
    gopts = <<>>;
    gopts.timeout = "T1s";
    gopts.eos = "\r\n";
  }
  else
  {
    gopts = gpib_opts
  }
  GPIB_ID  = ibdev(0, gpib_major, 0, gopts);

  public(debug);
  debug = function ( val )
  {   
    if (exist(val))
    {
      _LIB_DEBUG = (val > 0);
    }
    return _LIB_DEBUG;
  };

  // common csp commands
  public(cls,wait,ese,esr,idn,psc,sre,stb,rcl,lcl,rst,save,tst);
  cls = function()
  {
    _cmd = "*CLS";
    _write_to_device(GPIB_ID, _cmd );
    return (0);
  };
  wait = function()
  {
    _cmd = "*WAI";
    _write_to_device(GPIB_ID, _cmd );
    return (0);
  };
  ese = function( x )
  {
    _cmd = "*ESE";
    return _send_query_command_to_device_num(GPIB_ID, _cmd, x );
  };
  esr = function(x)
  {
    _cmd = "*ESR";
    return _send_query_command_to_device_num(GPIB_ID,_cmd, x, "%.0f" );
  };
  idn = function()
  {
    _cmd = "*IDN";
    return _send_query_command_to_device_char(GPIB_ID,_cmd );
  };
  psc = function( x )
  {
    _cmd = "*PSC";
    return _send_query_command_to_device_num(GPIB_ID,_cmd, x, "%.0f" );
  };
  sre = function( x )
  {
    _cmd = "*SRE";
    return _send_query_command_to_device_num(GPIB_ID,_cmd, x, "%.0f" );
  };
  stb = function()
  {
    _cmd = "*STB";
    return _send_query_command_to_device_num(GPIB_ID,_cmd );
  };
  rcl = function( x )
  {
    _cmd = "*RCL";
    if (!exist(x))
    { error("Command " + _cmd + " requires parameter!"); }
    return _send_query_command_to_device_num(GPIB_ID,_cmd, x, "%.0f" );
  };
  lcl = function()
  {
    ibloc(GPIB_ID);
    return 0;
  };
  rst = function()
  {
    _cmd = "*RST";
    _write_to_device(GPIB_ID, _cmd );
    return (0);
  };
  save = function( x )
  {
    _cmd = "*SAV";
    if (!exist(x))
    { error("Command " + _cmd + " requires parameter!"); }
    return _send_query_command_to_device_num(GPIB_ID,_cmd, x, "%.0f" );
  };
  tst = function()
  {
    _cmd = "*TST?";
    _write_to_device(GPIB_ID, _cmd );
    x = _read_from_device(GPIB_ID);
    return x;
  };
  public(ampl);
  ampl = <<>>;
  ampl.vpp = function(v)
  {
    _cmd = "AMPL";
    _unit = "VP";
    if (class(v) == "num")
    {
      _cmd = _cmd + num2str(v[1]," %g" + _unit);
      _write_to_device(_cmd);
      return v;
    }
    if (class(v) == "list")
    {
      _cmd = _cmd + num2str(v.val," %g" + v.unit);
    }

    _cmd = _cmd + "?";
    _write_to_device(GPIB_ID,_cmd);
    x = _read_from_device(GPIB_ID);

    if (isempty(x))
    {
      return <<>>;
    }

    i = strindex(x,"V");
    if (i)
    {
      rval = strtod(substr(x,1:(i-1)));
      unit = substr(x,i:(i+1));
      if (unit == "VP")
      { unit = "VPP"; }
      if (unit == "VR")
      { unit = "VRMS"; }
      return <<val=rval;unit=unit>>;
    }
    return <<>>;
  };
  ampl.vrms = function(v)
  {
    _cmd = "AMPL";
    _unit = "VR";
    if (class(v) == "num")
    {
      _cmd = _cmd + num2str(v[1]," %g" + _unit);
      _write_to_device(_cmd);
      return v;
    }

    if (class(v) == "list")
    {
      _cmd = _cmd + num2str(v.val," %g" + substring(v.unit,1:2));
      _write_to_device(GPIB_ID,_cmd);
      return v;
    }

    _cmd = _cmd + "?";
    _write_to_device(GPIB_ID,_cmd);
    x = _read_from_device(GPIB_ID);

    if (isempty(x))
    {
      return <<>>;
    }

    i = strindex(x,"V");
    if (i)
    {
      rval = strtod(substr(x,1:(i-1)));
      unit = substr(x,i:(i+1));
      if (unit == "VP")
      { unit = "VPP"; }
      if (unit == "VR")
      { unit = "VRMS"; }
      return <<val=rval;unit=unit>>;
    }
    return <<>>;
  };
  public(freq,v,output,pulse,burst);
  freq = function(f)
  {
    _cmd = "FREQ";
    return _send_query_command_to_device_num(GPIB_ID,_cmd, f, "%.6f" );
  };
  v = <<>>;
  v.ampl = function(f)
  {
    _cmd = "VOLT";
    return _send_query_command_to_device_num(GPIB_ID,_cmd, f, "%.6f" );
  };
  v.offs = function(f)
  {
    _cmd = "VOLT:OFFS";
    return _send_query_command_to_device_num(GPIB_ID,_cmd, f, "%.6f" );
  };
  v.high = function(f)
  {
    _cmd = "VOLT:HIGH";
    return _send_query_command_to_device_num(GPIB_ID,_cmd, f, "%.6f" );
  };
  v.low = function(f)
  {
    _cmd = "VOLT:LOW";
    return _send_query_command_to_device_num(GPIB_ID,_cmd, f, "%.6f" );
  };
  v.unit = function(f)
  {
    _cmd = "VOLT:UNIT";
    return _send_query_command_to_device_char(GPIB_ID,_cmd, f, ["VPP", "VRMS", "DBM"] );
  };
  output = <<>>;
  output.on = function()
  {
    _cmd = "OUTP";
    return _send_query_command_to_device_char(GPIB_ID,_cmd, "ON" );
  };
  output.off = function()
  {
    _cmd = "OUTP";
    return _send_query_command_to_device_char(GPIB_ID,_cmd, "OFF" );
  };
  output.load = function(f)
  {
    _cmd = "OUTP:LOAD";
    if (strindex(f, "INF")>0)
    {
      _cmd = _cmd + " " + f;
    }
    if (strindex(f, "MIN")>0)
    {
      _cmd = _cmd + " " + f;
    }
    if (strindex(f, "MAX")>0)
    {
      _cmd = _cmd + " " + f;
    }
    if (isnumber(f)>0)
    {
      _cmd = _cmd + num2str(f, " %g");
    }
    return strtod(_send_query_command_to_device_char(GPIB_ID,_cmd ));
  };
  output.polarity = <<>>;
  output.polarity.normal = function()
  {
    _cmd = "OUTP:POL";
    return _send_query_command_to_device_char(GPIB_ID,_cmd, "NORM" );
  };
  output.polarity.inverted = function()
  {
    _cmd = "OUTP:POL";
    return _send_query_command_to_device_char(GPIB_ID,_cmd, "INV" );
  };
  pulse=<<>>;
  pulse.period = function(f)
  {
    _cmd = "PULS:PER";
    return _send_query_command_to_device_num(GPIB_ID,_cmd, f, "%e" );
  };
  pulse.freq = function(f)
  {
    _cmd = "PULS:PER";
    if (isnumber(f))
    {
      tp = 1./f;
    }
    rval = _send_query_command_to_device_num(GPIB_ID,_cmd, tp, "%e" );
    return (1./rval);
  };
  pulse.width = function(f)
  {
    _cmd = "PULS:WIDT";
    return _send_query_command_to_device_num(GPIB_ID,_cmd, f, "%e" );
  };
  pulse.trans = function(f)
  {
    _cmd = "PULS:TRAN";
    return _send_query_command_to_device_num(GPIB_ID,_cmd, f, "%e" );
  };
  burst = <<>>;
  burst.state = <<>>;
  burst.state.on = function()
  {
    _cmd = "BURS:STAT";
    return _send_query_command_to_device_char(GPIB_ID,_cmd, "ON" );
  };
  burst.state.off = function()
  {
    _cmd = "BURS:STAT";
    return _send_query_command_to_device_char(GPIB_ID,_cmd, "OFF" );
  };
  burst.mode = <<>>;
  burst.mode.trig = function()
  { 
    _cmd = "BURS:MODE";
    return _send_query_command_to_device_char(GPIB_ID,_cmd, "TRIG" );
  };
  burst.mode.gated = function()
  {
    _cmd = "BURS:MODE";
    return _send_query_command_to_device_char(GPIB_ID,_cmd, "GAT" );
  };
  burst.ncycles = function(f)
  {
    _cmd = "BURS:NCYC";
    return _send_query_command_to_device_num(GPIB_ID,_cmd, f, "%e" );
  };
  burst.period = function(f)
  {
    _cmd = "BURS:INT:PER";
    return _send_query_command_to_device_num(GPIB_ID,_cmd, f, "%e" );
  };
  burst.freq = function(f)
  {
    if (isnumber(f)>0)
    {
      tp = 1 ./ f;
    }
    _cmd = "BURS:INT:PER";
    rval = _send_query_command_to_device_num(GPIB_ID,_cmd, tp, "%e" );
    return (1./rval);
  };
  burst.phase = function(f)
  {
    _cmd = "BURS:PHAS";
    return _send_query_command_to_device_num(GPIB_ID,_cmd, f, "%e" );
  };
  public(trigger,system,fn,display);
  trigger = <<>>;
  trigger.now = function()
  {
    _cmd = "*TRG";
    _write_to_device(GPIB_ID,_cmd);
    return 0;
  };
  trigger.source = <<>>;
  trigger.source.imm = function()
  {
    _cmd = "TRIG:SOUR";
    return _send_query_command_to_device_char(GPIB_ID,_cmd, "IMM" );
  };
  trigger.source.bus = function()
  {
    _cmd = "TRIG:SOUR";
    return _send_query_command_to_device_char(GPIB_ID,_cmd, "BUS" );
  };
  trigger.source.ext = function()
  {
    _cmd = "TRIG:SOUR";
    return _send_query_command_to_device_char(GPIB_ID,_cmd, "EXT" );
  };
  system = <<>>;
  system.beep = function(f)
  {
    _cmd = "SYST:BEEP";
    if (isnumber(f))
    {
      if (f)
      {
        return _send_query_command_to_device_char(GPIB_ID,_cmd, "ON");
      }
      return _send_query_command_to_device_char(GPIB_ID,_cmd, "OFF");
    }
    return _write_to_device(GPIB_ID,_cmd);
  };
  fn = function(f)
  {
    _cmd = "FUNC";
    return _send_query_command_to_device_char(GPIB_ID,_cmd, f, ...
        ["SIN", "SQU", "RAND", "PULS", "NOIS", "DC", "USER"]);
  };
  display = function(f)
  {
    if (strlen(f)>=0)
    {
      _cmd = "DISP:TEXT";
      if (strlen(f)>0)
      {
        return _send_query_command_to_device_char(GPIB_ID,_cmd, "\"" + f + "\"");
      }
      return _send_query_command_to_device_char(GPIB_ID,_cmd, "CLEAR");
    }

    _cmd = "DISP";
    if (isnumber(f)>0)
    {
      if (f>0)
      {
        return _send_query_command_to_device_char(GPIB_ID,_cmd, "ON" );
      }
      return _send_query_command_to_device_char(GPIB_ID,_cmd, "OFF" );
    }

    return strtod(_send_query_command_to_device_char(GPIB_ID,_cmd));
  };

};




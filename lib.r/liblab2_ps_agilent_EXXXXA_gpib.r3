//
// liblab2_ps_keysight_N5700_gpib
//

if (!exist(ibdev))
{
  rfile libgpib.so
}

static(_THIS_LIB, _LIB_DEBUG);
if (!exist(_THIS_LIB))
{
  _THIS_LIB = "liblab2_ps_keysight_N5700_gpib: ";
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
    rval = ibwrt(gid, _c + "\n");

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
      error(_this_solver ...
          + ": 3rd parameter string array 'choices' must be vector, or two-column!");
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

ps_agilent_class = classdef(gpib_major, gpib_opts)
{
  static(GPIB_ID,gopts);
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
  public(gpib_id,cls,wait,ese,esr,idn,sre,stb,rcl,lcl,rst,save,trg,tst);
  gpib_id = function ()
  {
    return GPIB_ID;
  };
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
  trg = function()
  {
    _cmd = "*TRG";
    _write_to_device(GPIB_ID, _cmd );
    x = _read_from_device(GPIB_ID);
    return x;
  };
  tst = function()
  {
    _cmd = "*TST?";
    _write_to_device(GPIB_ID, _cmd );
    x = _read_from_device(GPIB_ID);
    return x;
  };
  public(inst,source,output,sys,v1,v2,v3,i1,i2,i3);
  inst = <<>>;
  inst.select = function ( arg1 )
  {
    if (exist(arg1))
    {
      if (class(arg1) == "num")
      {
        _cmd = "INST:NSEL" + num2str(arg1," %.0f");
      }
      else if (class(arg1) == "string")
      {
        _cmd = "INST:SEL " + arg1;
      }
      else
      {
        error("Don't know what to do!");
      }

      "CMD: " + _cmd + "\n" ?

          _write_to_device(GPIB_ID,_cmd);
      return arg1;
    }

    _cmd = "INST:NSEL";
    _write_to_device(GPIB_ID, _cmd + "?");
    x = _read_from_device(GPIB_ID);
    x?
    if (strindex(x,_cmd))
    { x = substr(x, strindex(x,_cmd) + strlen(_cmd):strlen(x) )}
    x = strtod(x);
    return x;
  };
  i1 = <<>>;
  i1.level = function(f)
  {
    _cmd = "INST:NSEL 1";
    _write_to_device(GPIB_ID, _cmd);
    _cmd = "CURR";
    if (isnumber(f)>0)
    {
      return _send_query_command_to_device_num(GPIB_ID,_cmd, f, "%g" );
    }
    return _send_query_command_to_device_num(GPIB_ID,_cmd);
  };
  i1.meas = function()
  {
    _cmd = "INST:NSEL 1";
    _write_to_device(GPIB_ID, _cmd);
    _cmd = "MEAS:CURR:DC";
    return _send_query_command_to_device_num(GPIB_ID, _cmd);
  };
  i2 = <<>>;
  i2.level = function(f)
  {
    _cmd = "INST:NSEL 2";
    _write_to_device(GPIB_ID, _cmd);
    _cmd = "CURR";
    if (isnumber(f)>0)
    {
      return _send_query_command_to_device_num(GPIB_ID,_cmd, f, "%g" );
    }
    return _send_query_command_to_device_num(GPIB_ID,_cmd);
  };
  i2.meas = function()
  {
    _cmd = "INST:NSEL 2";
    _write_to_device(GPIB_ID, _cmd);
    _cmd = "MEAS:CURR:DC";
    return _send_query_command_to_device_num(GPIB_ID, _cmd);
  };
  i3 = <<>>;
  i3.level = function(f)
  {
    _cmd = "INST:NSEL 3";
    _write_to_device(GPIB_ID, _cmd);
    _cmd = "CURR";
    if (isnumber(f)>0)
    {
      return _send_query_command_to_device_num(GPIB_ID,_cmd, f, "%g" );
    }
    return _send_query_command_to_device_num(GPIB_ID,_cmd);
  };
  i3.meas = function()
  {
    _cmd = "INST:NSEL 3";
    _write_to_device(GPIB_ID, _cmd);
    _cmd = "MEAS:CURR:DC";
    return _send_query_command_to_device_num(GPIB_ID, _cmd);
  };
  v1 = <<>>;
  v1.level = function(f)
  {
    _cmd = "INST:NSEL 1";
    _write_to_device(GPIB_ID, _cmd);
    _cmd = "VOLT";
    if (isnumber(f)>0)
    {
      return _send_query_command_to_device_num(GPIB_ID,_cmd, f, "%g" );
    }
    return _send_query_command_to_device_num(GPIB_ID,_cmd);
  };
  v1.meas = function()
  {
    _cmd = "INST:NSEL 1";
    _write_to_device(GPIB_ID, _cmd);
    _cmd = "MEAS:VOLT:DC";
    return _send_query_command_to_device_num(GPIB_ID, _cmd);
  };
  v2 = <<>>;
  v2.level = function(f)
  {
    _cmd = "INST:NSEL 2";
    _write_to_device(GPIB_ID, _cmd);
    _cmd = "VOLT";
    if (isnumber(f)>0)
    {
      return _send_query_command_to_device_num(GPIB_ID,_cmd, f, "%g" );
    }
    return _send_query_command_to_device_num(GPIB_ID,_cmd);
  };
  v2.meas = function()
  {
    _cmd = "INST:NSEL 2";
    _write_to_device(GPIB_ID, _cmd);
    _cmd = "MEAS:VOLT:DC";
    return _send_query_command_to_device_num(GPIB_ID, _cmd);
  };
  v3 = <<>>;
  v3.level = function(f)
  {
    _cmd = "INST:NSEL 3";
    _write_to_device(GPIB_ID, _cmd);
    _cmd = "VOLT";
    if (isnumber(f)>0)
    {
      return _send_query_command_to_device_num(GPIB_ID,_cmd, f, "%g" );
    }
    return _send_query_command_to_device_num(GPIB_ID,_cmd);
  };
  v3.meas = function()
  {
    _cmd = "INST:NSEL 3";
    _write_to_device(GPIB_ID, _cmd);
    _cmd = "MEAS:VOLT:DC";
    return _send_query_command_to_device_num(GPIB_ID, _cmd);
  };
  source = <<>>;
  source.v = <<>>;
  source.v.prot = function(f)
  {
    _cmd = "SOUR:VOLT:PROT";
    if (isnumber(f)>0)
    {
      return _send_query_command_to_device_num(GPIB_ID,_cmd, f, "%g" );
    }
    return _send_query_command_to_device_num(GPIB_ID,_cmd);
  };
  source.v.limit = <<>>;
  source.v.limit.low function(f)
  {
    _cmd = "SOUR:VOLT:LIM:LOW";
    if (isnumber(f)>0)
    {
      return _send_query_command_to_device_num(GPIB_ID,_cmd, f, "%g" );
    }
    return _send_query_command_to_device_num(GPIB_ID,_cmd);
  };
  source.v.level.trg = function(f)
  {
    _cmd = "SOUR:VOLT:LEV:TRIG";
    if (isnumber(f)>0)
    {
      return _send_query_command_to_device_num(GPIB_ID,_cmd, f, "%g" );
    }
    return _send_query_command_to_device_num(GPIB_ID,_cmd);
  };
  source.i = <<>>;
  source.i.prot = function(f)
  {
    _cmd = "SOUR:CURR:PROT:STAT";
    if (isnumber(f)>0)
    {
      return _send_query_command_to_device_num(GPIB_ID,_cmd, (f>0), "%.0f" );
    }
    return _send_query_command_to_device_num(GPIB_ID,_cmd);
  };
  source.i.level = <<>>;
  source.i.level.imm = function(f)
  {
    _cmd = "SOUR:CURR:LEV";
    if (isnumber(f)>0)
    {
      return _send_query_command_to_device_num(GPIB_ID,_cmd, f, "%g" );
    }
    return _send_query_command_to_device_num(GPIB_ID,_cmd);
  };
  source.i.level.trg = function(f)
  {
    _cmd = "SOUR:CURR:LEV:TRIG";
    if (isnumber(f)>0)
    {
      return _send_query_command_to_device_num(GPIB_ID,_cmd, f, "%g" );
    }
    return _send_query_command_to_device_num(GPIB_ID,_cmd);
  };
  output = <<>>;
  output.on = function()
  {
    _cmd = "OUTP ON";
    return _write_raw(GPIB_ID, _cmd, "%s");
  };
  output.off = function()
  {
    _cmd = "OUTP OFF";
    return _write_raw(GPIB_ID, _cmd, "%s");
  };
  sys = <<>>;
  sys.err = function()
  {
    _cmd = "SYST:ERR";
    return _send_query_command_to_device_char(GPIB_ID,_cmd);
  };
};




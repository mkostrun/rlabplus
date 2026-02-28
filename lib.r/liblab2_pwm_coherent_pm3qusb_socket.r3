/// As a superuser do:
///   socat TCP-LISTEN:8000,fork,reuseaddr FILE:/dev/ttyACM0,cs8,b9600,nonblock,raw,echo=0
/// then use
///   "tcp://127.0.0.1:8000" as the URL for read/write
///
static(_THIS_LIB, _LIB_DEBUG);
static(LOGFN, LOGON);

if (!exist(_THIS_LIB))
{
  _THIS_LIB = "liblab2_pwm_coherent_pm3qusb_serial: ";
}
if (!exist(_LIB_DEBUG))
{
  _LIB_DEBUG = 0;
}

static(_writem, _readm);
_writem = function(lsocket, cmd)
{
  writem(lsocket, cmd);
  chomp(cmd);
  if (_LIB_DEBUG)
  {
    printf("%s\n", cmd);
  }
  if (LOGON)
  {
    if (strlen(LOGFN)>0)
    {
      fprintf(LOGFN, "%s: %s\r\n", timestamp("%Y-%m-%dT%H:%M:%S%z"), cmd);
    }
  }
  return 0;
};
_readm = function(lsocket)
{
  rval = readm(lsocket);
  if (strlen(rval)>1)
  {
    chomp(rval);
    if (_LIB_DEBUG)
    {
      printf("%s\n", rval);
    }
    if (LOGON)
    {
      if (strlen(LOGFN)>0)
      {
        fprintf(LOGFN, "%s: %s\r\n", timestamp("%Y-%m-%dT%H:%M:%S%z"), rval);
      }
    }
  }
  return rval;
};

pwm_coherent_pm3qusb_class = classdef(lsocket, lsocket_opts)
{
  static(LSOCKET, EOL);
  if (strlen(lsocket)<1)
  {
    EOF
  }
  if (!exist(EOL))
  {
    EOL = "\r\n";
  }
  LSOCKET = lsocket;
  if (!open(LSOCKET,lsocket_opts))
  {
    sleep (2);
  }
  if (!exist(LOGON))
  {
    LOGON = 0;
  }
  printf("Coherent Laser Power/Energy Meter PM3Q at %s is online!\n", LSOCKET);

  public(debug,diary);
  debug = function ( val )
  {
    if (exist(val))
    {
      _LIB_DEBUG = (val > 0);
    }
    return _LIB_DEBUG;
  };
  diary = <<>>;
  diary.filename = function (fn)
  {
    if (strlen(fn) > 0)
    {
      LOGFN = fn;
    }
    LOGON = 0;
    return LOGFN;
  };
  diary.start = function(mode)
  {
    if (strlen(LOGFN)>0)
    {
      LOGON = 1;
      if (strlen(mode)<1)
      {
        mode = "a";
      }
      open(LOGFN,mode);
    }
    return LOGON;
  };
  diary.stop = function()
  {
    LOGON = 0;
    close(LOGFN);
    return LOGON;
  };
  public(readm,writem,sn,pn,model,cdate,idn,rst,sys,config);
  readm = function (eol)
  {
    return _readm(LSOCKET);
  };
  writem = function (_cmd)
  {
    return _writem(LSOCKET, _cmd + EOL);
  };
  idn = function()
  {
    _writem(LSOCKET, "*IDN?" + EOL);
    rval = _readm(LSOCKET);
    if (strlen(rval)>0)
    {
      chomp(rval);
    }
    return rval;
  };
  sn = function()
  {
    _writem(LSOCKET, "SYST:INF:SNUM?" + EOL);
    rval = _readm(LSOCKET);
    if (strlen(rval)>0)
    {
      chomp(rval);
    }
    return rval;
  };
  pn = function()
  {
    _writem(LSOCKET, "SYST:INF:PNUM?" + EOL);
    rval = _readm(LSOCKET);
    if (strlen(rval)>0)
    {
      chomp(rval);
    }
    return rval;
  };
  model = function()
  {
    _writem(LSOCKET, "SYST:INF:MOD?" + EOL);
    rval = _readm(LSOCKET);
    if (strlen(rval)>0)
    {
      chomp(rval);
    }
    return rval;
  };
  cdate = function()
  {
    _writem(LSOCKET, "SYST:INF:CDAT?" + EOL);
    rval = _readm(LSOCKET);
    if (strlen(rval)>0)
    {
      chomp(rval);
    }
    return rval;
  };
  rst = function(f)
  {
    _writem(LSOCKET, "*RST" + EOL);
    return 0;
  };
  sys = <<>>;
  sys.status = function()
  {
    _writem(LSOCKET, "SYST:STAT?" + EOL);
    rval = _readm(LSOCKET);
    if (strlen(rval)>0)
    {
      chomp(rval);
    }
    return rval;
  };
  sys.temp = function ()
  {
    _writem(LSOCKET, "SYST:INF:TEMP?" + EOL);
    rval = _readm(LSOCKET);
    if (strlen(rval)>0)
    {
      chomp(rval);
      rval = strtod(rval);
    }
    return rval;
  };
  sys.sync = <<>>;
  sys.sync.now = function()
  {
    _writem(LSOCKET, "SYST:SYNC?" + EOL);
    rval = _readm(LSOCKET);
    if (strlen(rval)>0)
    {
      chomp(rval);
    }
    return rval;
  };
  sys.sync.reset = function(f)
  {
    _writem(LSOCKET, "SYST:SYNC" + EOL);
    return 0;
  };
  sys.handshake = function (f)
  {
    _cmd = "SYST:COMM:HAND";
    if (isnumber(f)>0)
    {
      if (f)
      {
        _writem(LSOCKET, _cmd + " ON" + EOL);
      }
      if (!f)
      {
        _writem(LSOCKET, _cmd + " OFF" + EOL);
      }
    }
    _writem(LSOCKET, _cmd +    "?" + EOL);
    rval = _readm(LSOCKET);
    if (strlen(rval)>0)
    {
      chomp(rval);
    }
    return rval;
  };
  sys.error = <<>>;
  sys.error.count = function()
  {
    _cmd = "SYST:ERR:COUNT";
    _writem(LSOCKET, _cmd +    "?" + EOL);
    rval = _readm(LSOCKET);
    if (strlen(rval)>0)
    {
      chomp(rval);
    }
    return rval;
  };
  sys.error.next = function()
  {
    _cmd = "SYST:ERR:NEXT";
    _writem(LSOCKET, _cmd +    "?" + EOL);
    rval = _readm(LSOCKET);
    if (strlen(rval)>0)
    {
      chomp(rval);
    }
    return rval;
  };
  sys.error.all = function()
  {
    _cmd = "SYST:ERR:ALL";
    _writem(LSOCKET, _cmd +    "?" + EOL);
    rval = _readm(LSOCKET);
    if (strlen(rval)>0)
    {
      chomp(rval);
    }
    return rval;
  };
  sys.error.clear = function()
  {
    _cmd = "SYST:ERR:CLE";
    _writem(LSOCKET, _cmd +    "?" + EOL);
    return 0;
  };
  config = <<>>;
  config.gain = function(f)
  {
    _cmd = "CONF:GAIN";
    if (isnumber(f)>0)
    {
      if (f > 0)
      {
        _writem(LSOCKET, _cmd + ":COMP ON" + EOL);
        _cmd2 = num2str(f,":FACT %.e");
        _writem(LSOCKET, _cmd + _cmd2  + EOL);
      }
      if (f == 0)
      {
        _writem(LSOCKET, _cmd + " OFF" + EOL);
      }
      return f;
    }
    _writem(LSOCKET, _cmd + ":COMP?" + EOL);
    s = _readm(LSOCKET);
    if (strlen(s)>0)
    {
      chomp(s);
    }
    rval = 0;
    if (strindex(s,"ON")>0)
    {
      _writem(LSOCKET, _cmd + ":FACT?" + EOL);
      s = _readm(LSOCKET);
      chomp(s);
      rval = strtod(s);
    }
    return rval;
  };
  config.speedup = function(f)
  {
    _cmd = "CONF:SPE";
    if (isnumber(f)==1)
    {
      if (f)
      {
        _writem(LSOCKET, _cmd + " ON" + EOL);
      }
      if (!f)
      {
        _writem(LSOCKET, _cmd + " OFF" + EOL);
      }
    }
    _writem(LSOCKET, _cmd + "?" + EOL);
    s = _readm(LSOCKET);
    if (strlen(s)>0)
    {
      chomp(s);
    }
    rval = nan();
    if (strlen(s)>0)
    {
      rval = 0;
      chomp(s);
      if (strindex(s,"ON")>0)
      {
        rval = 1;
      }
    }
    return rval;
  };
  config.item = function(f)
  {
    _cmd = "CONF:ITEM";
    if (strlen(f)>0)
    {
      _writem(LSOCKET, _cmd + " " + f + EOL);
    }
    _writem(LSOCKET, _cmd + "?" + EOL);
    rval = _readm(LSOCKET);
    if (strlen(rval)>0)
    {
      chomp(rval);
    }
    return rval;
  };
  config.zero = function()
  {
    _cmd = "CONF:ZERO";
    _writem(LSOCKET, _cmd + EOL);
    return 0;
  };
  config.diameter_mm = function(f)
  {
    _cmd = "CONF:DIAM";
    if (isnumber(f)>0)
    {
      if (f > 0)
      {
        _cmd2 = num2str(f," %.0f");
        _writem(LSOCKET, _cmd + _cmd2  + EOL);
      }
      return f;
    }
    _writem(LSOCKET, _cmd + "?" + EOL);
    s = _readm(LSOCKET);
    rval = nan();
    if (strlen(s)>0)
    {
      chomp(s);
      rval = strtod(s);
    }
    return rval;
  };
  config.meas= <<>>;
  config.meas.power = function()
  {
    _cmd = "CONF:MEAS W";
    _writem(LSOCKET, _cmd + EOL);
    return 0;
  };
  config.meas.energy = function()
  {
    _cmd = "CONF:MEAS J";
    _writem(LSOCKET, _cmd + EOL);
    return 0;
  };
  config.wavelength_nm = function(f)
  {
    _cmd = "CONF:WAVE";
    if (isnumber(f)>0)
    {
      if (f > 0)
      {
        _cmd2 = num2str(f," %.0f");
        _writem(LSOCKET, _cmd + _cmd2  + EOL);
      }
      return f;
    }
    _writem(LSOCKET, _cmd + "?" + EOL);
    s = _readm(LSOCKET);
    rval = nan();
    if (strlen(s)>0)
    {
      chomp(s);
      rval = strtod(s);
    }
    return rval;
  };
  public(read,stream,trigger);
  read = function ()
  {
    _cmd = "READ?";
    _writem(LSOCKET, _cmd  + EOL);
    s = _readm(LSOCKET);
    "reply = " ?
    s?
    rval = nan();
    if (strlen(s)>0)
    {
      chomp(s);
      rval = strtod(s)[1];
    }
    return rval;
  };
  stream = <<>>;
  stream.start = function ()
  {
    _cmd = "DST";
    _writem(LSOCKET, _cmd + EOL);
    return 0;
  };
  stream.stop = function ()
  {
    _cmd = "DSP";
    _writem(LSOCKET, _cmd + EOL);
    return 0;
  };
  trigger = <<>>;
  trigger.source = <<>>;
  trigger.source.int = function ()
  {
    _cmd = "TRIG:SOUR:INT";
    _writem(LSOCKET, _cmd + EOL);
    return 0;
  };
  trigger.source.ext = function ()
  {
    _cmd = "TRIG:SOUR:EXT";
    _writem(LSOCKET, _cmd + EOL);
    return 0;
  };
};


















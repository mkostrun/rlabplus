///
///
///
static(_THIS_LIB, _LIB_DEBUG);
if (!exist(_THIS_LIB))
{
  _THIS_LIB = "liblab2_pwm_coherent_pm3qusb_serial: ";
}
if (!exist(_LIB_DEBUG))
{
  _LIB_DEBUG = 0;
}

static(read_nbytes);
read_nbytes = function(u, n, twait)
{
  if (!exist(twait))
  { twait = 0.1; }

  ret = readm(u,n);

  tic(WAIT_TIMER_ID);
  while (length(ret) != n && toc(WAIT_TIMER_ID)<twait)
  { ret = [ret, readm(u,n-length(ret)) ]; }

  if (isempty(ret))
  { stop("Warning: No response from the controller!\n"); }

  if (length(ret)!=n)
  { printf("Warning: Requested %g bytes, but received only %g!\n", n, length(ret)); }

  return ret;
};

static (_error_msgs);
_error_msgs = [ ...
    "Command not found", ...
    "Invalid parameter", ...
    "Not enough parameters", ...
    "Head is not available", ...
[]];

static (_sampling_params_names);
_sampling_params_names = [...
    "rate", ...
    "period", ...
    "length", ...
    "time_stamp", ...
[]];

static(_qreadnbytes, _qreadnbytes_of_data);
_qreadnbytes = function (ttyport, bincmd)
{
  writem(ttyport, bincmd);
  x = read_nbytes(ttyport,4);
  n = bytejoin(x[3:4],1);
  if (x[2] == 10L)
  {
    x = read_nbytes(ttyport, n);
    x?
    return x;
  }
  return _error_msgs[n + 0.0];
};

_qreadnbytes_of_data = function (ttyport, bincmd)
{
  if (_LIB_DEBUG)
  {
    printf(_THIS_LIB + "_qreadnbytes_of_data: %s\n", bincmd);
  }
  writem(ttyport, bincmd);
  sleep(0.01);
  x = read_nbytes(ttyport,4);
  if (_LIB_DEBUG)
  {
    printf(_THIS_LIB + "_qreadnbytes_of_data: %02x:%02x:%02x:%02x\n", x[1],x[2],x[3],x[4]);
  }
  rval = <<>>;
  if (isempty(x))
  {
    return rval;
  }
  if (x[2] == 11L)
  {
    sleep(0.1);
    writem(ttyport, bincmd);
    sleep(0.1);
    x = read_nbytes(ttyport,4);
  }
  x?
  if (x[2] == 11L)
  {
    if (x.n == 4)
    {
      err_no = bytejoin(x[3:4],1) + 0.0;
    }
    rval.err = _error_msgs[ err_no ];
    return rval;
  }
  x = read_nbytes(ttyport,8);
  n_header = bytejoin(x[1:4],1);
  n_data   = bytejoin(x[5:8],1);
  [n_header, n_data]?
  rval.header = char(read_nbytes(ttyport, n_header));
  data_val    = read_nbytes(ttyport, n_data);
  data_val    = bytejoin(data_val,0,4);
  resize (data_val,6,data_val.n/6);
  data_val = data_val';
  rval.val = [data_val[;5].*divider_unit(data_val[;6]),data_val[;3].*divider_unit(data_val[;4])];
  return rval;
};

static(_readstring);
_readstring = function (ttyport)
{
  x="";
  tic(12);
  _l0 = -1;
  _l1 = 0;
  while (substr(x,strlen(x))!="\r")
  {
    _l0 = _l1;
    x = x + readm(ttyport);
    _l1 = strlen(x);
    if (_l0 < _l1)
    {
      tic(12);
    }
    if (toc(12)>0.5)
    {
      break;
    }
  }
  chomp(x);
  return x;
};

pwm_coherent_pm3qusb_class = classdef(ttyport)
{
  static(TTYPORT, LOGFN, LOGON);
  if (strlen(ttyport)>0)
  {
    TTYPORT = ttyport;
  }
  else
  {
    attr=<<>>;
    attr.devname = "tty";
    attr.id_vendor_id = "0d4d";
    CHIP_ADD = usbdevname(attr);
    if (isempty(CHIP_ADD))
    {
      printf("Horrible internal error: Cannot find the contoller!\n");
      EOF
    }
    TTYPORT_OPTS = <<>>;
    TTYPORT_OPTS.data_format  = "8N1";
    //TTYPORT_OPTS.speed    = 9600;
    TTYPORT_OPTS.speed    = 57600;
    //TTYPORT_OPTS.speed    = 38400;
    TTYPORT_OPTS.debug    = 1;
    TTYPORT_OPTS.raw = 0;
    TTYPORT_OPTS.hupcl = 1;
    TTYPORT_OPTS.eol = "\n";
    TTYPORT_OPTS.flow_control = "none";
    TTYPORT_OPTS.debug = "hex";
    //TTYPORT_OPTS.flow_control = "hw";
    TTYPORT = "serial://" + CHIP_ADD;
  }
  if (!open(TTYPORT,TTYPORT_OPTS))
  {
    sleep (3);
  }
  if (!exist(LOGON))
  {
    LOGON = 0;
  }
  printf("Coherent Laser Power/Energy Meter PM3Q at %s is online!\n", TTYPORT);

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
  public(readm,writem,sn,idn,rst,sys,config);
  readm = function ()
  {
    return _readstring(TTYPORT);
  };
  writem = function (_cmd)
  {
    return writem(TTYPORT, _cmd);
  };
  idn = function(f)
  {
    writem(TTYPORT, "*IDN?");
    return _readstring(TTYPORT);
    //return _readstring(TTYPORT);
  };
  sn = function(f)
  {
    writem(TTYPORT, "sn?");
    return _readstring(TTYPORT);
  };
  rst = function(f)
  {
    writem(TTYPORT, "*RST");
    return 0;
  };
  sys = <<>>;
  sys.status = function()
  {
    writem(TTYPORT, "SYST:STAT?");
    return _readstring(TTYPORT);
  };
  sys.temp = function ()
  {
    writem(TTYPORT, "SYST:INF:TEMP?");
    return _readstring(TTYPORT);
  };
  sys.sync = <<>>;
  sys.sync.now = function()
  {
    writem(TTYPORT, "SYST:SYNC?");
    return _readstring(TTYPORT);
  };
  sys.sync.reset = function(f)
  {
    writem(TTYPORT, "SYST:SYNC");
    return 0;
  };
  sys.handshake = function (f)
  {
    _cmd = "SYST:COMM:HAND";
    if (isnumber(f)>0)
    {
      if (f)
      {
        writem(TTYPORT, _cmd + " ON");
      }
      if (!f)
      {
        writem(TTYPORT, _cmd + " OFF");
      }
    }
    writem(TTYPORT, _cmd +    "?");
    return _readstring(TTYPORT);
  };
  sys.error = <<>>;
  sys.error.count = function()
  {
    _cmd = "SYST:ERR:COUNT";
    writem(TTYPORT, _cmd +    "?");
    return _readstring(TTYPORT);
  };
  sys.error.next = function()
  {
    _cmd = "SYST:ERR:NEXT";
    writem(TTYPORT, _cmd +    "?");
    return _readstring(TTYPORT);
  };
  sys.error.all = function()
  {
    _cmd = "SYST:ERR:ALL";
    writem(TTYPORT, _cmd +    "?");
    return _readstring(TTYPORT);
  };
  sys.error.clear = function()
  {
    _cmd = "SYST:ERR:CLE";
    writem(TTYPORT, _cmd +    "?");
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
        writem(TTYPORT, _cmd + ":COMP ON");
        _cmd2 = num2str(f,":FACT %.e");
        writem(TTYPORT, _cmd + _cmd2 );
      }
      if (f == 0)
      {
        writem(TTYPORT, _cmd + " OFF");
      }
      return f;
    }
    writem(TTYPORT, _cmd + ":COMP?");
    s = _readstring(TTYPORT);
    rval = nan();
    if (strindex(s,"ON")>0)
    {
      writem(TTYPORT, _cmd + ":FACT?");
      s = _readstring(TTYPORT);
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
        writem(TTYPORT, _cmd + " ON");
      }
      if (!f)
      {
        writem(TTYPORT, _cmd + " OFF");
      }
    }
    writem(TTYPORT, _cmd + "?");
    s = _readstring(TTYPORT);
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
      writem(TTYPORT, _cmd + " " + f);
    }
    writem(TTYPORT, _cmd + "?");
    s = _readstring(TTYPORT);
    return s;
  };
  config.zero = function()
  {
    _cmd = "CONF:ZERO";
    writem(TTYPORT, _cmd);
    return 0;
  };
  config.mode = <<>>;
  config.mode.power = function()
  {
    _cmd = "CONF:MEAS W";
    writem(TTYPORT, _cmd);
    return 0;
  };
  config.mode.energy = function()
  {
    _cmd = "CONF:MEAS J";
    writem(TTYPORT, _cmd);
    return 0;
  };
  config.wlen = function(f)
  {
    _cmd = "CONF:WAVE";
    if (isnumber(f)>0)
    {
      if (f > 0)
      {
        _cmd2 = num2str(f," %.0f");
        writem(TTYPORT, _cmd + _cmd2 );
      }
      return f;
    }
    writem(TTYPORT, _cmd + "?");
    s = _readstring(TTYPORT);
    rval = nan();
    if (strlen(s)>0)
    {
      chomp(s);
      rval = strtod(s);
    }
    return rval;
  };
  public(ver,val,stream);
  val = function ()
  {
    _cmd = "READ";
    writem(TTYPORT, _cmd + "?");
    s = _readstring(TTYPORT);
    rval = nan();
    if (strlen(s)>0)
    {
      chomp(s);
      rval = strtod(s);
    }
    return rval;
  };
  stream = <<>>;
  stream.start = function ()
  {
    _cmd = "DST";
    writem(TTYPORT, _cmd);
    return 0;
  };
  stream.stop = function ()
  {
    _cmd = "DSP";
    writem(TTYPORT, _cmd);
    return 0;
  };
};


















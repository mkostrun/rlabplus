//
//
//
static(_THIS_LIB, _LIB_DEBUG);
static(LOGFN, LOGON);
if (!exist(_THIS_LIB))
{
  _THIS_LIB = "liblab_osc_tds3054c_rfile:";
}
if (!exist(_LIB_DEBUG))
{
  _LIB_DEBUG = 0;
}

static(BASE_RECLEN,RECLEN,WIDTH,FORMAT,TIMEAX,VERTAX,_OSC_WAIT);
BASE_RECLEN = 500;
RECLEN = 500;
WIDTH = 2;
FORMAT = "RIBINARY";
TIMEAX  = 10;   // horizontal division
VERTAX  = 10;   // vertical division (only 8 visible)

_OSC_WAIT = 1;

static(_read_from_device, _write_to_device);
_write_to_device = function(lsocket, cmd)
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
_read_from_device = function(lsocket)
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

osc_tds3054c_class = classdef(lsocket, lsocket_opts)
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
  printf("Oscilloscope TDS3054c at %s is online!\n", LSOCKET);
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
  //*****************************************************
  //
  // common SCPI commands
  //
  //*****************************************************
  public(cls,ese,esr,evm,idn,lcl,lrn,opc,opt,rst,sre,stb,trg,wait,abort,event);
  cls = function ()
  {

    _cmd = "*CLS";
    _write_to_device (LSOCKET,_cmd);
    return 0;
  };
  ese = function( x )
  {
    _cmd = "*ESE";
    _write_to_device (LSOCKET,_cmd);
    return _read_from_device(LSOCKET);
  };
  esr = function()
  {
    _cmd = "*ESR";
    _write_to_device (LSOCKET,_cmd);
    return _read_from_device(LSOCKET);
  };
  esr = function()
  {
    _cmd = "*EVM";
    _write_to_device (LSOCKET,_cmd);
    return _read_from_device(LSOCKET);
  };
  idn = function()
  {
    _cmd = "*IDN";
    _write_to_device (LSOCKET,_cmd);
    return _read_from_device(LSOCKET);
  };
  lrn = function ()
  {
    _cmd = "*LRN?";
    _write_to_device (LSOCKET,_cmd);
    return _read_from_device(LSOCKET);
  };
  opc = function( x )
  {
    _cmd = "*OPC";
    _write_to_device (LSOCKET,_cmd);
    return _read_from_device(LSOCKET);
  };
  opt = function()
  {
    _cmd = "*OPT";
    _write_to_device (LSOCKET,_cmd);
    return _read_from_device(LSOCKET);

  };
  rst = function ()
  {
    _cmd = "*RST";
    _write_to_device (LSOCKET,_cmd);
    return 0;
  };
  sre = function( x )
  {
    _cmd = "*SRE";
    _write_to_device (LSOCKET,_cmd);
    return _read_from_device(LSOCKET);
  };
  stb = function ()
  {
    _cmd = "*STB?";
    _write_to_device (LSOCKET,_cmd);
    return _read_from_device(LSOCKET);
  };
  trg = function ()
  {
    _cmd = "*TRG";
    _write_to_device (LSOCKET,_cmd);
    return 0;
  };
  wait = function ()
  {
    _cmd = "*WAI";
    _write_to_device (LSOCKET,_cmd);
    return 0;
  };
  abort = function ()
  {
    _cmd = ":ABOR";
    _write_to_device (LSOCKET,_cmd);
    return 0;
  };
  event = function ( )
  {
    _cmd = "EVENT?";
    _write_to_device (LSOCKET,_cmd);
    return _read_from_device(LSOCKET);
  };
};



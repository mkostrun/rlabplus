//
//
//
static(_THIS_LIB, _LIB_DEBUG);
static(LOGFN, LOGON);
if (!exist(_THIS_LIB))
{
  _THIS_LIB = "liblab2_opm_thorlabs_pm16_rfile:";
}
if (!exist(_LIB_DEBUG))
{
  _LIB_DEBUG = 0;
}

static(_read_from_usbtmc, _write_to_usbtmc, _query_usbtmc);

_query_usbtmc = function(lsocket, cmd)
{
  rval = readm(lsocket, <<query=cmd>>);
  chomp(rval);
  if (_LIB_DEBUG)
  {
    printf("%s\n", cmd);
    writem("stdout", rval);
  }
  if (LOGON)
  {
    if (strlen(LOGFN)>0)
    {
      fprintf(LOGFN, "%s: %s\r\n", timestamp("%Y-%m-%dT%H:%M:%S%z"), cmd);
      writem(LOGFN, rval);
    }
  }
  return rval;
};

_write_to_usbtmc = function(lsocket, cmd)
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

_read_from_usbtmc = function(lsocket)
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

opm_thorlabs_pm16_class = classdef(lsocket)
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
  if (!open(LSOCKET))
  {
    sleep (2);
  }
  if (!exist(LOGON))
  {
    LOGON = 0;
  }
  printf("Thorlabs Optical Power Meter (%s) at %s is online!\n", _query_usbtmc (LSOCKET,"*IDN?"), LSOCKET);
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
    _write_to_usbtmc (LSOCKET,_cmd);
    return 0;
  };
  ese = function( x )
  {
    _cmd = "*ESE";
    _write_to_usbtmc (LSOCKET,_cmd);
    return _read_from_usbtmc(LSOCKET);
  };
  esr = function()
  {
    _cmd = "*ESR";
    _write_to_usbtmc (LSOCKET,_cmd);
    return _read_from_usbtmc(LSOCKET);
  };
  esr = function()
  {
    _cmd = "*EVM";
    _write_to_usbtmc (LSOCKET,_cmd);
    return _read_from_usbtmc(LSOCKET);
  };
  idn = function()
  {
    _cmd = "*IDN?";
    return _query_usbtmc (LSOCKET,_cmd);
  };
  lrn = function ()
  {
    _cmd = "*LRN?";
    _write_to_usbtmc (LSOCKET,_cmd);
    return _read_from_usbtmc(LSOCKET);
  };
  opc = function( x )
  {
    _cmd = "*OPC";
    _write_to_usbtmc (LSOCKET,_cmd);
    return _read_from_usbtmc(LSOCKET);
  };
  opt = function()
  {
    _cmd = "*OPT";
    _write_to_usbtmc (LSOCKET,_cmd);
    return _read_from_usbtmc(LSOCKET);

  };
  rst = function ()
  {
    _cmd = "*RST";
    _write_to_usbtmc (LSOCKET,_cmd);
    return 0;
  };
  sre = function( x )
  {
    _cmd = "*SRE";
    _write_to_usbtmc (LSOCKET,_cmd);
    return _read_from_usbtmc(LSOCKET);
  };
  stb = function ()
  {
    _cmd = "*STB?";
    _write_to_usbtmc (LSOCKET,_cmd);
    return _read_from_usbtmc(LSOCKET);
  };
  trg = function ()
  {
    _cmd = "*TRG";
    _write_to_usbtmc (LSOCKET,_cmd);
    return 0;
  };
  wait = function ()
  {
    _cmd = "*WAI";
    _write_to_usbtmc (LSOCKET,_cmd);
    return 0;
  };
  abort = function ()
  {
    _cmd = "ABOR";
    _write_to_usbtmc (LSOCKET,_cmd);
    return 0;
  };
  event = function ( )
  {
    _cmd = "EVENT?";
    _write_to_usbtmc (LSOCKET,_cmd);
    return _read_from_usbtmc(LSOCKET);
  };
  //*****************************************************
  //
  // device specific SCPI commands
  //
  //*****************************************************
  public(wavelength_nm, responsivity, range, conf, meas, read, init, init_cont, conf_array, fetch,fetch_stat,delay);
  wavelength_nm = function ( x )
  {
    if (!exist(x))
    {
      _cmd = "SENS:CORR:WAV";
      r = _query_usbtmc(LSOCKET, _cmd + "?");
      return strtod(r)[1];
    }
    _cmd = num2str(int(x), _cmd + " %d");
    _write_to_usbtmc(LSOCKET,_cmd);
    return x;
  };
  responsivity = function ( x )
  {
    if (!exist(x))
    {
      _cmd = "SENS:CORR:POW?";
      r = _query_usbtmc(LSOCKET, _cmd);
      return strtod(r)[1];
    }
    _cmd = num2str(x + 0.0, "SENS:CORR:POW %f");
    _write_to_usbtmc(LSOCKET,_cmd);
    return x;
  };
  range = function ( x )
  {
    if (!exist(x))
    {
      _cmd = "SENS:CURR:RANG?";
      r = _query_usbtmc(LSOCKET, _cmd);
      return strtod(r)[1];
    }
    _cmd = num2str(x + 0.0, "SENS:CURR:POW %f");
    _write_to_usbtmc(LSOCKET,_cmd);
    return x;
  };
  conf = function ( x )
  {
    if (!exist(x))
    {
      _cmd = "CONF";
      r = _query_usbtmc(LSOCKET, _cmd + "?");
      r = strsplt(r,"\n")[1];
      return r;
    }
    _write_to_usbtmc(LSOCKET,_cmd + x);
    return x;
  };
  meas = function()
  {
    _cmd = "MEAS";
    r = _query_usbtmc(LSOCKET, _cmd + "?");
    return strtod(r)[1];
  };
  read = function()
  {
    _cmd = "READ";
    r = _query_usbtmc(LSOCKET, _cmd + "?");
    return strtod(r)[1];
  };
  init = function()
  {
    _cmd = "INIT";
    _write_to_usbtmc(LSOCKET,_cmd);
    return 0;
  };
  init_cont = function()
  {
    _cmd = "INIT:CONT";
    _write_to_usbtmc(LSOCKET,_cmd);
    return 0;
  };
  fetch_stat = function()
  {
    _cmd = "FETC:STAT?";
    _write_to_usbtmc(LSOCKET,_cmd);
    return 0;
  };
  delay = function()
  {
    _cmd = "DELAY";
    _write_to_usbtmc(LSOCKET,_cmd);
    return 0;
  };
  fetch = function ( x )
  {
    if (!exist(x))
    {
      _cmd = "FETC?";
      r = _query_usbtmc(LSOCKET, _cmd);
      return strtod(r);
    }
    _cmd = num2str(int(x), "FETC? %d");
    r = _query_usbtmc(LSOCKET,_cmd);
    return strtod(r);
  };
  conf_array = function(n, dt_us)
  {
    if ( !exist(n) || !exist(dt_us))
    {
      printf("conf_array: need to specify 'n' and 'dt_us' ");
      return 1;
    }
    if (dt_us < 100)
    {
      printf("conf_array: shortest 'dt_us' can be 100\n");
      dt_us = 100;
    }
    if (n < 1)
    {
      printf("conf_array: 'n' cannot be less than 1\n");
      n = 1;
    }
    sprintf(_cmd,"CONF:ARR %d, %d", int(n), int(dt_us));
    _write_to_usbtmc(LSOCKET,_cmd);
    return 0;
  };
};



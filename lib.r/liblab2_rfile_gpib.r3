//
// liblab_classdef_rfile_gpib.r3:
//    library of functions common to all instruments 
//    using  'gpib'  or  'serial' or 'tcp raw socket'
//
// (C) 2011,2016 Marijan Kostrun
//
//
// write, qread and read from the default device
//
static(INIT);
if (exist(INIT))
{ EOF }

_write_cmd_2_url = function(_url, _cmd, _eos, _debug)
{
  static(INIT_GPIB);
  if (!exist(INIT_GPIB))
  { INIT_GPIB = 0; }

  if (!isscalar(_url))
  { return 1; }

  if (all(strlen(_cmd) < 1))
  { return 2; }

  if (!exist(_eos))
  { _eos = ""; }

  if (!exist(_debug))
  { _debug = 0; }

  _this_solver = "_write_cmd_2_url";

  if (class(_url) == "num")
  {
    if (!INIT_GPIB)
    {
      // initialize gpib library in ~/rlab/lib.so/libgpib.so.r
      rfile libgpib.so
      INIT_GPIB = 1;
    }

    for (_c in _cmd)
    {
      if (strlen(_c)<1)
      { continue; }

      if (_debug)
      { printf(_this_solver + ": %s\n", _c); }

      rval = -ibwrt ( _url, _c + _eos);
    }
    return rval;
  }

  if (class(_url) != "string")
  { return 3;}

  if ( (strindex(_url, "tcp") != 1)  && ...
       (strindex(_url, "serial")!=1) && (strindex(_url, "tty")==0) )
  {
    return 4;
  }

  for (_c in _cmd)
  {
    if (strlen(_c)<1)
    { continue; }

    if (_debug)
    { printf(_this_solver + ": %s\n", _c); }

    rval = writem ( _SMU_ID_DEFAULT_DEVICE, _c + _SMU_EOS);
  }
  return rval;
};

_qread_from_url = function(_url, _cmd, _eos, _debug, _timeout)
{
  if (!isscalar(_url))
  { return 1; }

  static(SMU_TOC);
  if (!exist(SMU_TOC))
  { SMU_TOC = 31; }

  if (!exist(_timeout))
  { _timeout = 5; }

  static(INIT_GPIB);
  if (!exist(INIT_GPIB))
  { INIT_GPIB = 0; }

  global(_write_cmd_2_url);

  if (exist(_cmd))
  {
    _write_cmd_2_url (_url, _cmd, _eos, _debug);
  }

  _this_solver = "_qread_from_url";

  if (class(_url) == "num")
  {
    if (!INIT_GPIB)
    {
      // initialize gpib library in ~/rlab/lib.so/libgpib.so.r
      rfile libgpib.so
      INIT_GPIB = 1;
    }

    tic(SMU_TOC);
    rval = ibrd ( _url );
    while ((rval == "" || isempty(rval)) && toc(SMU_TOC) < _timeout)
    { rval = ibrd ( _url ); }

    if (isempty(rval))
    { rval = ""; }
    if (_debug)
    { printf(_this_solver + ": %s\n", rval); }
    return rval;
  }

  if (class(_url) != "string")
  { return 3;}

  if ( (strindex(_url, "tcp") != 1)  && ...
       (strindex(_url, "serial")!=1) && (strindex(_url, "tty")==0) )
  {
    return 4;
  }

  tic(SMU_TOC);
  rval = readm(_url);
  while ((toc(SMU_TOC)<_timeout) && (isempty(rval)))
  {
    rval = readm (_url);
  }

  if (isempty(rval))
  { rval = ""; }
  if (_debug)
  { printf(_this_solver + ": %s\n", rval); }

  return rval;
};

_gpib_address_from_url = function (_url)
{
  // check the protocol
  if (strlen(_url)<12)
  { return []; }

  if (strindex(_url, "gpib://") != 1)
  { return [];}

  _addr = strtod(strsplt(substr(_url, 8:strlen(_url)), ":"));

  if (length(_addr)!=3)
  { return []; }

  return _addr;
};

_init_url = function (url, url_opts, config_cmds)
{
  _this_solver = "_init_url";

  if (!exist(url))
  { error (_this_solver + "url of the device to be initialized is required!")}

  if (strindex(url, "gpib://") == 1)
  {
    // initialize gpib library in ~/rlab/lib.so/libgpib.so.r
    rfile libgpib.so

    _addr = strsplt(substr(url, 8:strlen(url)), ":");

    rval = ibdev(_addr[1],_addr[2],_addr[3], url_opts);
  }
  else
  {
    open  (url, url_opts);
    rval = url;
  }

  if ( exist(config_cmds) )
  {_write_cmd_2_url = function(_url, _cmd, _eos, _debug)

    _write_cmd_2_url(rval, config_cmds );
  }



  return url;
};


INIT = 1;










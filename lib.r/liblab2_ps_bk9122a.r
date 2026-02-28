//
// liblab_bk9122a.r:
//
static(_THIS_LIB, LIB_DEBUG, EOS);
_THIS_LIB = "liblab_bk9122a";
LIB_DEBUG = 0;
EOS = "\n";

static(_read_from_device, _write_to_device);
_read_from_device = function (url)
{
  _THIS_CMD = "_read_from_device";

  x = readm(url);
  if (LIB_DEBUG)
  { printf("%s: %s: %s\n", _THIS_LIB, _THIS_CMD, x); }

  // remove '\n' and '\r' and white space from the end of the string
  x = rstrip(x, "\r");
  x = rstrip(x, "\n");
  x = rstrip(x, " ");
  return x;
};

_write_to_device = function (url, _cmd )
{
  _THIS_CMD = "_write_to_device";

  for (_c in _cmd)
  {
    writem(url, _c + EOS);
    sleep (0.01);
    if (LIB_DEBUG)
    { printf("%s: %s: %s\n", _THIS_LIB, _THIS_CMD, _c); }
  }

  return rval;
};

// *********************************************
//
// commands for bk889b
//
// *********************************************

bk9122a_debug = function( arg1 )
{
  if (exist(arg1))
  {
    if (arg1>0)
    {
      LIB_DEBUG = 1;
    else
      LIB_DEBUG = 0;
    }
  }
  return LIB_DEBUG;
};

// GLOBAL FUNCTIONS
//
// set/read source levels
//
_bk_source_level = function(url, iv, arg1)
{
  if (iv == "v")
  {
    _cmd = "SOUR:VOLT:LEV";
  else
    _cmd = "SOUR:CURR:LEV";
  }

  if (exist(arg1))
  {
    rval = nan();
    if (class(arg1) == "num")
    {
      _cmd = _cmd + text(arg1[1]," %f");
      _write_to_device(url, _cmd);
      rval = arg1[1];
    }
    return rval;
  }

  _cmd = _cmd + "?";
  _write_to_device(url, _cmd);
  x = _read_from_device(url);

  if (isempty(x))
  {
    return nan();
  else
    return strtod(x);
  }
};

_bk_output = function(url, arg1)
{
  _cmd = "OUTP";

  if (exist(arg1))
  {
    rval = nan();
    if (class(arg1) == "num")
    {
      _cmd = _cmd + text((arg1[1]>0)," %.0f");
      _write_to_device(url, _cmd);
      rval = arg1[1];
    }
    return rval;
  }

  _cmd = _cmd + "?";
  _write_to_device(url, _cmd);
  x = _read_from_device(url);

  if (isempty(x))
  {
    return nan();
  else
    return strtod(x);
  }
};

_bk_local = function(url, ir)
{
  if (ir==1)
  {
    _cmd = "SYST:LOC";
  else
    _cmd = "SYST:REM";
  }

  _write_to_device(url, _cmd);
  x = _read_from_device(url);

  if (isempty(x))
  {
    return nan();
  else
    return strtod(x);
  }
};

_bk_meas = function(url, ir)
{
  if (ir==1)
  {
    _cmd = "MEAS:SCAL:VOLT?";
  else if (ir==2)
  {
    _cmd = "MEAS:SCAL:CURR?";
  else if (ir==3)
  {
    _cmd = "MEAS:SCAL:POW?";
  else if (ir==4)
  {
    _cmd = "MEAS:SCAL:RES?";
  }}}}

  _write_to_device(url, _cmd);
  x = _read_from_device(url);

  if (isempty(x))
  {
    return nan();
  else
    return strtod(x);
  }
};

bk9122a_init = function(url, url_opts)
{
  open (url, url_opts);
  sleep(1);

  rval = <<>>;
  rval.source = <<>>;

  // set/read voltage/current level
  s1 = "function(x){ return _bk_source_level(\""+url+"\",\"v\", x); };";
  rval.source.vlevel = eval(s1);
  s1 = "function(x){ return _bk_source_level(\""+url+"\", \"i\",x); };";
  rval.source.ilevel = eval(s1);

  // set/read output
  s1 = "function(x){ return _bk_output(\""+url+"\", x); };";
  rval.output = eval(s1);

  // enable/disable local/remote
  s1 = "function(x){ return _bk_local(\""+url+"\",1); };";
  rval.loc  = eval(s1);
  s1 = "function(x){ return _bk_local(\""+url+"\",0); };";
  rval.rem = eval(s1);

  // output mode
  s1 = "function(x){ return _bk_outmode(\""+url+"\",1); };";
  rval.cc = eval(s1);
  s1 = "function(x){ return _bk_outmode(\""+url+"\",0); };";
  rval.cv = eval(s1);

  // measure
  s1 = "function(x){ return _bk_meas(\""+url+"\",1); };";
  rval.measure.v = eval(s1);
  s1 = "function(x){ return _bk_meas(\""+url+"\",2); };";
  rval.measure.i = eval(s1);
  s1 = "function(x){ return _bk_meas(\""+url+"\",3); };";
  rval.measure.p = eval(s1);
  s1 = "function(x){ return _bk_meas(\""+url+"\",4); };";
  rval.measure.r = eval(s1);

  return rval;
};









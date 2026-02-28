//
// liblab_smu_rfile_gpib.r:
//    library of functions to control Keithley source meters
//    using  'gpib'  or  'raw socket'
//
// (C) 2011, Marijan Kostrun
//
static(_SMU_LIB);
_SMU_LIB = "liblab_smu_rfile_gpib";

// models supported by the library
static(SMU_MODELS);
SMU_MODELS = ["2602A", "2612A"];

// get generic (q)read,write functions
rfile liblab2_rfile_gpib

//
//
//
smuclass = classdef(arg1_url, arg2_url_opts, arg3_config_cmds)
{
  if (!exist(arg1_url))
  { error("classdef: smu: first argument 'url' is missing. Cannot continue!\n"); }

  private(SMU_MODELS);
  SMU_MODELS = ["2602A", "2612A"];

  static(_GPIB_SMU_IBRD_WAIT_TIME, _GPIB_SMU_IBRD_WAITS);
  _GPIB_SMU_IBRD_WAITS = 10;       // gpib: how many attempts at ibrd command
  _GPIB_SMU_IBRD_WAIT_TIME = 0.05; // gpib: how much to wait in seconds between the attempts
  // smu: nplc management:
  static(_SMU_NPLC_MIN, _SMU_NPLC_MAX, _SMU_NPLC_FAC, _SMU_NPLC_ACFREQ);
  _SMU_NPLC_MIN = 0.001;
  _SMU_NPLC_MAX = 25;
  _SMU_NPLC_FAC = 0.5;
  _SMU_NPLC_ACFREQ = 60; // (Hz) local frequency of the AC power input
  static(_SMU_DEBUG, _SMU_TIMEOUT, _SMU_TOC);
  _SMU_DEBUG = 0;
  _SMU_TIMEOUT = 5;
  _SMU_TOC = 31;

  // check the protocol
  static(_SMU_ID_DEVICE);
  if (strindex(arg1_url, "gpib://") == 1)
  {
    // initialize gpib library in ~/rlab/lib.so/libgpib.so.r
    rfile libgpib.so

    // figure gpib board, minor, major addresses
    _SMU_ID_DEVICE = <<>>;
    _SMU_ID_DEVICE.protocol = "gpib";
    _SMU_ID_DEVICE.address  = strtod(_addr);
    _SMU_ID_DEVICE.options  = url_opts;

    if (exist(url_opts.eos))
    { _SMU_EOS = url_opts.eos; }

    if ( ! exist(_SMU_ID_DEVICE.[ url ].id) )
    {
      _SMU_ID_DEVICE.[ url ].id = ibdev(...
          _SMU_ID_DEVICE.[url].address[1], ...
          _SMU_ID_DEVICE.[url].address[2], ...
              _SMU_ID_DEVICE.[url].address[3], ...
                  _SMU_ID_DEVICE.[url].options ...
                                       );
    }

    else if (strindex(url, "tcp://") == 1)
    {
    // if raw sockets, not much to do : we use rlab built-in interface to
    // raw socket communications implemented in open/close/readm/writem functions
      if (!exist(_SMU_ID_DEVICE.[url]))
      {
        _SMU_ID = [_SMU_ID, url];
      }

    // make note that it is 'tcp'
      _SMU_ID_DEVICE.[url] = <<>>;
      _SMU_ID_DEVICE.[url].protocol = "tcp";
      _SMU_ID_DEFAULT_DEVICE = url;

      if (exist(url_opts.eos))
      { _SMU_EOS = url_opts.eos; }

    // poke the instrument so it switches to REMote mode
    // keep it open within RLaB - this may prevent other devices from
    // accessing it when RLaB is not talking to device!
      open  (_SMU_ID_DEFAULT_DEVICE);
      readm (_SMU_ID_DEFAULT_DEVICE);
      writem(_SMU_ID_DEFAULT_DEVICE, _SMU_EOS);
      readm (_SMU_ID_DEFAULT_DEVICE);
      writem(_SMU_ID_DEFAULT_DEVICE, _SMU_EOS);
      readm (_SMU_ID_DEFAULT_DEVICE);

      else if (strindex(url, "serial://") == 1)
      {
    // if serial interface there is not much to do : we use rlab built-in interface to
    // serial port communications implemented in open/close/readm/writem functions
        _SMU_ID = [_SMU_ID, url];
        _SMU_ID_DEFAULT_DEVICE = url;

    // make note that it is 'tcp'
        _SMU_ID_DEVICE.[url] = <<>>;
        _SMU_ID_DEVICE.[url].protocol = "serial";
        _SMU_ID_DEVICE.[url].options  = url_opts;

        if (exist(url_opts.eos))
        { _SMU_EOS = url_opts.eos; }

    // keep it open within RLaB - this may prevent other devices from
    // accessing it when RLaB is not talking to device!
        open  (_SMU_ID_DEFAULT_DEVICE, url_opts);
        sleep (1);
        writem(_SMU_ID_DEFAULT_DEVICE, _SMU_EOS);
        readm (_SMU_ID_DEFAULT_DEVICE);
        writem(_SMU_ID_DEFAULT_DEVICE, _SMU_EOS);
        readm (_SMU_ID_DEFAULT_DEVICE);

      }}}

      if ( exist(config_cmds) )
      {
        _write_cmd_2_default_smu ( config_cmds );
      }

      //
  // at this point the instrument is initialized: we proceed to determine
  // its model
      //
      s = _qread_from_default_smu ("*IDN?");
      if (strindex(s, "2602"))
      { SMU_MODEL = "2602A"; }
      if (strindex(s, "2612"))
      { SMU_MODEL = "2612A"; }


  static(_url,SMU_MODEL,_write);
  if (strindex(arg1_url,"gpib://")==1)
  {
    _url = strtod(substr(arg1_url, 8:strlen(arg1_url)), <<csp=":">>);
    _write = function (_cmd)
    {
      for (_c in _cmd)
      {
        if (_SMU_DEBUG)
        { printf("write: %s\n", _c); }
        rval = -ibwrt ( _url, _c + _SMU_EOS);
      }
      return rval;
    };
  }
  else if ((strindex(arg1_url,"tcp://")==1)||(strindex(arg1_url,"serial://")==1))
  {
    _write = function (_cmd)
    {
      for (_c in _cmd)
      {
        if (_SMU_DEBUG)
        { printf("write: %s\n", _c); }
        rval = writem ( _url, _c + _SMU_EOS);
      }
      return rval;
    };
  }
  else
  {
    error("classdef: smu: first argument 'url' is improper. Cannot continue!\n");
  }

  public(url,model,debug);
  url = function(x)
  {
    if (type(x)=="string")
    { _url = x; }
    return _url;
  };
  model = function()
  {
    return SMU_MODEL;
  };
  debug = function(x)
  {
    if (exist(x))
    {
      if (x>0)
      { _SMU_DEBUG = 1; }
    }
    return _SMU_DEBUG;
  };
};

    


// ****************************************************
//
//  initialize connection to the instrument
//
// ****************************************************
$$.[ namespace + "_init_url" ] = function (url, url_opts, config_cmds)
{
  _THIS_FUNC = namespace  + "_init_url";
  if (!exist(url))
  { error (_THIS_FUNC + "url of the device to be initialized is required!")}

  // check the protocol
  if (strindex(url, "gpib://") == 1)
  {
    // initialize gpib library in ~/rlab/lib.so/libgpib.so.r
    rfile libgpib.so

    if ( ! exist(_SMU_ID_DEVICE.[ url ]) )
    {
      _SMU_ID = [_SMU_ID, url];

      _addr = strsplt(substr(url, 8:strlen(url)), ":");

      // figure gpib board, minor, major addresses
      _SMU_ID_DEVICE.[url] = <<>>;
      _SMU_ID_DEVICE.[url].protocol = "gpib";
      _SMU_ID_DEVICE.[url].address  = strtod(_addr);
      _SMU_ID_DEVICE.[url].options  = url_opts;
    }
    _SMU_ID_DEFAULT_DEVICE = url;

    if (exist(url_opts.eos))
    { _SMU_EOS = url_opts.eos; }

    if ( ! exist(_SMU_ID_DEVICE.[ url ].id) )
    {
      _SMU_ID_DEVICE.[ url ].id = ibdev(...
          _SMU_ID_DEVICE.[url].address[1], ...
          _SMU_ID_DEVICE.[url].address[2], ...
          _SMU_ID_DEVICE.[url].address[3], ...
          _SMU_ID_DEVICE.[url].options ...
          );
    }
  }
  else if (strindex(url, "tcp://") == 1)
  {
    // if raw sockets, not much to do : we use rlab built-in interface to
    // raw socket communications implemented in open/close/readm/writem functions
    if (!exist(_SMU_ID_DEVICE.[url]))
    {
      _SMU_ID = [_SMU_ID, url];
    }

    // make note that it is 'tcp'
    _SMU_ID_DEVICE.[url] = <<>>;
    _SMU_ID_DEVICE.[url].protocol = "tcp";
    _SMU_ID_DEFAULT_DEVICE = url;

    if (exist(url_opts.eos))
    { _SMU_EOS = url_opts.eos; }

    // poke the instrument so it switches to REMote mode
    // keep it open within RLaB - this may prevent other devices from
    // accessing it when RLaB is not talking to device!
    open  (_SMU_ID_DEFAULT_DEVICE);
    readm (_SMU_ID_DEFAULT_DEVICE);
    writem(_SMU_ID_DEFAULT_DEVICE, _SMU_EOS);
    readm (_SMU_ID_DEFAULT_DEVICE);
    writem(_SMU_ID_DEFAULT_DEVICE, _SMU_EOS);
    readm (_SMU_ID_DEFAULT_DEVICE);
  }
  else if (strindex(url, "serial://") == 1)
  {
    // if serial interface there is not much to do : we use rlab built-in interface to
    // serial port communications implemented in open/close/readm/writem functions
    _SMU_ID = [_SMU_ID, url];
    _SMU_ID_DEFAULT_DEVICE = url;

    // make note that it is 'tcp'
    _SMU_ID_DEVICE.[url] = <<>>;
    _SMU_ID_DEVICE.[url].protocol = "serial";
    _SMU_ID_DEVICE.[url].options  = url_opts;

    if (exist(url_opts.eos))
    { _SMU_EOS = url_opts.eos; }

    // keep it open within RLaB - this may prevent other devices from
    // accessing it when RLaB is not talking to device!
    open  (_SMU_ID_DEFAULT_DEVICE, url_opts);
    sleep (1);
    writem(_SMU_ID_DEFAULT_DEVICE, _SMU_EOS);
    readm (_SMU_ID_DEFAULT_DEVICE);
    writem(_SMU_ID_DEFAULT_DEVICE, _SMU_EOS);
    readm (_SMU_ID_DEFAULT_DEVICE);
  }

  if ( exist(config_cmds) )
  {
    _write_cmd_2_default_smu ( config_cmds );
  }

  //
  // at this point the instrument is initialized: we proceed to determine
  // its model
  //
  s = _qread_from_default_smu ("*IDN?");
  if (strindex(s, "2602"))
  { SMU_MODEL = "2602A"; }
  if (strindex(s, "2612"))
  { SMU_MODEL = "2612A"; }

  return 0;
};


// ****************************************************
//
//  set default device
//
// ****************************************************
$$.[ namespace + "_default" ] = function (url)
{
  _THIS_FUNC = namespace  + "_default: ";

  if (!exist(url))
  {
    return _SMU_ID_DEFAULT_DEVICE;
  }

  // check if it is defined
  if ( !exist(_SMU_ID_DEVICE.[ url ]) )
  { error (_THIS_FUNC + "the device is not declared. Use 'smu_init_url' first!"); }

  _SMU_ID_DEFAULT_DEVICE = url;

  return _SMU_ID_DEFAULT_DEVICE;
};


$$.[ namespace + "_debug" ] = function ( val )
{
  if (exist(val))
  {
    if (val == 1)
    {
      _SMU_DEBUG = 1;
    }
    else if (val == 0)
    {
      _SMU_DEBUG = 0;
    }
  }

  return _SMU_DEBUG;
};


//*****************************************************
//
//
// Keithley source meter commands - smua, smub
//
//
//*****************************************************
$$.[ namespace ] = <<>>;
$$.[ namespace + "a" ] = <<>>;
$$.[ namespace + "b" ] = <<>>;

// **************************
// reset
// **************************
static(_smu_reset);
_smu_reset = function( _ch )
{
  _THIS_FUNC = "_smu_reset: ";
  if (!exist( _SMU_ID_DEFAULT_DEVICE ))
  { error (_THIS_FUNC + "Cannot continue: Default device not defined!"); }

  _cmd = "smu" + _ch + ".reset()";

  if (_SMU_DEBUG)
  { printf(_THIS_FUNC + _cmd + "\n"); }
  rval = _write_cmd_2_default_smu( _cmd );

  return 0;
};

$$.[ namespace + "a" ].reset = function( )
{
  rval = _smu_reset("a");
  return rval;
};
$$.[ namespace + "b" ].reset = function( )
{
  rval = _smu_reset("b");
  return rval;
};


//
//
// smu.source
//
//
$$.[ namespace + "a" ].source = <<>>;
$$.[ namespace + "b" ].source = <<>>;

// **************************
// offlimit
// **************************
_smu_source_offlimiti = function(_ch, _ac )
{
  _THIS_FUNC = "_smu_source_offlimiti: ";
  if (!exist( _SMU_ID_DEFAULT_DEVICE ))
  { error (_THIS_FUNC + "Cannot continue: Default device not defined!"); }

  if (!exist(_ch))
  { _ch = "a"; }
  _ch = tolower(_ch);

  if (exist(_ac))
  {
    if (_ac > 0)
    {
      _cmd = "smu" + _ch + ".source.offlimiti = " + text(_ac);
      _write_cmd_2_default_smu( _cmd );
    }

    return _ac;
  }

  _cmd = [ ...
    "reading = smu" + _ch + ".source.offlimiti", ...
    "print(reading)" ...
  ];

  rval = strtod(_qread_from_default_smu( _cmd  ));
  return rval;
};

$$.[ namespace + "a" ].source.offlimiti = function( _ac )
{
  rval = _smu_source_offlimiti("a", _ac);
  return rval;
};
$$.[ namespace + "b" ].source.offlimiti = function( _ac )
{
  rval = _smu_source_offlimiti("b", _ac);
  return rval;
};

_smu_source_offmode = function(_ch, _ac )
{
  _THIS_FUNC = "_smu_source_offmode: ";
  if (!exist( _SMU_ID_DEFAULT_DEVICE ))
  { error (_THIS_FUNC + "Cannot continue: Default device not defined!"); }

  if (!exist(_ch))
  { _ch = "a"; }
  _ch = tolower(_ch);

  if (exist(_ac))
  {
    if (class(_ac) == "string")
    {
      if (strindex(tolower(_ac),"high"))
      {
        _cmd = "smu" + _ch + ".source.offmode = " + "smu" + _ch + ".OUTPUT_HIGH_Z";
      }
      else if (strindex(tolower(_ac),"zer"))
      {
        _cmd = "smu" + _ch + ".source.offmode = " + "smu" + _ch + ".OUTPUT_ZERO";
      }
      else
      {
        _cmd = "smu" + _ch + ".source.offmode = " + "smu" + _ch + ".OUTPUT_NORMAL";
      }
      _write_cmd_2_default_smu( _cmd );
    }

    return _ac;
  }

  _cmd = [ ...
    "reading = smu" + _ch + ".source.offmode", ...
    "print(reading)" ...
  ];
  rval = strtod(_qread_from_default_smu( _cmd ));

  if (rval == 0)
  {
    rval = "normal";
  }
  else if (rval == 1)
  {
    rval = "zero";
  }
  else
  {
    rval = "high";
  }
  return rval;
};
$$.[ namespace + "a" ].source.offmode = function( _ac )
{
  rval = _smu_source_offmode("a", _ac);
  return rval;
};
$$.[ namespace + "b" ].source.offmode = function( _ac )
{
  rval = _smu_source_offmode("b", _ac);
  return rval;
};

// **************************
// on, off
// **************************
static(_smu_source_output);
_smu_source_output = function(_ch, _ac )
{
  _THIS_FUNC = "_smu_source_output: ";
  if (!exist( _SMU_ID_DEFAULT_DEVICE ))
  { error (_THIS_FUNC + "Cannot continue: Default device not defined!"); }

  if (!exist(_ch))
  { _ch = "a"; }
  _ch = tolower(_ch);

  if (exist(_ac))
  {
    if (_ac)
    {
      _cmd = "smu" + _ch + ".source.output = smu" + _ch + ".OUTPUT_ON";
    }
    else
    {
      _cmd = "smu" + _ch + ".source.output = smu" + _ch + ".OUTPUT_OFF";
    }

    if (_SMU_DEBUG)
    { printf(_THIS_FUNC + _cmd + "\n"); }
    _write_cmd_2_default_smu( _cmd );

    return _ac;
  }

  _cmd = [ ...
    "reading = smu" + _ch + ".source.output", ...
    "print(reading)" ...
  ];

  rval = strtod(_qread_from_default_smu( _cmd  ));

  return rval;
};

$$.[ namespace + "a" ].source.output = function( _ac )
{
  rval = _smu_source_output("a", _ac);
  return rval;
};
$$.[ namespace + "b" ].source.output = function( _ac )
{
  rval = _smu_source_output("b", _ac);
  return rval;
};


// **************************
// limits and ranges
// **************************
static(_smu_limit_range);
_smu_limit_range = function( _ch, _func, _iv, _lr, _val )
{
  _THIS_FUNC = "_smu_limit_range: ";
  if (!exist( _SMU_ID_DEFAULT_DEVICE ))
  { error (_THIS_FUNC + "Cannot continue: Default device not defined!"); }

  if (exist(_val))
  {
    if (_val>0 && _val<inf())
    {
      _cmd = [ ...
        "smu" + _ch + "." + _func + ".autorange" + _iv + " = smu" + _ch + ".AUTORANGE_OFF", ...
        "smu" + _ch + "." + _func + "." + _lr + _iv + " = " + text(_val) ...
      ];
    }
    else
    {
      _cmd = [ ...
        "smu" + _ch + "." + _func + ".autorange" + _iv + " = smu" + _ch + ".AUTORANGE_ON" ...
      ];
    }

    if (_SMU_DEBUG)
    { printf(_THIS_FUNC + _cmd + "\n"); }
    _write_cmd_2_default_smu( _cmd);

    return _val;
  }

  _cmd = [ ...
    "reading = smu" + _ch + "."+_func+"." + _lr + _iv, ...
    "print(reading)" ...
  ];

  rval = _qread_from_default_smu( _cmd  );

  if (strindex(rval,"nil"))
  {
    rval = nan();
  }
  else
  {
    rval = strtod( rval );
  }

  return rval;
};

$$.[ namespace + "a" ].source.limitv = function(_v)
{
  rval = _smu_limit_range("a", "source", "v", "limit", _v);
  return rval;
};
$$.[ namespace + "a" ].source.limiti = function(_v)
{
  rval = _smu_limit_range("a", "source", "i", "limit", _v);
  return rval;
};

smub.source.limitv = function(_v)
{
  rval = _smu_limit_range("b", "source", "v", "limit", _v);
  return rval;
};
smub.source.limiti = function(_v)
{
  rval = _smu_limit_range("b", "source", "i", "limit", _v);
  return rval;
};
$$.[ namespace + "a" ].source.rangev = function(_v)
{
  rval = _smu_limit_range("a", "source", "v", "range", _v);
  return rval;
};
$$.[ namespace + "a" ].source.rangei = function(_v)
{
  rval = _smu_limit_range("a", "source", "i", "range", _v);
  return rval;
};
$$.[ namespace + "a" ].measure.rangev = function(_v)
{
  rval = _smu_limit_range("a", "measure", "v", "range", _v);
  return rval;
};
$$.[ namespace + "a" ].measure.rangei = function(_v)
{
  rval = _smu_limit_range("a", "measure", "i", "range", _v);
  return rval;
};
smub.source.rangev = function(_v)
{
  rval = _smu_limit_range("b", "source", "v", "range", _v);
  return rval;
};
smub.source.rangei = function(_v)
{
  rval = _smu_limit_range("b", "source", "i", "range", _v);
  return rval;
};
smub.measure.rangev = function(_v)
{
  rval = _smu_limit_range("b", "measure", "v", "range", _v);
  return rval;
};
smub.measure.rangei = function(_v)
{
  rval = _smu_limit_range("b", "measure", "i", "range", _v);
  return rval;
};

// **************************
// local (2wires) or remote (4 wires) sensing
// **************************
static(_smu_sense);
_smu_sense = function(_ch, _w )
{
  _THIS_FUNC = "_smu_sense: ";
  if (!exist( _SMU_ID_DEFAULT_DEVICE ))
  { error (_THIS_FUNC + "Cannot continue: Default device not defined!"); }

  if (exist(_w))
  {
    if (_w == "remote" || _w == 1)
    {
      _cmd = "smu" + _ch + ".sense = smu" + _ch + ".SENSE_REMOTE";
    }
    else if (_w == "local" || _w == 0)
    {
      _cmd = "smu" + _ch + ".sense = smu" + _ch + ".SENSE_LOCAL";
    }

    if (_SMU_DEBUG)
    { printf(_THIS_FUNC + _cmd + "\n"); }
    _write_cmd_2_default_smu( _cmd );

    return _w;
  }

  _cmd = [ ...
    "reading = smu" + _ch + ".sense", ...
    "print(reading)" ...
  ];

  rval = strtod(_qread_from_default_smu( _cmd  ));

  return rval;
};

$$.[ namespace + "a" ].sense = function( _w )
{
  rval = _smu_sense("a", _w);
  return rval;
};
smub.sense = function( _w )
{
  rval = _smu_sense("b", _w);
  return rval;
};


// **************************
// level
// **************************
static(_smu_source_level);
_smu_source_level = function( _ch, _iv, _val )
{
  _THIS_FUNC = "_smua_source_level: ";
  if (!exist( _SMU_ID_DEFAULT_DEVICE ))
  { error (_THIS_FUNC + "Cannot continue: Default device not defined!"); }

  if (exist(_val))
  {
    if (_iv == "i")
    {
      _cmd = "smu" + _ch + ".source.func = smu" + _ch +".OUTPUT_DCAMPS";
    }
    else if (_iv == "v")
    {
      _cmd = "smu" + _ch + ".source.func = smu" + _ch +".OUTPUT_DCVOLTS";
    }

    _cmd = [ ...
      _cmd, ...
      "smu" + _ch + ".source.level" + _iv + " = " + text(_val) ...
    ];

    if (_SMU_DEBUG)
    { printf(_THIS_FUNC + _cmd + "\n"); }
    _write_cmd_2_default_smu( _cmd);

    return _val;
  }

  _cmd = [ ...
    "reading = smu" + _ch + ".source.level" + _iv, ...
    "print(reading)" ...
  ];

  rval = _qread_from_default_smu( _cmd  );

  if (strindex(rval,"nil"))
  {
    rval = nan();
  }
  else
  {
    rval = strtod( rval );
  }

  return rval;
};

$$.[ namespace + "a" ].source.levelv = function( _val )
{
  rval = _smu_source_level("a", "v", _val);
  return rval;
};
$$.[ namespace + "a" ].source.leveli = function( _val )
{
  rval = _smu_source_level("a", "i", _val);
  return rval;
};
smub.source.levelv = function( _val )
{
  rval = _smu_source_level("b", "v", _val);
  return rval;
};
smub.source.leveli = function( _val )
{
  rval = _smu_source_level("b", "i", _val);
  return rval;
};



// **************************
// measure
// **************************
static(_smu_measure);
_smu_measure = function( _ch, _iv)
{
  _THIS_FUNC = "_smu_measure: ";
  if (!exist( _SMU_ID_DEFAULT_DEVICE ))
  { error (_THIS_FUNC + "Cannot continue: Default device not defined!"); }

  _cmd = [ ...
    "reading = smu" + _ch + ".measure." + _iv + "()", ...
    "print(reading)" ...
  ];

  rval = _qread_from_default_smu( _cmd  );

  if (strindex(rval,"nil"))
  {
    rval = nan();
  }
  else
  {
    rval = strtod( rval );
  }

  return rval;
};

$$.[ namespace + "a" ].measure.v = function( )
{
  rval = _smu_measure("a", "v");
  return rval;
};
$$.[ namespace + "a" ].measure.i = function( )
{
  rval = _smu_measure("a", "i");
  return rval;
};
$$.[ namespace + "a" ].measure.r = function( )
{
  rval = _smu_measure("a", "r");
  return rval;
};
$$.[ namespace + "b" ].measure.v = function( )
{
  rval = _smu_measure("b", "v");
  return rval;
};
$$.[ namespace + "b" ].measure.i = function( )
{
  rval = _smu_measure("b", "i");
  return rval;
};
$$.[ namespace + "b" ].measure.r = function( )
{
  rval = _smu_measure("b", "r");
  return rval;
};

// **************************
// speed: nplc
// **************************
static(_smu_measure_delay);
_smu_measure_delay = function( _ch, _what, _val )
{
  _THIS_FUNC = "_smu_measure_delay: ";
  if (!exist( _SMU_ID_DEFAULT_DEVICE ))
  { error (_THIS_FUNC + "Cannot continue: Default device not defined!"); }

  if (exist(_val))
  {
    _cmd = "smu" + _ch + ".measure."+_what+" = " + text(_val);
    _write_cmd_2_default_smu( _cmd);
    return _val;
  }

  _cmd = [ ...
    "reading = smu" + _ch + ".measure." + _what, ...
    "print(reading)" ...
  ];

  rval = _qread_from_default_smu( _cmd  );

  if (strindex(rval,"nil"))
  {
    rval = nan();
  }
  else
  {
    rval = strtod( rval );
  }

  return rval;
};
$$.[ namespace + "a" ].measure.delay = function(x)
{
  rval = _smu_measure_delay("a", "delay", x);
  return rval;
};
$$.[ namespace + "a" ].measure.delayfactor = function(x)
{
  rval = _smu_measure_delay("a", "delayfactor", x);
  return rval;
};
$$.[ namespace + "b" ].measure.delay = function(x)
{
  rval = _smu_measure_delay("b", "delay", x);
  return rval;
};
$$.[ namespace + "b" ].measure.delayfactor = function(x)
{
  rval = _smu_measure_delay("b", "delayfactor", x);
  return rval;
};

static(_smu_source_delay);
_smu_source_delay = function( _ch, _what, _val )
{
  _THIS_FUNC = "_smu_source_delay: ";
  if (!exist( _SMU_ID_DEFAULT_DEVICE ))
  { error (_THIS_FUNC + "Cannot continue: Default device not defined!"); }

  if (exist(_val))
  {
    _cmd = "smu" + _ch + ".source."+_what+" = " + text(_val);

    if (_SMU_DEBUG)
    { printf(_THIS_FUNC + _cmd + "\n"); }
    _write_cmd_2_default_smu( _cmd);

    return _val;
  }

  _cmd = [ ...
    "reading = smu" + _ch + ".source." + _what, ...
    "print(reading)" ...
  ];

  rval = _qread_from_default_smu( _cmd  );

  if (strindex(rval,"nil"))
  {
    rval = nan();
  }
  else
  {
    rval = strtod( rval );
  }

  return rval;
};
$$.[ namespace + "a" ].source.delay = function(x)
{
  rval = _smu_measure_delay("a", "delay", x);
  return rval;
};
$$.[ namespace + "b" ].source.delay = function(x)
{
  rval = _smu_measure_delay("b", "delay", x);
  return rval;
};
// **************************
// speed: nplc
// **************************
static(_smu_measure_nplc);
_smu_measure_nplc = function( _ch, _val )
{
  _THIS_FUNC = "_smu_measure_nplc: ";
  if (!exist( _SMU_ID_DEFAULT_DEVICE ))
  { error (_THIS_FUNC + "Cannot continue: Default device not defined!"); }

  if (exist(_val))
  {
    _cmd = "smu" + _ch + ".measure.nplc = " + text(_val);

    if (_SMU_DEBUG)
    { printf(_THIS_FUNC + _cmd + "\n"); }
    _write_cmd_2_default_smu( _cmd);

    return _val;
  }

  _cmd = [ ...
    "reading = smu" + _ch + ".measure.nplc", ...
    "print(reading)" ...
  ];

  rval = _qread_from_default_smu( _cmd  );

  if (strindex(rval,"nil"))
  {
    rval = nan();
  }
  else
  {
    rval = strtod( rval );
  }

  return rval;
};

$$.[ namespace + "a" ].measure.nplc = function( _val )
{
  rval = _smu_measure_nplc("a", _val);
  return rval;
};
smub.measure.nplc = function( _val )
{
  rval = _smu_measure_nplc("b", _val);
  return rval;
};



// **************************
//
// measure autozero
//
// **************************
static(_smu_measure_autozero);
_smu_measure_autozero = function( _ch, _val )
{
  _THIS_FUNC = "_smu_measure_autozero: ";
  if (!exist( _SMU_ID_DEFAULT_DEVICE ))
  { error (_THIS_FUNC + "Cannot continue: Default device not defined!"); }

  if (exist(_val))
  {
    if (_val == 0)
    {
      _v = "AUTOZERO_OFF";
    }
    else if (_val == 1)
    {
      _v = "AUTOZERO_ONCE";
    }
    else
    {
      _v = "AUTOZERO_AUTO";
    }
    _cmd = "smu" + _ch + ".measure.autozero = smu" + _ch + "." + _v;

    if (_SMU_DEBUG)
    { printf(_THIS_FUNC + _cmd + "\n"); }
    _write_cmd_2_default_smu( _cmd);

    return _val;
  }

  _cmd = [ ...
    "reading = smu" + _ch + ".measure.autozero", ...
    "print(reading)" ...
  ];

  rval = _qread_from_default_smu( _cmd  );

  if (strindex(rval,"nil"))
  {
    rval = nan();
  }
  else
  {
    rval = strtod( rval );
  }

  return rval;
};

$$.[ namespace + "a" ].measure.autozero = function( _val )
{
  rval = _smu_measure_autozero("a", _val);
  return rval;
};
smub.measure.autozero = function( _val )
{
  rval = _smu_measure_autozero("b", _val);
  return rval;
};


// ****************************************************
//
// pulse source 'x' linearly and measure 'y'
//
// ****************************************************
static(_smu_pulsex_gety)
_smu_pulsex_gety = function( _ch, _x, _rx, _ton, _toff, _limy)
{
  global(smua, smub);

  _THIS_FUNC = "_smu_sweeplin: ";
  if (!exist( _SMU_ID_DEFAULT_DEVICE ))
  { error (_THIS_FUNC + "Cannot continue: Default device not defined!"); }

  // figure out what is 'y'
  if (_x == "i")
  {
    _f = "OUTPUT_DCAMPS";
    _y = "v";
  }
  else
  {
    _f = "OUTPUT_DCVOLTS";
    _y = "i";
  }

  x1 = min(_rx);
  x2 = max(_rx);
  nm = _rx.n;

  // determine nlpc:
  nplc = min(_SMU_NPLC_MAX, max(_SMU_NPLC_MIN, _SMU_NPLC_FAC * _SMU_NPLC_ACFREQ * _ton));

  // disable autorange of the source and of the measure
  // choose function and set nlpc
  // clear buffer
  _cmd = [ ...
    "smu" + _ch + ".source.autorangev = smu" + _ch + ".AUTORANGE_OFF", ...
    "smu" + _ch + ".source.autorangei = smu" + _ch + ".AUTORANGE_OFF", ...
    "smu" + _ch + ".measure.autorangev = smu" + _ch + ".AUTORANGE_OFF", ...
    "smu" + _ch + ".measure.autorangei = smu" + _ch + ".AUTORANGE_OFF", ...
    "smu" + _ch + ".source.func = smu" + _ch + "." + _f, ...
    "smu" + _ch + ".measure.nplc = " + text(nplc), ...
    "format.data = format.ASCII", ...
    "smu" + _ch + ".nvbuffer1.clear()", ...
    "smu" + _ch + ".nvbuffer1.appendmode = 0"  ...
  ];
  if (_SMU_DEBUG)
  { printf(_THIS_FUNC + _cmd + "\n"); }
  _write_cmd_2_default_smu ( _cmd);

  // create request for sweep by the instrument
  // configure pulse:
  if (x1 != x2)
  {
    _cmd = [ ...
      "res,msg=ConfigPulse" + toupper(_x) + "Measure" + toupper(_y) + "SweepLin(" + ...
        "smu" + _ch + ", 0, "+text(x1)+", "+text(x2)+", "+text(_limy)+","+text(_ton)+ ...
          ", "+text(_toff)+", "+text(nm)+", smu" + _ch + ".nvbuffer1, 1)" ...
    ];
  }
  else
  {
    _cmd = [ ...
      "res,msg=ConfigPulse" + toupper(_x) + "Measure" + toupper(_y) + "(" + ...
        "smu" + _ch + ", 0, "+text(x1)+", "+text(_limy)+","+text(_ton)+ ...
          ", "+text(_toff)+", "+text(nm)+", smu" + _ch + ".nvbuffer1, 1)" ...
    ];
  }
  if (_SMU_DEBUG)
  { printf(_THIS_FUNC + _cmd + "\n"); }
  _write_cmd_2_default_smu ( _cmd);

  // did it go well?
  res = _qread_from_default_smu ( "print(res)");
  if (isempty(res))
  {
    printf( _SMU_LIB + _THIS_FUNC + "No response from the device!\n");
    printf( _SMU_LIB + _THIS_FUNC + "Is it configured properly?\n");
    return [];
  }
  if (strindex(res, "false"))
  {
    // there was an error in configuration:
    _smu_source_output (_ch, 0);  // turn output off

    _res  = _qread_from_default_smu( "print(msg)" );

    printf( _SMU_LIB + _THIS_FUNC + "Error in pulse execution occured!\n");
    printf( _SMU_LIB + _THIS_FUNC + "SMU reports: ");
    colors ("red");
    printf ("\n%s\n", _res);
    colors ();

    return [];
  }

  // prepare for the measurement
  _smu_source_output (_ch, 1);    // turn output on
  _smu_measure_autozero(_ch, 1);  // autozero it once

  // do the test
  _cmd = [ ...
    "res,msg = InitiatePulseTest(1)", ...
    "print(res)" ...
  ];

  res = _qread_from_default_smu( _cmd );
  if (strindex(res, "false"))
  {
    // there was an error in configuration
    _smu_source_output (_ch, 0);

    _res = _qread_from_default_smu( "print(msg)" );

    printf( _SMU_LIB + _THIS_FUNC + "Error in pulse execution occured!\n");
    printf( _SMU_LIB + _THIS_FUNC + "SMU reports: ");
    colors ("red");
    printf ("\n%s\n", _res);
    colors ();

    return [];
  }

  _cmd = "printbuffer(1,"+text(nm)+",smu" + _ch + ".nvbuffer1.readings)";
  res = _qread_from_default_smu (_cmd);

  if (isempty(res) || res == "")
  {
    data = [];
  }
  else
  {

    data = strtod(res)';

    //
    if (nm > 1)
    {
      dx = (x2-x1)/(nm-1);
      xi = x1 + dx .* [0:nm-1]';
    }
    else
    {
      xi = x1;
    }
    data = [xi, data];

  }

  _smu_source_output (_ch, 0);

  return data;
};

smua.PulseIMeasureV = function( ri, limv, ton, toff)
{
  rval = _smu_pulsex_gety( "a", "i", ri, ton, toff, limv);
  return rval;
};
smua.PulseVMeasureI = function( rv, limi, ton, toff)
{
  rval = _smu_pulsex_gety( "a", "v", rv, ton, toff, limi);
  return rval;
};
smub.PulseIMeasureV = function( ri, limv, ton, toff)
{
  rval = _smu_pulsex_gety( "b", "i", ri, ton, toff, limv);
  return rval;
};
smub.PulseVMeasureI = function( rv, limi, ton, toff)
{
  rval = _smu_pulsex_gety( "b", "v", rv, ton, toff, limi);
  return rval;
};

//
//
// functions for the entire instrument
//
//
$$.[ namespace ].lcl = function()
{
  _cmd = "GTL";
  _write_cmd_2_default_smu ( _cmd );
  return 0;  
};

$$.[ namespace ].display = <<>>;
$$.[ namespace ].display.clear = function ()
{
  _cmd = "display.clear()";
  _write_cmd_2_default_smu ( _cmd );
  return 0;
};

$$.[ namespace ].display.cursor = function(row, col)
{
  if (exist(row) || exist(col))
  {
    if (!exist(row))
    { row = 1; }
    if (!exist(col))
    { col = 1; }
    row = max(1,min([ 2,row]));
    col = max(1,min([32,col]));

    _cmd = "display.setcursor("+text(row,"%.0f")+","+text(col,"%.0f")+")";
    if (_SMU_DEBUG)
    { printf(_THIS_FUNC + _cmd + "\n"); }
    _write_cmd_2_default_smu ( _cmd);

    return 0;
  }

  _cmd = "row,col = display.getcursor()";
  if (_SMU_DEBUG)
  { printf(_THIS_FUNC + _cmd + "\n"); }
  _write_cmd_2_default_smu ( _cmd);

  _cmd = "print(row)";
  s = _qread_from_default_smu (_cmd);
  row = strtod(s);

  _cmd = "print(col)";
  s = _qread_from_default_smu (_cmd);
  col = strtod(s);

  return [row,col];
};

$$.[ namespace ].display.text = function(msg)
{
  if (exist(msg))
  {
    _cmd = "display.settext(\""+ msg + "\")";
    _write_cmd_2_default_smu ( _cmd );
    return 0;
  }

  _cmd = "text = display.gettext()";
  _write_cmd_2_default_smu ( _cmd );
  _cmd = "print(text)";
  s = _qread_from_default_smu (_cmd);

  return s;
};

$$.[ namespace ].display.screen = function( id )
{
  if (id == 0 || id=="a" || id=="A")
  {
    _cmd = "display.screen = display.SMUA";
  }
  else if (id == 1 || id=="b" || id=="B")
  {
    _cmd = "display.screen = display.SMUB";
  }
  else if (id == 2 || id=="ab" || id=="AB")
  {
    _cmd = "display.screen = display.SMUA_SMUB";
  }
  else if (id == 3 || id=="user" || id=="USER")
  {
    _cmd = "display.screen = display.USER";
  }

  if (_SMU_DEBUG)
  { printf(_THIS_FUNC + _cmd + "\n"); }
  _write_cmd_2_default_smu ( _cmd);

  return 0;
};


$$.[ namespace ].info = function()
{
  _cmd = "*IDN?";
  s = _qread_from_default_smu (_cmd);

  rval = <<>>;
  if (class(s) == "string")
  {
    s = strsplt(s, ",");
    if (length(s)>=1)
    { rval.maker = s[1]; }
    if (length(s)>=2)
    { rval.model = s[2]; }
    if (length(s)>=3)
    { rval.serial = s[3]; }
    if (length(s)>=4)
    { rval.firmware = s[4]; }
  }

  return rval;
};

$$.[ namespace ].digio = <<>>;
$$.[ namespace ].digio.write = function(val1, val2)
{

  if (!exist(val1))
  { return 1; }

  if(!exist(val2))
  {
    if (val1<0 || val1 > 255)
    { return 1; }

    _cmd = "digio.writeport(%.0f)";
    _cmd = text(val1, _cmd);
    _write_cmd_2_default_smu(_cmd);
    return 0;
  }

  if (any(val1<1) || any(val1 > 14))
  { return 1; }
  s1 = text(val1, "%.0f");
  s2 = text(val2>0, "%.0f");
  _cmd = "digio.writebit(" + s1 + "," + s2 +")";
  _write_cmd_2_default_smu(_cmd);
  return 0;
};



// ****************************************************
//
// sweep source 'x' linearly and measure 'y'
//
// ****************************************************
static(_smu_sweepx_gety)
_smu_sweepx_gety = function( ch, x, mod, startx, stopx, stime, np)
{
  np=int(np);

  _THIS_FUNC = "_smu_sweepx_gety: ";
  if (!exist( _SMU_ID_DEFAULT_DEVICE ))
  { error (_THIS_FUNC + "Cannot continue: Default device not defined!"); }

  global(smua, smub);

  CH="smua";
  if (strindex(tolower(ch), "b"))
  { CH = "smub"; }
  MOD= "Lin";
  if (strindex(tolower(mod), "log"))
  { MOD = "Log"; }
  SWEEP= "V"; MEAS="I";FUNC="OUTPUT_DCVOLTS";
  if (strindex(tolower(x), "i"))
  { SWEEP = "I"; MEAS="V"; FUNC="OUTPUT_DCAMPS"; }

  _cmd = [ ...
    "smu" + ch + ".source.autorangev = smu" + ch + ".AUTORANGE_ON", ...
    "smu" + ch + ".source.autorangei = smu" + ch + ".AUTORANGE_ON", ...
    "smu" + ch + ".measure.autorangev = smu" + ch + ".AUTORANGE_ON", ...
    "smu" + ch + ".measure.autorangei = smu" + ch + ".AUTORANGE_ON", ...
    "smu" + ch + ".source.func = smu" + ch + "." + FUNC, ...
    "format.data = format.ASCII", ...
    "smu" + ch + ".nvbuffer1.clear()", ...
    "smu" + ch + ".nvbuffer1.appendmode = 0"  ...
  ];
  _write_cmd_2_default_smu ( _cmd );

  _cmd = "Sweep"+SWEEP+MOD+"Measure"+MEAS;
  _cmd  = _cmd + "(smu" + ch + ", "+text([startx,stopx,stime,np],"%g",",")+")";
  res = _write_cmd_2_default_smu (_cmd);

  _cmd = "printbuffer(1,"+text(np)+",smu" + ch + ".nvbuffer1.readings)";
  res = _qread_from_default_smu (_cmd);

  tic(_SMU_TOC);
  while ( (toc(_SMU_TOC)<_SMU_TIMEOUT) && (length(strsplt(res,",")) < np) )
  {
    res = res + _qread_from_default_smu ();
  }

  y = strtod(res,",")';
  if (length(y)>0)
  {
    x = startx + (stopx - startx) .* [0:(np-1)]' ./ (length(y)-1);
    return [x,y];
  }

  return [];
};

smua.SweepVLinMeasureI = function(startx, stopx, stime, np)
{
  rval = _smu_sweepx_gety( "a", "v", "lin", startx, stopx, stime, np);
  return rval;
};
smub.SweepVLinMeasureI = function(startx, stopx, stime, np)
{
  rval = _smu_sweepx_gety( "b", "v", "lin", startx, stopx, stime, np);
  return rval;
};
smua.SweepILinMeasureV = function(startx, stopx, stime, np)
{
  rval = _smu_sweepx_gety( "a", "i", "lin", startx, stopx, stime, np);
  return rval;
};
smub.SweepILinMeasureV = function(startx, stopx, stime, np)
{
  rval = _smu_sweepx_gety( "b", "i", "lin", startx, stopx, stime, np);
  return rval;
};

smua.SweepVLogMeasureI = function(startx, stopx, stime, np)
{
  rval = _smu_sweepx_gety( "a", "v", "log", startx, stopx, stime, np);
  return rval;
};
smub.SweepVLogMeasureI = function(startx, stopx, stime, np)
{
  rval = _smu_sweepx_gety( "b", "v", "log", startx, stopx, stime, np);
  return rval;
};
smua.SweepILogMeasureV = function(startx, stopx, stime, np)
{
  rval = _smu_sweepx_gety( "a", "i", "log", startx, stopx, stime, np);
  return rval;
};
smub.SweepILogMeasureV = function(startx, stopx, stime, np)
{
  rval = _smu_sweepx_gety( "b", "i", "log", startx, stopx, stime, np);
  return rval;
};


// ****************************************************
//
//  max compliance source ranges, p.3-5
//
// ****************************************************
static(_smu_source_vi_maxrange, _smu_source_iv_maxrange);
_smu_source_vi_maxrange = <<>>;
_smu_source_vi_maxrange.["2602A"] = [...
    0.1, 3; ...
      1, 3; ...
      6, 3; ...
     40, 1  ];
_smu_source_iv_maxrange.["2602A"] = [...
    1e-7, 40; ...
    1e-6, 40; ...
    1e-5, 40; ...
    1e-4, 40; ...
    1e-3, 40; ...
    1e-2, 40; ...
    1e-1, 40; ...
       1, 40; ...
       3,  6  ];
_smu_source_vi_maxrange.["2612A"] = [...
    0.2, 1.5; ...
      2, 1.5; ...
     20, 1.5; ...
    200, 0.1  ];
_smu_source_iv_maxrange.["2612A"] = [...
    1e-7, 200; ...
    1e-6, 200; ...
    1e-5, 200; ...
    1e-4, 200; ...
    1e-3, 200; ...
    1e-2, 200; ...
    1e-1, 200; ...
       1, 20; ...
     1.5, 20  ];

static(_smu_measure_i_range, _smu_measure_v_range);
// 2602A:
_smu_measure_i_range.["2602A"] = [100e-9, 1e-6, 10e-6, 100e-6, 1e-3, 10e-3, 100e-3, 1, 3];
_smu_measure_v_range.["2602A"] = [0.1, 1, 6, 40];
// 2612A:
_smu_measure_i_range.["2612A"] = [100e-9, 1e-6, 10e-6, 100e-6, 1e-3, 10e-3, 100e-3, 1.5];
_smu_measure_v_range.["2612A"] = [0.2, 2, 20, 200];

//
// range current
//
smu_range_i = function ( range )
{
  if (!exist(range))
  { return _smu_measure_i_range.[SMU_MODEL]; }

  if (range > max(_smu_measure_i_range.[SMU_MODEL]))
  {
    printf ("Warning: Attempting to source current that is beyond the capacity of %s!\n", SMU_MODEL);
    return [];
  }

  i = find (range <= _smu_measure_i_range.[SMU_MODEL]);
  i = min( i );

  return _smu_measure_i_range.[SMU_MODEL][i];
};

smu_range_i_maxv = function (range)
{
  global(smu_range_i);

  irange = smu_range_i(range);
  idx_i_maxv = find(_smu_source_iv_maxrange.[SMU_MODEL][;1] == irange);

  if (isempty(idx_i_maxv))
  { return [];}

  return max(_smu_source_iv_maxrange.[SMU_MODEL][idx_i_maxv;2]);
};

//
// range voltage
//
smu_range_v = function ( range )
{
  if (!exist(range))
  { return _smu_measure_v_range.[SMU_MODEL]; }

  if (range > max(_smu_measure_v_range.[SMU_MODEL]))
  {
    printf ("Warning: Attempting to source voltage that is beyond the capacity of %s!\n", SMU_MODEL);
    return [];
  }

  i = find (range <= _smu_measure_v_range.[SMU_MODEL]);
  i = min( i );

  return _smu_measure_v_range.[SMU_MODEL][i];
};

smu_range_v_maxi = function (range)
{
  global(smu_range_v);
  vrange = smu_range_v(range);
  idx_v_maxi = find(_smu_source_vi_maxrange.[SMU_MODEL][;1] == vrange);
  if (isempty(idx_maxv))
  { return [];}

  return max(_smu_source_vi_maxrange.[SMU_MODEL][idx_i_maxv;2]);
};
















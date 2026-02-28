//
// liblab_pid_omg_cni.r3:
//
static(INIT);

if (exist(INIT))
{
  EOF
}

static(_THIS_LIB, LIB_DEBUG);
_THIS_LIB = "liblab_pid_julabo";
LIB_DEBUG = 0;

static(URL_OPTS);
URL_OPTS = <<>>;
URL_OPTS.speed = 9600;
URL_OPTS.data_parity_stop = "8N1";
URL_OPTS.flow_control = "x";
URL_OPTS.debug = 0;
URL_OPTS.eol = "\r";

julabo_blackbody = classdef(dev)
{
  // static = class-member private declaration:
  //    dynamic memory storage that follows the class-member
  static(_device);
  
  //
  if ( (class(dev)!="string") || (length(dev)!=1) )
  {
    EOF
  }
  _device = dev;
  if (!open(_device,URL_OPTS))
  { sleep(0.5); }

  public(device, temp, heating_power, temp_safety_sensor, temp_safe, t1, t2, t3, t_low, t_high, ...
      pump, param, cntl, circulator, status,set_point, version);

  device  = function()
  {
    return _device;
  };

  

  version = function()
  {
    _cmd = "version";
    writem(_device, _cmd);
    sleep(0.1);
    t = readm(_device);
    chomp(t);
    return t;
  };

  set_point = function(val)
  {
    if (class(val) == "string")
    {
      if (val == "t1")
      {
        _cmd = "out_mode_01 0";
      }
      else if (val == "t2")
      {
        _cmd = "out_mode_01 1";
      }
      else if (val == "t3")
      {
        _cmd = "out_mode_01 2";
      }
      else
      {
        printf("set_point: Unknown value. Cannot continue!\n");
        return blank();
      }
      writem(_device, _cmd);
      sleep (0.1);
    }

    _cmd = "in_mode_01";
    writem(_device, _cmd);
    sleep(0.1);
    t = readm(_device);
    chomp(t);
    t = strtod(t);
    if (t == 0)
    { return "t1"; }
    if (t == 1)
    { return "t2"; }
    if (t == 2)
    { return "t3"; }

    return blank();
  };

  status = function()
  {
    _cmd = "status";
    writem(_device, _cmd);
    sleep(0.1);
    t = readm(_device);
    chomp(t);
    i = strindex(t, " ");
    rval = <<>>;
    rval.status = strtod(substr(t,1:(i-1)));
    rval.desc = substr(t,(i+1):strlen(t));
    return rval;
  };

  circulator = function(val)
  {
    if (class(val) == "num")
    {
      if (val == 0 || val == 1)
      {
        _cmd = num2str(val,"out_mode_05 %.0f");
        writem(_device, _cmd);
        sleep (0.1);
      }
    }

    _cmd = "in_mode_05";
    writem(_device, _cmd);
    sleep(0.1);
    t = readm(_device);
    chomp(t);
    return strtod(t);
  };

  temp = function()
  {
    _cmd = "in_pv_00";
    writem(_device, _cmd);
    sleep(0.1);
    t = readm(_device);
    chomp(t);
    return strtod(t);
  };

  heating_power = function()
  {
    _cmd = "in_pv_01";
    writem(_device, _cmd);
    sleep(0.1);
    t = readm(_device);
    chomp(t);
    return strtod(t);
  };

  temp_safety_sensor = function()
  {
    _cmd = "in_pv_03";
    writem(_device, _cmd);
    sleep(0.1);
    t = readm(_device);
    chomp(t);
    return strtod(t);
  };

  temp_safe = function()
  {
    _cmd = "in_pv_04";
    writem(_device, _cmd);
    sleep(0.1);
    t = readm(_device);
    chomp(t);
    return strtod(t);
  };

  t1 = function( val )
  {
    if (class(val) == "num")
    {
      if (val > 20 && val < 85)
      {
        _cmd = num2str(val,"out_sp_00 %.2f");
        writem(_device, _cmd);
        sleep (0.1);
      }
    }
    _cmd = "in_sp_00";
    writem(_device, _cmd);
    sleep(0.1);
    t = readm(_device);
    chomp(t);
    return strtod(t);
  };

  t2 = function( val )
  {
    if (class(val) == "num")
    {
      if (val > 20 && val < 85)
      {
        _cmd = num2str(val,"out_sp_01 %.2f");
        writem(_device, _cmd);
        sleep (0.1);
      }
    }
    _cmd = "in_sp_01";
    writem(_device, _cmd);
    sleep(0.1);
    t = readm(_device);
    chomp(t);
    return strtod(t);
  };

  t3 = function( val )
  {
    if (class(val) == "num")
    {
      if (val > 20 && val < 85)
      {
        _cmd = num2str(val,"out_sp_02 %.2f");
        writem(_device, _cmd);
        sleep (0.1);
      }
    }
    _cmd = "in_sp_02";
    writem(_device, _cmd);
    sleep(0.1);
    t = readm(_device);
    chomp(t);
    return strtod(t);
  };

  t_high = function( val )
  {
    if (class(val) == "num")
    {
      if (val > 20 && val < 85)
      {
        _cmd = num2str(val,"out_sp_03 %.2f");
        writem(_device, _cmd);
        sleep (0.1);
      }
    }
    _cmd = "in_sp_03";
    writem(_device, _cmd);
    sleep(0.1);
    t = readm(_device);
    chomp(t);
    return strtod(t);
  };

  t_low = function( val )
  {
    if (class(val) == "num")
    {
      if (val > 20 && val < 85)
      {
        _cmd = num2str(val,"out_sp_04 %.2f");
        writem(_device, _cmd);
        sleep (0.1);
      }
    }
    _cmd = "in_sp_04";
    writem(_device, _cmd);
    sleep(0.1);
    t = readm(_device);
    chomp(t);
    return strtod(t);
  };

  pump = function( val )
  {
    if (class(val) == "num")
    {
      if (val >= 1 && val <= 4)
      {
        _cmd = num2str(val,"out_sp_07 %.0f");
        writem(_device, _cmd);
        sleep (0.1);
      }
    }
    _cmd = "in_sp_07";
    writem(_device, _cmd);
    sleep(0.1);
    t = readm(_device);
    chomp(t);
    return strtod(t);
  };

  param = <<>>;
  param.te = function( val )
  {
    _cmd = "in_par_01";
    writem(_device, _cmd);
    sleep(0.1);
    t = readm(_device);
    chomp(t);
    return strtod(t);
  };
  param.si = function( val )
  {
    _cmd = "in_par_02";
    writem(_device, _cmd);
    sleep(0.1);
    t = readm(_device);
    chomp(t);
    return strtod(t);
  };
  param.ti = function( val )
  {
    _cmd = "in_par_03";
    writem(_device, _cmd);
    sleep(0.1);
    t = readm(_device);
    chomp(t);
    return strtod(t);
  };

  cntl = <<>>;
  cntl.xp = function( val )
  {
    if (class(val) == "num")
    {
      if (val >= 0 && val <= 999)
      {
        _cmd = num2str(val,"out_par_06 %.2f");
        writem(_device, _cmd);
        sleep (0.1);
      }
    }
    _cmd = "in_par_06";
    writem(_device, _cmd);
    sleep(0.1);
    t = readm(_device);
    chomp(t);
    return strtod(t);
  };
  cntl.tn = function( val )
  {
    if (class(val) == "num")
    {
      if (val >= 0 && val <= 999)
      {
        _cmd = num2str(val,"out_par_07 %.0f");
        writem(_device, _cmd);
        sleep (0.1);
      }
    }
    _cmd = "in_par_07";
    writem(_device, _cmd);
    sleep(0.1);
    t = readm(_device);
    chomp(t);
    return strtod(t);
  };
  cntl.tv = function( val )
  {
    if (class(val) == "num")
    {
      if (val >= 0 && val <= 999)
      {
        _cmd = num2str(val,"out_par_08 %.0f");
        writem(_device, _cmd);
        sleep (0.1);
      }
    }
    _cmd = "in_par_08";
    writem(_device, _cmd);
    sleep(0.1);
    t = readm(_device);
    chomp(t);
    return strtod(t);
  };

};



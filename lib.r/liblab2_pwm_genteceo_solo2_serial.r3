///
///
///
// static(URL, URL_OPTS,CHIP_ADD,TIME_TO_WAIT,WAIT_TIMER_ID);
// if (!exist(URL))
// {
//   attr=<<>>;
//   attr.devname = "tty";
//   attr.id_vendor_id = "0403";
//   attr.id_serial_short = "A80041vD";  // usb cable
//   CHIP_ADD = usbdevname(attr);
//
//   if (isempty(CHIP_ADD))
//   { stop("Horrible internal error: Cannot find the contoller!\n"); }
//
//   URL_OPTS = <<>>;
//   URL_OPTS.data_format  = "8N1";
//   URL_OPTS.speed    = 115200;
//   URL_OPTS.debug    = 0;
//   URL_OPTS.hupcl    = 1;
//   URL_OPTS.flow_control = "hardware";
//   URL = "serial://" + CHIP_ADD;
//
//   // global variables for user:
//   url = URL;
//   url_opts = URL_OPTS;
//
//   //
//   TIME_TO_WAIT  = 30;
//   WAIT_TIMER_ID = 23;
// }
//
// printf("Gentec-EO Laser Power/Energy Meter is at %s\n", CHIP_ADD);
//
// if (!open(URL,URL_OPTS))
// { sleep (3); }

static(_THIS_LIB, _LIB_DEBUG);
if (!exist(_THIS_LIB))
{
  _THIS_LIB = "liblab2_pwm_genteceo_solo2_serial: ";
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

static  (_stat_params_names);
_stat_params_names = [...
    "val", ...
    "max", ...
    "min", ...
    "avg", ...
    "std", ...
    "rms_stability", ...
    "ptp_stability", ...
    "time", ...
    "acquisition_time", ...
    "raw", ...
[]];

static (_sampling_params_names);
_sampling_params_names = [...
    "rate", ...
    "period", ...
    "length", ...
    "time_stamp", ...
[]];

static (divider_unit);
divider_unit = function (x)
{
  rval = ones(x);
  idx2  = find(x==2);
  idx3  = find(x==3);
  idx4  = find(x==4);
  idx5  = find(x==5);
  idx64 = find(x==64);
  if (!isempty(idx2))
  {
    rval [idx2]  = 1e-3 .* ones(idx2);
  }
  if (!isempty(idx3))
  {
    rval [idx3]  = 1e-6 .* ones(idx3);
  }
  if (!isempty(idx4))
  {
    rval [idx4]  = 1e-9 .* ones(idx4);
  }
  if (!isempty(idx5))
  {
    rval [idx5]  = 1e-12 .* ones(idx5);
  }
  if (!isempty(idx64))
  {
    rval [idx64] = 1e-15 .* ones(idx64);
  }
  return rval;
};

static (_sampling_timeunit_to_index, _sampling_timeunit_to_duration);
_sampling_timeunit_to_index = <<>>;
_sampling_timeunit_to_index.["sec"] = 0;
_sampling_timeunit_to_index.["min"] = 1;
_sampling_timeunit_to_index.["hou"] = 2;
_sampling_timeunit_to_index.["day"] = 3;
_sampling_timeunit_to_index.["wee"] = 4;
//
_sampling_timeunit_to_duration = <<>>;
_sampling_timeunit_to_duration.["per"] = 0;
_sampling_timeunit_to_duration.["sec"] = 1;
_sampling_timeunit_to_duration.["min"] = 2;
_sampling_timeunit_to_duration.["hou"] = 3;
_sampling_timeunit_to_duration.["day"] = 4;
_sampling_timeunit_to_duration.["wee"] = 5;

static(_qreadnbytes, _qreadnbytes_of_data);
_qreadnbytes = function (ttyport, bincmd)
{
  writem(ttyport, bincmd);
  x = read_nbytes(ttyport,4);
  n = bytejoin(x[3:4],1,"uint16");
  if (x[2] == 10L)
  {
    x = read_nbytes(ttyport, n);
    //x
    return x;
  }
  return _error_msgs[n + 0.0];
};

_qreadnbytes_of_data = function (ttyport, bincmd)
{
  writem(ttyport, bincmd);
  sleep(0.1);
  x = read_nbytes(ttyport,4);
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
  if (x[2] == 11L)
  {
    if (x.n == 4)
    {
      err_no = bytejoin(x[3:4],1,"uint16") + 0.0;
    }
    rval.err = _error_msgs[ err_no ];
    return rval;
  }
  x = read_nbytes(ttyport,8);
  n_header = bytejoin(x[1:4],1,"uint32");
  n_data   = bytejoin(x[5:8],1,"uint32");
//  [n_header, n_data]?
  rval.header = char(read_nbytes(ttyport, n_header));
  data_val    = read_nbytes(ttyport, n_data);
  data_val    = bytejoin(data_val,0,"uint32");
  resize (data_val,6,data_val.n/6);
  data_val = data_val';
  rval.val = [data_val[;5].*divider_unit(data_val[;6]),data_val[;3].*divider_unit(data_val[;4])];
  return rval;
};

static(_readstring);
_readstring = function (ttyport, eol)
{
  if (!exist(eol))
  {
    eol = "\n";
  }
  if (strlen(eol)>0)
  {
    x="";
    tic(12);
    _l0 = -1;
    _l1 = 0;
    while (substr(x,strlen(x))!="\n")
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
  }

  // special case when eol is number like 255
  if (eol != 255L)
  {
    return [];
  }
  x="";
  _l0 = -1;
  _l1 = 0;
  dt = 0.1;
  tic(12);
  while ((_l0 < _l1) || (_l1<1))
  {
    _l0 = _l1;
    x = x + readm(ttyport);
    if (_l0 < _l1)
    {
      tic(12);
    }
    if (toc(12)>0.5)
    {
      break;
    }
    if (strindex(x,"EOF")>0)
    {
      break;
    }
    _l1 = strlen(x);
    //[_l0, _l1 ]?
  }
//  x?
//   while (strindex(x,"EOF")<1)
//   {
//    sleep(0.2);
//    x = x + readm(ttyport);
//  }
  chomp(x);
  x=strsplt(x, "\n");
  x = x[13:(x.n-1)];
  chomp(x);
  resize(x, x.n, 1);
  x = strtod(x,<<csp="'BLANK";lstrip="'BLANK">>);
  return x;
};

pwm_genteceo_solo2_class = classdef(ttyport)
{
  static(TTYPORT);
  if (strlen(ttyport)>0)
  {
    TTYPORT = ttyport;
  }
  else
  {
    attr=<<>>;
    attr.devname = "tty";
    attr.id_vendor_id = "0403";
    attr.id_serial_short = "A80041vD";  // usb cable
    CHIP_ADD = usbdevname(attr);
    if (isempty(CHIP_ADD))
    {
      printf("Horrible internal error: Cannot find the contoller!\n");
      EOF
    }
    TTYPORT_OPTS = <<>>;
    TTYPORT_OPTS.data_format  = "8N1";
    TTYPORT_OPTS.speed    = 115200;
    //TTYPORT_OPTS.speed    = 38400;
    TTYPORT_OPTS.debug    = 0;
    TTYPORT = "serial://" + CHIP_ADD;
  }
  if (!open(TTYPORT,TTYPORT_OPTS))
  {
    sleep (3);
  }
  printf("Gentec-EO Laser Power/Energy Meter at %s is online!\n", TTYPORT);

  public(debug);
  debug = function ( val )
  {
    if (exist(val))
    {
      _LIB_DEBUG = (val > 0);
    }
    return _LIB_DEBUG;
  };
  public(readm,writem,accel);
  readm = function (eol)
  {
    return _readstring(TTYPORT, eol);
  };
  writem = function (_cmd)
  {
    return writem(TTYPORT, _cmd);
  };
  accel = function(f)
  {
    if (isnumber(f)<1)
    {
      return 1;
    }
    if (f)
    {
      writem(TTYPORT, "*EAA 1\n");
    }
    if (!f)
    {
      writem(TTYPORT, "*EAA 0\n");
    }
    _readstring(TTYPORT);
    return 0;
  };
  public(set,ver,val,stat,log,sample,meas,status);
  status = function()
  {
    _cmd = num2str("*STA\n");
    writem(TTYPORT, _cmd);
    x = _readstring(TTYPORT);
    chomp(x);
    x = strsplt(x,"\t");
    chomp(x);
    if (length(x)==33)
    {
      rval = <<>>;
      for (i in range(x))
      {
        if (strlen(x[i])>0)
        {
          a = strsplt(x[i],":");
          a = strip(a, "'BLANK", "'BLANK");
          if (length(a)==2)
          {
            a[1] = gsub("_", " ", a[1]).string;
            rval.[a[1]] = a[2];
          }
        }
      }
      return rval;
    }
    printf("Warning: Incomplete return. Inspect the result and try again!\n");
    return x;
  };
  sample = function(f)
  {
    if (class(f)=="list")
    {
      if (!exist(f.rate) || !exist(f.rate_unit))
      {
        return 1;
      }
      if (!exist(f.period) || !exist(f.period_unit))
      {
        return 1;
      }
      if (!exist(f.duration) || !exist(f.duration_unit))
      {
        return 1;
      }

      sprintf(_cmd, "*DSU %.0f %.0f %.0f %.0f %.0f %.0f 1\n", ...
          f.rate,   _sampling_timeunit_to_index.[ substr(tolower(f.rate_unit  ),1:3) ], ...
              f.period, _sampling_timeunit_to_index.[ substr(tolower(f.period_unit),1:3) ], ...
                  f.duration, _sampling_timeunit_to_duration.[ substr(tolower(f.duration_unit),1:3) ] );
      writem(TTYPORT, _cmd);
    }
    _cmd = num2str("*RDS\n");
    writem(TTYPORT, _cmd);
    x = _readstring(TTYPORT);
    x = strsplt(x, "\t");
    rval = <<>>;
    // rate and rate_unit
    a = strsplt(x[1],":")[2];
    b = strsplt(a, "/");
    b = strip(b,"'BLANK","'BLANK");
    rval.rate_unit = b[2];
    rval.rate = strtod(b[1]);
    // period and period_unit
    a = strsplt(x[2],":")[2];
    b = strsplt(a, "/");
    b = strip(b,"'BLANK","'BLANK");
    rval.period_unit = b[2];
    rval.period = strtod(b[1]);
    // duration
    a = strsplt(x[3],":")[2];
    a = strip(a,"'BLANK","'BLANK");
    b = strsplt(a, " ");
    rval.duration_unit = b[2];
    rval.duration = strtod(b[1]);
    return rval;
  };
  set = <<>>;
  set.scale = function (f)
  {
    if (strlen(f)<1)
    {
      return 1;
    }
    UNIT = "";
    if (strindex(f,"p")>0)
    {
      UNIT = "p";
    }
    if (strindex(f,"n")>0)
    {
      UNIT = "n";
    }
    if (strindex(f,"u")>0)
    {
      UNIT = "u";
    }
    if (strindex(f,"m")>0)
    {
      UNIT = "m";
    }
    if (strindex(f,"k")>0)
    {
      UNIT = "k";
    }
    if (strindex(f,"meg")>0)
    {
      UNIT = "meg";
    }
    VAL = strtod(f);
    if ((VAL != 1) && (VAL != 3) && (VAL != 10) && (VAL != 30) && (VAL != 100) && (VAL != 300))
    {
      return 2;
    }
    _cmd = num2str(VAL, "*SSA %.0f" + UNIT + "\n");
    _cmd ?
    writem(TTYPORT, _cmd);
    return 0;
  };
  set.wlen = function (f)
  {
    if (isnumber(f)<1)
    {
      return 1;
    }
    _cmd = num2str(f, "*SWA %f\n");
    writem(TTYPORT, _cmd);
    _readstring(TTYPORT);
    return 0;
  };
  set.zero = function (f)
  {
    if (isnumber(f)<1)
    {
      return 1;
    }
    _cmd = num2str((f==1), "*EOA %.0f\n");
    writem(TTYPORT, _cmd);
    _readstring(TTYPORT);
    return 0;
  };
  set.clock = function ()
  {
    t = clock();
    sprintf(_cmd, "*CLK %.0f %.0f %.0f %.0f %.0f %.0f %.0f\n", ...
        t[3], t[2], t[1], t[4], t[5], t[6], (t[4]>=12) );
    writem(TTYPORT, _cmd);
    _readstring(TTYPORT);
    return 0;
  };
  set.meas = <<>>;
  set.meas.power = function ()
  {
    _cmd = num2str(f, "*SCA 0\n");
    writem(TTYPORT, _cmd);
    _readstring(TTYPORT);
    return 0;
  };
  set.meas.energy = function ()
  {
    _cmd = num2str(f, "*SCA 1\n");
    writem(TTYPORT, _cmd);
    _readstring(TTYPORT);
    return 0;
  };
  set.attn = function (f)
  {
    if (isnumber(f)<1)
    {
      return 1;
    }
    _cmd = num2str(f, "*ATU %.0f\n");
    writem(TTYPORT, _cmd);
    _readstring(TTYPORT);
    return 0;
  };
  set.mult1 = function (f)
  {
    if (isnumber(f)<1)
    {
      return 1;
    }
    _cmd = num2str(f, "*SMU 1,%f\n");
    writem(TTYPORT, _cmd);
    _readstring(TTYPORT);
    return 0;
  };
  set.offs1 = function (f)
  {
    if (isnumber(f)<1)
    {
      return 1;
    }
    _cmd = num2str(f, "*SOU 1,%f\n");
    writem(TTYPORT, _cmd);
    _readstring(TTYPORT);
    return 0;
  };
  set.mult2 = function (f)
  {
    if (isnumber(f)<1)
    {
      return 1;
    }
    _cmd = num2str(f, "*SMU 2,%f\n");
    writem(TTYPORT, _cmd);
    _readstring(TTYPORT);
    return 0;
  };
  set.offs2 = function (f)
  {
    if (isnumber(f)<1)
    {
      return 1;
    }
    _cmd = num2str(f, "*SOU 2,%f\n");
    writem(TTYPORT, _cmd);
    _readstring(TTYPORT);
    return 0;
  };
  set.display = <<>>;
  set.display.rt = function ()
  {
    writem(TTYPORT, "*SDU 0\n");
    _readstring(TTYPORT);
    return 0;
  };
  set.display.hist = function ()
  {
    writem(TTYPORT, "*SDU 1\n");
    _readstring(TTYPORT);
    return 0;
  };
  set.display.stat = function ()
  {
    writem(TTYPORT, "*SDU 2\n");
    _readstring(TTYPORT);
    return 0;
  };
  set.display.dig = function ()
  {
    writem(TTYPORT, "*SDU 3\n");
    _readstring(TTYPORT);
    return 0;
  };
  set.display.hires = function (f)
  {
    if (isnumber(f)<1)
    {
      return 1;
    }
    if (f)
    {
      writem(TTYPORT, "*SHL 1\n");
    }
    if (!f)
    {
      writem(TTYPORT, "*SHL 0\n");
    }
    _readstring(TTYPORT);
    return 0;
  };
  val = <<>>;
  val.ready = function ()
  {
    _cmd = 1201L;
    _bcmd = bytesplit(_cmd,1,"uint16");
    x = _qreadnbytes(TTYPORT, _bcmd);
    if (strlen(x)>0)
    {
      printf("Error: %s", x);
      return [];
    }
    x = bytejoin(x,1,"uint32");
    return x;
  };
  val.now = function ()
  {
    _cmd = 1200L;
    _bcmd = bytesplit(_cmd,1,"uint16");
    x = _qreadnbytes(TTYPORT, _bcmd);
    if (strlen(x)>0)
    {
      printf("Error: %s", x);
      return [];
    }
    x = bytejoin(x,1,"uint32");
    rval = x[1] * 10^(-13+x[2]);
    return rval;
  };
  val.ascii = function ()
  {
    writem(TTYPORT, "*CVU\n");
    x = _readstring(TTYPORT);
    return strtod(x,<<note="Current Value:";lstrip="'BLANK">>);
  };
  stat = <<>>;
  stat.val = function ()
  {
    writem(TTYPORT, "*VSU\n");
    x = _readstring(TTYPORT);
    x = strsplt(x, "\t");
    rval = <<>>;
    for (i in 1:length(x))
    {
      s = strsplt(x[i],":")[2];
      d = strtod(s,<<lstrip="'BLANK">>);
      if (i <= length(_stat_params_names))
      {
        rval.[ _stat_params_names[i] ] = d;
      }
    }
    return rval;
  };
  stat.start = function ()
  {
    writem(TTYPORT, "*ESU 1\n");
    _readstring(TTYPORT);
    return 0;
  };
  stat.stop = function ()
  {
    writem(TTYPORT, "*ESU 0\n");
    _readstring(TTYPORT);
    return 0;
  };
  stat.reset = function ()
  {
    writem(TTYPORT, "*ESU 2\n");
    _readstring(TTYPORT);
    return 0;
  };
  log = <<>>;
  log.stop = function ()
  {
    writem(TTYPORT, "*LOG 0\n");
    _readstring(TTYPORT);
    return 0;
  };
  log.start = <<>>;
  log.start.data = function ()
  {
    writem(TTYPORT, "*LOG 1\n");
    _readstring(TTYPORT);
    return 0;
  };
  log.start.stat = function ()
  {
    writem(TTYPORT, "*LOG 2\n");
    _readstring(TTYPORT);
    return 0;
  };
  log.start.both = function ()
  {
    writem(TTYPORT, "*LOG 3\n");
    _readstring(TTYPORT);
    return 0;
  };
  log.download = function (verbose)
  {
    _cmd = 1172L;
    _par = 0;
    _bcmd = bytesplit(_cmd,1,"uint16");
    rval = _qreadnbytes_of_data(TTYPORT, _bcmd);
    return rval;
  };
  log.download_ascii = function (verbose)
  {
    if (isnumber(verbose)<1)
    {
      verbose = 0;
    }
    x = "";
    tic(16);
    dt = 0;
    writem(TTYPORT, "*FDL\n");
    if (verbose)
    {
      j=0;
    }
    have_eof = 0;
    while (1)
    {
      _x = readm(TTYPORT, 50000);
      dt = toc(16);
      if (dt > 20)
      {
        printf("Warning: The serial port is not responding for at least 10 sec. Stopping now!\n");
        printf("Warning: Downloaded data may be incomplete!\n");
        break;
      }
      if (_x.n == 0)
      {
        sleep(0.05);
        continue;
      }
      delta_x = char(_x);
      x = x + delta_x;
      if (strindex(x,"Error 24")>0)
      {
        printf("%s\n", x);
        return <<>>;
      }
      if (strindex(x,"EOF"))
      {
        have_eof=1;
        break;
      }
      if (verbose)
      {
        j++;
        "download: "?
        [j, strlen(x), strlen(delta_x)]?
      }
      tic(16);
    }
    rval = <<>>;
    x=strsplt(x, "\r\n");
    rval.header = x[1:10];
    n = x.n;
    if (have_eof)
    {
      n--;
    }
    x = x[13:n];
    chomp(x);
    resize(x, x.n, 1);
    rval.val_raw = x;
    x = strtod(x,<<csp="'BLANK";lstrip="'BLANK">>);
    rval.val = x;
    return rval;
  };
  ver = function ()
  {
    _cmd = "*VER\n";
    writem(TTYPORT, _cmd);
    return readm(TTYPORT);
  };
};


















//
// liblab_pid_omg_cni.r3:
//
static(INIT);

if (exist(INIT))
{
  EOF
}

static(_THIS_LIB, LIB_DEBUG);
_THIS_LIB = "liblab_pid_omg_cni";
LIB_DEBUG = 0;

static(REC_CHAR, READ_CMD, GET_CMD, WRITE_CMD, PUT_CMD, ECHO, ENABLE_CMD, DISABLE_CMD);
REC_CHAR = "*";
READ_CMD = "R";
GET_CMD = "G";
ENABLE_CMD = "E";
DISABLE_CMD = "D";
WRITE_CMD = "W";
PUT_CMD = "P";
ECHO = 1;

static(FORMAT,FORMAT2);
FORMAT ="%04X";
FORMAT2="%02X";

static(URL_OPTS);

URL_OPTS = <<>>;
URL_OPTS.data_parity_stop = "8N1";
URL_OPTS.speed = 9600;
URL_OPTS.debug = 0;
URL_OPTS.eol = "\r";

//
// INPUT
//
static(supported_tc, tc_id);
supported_tc = ["j", "k", "t", "e", "n", "din-j", "r", "s", "b", "c"];
tc_id        = [ 0 ,  1 ,  2 ,  3,   4,   5,       6,   7,   8,   9 ];

static(convert_from_iseries);
convert_from_iseries = function (val)
{
  n = strlen(val) / 2;
  if (n == 3)
  {
    b = bitsplit(strtod("0x" + val),n,0) + 0.0;
    s = 1 - 2 * b[1];
    d = polyval(2,b[2:4]);
    if (d==0)
    { error("Response '000' is not allowed\n"); }
    df = 10 .^ (1 - d);
    v = polyval(2+0.0,b[5:n*8]+0.0);
    v = s * v * df;
    return v;
  }

  if ((n==2) || (n==1))
  {
    v = strtod("0x" + val);
    return v;
  }
};

static(convert_to_iseries);
convert_to_iseries = function (val, d)
{
  v = abs(val) * (10 .^ d);
  b = bitsplit(v,3,0) + 0.0;
  b[1] = ( val<0 );
  b[2:4] = bitsplit(d+1,1)[6:8];
  rval = num2str(int(polyval(2,b)),"%06X");
  return rval;
};

static(read_pid);
read_pid = function ( url, cmd, isget )
{
  if (!exist(isget))
  { isget=0; }

  // ask instrument
  if (LIB_DEBUG)
  {
    printf("read_pid (write): %s\n", REC_CHAR + READ_CMD + cmd);
  }

  switch (isget)
  {
    case 1:
      writem(url, REC_CHAR + GET_CMD + cmd);
      break;

    default:
      writem(url, REC_CHAR + READ_CMD + cmd);
      break;
  }

  // get response
  rval = "";
  do
  {
    rval = rval + readm(url);
  }
  while (strlen(rval)<=1 && isnumber(rval))

  if (LIB_DEBUG)
  {
    printf("read_pid (read) : %s\n", rval);
  }
  if (strlen(rval) > 0)
  {
    rval = rstrip(rval, ["\n", "\r"]);
    i = strindex(rval, cmd);
    if (i>0)
    { rval = substr(rval, (strlen(cmd) + i):strlen(rval)); }
  }

  //
  return rval;
};

static(write_2_pid);
write_2_pid = function ( url, cmd, ival )
{
  // ask instrument
  if (LIB_DEBUG)
  {
    printf("write_2_pid: %s\n", REC_CHAR + WRITE_CMD + cmd + ival);
  }
  writem(url, REC_CHAR + WRITE_CMD + cmd + ival);

  // get response
  if (ECHO)
  {
    rval = readm(url);
    if (strlen(rval) > 0)
    {
      rval = rstrip(rval, ["\n", "\r"]);
      if (strindex(rval, cmd)>0)
      { return 0; }
      return 1;
    }
  }
  return 0;
};


omega_iseries = classdef(dev)
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

  public(restart, sp1, device, out1, inp, rdgoff, anloff, offset_adj, val, alarm1, out1, deg, dec);
  device  = function()
  {
    return _device;
  };
  restart = function()
  {
    _cmd = "Z02";
    x = writem(_device, REC_CHAR + _cmd);
    return 0;
  };
  sp1 = function (val, c)
  {
    _cmd = "01";
    if (!exist(val))
    {
      rval = read_pid(_device, _cmd, 1);
      return convert_from_iseries(rval);
    }
    // sending nan() disables set point 1
    if (isnan(val))
    {
      writem(_device, REC_CHAR + DISABLE_CMD + _cmd);
      rval = 0;
      if (ECHO)
      {
        x = readm(_device);
        if (strindex(x,_cmd) < 1)
        { rval = 1; }
      }
      return rval;
    }

    if (class(c)!="num")
    { c = 0;}

    // process regular value
    rval = convert_to_iseries(val,1);

    _write_cmd = PUT_CMD;
    if (c)
    { _write_cmd = WRITE_CMD; }
    writem(_device, REC_CHAR + _write_cmd + _cmd + rval);

    //writem(_device, REC_CHAR + PUT_CMD + _cmd + rval);
    // writem(_device, REC_CHAR + WRITE_CMD + _cmd + rval);
    rval = val;
    if (ECHO)
    {
      x = readm(_device);
      if (strindex(x,_cmd) < 1)
      { rval = nan(); }
    }
    return rval;
  };
  if (!exist(out1))
  { out1 = <<>>; }
  out1.autotune = function (val)
  {
    _n = 6;
    _cmd = "0C"; // OUT1CNF
    // get the existing data
    rval = read_pid(_device,_cmd);
    v = strtod("0x" + rval);
    b = bitsplit(v) + 0.0;

    if (exist(val))
    {
      b[_n] = 0;
      if (val>0)
      { b[_n] = 1; }
      v = polyval(2,b);
      s = num2str(int(v),"%02X");
      write_2_pid(_device, _cmd, s);
    }

    return b[_n];
  };
  out1.autopid = function( val )
  {
    _n = 3;
    _cmd = "0C"; // OUT1CNF
    // get the existing data
    rval = read_pid(_device, _cmd);
    v = strtod("0x" + rval);
    b = bitsplit(v) + 0.0;

    if (exist(val))
    {
      b[_n] = 0;
      if (val>0)
      { b[_n] = 1; }
      v = polyval(2,b);
      s = num2str(int(v),"%02X");
      write_2_pid(_device, _cmd, s);
    }

    return b[_n];
  };
  out1.antiwind = function( val )
  {
    _n = 5;
    _cmd = "0C"; // OUT1CNF
    // get the existing data
    rval = read_pid(_device,_cmd);
    v = strtod("0x" + rval);
    b = bitsplit(v) + 0.0;
    if (exist(val))
    {
      b[_n] = 0;
      if (val>0)
      { b[_n] = 1; }
      v = polyval(2,b);
      s = num2str(int(v),"%02X");
      write_2_pid(_device,_cmd, s);
    }

    return b[_n];
  };
  out1.analog_prop = function( val )
  {
    _n = 7;
    _cmd = "0C"; // OUT1CNF
    // get the existing data
    rval = read_pid(_device,_cmd);
    v = strtod("0x" + rval);
    b = bitsplit(v) + 0.0;
    if (exist(val))
    {
      b[_n] = 0;
      if (val>0)
      { b[_n] = 1; }
      v = polyval(2,b);
      s = num2str(int(v),"%02X");
      write_2_pid(_device,_cmd, s);
    }
    return b[_n];
  };
  out1.pid = function( val )
  {
    _n = 1;
    _cmd = "0C"; // OUT1CNF

    // get the existing data
    rval = read_pid(_device,_cmd);
    v = strtod("0x" + rval);
    b = bitsplit(v) + 0.0;

    if (exist(val))
    {
      b[_n] = 0;
      if (val>0)
      { b[_n] = 1; }
      v = polyval(2,b);
      s = num2str(int(v),"%02X");
      write_2_pid(_device,_cmd, s);
    }

    return b[_n];
  };
  out1.reverse_direct = function( val )
  {
    _n = 2;
    _cmd = "0C"; // OUT1CNF

    // get the existing data
    rval = read_pid(_device,_cmd);
    v = strtod("0x" + rval);
    b = bitsplit(v) + 0.0;

    if (exist(val))
    {
      b[_n] = 0;
      if (val>0)
      { b[_n] = 1; }
      v = polyval(2,b);
      s = num2str(int(v),"%02X");
      write_2_pid(_device,_cmd, s);
    }

    return b[_n];
  };
  out1.band = function (val)
  {
    _cmd = "17";
    if (!exist(val))
    {
      rval = read_pid(_device,_cmd);
      return convert_from_iseries(rval);
    }
    // process regular value
    if ((val > 100) || (val<0))
    { return -1; }
    rval = num2str(int(val), FORMAT);
    writem(_device,REC_CHAR + WRITE_CMD + _cmd + rval);
    rval = val;
    if (ECHO)
    {
      x = readm(_device);
      if (strindex(x,_cmd) < 1)
      { rval = nan(); }
    }
    return rval;
  };
  out1.cycle = function (val)
  {
    _cmd = "1A";
    if (!exist(val))
    {
      rval = read_pid(_device,_cmd);
      return convert_from_iseries(rval);
    }
    // process regular value
    if ((val > 100) || (val<0))
    { return -1; }
    rval = num2str(int(val), FORMAT2);
    writem(_device, REC_CHAR + WRITE_CMD + _cmd + rval);
    rval = val;
    if (ECHO)
    {
      x = readm(_device);
      if (strindex(x,_cmd) < 1)
      { rval = nan(); }
    }
    return rval;
  };
  out1.rate = function (val)
  {
    _cmd = "19";
    if (!exist(val))
    {
      rval = read_pid(_device,_cmd);
      return convert_from_iseries(rval);
    }
    // process regular value
    if ((val > 3999) || (val<0))
    { return -1; }
    rval = num2str(int(val), FORMAT);
    writem(_device, REC_CHAR + WRITE_CMD + _cmd + rval);
    rval = val;
    if (ECHO)
    {
      x = readm(_device);
      if (strindex(x,_cmd) < 1)
      { rval = nan(); }
    }
    return rval;
  };

  if (!exist(inp))
  { inp = <<>>; }
  inp.tc = function( val )
  {
    _cmd = "07"; // INPUT
    v = read_pid(_device,_cmd);
    b = bitsplit( strtod("0x" + v ) ) + 0.0;

    if (!exist(val))
    {
      // check what tc is used
      if (any(b[7:8]!=0))
      { return blank(); }
      idx = find (polyval(2,b[3:6]) == tc_id);
      if (isempty(idx))
      { return blank(); }
      return supported_tc[idx];
    }

    // find what tc to use
    idx = find (supported_tc == val);
    if (isempty(idx))
    { return blank(); }
    b[3:6] = bitsplit(tc_id[idx])[5:8];
    // get the existing data
    v = polyval(2,b+0.0);
    s = num2str(int(v),"%02X");
    write_2_pid(_device,_cmd, s);
    return val;
  };
  rdgoff = function (val)
  {
    _cmd = "03";
    if (!exist(val))
    {
      rval = read_pid(_device, _cmd);
      return convert_from_iseries(rval);
    }

    // sending nan() disables set point 1
    if (isnan(val))
    {
      writem(_device, REC_CHAR + DISABLE_CMD + _cmd);
      rval = 0;
      if (ECHO)
      {
        x = readm(URL);
        x
            if (strindex(x,_cmd) < 1)
        { rval = 1; }
      }
      return rval;
    }

    // process regular value
    rval = convert_to_iseries(val,1);
    writem(_device, REC_CHAR + WRITE_CMD + _cmd + rval);
    rval = val;
    if (ECHO)
    {
      x = readm(_device);
      if (strindex(x,_cmd) < 1)
      { rval = nan(); }
    }
    return rval;
  };
  anloff = function (val)
  {
    _cmd = "03";
    if (!exist(val))
    {
      rval = read_pid(_device,_cmd);
      return convert_from_iseries(rval);
    }
    // sending nan() disables set point 1
    if (isnan(val))
    {
      writem(_device, REC_CHAR + DISABLE_CMD + _cmd);
      rval = 0;
      if (ECHO)
      {
        x = readm(_device);
        if (strindex(x,_cmd) < 1)
        { rval = 1; }
      }
      return rval;
    }
    // process regular value
    rval = convert_to_iseries(val,1);
    writem(_device, REC_CHAR + WRITE_CMD + _cmd + rval);
    rval = val;
    if (ECHO)
    {
      x = readm(_device);
      if (strindex(x,_cmd) < 1)
      { rval = nan(); }
    }
    return rval;
  };
  offset_adj = function (val)
  {
    _cmd = "25";
    if (!exist(val))
    {
      rval = read_pid(_device,_cmd);
      return convert_from_iseries(rval);
    }
    // sending nan() disables set point 1
    if (isnan(val))
    {
      writem(_device, REC_CHAR + DISABLE_CMD + _cmd);
      rval = 0;
      if (ECHO)
      {
        x = readm(_device);
        if (strindex(x,_cmd) < 1)
        { rval = 1; }
      }
      return rval;
    }
    // process regular value
    rval = convert_to_iseries(val,1);
    writem(_device, REC_CHAR + WRITE_CMD + _cmd + rval);
    rval = val;
    if (ECHO)
    {
      x = readm(_device);
      if (strindex(x,_cmd) < 1)
      { rval = nan(); }
    }
    return rval;
  };
  val = function ()
  {
    _cmd = "X01";
    writem(_device, REC_CHAR + _cmd);
    rval = "";
    j=0;
    do
    {
      rval = rval + readm(_device);
      if (strindex(rval,"01")==1)
      { rval = "X" + rval; }
      if ((j++)>5)
      {
        stop("val: Device stopped responding!");
      }
    }
    while ( (strindex(rval, ".")==0) || (strlen(rval)<strindex(rval, ".")) );

    i = strindex(rval,_cmd);
    if (i >0)
    {
      rval = substr(rval, (i+3):strlen(rval));
    }
    rval = strtod(rval);
    return rval;
  };
  alarm1 = function (val)
  {
    if (!exist(val))
    { return []; }

    if (val>0)
    {
      writem(_device, REC_CHAR + "E01");
      return 1;
    }
    writem(_device,REC_CHAR + "D01");
    return 0;
  };
  deg = function( val )
  {
    _cmd = "08"; // INPUT
    v = read_pid(_device,_cmd);
    b = bitsplit( strtod("0x" + v ) ) + 0.0;
    if (!exist(val))
    {
      // check what tc is used
      if (any(b[4]==0))
      { return "c"; }
      return "f";
    }
    // find what tc to use
    b[4] = 0;
    if(strindex(tolower(val),"f"))
    { b[4] = 1; }
    // get the existing data
    v = polyval(2,b+0.0);
    s = num2str(int(v),"%02X");
    write_2_pid(_device, _cmd, s);
    return val;
  };
  dec = function( val )
  {
    _cmd = "08"; // INPUT
    v = read_pid(_device,_cmd);
    b = bitsplit( strtod("0x" + v ) ) + 0.0;
    if (!exist(val))
    {
      v = polyval(2,b[6:8]);
      return v;
    }
    // find what tc to use
    if((val<1) || (val>4))
    { return -1; }
    switch (val)
    {
      case 1:
        b[6:8] = [0, 0, 1];
        break;
      case 2:
        b[6:8] = [0, 1, 0];
        break;
      case 3:
        b[6:8] = [0, 1, 1];
        break;
      default:
        b[6:8] = [1, 0, 0];
        break;
    }
    // get the existing data
    v = polyval(2,b+0.0);
    s = num2str(int(v),"%02X");
    write_2_pid(_device,_cmd, s);
    return val;
  };



};



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
URL_OPTS.speed = 115200;
URL_OPTS.data_parity_stop = "8N2";
//URL_OPTS.flow_control = "x";
URL_OPTS.debug = 0;
//URL_OPTS.eol = "\r";

standa_laser_shutter_class = classdef(dev)
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
  public(device, geti, home, left, rigt);
  device  = function()
  {
    return _device;
  };
  geti  = function()
  {
    _cmd = [0x67, 0x65, 0x74, 0x69];
    writem(_device, _cmd);
    nresp=36;
    t = readm(_device,nresp);
    while (t.n < nresp)
    {
      t.n?
      t = [t, readm(_device,nresp-t.n)];
    }
    rval = <<>>;
    rval.mfg      = t[5,6];
    rval.product  = t[7];
    rval.hw_ver   = t[8,9];
    rval.release  = bytejoin(t[10,11],0,"uint16_t");
    return rval;
  };
  home = function()
  {
    _cmd = [0x68, 0x6f, 0x6d, 0x65];
    writem(_device, _cmd);
    nresp=4;
    t = readm(_device,nresp);
    while (t.n < nresp)
    {
      t = [t, readm(_device,nresp-t.n)];
    }
    return 0;
  };
  left = function()
  {
    _cmd = [0x6c, 0x65, 0x66, 0x74];
    writem(_device, _cmd);
    nresp=4;
    t = readm(_device,nresp);
    while (t.n < nresp)
    {
      t = [t, readm(_device,nresp-t.n)];
    }
    return 0;
  };
  rigt = function()
  {
    _cmd = [0x72, 0x69, 0x67, 0x74];
    writem(_device, _cmd);
    nresp=4;
    t = readm(_device,nresp);
    while (t.n < nresp)
    {
      t = [t, readm(_device,nresp-t.n)];
    }
    return 0;
  };
};



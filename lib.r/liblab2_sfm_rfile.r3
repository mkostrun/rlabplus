//
// liblab2_sfm_rfile.r3:
//
static(INIT);

if (exist(INIT,WHICH,FORTUNE))
{
  EOF
}

// check for 'fortune'
if (!exist(WHICH))
{
  WHICH = reads("|which which");
}
FORTUNE = reads("|" + WHICH + " fortune 2>/dev/null");
if (isempty(FORTUNE))
{
  clear(FORTUNE);
}
if (strlen(FORTUNE)<strlen("fortune"))
{
  clear(FORTUNE);
}

static(_THIS_LIB,LOGFN,LOGON);
if (!exist(LOGON))
{
  LOGON = 0;
}
_THIS_LIB = "liblab2_sfm_rfile";

static(URL_OPTS);
URL_OPTS = <<>>;
//URL_OPTS.speed = 230400;
URL_OPTS.speed = 921600; // since 2012-07-29, CDAEM SFM 1.30.1.M1, .M2, .M3
//URL_OPTS.speed = 460800; //XM250
URL_OPTS.data_parity_stop = "8N1";

static(SFM_GPIO_PORTA,SFM_GPIO_PORTB,SFM_GPIO_PORTC);
SFM_GPIO_PORTA = 0x00000000;
SFM_GPIO_PORTB = 0x00010000;
SFM_GPIO_PORTC = 0x00020000;
static(SFM_GPIO_PIN00,SFM_GPIO_PIN01,SFM_GPIO_PIN02,SFM_GPIO_PIN03,SFM_GPIO_PIN04);
SFM_GPIO_PIN00 = 0x00000001;
SFM_GPIO_PIN01 = 0x00000002;
SFM_GPIO_PIN02 = 0x00000004;
SFM_GPIO_PIN03 = 0x00000008;
SFM_GPIO_PIN04 = 0x00000010;

//
//
// FPGA: WRITE REGISTERS
//
//
static(SFM_FPGA_ADDR_FPGA_CONTROL);
SFM_FPGA_ADDR_FPGA_CONTROL    = 0x4c00018a;
static(SFM_FPGA_ADDR_TDC_CONTROL, SFM_FPGA_ADDR_TDC_COMMAND, SFM_FPGA_ADDR_TPG_START_DELAY, SFM_FPGA_ADDR_COMP_LE_DELAY);
SFM_FPGA_ADDR_COMP_LE_DELAY   = 0x4c00018c;
SFM_FPGA_ADDR_TDC_CONTROL     = 0x4c00018e;
SFM_FPGA_ADDR_TDC_COMMAND     = 0x4c000190;
SFM_FPGA_ADDR_TPG_START_DELAY = 0x4c000192;

//
//
// FPGA: READ REGISTERS
//
//
static(SFM_FPGA_ADDR_FPGA_REVISION);
SFM_FPGA_ADDR_FPGA_REVISION    = 0x40000194;



static(RECORD_ID_ACCEL0,RECORD_ID_ACCEL1,RECORD_ID_ACCEL2,RECORD_ID_ACCEL3,...
    RECORD_ID_SENSOR_DATA,RECORD_ID_SYS_DATA,RECORD_ID_EMMC_MISSION_COMMENT);
RECORD_ID_ACCEL0      = 0x01;
RECORD_ID_ACCEL1      = 0x02;
RECORD_ID_ACCEL2      = 0x03;
RECORD_ID_ACCEL3      = 0x04;
RECORD_ID_SYS_DATA    = 0x05;
RECORD_ID_SENSOR_DATA = 0x06;
RECORD_ID_EMMC_MISSION_COMMENT = 0xBB;

static(SFM_TACTICAL_MISSION_COMMENT_DEFAULT);
SFM_TACTICAL_MISSION_COMMENT_DEFAULT = "This is rlab3, and today is " + time2dstr(seconds());

static(_read_from_fpga,_write_to_fpga,_sfm_gpio_set,_collect_sfm_data,_sfm_command_resp,_sfm_command,_process_sfm_block);
_process_sfm_block = function(blk)
{
  if (mod(length(blk),512))
  {
    printf("The block size %f is not supported: Block size has to be multiple of 512!\n", length(blk));
    return <<>>;
  }
  //rec  = blk;
  //resize(rec,128,round(blk.n/128));
  rec = reshape(blk,128,round(blk.n/128.0));
  rec = rec';
  //
  // count the sensor, system and acceleration data records
  //
  ncomment = 0;
  nrec     = 0;
  nrec_sys = 0;
  nrec_acc = 0;
  for (i in 1:rec.nr)
  {
    if (rec[i;1] == RECORD_ID_EMMC_MISSION_COMMENT)
    {
      ncomment++;
      continue;
    }
    if (rec[i;1] == RECORD_ID_SENSOR_DATA)
    {
      nrec++;
      continue;
    }
    if (rec[i;1] == RECORD_ID_SYS_DATA)
    {
      nrec_sys++;
      continue;
    }
    if ((rec[i;1] == RECORD_ID_ACCEL0)||(rec[i;1] == RECORD_ID_ACCEL1)||(rec[i;1] == RECORD_ID_ACCEL2)...
         ||(rec[i;1] == RECORD_ID_ACCEL3))
    {
      nrec_acc++;
      continue;
    }
  }
  // comment with data file
  if (ncomment > 0)
  {
    icomment = int(zeros(1,508));
  }
  // sensor data record
  if (nrec>0)
  {
    m_time_s = zeros(nrec,1);
    pri_us    = zeros(nrec,1);
    fpga_ctrl_reg = int(zeros(nrec,1));
    pir = zeros(nrec,12);
    ac_thresh = zeros(nrec,1);
    tdc_ch_tof = int(zeros(nrec,6));
    tdc_ch_cal1 = int(zeros(nrec,6));
    tdc_ch_cal2 = int(zeros(nrec,6));
    status_reg = int(zeros(nrec,1));
    range_m = zeros(nrec,3);
    pulse_width = zeros(nrec,3);
  }
  // system data record
  if (nrec_sys > 0)
  {
    m_time_s_sys = zeros(2*nrec_sys,1);
    sample_num_sys = int(zeros(2*nrec_sys,1));
    voltages = int(zeros(2*nrec_sys,14));
    mission_flags = int(zeros(2*nrec_sys,1));
    gpio_data = int(zeros(2*nrec_sys,6));
  }
  if (nrec_acc > 0)
  {
    m_time_s_acc = zeros(8*nrec_acc,1);
    sample_num_acc = int(zeros(8*nrec_acc,1));
    acc_idx = int(zeros(8*nrec_acc,1));
    xyz_data = int(zeros(8*nrec_acc,3));
  }
  // accel data record
  j = 0;
  j_com = 0;
  j_acc = 0;
  j_sys = 0;
  for (i in 1:rec.nr)
  {
    if (rec[i;1] == RECORD_ID_SENSOR_DATA)
    {
      j++;
      //
      // sensor data record
      //
      m_time_s[j]       = bytejoin(rec[i; 2: 5],0,"uint32") .* 1e-6;
      // registered 6:7
      pri_us[j]         = bytejoin(rec[i; 8: 9],0,"uint16") * 1e-2;
      fpga_ctrl_reg[j]  = bytejoin(rec[i;10:11],0,"uint16");
      // pir: 12 14 16 18 20 22 [] 26 28 30 32 34 36
      pir[j;] = bytejoin(rec[i;12:23,26:37],0,"int16") .* 5 ./ 32768;
      // ac_thresh 24
      //ac_thresh[j]  = bytejoin(rec[i;24,25],0,"int16") .* 5 ./ 32768;
      ac_thresh[j]  = bytejoin(rec[i;24,25],0,"int16") .* 6.25 ./ 32768;
      // tdc_ch_tof: 38 40 42 44 46 48
      tdc_ch_tof[j;] = bytejoin(rec[i;38:49],0,"uint16");
      // tdc_ch_cal1: 50 52 54 56 58 60
      tdc_ch_cal1[j;] = bytejoin(rec[i;50:61],0,"uint16");
      // tdc_ch_cal2: 62 64 66 68 70 72
      tdc_ch_cal2[j;] = bytejoin(rec[i;62:73],0,"uint16");
      // status_reg: 74
      status_reg[j]  = bytejoin(rec[i;74,75],0,"uint16");
      // range_m: 76 80 84
      range_m[j;]  = bytejoin(rec[i;76:87],0,"float");
      // pulse_width: 88 92 96
      pulse_width[j;]  = bytejoin(rec[i;88:99],0,"float");
      continue;
    }
    if (rec[i;1] == RECORD_ID_SYS_DATA)
    {
      j_sys++;
      for (j2 in 0:1)
      {
        _dof = 1 + (j2*64);
        m_time_s_sys[j_sys + j2]  = bytejoin(rec[i; 1+_dof:  4+_dof],0,"uint32") .* 1e-6;
        sample_num_sys[j_sys +j2] = bytejoin(rec[i; 5+_dof:  6+_dof],0,"uint16");
        voltages[j_sys +j2;]      = bytejoin(rec[i; 7+_dof: 34+_dof],0,"uint16");
        mission_flags[j_sys +j2]  = bytejoin(rec[i;35+_dof: 36+_dof],0,"uint16");
        gpio_data[j_sys + j2;]    = bytejoin(rec[i;37+_dof: 48+_dof],0,"uint16");
      }
      continue;
    }
    if ((rec[i;1] == RECORD_ID_ACCEL0)||(rec[i;1] == RECORD_ID_ACCEL1)||(rec[i;1] == RECORD_ID_ACCEL2)...
         ||(rec[i;1] == RECORD_ID_ACCEL3))
    {
      j_acc++;
      for (j2 in 0:7)
      {
        _dof = (j2*16);
        acc_idx[j_acc + j2]       =          rec[i; 1+_dof];
        m_time_s_acc[j_acc + j2]  = bytejoin(rec[i; 2+_dof: 5+_dof],0,"uint32") .* 1e-6;
        sample_num_acc[j_acc +j2] = bytejoin(rec[i; 6+_dof: 7+_dof],0,"uint16");
        xyz_data[j_acc+j2;]       = bytejoin(rec[i; 8+_dof: 13+_dof],0,"int16");
      }
      continue;
    }
    if (rec[i;1] == RECORD_ID_EMMC_MISSION_COMMENT)
    {
      j_com++;
      icomment[j_com:(j_com+126)] = rec[i; 2:128];
      j_com += 127;
    }
  }
  //
  rval = <<>>;
  if (nrec>0)
  {
    rval.sensor = <<>>;
    rval.sensor.m_time_s = m_time_s;
    rval.sensor.pri_us = pri_us;
    rval.sensor.fpga_ctrl_reg = fpga_ctrl_reg;
    rval.sensor.pir = pir;
    rval.sensor.ac_thresh = ac_thresh;
    rval.sensor.tdc_ch_tof = tdc_ch_tof;
    rval.sensor.tdc_ch_cal1 = tdc_ch_cal1;
    rval.sensor.tdc_ch_cal2 = tdc_ch_cal2;
    rval.sensor.status_reg = status_reg;
    rval.sensor.range_m = range_m;
    rval.sensor.pulse_width = pulse_width;
  }
  if (nrec_sys > 0)
  {
    rval.sys = <<>>;
    rval.sys.m_time_s =  m_time_s_sys;
    rval.sys.sample_num = sample_num_sys;
    rval.sys.voltages = voltages;
    rval.sys.mission_flags = mission_flags;
    rval.sys.gpio_data = gpio_data;
  }
  if (nrec_acc > 0)
  {
    rval.acc = <<>>;
    rval.acc.m_time_s = m_time_s_acc;
    rval.acc.idx = acc_idx;
    rval.acc.sample_num = sample_num_acc;
    rval.acc.xyz_data = xyz_data;
  }
  if (ncomment > 0)
  {
    rval.comment = char(icomment);
  }
  return rval;
};

_sfm_gpio_set = function(DEVICE, TOSFM, port, pin, val)
{
  _this_function = "_sfm_gpio_set";
  if ((length(port)!=1)||(length(pin)!=1))
  {
    return nan();
  }
  if (isnumber(val)<1)
  {
    return -1;
  }
  addr = int(port) + int(pin);
  datalen = 5L;
  cmd = [0x5A,0x36,TOSFM,bytesplit(datalen,0,"uint16")];
  cmd_chksum = (256L-sum(cmd)) && 0xff;
  data = [int(val), bytesplit(addr, 0, "uint32")];
  data_chksum = (256L-sum(data)) && 0xff;
  writem(DEVICE, [cmd, cmd_chksum, data, data_chksum]);
  if (LOGON)
  {
    writem(LOGFN,[cmd,cmd_chksum, data, data_chksum],<<format="%02x";eol="\n";csp=" ">>);
  }
  return 0;
};

_sfm_command_resp = function(DEVICE, TOSFM, respcmdval)
{
  //
  // waiting for reply
  //
  t = readm(DEVICE,6);
  while (length(t) < 6)
  {
    t = [t,readm(DEVICE,6-t.n)];
  }
  datalen = bytejoin(t[4:5]);
  if (datalen == 0)
  {
    if (t[2] == respcmdval)
    {
      return 0;
    }
    return 1;
  }
  d = readm(DEVICE,datalen+1);
  while (d.n < datalen+1)
  {
    d = [d, readm(DEVICE,datalen+1-d.n)];
  }
  return d[1:datalen];
};

_sfm_command = function(DEVICE, TOSFM, cmdval, data, wfr)
{
  _this_function = "_sfm_command";
  data_chksum = [];
  if (!exist(wfr))
  {
    wfr = 0;
  }
  if (!exist(data))
  {
    data = [];
  }
  datalen = int(length(data));
  cmd = [0x5A, int(cmdval), TOSFM, bytesplit(datalen,0,"uint16")];
  cmd_chksum = (256L-sum(cmd)) && 0xff;
  if (datalen > 0)
  {
    data_chksum = (256L-sum(data)) && 0xff;
  }
  writem(DEVICE, [cmd, cmd_chksum, data, data_chksum]);
  if (LOGON)
  {
    writem(LOGFN,[cmd, cmd_chksum, data, data_chksum],<<format="%02x";eol="\n";csp=" ">>);
  }
  if (!wfr)
  {
    return 0;
  }
  //
  // waiting for reply
  //
  t = readm(DEVICE,6);
  while (length(t) < 6)
  {
    t = [t,readm(DEVICE,6-t.n)];
  }
  datalen = bytejoin(t[4:5]);
  if (datalen == 0)
  {
    if (t[2] == cmdval+1)
    {
      return 0;
    }
    return 1;
  }
  d = readm(DEVICE,datalen+1);
  while (d.n < datalen+1)
  {
    d = [d, readm(DEVICE,datalen+1-d.n)];
  }
  if (LOGON)
  {
    writem(LOGFN,t,<<format="%02x";eol="";csp=" ">>);
  }
  if (t[2] == 0x1d)
  {
    d = char(d[1:(d.n-1)]);
    chomp(d);
    if (LOGON)
    {
      fprintf(LOGFN,d +"\n");
    }
  }
  if (t[2] != 0x1d)
  {
    if (LOGON)
    {
      writem(LOGFN,[t, d],<<format="%02x";eol="\n";csp=" ">>);
    }
    d = d[1:(d.n-1)];
  }
  if (t[2] == 0x2e)
  {
    // live range data
    d = bytejoin(d,0,"float");
  }
  if (t[2] == 0x2f)
  {
    // live passive data
    d = bytejoin(d,0,"uint16");
  }
  return d;
};

_collect_sfm_data = function(DEVICE, TOSFM, wh, pri, dur, nwt)
{
  _this_function = "_collect_sfm_data";
  if (isnumber(wh)<1)
  {
    return nan();
  }
  _have_lrf = 0L;
  _have_pir = 0L;
  if ((wh == 1) || (wh == 3))
  {
    _have_lrf = 1L;
  }
  if ((wh == 2) || (wh == 3))
  {
    _have_pir = 1L;
  }
  if (isnumber(dur)<1)
  {
    return nan();
  }
  if (isnumber(pri)<1)
  {
    return nan();
  }
  if (pri < 1300)
  {
    printf(_this_function+": PRI cannot be less than 1300 -> PRI requested was %g\n", pri);
    stop(_this_function  +": Horrible Internal Error: Cannot continue!\n");
  }
  if (dur < 0)
  {
    printf(_this_function+": duration cannot be less than 0 -> requested duration was %g\n", dur);
    stop(_this_function  +": Horrible Internal Error: Cannot continue!\n");
  }
  if (isnumber(nwt)<1)
  {
    nwt = 0;
  }
  //
  // request data streaming session
  //
  datalen = 8L;
  cmd = [0x5A,0x2c,TOSFM,bytesplit(datalen,0,"uint16")];
  cmd_chksum = (256L-sum(cmd)) && 0xff;
  data = [_have_lrf,_have_pir,bytesplit(int(1e8/pri),0,"uint16"),bytesplit(dur,0,"float")];
  data_chksum = (256L-sum(data)) && 0xff;
  if (LOGON)
  {
   writem(LOGFN,[cmd,cmd_chksum, data, data_chksum],<<format="%02x";eol="\n";csp=" ">>);
  }
  writem(DEVICE, [cmd, cmd_chksum, data, data_chksum]);
  //
  // about response
  //
  rval_r = [];
  rval_p = [];
  rval = <<>>;
  // do we wait for response?
  if (nwt)
  {
    return rval;
  }
  //
  // get response
  //
  tic(24);
  printf("Streaming data: ");
  while (toc(24) < dur)
  {
    spinner();
    t = readm(DEVICE,6);
    if ((isempty(t)) && (toc(24)>dur))
    {
      break;
    }
    while ( (length(t)<6) && (toc(24) < dur) )
    {
      t = [t,readm(DEVICE,6-t.n)];
    }
    if (isempty(t))
    {
      printf(_this_function + ": Warning: No data received: Check installation!\n");
      break;
    }
    datalen = bytejoin(t[4:5]);
    d = readm(DEVICE,datalen+1);
    while (d.n < datalen+1)
    {
      d = [d, readm(DEVICE,datalen+1-d.n)];
    }
    if (t[2] == 0x2e)
    {
      // live range data
      rval1  = bytejoin(d[1:(d.n-1)],0,"float");
      rval_r = [rval_r, rval1];
      continue;
    }
    if (t[2] == 0x2f)
    {
      // live passive data
      rval1 = bytejoin(d[1:(d.n-1)],0,"int16");
      rval_p = [rval_p, rval1];
      continue;
    }
  }
  printf("Done!\n");
  rval = <<>>;
  rval.lrf = rval_r .* [1,1,1,6.25/5];
  rval.pir = 5.0 ./ 32768 .* rval_p;
  return rval;
};

_write_to_fpga = function(DEVICE, TOSFM, fpga_addr, fpga_val, wait_time)
{
  _this_function = "_write_to_fpga";
  if ((length(fpga_addr)!=1)||(length(fpga_val)!=1))
  {
    return nan();
  }
  if ((fpga_addr<0x4c000000)||(fpga_addr>0x4c00ffff))
  {
    printf(_this_function+": Allowed addresses are in range 0x4c00{0000:ffff}\n");
    stop(_this_function  +": Horrible Internal Error: Cannot continue!\n");
  }
  if (fpga_val>0xffff)
  {
    printf(_this_function+": Allowed values are in range 0x0000:0xffff\n");
    stop(_this_function  +": Horrible Internal Error: Cannot continue!\n");
  }
  if (!exist(wait_time))
  {
    wait_time = 0.0;
  }
  datalen = 6L;
  cmd = [0x5A,0x34,TOSFM,bytesplit(datalen,0,"uint16")];
  cmd_chksum = (256L-sum(cmd)) && 0xff;
  data = [bytesplit(fpga_addr, 0, "uint32"), bytesplit(fpga_val, 0, "uint16")];
  data_chksum = (256L-sum(data)) && 0xff;
  rval = <<>>;
  rval.addr = nan();
  rval.val  = nan();
  writem(DEVICE, [cmd, cmd_chksum, data, data_chksum]);
  if (wait_time)
  {
    sleep(wait_time);
  }
  //
  t = readm(DEVICE,6);
  while (length(t) < 6)
  {
    t = [t,readm(DEVICE,6-t.n)];
  }
  if (length(t)==6)
  {
    datalen = bytejoin(t[4:5]);
    d = readm(DEVICE,datalen+1);
    while (d.n < datalen+1)
    {
      d = [d, readm(DEVICE,datalen+1-d.n)];
    }
    d = char(d[1:datalen]);
    msg = d;
    i1 = strindex(d, "Writing");
    if (i1 > 0)
    {
      i1 = i1 + 8;
    }
    i2 = strindex(d, "to");
    if (i2 > i1)
    {
      i2 = i2 - 2;
    }
    if ((i1>0)&&(i2>0))
    {
      rval.val  = int(strtod(substr(d, i1:i2)));
    }
    i1 = strindex(d, "address");
    if (i1 > 0)
    {
      i1 = i1 + 8;
    }
    i2 = strlen(d);
    if ((i1>0)&&(i2>0))
    {
      rval.addr = int(strtod(substr(d, i1:i2)));
    }
  }
  if (LOGON)
  {
    writem(LOGFN,[cmd,cmd_chksum, data, data_chksum],<<format="%02x";eol="\n";csp=" ">>);
    writem(LOGFN,t,<<format="%02x";eol="\n";csp=" ">>);
    writem(LOGFN,msg,<<format="%s";eol="\n";csp=" ">>);
  }
  return rval;
};

_read_from_fpga = function(DEVICE, TOSFM, fpga_addr, fpga_val, wait_time)
{
  _this_function = "_write_to_fpga";
  if ((length(fpga_addr)!=1)||(length(fpga_val)!=1))
  {
    return nan();
  }
  if ((fpga_addr<0x4c000000)||(fpga_addr>0x4c00ffff))
  {
    printf(_this_function+": Allowed addresses are in range 0x4c00{0000:ffff}\n");
    stop(_this_function  +": Horrible Internal Error: Cannot continue!\n");
  }
  if (fpga_val>0xffff)
  {
    printf(_this_function+": Allowed values are in range 0x0000:0xffff\n");
    stop(_this_function  +": Horrible Internal Error: Cannot continue!\n");
  }
  if (!exist(wait_time))
  {
    wait_time = 0.0;
  }
  datalen = 6L;
  cmd = [0x5A,0x32,TOSFM,bytesplit(datalen,0,"uint16")];
  cmd_chksum = (256L-sum(cmd)) && 0xff;
  data = [bytesplit(fpga_addr, 0, "uint32"), bytesplit(fpga_val, 0, "uint16")];
  data_chksum = (256L-sum(data)) && 0xff;
  rval = <<>>;
  rval.addr = nan();
  rval.val  = nan();
  writem(DEVICE, [cmd, cmd_chksum, data, data_chksum]);
  if (wait_time)
  {
    sleep(wait_time);
  }
  //
  t = readm(DEVICE,6);
  while (length(t) < 6)
  {
    t = [t,readm(DEVICE,6-t.n)];
  }
  if (length(t)==6)
  {
    datalen = bytejoin(t[4:5]);
    d = readm(DEVICE,datalen+1);
    while (d.n < datalen+1)
    {
      d = [d, readm(DEVICE,datalen+1-d.n)];
    }
    rval.val = bytejoin(d[1:datalen]);
  }
  if (LOGON)
  {
    writem(LOGFN,[cmd,cmd_chksum, data, data_chksum],<<format="%02x";eol="\n";csp=" ">>);
    writem(LOGFN,t,<<format="%02x";eol="\n";csp=" ">>);
    writem(LOGFN,msg,<<format="%s";eol="\n";csp=" ">>);
  }
  return rval;
};

load_sfm_mission_file = function(fn)
{
  _this_solver = "load_sfm_mission_file";
  if (strlen(fn) < 0)
  {
    printf(_this_solver + ": string expected as 1st argument!\n");
    return <<>>;
  }
  if (!isfile(fn))
  {
    printf(_this_solver + ": valid filename expected as 1st argument!\n");
    return <<>>;
  }
  rval = <<>>;
  open(fn, "rb");
  do
  {
    spinner();
    //blk  = fread(fn, 262144, "uint8");
    blk  = fread(fn, 2097152, "uint8");
    rval1 = _process_sfm_block(blk);
    for (s in ["sensor", "acc", "sys"])
    {
      if (!exist(rval1.[s]))
      {
        continue;
      }
      //
      if (exist(rval.[s]))
      {
        for (m in members(rval1.[s]))
        {
          rval.[s].[m] = [rval.[s].[m];rval1.[s].[m]];
        }
        continue;
      }
      //
      rval.[s] = <<>>;
      for (m in members(rval1.[s]))
      {
        rval.[s].[m] = rval1.[s].[m];
      }
    }
    if (exist(rval1.comment))
    {
      if (exist(rval.comment))
      {
        rval.comment = rval.comment + rval1.comment;
      }
      else
      {
        rval.comment = rval1.comment;
      }
    }
  }
  while(!isempty(blk));
  close(fn);

  if (exist(rval.sys))
  {
    if (exist(rval.sys.voltages))
    {
      idx_nz = find(rval.sys.voltages[;1] != 0);
      rval.sys.m_time_s = rval.sys.m_time_s[idx_nz;];
      voltages = rval.sys.voltages[idx_nz;];
      rval.sys.vdd = <<>>;
      rval.sys.vdd.cnt = voltages[;1];
      rval.sys.vdd.val = 1.052616 ./ 1438.0 .* voltages[;1];
      rval.sys.vcc1_5v = <<>>;
      rval.sys.vcc1_5v.cnt = voltages[;2];
      rval.sys.vcc1_5v.val = 1.51158 ./ 2065.0 .* voltages[;2];
      rval.sys.vcc3_3v = <<>>;
      rval.sys.vcc3_3v.cnt = voltages[;3];
      rval.sys.vcc3_3v.val = 3.350455 ./ 2287.0 .* voltages[;3];
      rval.sys.laser_b = <<>>;
      rval.sys.laser_b.cnt = voltages[;4];
      rval.sys.laser_b.val = 150.0 ./ 118.0 .* 119.288 ./ 1612.0 .* voltages[;4];
      rval.sys.p6v = <<>>;
      rval.sys.p6v.cnt = voltages[;5];
      rval.sys.p6v.val = 5.96528 ./ 1621.0 .* voltages[;5];
      rval.sys.a6v = <<>>;
      rval.sys.a6v.cnt = voltages[;6];
      rval.sys.a6v.val = 6.0082 ./ 2731.0 .* voltages[;6];
      //
      rval.sys.apd_bias = <<>>;
      rval.sys.apd_bias.cnt = voltages[;7];
      //rval.sys.apd_bias.val = 195 ./ 170 .* 106.2513 ./ 1296.0 .* voltages[;7];
      rval.sys.apd_bias.val = 0.082443811 .* voltages[;7];
      rval.sys.tb15v = <<>>;
      rval.sys.tb15v.cnt = voltages[;8];
      rval.sys.tb15v.val = 18.018 ./ 4095.0 .* voltages[;8];
      rval.sys.tb_pos_curr = <<>>;
      rval.sys.tb_pos_curr.cnt = voltages[;9];
      rval.sys.tb_pos_curr.val = 0.221064 ./ 302.0 .* voltages[;9];
      rval.sys.tb_neg_curr = <<>>;
      rval.sys.tb_neg_curr.cnt = voltages[;10];
      rval.sys.tb_neg_curr.val = 0.130296 ./ 178.0 .* voltages[;10];
      rval.sys.a5v = <<>>;
      rval.sys.a5v.cnt = voltages[;11];
      rval.sys.a5v.val = 4.888705 ./ 3337.0 .* voltages[;11];
      rval.sys.board_temp = <<>>;
      rval.sys.board_temp.cnt = voltages[;14];
      rval.sys.board_temp.val = 299.937 ./ 1639.0 .* voltages[;14] - 273.15;
      //
    }
  }

  return rval;
};

sfmclass = classdef(dev,myaddr,sfmaddr)
{
  // static = class-member private declaration:
  //    dynamic memory storage that follows the class-member
  static(_device,_myaddr,_sfmaddr,_tosfm,_fromsfm,_m_iter);
  if ( (class(dev)!="string") || (length(dev)!=1) )
  {
    EOF
  }
  _device = dev;
  if (!open(_device,URL_OPTS))
  { sleep(2); }
  _myaddr = 0x05;
  if (isnumber(myaddr)>0)
  {
    _myaddr = myaddr;
  }
  _sfmaddr= 0x01;
  if (isnumber(sfmaddr)>0)
  {
    _sfmaddr = sfmaddr;
  }
  _fromsfm = _myaddr*16L + _sfmaddr    ;
  _tosfm   = _myaddr     + _sfmaddr*16L;
  //
  // get M-class of the SFM firmware
  //
  _m_iter = 0;
  d = char(_sfm_command(_device, _tosfm, 0x01, [], 1));
  chomp(d);
  _im = strindex(d, ".M");
  if (_im > 0)
  {
    _m_iter = strtod( substr(d,[_im+2:strlen(d)]) );
  }
  //
  //  mission accounting
  //
  static(NUM_MISSIONS, FIRST_BLOCK, LAST_BLOCK);
  NUM_MISSIONS = nan();
  FIRST_BLOCK = nan();
  LAST_BLOCK  = nan();
  //
  // service functions
  //
  public(device, sfm, addr, diary);
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
  device = function()
  {
    return _device;
  };
  sfm = function(sfmaddr)
  {
    if (isnumber(sfmaddr)>0)
    {
      _myaddr  = myaddr;
      _fromsfm = _myaddr*16L + _sfmaddr    ;
      _tosfm   = _myaddr     + _sfmaddr*16L;
    }
    return _sfmaddr;
  };
  addr = function(myaddr)
  {
    if (isnumber(myaddr)>0)
    {
      _myaddr  = myaddr;
      _fromsfm = _myaddr*16L + _sfmaddr    ;
      _tosfm   = _myaddr     + _sfmaddr*16L;
    }
    return _myaddr;
  };
  public (get_sn,get_sn_old, get_hadc,get_fpga_sample, set_tec);
  get_sn = function()
  {
    d = _sfm_command(_device, _tosfm, 0x07, [], 1);
    rval = <<>>;
    if (length(d)==6)
    {
      rval.power_board_id = bytejoin(d[1,2],0, "uint16");
      rval.sfm_id      = bytejoin(d[3,4],0, "uint16");
      rval.esad_id     = bytejoin(d[5,6],0, "uint16");
    }
    return rval;
  };
  set_tec = function(val)
  {
    ival = 0L;
    if (val > 0)
    {
      ival = 1L;
    }
    rval = _sfm_gpio_set(_device, _tosfm, SFM_GPIO_PORTA, SFM_GPIO_PIN03, ival);
    return rval;
  };
  get_fpga_sample = function()
  {
    d = _sfm_command(_device, _tosfm, 0x30, [], 1);
    rval = <<>>;
    rval.ac_thresh = <<>>;
    rval.ac_thresh.cnt = bytejoin(d[13:14],0,"int16");
    //rval.ac_thresh.val = 5 * rval.ac_thresh.cnt ./ 32768.0;
    rval.ac_thresh.val = 6.25 * rval.ac_thresh.cnt ./ 32768.0
    rval.pir_ch = <<>>;
    rval.pir_ch.cnt = bytejoin(d[1:12, 15:26],0,"int16");
    rval.pir_ch.val = 5.0 .* rval.pir_ch.cnt ./ 32768.0;
    rval.all = d[1:(d.n-1)];
    return rval;
  };
  get_hadc = function(rev)
  {
    cmd = 0x03;
    if (!exist(rev))
    {
      rev = 1;
    }
    d = _sfm_command(_device, _tosfm, cmd, [], 1);
    rval = <<>>;
    rval.vdd = <<>>;
    rval.vdd.cnt = bytejoin(d[1:2],0,"uint16");
    rval.vdd.val = bytejoin(d[3:6],0,"float");
    rval.vcc1_5v = <<>>;
    rval.vcc1_5v.cnt = bytejoin(d[7:8],0,"uint16");
    rval.vcc1_5v.val = bytejoin(d[9:12],0,"float");
    rval.vcc3_3v = <<>>;
    rval.vcc3_3v.cnt = bytejoin(d[13:14],0,"uint16");
    rval.vcc3_3v.val = bytejoin(d[15:18],0,"float");
    rval.laser_b = <<>>;
    rval.laser_b.cnt = bytejoin(d[19:20],0,"uint16");
    rval.laser_b.val = bytejoin(d[21:24],0,"float");
    if (rev == 1)
    {
      rval.laser_b.val = 150/128 * rval.laser_b.val;
    }
    rval.p6v = <<>>;
    rval.p6v.cnt = bytejoin(d[25:26],0,"uint16");
    rval.p6v.val = bytejoin(d[27:30],0,"float");
    rval.a6v = <<>>;
    rval.a6v.cnt = bytejoin(d[31:32],0,"uint16");
    rval.a6v.val = bytejoin(d[33:36],0,"float");
    rval.apd_bias = <<>>;
    rval.apd_bias.cnt = bytejoin(d[37:38],0,"uint16");
    rval.apd_bias.val = 127.936044 * bytejoin(d[39:42],0,"float") + 1.480290;
    if (rev == 2)
    {
      rval.apd_bias.val = 0.874082434 .* rval.apd_bias.val - 0.246754659;
    }
    rval.tb15v = <<>>;
    rval.tb15v.cnt = bytejoin(d[43:44],0,"uint16");
    rval.tb15v.val = bytejoin(d[45:48],0,"float");
    rval.tb_pos_curr = <<>>;
    rval.tb_pos_curr.cnt = bytejoin(d[49:50],0,"uint16");
    rval.tb_pos_curr.val = bytejoin(d[51:54],0,"float");
    rval.tb_neg_curr = <<>>;
    rval.tb_neg_curr.cnt = bytejoin(d[55:56],0,"uint16");
    rval.tb_neg_curr.val = bytejoin(d[57:60],0,"float");
    rval.a5v = <<>>;
    rval.a5v.cnt = bytejoin(d[61:62],0,"uint16");
    rval.a5v.val = bytejoin(d[63:66],0,"float");
    rval.board_temp = <<>>;
    rval.board_temp.cnt = bytejoin(d[79:80],0,"uint16");
    rval.board_temp.val = bytejoin(d[81:84],0,"float")- 273.15;
    rval.all = d;
    return rval;
  };
  public(version, set_apd_bias, set_afe_pot, set_tpg_offset, reset_fpga_cntl_reg, set_blanking_period, set_comp_le_offset,...
      write_to_fpga,mversion);
  set_blanking_period = function( bp )
  {
    if (isnumber(bp) < 1)
    {
      return 1;
    }
    if (bp < 0)
    {
      bp = 0L;
    }
    if (bp > 20L)
    {
      printf(_this_function+": blanking period longer than 20 CLKS is not possible. Forget %g!\n", ...
             bp+0.0);
      bp = 20L;
    }
    bp = 0x4900 + int(bp);
    _write_to_fpga(_device, _tosfm, SFM_FPGA_ADDR_TDC_CONTROL, 0x003f);
    _write_to_fpga(_device, _tosfm, SFM_FPGA_ADDR_TDC_COMMAND, 0x4800);
    _write_to_fpga(_device, _tosfm, SFM_FPGA_ADDR_TDC_COMMAND, bp);
    return 0;
  };
  set_tpg_offset = function( offs )
  {
    _this_function = "set_tpg_offset";
    if (isnumber(offs)<0)
    {
      return 1;
    }
    if (offs<0)
    {
      offs = 0;
    }
    if (offs>600)
    {
      offs = 600;
    }
    if (offs != round(offs,<<bin=10>>))
    {
      printf(_this_function+": offset %g is not possible. Using %g instead\n", offs+0.0, ...
          round(offs,<<bin=10>>));
    }
    offs = int(round(offs,<<bin=10>>)/10) && 0xffff;
    rval = _write_to_fpga(_device, _tosfm, SFM_FPGA_ADDR_TPG_START_DELAY, offs);
    return rval;
  };
  set_comp_le_offset = function( offs )
  {
    _this_function = "set_comp_le_offset";
    if (isnumber(offs)<0)
    {
      return 1;
    }
    if (offs<0)
    {
      offs = 0;
    }
    if (offs != round(offs,<<bin=10>>))
    {
      printf(_this_function+": offset %g is not possible. Using %g instead\n", offs+0.0, ...
          round(offs,<<bin=10>>));
    }
    offs = int(round(offs,<<bin=10>>)/10) && 0xffff;
    rval = _write_to_fpga(_device, _tosfm, SFM_FPGA_ADDR_COMP_LE_DELAY, offs);
    return rval;
  };
  reset_fpga_cntl_reg = function ()
  {
    _this_function = "reset_fpga_cntl_reg";
    rval = _write_to_fpga(_device, _tosfm, SFM_FPGA_ADDR_FPGA_CONTROL, 0x0000);
    return rval;
  };
  write_to_fpga = function(fpga_addr, fpga_val, wait_time)
  {
    rval = _write_to_fpga(_device, _tosfm, fpga_addr, fpga_val, wait_time);
    return rval;
  };
  version = function()
  {
    cmd = 0x01;
    d = _sfm_command(_device, _tosfm, cmd, [], 1);
    msg = char(d);
    chomp(msg);
    return msg;
  };
  mversion = function()
  {
    return _m_iter;
  };
  set_afe_pot = function(val)
  {
    _this_function = "set_afe_pot";
    if (isnumber(val)<1)
    {
      return nan();
    }
    if (val < 0)
    {
      printf(_this_function+": AFE Pot cannot be less than 0%% -> pot requested %g%%\n", val);
      stop(_this_function  +": Horrible Internal Error: Cannot continue!\n");
    }
    if (val > 100)
    {
      printf(_this_function+": AFE Pot cannot be greater than than 100%% -> pot requested %g%%\n", ...
          val);
      stop(_this_function  +": Horrible Internal Error: Cannot continue!\n");
    }
    cmd  = 0x40;
    data = int(round(2.55 * val));
    if (data < 0)
    {
      data = 0L;
    }
    if (data > 255L)
    {
      data = 255L;
    }
    rval = nan();
    d = _sfm_command(_device, _tosfm, cmd, data, 1);
    i1 = strindex(d, "to");
    if (i1 > 0)
    {
      i1 = i1 + 3;
    }
    i2 = strindex(d, "counts");
    if (i2 > i1)
    {
      i2 = i2 - 2;
    }
    if ((i1>0)&&(i2>0))
    {
      rval  = round(strtod(substr(d, i1:i2)) * 100/255, <<bin=0.1>>);
    }
    return rval;
  };
  set_apd_bias = function(val, rev)
  {
    _this_function = "set_apd_bias";
    if (isnumber(rev)<1)
    {
      rev = 1;
    }
    if (isnumber(val)<1)
    {
      return nan();
    }
    if (rev == 1)
    {
      if (val < 125)
      {
        printf(_this_function+": APD Bias cannot be less than 125V -> bias requested %g V\n", val);
        stop(_this_function  +": Horrible Internal Error: Cannot continue!\n");
      }
      if (val > 210)
      {
        printf(_this_function+": APD Bias cannot be higher than 210 V -> bias requested %g V\n", val);
        stop(_this_function  +": Horrible Internal Error: Cannot continue!\n");
      }
    }
    if (rev == 2)
    {
      if (val < 45)
      {
        printf(_this_function+": APD Bias cannot be less than 45V -> bias requested %g V\n", val);
        stop(_this_function  +": Horrible Internal Error: Cannot continue!\n");
      }
      if (val > 210)
      {
        printf(_this_function+": APD Bias cannot be higher than 210 V -> bias requested %g V\n", val);
        stop(_this_function  +": Horrible Internal Error: Cannot continue!\n");
      }
    }
    cmd = 0x38;
    if (rev == 1)
    {
      cnt = int(round(22.0080564 * val - 2692.56167));
    }
    if (rev == 2)
    {
      cnt = int(round(18.2 * val - 819.0));
    }
    if (cnt<0)
    {
      cnt = 0L;
    }
    data = bytesplit(int(cnt),0,"uint16");
    d = _sfm_command(_device, _tosfm, cmd, data, 1);
    rval = nan();
    if (d.class == "num")
    {
      d = bytejoin(d, 0, "uint32");
      rval = 1;
      if (d == 0x1f)
      {
        rval = 0;
      }
    }
    if (d.class == "string")
    {
      i1 = strindex(d, "to");
      if (i1 > 0)
      {
        i1 = i1 + 3;
      }
      i2 = strindex(d, "counts");
      if (i2 > i1)
      {
        i2 = i2 - 2;
      }
      if ((i1>0)&&(i2>0))
      {
        d  = strtod(substr(d, i1:i2));
        if (isnumber(rval)>0)
        {
          if (rev == 1)
          {
            rval = round(0.0454378555 * d + 122.344416, <<bin=0.1>>);
          }
          if (rev == 2)
          {
            rval = round(0.0549450549 * d +  45.0, <<bin=0.1>>);
          }
        }
      }
    }
    return rval;
  };
  public(collect_range_data, collect_pir_data, collect_data,mission, emmc);
  collect_range_data = function(pri,dur,nwt)
  {
    rval = _collect_sfm_data (_device, _tosfm, 1, pri, dur, nwt);
    if (!isempty(rval.lrf))
    {
      ncol = 3;
      if (_m_iter > 3)
      {
        ncol = 4;
      }
      //resize(rval.lrf,ncol,rval.lrf.n/ncol);
      _reshape(rval.lrf,ncol,rval.lrf.n/ncol);
      rval.lrf = rval.lrf';
      if (_m_iter > 3)
      {
        rval.ac_thresh = rval.lrf[;4];
        rval.lrf = rval.lrf[;1,2,3];
      }
    }
    else
    {
      printf("collect_range_data: Warning: No data received: Check your installation!\n");
    }
    return rval;
  };
  collect_pir_data = function(pri,dur,nwt)
  {
    rval = _collect_sfm_data (_device, _tosfm, 2, pri, dur, nwt);
    if (!isempty(rval.pir))
    {
      ncol = 12;
      if (_m_iter == 3)
      {
        ncol = 13;
      }
      //resize(rval.pir,ncol,rval.pir.n/ncol);
      _reshape(rval.pir,ncol,rval.pir.n/ncol);
      rval.pir= rval.pir';
      if (_m_iter == 3)
      {
        rval.ac_thresh = rval.pir[;13];
        rval.pir = rval.pir[;1:12];
      }
    }
    return rval;
  };
  collect_data = function(pri,dur,nwt)
  {
    rval = _collect_sfm_data (_device, _tosfm, 3, pri, dur, nwt);
    if (!isempty(rval.pir))
    {
      ncol = 12;
      if (_m_iter == 3)
      {
        ncol = 13;
      }
      //resize(rval.pir,ncol,rval.pir.n/ncol);
      _reshape(rval.pir,ncol,rval.pir.n/ncol);
      rval.pir= rval.pir';
      if (_m_iter == 3)
      {
        rval.ac_thresh = rval.pir[;13];
        rval.pir = rval.pir[;1:12];
      }
    }
    if (!isempty(rval.lrf))
    {
      ncol = 3;
      if (_m_iter > 3)
      {
        ncol = 4;
      }
      //resize(rval.lrf,ncol,rval.lrf.n/ncol);
      _reshape(rval.lrf,ncol,rval.lrf.n/ncol);
      rval.lrf = rval.lrf';
      rval.ac_thresh = rval.lrf[;4];
      rval.lrf = rval.lrf[;1,2,3];
    }
    return rval;
  };
  mission = <<>>;
  mission.start = <<>>;
  mission.start.tactical = function(wtd, comment)
  {
    _this_function = "mission.start.tactical";
    data = [];
    if (_m_iter > 1)
    {
      comment = SFM_TACTICAL_MISSION_COMMENT_DEFAULT;
      if (exist(FORTUNE))
      {
        comment = sum(reads( "|" + FORTUNE + " 2>/dev/null" ), "\n");
      }
      data = ascii(comment);
      if (length(data)>506)
      {
        data = data[1:506];
      }
    }
    else
    {
      printf("Version M%.0f does not support header/comment and blocks with 0xBB identification!\n", _m_iter);
    }
    cmd = 0x1e;
    if (wtd)
    {
      if (_m_iter > 3)
      {
        // first response:
        rval = _sfm_command(_device, _tosfm, cmd, data, wtd);
        d = _sfm_command_resp(_device, _tosfm, 0x20);
        // second response:
        d = _sfm_command_resp(_device, _tosfm, 0x3d);
        if (length(d) == 8)
        {
          rval = bytejoin(d,0,"uint32");
        }
      }
      else
      {
        // first response:
        rval = _sfm_command(_device, _tosfm, cmd, data, wtd);
        _sfm_command_resp(_device, _tosfm, 0x20);
      }
    }
    else
    {
      rval = _sfm_command(_device, _tosfm, cmd, data, wtd);
    }
    return rval;
  };
  mission.abort = function(wtd)
  {
    _this_function = "mission.abort";
    cmd = 0x2a;
    d = _sfm_command(_device, _tosfm, cmd, [], 1);
    rval = 1;
    if (d.n >= 1)
    {
      if (d[1] == 0x1f)
      {
        rval = 0;
      }
    }
    return rval;
  };
  mission.parameters = function(val)
  {
    _this_function = "mission.parameters";
    cmd = 0x1b;
    d = _sfm_command(_device, _tosfm, cmd, [], 1);
    rval = <<>>;
    rval.pri_us  = bytejoin(d[5:6],0,"uint16");
    rval.pri_hz  = 1e6 ./ rval.pri_us;
    if (_m_iter > 2)
    {
      rval.sample_count = bytejoin(d[1:4],0,"uint32");
      rval.time_s = rval.pri_us .* 1e-6 .* rval.sample_count;
      rval.time_ms = 1e3 * rval.time_s;
    }
    else
    {
      rval.time_ms = bytejoin(d[1:4],0,"uint32");
      rval.time_s  = 1e-3 * rval.time_ms;
      rval.sample_count = rval.pri_hz .* rval.time_s;
    }
    rval.accel = d[7:30];
    rval.dynamic_pri_enable = d[31];
    rval.das_debug_msg_enable = d[32];
    if (_m_iter > 2)
    {
      rval.laser_inhibit = d[33];
    }
    else
    {
      rval.auto_mission_enable = d[33];
    }
    //
    if (type(val) != "list")
    {
      return rval;
    }
    do_update = 0;
    // for now don't do anything with acceleration registers
    d[7:30] = int(zeros(d[7:30]));
    if (isnumber(val.pri_us)>0)
    {
      rval.pri_us = val.pri_us;
      rval.pri_hz = round(1e6 ./ val.pri_us);
      _p = int(rval.pri_us);
      d[5:6] = bytesplit(_p,0,"uint16");
      do_update = 1;
    }
    else if (isnumber(val.pri_hz)>0)
    {
      rval.pri_hz = val.pri_hz;
      rval.pri_us = round(1e6 ./ val.pri_hz);
      _p = int(rval.pri_us);
      d[5:6] = bytesplit(_p,0,"uint16");
      do_update = 1;
    }
    //
    if (isnumber(val.sample_count)>0)
    {
      rval.sample_count =  val.sample_count;
      rval.time_s = rval.sample_count ./ rval.pri_hz;
      rval.time_ms = 1e3 .* rval.time_s;
      do_update = 1;
      if (_m_iter > 2)
      {
        d[1:4] = bytesplit(int(round(rval.sample_count+0.0)),0,"uint32");
      }
      else
      {
        d[1:4] = bytesplit(int(round(rval.time_ms+0.0)),0,"uint32");
      }
    }
    else if (isnumber(val.time_ms)>0)
    {
      rval.time_ms = val.time_ms;
      rval.time_s  = 1e-3 * rval.time_ms;
      rval.sample_count = rval.pri_hz .* rval.time_s;
      do_update = 1;
      if (_m_iter > 2)
      {
        d[1:4] = bytesplit(int(round(rval.sample_count+0.0)),0,"uint32");
      }
      else
      {
        d[1:4] = bytesplit(int(round(rval.time_ms+0.0)),0,"uint32");
      }
    }
    else if (isnumber(val.time_s)>0)
    {
      rval.time_s = val.time_s;
      rval.time_ms  = 1e3 * rval.time_s;
      rval.sample_count = rval.pri_hz .* rval.time_s;
      do_update = 1;
      if (_m_iter > 2)
      {
        d[1:4] = bytesplit(int(round(rval.sample_count+0.0)),0,"uint32");
      }
      else
      {
        d[1:4] = bytesplit(int(round(rval.time_ms+0.0)),0,"uint32");
      }
    }
    //
    if (isnumber(val.dynamic_pri_enable)>0)
    {
      d[31] = int(round(val.dynamic_pri_enable+0.0));
      rval.dynamic_pri_enable = val.dynamic_pri_enable;
      do_update = 1;
    }
    if (isnumber(val.das_debug_msg_enable)>0)
    {
      d[32] = int(round(val.das_debug_msg_enable+0.0));
      rval.das_debug_msg_enable = val.das_debug_msg_enable;
      do_update = 1;
    }
    if (_m_iter > 2)
    {
      if (isnumber(val.laser_inhibit)>0)
      {
        d[33] = int(round(val.laser_inhibit+0.0));
        rval.laser_inhibit = val.laser_inhibit;
        do_update = 1;
      }
      if (isnumber(val.auto_mission_enable)>0)
      {
        printf("The M%.0f version of SFM does not support 'auto_mission_enable' flag\n",...
            _m_iter);
      }
    }
    else
    {
      if (isnumber(val.auto_mission_enable)>0)
      {
        d[33] = int(round(val.auto_mission_enable+0.0));
        rval.auto_mission_enable = val.auto_mission_enable;
        do_update = 1;
      }
      if (isnumber(val.laser_inhibit)>0)
      {
        printf("The M%.0f version of SFM does not support 'laser_inhibit' flag\n",...
            _m_iter);
      }
    }
    if (do_update)
    {
      datalen = 33L;
      cmd = [0x5A,0x19,_tosfm,bytesplit(datalen,0,"uint16")];
      cmd_chksum = (256L-sum(cmd)) && 0xff;
      data_chksum = (256L-sum(d[1:datalen])) && 0xff;
      d[34] = data_chksum;
      if (LOGON)
      {
        writem(LOGFN,[cmd,cmd_chksum, d], ...
            <<format="%02x";eol="\n";csp=" ">>);
      }
      writem(_device, [cmd, cmd_chksum, d]);
    }

    return rval;
  };
  emmc = <<>>;
  emmc.mission.number = function()
  {
    return NUM_MISSIONS;
  };
  emmc.mission.blocks = function()
  {
    if (isnan(NUM_MISSIONS))
    {
      return <<>>;
    }
    if (NUM_MISSIONS<1)
    {
      return <<>>;
    }
    return <<first_block=FIRST_BLOCK;last_block=LAST_BLOCK>>;
  };
  emmc.mission.save_to_file = function (midx, fn, ddir)
  {
    if (isnumber(midx)<1)
    {
      return 1;
    }
    [midx, NUM_MISSIONS]?
    if (midx > NUM_MISSIONS)
    {
      return 3;
    }
    b1 = FIRST_BLOCK[midx];
    lb = LAST_BLOCK[midx];
    "expected blocks"?
    [b1, lb]?
    if (strlen(fn)<1)
    {
      //
      // check first block for filename
      //
      db = 1;
      data = [bytesplit(int(b1),0,"uint32") , int(db)];
      cmd = 0x69;
      d = _sfm_command(_device, _tosfm, cmd, data, 1);
      if (d.n != 519)
      {
        printf("Problem in the first data block transfer: Expected %g \t Received %g\n", ...
            519, d.n);
      }
      //
      // process block data
      //
      if (d[6] == RECORD_ID_EMMC_MISSION_COMMENT)
      {
        fn = char(d[7:133,135:261,263:389,391:517]);
        if (strlen(ddir) > 0)
        {
          fn = ddir + fn;
        }
      }
      b1 = b1 + db;
    }
    open(fn,"wb");
    while (b1 <= lb)
    {
      db = min(lb-b1+1,6);
      data = [bytesplit(int(b1),0,"uint32") , int(db)];
      cmd = 0x69;
      d = _sfm_command(_device, _tosfm, cmd, data, 1);
      if (d.n != db * 512 + 7)
      {
        printf("Problem in data block transfer: Expected %g \t Received %g\n", ...
            db*512+7, d.n);
      }
      //
      // process block data
      //
      writem(fn, d[6:(d.n-2)], <<format="%c";eol="";csp="">>);
      b1 = b1 + db;
    }
    close(fn);
    return 0;
  };
  emmc.mission.save_blocks_to_file = function (fb, lb, fn)
  {
    if (isnumber(fb)<1)
    {
      return [];
    }
    if (isnumber(lb)<1)
    {
      return [];
    }
    if (strlen(fn)<1)
    {
      return [];
    }
    open(fn,"wb");
    b1 = fb;
    while (b1 <= lb)
    {
      db = min(lb-b1+1,6);
      data = [bytesplit(int(b1),0,"uint32") , int(db)];
      cmd = 0x69;
      d = _sfm_command(_device, _tosfm, cmd, data, 1);
      if (d.n != db * 512 + 7)
      {
        printf("Problem in data block transfer: Expected %g \t Received %g\n", ...
            db*512+7, d.n);
      }
      //
      // process block data
      //
      bidx = bytejoin(d[1:4],0,"uint32");
      nblk = floor(d[5]);
      crc  = bytejoin(d[d.n-1,d.n],0,"uint16");
      blk  = d[6:(d.n-2)];
      writem(fn, d[6:(d.n-2)], <<format="%c";eol="";csp="">>);
      b1 = b1 + db;
    }
    close(fn);
    return 0;
  };
  emmc.clearall = function()
  {
    _this_function = "emmc.status";
    cmd = 0x7a;
    d = _sfm_command(_device, _tosfm, cmd, [], 1);
    rval = <<>>;
  };
  emmc.status = function()
  {
    _this_function = "emmc.status";
    cmd = 0x78;
    d = _sfm_command(_device, _tosfm, cmd, [], 1);
    rval = <<>>;
    rval.raw = d;
    //d = char(d);
    //i1 = strindex(d, "Check: ") + 6;
    //i2 = strindex(d, " Missions") - 1;
    //rval.num_missions = strtod(substr(d,i1:i2),<<lstrip="'BLANK">>);
    //i1 = strindex(d, "memory. ") + 8;
    //i2 = strindex(d, " Blocks used") - 1;
    //rval.blocks_used = strtod(substr(d,i1:i2),<<lstrip="'BLANK">>);
    //i1 = strindex(d, "used. ") + 6;
    //i2 = strindex(d, " Blocks available") - 1;
    //rval.blocks_available = strtod(substr(d,i1:i2),<<lstrip="'BLANK">>);
    rval.num_missions = bytejoin(d[2:3],0,"uint16");
    rval.blocks_used = bytejoin(d[4:7],0,"uint32");
    rval.blocks_available = bytejoin(d[8:11],0,"uint32");
    rval.first_block = zeros(rval.num_missions,1);
    rval.last_block = zeros(rval.num_missions,1);
    rval.accel_sys_records = zeros(rval.num_missions,1);
    rval.sensor_records = zeros(rval.num_missions,1);
    ilo = 513;
    ihi = ilo + 15;
    for (i in 1:rval.num_missions)
    {
      x = bytejoin(d[ilo:ihi],0,"uint32");
      rval.first_block[i] = round(x[1]+0.0);
      rval.last_block[i] = round(x[2]+0.0);
      rval.accel_sys_records[i] = round(x[3]+0.0);
      rval.sensor_records[i] = round(x[4]+0.0);
      ilo = ilo + 16;
      ihi = ihi + 16;
    }
    NUM_MISSIONS  = round(rval.num_missions+0.0);
    FIRST_BLOCK   = round(rval.first_block+0.0);
    LAST_BLOCK    = round(rval.last_block+0.0);
    return rval;
  };
  emmc.get_data_blocks = function (fb, nb)
  {
    if (isnumber(nb)<1)
    {
      nb = 1;
    }
    if (nb > 6)
    {
      printf("Cannot download more than 6 blocks");
      nb = 6;
    }
    data = [bytesplit(int(fb),0,"uint32") , int(nb)];
    cmd = 0x69;
    d = _sfm_command(_device, _tosfm, cmd, data, 1);
    if (d.n != nb * 512 + 7)
    {
      printf("Problem in data block transfer: Expected %g \t Received %g\n", ...
          nb*512+7, d.n);
    }
    //
    // process block data
    //
    bidx = bytejoin(d[1:4],0,"uint32");
    nblk = floor(d[5]);
    crc  = bytejoin(d[d.n-1,d.n],0,"uint16");
    blk  = d[6:(d.n-2)];
    rec  = blk;
    if (nblk > 1)
    {
      //resize(blk, blk.n/nblk, nblk);
      _reshape(blk, blk.n/nblk, nblk);
      blk = blk';
    }
    //
    //resize(rec,128,nblk*4);
    _reshape(rec,128,nblk*4);
    rec = rec';

    nrec     = 0;
    nrec_sys = 0;
    nrec_acc = 0;
    for (i in 1:rec.nr)
    {
      if (rec[i;1] == RECORD_ID_SENSOR_DATA)
      {
        nrec++;
        continue;
      }
      if (rec[i;1] == RECORD_ID_SYS_DATA)
      {
        nrec_sys++;
        continue;
      }
      if ((rec[i;1] == RECORD_ID_ACCEL0)||(rec[i;1] == RECORD_ID_ACCEL1)||(rec[i;1] == RECORD_ID_ACCEL2)...
           ||(rec[i;1] == RECORD_ID_ACCEL3))
      {
        nrec_acc++;
        continue;
      }
    }
    // sensor data record
    if (nrec>0)
    {
      m_time_us = int(zeros(nrec,1));
      pri_us    = zeros(nrec,1);
      fpga_ctrl_reg = int(zeros(nrec,1));
      pir = int(zeros(nrec,12));
      ac_thresh = int(zeros(nrec,1));
      tdc_ch_tof = int(zeros(nrec,6));
      tdc_ch_cal1 = int(zeros(nrec,6));
      tdc_ch_cal2 = int(zeros(nrec,6));
      status_reg = int(zeros(nrec,1));
      range_m = zeros(nrec,3);
      pulse_width = zeros(nrec,3);
    }
    // system data record
    if (nrec_sys > 0)
    {
      m_time_us_sys = int(zeros(2*nrec_sys,1));
      sample_num_sys = int(zeros(2*nrec_sys,1));
      voltages = int(zeros(2*nrec_sys,14));
      mission_flags = int(zeros(2*nrec_sys,1));
      gpio_data = int(zeros(2*nrec_sys,6));
    }
    if (nrec_acc > 0)
    {
      m_time_us_acc = int(zeros(8*nrec_acc,1));
      sample_num_acc = int(zeros(8*nrec_acc,1));
      acc_idx = int(zeros(8*nrec_acc,1));
      xyz_data = int(zeros(8*nrec_acc,3));
    }
    // accel data record
    j = 0;
    j_acc = 0;
    j_sys = 0;
    for (i in 1:rec.nr)
    {
      if (rec[i;1] == RECORD_ID_SENSOR_DATA)
      {
        j++;
        //
        // sensor data record
        //
        m_time_us[j]      = bytejoin(rec[i; 2: 5],0,"uint32");
        // registered 6:7
        pri_us[j]         = bytejoin(rec[i; 8: 9],0,"uint16") * 1e-2;
        fpga_ctrl_reg[j]  = bytejoin(rec[i;10:11],0,"uint16");
        // pir: 12 14 16 18 20 22 [] 26 28 30 32 34 36
        pir[j;] = bytejoin(rec[i;12:23,26:37],0,"int16");
        // ac_thresh 24
        ac_thresh[j]  = bytejoin(rec[i;24,25],0,"uint16");
        // tdc_ch_tof: 38 40 42 44 46 48
        tdc_ch_tof[j;] = bytejoin(rec[i;38:49],0,"uint16");
        // tdc_ch_cal1: 50 52 54 56 58 60
        tdc_ch_cal1[j;] = bytejoin(rec[i;50:61],0,"uint16");
        // tdc_ch_cal2: 62 64 66 68 70 72
        tdc_ch_cal2[j;] = bytejoin(rec[i;62:73],0,"uint16");
        // status_reg: 74
        status_reg[j]  = bytejoin(rec[i;74,75],0,"uint16");
        // range_m: 76 80 84
        range_m[j;]  = bytejoin(rec[i;76:87],0,"float");
        // pulse_width: 88 92 96
        pulse_width[j;]  = bytejoin(rec[i;88:99],0,"float");
        continue;
      }
      if ((rec[i;1] == RECORD_ID_ACCEL0)||(rec[i;1] == RECORD_ID_ACCEL1)||(rec[i;1] == RECORD_ID_ACCEL2)...
           ||(rec[i;1] == RECORD_ID_ACCEL3))
      {
        j_acc++;
        for (j2 in 0:7)
        {
          _dof = (j2*16);
          acc_idx[j_acc + j2]       =          rec[i; 1+_dof];
          m_time_us_acc[j_acc + j2] = bytejoin(rec[i; 2+_dof: 5+_dof],0,"uint32");
          sample_num_acc[j_acc +j2] = bytejoin(rec[i; 6+_dof: 7+_dof],0,"uint16");
          xyz_data[j_acc+j2;]       = bytejoin(rec[i; 8+_dof: 13+_dof],0,"int16");
        }
        continue;
      }
      if (rec[i;1] == RECORD_ID_SYS_DATA)
      {
        j_sys++;
        for (j2 in 0:1)
        {
          _dof = 1 + (j2*64);
          m_time_us_sys[j_sys + j2] = bytejoin(rec[i; 1+_dof:  4+_dof],0,"uint32");
          sample_num_sys[j_sys +j2] = bytejoin(rec[i; 5+_dof:  6+_dof],0,"uint16");
          voltages[j_sys +j2;]      = bytejoin(rec[i; 7+_dof: 34+_dof],0,"uint16");
          mission_flags[j_sys +j2]  = bytejoin(rec[i;35+_dof: 36+_dof],0,"uint16");
          gpio_data[j_sys + j2;]    = bytejoin(rec[i;37+_dof: 48+_dof],0,"uint16");
        }
        continue;
      }
    }
    //
    rval = <<>>;
    rval.block_idx = [bidx:(bidx+nblk-1)]';
    rval.block_crc16 = crc;
    rval.block = blk;
    rval.records = rec;
    //
    if (nrec>0)
    {
      rval.sensor = <<>>;
      rval.sensor.m_time_us = m_time_us;
      rval.sensor.pri_us = pri_us;
      rval.sensor.fpga_ctrl_reg = fpga_ctrl_reg;
      rval.sensor.pir = pir;
      rval.sensor.ac_thresh = ac_thresh;
      rval.sensor.tdc_ch_tof = tdc_ch_tof;
      rval.sensor.tdc_ch_cal1 = tdc_ch_cal1;
      rval.sensor.tdc_ch_cal2 = tdc_ch_cal2;
      rval.sensor.status_reg = status_reg;
      rval.sensor.range_m = range_m;
      rval.sensor.pulse_width = pulse_width;
    }
    if (nrec_sys > 0)
    {
      rval.sys = <<>>;
      rval.sys.m_time_us =  m_time_us_sys;
      rval.sys.sample_num = sample_num_sys;
      //
      //rval.sys.voltages = voltages;
      //
      rval.sys.vdd = <<>>;
      rval.sys.vdd.cnt = voltages[;1];
      rval.sys.vdd.val = 1.052616 ./ 1438.0 .* voltages[;1]
      rval.sys.vcc1_5v = <<>>;
      rval.sys.vcc1_5v.cnt = voltages[;2];
      rval.sys.vcc1_5v.val = 1.51158 ./ 2065.0 .* voltages[;2];
      rval.sys.vcc3_3v = <<>>;
      rval.sys.vcc3_3v.cnt = voltages[;3];
      rval.sys.vcc3_3v.val = 3.350455 ./ 2287.0 .* voltages[;3];
      rval.sys.laser_b = <<>>;
      rval.sys.laser_b.cnt = voltages[;4];
      rval.sys.laser_b.val = 119.288 ./ 1612.0 .* voltages[;4];
      rval.sys.p6v = <<>>;
      rval.sys.p6v.cnt = voltages[;5];
      rval.sys.p6v.val = 5.96528 ./ 1621.0 .* voltages[;5];
      rval.sys.a6v = <<>>;
      rval.sys.a6v.cnt = voltages[;6];
      rval.sys.a6v.val = 6.0082 ./ 2731.0 .* voltages[;6];
      rval.sys.apd_bias = <<>>;
      rval.sys.apd_bias.cnt = voltages[;7];
      rval.sys.apd_bias.val = 106.2513 ./ 1296.0 .* voltages[;7];
      rval.sys.tb15v = <<>>;
      rval.sys.tb15v.cnt = voltages[;8];
      rval.sys.tb15v.val = 18.018 ./ 4095.0 .* voltages[;8];
      rval.sys.tb_pos_curr = <<>>;
      rval.sys.tb_pos_curr.cnt = voltages[;9];
      rval.sys.tb_pos_curr.val = 0.221064 ./ 302.0 .* voltages[;9];
      rval.sys.tb_neg_curr = <<>>;
      rval.sys.tb_neg_curr.cnt = voltages[;10];
      rval.sys.tb_neg_curr.val = 0.130296 ./ 178.0 .* voltages[;10];
      rval.sys.a5v = <<>>;
      rval.sys.a5v.cnt = voltages[;11];
      rval.sys.a5v.val = 4.888705 ./ 3337.0 .* voltages[;11];
      rval.sys.board_temp = <<>>;
      rval.sys.board_temp.cnt = voltages[;14];
      rval.sys.board_temp.val = 299.937 ./ 1639.0 .* voltages[;14] - 273.15;
      //
      rval.sys.mission_flags = mission_flags;
      rval.sys.gpio_data = gpio_data;
    }
    if (nrec_acc > 0)
    {
      rval.acc = <<>>;
      rval.acc.m_time_us = m_time_us_acc;
      rval.acc.idx = acc_idx;
      rval.acc.sample_num = sample_num_acc;
      rval.acc.xyz_data = xyz_data;
    }
    return rval;
  };
};



















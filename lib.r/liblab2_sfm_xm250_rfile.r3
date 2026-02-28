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
_THIS_LIB = "liblab2_sfm_xm250_rfile";

static(URL_OPTS);
URL_OPTS = <<>>;
//URL_OPTS.speed = 230400;
//URL_OPTS.speed = 921600; // since 2012-07-29, CDAEM SFM 1.30.1.M1, .M2, .M3
URL_OPTS.speed = 460800; //XM250
URL_OPTS.data_parity_stop = "8N1";

static(SFM_GPIO_PORTA,SFM_GPIO_PORTB,SFM_GPIO_PORTC,SFM_GPIO_PORTD,SFM_GPIO_PORTE,SFM_GPIO_PORTF,SFM_GPIO_PORTG,HEADER_LEN);
SFM_GPIO_PORTA = 0x00;
SFM_GPIO_PORTB = 0x01;
SFM_GPIO_PORTC = 0x02;
SFM_GPIO_PORTD = 0x03;
SFM_GPIO_PORTE = 0x04;
SFM_GPIO_PORTF = 0x05;
SFM_GPIO_PORTG = 0x06;
// XM250: HEADER LENGTH:
HEADER_LEN = 8;
static(SFM_GPIO_PIN00,SFM_GPIO_PIN01,SFM_GPIO_PIN02,SFM_GPIO_PIN03,SFM_GPIO_PIN04,SFM_GPIO_PIN05,SFM_GPIO_PIN06,SFM_GPIO_PIN07,SFM_GPIO_PIN08,SFM_GPIO_PIN09);
SFM_GPIO_PIN00 = 0000000000000001B;
SFM_GPIO_PIN01 = 0000000000000010B;
SFM_GPIO_PIN02 = 0000000000000100B;
SFM_GPIO_PIN03 = 0000000000001000B;
SFM_GPIO_PIN04 = 0000000000010000B;
SFM_GPIO_PIN05 = 0000000000100000B;
SFM_GPIO_PIN06 = 0000000001000000B;
SFM_GPIO_PIN07 = 0000000010000000B;
SFM_GPIO_PIN08 = 0000000100000000B;
SFM_GPIO_PIN09 = 0000001000000000B;

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

static(_convert_apd_bias_v_to_cnt, _convert_afe_pot_p_to_cnt);
_convert_apd_bias_v_to_cnt = function (val)
{
  if (isnumber(val)<1)
  {
    return -1;
  }
  if (val < 40)
  {
    val = 40;
  }
  if (val > 210)
  {
    val = 210;
  }

  // val
  val = (val - 6.06963513) ./ 0.825176252;
  cnt = int(round((val - 40)./240.*4095));
  return cnt;
};

_convert_afe_pot_p_to_cnt = function (val)
{
  if (isnumber(val)<1)
  {
    return nan();
  }
  if (val < 0)
  {
    val = 0;
  }
  if (val > 100)
  {
    val = 100;
  }
  cnt = int(round(2.55 * val)) && 0xff;
  return cnt;
};


static(_read_from_fpga,_write_to_fpga,_sfm_gpio_set,_collect_sfm_data,_sfm_command_resp,_sfm_command);
_sfm_gpio_set = function(DEVICE, port, pin, val)
{
  _this_function = "_sfm_gpio_set";
  if ((length(port)!=1)||(length(pin)!=1))
  {
    return nan();
  }
  if (!exist(val))
  {
    val = 0x02;
  }
  if (val<=0)
  {
    val = 0;
  }
  if ((val != 0) && (val!=2))
  {
    val = 1;
  }
  //
  msg_type  = 0x811c;
  datalen   = 4L;
  cmd = [0x5A, bytesplit(msg_type,0,"uint16"), 0x00, 0x00, bytesplit(datalen,0,"uint16")];
  cmd_chksum = (256L-sum(cmd)) && 0xff;
  //
  data = [int(val), bytesplit(pin,0,"uint16"), port];
  data_chksum = (256L-sum(data)) && 0xff;
  writem(DEVICE, [cmd, cmd_chksum, data, data_chksum]);
  if (LOGON)
  {
    writem(LOGFN,[cmd,cmd_chksum, data, data_chksum],<<format="%02x";eol="\n";csp=" ">>);
  }
  t = readm(DEVICE,3);
  return t[2];
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

_sfm_command = function(DEVICE, msg_type, data, wfr)
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
  msg_type  = int(msg_type);
  datalen   = int(length(data));
  cmd = [0x5A, bytesplit(msg_type,0,"uint16"), 0x00, 0x00, bytesplit(datalen,0,"uint16")];
  cmd_chksum = (256L-sum(cmd)) && 0xff;
  if (datalen > 0)
  {
    data_chksum = (256L-sum(data)) && 0xff;
  }
  _d = [cmd, cmd_chksum, data, data_chksum];
  writem(DEVICE, _d);
  if (LOGON)
  {
    writem(LOGFN, _d,<<format="%02x";eol="\n";csp=" ">>);
  }
  if (!wfr)
  {
    return 0;
  }
  //
  // waiting for reply
  //
  t = readm(DEVICE,HEADER_LEN);
  while (length(t) < HEADER_LEN)
  {
    t = [t,readm(DEVICE,HEADER_LEN-t.n)];
  }
  datalen = bytejoin(t[HEADER_LEN-2:HEADER_LEN-1]);
  if (datalen == 0)
  {
    if (t[2] == msg_type+1)
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

_collect_sfm_data = function(DEVICE, wh, pri, dur, params)
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
  if (pri < 1525)
  {
    printf(_this_function+": PRI cannot be less than 1525 -> PRI requested was %g: Set to 1525.\n", pri);
    pri = 1525;
  }
  if (dur < 0)
  {
    printf(_this_function+": duration cannot be less than 0 -> requested duration was %g: Set to 1 second.\n", dur);
    dur =1;
  }
  //
  // process streaming parameters
  //
  apd_bias_cnt      = -1;
  afe_pot_cnt       = -1;
  tdc_blanking_cnt  = -1;
  pulse_width_hw_blank_cnt = -1L;
  comp_le_delay_cnt = -1;
  check_for_message = 0;
  if (type(params)=="list")
  {
    if (exist(params.apd_bias))
    {
      if (params.apd_bias > -1)
      {
        apd_bias_cnt = _convert_apd_bias_v_to_cnt(params.apd_bias);
      }
    }
    if (exist(params.afe_pot))
    {
      if (params.afe_pot > -1)
      {
        afe_pot_cnt = _convert_afe_pot_p_to_cnt(params.afe_pot);
      }
    }
    if (exist(params.tdc_blanking))
    {
      if (params.tdc_blanking > -1)
      {
        tdc_blanking_cnt = int(params.tdc_blanking);
      }
    }
    if (exist(params.hw_blanking))
    {
      if (params.hw_blanking > -1)
      {
        //
        if (pulse_width_hw_blank_cnt == -1)
        {
          pulse_width_hw_blank_cnt = 0L;
        }
        else
        {
          pulse_width_hw_blank_cnt = pulse_width_hw_blank_cnt && 0xc000;
        }
        //
        pulse_width_hw_blank_cnt = pulse_width_hw_blank_cnt || int(params.hw_blanking);
      }
    }
    if (exist(params.pulse_width_ns))
    {
      if (params.pulse_width_ns > -1)
      {
        //
        if (pulse_width_hw_blank_cnt == -1)
        {
          pulse_width_hw_blank_cnt = 0L;
        }
        else
        {
          pulse_width_hw_blank_cnt = pulse_width_hw_blank_cnt && 0x3fff;
        }
        //
        if (params.pulse_width_ns == 25)
        {
          pulse_width_hw_blank_cnt = pulse_width_hw_blank_cnt || 0xc000;
        }
        else if (params.pulse_width_ns == 20)
        {
          pulse_width_hw_blank_cnt = pulse_width_hw_blank_cnt || 0x8000;
        }
        else if (params.pulse_width_ns == 15)
        {
          pulse_width_hw_blank_cnt = pulse_width_hw_blank_cnt || 0x4000;
        }
      }
    }
    if (exist(params.comp_le_delay))
    {
      if (params.comp_le_delay > -1)
      {
        comp_le_delay_cnt = int(params.comp_le_delay);
      }
    }
    if (exist(params.lrf_ch2_enable))
    {
      if ( (params.lrf_ch2_enable == 1) && (_have_lrf) )
      {
        _have_lrf = 2;
      }
    }
    if (exist(params.exec))
    {
      check_for_message = 1;
      wait_for_message  = "collected";
      commands_to_execute = params.exec;
    }
  }
  pri_10ns = 1./pri*1e8;
  if (pri_10ns > 65535)
  {
    pri_10ns = 65535;
  }
  //
  // request data streaming session
  //
  msg_type  = 0x811f;
  datalen = 17L;
  cmd = [0x5A, bytesplit(msg_type,0,"uint16"), 0x00, 0x00, bytesplit(datalen,0,"uint16")];
  cmd_chksum = (256L-sum(cmd)) && 0xff;
  // define configuration bytes:
  // channel configuration
  chan_conf = 0x0000;
  if (_have_lrf == 1)
  {
    chan_conf = chan_conf || 0x080e;  // report AC_THRESH, LRF_1[1], LRF_1[2], LRF_1[3]
  }
  else if (_have_lrf == 2)
  {
    chan_conf = chan_conf || 0x280e;  // report AC_THRESH, LRF_1[1], LRF_2[1], LRF_1[2], LRF_2[2], LRF_1[3], LRF_2[3]
  }
  if (_have_pir)
  {
    chan_conf = chan_conf || 0x00f0;  // report PIR[1,3] LO_GAIN and HI_GAIN
  }
  // define parameter bytes: always request number of samples
  n_samples = round(dur*pri);
  param_conf = 0x20;
  if (apd_bias_cnt >= 0)
  {
    param_conf = param_conf || 0x01;
  }
  if (afe_pot_cnt >= 0)
  {
    param_conf = param_conf || 0x02;
  }
  if (pulse_width_hw_blank_cnt >= 0)
  {
    param_conf = param_conf || 0x04;
  }
  if (comp_le_delay_cnt >= 0)
  {
    param_conf = param_conf || 0x08;
  }
  if (tdc_blanking_cnt >= 0)
  {
    param_conf = param_conf || 0x10;
  }
  // construct data packet:
  data = [ ...
      bytesplit(int(pri_10ns),0,"uint16"), ...
      bytesplit(n_samples,0,"float"), ...
      bytesplit(chan_conf,0,"uint16"), ...
      int(param_conf) && 0xff, ...
      bytesplit(apd_bias_cnt,0,"uint16"), ...
      int(afe_pot_cnt) && 0xff, ...
      bytesplit(pulse_width_hw_blank_cnt,0,"uint16"), ...
      bytesplit(comp_le_delay_cnt,0,"uint16"), ...
      int(tdc_blanking_cnt) && 0xff, ...
  []];
  data_chksum = (256L-(sum(data)&&0xff));
  if (LOGON)
  {
   writem(LOGFN,[cmd,cmd_chksum, data, data_chksum],<<format="%02x";eol="\n";csp=" ">>);
  }
  writem(DEVICE, [cmd, cmd_chksum, data, data_chksum]);
  "_collect_sfm_data: request for data sent\n"?
  //
  // get response: started sensor data collection
  //
  while (1)
  {
    //
    // check for incoming traffic
    // 
    t = readm(DEVICE,1);
    if (t != 0x5a)
    {
      continue;
    }
    while (t.n < HEADER_LEN)
    {
      t = [t,readm(DEVICE,HEADER_LEN-t.n)];
    }
    msg_type = bytejoin(t[2:3]);
    datalen = bytejoin(t[HEADER_LEN-2:HEADER_LEN-1]);
    d = [];
    if (datalen > 0)
    {
      d = readm(DEVICE,datalen+1);
    }
    //
    // print out debug messages if any:
    // 
    if (msg_type == 0x811d)
    {
      d_s = char(d);
      chomp(d_s);
      printf("_collect_sfm_data: %s\n", d_s);
      continue;
    }
    //
    // exit loop if "Started Sensor Data Collection"
    //
    if (msg_type == 0x8121)
    {
      printf("_collect_sfm_data: Started Sensor Data Collection\n");
      break;
    }
  }

  //
  // get response: live sensor data collection
  //
  ncols = 4;
  if(_have_lrf == 1)
  {
    ncols = ncols + 14;
  }
  else if (_have_lrf == 2)
  {
    ncols = ncols + 26;
  }
  if (_have_pir)
  {
    ncols = ncols + 8;
  }
  data = zeros(1,n_samples*ncols);
  idx_d  = 1;
  while (1)
  {
    spinner();
    // get packet header
    t = readm(DEVICE,HEADER_LEN);
    while (t.n < HEADER_LEN)
    {
      t = [t, readm(DEVICE,HEADER_LEN-t.n)];
    }

    if (isempty(t))
    {
      printf(_this_function + ": Warning: No data received: Check installation!\n");
      break;
    }
    // check if datalen=0 and the msg_type=0x8122
    msg_type = bytejoin(t[2:3]);
    datalen = bytejoin(t[HEADER_LEN-2:HEADER_LEN-1]);
    if ( (datalen==0) && (t[2]==0x22) && (t[3]==0x81) )
    {
      printf("_collect_sfm_data: Stopped Sensor Data Collection\n");
      break; // no more data
    }

    // it is live sensor packet
    d = readm(DEVICE,datalen+1);
    while (d.n < datalen+1)
    {
      d = [d, readm(DEVICE,datalen+1-d.n)];
    }

    if (msg_type == 0x811d)
    {
      d_s = char(d[1:datalen]);
      chomp(d_s);
      printf("_collect_sfm_data: %s\n", d_s);
      if (check_for_message)
      {
        if (!isempty(grep(d_s, wait_for_message)))
        {
          commands_to_execute();
          printf("_collect_sfm_data: command executed\n");
        }
      }

      continue;
    }

    // process live sensor packet
    chan1_conf = bytejoin(d[1:2]);
    n1_samples = bytejoin(d[3:6],0,"uint32");
    d1 = d[7:(datalen-2)];
    data[idx_d:(idx_d + n1_samples*ncols-1)] = d1 + 0.0;
    idx_d = idx_d + n1_samples*ncols;
  }

  rval = <<>>;
  _reshape(data,ncols,data.n/ncols);
  data = data';
  t_s = zeros(data.nr,1);
  if (_have_lrf)
  {
    ac_thresh_v = zeros(data.nr,1);
    if (_have_lrf == 2)
    {
      lrf_m = zeros(data.nr,3);
      tp_ns = zeros(data.nr,3);
      for (i in 1:data.nr)
      {
        t_s[i;] =  1e-6 .* bytejoin(data[i;1:4],0,"uint32");
        _rr = bytejoin(data[i;5:28],0,"float"); // lrf_m[i;] = bytejoin(data[i;5:16],0,"float");
        //lrf_m[i;] = [_rr[1,3,5], 2 .* (_rr[1,3,5] - _rr[2,4,6]) ./ 0.3 ]; // LRF[1:3] meters, DT[1:3] nanoseconds
        lrf_m[i;] = _rr[1,3,5]; // LRF[1:3] meters, DT[1:3] nanoseconds
        tp_ns[i;] = 2 .* (_rr[1,3,5] - _rr[2,4,6]) ./ 0.3;
        ac_thresh_v[i;] = 5.12 .* bytejoin(data[i;29:30],0,"int16") ./ 32767; // ac_thresh_v[i;] = 5.12 .* bytejoin(data[i;17:18],0,"int16") ./ 32767;
      }
      rval.tp_ns = tp_ns;
    }
    else
    {
      lrf_m = zeros(data.nr,3);
      for (i in 1:data.nr)
      {
        t_s[i;] =  1e-6 .* bytejoin(data[i;1:4],0,"uint32");
        lrf_m[i;] = bytejoin(data[i;5:16],0,"float");
        ac_thresh_v[i;] = 5.12 .* bytejoin(data[i;17:18],0,"int16") ./ 32767;
      }
    }
    rval.lrf_m = lrf_m;
    rval.ac_thresh_v = ac_thresh_v;
  }
  else if (_have_pir)
  {
    pir_v = zeros(data.nr,4);
    for (i in 1:data.nr)
    {
      t_s[i;] =  1e-6 .* bytejoin(data[i;1:4],0,"uint32");
      _rr = bytejoin(data[i;5:12],0,"int16");
      pir_v[i;] = [_rr[1,3], _rr[2,4]] .* 5.12 ./ 32767; // hi[1], hi[2], lo[1], lo[2]
    }
    rval.pir_v = pir_v;
  }
  rval.chan_conf = chan1_conf;
  rval.t_s = t_s;
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
  msg_type  = 0x8127;
  datalen   = 6L;
  cmd = [0x5A, bytesplit(msg_type,0,"uint16"), 0x00, 0x00, bytesplit(datalen,0,"uint16")];
  cmd_chksum = (256L-sum(cmd)) && 0xff;
  data = [bytesplit(fpga_addr, 0, "uint32"), bytesplit(fpga_val, 0, "uint16")];
  data_chksum = (256L-sum(data)) && 0xff;
  _d = [cmd, cmd_chksum, data, data_chksum];
  writem(DEVICE, _d);
  if (wait_time)
  {
    sleep(wait_time);
  }
  //
  t = readm(DEVICE,HEADER_LEN);
  while (length(t) < HEADER_LEN)
  {
    t = [t,readm(DEVICE,HEADER_LEN-t.n)];
  }
  //
  rval = -1;
  if (length(t)==HEADER_LEN)
  {
    datalen = bytejoin(t[HEADER_LEN-2:HEADER_LEN-1]);
    d = readm(DEVICE,2);
    if (d[1]==0x1f)
    {
      rval = fpga_val;
    }
  }
  if (LOGON)
  {
    writem(LOGFN, _d,<<format="%02x";eol="\n";csp=" ">>);
    writem(LOGFN,  t,<<format="%02x";eol="\n";csp=" ">>);
  }
  return rval;
};

_read_from_fpga = function(DEVICE, fpga_addr, wait_time)
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
  msg_type  = 0x8125;
  datalen   = 4L;
  cmd = [0x5A, bytesplit(msg_type,0,"uint16"), 0x00, 0x00, bytesplit(datalen,0,"uint16")];
  cmd_chksum = (256L-sum(cmd)) && 0xff;
  data = bytesplit(fpga_addr, 0, "uint32");
  data_chksum = (256L-sum(data)) && 0xff;
  _d = [cmd, cmd_chksum, data, data_chksum];
  writem(DEVICE, _d);
  //
  if (wait_time)
  {
    sleep(wait_time);
  }
  //
  t = readm(DEVICE,HEADER_LEN);
  while (length(t) < HEADER_LEN)
  {
    t = [t,readm(DEVICE,HEADER_LEN-t.n)];
  }
  rval = -1;
  if (length(t)==HEADER_LEN)
  {
    d = readm(DEVICE,3);
    rval = bytejoin(d[1:2]);
  }
  if (LOGON)
  {
    writem(LOGFN,_d,<<format="%02x";eol="\n";csp=" ">>);
    writem(LOGFN, t,<<format="%02x";eol="\n";csp=" ">>);
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

xm250class = classdef(dev)
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
  public(device, diary);
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
  public (get_sw_sn, get_sn, get_fpga_sample, set_tec);
  get_sw_sn = function()
  {
    d = _sfm_command(_device, 0x8101, [], 1);
    return char(d);
  };
  get_sn = function()
  {
    "get_sn: Not implemented yet\n"?
    return <<>>;
    d = _sfm_command(_device, 0x8107, [], 1);
    rval = <<>>;
    if (length(d) == 6)
    {
      rval.power_board_id = bytejoin(d[1,2],0, "uint16");
      rval.proc_board_id  = bytejoin(d[3,4],0, "uint16");
      rval.esad_board_id  = bytejoin(d[5,6],0, "uint16");
    }
    return rval;
  };
  get_fpga_sample = function()
  {
    d = _sfm_command(_device, 0x8123, [], 1);
    if (length(d) < 64)
    {
      printf("get_fpga_sample: 64 bytes expected but %g bytes received\n", length(d));
      return <<>>;
    }
    d = [bytejoin(d[1:16],0,"uint16"),bytejoin(d[27:64],0,"uint16")];
    rval = <<>>;
    rval.raw = d;
    // pir:
    pir = <<>>;
    pir.gain_hi = <<>>;
    pir.gain_hi.cnt = [d[1],0L,d[3]];
    pir.gain_hi.val = 5.12 ./ 32767 .* pir.gain_hi.cnt;
    pir.gain_lo = <<>>;
    pir.gain_lo.cnt = [d[2],0L,d[4]];
    pir.gain_lo.val = 5.12 ./ 32767 .* pir.gain_lo.cnt;
    // temps:
    temp = <<>>;
    temp.board = <<>>;
    temp.board.cnt = d[5];
    temp.board.val = 5.12 .* d[5] ./ 32767;
    temp.air   = <<>>;
    temp.air.cnt   = d[6];
    temp.air.val   = 5.12 .* d[6] ./ 32767;
    // bias 8.2V
    det_bias_8_2v = <<>>;
    det_bias_8_2v.cnt = d[7];
    det_bias_8_2v.val = d[7] .* 10.24 ./ 32767;
    // ac threshold for lrf
    ac_thresh = <<>>;
    ac_thresh.cnt = d[8];
    ac_thresh.val = 5.12 .* d[8] ./ 32767;
    // tdc:
    tdc = <<>>;
    for (i in 1:3)
    {
      tdc.[i]      = <<>>;
      tdc.[i].tof  = <<>>;
      tdc.[i].cal1 = <<>>;
      tdc.[i].cal2 = <<>>;
    }
    tdc.[1].tof.cnt  = d[9,10];
    tdc.[1].cal1.cnt = d[15,16];
    tdc.[1].cal2.cnt = d[21,22];
    // tdc2:
    tdc.[2].tof.cnt  = d[11,12];
    tdc.[2].cal1.cnt = d[17,18];
    tdc.[2].cal2.cnt = d[23,24];
    // tdc3:
    tdc.[3].tof.cnt  = d[13,14];
    tdc.[3].cal1.cnt = d[19,20];
    tdc.[3].cal2.cnt = d[25,26];
    //
    for (i in 1:3)
    {
      tdc.[i].t        = tdc.[i].tof.cnt ./ (tdc.[i].cal2.cnt -  tdc.[i].cal1.cnt)  .* 900e-9;  // actual return time
      tdc.[i].m        = 149896229 .* tdc.[i].t;                                                // range equivalent of 1/2 return time
    }
    //
    rval.tdc = tdc;
    rval.pir = pir;
    rval.temp = temp;
    rval.det_bias_8_2v = det_bias_8_2v;
    rval.ac_thresh = ac_thresh;
    return rval;
  };
  public(set_apd_bias, set_afe_pot, set_tpg_offset, reset_fpga_cntl_reg, set_blanking_period, set_comp_le_offset,...
      write_to_fpga);
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
    rval = _write_to_fpga(_device, fpga_addr, fpga_val, wait_time);
    return rval;
  };
  set_afe_pot = function(val)
  {
    _this_function = "set_afe_pot";
    cnt = _convert_afe_pot_p_to_cnt(val);
    _sfm_command(_device, 0x812B, cnt, 0);
    return 0;
  };
  set_apd_bias = function(val)
  {
    _this_function = "set_apd_bias";
    cnt = _convert_apd_bias_v_to_cnt(val);
    data = bytesplit(int(cnt),0,"uint16");
    _sfm_command(_device, 0x8129, data, 0);
    return 0;
  };
  public(collect_range_data, collect_pir_data, collect_data,mission, emmc);
  collect_range_data = function(pri,dur,params)
  {
    rval = _collect_sfm_data (_device, 1, pri, dur, params);
    return rval;
  };
  collect_pir_data = function(pri,dur,nwt)
  {
    rval = _collect_sfm_data (_device, 2, pri, dur, params);
    return rval;
  };
  collect_data = function(pri,dur,nwt)
  {
    rval = _collect_sfm_data (_device, _tosfm, 3, pri, dur, nwt);
    return rval;
  };
};



















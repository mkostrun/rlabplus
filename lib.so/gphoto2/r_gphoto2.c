//
// rgphoto2.c:  rlab's interface to GPhoto2 library
// Marijan Kostrun, VII-2013
//
// This file is a part of RLaB + rlabplus
// Copyright (C) 2013  Marijan Kostrun
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
// See the file ../COPYING

#include "rlab.h"
#include "ent.h"
#include "class.h"
#include "symbol.h"
#include "mem.h"
#include "mdr.h"
#include "mdrf1.h"
#include "mds.h"
#include "list.h"
#include "btree.h"
#include "bltin.h"
#include "util.h"
#include "mathl.h"
#include "function.h"
#include "lp.h"

#include <stdio.h>
#include <fcntl.h>

//
#include "rlab_solver_parameters_names.h"
#include "r_gphoto2.h"

#define THIS_LIBRARY "gphoto2"


// local variables to keep track of the open camera
static Camera *camera=0;
static CameraAbilitiesList *al=0;
static CameraAbilities abilities;
static GPPortInfo info;
static GPPortInfoList *il=0;
GPContext *context=0;

static void GP_CHECK(int f)
{
  int res = f;
  if (res < 0)
    printf ("ERROR: %s\n", gp_result_as_string (res));
  return;
};

static int count_bits_in_byte(unsigned char * x)
{
  unsigned char i = 1;
  int s = i & (*x);
  while (i != 0x80)
  {
    i = i<<1;
    if ((i & (*x)) == i)
      s++;
  }
  return s;
};

#undef  THIS_SOLVER
#define THIS_SOLVER RLAB_NAME_EMBED_GPHOTO2 ".capture"
//
// initialize camera using gphoto2 library
//
Ent *
ent_gp_camera_capture (int nargs, Datum args[])
{
  Ent *e1=0, *rent;
  char *filename=0;
  int fd, rval=1;
  CameraFile *file;
  CameraFilePath camera_file_path;
  CameraEventType type;
  void *data;
  int waittime = 2000;

  if (nargs > 1)
  {
    fprintf (stdout,
             THIS_SOLVER ": gphoto2 wrapper library for RLaB\n");
    fprintf (stdout,
             THIS_SOLVER ": Capture an image on a initialized camera\n");
    fprintf (stdout,
             THIS_SOLVER ": Format:\n");
    fprintf (stdout,
             THIS_SOLVER ":   " THIS_SOLVER "(/filename/),\n");
    fprintf (stdout,
             THIS_SOLVER ": where\n");
    fprintf (stdout,
             THIS_SOLVER ":   'filename' is the file where the picture is downloaded.\n");
    fprintf (stdout,
             THIS_SOLVER ": See manual for details.\n");
    rerror ( THIS_SOLVER ": requires none or 1 argument");
  }

  if (nargs == 1)
  {
    e1 = bltin_get_ent (args[0]);
    if (ent_type(e1) != MATRIX_DENSE_STRING)
      rerror(THIS_SOLVER ": First argument 'filename' has to be string!");

    filename  = class_char_pointer(e1);
    if (!filename)
      rerror(THIS_SOLVER ": First argument 'filename' has to be string!");
  }


  //
  rent = ent_Create ();

  //
  // generic load of the library
  //
  if (!camera)
  {
    ent_data (rent) = mdi_CreateScalar(rval);
    ent_type (rent) = MATRIX_DENSE_REAL;
    return (rent);
  }
  if (!context)
    GP_CHECK (gp_camera_init (camera, context));

  GP_CHECK (gp_camera_capture(camera, GP_CAPTURE_IMAGE, &camera_file_path, context));

  while (1)
  {
    GP_CHECK (gp_camera_wait_for_event(camera, waittime, &type, &data, context));
    if (type == GP_EVENT_TIMEOUT)
      break;
    else if (type == GP_EVENT_CAPTURE_COMPLETE)
    {
      waittime = 100;
    }
  }

  rval = 0;
  if (filename)
  {
    // open file on local filesystem
    fd = open(filename, O_CREAT | O_WRONLY, 0644);
    // pass its stream to gphoto2
    GP_CHECK (gp_file_new_from_fd(&file, fd));
    // transfer file from camera to local file system
    GP_CHECK (gp_camera_file_get(camera, camera_file_path.folder, camera_file_path.name,
                                 GP_FILE_TYPE_NORMAL, file, context));
    // delete local file on camera
    GP_CHECK (gp_camera_file_delete(camera, camera_file_path.folder, camera_file_path.name,context));
    // liberate resources
    gp_file_free(file);
  }


  // clean up rlab
  ent_Clean(e1);

  ent_data (rent) = mdr_CreateScalar(rval);
  ent_type (rent) = MATRIX_DENSE_REAL;
  return (rent);
}

//
//
//
static void rlab_gp_widget_iterator(CameraWidget *xw, char **conf_names, int *conf_name_count)
{
  CameraWidget *xw_child=0;
  CameraWidgetType wtype;
  int j, d1;
  char *gp_value_string=0;

  GP_CHECK (gp_widget_get_type(xw, &wtype));

  switch(wtype)
  {
    case GP_WIDGET_RADIO:
    case GP_WIDGET_MENU:
    case GP_WIDGET_TEXT:
    case GP_WIDGET_RANGE:
    case GP_WIDGET_TOGGLE:
    case GP_WIDGET_BUTTON:
    case GP_WIDGET_DATE:
      GP_CHECK (gp_widget_get_name(xw, (const char **) &gp_value_string));
      conf_names[(*conf_name_count)++] = cpstr(gp_value_string);
      break;

    case GP_WIDGET_WINDOW:
    case GP_WIDGET_SECTION:
      d1 = gp_widget_count_children (xw);
      if (d1 > 0)
      {
        for (j=0; j<d1; j++)
        {
          GP_CHECK (gp_widget_get_child (xw, j, &xw_child));
          rlab_gp_widget_iterator(xw_child, conf_names, conf_name_count);
        }
      }
      break;

    default:
      break;
  }

  return;
}

//
// get options and values/choices of the initialized camera
//
#undef THIS_SOLVER
#define THIS_SOLVER RLAB_NAME_EMBED_GPHOTO2 ".config_options"
Ent * ent_gp_camera_config_options (int nargs, Datum args[])
{
  Ent *rent=ent_Create(), *rval=0;
  MDS *rs=0;
  MDR *rr=0;
  char *gp_value_string=0;

  CameraWidget *widgettree=0, *xwidget=0;
  CameraWidgetType wtype;
  int m, i, j;
  Btree *b = btree_Create();

  //
  // generic load of the library
  //
  if ((!camera) || (!context))
    goto exit;

  GP_CHECK (gp_camera_get_config(camera, &widgettree, context));

  char *conf_names[500]={NULL};
  int    conf_name_count=0;
  int    value_int;
  float  value_float_min,value_float_max,value_float_inc;

  rlab_gp_widget_iterator(widgettree, conf_names, &conf_name_count);

  for (j=0; j<conf_name_count; j++)
  {
    GP_CHECK (gp_widget_get_child_by_name (widgettree, conf_names[j], &xwidget));
    GP_CHECK (gp_widget_get_type(xwidget, &wtype));
    switch (wtype)
    {
      case GP_WIDGET_RADIO:
      case GP_WIDGET_MENU:
      case GP_WIDGET_TEXT:
        m = gp_widget_count_choices(xwidget);
        if (m>0)
        {
          rs = mds_Create(1,m);
          for (i=0; i<m; i++)
          {
            GP_CHECK (gp_widget_get_choice(xwidget,i, (const char **) &gp_value_string));
            MdsV0(rs, i) = cpstr(gp_value_string);
          }
        }
        else
        {
          GP_CHECK (gp_widget_get_value(xwidget,(const char **) &gp_value_string));
          rs = mds_CreateScalar(gp_value_string);
        }
        rval = ent_Assign_Rlab_MDS(rs);
        break;

      case GP_WIDGET_RANGE:
        rr = mdr_Create(1,3);
        GP_CHECK (gp_widget_get_range(xwidget,
                  &value_float_min,&value_float_max,&value_float_inc));
        MdrV0(rr,0) = value_float_min;
        MdrV0(rr,1) = value_float_max;
        MdrV0(rr,2) = value_float_inc;
        rval = ent_Assign_Rlab_MDR(rr);
        break;

      case GP_WIDGET_DATE:
      case GP_WIDGET_TOGGLE:
        GP_CHECK (gp_widget_get_value(xwidget,&value_int));
        rr = mdi_CreateScalar(value_int);
        rval = ent_Assign_Rlab_MDR(rr);
        break;

      default:
        rr = mdr_Create(0,0);
        rval = ent_Assign_Rlab_MDR(rr);
        break;
    }
    install (b, conf_names[j], rval);
    if (conf_names[j])
      GC_FREE(conf_names[j]);
  }

exit:

  ent_data (rent) = b;
  ent_type (rent) = BTREE;
  return (rent);
}


//
// initialize camera using gphoto2 library
//
#undef THIS_SOLVER
#define THIS_SOLVER RLAB_NAME_EMBED_GPHOTO2 ".config"
Ent * ent_gp_camera_config (int nargs, Datum args[])
{
  Ent *e1=0, *e2=0, *rent;
  MDS *rs=0;
  MDR *rr=0;
  char *feature=0;

  CameraWidget *widgettree=0, *xwidget=0;
  int ic=-1, m, rval=1, i;

  if (nargs != 1 && nargs != 2)
  {
    fprintf (stdout,
             THIS_SOLVER ": gphoto2 wrapper library for RLaB\n");
    fprintf (stdout,
             THIS_SOLVER ": Check the features of the provided camera model.\n");
    fprintf (stdout,
             THIS_SOLVER ": Format:\n");
    fprintf (stdout,
             THIS_SOLVER ":   " THIS_SOLVER "(feature,/val/),\n");
    fprintf (stdout,
             THIS_SOLVER ": where\n");
    fprintf (stdout,
             THIS_SOLVER ":   'feature' is a unique gphoto2-recognized name of the feature of the camera.\n");
    fprintf (stdout,
             THIS_SOLVER ":   while 'val', if provided, is the new value for that feature.\n");
    fprintf (stdout,
             THIS_SOLVER ": See manual for details.\n");
    rerror ( THIS_SOLVER ": requires 1 or 2 arguments");
  }

  e1 = bltin_get_ent (args[0]);
  if (ent_type(e1) != MATRIX_DENSE_STRING)
    rerror(THIS_SOLVER ": First argument 'camera' has to be string!");

  feature  = class_char_pointer(e1);
  if (!feature)
    rerror(THIS_SOLVER ": First argument 'camera' has to be string!");

  if (nargs==2)
  {
    e2 = bltin_get_ent (args[1]);
    if (  (ent_type(e2) != MATRIX_DENSE_STRING)
      &&  (ent_type(e2) != MATRIX_DENSE_REAL) )
      rerror(THIS_SOLVER ": Second argument has to be real or string!");
  }

  //
  rent = ent_Create ();

  //
  // generic load of the library
  //
  if (!camera)
  {
    ent_data (rent) = mdr_Create(0,0);
    ent_type (rent) = MATRIX_DENSE_REAL;
    return (rent);
  }
  if (!context)
    GP_CHECK (gp_camera_init (camera, context));

  GP_CHECK (gp_camera_get_config(camera, &widgettree, context));
  GP_CHECK (gp_widget_get_child_by_name (widgettree, feature, &xwidget));

  char  *value_string=0;
  const char  *gp_value_string=0;
  int    value_int;
  float  value_float_min,value_float_max,value_float_inc;

  if (xwidget)
  {
    CameraWidgetType wtype;
    GP_CHECK (gp_widget_get_type(xwidget, &wtype));

    switch (wtype)
    {
      case GP_WIDGET_RADIO:
      case GP_WIDGET_MENU:
      case GP_WIDGET_TEXT:
        if (e2)
        {
          if (ent_type(e2) == MATRIX_DENSE_REAL)
          {
            ic = (int) class_double(e2);
            m = gp_widget_count_choices(xwidget);
            if (ic <= m && ic >= 1)
            {
              GP_CHECK (gp_widget_get_choice(xwidget,ic-1,&gp_value_string));
              GP_CHECK (gp_widget_set_value (xwidget,gp_value_string));
              GP_CHECK (gp_widget_set_changed (xwidget,1));
              GP_CHECK (gp_camera_set_config (camera, widgettree, context));
              rval = 0;
            }
          }
          else if (ent_type(e2) == MATRIX_DENSE_STRING)
          {
            value_string = class_char_pointer(e2);
            m = gp_widget_count_choices(xwidget);
            for (i=0; i<m; i++)
            {
              GP_CHECK (gp_widget_get_choice(xwidget,i,&gp_value_string));
              if (!strcmp(gp_value_string, value_string))
              {
                GP_CHECK (gp_widget_set_value (xwidget,value_string));
                GP_CHECK (gp_widget_set_changed (xwidget,1));
                GP_CHECK (gp_camera_set_config (camera, widgettree, context));
                rval = 0;
                break;
              }
            }
          }
          else
            rerror(THIS_SOLVER ": Second argument is out of range!\n");

          //
          rr = mdr_CreateScalar(rval);
          ent_data (rent) = rr;
          ent_type (rent) = MATRIX_DENSE_REAL;
          goto feature_exit;
        }
        else
        {
          GP_CHECK (gp_widget_get_value(xwidget,&value_string));
          rs = mds_CreateScalar((value_string));
          ent_data (rent) = rs;
          ent_type (rent) = MATRIX_DENSE_STRING;
        }
        break;

      case GP_WIDGET_RANGE:
        if (e2)
        {
          if (ent_type(e2) == MATRIX_DENSE_REAL)
          {
            rr = ent_data(e2);
            if (rr->nrow * rr->ncol == 3)
            {
              value_float_min = MdrV0(rr,0);
              value_float_max = MdrV0(rr,1);
              value_float_inc = MdrV0(rr,2);
              GP_CHECK (gp_widget_set_range(xwidget,
                                            value_float_min,value_float_max,value_float_inc));
              GP_CHECK (gp_widget_set_changed (xwidget,1));
              GP_CHECK (gp_camera_set_config (camera, widgettree, context));
              rval = 0;
            }

            //
            rr = mdr_CreateScalar(rval);
            ent_data (rent) = rr;
            ent_type (rent) = MATRIX_DENSE_REAL;
            goto feature_exit;
          }
        }
        else
        {
          rr = mdr_Create(1,3);
          GP_CHECK (gp_widget_get_range(xwidget,
                                        &value_float_min,&value_float_max,&value_float_inc));

          MdrV0(rr,0) = value_float_min;
          MdrV0(rr,1) = value_float_max;
          MdrV0(rr,2) = value_float_inc;
          ent_data (rent) = rr;
          ent_type (rent) = MATRIX_DENSE_REAL;
        }
        break;

      case GP_WIDGET_DATE:
      case GP_WIDGET_TOGGLE:
        if (e2)
        {
          if (ent_type(e2) == MATRIX_DENSE_REAL)
          {
            ic = (int) class_double(e2);
            GP_CHECK (gp_widget_set_value (xwidget,&ic));
            GP_CHECK (gp_widget_set_changed (xwidget,1));
            GP_CHECK (gp_camera_set_config (camera, widgettree, context));
            rval = 0;
          }
          //
          rr = mdr_CreateScalar(rval);
          ent_data (rent) = rr;
          ent_type (rent) = MATRIX_DENSE_REAL;
          goto feature_exit;
        }
        else
        {
          GP_CHECK (gp_widget_get_value(xwidget,&value_int));
          rr = mdi_CreateScalar(value_int);
          ent_data (rent) = rr;
          ent_type (rent) = MATRIX_DENSE_REAL;
        }
        break;

      default:
        if (e2)
        {
          rval = 2;
          rr = mdr_CreateScalar(rval);
          ent_data (rent) = rr;
          ent_type (rent) = MATRIX_DENSE_REAL;
          goto feature_exit;
        }
        else
        {
          rr = mdr_Create(0,0);
          ent_data (rent) = rr;
          ent_type (rent) = MATRIX_DENSE_REAL;
        }
        break;
    }
  }
  else
  {
    rr = mdr_Create(0,0);
    ent_data (rent) = rr;
    ent_type (rent) = MATRIX_DENSE_REAL;
  }

feature_exit:

  // clean up rlab
  ent_Clean(e1);
  ent_Clean(e2);

  return (rent);
}


//
// get info about the level of support of a camera supported by the library
//
#undef THIS_SOLVER
#define THIS_SOLVER RLAB_NAME_EMBED_GPHOTO2 ".info"
Ent *
ent_gp_camera_info (int nargs, Datum args[])
{
  MDS *cam_ops=0, *cam_file_ops=0, *cam_folder_ops=0, *cam_status=0;

  CameraText info_text;

  unsigned char x;

  int m, j;

  if (nargs)
  {
    fprintf (stdout,
             THIS_SOLVER ": gphoto2 wrapper library for RLaB\n");
    fprintf (stdout,
             THIS_SOLVER ": Check the features of the provided camera model.\n");
    fprintf (stdout,
             THIS_SOLVER ": Format:\n");
    fprintf (stdout,
             THIS_SOLVER ":   " THIS_SOLVER "().\n");
    rerror ( THIS_SOLVER ": requires no arguments");
  }

  //
  // check if library is loaded
  //
  if (!camera)
    rerror(THIS_SOLVER ": Camera needs to be initialized first!");


  //
  // figure out what operations are possible
  //
  x = abilities.operations;
  m = count_bits_in_byte( &x );
  if (m)
  {
    cam_ops = mds_Create(1,m);
    j = 0;
    if ((x & GP_OPERATION_NONE) != 0)
      MdsV0(cam_ops,j++) = cpstr("none");
    if ((x & GP_OPERATION_CAPTURE_IMAGE) !=  0)
      MdsV0(cam_ops,j++) = cpstr("image");
    if ((x & GP_OPERATION_CAPTURE_VIDEO) != 0)
      MdsV0(cam_ops,j++) = cpstr("video");
    if ((x & GP_OPERATION_CAPTURE_AUDIO) != 0)
      MdsV0(cam_ops,j++) = cpstr("audio");
    if ((x & GP_OPERATION_CAPTURE_PREVIEW) != 0)
      MdsV0(cam_ops,j++) = cpstr("preview");
    if ((x & GP_OPERATION_CONFIG) != 0)
      MdsV0(cam_ops,j++) = cpstr("config");
  }
  else
    cam_ops = mds_CreateScalar(("none"));

  //
  // file operations
  //
  x = abilities.file_operations;
  m = count_bits_in_byte( &x );
  if (m)
  {
    cam_file_ops = mds_Create(1,m);
    j = 0;
    if ((x & GP_FILE_OPERATION_NONE) != 0)
      MdsV0(cam_file_ops,j++) = cpstr("none");
    if ((x & GP_FILE_OPERATION_DELETE) !=  0)
      MdsV0(cam_file_ops,j++) = cpstr("delete");
    if ((x & GP_FILE_OPERATION_PREVIEW) != 0)
      MdsV0(cam_file_ops,j++) = cpstr("preview");
    if ((x & GP_FILE_OPERATION_RAW) != 0)
      MdsV0(cam_file_ops,j++) = cpstr("raw");
    if ((x & GP_FILE_OPERATION_AUDIO) != 0)
      MdsV0(cam_file_ops,j++) = cpstr("audio");
    if ((x & GP_FILE_OPERATION_EXIF) != 0)
      MdsV0(cam_file_ops,j++) = cpstr("exif");
  }
  else
    cam_file_ops = mds_CreateScalar(("none"));

  //
  // folder operations
  //
  x = abilities.folder_operations;
  m = count_bits_in_byte( &x );
  if (m)
  {
    cam_folder_ops = mds_Create(1,m);
    j = 0;
    if ((x & GP_FOLDER_OPERATION_DELETE_ALL) != 0)
      MdsV0(cam_folder_ops,j++) = cpstr("delete_all");
    if ((x & GP_FOLDER_OPERATION_PUT_FILE) !=  0)
      MdsV0(cam_folder_ops,j++) = cpstr("put_file");
    if ((x & GP_FOLDER_OPERATION_MAKE_DIR) != 0)
      MdsV0(cam_folder_ops,j++) = cpstr("make_dir");
    if ((x & GP_FOLDER_OPERATION_REMOVE_DIR) != 0)
      MdsV0(cam_folder_ops,j++) = cpstr("remove_dir");
  }
  else
    cam_folder_ops = mds_CreateScalar(("none"));

  //
  // status byte
  //
  x = abilities.status;
  if (x == GP_DRIVER_STATUS_PRODUCTION)
    cam_status = mds_CreateScalar(("production"));
  else if (x == GP_DRIVER_STATUS_TESTING)
    cam_status = mds_CreateScalar(("testing"));
  else if (x == GP_DRIVER_STATUS_EXPERIMENTAL)
    cam_status = mds_CreateScalar(("experimental"));
  else if (x == GP_DRIVER_STATUS_DEPRECATED)
    cam_status = mds_CreateScalar(("deprecated"));
  else
    cam_status = mds_CreateScalar(("what?"));

  GP_CHECK (gp_camera_set_abilities (camera, abilities));

  //
  // create output list
  //
  Btree * bw = btree_Create ();
  install (bw, RLAB_NAME_GPHOTO2_INFO_CAM_OPS, ent_Assign_Rlab_MDS(cam_ops));
  install (bw, RLAB_NAME_GPHOTO2_INFO_FILE_OPS, ent_Assign_Rlab_MDS(cam_file_ops));
  install (bw, RLAB_NAME_GPHOTO2_INFO_FOLDER_OPS, ent_Assign_Rlab_MDS(cam_folder_ops));
  install (bw, RLAB_NAME_GPHOTO2_INFO_STATUS, ent_Assign_Rlab_MDS(cam_status));
  //
  gp_camera_get_summary (camera, &info_text, context);
  install (bw, RLAB_NAME_GPHOTO2_INFO_SUMMARY, ent_Create_Rlab_String(info_text.text));

  return  ent_Assign_Rlab_BTREE(bw);
}


//
// get the list of cameras supported by the library
//
Ent *
ent_gp_info_supported_cameras (int nargs, Datum args[])
{
  Ent *rent;
  MDS *s=0;

  int m, i;

  //
  // generic load of the library
  //
  if (!camera)
    GP_CHECK (gp_camera_new (&camera));

  GP_CHECK (gp_port_info_list_new (&il));
  GP_CHECK (gp_port_info_list_load (il));
  GP_CHECK (i = gp_port_info_list_lookup_path (il, "usb:"));
  GP_CHECK (gp_port_info_list_get_info (il, i, &info));
  GP_CHECK (gp_camera_set_port_info(camera, info));
  GP_CHECK (gp_abilities_list_new (&al));
  GP_CHECK (gp_abilities_list_load (al, NULL));

  m = gp_abilities_list_count(al);
  s = mds_Create(m,1);
  for (i=0; i<m; i++)
  {
    gp_abilities_list_get_abilities (al, i, &abilities);
    MdsV0(s,i) = cpstr(abilities.model);
  }

  // clean up gphoto
  if (il)
  {
    GP_CHECK (gp_port_info_list_free (il));
    il=0;
  }
  if (al)
  {
    GP_CHECK (gp_abilities_list_free (al));
    al=0;
  }

  if (context)
    gp_context_unref(context);

  if (camera)
  {
    GP_CHECK (gp_camera_exit(camera,context));
    camera = 0;
  }


  rent = ent_Create ();
  ent_data (rent) = s;
  ent_type (rent) = MATRIX_DENSE_STRING;
  return (rent);
}


//
// initialize camera using gphoto2 library
//
#undef  THIS_SOLVER
#define THIS_SOLVER RLAB_NAME_EMBED_GPHOTO2 ".init"
Ent * ent_gp_camera_init (int nargs, Datum args[])
{
  Ent *e1=0, *rent;
  char *camera_model="USB PTP Class Camera", *s=0;

  int m, i, c, retval=1, idx_model;

  if (camera)
    goto go_out_now;

  if (nargs == 1)
  {
    e1 = bltin_get_ent (args[0]);
    if (ent_type(e1) == MATRIX_DENSE_STRING)
    {
      s = class_char_pointer(e1);
      if (isvalidstring(s))
        camera_model = s;
    }
  }

  //
  // generic load of the library
  //
  GP_CHECK (gp_camera_new (&camera));
  GP_CHECK (gp_port_info_list_new (&il));
  GP_CHECK (gp_port_info_list_load (il));
  GP_CHECK (i = gp_port_info_list_lookup_path (il, "usb:"));
  GP_CHECK (gp_port_info_list_get_info (il, i, &info));
  GP_CHECK (gp_camera_set_port_info(camera, info));
  GP_CHECK (gp_abilities_list_new (&al));
  GP_CHECK (gp_abilities_list_load (al, NULL));

  if (camera_model)
  {
    c = 0;
    m = gp_abilities_list_count(al);
    for (i=0; i<m; i++)
    {
      gp_abilities_list_get_abilities (al, i, &abilities);
      if (strstr(abilities.model, camera_model))
      {
        c++;
        idx_model = i;
      }
    }

    if (c==1)
    {
      GP_CHECK (gp_abilities_list_get_abilities (al, idx_model, &abilities));
      GP_CHECK (gp_camera_set_abilities (camera, abilities));

      if (camera)
      {
        retval = 0;
        context = gp_context_new();
        GP_CHECK (gp_camera_init (camera, context));
      }
    }
  }

  if (!camera || !context)
  {
    // clean up gphoto
    if (il)
    {
      GP_CHECK (gp_port_info_list_free (il));
      il=0;
    }
    else
      retval += 1;
    if (al)
    {
      GP_CHECK (gp_abilities_list_free (al));
      al=0;
    }
    else
      retval += 2;

    if (camera)
    {
      GP_CHECK (gp_camera_exit (camera,context));
      GP_CHECK (gp_camera_unref (camera));
      if (context)
        gp_context_unref (context);
      context = 0;
      camera = 0;
    }
    else
      retval += 4;
  }

  // clean up rlab
  ent_Clean(e1);

go_out_now:

  rent = ent_Create ();
  ent_data (rent) = mdr_CreateScalar ((double) retval);
  ent_type (rent) = MATRIX_DENSE_REAL;
  return (rent);
}

//
// initialize camera using gphoto2 library
//
Ent *
ent_gp_camera_exit (int nargs, Datum args[])
{
  Ent *rent;
  int retval=0;

  // clean up gphoto
  if (il)
  {
    GP_CHECK (gp_port_info_list_free (il));
    il=0;
  }
  else
    retval += 1;
  if (al)
  {
    GP_CHECK (gp_abilities_list_free (al));
    al=0;
  }
  else
    retval += 2;

  if (camera)
  {
    GP_CHECK (gp_camera_exit(camera,context));
    GP_CHECK (gp_camera_unref (camera));
    if (context)
    {
      gp_context_unref(context);
      context = 0;
    }
    camera = 0;
  }
  else
    retval += 4;

  rent = ent_Create ();
  ent_data (rent) = mdi_CreateScalar (retval);
  ent_type (rent) = MATRIX_DENSE_REAL;
  return (rent);
}


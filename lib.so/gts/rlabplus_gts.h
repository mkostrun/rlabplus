#ifndef _RLABPLUS_GTS_H_

#define _RLABPLUS_GTS_H_

#define RLAB_GTS_VERTEX       "v"
#define RLAB_GTS_EDGE         "e"
#define RLAB_GTS_FACE         "f"
#define RLAB_GTS_FACE_COLOR   "c"
#define RLAB_GTS_NR_EDGES     "nr_edge"
#define RLAB_GTS_NR_VERTICES  "nr_vertex"
#define RLAB_GTS_NR_FACES     "nr_face"
#define RLAB_GTS_MODIFY_OBJ   "modify_obj"
#define RLAB_GTS_STACK_PTS    "stack_pts"
#define RLAB_GTS_FACE_Q       "face_quality"
#define RLAB_GTS_FACE_AREA    "face_area"
#define RLAB_GTS_EDGE_LEN     "face_length"
#define RLAB_GTS_EDGE_ANGLE   "edge_angle"
#define RLAB_GTS_NR_DUP_EDGES "n_duplicate_edges"
#define RLAB_GTS_NR_DUP_FACES "n_duplicate_faces"
#define RLAB_GTS_NR_INC_FACES "n_incompatible_faces"
#define RLAB_GTS_NR_BND_EDGES "n_boundary_edges"
#define RLAB_GTS_NR_NON_MFLD_EDGES  "n_non_manifold_edges"

#define RLAB_GTS_NEW_SURFACE_DESCRIPTION  "desc"
#define RLAB_GTS_NEW_SURFACE_SPHERE_LEVEL "level"
#define RLAB_GTS_NEW_SURFACE_RECT_BBOX_X  "bbox_x"
#define RLAB_GTS_NEW_SURFACE_RECT_BBOX_Y  "bbox_y"
#define RLAB_GTS_NEW_SURFACE_RECT_BBOX_Z  "bbox_z"
#define RLAB_GTS_NEW_SURFACE_SCALE        "scale"
#define RLAB_GTS_NEW_SURFACE_CENTER       "center"
#define RLAB_GTS_NEW_SURFACE_TRANSL       "transl"
#define RLAB_GTS_NEW_SURFACE_ROT_X        "roll"
#define RLAB_GTS_NEW_SURFACE_ROT_Y        "pitch"
#define RLAB_GTS_NEW_SURFACE_ROT_Z        "yaw"
#define RLAB_GTS_NEW_SURFACE_METHOD       "method"
#define RLAB_GTS_NEW_SURFACE_COLOR        "color"
#define RLAB_GTS_NEW_SURFACE_THRESHOLD    "threshold"
#define RLAB_GTS_NEW_SURFACE_REMOVE_FACE  "remove_face"
#define RLAB_GTS_NEW_SURFACE_CART_FUNC          "f"
#define RLAB_GTS_NEW_SURFACE_CART_FUNC_PARAMS   "f_params"
#define RLAB_GTS_NEW_SURFACE_CART_MESH          "mesh"
#define RLAB_GTS_NEW_SURFACE_CART_X             "x"
#define RLAB_GTS_NEW_SURFACE_CART_Y             "y"
#define RLAB_GTS_NEW_SURFACE_CART_Z             "z"
#define RLAB_GTS_NEW_SURFACE_FORMAT             RLAB_NAME_WRITEM_FORMAT

#define RLAB_GTS_NEW_SURFACE_CART_NX            "nx"
#define RLAB_GTS_NEW_SURFACE_CART_NY            "ny"
#define RLAB_GTS_NEW_SURFACE_CART_XRANGE        "xrange"
#define RLAB_GTS_NEW_SURFACE_CART_YRANGE        "yrange"

#define RLAB_GTS_BOOLEAN_1_OUT_2          "1_OUT_2"
#define RLAB_GTS_BOOLEAN_2_OUT_1          "2_OUT_1"
#define RLAB_GTS_BOOLEAN_1_IN_2           "1_IN_2"
#define RLAB_GTS_BOOLEAN_2_IN_1           "2_IN_1"
#define RLAB_GTS_BOOLEAN_UNION            "union"
#define RLAB_GTS_BOOLEAN_INTERSECTION     "inter"
#define RLAB_GTS_BOOLEAN_DIFFERENCE       "diff"

#define RLAB_GTS_PARAM_RETURN_PT          "ret"
#define RLAB_GTS_PARAM_DIM                "dim"
//
// RLaB GTS Surface List
//

#include <libqhull_r/qhull_ra.h>

#define CONVHULL_3D_ENABLE
#include "convhull_3d.h"

struct _rlabplus_gts_surface
{
  char *name;
  GtsSurface *s;
  guint n_vertex;
  guint n_edge;
  guint n_face;
//  GtsFace *f;
//  GtsEdge *e;
//  GtsVertex *v;
  struct _rlabplus_gts_surface * next;
};

typedef struct _rlabplus_gts_surface r_gts_surface;

struct _rlabplus_gts_face_vx_count
{
  GtsSurface  *s;
  GtsVertex *vx1;
  GtsVertex *vx2;
  GtsVertex *vx3;
  guint count;
};
typedef struct _rlabplus_gts_face_vx_count r_gts_face_vx;

struct _rlabplus_gts_void_ptrs
{
  void *vptr1;
  void *vptr2;
  void *vptr3;
  void *vptr4;
};
typedef struct _rlabplus_gts_void_ptrs r_gts_void_ptrs;

//
// RLaB GTS functins list
//
struct _rlabplus_gts_func_args
{
  int  n_args;
  int  n_pts;
  int  modify_obj;
  int  stack_pts;
  Ent *fname;
  Ent *arg1;
  Ent *arg2;
  Ent *rval;
};
typedef struct _rlabplus_gts_func_args r_gts_func_args;


//
// RLaB GTS surface transform list
//
struct _rlabplus_gts_surface_transform_args
{
  MDR *rot_x;
  MDR *rot_y;
  MDR *rot_z;
  MDR *scale;
  MDR *transl;
  MDR *center;
  MDR *vertex;
  MDR *result;
};
typedef struct _rlabplus_gts_surface_transform_args r_gts_surface_transform;

//
// RLaB GTS functins list
//
struct _rlabplus_gts_func_vertex
{
  MDR *old_vx;
  MDR *new_vx;
  int *idx;
};
typedef struct _rlabplus_gts_func_vertex r_gts_func_vertex;

struct _rlabplus_gts_func_1arg
{
  void *edges;
  int *count;
};
typedef struct _rlabplus_gts_func_1arg r_gts_func_1arg;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
//
//
// RLAB GTS Face class definition: heavily based on GTS/examples/traverse.c
//
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
struct _ColorFace
{
  GtsFaceClass face;
  GtsColor c;
};
typedef struct _ColorFace         ColorFace;

struct _ColorFaceClass
{
  GtsFaceClass parent_class;
};
typedef struct _ColorFaceClass    ColorFaceClass;

#define COLOR_FACE(obj)         GTS_OBJECT_CAST (obj, ColorFace, color_face_class ())

#define COLOR_FACE_CLASS(klass) GTS_OBJECT_CLASS_CAST (klass, ColorFaceClass, color_face_class())

#define IS_COLOR_FACE(obj)     (gts_object_is_from_class (obj,color_face_class ()))

extern GtsColor GTS_DEFAULT_COLOR;

static GtsColor get_face_color (GtsObject * object)
{
  return ((GtsColor) COLOR_FACE (object)->c);
}

static void color_face_init (ColorFace * dface)
{
  //dface->c =  GTS_DEFAULT_COLOR;
  dface->c.r =  1;
  dface->c.g =  1;
  dface->c.b =  1;
  return;
}

static void color_face_class_init (ColorFaceClass * klass)
{
  /* overload color definition */
  GTS_OBJECT_CLASS (klass)->color = get_face_color;
}

ColorFaceClass * color_face_class (void)
{
  static ColorFaceClass * klass = NULL;
  if (klass == NULL)
  {
    GtsObjectClassInfo color_face_info =
    {
      "ColorFace",
      sizeof (ColorFace),
      sizeof (ColorFaceClass),
      (GtsObjectClassInitFunc) color_face_class_init,
      (GtsObjectInitFunc) color_face_init,
      (GtsArgSetFunc) NULL,
      (GtsArgGetFunc) NULL
    };
    klass = gts_object_class_new (GTS_OBJECT_CLASS (gts_face_class ()), &color_face_info);
  }
  return klass;
}







































#endif
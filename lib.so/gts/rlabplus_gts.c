// Copyright (C) 2003-2021 Marijan Kostrun
//   part of rlabplus for linux project on rlabplus.sourceforge.net
//
// gts wrapper for rlabplus
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
// See the file ./COPYING

//
// rlab headers
//
#include <rlab/rlab.h>
#include <rlab/mdr.h>
#include <rlab/mdc.h>
#include <rlab/mdr_mdc.h>
#include <rlab/mdcf1.h>
#include <rlab/mdcf2.h>
#include <rlab/complex.h>
#include <rlab/ent.h>
#include <rlab/symbol.h>
#include <rlab/class.h>
#include <rlab/mem.h>
#include <rlab/bltin.h>
#include <rlab/util.h>
#include <rlab/rlab_solver_parameters_names.h>
#include <rlab/rlab_macros_code.h>
#include <rlab/rlab_macros.h>
#include <rlab/mathl.h>
#include <rlab/rfileio.h>
#include <rlab/sort.h>

extern
    Rfile * rfile_find (char *);
extern
    int rfile_Destroy(char *);
extern
    Rfile * get_rfile_ds (char * name,  enum RFILE_TYPE rftype, ...);
extern
    FILE *RLAB_STDERR_DS;

//
// standard headers
//
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdint.h>
#include <getopt.h>
#include <inttypes.h>

#define _THIS_LIB "rlabplus_gts.c"

#define DEBUG

//
// gts and rlab-gts headers
//
#include <gts.h>
#include "rlabplus_gts.h"

static r_gts_surface * r_gts_surface_list = 0;     // Ptr to 1st surface

static int nr_gts_surface = 0, debug=0;            // Number of named surfaces

GtsColor GTS_DEFAULT_COLOR={1.0, 1.0, 1.0};

static double r_gts_eabs = 1e-9, r_gts_erel = 1e-6;


#include "r_gts_util.c"

//
// Create a new Rfile struct, and tack it on
// the front of the list. Return a ptr to the
// new Rfile struct. Make sure and bump the
// list element count.
//
static void free_slist (GtsObject * o)
{
  g_slist_free (o->reserved);
  o->reserved = NULL;
}

static void free_glist (GtsObject * o)
{
  g_list_foreach (o->reserved, (GFunc) gts_object_reset_reserved, NULL);
  g_list_free (o->reserved);
  o->reserved = NULL;
}

r_gts_surface * r_gts_surface_Create (void)
{
  r_gts_surface *new = (r_gts_surface *) GC_MALLOC (sizeof (r_gts_surface));

  if (!new)
    rerror ("out of memory");

  // zero the file structure
  new->name = 0;
  new->s = 0;
  new->n_vertex = 0;
  new->n_edge = 0;
  new->n_face = 0;

  // finish
  new->next = (r_gts_surface *) r_gts_surface_list;
  nr_gts_surface++;
  r_gts_surface_list = new;
  return (new);
}

int r_gts_surface_Destroy (char *name)
{
  r_gts_surface *rf=0, *next=0, *prev=0;
  rf = r_gts_surface_list;
  next = rf->next;
  prev = rf;

  while (next)
  {
    if (!strcmp (rf->name, name))
    {
      // Found it!

      // Re-Hook up list
      prev->next = next;

      // Check two special cases
      if (prev == rf)     // 1st element
        r_gts_surface_list = next;
      else if (next == 0) // Last element
        prev->next = 0;

      // Now destroy the element
      gts_object_destroy (GTS_OBJECT (rf->s));
      rf->s = 0;

      // Erase its name
      if (rf->name)
      {
        GC_FREE (rf->name);
        rf->name = 0;
      }
      rf->next = 0;
      GC_FREE (rf);
      rf = 0;

      nr_gts_surface--;
      return (1);   /* Success: an GTS surface is destroyed */
    }

    prev = rf;
    rf = next;
    next = rf->next;
  }

  return (0);     /* Failure: no GTS surface  with such name was found, so none was destroyed */
}


//
// Initialize the GTS surface list
//
void init_r_gts_surface_list (void)
{
  r_gts_surface_list = r_gts_surface_Create ();
}

//
// Walk the list, destroying each node
//
void destroy_r_gts_surface_list (void)
{
  r_gts_surface *rf=0, *next=0;

  rf = r_gts_surface_list;
  next = rf->next;

  while (next)
  {
    r_gts_surface_Destroy (rf->name);
    rf = next;
    next = rf->next;
  }
}

//
// Walk the list, destroying each node
//
void copy_names_of_r_gts_surface_list (MDS * snames)
{
  r_gts_surface *rf=0, *next=0;

  if (!nr_gts_surface)
  {
    return;
  }

  int i=0;

  rf = r_gts_surface_list;
  next = rf->next;
  while (next)
  {
    if (isvalidstring(rf->name)>0)
    {
      MdsV0(snames,i) = cpstr(rf->name);
    }
    else
    {
      MdsV0(snames,i) = cpstr("(null)");
    }
    rf = next;
    next = rf->next;
    i++;
  }

  return;
}


//
// Walk the list, look for a particular name
//
r_gts_surface * r_gts_surface_list_find (char *name)
{
  r_gts_surface *rf, *next;

  rf = r_gts_surface_list;
  next = rf->next;

  while (next)
  {
    if (!strcmp (rf->name, name))
    { return (rf); }
    rf = next;
    next = rf->next;
  }

  return (0);
}

static int gts_remove_duplicate_vertices_old(GtsVertex *v1, GtsSurface *s)
{
  GtsVertex * v = gts_delaunay_add_vertex (s, v1, NULL);
  if (v == v1)
  {
    return 1;
  }

  if (v != NULL)
  {
    gts_vertex_replace (v1, v);
  }

  return 0;
}

static int gts_remove_duplicate_vertices(GtsVertex *v, GtsSurface *s)
{
  GtsVertex *v1=0;

  //g_return_val_if_fail (surface != NULL, v);
  if (!s)
  {
    return 1;
  }

  //g_return_val_if_fail (v != NULL, v);
  if (!v)
  {
    return 2;
  }
  GtsFace * f=0;

//   fprintf(stderr, "gts_remove_duplicate_vertices: v=%g,%g,%g\n", v->p.x, v->p.y, v->p.z);


  f = gts_point_locate (GTS_POINT (v), s, NULL);
//   fprintf(stderr,  "f = %p\n", f);
  if (f)
  {
    GtsVertex *v1 = gts_delaunay_add_vertex_to_face (s, v, f);
    if ((v1 != NULL) && (v != v1))
    {
      gts_vertex_replace (v, v1);
      return 0;
    }

//     fprintf(stderr,  "failed: face found but failure in ((v1 != NULL) && (v != v1))\n");
    return 0;
  }

//   fprintf(stderr,  "f = %p\n", f);
  if (!f)
  {
    GtsVertex * v1 = gts_delaunay_add_vertex (s, v, NULL);
    if ((v1 != NULL) && (v != v1))
    {
      gts_vertex_replace (v, v1);
      return 0;
    }

//     fprintf(stderr,  "failed: face NOT found AND failure in ((v1 != NULL) && (v != v1))\n");
  }


  return 0;
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// intersection of segment and a surface
//
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static void copy_slist_pt_to_mdr (GtsPoint *p, void **f)
{
  int *count = (int *) f[0];
  MDR *pts = (MDR *) f[1];
  Mdr0(pts, *count, 0) = p->x;
  Mdr0(pts, *count, 1) = p->y;
  Mdr0(pts, *count, 2) = p->z;
  *count += 1;
  return;
}

static int gts_surface_func_segment_triangle_intersection (gpointer item, gpointer data)
{
  // ARG1: face
  GtsFace    *f = (GtsFace *) item;

  // ARG2:
  if (!f)
    return 1;

  r_gts_void_ptrs * dataptr = data;
  GtsSegment *s = (GtsSegment *) dataptr->vptr1;
  GtsPoint   *p = gts_segment_triangle_intersection (s, GTS_TRIANGLE(f), TRUE, gts_point_class());

  if (p)
  {
    GSList *pts_list = (GSList *) dataptr->vptr2;
    dataptr->vptr2 = g_slist_prepend (pts_list, p);
    *((int *) dataptr->vptr3) +=1;
  }

  return 0;
}

static void mdr_swap_rows(MDR *x, int i1, int i2)
{
  int nr=MNR(x), nc=MNC(x);
  double d;
  int id;

  if (i1 > nr || i1 < 0)
    return;
  if (i2 > nr || i2 < 0)
    return;

  int j;
  if (MD_TYPE_INT32(x))
  {
    for (j=0; j<nc;j++)
    {
      id = Mdi0(x,i1,j);
      Mdi0(x,i1,j) = Mdi0(x,i2,j);
      Mdi0(x,i2,j) = id;
    }
  }
  else if (MD_TYPE_DOUBLE(x))
  {
    for (j=0; j<nc;j++)
    {
      d = Mdr0(x,i1,j);
      Mdr0(x,i1,j) = Mdr0(x,i2,j);
      Mdr0(x,i2,j) = d;
    }
  }
  return;
}

#undef  THIS_SOLVER
#define THIS_SOLVER "gts_segment_intersect_surface"
Ent * ent_gts_segment_intersect_surface (int nargs, Datum args[])
{
  Ent *e1=0, *e2=0, *e3=0, *e4=0;
  FILE *rlab_stderr = (!RLAB_STDERR_DS) ? stderr : RLAB_STDERR_DS;
  char *name1=0;
  MDR *F=0, *N=0, *pts=0, *retpts=0;
  int i, i1, j, iret=-1 /*-1 report only the point with minimum 'd'; 0 report the maximum 'd'; 1-report minmax; 2,3-report all sorted asc,desc*/;
  double scale=1,d;

  r_gts_surface *rf1=0;

  if(!r_gts_surface_list)
  {
    init_r_gts_surface_list();
  }

  //
  if (nargs < 3)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_THREE_ARG_REQUIRED "\n");
    goto _exit;
  }

  //
  // ARG1: get name of the surface
  //
  e1 = bltin_get_ent(args[0]);
  if (ent_type(e1) != MATRIX_DENSE_STRING)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }
  name1 = class_char_pointer(e1);
  if (isvalidstring(name1) < 1)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }

  // ARG2: segment: point
  e2 = bltin_get_ent(args[1]);
  if (ent_type(e2) != MATRIX_DENSE_REAL)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG2_ROW_VECTOR_3 "\n");
    goto _exit;
  }
  F = ent_data(e2);
  if (SIZE(F)!=3)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG2_ROW_VECTOR_3 "\n");
    goto _exit;
  }

  // ARG3: segment: direction vector
  e3 = bltin_get_ent(args[2]);
  if (ent_type(e3) != MATRIX_DENSE_REAL)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG3_ROW_VECTOR_3 "\n");
    goto _exit;
  }
  N = ent_data(e3);
  if (SIZE(N)!=3)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG3_ROW_VECTOR_3 "\n");
    goto _exit;
  }

  //
  // get fourth last argument - options for the solver
  //
  if (nargs > 3)
  {
    e4 = bltin_get_ent(args[3]);
    if (ent_type(e4) == BTREE)
    {
      ListNode *node;
      RLABCODE_PROCESS_BTREE_ENTRY_BOOL(e4,node,RLAB_GTS_PARAM_RETURN_PT,iret);
//       RLABCODE_PROCESS_BTREE_ENTRY_BOOL(e4,node,RLAB_GTS_STACK_PTS,vfn.stack_pts);
    }
  }

  //
  // does 'name1' already exist in list of named surfaces
  //
  rf1 = r_gts_surface_list_find (name1);
  if (!rf1)
  {
    goto _exit;
  }

  //  pts = mdr_Create(pts_count,4);

  // create segment for face intersect segment search:
  //
  GtsVertex *vx1 = gts_vertex_new(gts_vertex_class(), mdrV0(F,0), mdrV0(F,1), mdrV0(F,2));
  GtsVertex *vx2 = gts_vertex_new(gts_vertex_class(), mdrV0(F,0) + scale * mdrV0(N,0),
                                  mdrV0(F,1) + scale * mdrV0(N,1),
                                        mdrV0(F,2) + scale * mdrV0(N,2));
  GtsSegment *s  = gts_segment_new(gts_segment_class(), vx1, vx2);
  int pts_count=0;

  GSList *pts_list=NULL;

  r_gts_void_ptrs dataptr;
  dataptr.vptr1 = (void *) s;
  dataptr.vptr2 = (void *) pts_list;
  dataptr.vptr3 = (void *) &pts_count;
  gts_surface_foreach_face (rf1->s, gts_surface_func_segment_triangle_intersection, (gpointer) &dataptr);
  pts_list = dataptr.vptr2;

  if (pts_count == 0)
    goto _exit;

  // copy all intersection points to the final result:
  void *f_args[2];
  pts = mdr_Create(pts_count,4);
  mdr_Zero(pts);
  pts_count = 0;
  f_args[0] = (void *) &pts_count;
  f_args[1] = (void *) pts;
  g_slist_foreach (pts_list, (GFunc) copy_slist_pt_to_mdr, f_args);

  // update the result with distance between the intersection point and the point F
  for (i=0; i < pts_count; i++)
  {
    d = mdrV0(F,0) - Mdr0(pts,i,0);
    Mdr0(pts,i,3) += d*d;
    d = mdrV0(F,1) - Mdr0(pts,i,1);
    Mdr0(pts,i,3) += d*d;
    d = mdrV0(F,2) - Mdr0(pts,i,2);
    Mdr0(pts,i,3) += d*d;
    Mdr0(pts,i,3) = sqrtf(Mdr0(pts,i,3));
  }

  if (pts_count == 1)
  {
    retpts = pts;
  }
  else
  {
    switch(iret)
    {
      case -1:
        retpts = mdr_Create(1,4);
        // find minimum distance:
        for (i=0; i<pts_count; i++)
        {
          if (Mdr0(pts,0,3) > Mdr0(pts,i,3))
          {
            mdr_swap_rows(pts, 0, i);
          }
        }
        for (j=0; j<4; j++)
        {
          MdrV0(retpts,j) = Mdr0(pts,0,j);
        }
        mdr_Destroy(pts);
        break;

      case 0:
        // find maximum distance:
        retpts = mdr_Create(1,4);
        for (i=1; i<pts_count; i++)
        {
          if (Mdr0(pts,0,3) < Mdr0(pts,i,3))
          {
            mdr_swap_rows(pts, 0, i);
          }
        }
        for (j=0; j<4; j++)
        {
          MdrV0(retpts,j) = Mdr0(pts,0,j);
        }
        mdr_Destroy(pts);
        break;

      case 1:
        // find maximum distance:
        retpts = mdr_Create(1,4);
        for (i=1; i<pts_count; i++)
        {
          if (Mdr0(pts,0,3) < Mdr0(pts,i,3))
          {
            mdr_swap_rows(pts, 0, i);
          }
        }
        for (j=0; j<4; j++)
        {
          MdrV0(retpts,j) = Mdr0(pts,0,j);
        }
        mdr_Destroy(pts);
        break;

      case 2:
        printf("MINMAX\n");
        // min max
        retpts = mdr_Create(2,4);
        // copy first point to [min;max]
        for (j=0; j<4; j++)
        {
          Mdr0(retpts,0,j) = Mdr0(pts,0,j);
          Mdr0(retpts,1,j) = Mdr0(pts,0,j);
        }
        for (i=1; i<pts_count; i++)
        {
          // check if we have minimum distance in row 0 of retpts
          if (Mdr0(retpts,0,3) > Mdr0(pts,i,3))
          {
            for (j=0; j<4; j++)
            {
              Mdr0(retpts,0,j) = Mdr0(pts,i,j);
            }
            continue;
          }
          // check if we have maximum distance in row 1 of retpts
          if (Mdr0(retpts,1,3) < Mdr0(pts,i,3))
          {
            for (j=0; j<4; j++)
            {
              Mdr0(retpts,1,j) = Mdr0(pts,i,j);
            }
          }
        }
        mdr_Destroy(pts);
        break;

      case 3:
        // sort descending
        for (i=0; i<(pts_count-1); i++)
        {
          for (i1=i+1; i<pts_count; i++)
          {
            if (Mdr0(pts,i,3) < Mdr0(pts,i1,3))
            {
              mdr_swap_rows(pts, i, i1);
            }
          }
        }
        retpts = pts;
        break;

      default:
        break;
    }
  }



    // do the sort, expect small number of points because the surface should be organized as a shell

_exit:

  // cleanup the mess
  ent_Clean(e1);
  ent_Clean(e2);
  ent_Clean(e3);
  ent_Clean(e4);
  g_slist_free(pts_list);
  gts_object_destroy(GTS_OBJECT(s));

  //
  // return the result of last function call
  //
  return ent_Assign_Rlab_MDR(retpts);
}


//
// vertex:
//    get all vertices associated with a surface
//    modify/but not delete vertices associated with a surface
//
#undef  THIS_SOLVER
#define THIS_SOLVER "gts_surface_from_vertices"
Ent * ent_gts_surface_from_vertices (int nargs, Datum args[])
{
  Ent *e1=0, *e2=0, *e3=0;
  FILE *rlab_stderr = (!RLAB_STDERR_DS) ? stderr : RLAB_STDERR_DS;
  char *name1=0;
  MDR *new_vx=0, *new_ed_tr =0, *is_constr=0, *color=0;

  int i, j, method=1, idim=3;
  gdouble dummy=-1, threshold=0.0, rval=1.0;
  ListNode *node=0;

  GtsColor c={1.0, 1.0, 1.0};

  r_gts_surface *rf1=0;

  if(!r_gts_surface_list)
  {
    init_r_gts_surface_list();
  }

  //
  if (nargs<2)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_TWO_THREE_OR_FOUR_ARG_REQUIRED "\n");
    goto _exit;
  }

  //
  // get the surface name: create it if nonexistent, or delete it and create new one
  //
  e1 = bltin_get_ent(args[0]);
  if (ent_type(e1) != MATRIX_DENSE_STRING)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }
  name1 = class_char_pointer(e1);
  if (isvalidstring(name1) < 1)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }
  rf1 = r_gts_surface_list_find (name1);
  if (rf1)
  {
    r_gts_surface_Destroy (name1);
    rf1 = 0;
  }
  if (!rf1)
  {
    rf1 = r_gts_surface_Create ();
    rf1->name = cpstr(name1);
    rf1->s = (GtsSurface *) gts_surface_new (gts_surface_class(),
              GTS_FACE_CLASS(color_face_class()), gts_edge_class(), gts_vertex_class());
  }

  if ((nargs != 2)&&(nargs != 3)&&(nargs != 4))
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_TWO_THREE_OR_FOUR_ARG_REQUIRED "\n");
    goto _exit;
  }

  //
  // get the 3-col matrix of vertices
  //
  if (nargs > 1)
  {
    e2 = bltin_get_ent(args[1]);
    if (ent_type(e2) != MATRIX_DENSE_REAL)
    {
      fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG2_MDR_MATRIX_3COL "\n");
      goto _exit;
    }
    new_vx = ent_data(e2);
    if (MNC(new_vx) != 3)
    {
      fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG2_MDR_MATRIX_3COL "\n");
      goto _exit;
    }
    if (MNR(new_vx) < 3)
    {
      fprintf (rlab_stderr, THIS_SOLVER ": At least three points are needed for face!\n");
      goto _exit;
    }
  }

  //
  // check if the last argument is a list of options
  //
  if (nargs > 2)
  {
    e3 = bltin_get_ent(args[2]);
    if (ent_type(e3) == BTREE)
    {

      //
      // f:
      //    the 3-col matrix of verticex indices making triangle faces
      //
      RLABCODE_PROCESS_BTREE_ENTRY_MD(e3,node,RLAB_GTS_FACE,new_ed_tr,MDR,MNC,3,NULL);

      //
      // method: for building faces
      //    "delaunay"
      //    "convexhull"
      node = btree_FindNode (ent_data (e3), RLAB_NAME_GEN_METHOD);
      if (node != 0)
      {
        char *s = class_char_pointer(var_ent(node));
        if((s[0]=='d') || (s[0]=='D'))
        {
          method = 1;
        }
        else if((s[0]=='c') || (s[0]=='C'))
        {
          method = 2;
        }
        else
        {
          fprintf (rlab_stderr, THIS_SOLVER ": Entry 'method' not recognized. "
              " Using default 'delaunay' triangulation.\n");
        }
      }

      RLABCODE_PROCESS_BTREE_ENTRY_MD(e3,node,RLAB_GTS_NEW_SURFACE_COLOR,color,MDR,SIZE,3,NULL);
      if (SIZE(color)>=3)
      {
        c.r = mdrV0(color,0);
        c.g = mdrV0(color,1);
        c.b = mdrV0(color,2);
      }

      dummy = 0;
      RLABCODE_PROCESS_BTREE_ENTRY_DOUBLE(e3, node, RLAB_GTS_PARAM_DIM, dummy);
      if ((dummy ==2) || (dummy==3))
      {
        idim = dummy;
      }

      dummy = 0;
      RLABCODE_PROCESS_BTREE_ENTRY_DOUBLE(e3, node, RLAB_GTS_NEW_SURFACE_THRESHOLD, dummy);
      if (dummy > 0)
      {
        threshold = dummy;
      }
    }
  }

  GtsTriangle *t=0;

  if (SIZE(new_ed_tr) > 1)
  {
    if (MNC(new_ed_tr) == 3)
    {
//       mdr_Print(new_vx,stdout);
//       mdr_Print(new_ed_tr,stdout);
      //
      // provided is the list of faces to be assembled from the vertices
      //
      int ir, i1, i2, i3;
      for (ir=0; ir<MNR(new_ed_tr); ir++)
      {
//         printf("ir=%i\n", ir);
        i1 = mdi0(new_ed_tr,ir,0)-1;
        i2 = mdi0(new_ed_tr,ir,1)-1;
        i3 = mdi0(new_ed_tr,ir,2)-1;
        // Create three vertices using indices provided in 3rd argument
        GtsVertex * vx1 = gts_vertex_new(gts_vertex_class(),
                                         mdr0(new_vx,i1,0), mdr0(new_vx,i1,1), mdr0(new_vx,i1,2));
        GtsVertex * vx2 = gts_vertex_new(gts_vertex_class(),
                                         mdr0(new_vx,i2,0), mdr0(new_vx,i2,1), mdr0(new_vx,i2,2));
        GtsVertex * vx3 = gts_vertex_new(gts_vertex_class(),
                                         mdr0(new_vx,i3,0), mdr0(new_vx,i3,1), mdr0(new_vx,i3,2));
// Create three edges:
        //  these edges are special in that they represent the boundaries
//         GtsEdge   *  e1 = gts_edge_new(GTS_EDGE_CLASS (gts_constraint_class ()), vx1, vx2);
//         GtsEdge   *  e2 = gts_edge_new(GTS_EDGE_CLASS (gts_constraint_class ()), vx2, vx3);
//         GtsEdge   *  e3 = gts_edge_new(GTS_EDGE_CLASS (gts_constraint_class ()), vx3, vx1);
        GtsEdge   *  e1 = gts_edge_new(gts_edge_class (), vx1, vx2);
        GtsEdge   *  e2 = gts_edge_new(gts_edge_class (), vx2, vx3);
        GtsEdge   *  e3 = gts_edge_new(gts_edge_class (), vx3, vx1);
        // Create face
        GtsFace * f = gts_face_new(GTS_FACE_CLASS(color_face_class()), e1, e2, e3);
        COLOR_FACE(f)->c.r = c.r;
        COLOR_FACE(f)->c.g = c.g;
        COLOR_FACE(f)->c.b = c.b;
//         fprintf(stderr, "2c\n");
        // add it to the surface
        gts_surface_add_face(rf1->s, f);
//         GtsSurface *sdummy = (GtsSurface *) gts_surface_new (gts_surface_class(),
//                                   GTS_FACE_CLASS(color_face_class()), gts_edge_class(), gts_vertex_class());
//         gts_surface_add_face(sdummy, f);
//         gts_surface_merge(rf1->s, sdummy);
//         gts_object_destroy(GTS_OBJECT (sdummy));
      }

    }
  }
  else
  {
    if (method ==1)
    {
      //
      // extract vertices from all points
      //
      GSList *l=NULL, *v_list=NULL;
      for (i=0; i<MNR(new_vx); i++)
      {
        GtsVertex *v = gts_vertex_new (gts_vertex_class (), Mdr0(new_vx,i,0), Mdr0(new_vx,i,1),
                                       Mdr0(new_vx,i,2));
        v_list = g_slist_prepend (v_list, v);
      }

      GtsVertex * v1, * v2, * v3;
      t = gts_triangle_enclosing (gts_triangle_class (), v_list, 1.0);
      gts_triangle_vertices (t, &v1, &v2, &v3);

      GtsFace * f = gts_face_new(GTS_FACE_CLASS(color_face_class()), t->e1, t->e2, t->e3);
      COLOR_FACE(f)->c.r = c.r;
      COLOR_FACE(f)->c.g = c.g;
      COLOR_FACE(f)->c.b = c.b;

      // add it to the surface
      gts_surface_add_face (rf1->s, f);
      l = v_list;
      while (l) {
        gts_delaunay_add_vertex (rf1->s, l->data, NULL);
        l = l->next;
      }
      gts_allow_floating_vertices = TRUE;
      gts_object_destroy (GTS_OBJECT (v1));
      gts_object_destroy (GTS_OBJECT (v2));
      gts_object_destroy (GTS_OBJECT (v3));
      gts_allow_floating_vertices = FALSE;
      g_slist_free (v_list);
    }
    else
    {
      double xavg=0,yavg=0;

      //
      // CONVHULL:
      //
      int *faceIndices = NULL;
      int i1, i2, i3, ix, nFaces, k;

      if (idim == 2)
      {
        double  *pts=(double *) GC_MALLOC(idim*MNR(new_vx)*sizeof(double));
        for (i=0; i<MNR(new_vx); i++)
        {
          for (j=0; j<idim; j++)
          {
            pts[i*idim + j] = mdr0(new_vx,i,j);
          }
        }
        convhull_nd_build(pts, MNR(new_vx), idim, &faceIndices, NULL, NULL, &nFaces);
        GC_FREE(pts);
      }
      else
      {
        mdr_Transpose_inplace (new_vx);
        convhull_3d_build((ch_vertex*)MDRPTR(new_vx), MNC(new_vx), &faceIndices, &nFaces);
        mdr_Transpose_inplace (new_vx);
      }

      xavg = yavg = 0;
      for (i=0; i<nFaces; i++)
      {
        for (j=0; j<idim; j++)
        {
          i1 = faceIndices[i*idim + j];
          xavg += mdr0(new_vx,i1,0);
          yavg += mdr0(new_vx,i1,1);
        }
      }
      xavg /= (2*nFaces);
      yavg /= (2*nFaces);

      GtsVertex *vx1, *vx2, *vx3;
      GtsEdge   *e1, *e2, *e3;
      GtsFace *f;

      // this vertex belongs to all triangles:
      vx1 = gts_vertex_new(gts_vertex_class(), xavg, yavg, 0.0);
      for (i=0; i<nFaces; i++)
      {
        // create remaining vertices of each triangle
        i2 = faceIndices[i*idim];
        i3 = faceIndices[i*idim + 1];
        vx2 = gts_vertex_new(gts_vertex_class(),
                             mdr0(new_vx,i2,0), mdr0(new_vx,i2,1), mdr0(new_vx,i2,2));
        vx3 = gts_vertex_new(gts_vertex_class(),
                             mdr0(new_vx,i3,0), mdr0(new_vx,i3,1), mdr0(new_vx,i3,2));
        // Create three edges:
        e1 = gts_edge_new(gts_edge_class (), vx1, vx2);
        e2 = gts_edge_new(gts_edge_class (), vx2, vx3);
        e3 = gts_edge_new(gts_edge_class (), vx3, vx1);

        // Create face
        f = gts_face_new(GTS_FACE_CLASS(color_face_class()), e1, e2, e3);
        COLOR_FACE(f)->c.r = c.r;
        COLOR_FACE(f)->c.g = c.g;
        COLOR_FACE(f)->c.b = c.b;

        // add it to the surface
        gts_surface_add_face(rf1->s, f);
      }

      free(faceIndices);

#if 0

      //
      // QHULL: combine vertices into surface using convex hull
      //
      int exitcode;             /* 0 if no error from qhull */
      qhT qh_qh;                /* Qhull's data structure.  First argument of most calls */
      qhT *qh=&qh_qh;
      vertexT *vertex;
      facetT *facet;            /* set by FORALLfacets */

      /* True if qhull should free points in qh_freeqhull() or reallocation */
      boolT ismalloc=False;

      /* option flags for qhull, see qh-quick.htm */
      char *flags=0;
      if (idim == 2)
      {
        flags= "qhull Qb2:0B2:0";
      }
      else
      {
        flags= "qhull";
      }

      /* output from qh_produce_output() use NULL to skip qh_produce_output() */
      FILE *outfile=NULL;
      /* error messages from qhull code */
      FILE *errfile=NULL;

      QHULL_LIB_CHECK

      qh_zero(qh, errfile);
      mdr_Transpose_inplace (new_vx);
      exitcode= qh_new_qhull(qh, 3, MNC(new_vx), MDRPTR(new_vx), ismalloc,
                             flags, outfile, errfile);
      mdr_Transpose_inplace (new_vx);
      if (exitcode)
      {
        printf("qhull reports error code: %i. Aborting evaluation!\n", exitcode);
        goto _exit;
      }

      int     *pt_idx=(int *) GC_MALLOC((qh->num_vertices)*sizeof(int));
      double  *ang_rad=(double *) GC_MALLOC((qh->num_vertices)*sizeof(double));

      // collect all points from the list entering the edges of convex hull
      i=0;
      FORALLvertices
      {
        i1 = pt_idx[i] = qh_pointid(qh, vertex->point);
        xavg += mdr0(new_vx,i1,0);
        yavg += mdr0(new_vx,i1,1);
        i++;
      }
      //
      // sort points in hull based on 2D convexity assumption
      //
      xavg /= qh->num_vertices;
      yavg /= qh->num_vertices;
      for (i=0; i<qh->num_vertices; i++)
      {
        i1 = pt_idx[i];
        ang_rad[i] = atan2(mdr0(new_vx,i1,1)-yavg,mdr0(new_vx,i1,0)-xavg);
      }
      double  *r_idx=(double *) GC_MALLOC((qh->num_vertices)*sizeof(double));
      for (i=0; i<qh->num_vertices; i++)
      {
        i1 = pt_idx[i];
        r_idx[i] = i;
      }
      r_sort (ang_rad, 0, qh->num_vertices-1, r_idx);

      GtsVertex *vx1, *vx2, *vx3;
      GtsEdge   *e1, *e2, *e3;
      GtsFace *f;

      // construct first triangle:
      i1 = pt_idx[(int)r_idx[0]];
      i2 = pt_idx[(int)r_idx[1]];
      i3 = pt_idx[(int)r_idx[2]];

      vx1 = gts_vertex_new(gts_vertex_class(),
                                       mdr0(new_vx,i1,0), mdr0(new_vx,i1,1), mdr0(new_vx,i1,2));
      vx2 = gts_vertex_new(gts_vertex_class(),
                                       mdr0(new_vx,i2,0), mdr0(new_vx,i2,1), mdr0(new_vx,i2,2));
      vx3 = gts_vertex_new(gts_vertex_class(),
                                       mdr0(new_vx,i3,0), mdr0(new_vx,i3,1), mdr0(new_vx,i3,2));
      // Create three edges:
      e1 = gts_edge_new(gts_edge_class (), vx1, vx2);
      e2 = gts_edge_new(gts_edge_class (), vx2, vx3);
      e3 = gts_edge_new(gts_edge_class (), vx3, vx1);

      // Create face
      f = gts_face_new(GTS_FACE_CLASS(color_face_class()), e1, e2, e3);
      COLOR_FACE(f)->c.r = c.r;
      COLOR_FACE(f)->c.g = c.g;
      COLOR_FACE(f)->c.b = c.b;

      // add it to the surface
      gts_surface_add_face(rf1->s, f);

      // now loop over the remaining N-2 points and add them as vertices and edges:
      for (ix=3; ix<qh->num_vertices; ix++)
      {
        i2 = pt_idx[(int)r_idx[ix-1]];
        i3 = pt_idx[(int)r_idx[ix]  ];
        vx2 = gts_vertex_new(gts_vertex_class(),
                                         mdr0(new_vx,i2,0), mdr0(new_vx,i2,1), mdr0(new_vx,i2,2));
        vx3 = gts_vertex_new(gts_vertex_class(),
                                         mdr0(new_vx,i3,0), mdr0(new_vx,i3,1), mdr0(new_vx,i3,2));
        // Create three edges:
        e1 = gts_edge_new(gts_edge_class (), vx1, vx2);
        e2 = gts_edge_new(gts_edge_class (), vx2, vx3);
        e3 = gts_edge_new(gts_edge_class (), vx3, vx1);
        // Create face
        f = gts_face_new(GTS_FACE_CLASS(color_face_class()), e1, e2, e3);
        COLOR_FACE(f)->c.r = c.r;
        COLOR_FACE(f)->c.g = c.g;
        COLOR_FACE(f)->c.b = c.b;
        // add it to the surface
        gts_surface_add_face(rf1->s, f);
      }

      qh_freeqhull(qh, qh_ALL);
      GC_FREE(pt_idx);
      GC_FREE(ang_rad);
      GC_FREE(r_idx);

#endif
    }
  }

  if (threshold)
  {
    r_gts_surface_cleanup(rf1->s, threshold);
  }

  rval = 0.0;

_exit:

  ent_Clean(e1);
  ent_Clean(e2);
  ent_Clean(e3);
  return ent_Create_Rlab_Double (rval);
}


//
// vertex:
//    get all vertices associated with a surface
//    modify/but not delete vertices associated with a surface
//
#undef  THIS_SOLVER
#define THIS_SOLVER "gts_surface_threshold"
Ent * ent_gts_surface_threshold (int nargs, Datum args[])
{
  Ent *e1=0, *e2=0;
  FILE *rlab_stderr = (!RLAB_STDERR_DS) ? stderr : RLAB_STDERR_DS;
  char *name1=0;
  MDR *color=0;
  double rval=-1;

  int i;
  gdouble dummy=-1, threshold=0.0;
  ListNode *node=0;

  GtsColor c={1.0, 1.0, 1.0};

  r_gts_surface *rf1=0;

  if(!r_gts_surface_list)
  {
    init_r_gts_surface_list();
  }

  //
  if (nargs<2)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_TWO_THREE_OR_FOUR_ARG_REQUIRED "\n");
    goto _exit;
  }

  //
  // get the surface name: create it if nonexistent, or delete it and create new one
  //
  e1 = bltin_get_ent(args[0]);
  if (ent_type(e1) != MATRIX_DENSE_STRING)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }
  name1 = class_char_pointer(e1);
  if (isvalidstring(name1) < 1)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }
  rf1 = r_gts_surface_list_find (name1);
  if (!rf1)
  {
    goto _exit;
  }

  if ((nargs != 2)&&(nargs != 3)&&(nargs != 4))
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_TWO_THREE_OR_FOUR_ARG_REQUIRED "\n");
    goto _exit;
  }

  //
  // check if the last argument is a list of options
  //
  if (nargs > 1)
  {
    e2 = bltin_get_ent(args[1]);
    if (ent_type(e2) == BTREE)
    {
      RLABCODE_PROCESS_BTREE_ENTRY_MD(e2,node,RLAB_GTS_NEW_SURFACE_COLOR,color,MDR,SIZE,3,NULL);
      if (SIZE(color)>=3)
      {
        c.r = mdrV0(color,0);
        c.g = mdrV0(color,1);
        c.b = mdrV0(color,2);
      }

      dummy = 0;
      RLABCODE_PROCESS_BTREE_ENTRY_DOUBLE(e2, node, RLAB_GTS_NEW_SURFACE_THRESHOLD, dummy);
      if (dummy > 0)
      {
        threshold = dummy;
      }

    }
  }

  if (threshold)
  {
    r_gts_surface_cleanup(rf1->s, threshold);
  }
  rval = 0.0;

_exit:

  ent_Clean(e1);
  ent_Clean(e2);
  return ent_Create_Rlab_Double (rval);
}


#undef  THIS_SOLVER
#define THIS_SOLVER "gts_surface_list"
Ent * ent_gts_surface_list(int nargs, Datum args[])
{
  MDS *snames=0;

  if (!nr_gts_surface)
  {
    snames=mds_Create(0,0);
    goto _exit;
  }

  snames = mds_Create(1,nr_gts_surface-1);
  copy_names_of_r_gts_surface_list(snames);

_exit:

  return ent_Assign_Rlab_MDS(snames);
}

#undef  THIS_SOLVER
#define THIS_SOLVER "gts_surface_export"
Ent * ent_gts_surface_export (int nargs, Datum args[])
{
  Ent *e1=0, *ent=0;
  FILE *rlab_stderr = (!RLAB_STDERR_DS) ? stderr : RLAB_STDERR_DS;
  char *name1=0, *name2=0;

  r_gts_surface *rf1=0;

  if(!r_gts_surface_list)
  {
    init_r_gts_surface_list();
  }

  Btree *rval = btree_Create();
  MDR *vertex=0, *edge=0, *face=0, *color=0;

  //
  if (nargs != 1)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ONE_ARG_REQUIRED "\n");
    goto _exit;
  }
  //
  // get the surface name
  //
  e1 = bltin_get_ent(args[0]);
  if (ent_type(e1) != MATRIX_DENSE_STRING)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }
  name1 = class_char_pointer(e1);
  if (isvalidstring(name1) < 1)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }
  //
  // does 'name1' already exist in list of named surfaces
  //
  rf1 = r_gts_surface_list_find (name1);
  if (!rf1)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": Surface '%s' does not exist. Nothing to do!\n", name1);
    goto _exit;
  }

  /* Get the number of vertices */
  r_gts_surface_export (rf1->s, &vertex, &edge, &face, &color);

_exit:

  ent_Clean(e1);
  if (vertex)
  {
    RLABCODE_INSTALL_ENTITY_IN_BTREE(rval,ent,MATRIX_DENSE_REAL,vertex,RLAB_GTS_VERTEX);
  }
  if (edge)
  {
    RLABCODE_INSTALL_ENTITY_IN_BTREE(rval,ent,MATRIX_DENSE_REAL,edge,RLAB_GTS_EDGE);
  }
  if (face)
  {
    RLABCODE_INSTALL_ENTITY_IN_BTREE(rval,ent,MATRIX_DENSE_REAL,face,RLAB_GTS_FACE);
  }
  if (color)
  {
    RLABCODE_INSTALL_ENTITY_IN_BTREE(rval,ent,MATRIX_DENSE_REAL,color,RLAB_GTS_FACE_COLOR);
  }
  return ent_Assign_Rlab_BTREE (rval);
}

#undef  THIS_SOLVER
#define THIS_SOLVER "gts_surface_vertices"
Ent * ent_gts_surface_vertices (int nargs, Datum args[])
{
  Ent *e1=0, *e2=0, *e3=0;
  FILE *rlab_stderr = (!RLAB_STDERR_DS) ? stderr : RLAB_STDERR_DS;
  char *name1=0, *name2=0;
  MDR *old_vx=0, *new_vx=0, *rval=0;

  r_gts_surface *rf1=0;

  if(!r_gts_surface_list)
  {
    init_r_gts_surface_list();
  }

  //
  if ((nargs != 1)&&(nargs != 3))
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ONE_OR_THREE_ARG_REQUIRED "\n");
    goto _exit;
  }

  //
  // get the surface name
  //
  e1 = bltin_get_ent(args[0]);
  if (ent_type(e1) != MATRIX_DENSE_STRING)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }
  name1 = class_char_pointer(e1);
  if (isvalidstring(name1) < 1)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }

  //
  // does 'name1' already exist in list of named surfaces
  //
  rf1 = r_gts_surface_list_find (name1);
  if (!rf1)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": Surface '%s' does not exist. Nothing to do!\n", name1);
    goto _exit;
  }

  /* Get the number of vertices */
  int N = gts_surface_vertex_number(rf1->s);

  if (nargs == 1)
  {
    rval = mdr_Create(N,3);
    r_copy_vertex_to_mdr (NULL, NULL); // reset the counter
    gts_surface_foreach_vertex(rf1->s, (GtsFunc) r_copy_vertex_to_mdr, (gpointer) rval);
  }
  else if (nargs > 1)
  {
    //
    // get the indices of the vertices to be replaced with new values
    //
    e2 = bltin_get_ent(args[1]);
    if (ent_type(e2) != MATRIX_DENSE_REAL)
    {
      fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG2_MDR_INTEGER "\n");
      goto _exit;
    }
    old_vx = ent_data(e2);

    e3 = bltin_get_ent(args[2]);
    if (ent_type(e3) != MATRIX_DENSE_REAL)
    {
      fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG3_MDR_MATRIX "\n");
      goto _exit;
    }
    new_vx = ent_data(e3);

    if ((MNR(old_vx)!=MNR(new_vx)) || (MNC(old_vx)!=3) || (MNC(new_vx)!=3))
    {
      fprintf (rlab_stderr, THIS_SOLVER ": Number of indices has to match number of rows of vertex matirx. Cannot continue!\n");
      goto _exit;
    }

    //
    // iterate over all edges and replace old_vx with new_vx
    //
    int *idx_row = GC_MALLOC(MNR(old_vx)*sizeof(int));
    r_gts_func_vertex data;
    data.old_vx = old_vx;
    data.new_vx = new_vx;
    data.idx    = idx_row;
    r_replace_mdr_with_mdr (NULL, (gpointer) &data); // reset all counters
    gts_surface_foreach_vertex(rf1->s, (GtsFunc) r_replace_mdr_with_mdr, (gpointer) &data);
    data.old_vx = 0;
    data.new_vx = 0;
    data.idx    = 0;
    GC_FREE(idx_row);
    rval = mdr_CreateScalar(0.0);
  }

_exit:

  ent_Clean(e1);
  ent_Clean(e2);
  ent_Clean(e3);
  return ent_Assign_Rlab_MDR (rval);
}

//
// query read - my invention
//
#undef  THIS_SOLVER
#define THIS_SOLVER "gts_point_new"
Ent *
ent_gts_point_new (int nargs, Datum args[])
{
  Ent *e1=0, *rent=0;
  int nr=0, nc=0, i, k;

  GtsPoint *p=NULL;

  FILE *rlab_stderr = (!RLAB_STDERR_DS) ? stderr : RLAB_STDERR_DS;

  MDR *xyz=0, *rval=0;

  //
  if (nargs != 1)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ONE_ARG_REQUIRED "\n");
    goto _exit;
  }

  //
  // get matrix of rows of points in [x,y,z] format
  //
  e1 = bltin_get_ent(args[0]);
  if (ent_type(e1) != MATRIX_DENSE_REAL)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_MDR_MATRIX_3COL "\n");
    goto _exit;
  }
  xyz = ent_data(e1);
  nc = MNC(xyz);
  if (nc != 3)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_MDR_MATRIX_3COL "\n");
    goto _exit;
  }

  //
  // go over rows, and for each create a GTS point
  //
  rval = mdi_Create(2, nr);
  for (i=0; i<nr; i++)
  {
    p = gts_point_new( gts_point_class(), (gdouble) mdr0(xyz,i,0), (gdouble) mdr0(xyz,i,1), (gdouble) mdr0(xyz,i,2) );
    intptr_t ival = (intptr_t) (p->object).reserved;
    intptr_t *iival = (intptr_t *) &Mdi0(rval, i, 0);
    *iival = ival;
  }

_exit:

  ent_Clean(e1);
  return ent_Assign_Rlab_MDR(rval);
}

static int r_foreach_transform_vertices(gpointer item, gpointer data)
{
  r_gts_surface_transform *st = (r_gts_surface_transform *) data;
  GtsVertex *v = (GtsVertex *) item;
  int i, ireplace=0;

  MDR *transl=st->transl, *center=st->center, *scale=st->scale, *rot_x=st->rot_x, *rot_y=st->rot_y, *rot_z=st->rot_z;
  MDR *vx = st->vertex;
  MDR *result = st->result;
  MdrV0(vx,0) = v->p.x;
  MdrV0(vx,1) = v->p.y;
  MdrV0(vx,2) = v->p.z;

  // center: 1st
  if (center)
  {
    MdrV0(vx,0) -= MdrV0(center,0);
    MdrV0(vx,1) -= MdrV0(center,1);
    MdrV0(vx,2) -= MdrV0(center,2);
  }
  // scaling:
  if (scale)
  {
    ireplace = 1;
    MdrV0(vx,0) *= mdrV0_safe(scale,0);
    MdrV0(vx,1) *= mdrV0_safe(scale,1);
    MdrV0(vx,2) *= mdrV0_safe(scale,2);
  }
  // rotations:
  //  rot_x:
  if (rot_x)
  {
    ireplace = 1;
    mdr_Multiply_store (rot_x, vx, result);
    // copy result to vx
    for (i=0; i<3; i++)
      MdrV0(vx,i) = MdrV0(result,i);
  }
  if (rot_y)
  {
    ireplace = 1;
    mdr_Multiply_store (rot_y, vx, result);
    // copy result to vx
    for (i=0; i<3; i++)
      MdrV0(vx,i) = MdrV0(result,i);
  }
  if (rot_z)
  {
    ireplace = 1;
    mdr_Multiply_store (rot_z, vx, result);
    // copy result to vx
    for (i=0; i<3; i++)
      MdrV0(vx,i) = MdrV0(result,i);
  }
  // center: last
  if (center)
  {
    MdrV0(vx,0) += MdrV0(center,0);
    MdrV0(vx,1) += MdrV0(center,1);
    MdrV0(vx,2) += MdrV0(center,2);
  }
  // translation: after recentering
  if (transl)
  {
    ireplace = 1;
    MdrV0(vx,0) += MdrV0(transl,0);
    MdrV0(vx,1) += MdrV0(transl,1);
    MdrV0(vx,2) += MdrV0(transl,2);
  }

  if (ireplace)
  {
    GtsVertex *with=gts_vertex_new(gts_vertex_class(), MdrV0(vx,0), MdrV0(vx,1), MdrV0(vx,2));
    gts_vertex_replace (v, with);
  }

  return 0;
}

static void surface_add_box (GtsSurface * s, MDR * bbox_x, MDR *bbox_y, MDR *bbox_z,
                             MDR *color, MDR *remf)
{
  gdouble x1, x2, y1, y2, z1, z2;
  GtsColor c=GTS_DEFAULT_COLOR;
  int ir, im, irf=0;

  //
  if (SIZE(bbox_x)<1)
  {
    x1 = 0;
    x2 = 0;
  }
  else if (SIZE(bbox_x)==1)
  {
    x1 = mdrV0(bbox_x,0);
    x2 = mdrV0(bbox_x,0);
  }
  else
  {
    x1 = mdrV0(bbox_x,0);
    x2 = mdrV0(bbox_x,1);
  }
  //
  if (SIZE(bbox_y)<1)
  {
    y1 = 0;
    y2 = 0;
  }
  else if (SIZE(bbox_y)==1)
  {
    y1 = mdrV0(bbox_y,0);
    y2 = mdrV0(bbox_y,0);
  }
  else
  {
    y1 = mdrV0(bbox_y,0);
    y2 = mdrV0(bbox_y,1);
  }
  //
  if (SIZE(bbox_z)<1)
  {
    z1 = 0;
    z2 = 0;
  }
  else if (SIZE(bbox_z)==1)
  {
    z1 = mdrV0(bbox_z,0);
    z2 = mdrV0(bbox_z,0);
  }
  else
  {
    z1 = mdrV0(bbox_z,0);
    z2 = mdrV0(bbox_z,1);
  }

  GtsVertex * v0 = gts_vertex_new (s->vertex_class, x1, y1, z1);
  GtsVertex * v1 = gts_vertex_new (s->vertex_class, x1, y1, z2);
  GtsVertex * v2 = gts_vertex_new (s->vertex_class, x1, y2, z2);
  GtsVertex * v3 = gts_vertex_new (s->vertex_class, x1, y2, z1);
  GtsVertex * v4 = gts_vertex_new (s->vertex_class, x2, y1, z1);
  GtsVertex * v5 = gts_vertex_new (s->vertex_class, x2, y1, z2);
  GtsVertex * v6 = gts_vertex_new (s->vertex_class, x2, y2, z2);
  GtsVertex * v7 = gts_vertex_new (s->vertex_class, x2, y2, z1);

  GtsEdge * e1 = gts_edge_new (s->edge_class, v0, v1);
  GtsEdge * e2 = gts_edge_new (s->edge_class, v1, v2);
  GtsEdge * e3 = gts_edge_new (s->edge_class, v2, v3);
  GtsEdge * e4 = gts_edge_new (s->edge_class, v3, v0);
  GtsEdge * e5 = gts_edge_new (s->edge_class, v0, v2);

  GtsEdge * e6 = gts_edge_new (s->edge_class, v4, v5);
  GtsEdge * e7 = gts_edge_new (s->edge_class, v5, v6);
  GtsEdge * e8 = gts_edge_new (s->edge_class, v6, v7);
  GtsEdge * e9 = gts_edge_new (s->edge_class, v7, v4);
  GtsEdge * e10 = gts_edge_new (s->edge_class, v4, v6);

  GtsEdge * e11 = gts_edge_new (s->edge_class, v3, v7);
  GtsEdge * e12 = gts_edge_new (s->edge_class, v2, v6);
  GtsEdge * e13 = gts_edge_new (s->edge_class, v1, v5);
  GtsEdge * e14 = gts_edge_new (s->edge_class, v0, v4);

  GtsEdge * e15 = gts_edge_new (s->edge_class, v1, v6);
  GtsEdge * e16 = gts_edge_new (s->edge_class, v2, v7);
  GtsEdge * e17 = gts_edge_new (s->edge_class, v3, v4);
  GtsEdge * e18 = gts_edge_new (s->edge_class, v0, v5);

  GtsFaceClass * klass = GTS_FACE_CLASS(color_face_class());
  GtsFace *f1,*f2,*f3,*f4,*f5,*f6,*f7,*f8,*f9,*f10,*f11,*f12;

  if (SIZE(remf)!=12)
  {
    irf = 0;
  }
  else
  {
    irf = MdrV0(remf,0);
  }
  if (!irf)
  {
    f1  = gts_face_new (GTS_FACE_CLASS(color_face_class()),  e1,  e2,  e5);
    ir = 0;
    im = MNR(color) - 1;
    if (color)
    {
      ir = MIN(ir,im);
      c.r = Mdr0(color, ir, 0);
      c.g = Mdr0(color, ir, 1);
      c.b = Mdr0(color, ir, 2);
      ir++;
    }
    COLOR_FACE(f1)->c.r = c.r;
    COLOR_FACE(f1)->c.g = c.g;
    COLOR_FACE(f1)->c.b = c.b;
    gts_surface_add_face (s, f1);
  }

  if (SIZE(remf)!=12)
  {
    irf = 0;
  }
  else
  {
    irf = MdrV0(remf,1);
  }
  if (!irf)
  {
    f2  = gts_face_new (GTS_FACE_CLASS(color_face_class()),  e5,  e3,  e4);
    if (color)
    {
      ir = MIN(ir,im);
      c.r = Mdr0(color, ir, 0);
      c.g = Mdr0(color, ir, 1);
      c.b = Mdr0(color, ir, 2);
      ir++;
    }
    COLOR_FACE(f2)->c.r = c.r;
    COLOR_FACE(f2)->c.g = c.g;
    COLOR_FACE(f2)->c.b = c.b;
    gts_surface_add_face (s, f2);
  }

  if (SIZE(remf)!=12)
  {
    irf = 0;
  }
  else
  {
    irf = MdrV0(remf,2);
  }
  if (!irf)
  {
    f3  = gts_face_new (GTS_FACE_CLASS(color_face_class()),  e6, e10,  e7);
    if (color)
    {
      ir = MIN(ir,im);
      c.r = Mdr0(color, ir, 0);
      c.g = Mdr0(color, ir, 1);
      c.b = Mdr0(color, ir, 2);
      ir++;
    }
    COLOR_FACE(f3)->c.r = c.r;
    COLOR_FACE(f3)->c.g = c.g;
    COLOR_FACE(f3)->c.b = c.b;
    gts_surface_add_face (s, f3);
  }

  if (SIZE(remf)!=12)
  {
    irf = 0;
  }
  else
  {
    irf = MdrV0(remf,3);
  }
  if (!irf)
  {
    f4  = gts_face_new (GTS_FACE_CLASS(color_face_class()), e10,  e9,  e8);
    if (color)
    {
      ir = MIN(ir,im);
      c.r = Mdr0(color, ir, 0);
      c.g = Mdr0(color, ir, 1);
      c.b = Mdr0(color, ir, 2);
      ir++;
    }
    COLOR_FACE(f4)->c.r = c.r;
    COLOR_FACE(f4)->c.g = c.g;
    COLOR_FACE(f4)->c.b = c.b;
    gts_surface_add_face (s, f4);
  }

  if (SIZE(remf)!=12)
  {
    irf = 0;
  }
  else
  {
    irf = MdrV0(remf,4);
  }
  if (!irf)
  {
    f5  = gts_face_new (GTS_FACE_CLASS(color_face_class()),  e2, e15, e12);
    if (color)
    {
      ir = MIN(ir,im);
      c.r = Mdr0(color, ir, 0);
      c.g = Mdr0(color, ir, 1);
      c.b = Mdr0(color, ir, 2);
      ir++;
    }
    COLOR_FACE(f5)->c.r = c.r;
    COLOR_FACE(f5)->c.g = c.g;
    COLOR_FACE(f5)->c.b = c.b;
    gts_surface_add_face (s, f5);
  }

  if (SIZE(remf)!=12)
  {
    irf = 0;
  }
  else
  {
    irf = MdrV0(remf,5);
  }
  if (!irf)
  {
    f6  = gts_face_new (GTS_FACE_CLASS(color_face_class()), e15, e13,  e7);
    if (color)
    {
      ir = MIN(ir,im);
      c.r = Mdr0(color, ir, 0);
      c.g = Mdr0(color, ir, 1);
      c.b = Mdr0(color, ir, 2);
      ir++;
    }
    COLOR_FACE(f6)->c.r = c.r;
    COLOR_FACE(f6)->c.g = c.g;
    COLOR_FACE(f6)->c.b = c.b;
    gts_surface_add_face (s, f6);
  }

  if (SIZE(remf)!=12)
  {
    irf = 0;
  }
  else
  {
    irf = MdrV0(remf,6);
  }
  if (!irf)
  {
    f7  = gts_face_new (GTS_FACE_CLASS(color_face_class()),  e3, e16, e11);
    if (color)
    {
      ir = MIN(ir,im);
      c.r = Mdr0(color, ir, 0);
      c.g = Mdr0(color, ir, 1);
      c.b = Mdr0(color, ir, 2);
      ir++;
    }
    COLOR_FACE(f7)->c.r = c.r;
    COLOR_FACE(f7)->c.g = c.g;
    COLOR_FACE(f7)->c.b = c.b;
    gts_surface_add_face (s, f7);
  }

  if (SIZE(remf)!=12)
  {
    irf = 0;
  }
  else
  {
    irf = MdrV0(remf,7);
  }
  if (!irf)
  {
    f8  = gts_face_new (GTS_FACE_CLASS(color_face_class()), e16, e12,  e8);
    if (color)
    {
      ir = MIN(ir,im);
      c.r = Mdr0(color, ir, 0);
      c.g = Mdr0(color, ir, 1);
      c.b = Mdr0(color, ir, 2);
      ir++;
    }
    COLOR_FACE(f8)->c.r = c.r;
    COLOR_FACE(f8)->c.g = c.g;
    COLOR_FACE(f8)->c.b = c.b;
    gts_surface_add_face (s, f8);
  }

  if (SIZE(remf)!=12)
  {
    irf = 0;
  }
  else
  {
    irf = MdrV0(remf,8);
  }
  if (!irf)
  {
    f9  = gts_face_new (GTS_FACE_CLASS(color_face_class()), e17, e14,  e4);
    if (color)
    {
      ir = MIN(ir,im);
      c.r = Mdr0(color, ir, 0);
      c.g = Mdr0(color, ir, 1);
      c.b = Mdr0(color, ir, 2);
      ir++;
    }
    COLOR_FACE(f9)->c.r = c.r;
    COLOR_FACE(f9)->c.g = c.g;
    COLOR_FACE(f9)->c.b = c.b;
    gts_surface_add_face (s, f9);
  }

  if (SIZE(remf)!=12)
  {
    irf = 0;
  }
  else
  {
    irf = MdrV0(remf,9);
  }
  if (!irf)
  {
    f10 = gts_face_new (GTS_FACE_CLASS(color_face_class()), e17, e11,  e9);
    if (color)
    {
      ir = MIN(ir,im);
      c.r = Mdr0(color, ir, 0);
      c.g = Mdr0(color, ir, 1);
      c.b = Mdr0(color, ir, 2);
      ir++;
    }
    COLOR_FACE(f10)->c.r = c.r;
    COLOR_FACE(f10)->c.g = c.g;
    COLOR_FACE(f10)->c.b = c.b;
    gts_surface_add_face (s, f10);
  }

  if (SIZE(remf)!=12)
  {
    irf = 0;
  }
  else
  {
    irf = MdrV0(remf,10);
  }
  if (!irf)
  {
    f11 = gts_face_new (GTS_FACE_CLASS(color_face_class()), e18, e13,  e1);
    if (color)
    {
      ir = MIN(ir,im);
      c.r = Mdr0(color, ir, 0);
      c.g = Mdr0(color, ir, 1);
      c.b = Mdr0(color, ir, 2);
      ir++;
    }
    COLOR_FACE(f11)->c.r = c.r;
    COLOR_FACE(f11)->c.g = c.g;
    COLOR_FACE(f11)->c.b = c.b;
    gts_surface_add_face (s, f11);
  }

  if (SIZE(remf)!=12)
  {
    irf = 0;
  }
  else
  {
    irf = MdrV0(remf,11);
  }
  if (!irf)
  {
    f12 = gts_face_new (GTS_FACE_CLASS(color_face_class()), e18, e14,  e6);
    if (color)
    {
      ir = MIN(ir,im);
      c.r = Mdr0(color, ir, 0);
      c.g = Mdr0(color, ir, 1);
      c.b = Mdr0(color, ir, 2);
      ir++;
    }
    COLOR_FACE(f12)->c.r = c.r;
    COLOR_FACE(f12)->c.g = c.g;
    COLOR_FACE(f12)->c.b = c.b;
    gts_surface_add_face (s, f12);
  }

  return;
}


//
// create new surface
//
#undef  THIS_SOLVER
#define THIS_SOLVER "gts_surface_modify"
Ent * ent_gts_surface_modify (int nargs, Datum args[])
{
  Ent *e1=0, *e2=0, *e3=0;
  FILE *rlab_stderr = (!RLAB_STDERR_DS) ? stderr : RLAB_STDERR_DS;
  char *name=0;
  GtsColor c = {1.0, 1.0, 1.0};

  r_gts_surface *rf=0;
  r_gts_surface_transform st;
  st.rot_x = 0;
  st.rot_y = 0;
  st.rot_z = 0;
  st.scale = 0;
  st.transl = 0;
  st.center = 0;

  if(!r_gts_surface_list)
  {
    init_r_gts_surface_list();
  }

  //
  if (nargs != 2)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_TWO_ARG_REQUIRED "\n");
    goto _exit;
  }

  //
  // get matrix of rows of points in [x,y,z] format
  //
  e1 = bltin_get_ent(args[0]);
  if (ent_type(e1) != MATRIX_DENSE_STRING)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }
  name = class_char_pointer(e1);
  if (isvalidstring(name) < 1)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }

  //
  // does 'name' already exist in list of named surfaces
  //
  rf = r_gts_surface_list_find (name);
  if (!rf)
  {
    goto _exit;
  }

  //
  // can be followed by the surface description and/or parametrization
  //
  e2 = bltin_get_ent(args[1]);
  if (ent_type(e2) != BTREE)
  {
    goto _exit;
  }

  ListNode *node=0;
  MDR *transl=0, *center=0, *scale=0, *rot_x=0, *rot_y=0, *rot_z=0, *color=0;
  int iterate=0;

  RLABCODE_PROCESS_BTREE_ENTRY_MD(e2,node,RLAB_GTS_NEW_SURFACE_COLOR,color,MDR,SIZE,3,NULL);
  if (color)
  {
    c.r = mdrV0(color,0);
    c.g = mdrV0(color,1);
    c.b = mdrV0(color,2);
  }
  RLABCODE_PROCESS_BTREE_ENTRY_MD(e2,node,RLAB_GTS_NEW_SURFACE_TRANSL,transl,MDR,SIZE,3,NULL);
  if (transl)
  {
    st.transl = transl;
    iterate = 1;
  }
  RLABCODE_PROCESS_BTREE_ENTRY_MD(e2,node,RLAB_GTS_NEW_SURFACE_CENTER,center,MDR,SIZE,3,NULL);
  if (center)
  {
    st.center = center;
  }
  RLABCODE_PROCESS_BTREE_ENTRY_MD(e2,node,RLAB_GTS_NEW_SURFACE_SCALE,scale,MDR,SIZE,3,NULL);
  if (scale)
  {
    st.scale = scale;
    iterate = 1;
  }
  RLABCODE_PROCESS_BTREE_ENTRY_MD(e2,node,RLAB_GTS_NEW_SURFACE_ROT_X,rot_x,MDR,SIZE,9,NULL);
  if (rot_x)
  {
    st.rot_x = rot_x;
    iterate = 1;
  }
  RLABCODE_PROCESS_BTREE_ENTRY_MD(e2,node,RLAB_GTS_NEW_SURFACE_ROT_Y,rot_y,MDR,SIZE,9,NULL);
  if (rot_y)
  {
    st.rot_y = rot_y;
    iterate = 1;
  }
  RLABCODE_PROCESS_BTREE_ENTRY_MD(e2,node,RLAB_GTS_NEW_SURFACE_ROT_Z,rot_z,MDR,SIZE,9,NULL);
  if (rot_z)
  {
    st.rot_z = rot_z;
    iterate = 1;
  }

  if (iterate)
  {
    // perform all point-transformations specified in the struct 'st'
    MDR *vertex = mdr_Create(3,1);
    MDR *result = mdr_Create(3,1);
    st.vertex = vertex;
    st.result = result;
    gts_surface_foreach_vertex(rf->s, (GtsFunc) r_foreach_transform_vertices, (gpointer) &st);
    mdr_Destroy(vertex);
    mdr_Destroy(result);
  }

_exit:

  ent_Clean(e1);
  ent_Clean(e2);
  ent_Clean(e3);
  return ent_Create_Rlab_Success ();
}

//
// add face (triangle) to surface: s1 -> s1 + f
//
#undef  THIS_SOLVER
#define THIS_SOLVER "gts_surface_add_face"
Ent * ent_gts_surface_add_face (int nargs, Datum args[])
{
  Ent *e1=0, *e2=0, *e3=0;
  FILE *rlab_stderr = (!RLAB_STDERR_DS) ? stderr : RLAB_STDERR_DS;
  char *name1=0;
  MDR *f1=0;
  int i;
  gdouble threshold = 0;

  //
  //
  //
  if (nargs<2)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_FOUR_ARG_REQUIRED "\n");
    goto _exit;
  }

  GtsColor c = {1.0, 1.0, 1.0};

  r_gts_surface *rf1=0;

  if(!r_gts_surface_list)
  {
    init_r_gts_surface_list();
  }

  //
  // name of the first surface
  //
  e1 = bltin_get_ent(args[0]);
  if (ent_type(e1) != MATRIX_DENSE_STRING)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }
  name1 = class_char_pointer(e1);
  if (isvalidstring(name1) < 1)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }

  //
  // does 'name' already exist in list of named surfaces
  //
  rf1 = r_gts_surface_list_find (name1);

  if (nargs < 2)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_FOUR_ARG_REQUIRED "\n");
    goto _exit;
  }

  //
  // get the 3points making the face to be added to the surface
  //
  e2 = bltin_get_ent(args[1]);
  if (ent_type(e2) != MATRIX_DENSE_REAL)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG2_ROW_VECTOR_3 "\n");
    goto _exit;
  }
  f1 = ent_data(e2);
  if (SIZE(f1)!=9)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG2_ROW_VECTOR_3 "\n");
    goto _exit;
  }

  //
  // check if the options are provided
  //
  if (nargs > 2)
  {
    e3 = bltin_get_ent(args[nargs-1]);
    if (ent_type(e3) == BTREE)
    {
      ListNode *node;
      MDR * color=0;
      RLABCODE_PROCESS_BTREE_ENTRY_MD_VECTOR(e3,node,RLAB_GTS_NEW_SURFACE_COLOR,color,MDR,3);

      if (color)
      {
        c.r = mdrV0(color,0);
        c.g = mdrV0(color,1);
        c.b = mdrV0(color,2);
      }
    }
  }


  //
  // Create three vertices
  //
  GtsVertex *vx1=0, *vx2=0, *vx3=0;
  if (MNC(f1)==3)
  {
    // f1 =  [vx1; vx2; vx3]
    vx1 = gts_vertex_new(gts_vertex_class(), mdr0(f1,0,0), mdr0(f1,0,1), mdr0(f1,0,2));
    vx2 = gts_vertex_new(gts_vertex_class(), mdr0(f1,1,0), mdr0(f1,1,1), mdr0(f1,1,2));
    vx3 = gts_vertex_new(gts_vertex_class(), mdr0(f1,2,0), mdr0(f1,2,1), mdr0(f1,2,2));
  }
  else
  {
    // f1 =  [vx1, vx2, vx3]
    vx1 = gts_vertex_new(gts_vertex_class(), mdrV0(f1,0), mdrV0(f1,1), mdrV0(f1,2));
    vx2 = gts_vertex_new(gts_vertex_class(), mdrV0(f1,3), mdrV0(f1,4), mdrV0(f1,5));
    vx3 = gts_vertex_new(gts_vertex_class(), mdrV0(f1,6), mdrV0(f1,7), mdrV0(f1,8));
  }

  //
  // Create three edges
  //
  GtsEdge *ed1 = gts_edge_new(gts_edge_class(), vx1, vx2);
  GtsEdge *ed2 = gts_edge_new(gts_edge_class(), vx2, vx3);
  GtsEdge *ed3 = gts_edge_new(gts_edge_class(), vx3, vx1);

  //
  // Create colored face
  //
  GtsFace *f = gts_face_new(GTS_FACE_CLASS(color_face_class()), ed1, ed2, ed3);
  COLOR_FACE(f)->c.r = c.r;
  COLOR_FACE(f)->c.g = c.g;
  COLOR_FACE(f)->c.b = c.b;

  double rval = 0;

  if (!rf1)
  {
    //
    // create new surface with only face 'f'
    //
    rf1 = r_gts_surface_Create ();
    rf1->name = cpstr(name1);
    rf1->s = (GtsSurface *) gts_surface_new (gts_surface_class(),
              GTS_FACE_CLASS(color_face_class()), gts_edge_class(), gts_vertex_class());
    gts_surface_add_face(rf1->s, f);
  }
  else
  {
    //
    // create surface with only face 'f' and add it to the existing surface
    //
    gts_remove_duplicate_vertices(vx1, rf1->s);
    gts_remove_duplicate_vertices(vx2, rf1->s);
    gts_remove_duplicate_vertices(vx3, rf1->s);
    gts_surface_add_face(rf1->s, f);
//     GtsSurface *sdummy = (GtsSurface *) gts_surface_new (gts_surface_class(),
//                           GTS_FACE_CLASS(color_face_class()), gts_edge_class(), gts_vertex_class());
//     gts_surface_add_face(sdummy, f);
//     gts_surface_merge(rf1->s, sdummy);
//     gts_object_destroy(GTS_OBJECT (sdummy));
  }

  if (threshold)
  {
    r_gts_surface_cleanup(rf1->s, threshold);
  }

  rval = 0;

_exit:

  ent_Clean(e1);
  ent_Clean(e2);
  ent_Clean(e3);

  return ent_Create_Rlab_Double ( rval );
}



//
// remove face (triangle) from surface: s1 -> s1 / f
//
#undef  THIS_SOLVER
#define THIS_SOLVER "gts_surface_remove_face"
Ent * ent_gts_surface_remove_face (int nargs, Datum args[])
{
  Ent *e1=0, *e2=0;
  FILE *rlab_stderr = (!RLAB_STDERR_DS) ? stderr : RLAB_STDERR_DS;
  char *name=0;
  MDR *v1=0;
  int i;
  double rval=1;
  gdouble threshold=1e-3;

  r_gts_surface *rf=0;

  if(!r_gts_surface_list)
  {
    init_r_gts_surface_list();
  }

  //
  if (nargs < 2)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_FOUR_ARG_REQUIRED "\n");
    goto _exit;
  }

  //
  // name of the first surface
  //
  e1 = bltin_get_ent(args[0]);
  if (ent_type(e1) != MATRIX_DENSE_STRING)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }
  name = class_char_pointer(e1);
  if (isvalidstring(name) < 1)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }

  //
  // does 'name' already exist in list of named surfaces
  //
  rf = r_gts_surface_list_find (name);
  if (!rf)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": surface '%s' does not exist. Create it first!\n", name);
    goto _exit;
  }

  //
  // get the 3points making the face to be added to the surface
  //
  e2 = bltin_get_ent(args[1]);
  if (ent_type(e2) != MATRIX_DENSE_REAL)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG2_ROW_VECTOR_3 "\n");
    goto _exit;
  }

  v1 = ent_data(e2);
  if (SIZE(v1)!=9)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG2_ROW_VECTOR_3 "\n");
    goto _exit;
  }

  GtsVertex *vx1=0, *vx2=0, *vx3=0;
  if (MNC(v1)==3)
  {
    vx1 = gts_vertex_new(gts_vertex_class(), mdr0(v1,0,0), mdr0(v1,0,1), mdr0(v1,0,2));
    vx2 = gts_vertex_new(gts_vertex_class(), mdr0(v1,1,0), mdr0(v1,1,1), mdr0(v1,1,2));
    vx3 = gts_vertex_new(gts_vertex_class(), mdr0(v1,2,0), mdr0(v1,2,1), mdr0(v1,2,2));
  }
  else
  {
    vx1 = gts_vertex_new(gts_vertex_class(), mdrV0(v1,0), mdrV0(v1,1), mdrV0(v1,2));
    vx2 = gts_vertex_new(gts_vertex_class(), mdrV0(v1,3), mdrV0(v1,4), mdrV0(v1,5));
    vx3 = gts_vertex_new(gts_vertex_class(), mdrV0(v1,6), mdrV0(v1,7), mdrV0(v1,8));
  }

  r_gts_face_vx x;
  x.s = rf->s;
  x.vx1 = vx1;
  x.vx2 = vx2;
  x.vx3 = vx3;
  x.count = 0;

  GtsVertex *v=0;
  v = gts_delaunay_add_vertex (rf->s, vx1, NULL);
  if (v != NULL)
  {
    if (v != vx1)
    {
      gts_vertex_replace (vx1, v);
    }
  }
  v = gts_delaunay_add_vertex (rf->s, vx2, NULL);
  if (v != NULL)
  {
    if (v != vx2)
    {
      gts_vertex_replace (vx2, v);
    }
  }
  v = gts_delaunay_add_vertex (rf->s, vx3, NULL);
  if (v != NULL)
  {
    if (v != vx3)
    {
      gts_vertex_replace (vx3, v);
    }
  }
  if (threshold)
  {
    r_gts_surface_cleanup(rf->s, threshold);
  }

  gts_surface_foreach_face_remove (rf->s, r_gts_remove_face_given_3vertices, &x);

  if (threshold)
  {
    r_gts_surface_cleanup(rf->s, threshold);
  }

  rval = 0;

_exit:

  ent_Clean(e1);
  ent_Clean(e2);
  return ent_Create_Rlab_Double(x.count);
}

//
// add points to surface and delaunay them: s1 -> s1 +{PTS}
//
#undef  THIS_SOLVER
#define THIS_SOLVER "gts_surface_add_point"
Ent * ent_gts_surface_add_point (int nargs, Datum args[])
{
  Ent *e1=0, *e2=0;
  FILE *rlab_stderr = (!RLAB_STDERR_DS) ? stderr : RLAB_STDERR_DS;
  char *name=0;
  MDR *v1=0;
  int i;
  double rval=1;
  gdouble threshold=0.0;

  r_gts_surface *rf=0;

  if(!r_gts_surface_list)
  {
    init_r_gts_surface_list();
  }

  //
  if (nargs < 2)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_FOUR_ARG_REQUIRED "\n");
    goto _exit;
  }

  //
  // name of the first surface
  //
  e1 = bltin_get_ent(args[0]);
  if (ent_type(e1) != MATRIX_DENSE_STRING)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }
  name = class_char_pointer(e1);
  if (isvalidstring(name) < 1)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }

  //
  // does 'name' already exist in list of named surfaces
  //
  rf = r_gts_surface_list_find (name);
  if (!rf)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": surface '%s' does not exist. Create it first!\n", name);
    goto _exit;
  }

  //
  // get the 3points making the face to be added to the surface
  //
  e2 = bltin_get_ent(args[1]);
  if (ent_type(e2) != MATRIX_DENSE_REAL)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG2_ROW_VECTOR_3 "\n");
    goto _exit;
  }

  v1 = ent_data(e2);
  if (MNC(v1)!=3)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG2_ROW_VECTOR_3 "\n");
    goto _exit;
  }

  GtsVertex *v=0, *vx=0;
  for (i=0; i<MNR(v1); i++)
  {
    vx = gts_vertex_new(gts_vertex_class(), mdr0(v1,i,0), mdr0(v1,i,1), mdr0(v1,i,2));
    v  = gts_delaunay_add_vertex (rf->s, vx, NULL);
    if (v != NULL)
    {
      if (v != vx)
      {
        gts_vertex_replace (vx, v);
      }
    }
  }
  if (threshold)
  {
    r_gts_surface_cleanup(rf->s, threshold);
  }

  rval = 0;

_exit:

  ent_Clean(e1);
  ent_Clean(e2);
  return ent_Create_Rlab_Double(rval);
}

static int r_gts_remove_face_from_surface(gpointer item, gpointer data)
{
  // data: surface
  r_gts_void_ptrs * x = (r_gts_void_ptrs * ) data;
  GtsSurface *s = x->vptr1;
  int *what_to_do = x->vptr2;
  GtsVertex *v=0, *vx1=0, *vx2=0, *vx3=0;

  // item: triangle
  GtsTriangle *t = (GtsTriangle *) item;
  gts_triangle_vertices ( t, &vx1, &vx2, &vx3);

  r_gts_face_vx y;
  y.vx1 = vx1;
  y.vx2 = vx2;
  y.vx3 = vx3;
  y.count = 0;

  switch ( *what_to_do)
  {
    case 0:
      v = gts_delaunay_add_vertex (s, vx1, NULL);
      if (v != NULL)
      {
        if (v != vx1)
        {
          gts_vertex_replace (vx1, v);
        }
      }
      v = gts_delaunay_add_vertex (s, vx2, NULL);
      if (v != NULL)
      {
        if (v != vx2)
        {
          gts_vertex_replace (vx2, v);
        }
      }
      v = gts_delaunay_add_vertex (s, vx3, NULL);
      if (v != NULL)
      {
        if (v != vx3)
        {
          gts_vertex_replace (vx3, v);
        }
      }
      break;

    default:
      gts_surface_foreach_face_remove (s, r_gts_remove_face_given_3vertices, &y);
      break;
  }


//   GtsEdge * e1 = new_edge (vx1, vx2);
//   GtsEdge * e2 = new_edge (vx2, vx3);
//   GtsEdge * e2 = new_edge (vx3, vx1);
//   gts_surface_add_face (s, gts_face_new (s->face_class, e1, e2, e3));

//   if (*what_to_do)
//   {
//
//   }

  return 0;
}

//
// remove face (triangle) from surface: s1 -> s1 / f
//
#undef  THIS_SOLVER
#define THIS_SOLVER "gts_surface_remove_surface"
Ent * ent_gts_surface_remove_surface (int nargs, Datum args[])
{
  Ent *e1=0, *e2=0;
  FILE *rlab_stderr = (!RLAB_STDERR_DS) ? stderr : RLAB_STDERR_DS;
  char *name1=0, *name2=0;
  MDR *v1=0;
  int i;
  double rval=1;
  gdouble threshold=1e-3;

  r_gts_surface *rf1=0, *rf2=0;

  if(!r_gts_surface_list)
  {
    init_r_gts_surface_list();
  }

  //
  if (nargs < 2)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_FOUR_ARG_REQUIRED "\n");
    goto _exit;
  }

  //
  // name of the first surface
  //
  e1 = bltin_get_ent(args[0]);
  if (ent_type(e1) != MATRIX_DENSE_STRING)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }
  name1 = class_char_pointer(e1);
  if (isvalidstring(name1) < 1)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }

  //
  // does 'name' already exist in list of named surfaces
  //
  rf1 = r_gts_surface_list_find (name1);
  if (!rf1)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": surface '%s' does not exist. Create it first!\n", name1);
    goto _exit;
  }

  //
  // get the 3points making the face to be added to the surface
  //
  e2 = bltin_get_ent(args[1]);
  if (ent_type(e2) != MATRIX_DENSE_STRING)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG2_STRING_OBJNAME "\n");
    goto _exit;
  }
  name2 = class_char_pointer(e2);
  if (isvalidstring(name2) < 1)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG2_STRING_OBJNAME "\n");
    goto _exit;
  }
  rf2 = r_gts_surface_list_find (name2);
  if (!rf2)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": surface '%s' does not exist. Create it first!\n", name2);
    goto _exit;
  }

  //
  // add vertices of surface 2 to delaunay tringulation of the surface 1
  //
  int what_to_do = 0; // for adding the vertices
  r_gts_void_ptrs dataptr;
  dataptr.vptr1 = rf1->s;
  dataptr.vptr2 = &what_to_do;
  gts_surface_foreach_face (rf2->s, r_gts_remove_face_from_surface, &dataptr);
  if (threshold)
  {
    r_gts_surface_cleanup(rf2->s, threshold);
  }

  what_to_do = 1; // after all vertices have been added, remove the faces
  gts_surface_foreach_face (rf2->s, r_gts_remove_face_from_surface, &dataptr);
  if (threshold)
  {
    r_gts_surface_cleanup(rf2->s, threshold);
  }

  rval = 0;

_exit:

  ent_Clean(e1);
  ent_Clean(e2);
  return ent_Create_Rlab_Double(rval);
}

//
// copy: s2 -> s1
//
#undef  THIS_SOLVER
#define THIS_SOLVER "gts_surface_copy"
Ent * ent_gts_surface_copy (int nargs, Datum args[])
{
  Ent *e1=0, *e2=0;
  FILE *rlab_stderr = (!RLAB_STDERR_DS) ? stderr : RLAB_STDERR_DS;
  char *name1=0, *name2=0;

  r_gts_surface *rf1=0, *rf2=0;

  if(!r_gts_surface_list)
  {
    init_r_gts_surface_list();
  }

  //
  if (nargs != 2)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_TWO_ARG_REQUIRED "\n");
    goto _exit;
  }

  e1 = bltin_get_ent(args[0]);
  if (ent_type(e1) != MATRIX_DENSE_STRING)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }
  name1 = class_char_pointer(e1);
  if (isvalidstring(name1) < 1)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }

  e2 = bltin_get_ent(args[1]);
  if (ent_type(e2) != MATRIX_DENSE_STRING)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }
  name2 = class_char_pointer(e2);
  if (isvalidstring(name2) < 1)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG2_STRING_OBJNAME "\n");
    goto _exit;
  }

  //
  // does 'name1' already exist in list of named surfaces
  //
  rf2 = r_gts_surface_list_find (name2);
  if (!rf2)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": Second surface is empty. Cannot copy it to first surface!\n");
    goto _exit;
  }

  //
  // does 'name1' already exist in list of named surfaces
  //
  rf1 = r_gts_surface_list_find (name1);
  if (rf1)
  {
    r_gts_surface_Destroy (name1);
    rf1 = 0;
  }
  if (!rf1)
  {
    rf1 = r_gts_surface_Create ();
    rf1->name = cpstr(name1);
    rf1->s = (GtsSurface *) gts_surface_new (gts_surface_class(),
              GTS_FACE_CLASS(color_face_class()), gts_edge_class(), gts_vertex_class());
  }

  rf1->s = gts_surface_copy(rf1->s, rf2->s);

_exit:

  ent_Clean(e1);
  ent_Clean(e2);
  return ent_Create_Rlab_Success ();
}

//
// intersect: s1 ^ s2 -> collection of edges
//
// find color of the edge from the second surface
static void copy_face_color(GtsTriangle * t, void **f_args)
{
  GtsVertex *v1 = f_args[0];
  GtsVertex *v2 = f_args[1];
  int *count = (int *) f_args[2];
  MDR *color = (MDR *) f_args[3];
  MDR *tri   = (MDR *) f_args[4];
  double d1=0, d2;

  d1 = gts_point_triangle_distance (GTS_POINT(v1), t);
  d2 = gts_point_triangle_distance (GTS_POINT(v2), t);
  if (d1 > r_gts_eabs)
  {
    return;
  }
  if (d2 > r_gts_eabs)
  {
    return;
  }
  GtsFace *f = GTS_FACE(t);
  // copy triangle face
  Mdr0(color, (*count), 0) = COLOR_FACE(f)->c.r;
  Mdr0(color, (*count), 1) = COLOR_FACE(f)->c.g;
  Mdr0(color, (*count), 2) = COLOR_FACE(f)->c.b;
  // copy parent triangle coordinates
  GtsVertex *x1, *x2, *x3;
  gts_triangle_vertices (t, &x1, &x2, &x3);
  Mdr0(tri, (*count), 0)   = x1->p.x;
  Mdr0(tri, (*count), 1)   = x1->p.y;
  Mdr0(tri, (*count), 2)   = x1->p.z;
  Mdr0(tri, (*count), 3)   = x2->p.x;
  Mdr0(tri, (*count), 4)   = x2->p.y;
  Mdr0(tri, (*count), 5)   = x2->p.z;
  Mdr0(tri, (*count), 6)   = x3->p.x;
  Mdr0(tri, (*count), 7)   = x3->p.y;
  Mdr0(tri, (*count), 8)   = x3->p.z;
  return;
}

static void prepend_triangle_bbox (GtsTriangle * t, GSList ** bboxes)
{
  *bboxes = g_slist_prepend (*bboxes, gts_bbox_triangle (gts_bbox_class (), t));
  return;
}

static void count_edges (GtsSegment * s, int * n)
{
  (*n)++;
  return;
}

static void copy_GtsSegment_2_MDR (GtsSegment * s, void ** f_args)
{
  int *count = (int *) f_args[0];
  MDR *edges = (MDR *) f_args[1];
  MDR *color = (MDR *) f_args[2];
  GtsSurface *surface = (GtsSurface *) f_args[3];
  MDR *tri   = (MDR *) f_args[4];
  if ( (MNC(edges) == 6) && ((*count) < MNR(edges)) )
  {
    // copy edges
    Mdr0(edges,(*count),0) = GTS_POINT (s->v1)->x;
    Mdr0(edges,(*count),1) = GTS_POINT (s->v1)->y;
    Mdr0(edges,(*count),2) = GTS_POINT (s->v1)->z;
    Mdr0(edges,(*count),3) = GTS_POINT (s->v2)->x;
    Mdr0(edges,(*count),4) = GTS_POINT (s->v2)->y;
    Mdr0(edges,(*count),5) = GTS_POINT (s->v2)->z;
    //
    // go over 'surface' and find its color
    //
    void *data[5];
    data[0] = (void *) s->v1;
    data[1] = (void *) s->v2;
    data[2] = (void *) count;
    data[3] = (void *) color;
    data[4] = (void *) tri;
    gts_surface_foreach_face (surface, (GtsFunc) copy_face_color, data);
  }
  else
  {
    fprintf(stderr, "copy_GtsEdge_2_MDR: Horrible internal error: Index '*count' (%i) out of bounds (%i)!\n",
           *count, MNR(edges));
  }

  (*count)++;

  return;
}

#undef  THIS_SOLVER
#define THIS_SOLVER "gts_surface_intersect"
Ent * ent_gts_surface_intersect (int nargs, Datum args[])
{
  Ent *e1=0, *e2=0;
  FILE *rlab_stderr = (!RLAB_STDERR_DS) ? stderr : RLAB_STDERR_DS;
  char *name1=0, *name2=0;

  r_gts_surface *rf1=0, *rf2=0;

  /* Based on test/boolean/set.c*/
  GtsSurfaceInter *si=0;
  GNode *tree1=0, *tree2=0;
  gboolean is_open1=0, is_open2=0;
  GSList * bboxes;
  MDR *edges=0, *cs=0, *tri=0;

  if(!r_gts_surface_list)
  {
    init_r_gts_surface_list();
  }

  //
  if (nargs != 2)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_TWO_ARG_REQUIRED "\n");
    goto _exit;
  }

  e1 = bltin_get_ent(args[0]);
  if (ent_type(e1) != MATRIX_DENSE_STRING)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }
  name1 = class_char_pointer(e1);
  if (isvalidstring(name1) < 1)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }

  e2 = bltin_get_ent(args[1]);
  if (ent_type(e2) != MATRIX_DENSE_STRING)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }
  name2 = class_char_pointer(e2);
  if (isvalidstring(name2) < 1)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG2_STRING_OBJNAME "\n");
    goto _exit;
  }

  //
  // does 'name1' already exist in list of named surfaces
  //
  rf2 = r_gts_surface_list_find (name2);
  if (!rf2)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG2_STRING_OBJNAME "\n");
    goto _exit;
  }

  //
  // does 'name1' already exist in list of named surfaces
  //
  rf1 = r_gts_surface_list_find (name1);
  if (!rf1)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }

//   fprintf(stderr, "1\n");

  /* check surfaces */
  if (!gts_surface_is_orientable (rf1->s) || !gts_surface_is_orientable (rf2->s))
  {
    fprintf (rlab_stderr, THIS_SOLVER ": One of the surfaces is not orientable. Cannot continue!\n");
    goto _exit;
  }

//   fprintf(stderr, "2:self-intersecting=%i,%i\n",
//           gts_surface_is_self_intersecting (rf1->s),
//           gts_surface_is_self_intersecting (rf2->s));

  if (gts_surface_is_self_intersecting (rf1->s))
  {
    fprintf (rlab_stderr, THIS_SOLVER ": ARG! Surface '%s' is self intersecting. Cannot continue!\n", rf1->name);
    goto _exit;
  }
  if (gts_surface_is_self_intersecting(rf2->s))
  {
    fprintf (rlab_stderr, THIS_SOLVER ": ARG2 Surface '%s' is self intersecting. Cannot continue!\n", rf2->name);
    goto _exit;
  }

  /* build bounding boxes for first surface */
  bboxes = NULL;
  gts_surface_foreach_face (rf1->s, (GtsFunc) prepend_triangle_bbox, &bboxes);
  tree1 = gts_bb_tree_new (bboxes);
  g_slist_free (bboxes);
  is_open1 = gts_surface_volume (rf1->s) < 0. ? TRUE : FALSE;
  /* build bounding boxes for second surface */
  bboxes = NULL;
  gts_surface_foreach_face (rf2->s, (GtsFunc) prepend_triangle_bbox, &bboxes);
  tree2 = gts_bb_tree_new (bboxes);
  g_slist_free (bboxes);
  is_open2 = gts_surface_volume (rf2->s) < 0. ? TRUE : FALSE;

//   fprintf(stderr, "3:is_open1,2=%i,%i\n", is_open1, is_open2);
//   fprintf(stderr, "3:surface(1)=%p [%p]\n", rf1->s, tree1);
//   fprintf(stderr, "3:surface(1)=\n");
//   gts_surface_write (rf1->s, stderr);
//   fprintf(stderr, "3:surface(2)=\n");
//   gts_surface_write (rf2->s, stderr);
//   fprintf(stderr, "3:surface(2)=%p [%p]\n", rf2->s, tree2);
//   fprintf(stderr, "3:calculating intersection:\n");
//   fflush(stderr);

  GSList * int_edges = gts_surface_intersection (rf1->s, rf2->s, tree1, tree2);
  int n_edges=0, count=0;
  g_slist_foreach (int_edges, (GFunc) count_edges, &n_edges);
//   fprintf(stderr, "2: n_edges=%i\n", n_edges);
  //
  if (n_edges)
  {
    void *f_args[5];
    edges = mdr_Create(n_edges, 6);
    cs    = mdr_Create(n_edges, 3);
    mdr_Zero(cs);
    tri   = mdr_Create(n_edges, 9);
    count = 0;
    f_args[0] = (void *) &count;
    f_args[1] = (void *) edges;
    f_args[2] = (void *) cs;
    f_args[3] = (void *) rf2->s;
    f_args[4] = (void *) tri;
//     fprintf(stderr, "2: before copy\n");
    g_slist_foreach (int_edges, (GFunc) copy_GtsSegment_2_MDR, f_args);
//     fprintf(stderr, "2: after copy\n");
  }

  /* destroy intersection */

  /* destroy bounding box trees (including bounding boxes) */
  gts_bb_tree_destroy (tree1, TRUE);
  gts_bb_tree_destroy (tree2, TRUE);
  g_slist_free(int_edges);

_exit:

  ent_Clean(e1);
  ent_Clean(e2);

  Btree * rval = btree_Create();
  // intersecting edge
  install(rval, RLAB_GTS_EDGE, ent_Assign_Rlab_MDR(edges));
  // parent face of the surface 2
  install(rval, RLAB_GTS_FACE, ent_Assign_Rlab_MDR(tri));
  // color of the parent face of the surface 2
  install(rval, RLAB_GTS_FACE_COLOR, ent_Assign_Rlab_MDR(cs));
  return ent_Assign_Rlab_BTREE(rval);
}

//
// merge: s1 -> s1 + s2
//
#undef  THIS_SOLVER
#define THIS_SOLVER "gts_surface_merge"
Ent * ent_gts_surface_merge (int nargs, Datum args[])
{
  Ent *e1=0, *e2=0;
  FILE *rlab_stderr = (!RLAB_STDERR_DS) ? stderr : RLAB_STDERR_DS;
  char *name1=0, *name2=0;

  r_gts_surface *rf1=0, *rf2=0;
  gdouble threshold=0.0;

  if(!r_gts_surface_list)
  {
    init_r_gts_surface_list();
  }

  //
  if (nargs != 2)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_TWO_ARG_REQUIRED "\n");
    goto _exit;
  }

  e1 = bltin_get_ent(args[0]);
  if (ent_type(e1) != MATRIX_DENSE_STRING)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }
  name1 = class_char_pointer(e1);
  if (isvalidstring(name1) < 1)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }

  e2 = bltin_get_ent(args[1]);
  if (ent_type(e2) != MATRIX_DENSE_STRING)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }
  name2 = class_char_pointer(e2);
  if (isvalidstring(name2) < 1)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG2_STRING_OBJNAME "\n");
    goto _exit;
  }

  //
  // does 'name1' already exist in list of named surfaces
  //
  rf2 = r_gts_surface_list_find (name2);
  if (!rf2)
  {
    goto _exit;
  }

  //
  // does 'name1' already exist in list of named surfaces
  //
  rf1 = r_gts_surface_list_find (name1);
  if (!rf1)
  {
    rf1 = r_gts_surface_Create ();
    rf1->name = cpstr(name1);
    rf1->s = (GtsSurface *) gts_surface_new (gts_surface_class(),
              GTS_FACE_CLASS(color_face_class()), gts_edge_class(), gts_vertex_class());
  }

  gts_surface_merge(rf1->s, rf2->s);
  if (threshold)
  {
    r_gts_surface_cleanup(rf1->s, threshold);
  }

_exit:

  ent_Clean(e1);
  ent_Clean(e2);
  return ent_Create_Rlab_Success ();
}

//
// is_{manifold,orientable,closed}: s1 -> {-1,0,1}
//
#undef  THIS_SOLVER
#define THIS_SOLVER "gts_surface_is"
Ent * ent_gts_surface_is (int nargs, Datum args[])
{
  Ent *e1=0, *e2=0;
  FILE *rlab_stderr = (!RLAB_STDERR_DS) ? stderr : RLAB_STDERR_DS;
  char *name1=0, *op=0;
  double result = -1;

  r_gts_surface *rf1=0, *rf2=0;

  if(!r_gts_surface_list)
  {
    init_r_gts_surface_list();
  }

  //
  if (nargs != 2)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_TWO_ARG_REQUIRED "\n");
    goto _exit;
  }

  e1 = bltin_get_ent(args[0]);
  if (ent_type(e1) != MATRIX_DENSE_STRING)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }
  name1 = class_char_pointer(e1);
  if (isvalidstring(name1) < 1)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }

  e2 = bltin_get_ent(args[1]);
  if (ent_type(e2) != MATRIX_DENSE_STRING)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG2_STRING_OBJNAME "\n");
    goto _exit;
  }
  op = class_char_pointer(e2);
  if (isvalidstring(name1) < 1)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG2_STRING_OBJNAME "\n");
    goto _exit;
  }

  //
  // does 'name1' already exist in list of named surfaces
  //
  rf1 = r_gts_surface_list_find (name1);
  if (!rf1)
  {
    rf1 = r_gts_surface_Create ();
    rf1->name = cpstr(name1);
    rf1->s = (GtsSurface *) gts_surface_new (gts_surface_class(), GTS_FACE_CLASS(color_face_class()), gts_edge_class(), gts_vertex_class());
  }

  if (strstr(op, "man"))
  {
    result = gts_surface_is_manifold (rf1->s);
  }
  else if (strstr(op, "ori"))
  {
    result = gts_surface_is_orientable (rf1->s);
  }
  else if (strstr(op, "clo"))
  {
    result = gts_surface_is_closed (rf1->s);
  }

_exit:

  ent_Clean(e1);
  ent_Clean(e2);
  return ent_Create_Rlab_Double(result);
}

//
// count edges, vertices, faces of a surface
//
#undef  THIS_SOLVER
#define THIS_SOLVER "gts_surface_info"
Ent * ent_gts_surface_info (int nargs, Datum args[])
{
  Ent *e1=0;
  FILE *rlab_stderr = (!RLAB_STDERR_DS) ? stderr : RLAB_STDERR_DS;
  char *name1=0;
  double nv=-1, ne=-1, nf=-1;

  r_gts_surface *rf1=0;

  if(!r_gts_surface_list)
  {
    init_r_gts_surface_list();
  }

  //
  if (nargs != 1)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ONE_ARG_REQUIRED "\n");
    goto _exit;
  }

  e1 = bltin_get_ent(args[0]);
  if (ent_type(e1) != MATRIX_DENSE_STRING)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }
  name1 = class_char_pointer(e1);
  if (isvalidstring(name1) < 1)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }

  //
  // does 'name1' already exist in list of named surfaces
  //
  rf1 = r_gts_surface_list_find (name1);
  if (!rf1)
  {
    rf1 = r_gts_surface_Create ();
    rf1->name = cpstr(name1);
    rf1->s = (GtsSurface *) gts_surface_new (gts_surface_class(), GTS_FACE_CLASS(color_face_class()), gts_edge_class(), gts_vertex_class());
  }

  nv = gts_surface_vertex_number(rf1->s);
  ne = gts_surface_edge_number(rf1->s);
  nf = gts_surface_face_number(rf1->s);

_exit:

  ent_Clean(e1);

  Btree * rval = btree_Create();
  install(rval, RLAB_GTS_NR_EDGES,    ent_Create_Rlab_Double(ne));
  install(rval, RLAB_GTS_NR_VERTICES, ent_Create_Rlab_Double(nv));
  install(rval, RLAB_GTS_NR_FACES,    ent_Create_Rlab_Double(nf));
  return ent_Assign_Rlab_BTREE(rval);
}

//
// area of surface
//
#undef  THIS_SOLVER
#define THIS_SOLVER "gts_surface_calc_area"
Ent * ent_gts_surface_calc_area (int nargs, Datum args[])
{
  Ent *e1=0;
  FILE *rlab_stderr = (!RLAB_STDERR_DS) ? stderr : RLAB_STDERR_DS;
  char *name1=0;
  double result = create_nan();

  r_gts_surface *rf1=0;

  if(!r_gts_surface_list)
  {
    init_r_gts_surface_list();
  }

  //
  if (nargs != 1)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ONE_ARG_REQUIRED "\n");
    goto _exit;
  }

  e1 = bltin_get_ent(args[0]);
  if (ent_type(e1) != MATRIX_DENSE_STRING)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }
  name1 = class_char_pointer(e1);
  if (isvalidstring(name1) < 1)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }

  //
  // does 'name1' already exist in list of named surfaces
  //
  rf1 = r_gts_surface_list_find (name1);
  if (!rf1)
  {
    rf1 = r_gts_surface_Create ();
    rf1->name = cpstr(name1);
    rf1->s = (GtsSurface *) gts_surface_new (gts_surface_class(), GTS_FACE_CLASS(color_face_class()), gts_edge_class(), gts_vertex_class());
  }

  result = gts_surface_area(rf1->s);

_exit:

  ent_Clean(e1);

  return ent_Create_Rlab_Double(result);
}

//
// area of surface
//
#undef  THIS_SOLVER
#define THIS_SOLVER "gts_surface_calc_volume"
Ent * ent_gts_surface_calc_volume (int nargs, Datum args[])
{
  Ent *e1=0;
  FILE *rlab_stderr = (!RLAB_STDERR_DS) ? stderr : RLAB_STDERR_DS;
  char *name1=0;
  double result = create_nan();

  r_gts_surface *rf1=0;

  if(!r_gts_surface_list)
  {
    init_r_gts_surface_list();
  }

  //
  if (nargs != 1)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ONE_ARG_REQUIRED "\n");
    goto _exit;
  }

  e1 = bltin_get_ent(args[0]);
  if (ent_type(e1) != MATRIX_DENSE_STRING)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }
  name1 = class_char_pointer(e1);
  if (isvalidstring(name1) < 1)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }

  //
  // does 'name1' already exist in list of named surfaces
  //
  rf1 = r_gts_surface_list_find (name1);
  if (!rf1)
  {
    rf1 = r_gts_surface_Create ();
    rf1->name = cpstr(name1);
    rf1->s = (GtsSurface *) gts_surface_new (gts_surface_class(), GTS_FACE_CLASS(color_face_class()), gts_edge_class(), gts_vertex_class());
  }

  result = gts_surface_volume (rf1->s);

_exit:

  ent_Clean(e1);

  return ent_Create_Rlab_Double(result);
}

//
// center of mass
//
#undef  THIS_SOLVER
#define THIS_SOLVER "gts_surface_calc_center_of_mass"
Ent * ent_gts_surface_calc_center_of_mass (int nargs, Datum args[])
{
  Ent *e1=0;
  FILE *rlab_stderr = (!RLAB_STDERR_DS) ? stderr : RLAB_STDERR_DS;
  char *name1=0;
  MDR *result=0;

  r_gts_surface *rf1=0;

  if(!r_gts_surface_list)
  {
    init_r_gts_surface_list();
  }

  //
  if (nargs != 1)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ONE_ARG_REQUIRED "\n");
    goto _exit;
  }

  e1 = bltin_get_ent(args[0]);
  if (ent_type(e1) != MATRIX_DENSE_STRING)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }
  name1 = class_char_pointer(e1);
  if (isvalidstring(name1) < 1)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }

  //
  // does 'name1' already exist in list of named surfaces
  //
  rf1 = r_gts_surface_list_find (name1);
  if (!rf1)
  {
    rf1 = r_gts_surface_Create ();
    rf1->name = cpstr(name1);
    rf1->s = (GtsSurface *) gts_surface_new (gts_surface_class(), GTS_FACE_CLASS(color_face_class()), gts_edge_class(), gts_vertex_class());
  }

  result = mdr_Create(1,3);
  gts_surface_center_of_mass (rf1->s, MDRPTR(result));

_exit:

  ent_Clean(e1);

  return ent_Assign_Rlab_MDR(result);
}

//
// center of mass
//
#undef  THIS_SOLVER
#define THIS_SOLVER "gts_surface_calc_center_of_area"
Ent * ent_gts_surface_calc_center_of_area (int nargs, Datum args[])
{
  Ent *e1=0;
  FILE *rlab_stderr = (!RLAB_STDERR_DS) ? stderr : RLAB_STDERR_DS;
  char *name1=0;
  MDR *result=0;

  r_gts_surface *rf1=0;

  if(!r_gts_surface_list)
  {
    init_r_gts_surface_list();
  }

  //
  if (nargs != 1)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ONE_ARG_REQUIRED "\n");
    goto _exit;
  }

  e1 = bltin_get_ent(args[0]);
  if (ent_type(e1) != MATRIX_DENSE_STRING)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }
  name1 = class_char_pointer(e1);
  if (isvalidstring(name1) < 1)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }

  //
  // does 'name1' already exist in list of named surfaces
  //
  rf1 = r_gts_surface_list_find (name1);
  if (!rf1)
  {
    rf1 = r_gts_surface_Create ();
    rf1->name = cpstr(name1);
    rf1->s = (GtsSurface *) gts_surface_new (gts_surface_class(), GTS_FACE_CLASS(color_face_class()), gts_edge_class(), gts_vertex_class());
  }

  result = mdr_Create(1,3);
  gts_surface_center_of_area (rf1->s, MDRPTR(result));

_exit:

  ent_Clean(e1);

  return ent_Assign_Rlab_MDR(result);
}

//
// count edges, vertices, faces of a surface
//
#undef  THIS_SOLVER
#define THIS_SOLVER "gts_surface_stats"
Ent * ent_gts_surface_stats (int nargs, Datum args[])
{
  Ent *e1=0;
  FILE *rlab_stderr = (!RLAB_STDERR_DS) ? stderr : RLAB_STDERR_DS;
  char *name1=0;
  double nv=-1, ne=-1, nf=-1;

  Btree * rval = btree_Create();

  r_gts_surface *rf1=0;

  if(!r_gts_surface_list)
  {
    init_r_gts_surface_list();
  }

  //
  if (nargs != 1)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ONE_ARG_REQUIRED "\n");
    goto _exit;
  }

  e1 = bltin_get_ent(args[0]);
  if (ent_type(e1) != MATRIX_DENSE_STRING)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }
  name1 = class_char_pointer(e1);
  if (isvalidstring(name1) < 1)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }

  //
  // does 'name1' already exist in list of named surfaces
  //
  rf1 = r_gts_surface_list_find (name1);
  if (!rf1)
  {
    rf1 = r_gts_surface_Create ();
    rf1->name = cpstr(name1);
    rf1->s = (GtsSurface *) gts_surface_new (gts_surface_class(), GTS_FACE_CLASS(color_face_class()),
              gts_edge_class(), gts_vertex_class());
  }

  nv = gts_surface_vertex_number(rf1->s);
  ne = gts_surface_edge_number(rf1->s);
  nf = gts_surface_face_number(rf1->s);

  GtsSurfaceStats stats;
  gts_surface_stats(rf1->s, (GtsSurfaceStats *) &stats);

  install(rval, RLAB_GTS_NR_DUP_EDGES, ent_Create_Rlab_Double(stats.n_duplicate_edges));
  install(rval, RLAB_GTS_NR_DUP_FACES, ent_Create_Rlab_Double(stats.n_duplicate_faces));
  install(rval, RLAB_GTS_NR_INC_FACES, ent_Create_Rlab_Double(stats.n_incompatible_faces));
  install(rval, RLAB_GTS_NR_BND_EDGES, ent_Create_Rlab_Double(stats.n_boundary_edges));
  install(rval, RLAB_GTS_NR_NON_MFLD_EDGES, ent_Create_Rlab_Double(stats.n_non_manifold_edges));

//   install(rval, RLAB_GTS_FACE_Q,      ent_Create_Rlab_Double(stats.face_quality));
//   install(rval, RLAB_GTS_FACE_AREA,   ent_Create_Rlab_Double(stats.face_area));
//   install(rval, RLAB_GTS_EDGE_LEN,    ent_Create_Rlab_Double(stats.edge_length));
//   install(rval, RLAB_GTS_EDGE_ANGLE,  ent_Create_Rlab_Double(stats.edge_angle));

_exit:

  ent_Clean(e1);

  install(rval, RLAB_GTS_NR_EDGES,    ent_Create_Rlab_Double(ne));
  install(rval, RLAB_GTS_NR_VERTICES, ent_Create_Rlab_Double(nv));
  install(rval, RLAB_GTS_NR_FACES,    ent_Create_Rlab_Double(nf));

  return ent_Assign_Rlab_BTREE(rval);
}

//
// count edges, vertices, faces of a surface
//
#undef  THIS_SOLVER
#define THIS_SOLVER "gts_surface_qstats"
Ent * ent_gts_surface_qstats (int nargs, Datum args[])
{
  Ent *e1=0;
  FILE *rlab_stderr = (!RLAB_STDERR_DS) ? stderr : RLAB_STDERR_DS;
  char *name1=0;
  double nv=-1, ne=-1, nf=-1;

  Btree * rval = btree_Create();

  r_gts_surface *rf1=0;

  if(!r_gts_surface_list)
  {
    init_r_gts_surface_list();
  }

  //
  if (nargs != 1)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ONE_ARG_REQUIRED "\n");
    goto _exit;
  }

  e1 = bltin_get_ent(args[0]);
  if (ent_type(e1) != MATRIX_DENSE_STRING)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }
  name1 = class_char_pointer(e1);
  if (isvalidstring(name1) < 1)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }

  //
  // does 'name1' already exist in list of named surfaces
  //
  rf1 = r_gts_surface_list_find (name1);
  if (!rf1)
  {
    rf1 = r_gts_surface_Create ();
    rf1->name = cpstr(name1);
    rf1->s = (GtsSurface *) gts_surface_new (gts_surface_class(), GTS_FACE_CLASS(color_face_class()), gts_edge_class(), gts_vertex_class());
  }

  nv = gts_surface_vertex_number(rf1->s);
  ne = gts_surface_edge_number(rf1->s);
  nf = gts_surface_face_number(rf1->s);

  GtsSurfaceQualityStats qstats;
  gts_surface_quality_stats(rf1->s, (GtsSurfaceQualityStats *) &qstats);

  Btree * subvalq = btree_Create();
  install(subvalq, "min",   ent_Create_Rlab_Double(qstats.face_quality.min));
  install(subvalq, "max",   ent_Create_Rlab_Double(qstats.face_quality.max));
  install(subvalq, "avg",   ent_Create_Rlab_Double(qstats.face_quality.mean));
  install(subvalq, "std",   ent_Create_Rlab_Double(qstats.face_quality.stddev));
  install(subvalq, "n",   ent_Create_Rlab_Double(qstats.face_quality.n));
  install(rval, RLAB_GTS_FACE_Q, ent_Assign_Rlab_BTREE(subvalq));

  Btree * subvala = btree_Create();
  install(subvala, "min",   ent_Create_Rlab_Double(qstats.face_area.min));
  install(subvala, "max",   ent_Create_Rlab_Double(qstats.face_area.max));
  install(subvala, "avg",   ent_Create_Rlab_Double(qstats.face_area.mean));
  install(subvala, "std",   ent_Create_Rlab_Double(qstats.face_area.stddev));
  install(subvala, "n",   ent_Create_Rlab_Double(qstats.face_area.n));
  install(rval, RLAB_GTS_FACE_AREA, ent_Assign_Rlab_BTREE(subvala));

  Btree * subvall = btree_Create();
  install(subvall, "min",   ent_Create_Rlab_Double(qstats.edge_length.min));
  install(subvall, "max",   ent_Create_Rlab_Double(qstats.edge_length.max));
  install(subvall, "avg",   ent_Create_Rlab_Double(qstats.edge_length.mean));
  install(subvall, "std",   ent_Create_Rlab_Double(qstats.edge_length.stddev));
  install(subvall, "n",   ent_Create_Rlab_Double(qstats.edge_length.n));
  install(rval, RLAB_GTS_EDGE_LEN, ent_Assign_Rlab_BTREE(subvall));

  Btree * subvale = btree_Create();
  install(subvale, "min",   ent_Create_Rlab_Double(qstats.edge_angle.min));
  install(subvale, "max",   ent_Create_Rlab_Double(qstats.edge_angle.max));
  install(subvale, "avg",   ent_Create_Rlab_Double(qstats.edge_angle.mean));
  install(subvale, "std",   ent_Create_Rlab_Double(qstats.edge_angle.stddev));
  install(subvale, "n",   ent_Create_Rlab_Double(qstats.edge_angle.n));
  install(rval, RLAB_GTS_EDGE_ANGLE, ent_Assign_Rlab_BTREE(subvale));

_exit:

  ent_Clean(e1);

  return ent_Assign_Rlab_BTREE(rval);
}

#undef  THIS_SOLVER
#define THIS_SOLVER "gts_surface_writem"
Ent * ent_gts_surface_writem (int nargs, Datum args[])
{
  Ent *e1=0, *e2=0, *e3=0;
  FILE *rlab_stderr = (!RLAB_STDERR_DS) ? stderr : RLAB_STDERR_DS;
  char *name1=0, *filenm=0, *op=0;
  double rval=-1;
  int saveas=0, close_after_rw=0;
  Rfile *rf=0;
  FILE *fp=0;

  r_gts_surface *rf1=0;

  if(!r_gts_surface_list)
  {
    init_r_gts_surface_list();
  }

  //
  if (nargs < 2)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_TWO_ARG_REQUIRED "\n");
    goto _exit;
  }

  e1 = bltin_get_ent(args[0]);
  if (ent_type(e1) != MATRIX_DENSE_STRING)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }
  name1 = class_char_pointer(e1);
  if (isvalidstring(name1) < 1)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }

  e2 = bltin_get_ent(args[1]);
  if (ent_type(e2) != MATRIX_DENSE_STRING)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG2_STRING_OBJNAME "\n");
    goto _exit;
  }
  filenm = class_char_pointer(e2);
  if (isvalidstring(filenm) < 1)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG2_STRING_OBJNAME "\n");
    goto _exit;
  }

  //
  // does 'name1' already exist in list of named surfaces
  //
  rf1 = r_gts_surface_list_find (name1);
  if (!rf1)
  {
    goto _exit;
  }

  if (nargs > 2)
  {
    e3 = bltin_get_ent(args[2]);
    if (ent_type(e3) != MATRIX_DENSE_STRING)
    {
      fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG2_STRING_OBJNAME "\n");
      goto _exit;
    }
    op = class_char_pointer(e3);
    if (isvalidstring(op) > 1)
    {
      if (strstr(op, "o"))
      {
        saveas = 1;
      }
      else if (strstr(op, "v"))
      {
        saveas = 2;
      }
    }
  }
  else
  {
    /* check the filename extension: */
    if (strstr(filenm, ".oogl") || strstr(filenm, "geomview"))
    {
      saveas = 1;
    }
    else if (strstr(filenm, ".vtk"))
    {
      saveas = 2;
    }
  }

  rval = 1;
  if (!strcmp ("stdout", filenm))
  {
    fp = stdout;
  }
  else if (!strcmp ("stderr", filenm))
  {
    fp = stderr;
  }
  else
  {
    rf = rfile_find(filenm);
    if (!rf)
    {
      close_after_rw = 1;
      rf = get_rfile_ds (filenm, RFILE_FILE, "w", 0);
    }
    fp = rf->fileds_f;
  }

  if (fp)
  {
    switch (saveas)
    {
      case 1: /* oogl */
        gts_surface_write_oogl (rf1->s, fp);
        break;

      case 2: /* wtk */
        gts_surface_write_vtk (rf1->s, fp);
        break;
        ;

      default:
        gts_surface_write (rf1->s, fp);

    } /* switch (saveas) */

    if (close_after_rw)
    {
      rfile_Destroy(filenm);
    }

    rval = 0;

  } /*if (!fptr)*/

_exit:

  ent_Clean(e1);
  ent_Clean(e2);
  ent_Clean(e3);

  return ent_Create_Rlab_Double(rval);
}

//
// create new surface
//
#undef  THIS_SOLVER
#define THIS_SOLVER "gts_surface_readm"
Ent * ent_gts_surface_readm (int nargs, Datum args[])
{
  Ent *e1=0, *e2=0;
  FILE *rlab_stderr = (!RLAB_STDERR_DS) ? stderr : RLAB_STDERR_DS;
  char *name1=0, *filenm=0, *op=0;
  double rval=-1;
  int saveas=0, close_after_rw=0;
  Rfile *rf=0;
  FILE *fp=0;
  GtsColor c = GTS_DEFAULT_COLOR;

  r_gts_surface *rf1=0;

  if(!r_gts_surface_list)
  {
    init_r_gts_surface_list();
  }

  ListNode *node=0;
  MDR *color=0;

  //
  if (nargs < 2)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_TWO_ARG_REQUIRED "\n");
    goto _exit;
  }

  e1 = bltin_get_ent(args[0]);
  if (ent_type(e1) != MATRIX_DENSE_STRING)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }
  name1 = class_char_pointer(e1);
  if (isvalidstring(name1) < 1)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }

  e2 = bltin_get_ent(args[1]);
  if (ent_type(e2) != MATRIX_DENSE_STRING)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG2_STRING_OBJNAME "\n");
    goto _exit;
  }
  filenm = class_char_pointer(e2);
  if (isvalidstring(filenm) < 1)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG2_STRING_OBJNAME "\n");
    goto _exit;
  }

  //
  //
  //
  rval = 1;
  if (!strcmp ("stdin", filenm))
  {
    fp = stdin;
  }
  else
  {
    rf = rfile_find(filenm);
    if (!rf)
    {
      close_after_rw = 1;
      rf = get_rfile_ds (filenm, RFILE_FILE, "r", 0);
    }
    if (!rf)
    {
      rfile_Destroy(filenm);
      fprintf (rlab_stderr, THIS_SOLVER ": File does not exist: Can't continue!\n");
      goto _exit;
    }
    fp = rf->fileds_f;
  }

  if (!fp)
  {
    rfile_Destroy(filenm);
    fprintf (rlab_stderr, THIS_SOLVER ": File does not exist: Can't continue!\n");
    goto _exit;
  }

  GtsSurface *s = gts_surface_new (gts_surface_class(),
                                   GTS_FACE_CLASS(color_face_class()), gts_edge_class(), gts_vertex_class());
  GtsFile * sfp = gts_file_new (fp);
  if (gts_surface_read (s, sfp))
  {
    fprintf(stderr, THIS_SOLVER ": File '%s' is not a valid GTS file\n", filenm);
    fprintf(stderr, THIS_SOLVER ":%d:%d: %s\n", sfp->line, sfp->pos, sfp->error);
    gts_object_destroy (GTS_OBJECT (s));
    s = NULL;
  }
  if (!gts_surface_is_closed (s) || !gts_surface_is_orientable (s))
  {
    fprintf(stderr, THIS_SOLVER ": Surface from file '%s' is not closed or not orientable: Read failed!\n", filenm);
    gts_object_destroy (GTS_OBJECT (s));
    s = NULL;
  }

  if (close_after_rw)
  {
    rfile_Destroy(filenm);
  }

  if (s)
  {
    //
    // does 'name' already exist in list of named surfaces
    //
    rf1 = r_gts_surface_list_find (name1);
    if (rf1)
    {
      r_gts_surface_Destroy (name1);
      rf1 = 0;
    }
    if (!rf1)
    {
      rf1 = r_gts_surface_Create ();
      rf1->name = cpstr(name1);
      rf1->s = s;
    }
  }
  gts_file_destroy (sfp);

  rval = 0;

_exit:

  ent_Clean(e1);
  ent_Clean(e2);
  return ent_Create_Rlab_Double(rval);
}

static int gts_surface_func (gpointer item, gpointer data)
{
  r_gts_func_args * vfn = (r_gts_func_args *) data;
  MDR *vx1 = ent_data(vfn->arg1);

  Ent *rent=0;

  GtsVertex *v=0, *v1=0, *v2=0, *v3=0;
  GtsEdge *ed1=0;
  GtsFace *f=0;

  //
  // item -> vertex: pass it to arg1 entity
  //
  if (vfn->n_pts == 1)
  {
    // call
    //    rlab_func1(vx_i/, params/)
    // for each vertex vx_i of the surface
    v = (GtsVertex *) item;
    MdrV0(vx1,0) = v->p.x;
    MdrV0(vx1,1) = v->p.y;
    MdrV0(vx1,2) = v->p.z;
  }
  else if (vfn->n_pts == 2)
  {
    // call
    //    rlab_func2(ed_i/, params/)
    // for each edge ed_i of the surface: ed_i = [vx_1; vx_2]
    ed1 = (GtsEdge *) item;
    if ((MNR(vx1)==2) && (MNC(vx1)==3))
    {
      Mdr0(vx1,0,0) = ed1->segment.v1->p.x;
      Mdr0(vx1,0,1) = ed1->segment.v1->p.y;
      Mdr0(vx1,0,2) = ed1->segment.v1->p.z;
      Mdr0(vx1,1,0) = ed1->segment.v2->p.x;
      Mdr0(vx1,1,1) = ed1->segment.v2->p.y;
      Mdr0(vx1,1,2) = ed1->segment.v2->p.z;
    }
    else if ((MNR(vx1)==1) && (MNC(vx1)==6))
    {
      Mdr0(vx1,0,0) = ed1->segment.v1->p.x;
      Mdr0(vx1,0,1) = ed1->segment.v1->p.y;
      Mdr0(vx1,0,2) = ed1->segment.v1->p.z;
      Mdr0(vx1,0,3) = ed1->segment.v2->p.x;
      Mdr0(vx1,0,4) = ed1->segment.v2->p.y;
      Mdr0(vx1,0,5) = ed1->segment.v2->p.z;
    }
  }
  else if (vfn->n_pts == 3)
  {
    // call
    //    rlab_func3( ed_{i,1}, ed_{i,2}, ed_{i,3} /, params/)
    // for each triangle of the surface, where the triangle is given as three edges
    f = (GtsFace *) item;
    gts_triangle_vertices(GTS_TRIANGLE(f), &v1, &v2, &v3 );
    //
    if ((MNR(vx1)==2) && (MNC(vx1)==3))
    {
      Mdr0(vx1,0,0) = (*v1).p.x;
      Mdr0(vx1,0,1) = (*v1).p.y;
      Mdr0(vx1,0,2) = (*v1).p.z;
      //
      Mdr0(vx1,1,0) = (*v2).p.x;
      Mdr0(vx1,1,1) = (*v2).p.y;
      Mdr0(vx1,1,2) = (*v2).p.z;
      //
      Mdr0(vx1,2,0) = (*v3).p.x;
      Mdr0(vx1,2,1) = (*v3).p.y;
      Mdr0(vx1,2,2) = (*v3).p.z;
    }
    else if ((MNR(vx1)==1) && (MNC(vx1)==9))
    {
      Mdr0(vx1,0,0) = (*v1).p.x;
      Mdr0(vx1,0,1) = (*v1).p.y;
      Mdr0(vx1,0,2) = (*v1).p.z;
      //
      Mdr0(vx1,0,3) = (*v2).p.x;
      Mdr0(vx1,0,4) = (*v2).p.y;
      Mdr0(vx1,0,5) = (*v2).p.z;
      //
      Mdr0(vx1,0,6) = (*v3).p.x;
      Mdr0(vx1,0,7) = (*v3).p.y;
      Mdr0(vx1,0,8) = (*v3).p.z;
    }
  }

  switch(vfn->n_args)
  {
    case 1: /* {vertex,edge,face},noparams */
      rent = ent_call_rlab_script_1arg (vfn->fname, vfn->arg1);
      break;

    case 2: /* {vertex,edge,face},params */
      rent = ent_call_rlab_script_2args(vfn->fname, vfn->arg1, vfn->arg2);
      break;
  }

  int rval = 1;
  if (!vfn->modify_obj)
  {
    // we are not modifying the points, so we pass the result of function call
    // to a variable that will be available to user after iteration completes
    if (vfn->rval)
    {
      //
      // we swap 'rent' and 'vfn->rval'
      //
      Ent *dummy = rent;
      rent = vfn->rval;
      vfn->rval = dummy;
      dummy = NULL;
    }
    else
    {
      if (ent_type(rent) == MATRIX_DENSE_REAL)
      {
        rval = class_int(rent);
      }
    }

    ent_Clean(rent);
    return rval;
  }

  // we are modifying points, but the function has to return
  // new value of the point, edge or face
  if (ent_type(rent) != MATRIX_DENSE_REAL)
  {
    ent_Clean(rent);
    return rval;
  }

  vx1 = ent_data(rent);
  if (vfn->n_pts == 1)
  {
    if (v)
    {
      if (SIZE(vx1) == 3)
      {
        rval = 0;
        v->p.x = MdrV0(vx1,0);
        v->p.y = MdrV0(vx1,1);
        v->p.z = MdrV0(vx1,2);
      }
    }
  }
  else if (vfn->n_pts == 2)
  {
    if (ed1)
    {
      if ((MNR(vx1)==2) && (MNC(vx1)==3))
      {
        ed1->segment.v1->p.x = Mdr0(vx1,0,0);
        ed1->segment.v1->p.y = Mdr0(vx1,0,1);
        ed1->segment.v1->p.z = Mdr0(vx1,0,2);
        ed1->segment.v2->p.x = Mdr0(vx1,1,0);
        ed1->segment.v2->p.y = Mdr0(vx1,1,1);
        ed1->segment.v2->p.z = Mdr0(vx1,1,2);
      }
      else if ((MNR(vx1)==1) && (MNC(vx1)==6))
      {
        ed1->segment.v1->p.x = Mdr0(vx1,0,0);
        ed1->segment.v1->p.y = Mdr0(vx1,0,1);
        ed1->segment.v1->p.z = Mdr0(vx1,0,2);
        ed1->segment.v2->p.x = Mdr0(vx1,0,3);
        ed1->segment.v2->p.y = Mdr0(vx1,0,4);
        ed1->segment.v2->p.z = Mdr0(vx1,0,5);
      }
    }
  }
  else if (vfn->n_pts == 3)
  {
    if (f)
    {
      if ((MNR(vx1)==3) && (MNC(vx1)==3))
      {
        (*v1).p.x = Mdr0(vx1,0,0);
        (*v1).p.y = Mdr0(vx1,0,1);
        (*v1).p.z = Mdr0(vx1,0,2);
        (*v2).p.x = Mdr0(vx1,1,0);
        (*v2).p.y = Mdr0(vx1,1,1);
        (*v2).p.z = Mdr0(vx1,1,2);
        (*v3).p.x = Mdr0(vx1,2,0);
        (*v3).p.y = Mdr0(vx1,2,1);
        (*v3).p.z = Mdr0(vx1,2,2);
      }
      else if ((MNR(vx1)==1) && (MNC(vx1)==9))
      {
        (*v1).p.x = Mdr0(vx1,0,0);
        (*v1).p.y = Mdr0(vx1,0,1);
        (*v1).p.z = Mdr0(vx1,0,2);
        (*v2).p.x = Mdr0(vx1,0,3);
        (*v2).p.y = Mdr0(vx1,0,4);
        (*v2).p.z = Mdr0(vx1,0,5);
        (*v3).p.x = Mdr0(vx1,0,6);
        (*v3).p.y = Mdr0(vx1,0,7);
        (*v3).p.z = Mdr0(vx1,0,8);
      }
    }
  }

  ent_Clean (rent);

  return rval;
}

#undef  THIS_SOLVER
#define THIS_SOLVER "gts_surface_foreach_vertex"
Ent * ent_gts_surface_foreach_vertex (int nargs, Datum args[])
{
  Ent *e1=0, *e2=0, *e3=0, *e4=0, *rent=0;
  FILE *rlab_stderr = (!RLAB_STDERR_DS) ? stderr : RLAB_STDERR_DS;
  char *name1=0;
  int rval=-1;

  r_gts_func_args vfn;
  vfn.n_args = 0;
  vfn.n_pts  = 1;
  vfn.rval   = 0;
  vfn.modify_obj = 0;
  vfn.stack_pts = 0;
  r_gts_surface *rf1=0;

  if(!r_gts_surface_list)
  {
    init_r_gts_surface_list();
  }

  //
  if (nargs < 2)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ONE_ARG_REQUIRED "\n");
    goto _exit;
  }

  e1 = bltin_get_ent(args[0]);
  if (ent_type(e1) != MATRIX_DENSE_STRING)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }
  name1 = class_char_pointer(e1);
  if (isvalidstring(name1) < 1)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }

  // get the function entity to be called
  e2 = bltin_get_ent(args[1]);
  if (!isfuncent(e2))
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }
  vfn.fname = e2;

  // get the third argument:
  //  options for the function to be called for each face/triangle
  //    func(triangle, func_opts)
  //
  if (nargs > 2)
  {
    e3 = bltin_get_ent(args[2]);
    if(ent_type(e3)!=UNDEF)
    {
      vfn.arg2 = e3;
      vfn.n_args += 1;
      ent_IncRef (vfn.arg2);
    }
  }

  //
  // get last argument - options
  //
  if (nargs > 3)
  {
    e4 = bltin_get_ent(args[3]);
    if (ent_type(e4) == BTREE)
    {
      ListNode *node;
      RLABCODE_PROCESS_BTREE_ENTRY_BOOL(e4,node,RLAB_GTS_MODIFY_OBJ,vfn.modify_obj);
      RLABCODE_PROCESS_BTREE_ENTRY_BOOL(e4,node,RLAB_GTS_STACK_PTS,vfn.stack_pts);
      //nargs--;
    }
  }

  //
  // does 'name1' already exist in list of named surfaces
  //
  rf1 = r_gts_surface_list_find (name1);
  if (!rf1)
  {
    goto _exit;
  }

  rval = 0;

  // this will store information for vertex in the function call
  vfn.arg1 = ent_Assign_Rlab_MDR(mdr_Create(vfn.n_pts,3));
  ent_IncRef (vfn.arg1);
  vfn.n_args += 1;
  vfn.rval   = ent_Assign_Rlab_MDR(mdr_CreateScalar(0));

  gts_surface_foreach_vertex (rf1->s, gts_surface_func, (gpointer) &vfn);

  ent_DecRef (vfn.arg1);
  ent_Destroy(vfn.arg1); /**/
  if (vfn.n_args > 1)
  {
    ent_DecRef (vfn.arg2);
  }

_exit:

  ent_Clean(e1);
  ent_Clean(e2);
  ent_Clean(e3);
  ent_Clean(e4);

  //
  // return the result of last function call
  //
  if (vfn.rval)
  {
    rent = vfn.rval;
    vfn.rval = 0;
  }
  else
  {
    rent = ent_Create_Rlab_Double(rval);
  }

  return rent;
}

#undef  THIS_SOLVER
#define THIS_SOLVER "gts_surface_foreach_edge"
Ent * ent_gts_surface_foreach_edge (int nargs, Datum args[])
{
  Ent *e1=0, *e2=0, *e3=0, *e4=0, *rent=0;
  FILE *rlab_stderr = (!RLAB_STDERR_DS) ? stderr : RLAB_STDERR_DS;
  char *name1=0;
  int rval=-1;

  r_gts_func_args vfn;
  vfn.n_args = 0;
  vfn.n_pts  = 2;
  vfn.rval   = 0;
  vfn.modify_obj = 0;
  vfn.stack_pts = 0;

  r_gts_surface *rf1=0;

  if(!r_gts_surface_list)
  {
    init_r_gts_surface_list();
  }

  //
  if (nargs < 2)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ONE_ARG_REQUIRED "\n");
    goto _exit;
  }

  e1 = bltin_get_ent(args[0]);
  if (ent_type(e1) != MATRIX_DENSE_STRING)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }
  name1 = class_char_pointer(e1);
  if (isvalidstring(name1) < 1)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }

  // get the function entity to be called
  e2 = bltin_get_ent(args[1]);
  if (!isfuncent(e2))
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }
  vfn.fname = e2;

  // get the third argument:
  //  options for the function to be called for each face/triangle
  //    func(triangle, func_opts)
  //
  if (nargs > 2)
  {
    e3 = bltin_get_ent(args[2]);
    if(ent_type(e3)!=UNDEF)
    {
      vfn.arg2 = e3;
      vfn.n_args += 1;
      ent_IncRef (vfn.arg2);
    }
  }

  //
  // get last argument - options
  //
  if (nargs > 3)
  {
    e4 = bltin_get_ent(args[3]);
    if (ent_type(e4) == BTREE)
    {
      ListNode *node;
      RLABCODE_PROCESS_BTREE_ENTRY_BOOL(e4,node,RLAB_GTS_MODIFY_OBJ,vfn.modify_obj);
      RLABCODE_PROCESS_BTREE_ENTRY_BOOL(e4,node,RLAB_GTS_STACK_PTS,vfn.stack_pts);
      //nargs--;
    }
  }

  //
  // does 'name1' already exist in list of named surfaces
  //
  rf1 = r_gts_surface_list_find (name1);
  if (!rf1)
  {
    goto _exit;
  }

  // this will store information for vertex in the function call
  if (vfn.stack_pts)
  {
    vfn.arg1 = ent_Assign_Rlab_MDR(mdr_Create(vfn.n_pts,3));
  }
  else
  {
    vfn.arg1 = ent_Assign_Rlab_MDR(mdr_Create(1, vfn.n_pts*3));
  }
  ent_IncRef (vfn.arg1);
  vfn.n_args += 1;
  vfn.rval   = ent_Assign_Rlab_MDR(mdr_CreateScalar(0));

  gts_surface_foreach_edge (rf1->s, gts_surface_func, (gpointer) &vfn);

  ent_DecRef (vfn.arg1);
  ent_Destroy(vfn.arg1); /**/
  if (vfn.n_args > 1)
  {
    ent_DecRef (vfn.arg2);
  }

_exit:

  ent_Clean(e1);
  ent_Clean(e2);
  ent_Clean(e3);
  ent_Clean(e4);

  //
  // return the result of last function call
  //
  if (vfn.rval)
  {
    rent = vfn.rval;
    vfn.rval = 0;
  }
  else
  {
    rent = ent_Create_Rlab_Double(rval);
  }

  return rent;
}

#undef  THIS_SOLVER
#define THIS_SOLVER "gts_surface_foreach_face"
Ent * ent_gts_surface_foreach_face (int nargs, Datum args[])
{
  Ent *e1=0, *e2=0, *e3=0, *e4=0, *rent=0;
  FILE *rlab_stderr = (!RLAB_STDERR_DS) ? stderr : RLAB_STDERR_DS;
  char *name1=0;
  int rval=-1;

  r_gts_func_args vfn;
  vfn.n_args = 0;
  vfn.n_pts  = 3;
  vfn.rval   = 0;
  vfn.modify_obj = 0;
  vfn.stack_pts = 0;

  r_gts_surface *rf1=0;

  if(!r_gts_surface_list)
  {
    init_r_gts_surface_list();
  }

  //
  if (nargs < 2)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ONE_ARG_REQUIRED "\n");
    goto _exit;
  }

  e1 = bltin_get_ent(args[0]);
  if (ent_type(e1) != MATRIX_DENSE_STRING)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }
  name1 = class_char_pointer(e1);
  if (isvalidstring(name1) < 1)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }

  // get the function entity to be called
  e2 = bltin_get_ent(args[1]);
  if (!isfuncent(e2))
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }
  vfn.fname = e2;

  // get the third argument:
  //  options for the function to be called for each face/triangle
  //    func(triangle, func_opts)
  //
  if (nargs > 2)
  {
    e3 = bltin_get_ent(args[2]);
    if(ent_type(e3)!=UNDEF)
    {
      vfn.arg2 = e3;
      vfn.n_args += 1;
      ent_IncRef (vfn.arg2);
    }
  }

  //
  // get fourth last argument - options for the solver
  //
  if (nargs > 3)
  {
    e4 = bltin_get_ent(args[3]);
    if (ent_type(e4) == BTREE)
    {
      ListNode *node;
      RLABCODE_PROCESS_BTREE_ENTRY_BOOL(e4,node,RLAB_GTS_MODIFY_OBJ,vfn.modify_obj);
      RLABCODE_PROCESS_BTREE_ENTRY_BOOL(e4,node,RLAB_GTS_STACK_PTS,vfn.stack_pts);
    }
  }

  //
  // does 'name1' already exist in list of named surfaces
  //
  rf1 = r_gts_surface_list_find (name1);
  if (!rf1)
  {
    goto _exit;
  }

  rval = 0;

  // this will store information for vertex in the function call
  if (vfn.stack_pts)
  {
    vfn.arg1 = ent_Assign_Rlab_MDR(mdr_Create(vfn.n_pts,3));
  }
  else
  {
    vfn.arg1 = ent_Assign_Rlab_MDR(mdr_Create(1, vfn.n_pts*3));
  }
  ent_IncRef (vfn.arg1);
  vfn.n_args += 1;
  vfn.rval   = ent_Assign_Rlab_MDR(mdr_CreateScalar(0));

  gts_surface_foreach_face (rf1->s, gts_surface_func, (gpointer) &vfn);

  ent_DecRef (vfn.arg1);
  ent_Destroy(vfn.arg1); /**/
  if (vfn.n_args == 2)
  {
    ent_DecRef (vfn.arg2);
  }

_exit:

  ent_Clean(e1);
  ent_Clean(e2);
  ent_Clean(e3);
  ent_Clean(e4);

  //
  // return the result of last function call
  //
  if (vfn.rval)
  {
    rent = vfn.rval;
    vfn.rval = 0;
  }
  else
  {
    rent = ent_Create_Rlab_Double(rval);
  }

  return rent;
}

#undef  THIS_SOLVER
#define THIS_SOLVER "gts_surface_foreach_face_remove"
Ent * ent_gts_surface_foreach_face_remove (int nargs, Datum args[])
{
  Ent *e1=0, *e2=0, *e3=0, *e4=0, *rent=0;
  FILE *rlab_stderr = (!RLAB_STDERR_DS) ? stderr : RLAB_STDERR_DS;
  char *name1=0;
  int rval=-1;

  r_gts_func_args vfn;
  vfn.n_args = 0;
  vfn.n_pts  = 3;
  vfn.rval   = 0;
  vfn.modify_obj = 0;
  vfn.stack_pts = 0;

  r_gts_surface *rf1=0;

  if(!r_gts_surface_list)
  {
    init_r_gts_surface_list();
  }

  //
  if (nargs < 2)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ONE_ARG_REQUIRED "\n");
    goto _exit;
  }

  e1 = bltin_get_ent(args[0]);
  if (ent_type(e1) != MATRIX_DENSE_STRING)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }
  name1 = class_char_pointer(e1);
  if (isvalidstring(name1) < 1)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }

  // get the function entity to be called
  e2 = bltin_get_ent(args[1]);
  if (!isfuncent(e2))
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }
  vfn.fname = e2;

  // get the third argument:
  //  options for the function to be called for each face/triangle
  //    func(triangle, func_opts)
  //
  if (nargs > 2)
  {
    e3 = bltin_get_ent(args[2]);
    if(ent_type(e3)!=UNDEF)
    {
      vfn.arg2 = e3;
      vfn.n_args += 1;
      ent_IncRef (vfn.arg2);
    }
  }

  //
  // get fourth last argument - options for the solver
  //
  if (nargs > 3)
  {
    e4 = bltin_get_ent(args[3]);
    if (ent_type(e4) == BTREE)
    {
      ListNode *node;
      RLABCODE_PROCESS_BTREE_ENTRY_BOOL(e4,node,RLAB_GTS_STACK_PTS,vfn.stack_pts);
    }
  }

  //
  // does 'name1' already exist in list of named surfaces
  //
  rf1 = r_gts_surface_list_find (name1);
  if (!rf1)
  {
    goto _exit;
  }

  rval = 0;

  // this will store information for vertex in the function call
  if (vfn.stack_pts)
  {
    // f = [vx1; vx2; vx3]
    vfn.arg1 = ent_Assign_Rlab_MDR(mdr_Create(vfn.n_pts,3));
  }
  else
  {
    // f = [vx1, vx2, vx3]
    vfn.arg1 = ent_Assign_Rlab_MDR(mdr_Create(1, vfn.n_pts*3));
  }
  ent_IncRef (vfn.arg1);
  vfn.n_args += 1;

  rval = gts_surface_foreach_face_remove (rf1->s, gts_surface_func, (gpointer) &vfn);

  ent_DecRef (vfn.arg1);
  ent_Destroy(vfn.arg1); /**/
  if (vfn.n_args == 2)
  {
    ent_DecRef (vfn.arg2);
  }

_exit:

  ent_Clean(e1);
  ent_Clean(e2);
  ent_Clean(e3);
  ent_Clean(e4);

  //
  // return
  //
  return ent_Create_Rlab_Double(rval);
}

#undef  THIS_SOLVER
#define THIS_SOLVER "gts_surface_tessellate"
Ent * ent_gts_surface_tessellate(int nargs, Datum args[])
{
  Ent *e1=0;
  FILE *rlab_stderr = (!RLAB_STDERR_DS) ? stderr : RLAB_STDERR_DS;
  char *name1=0;
  int rval=-1;

  r_gts_func_args vfn;
  vfn.n_args = 0;
  vfn.n_pts  = 3;
  vfn.rval   = 0;
  vfn.fname  = 0;

  r_gts_surface *rf1=0;

  if(!r_gts_surface_list)
  {
    init_r_gts_surface_list();
  }

  //
  if (nargs < 1)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ONE_ARG_REQUIRED "\n");
    goto _exit;
  }

  e1 = bltin_get_ent(args[0]);
  if (ent_type(e1) != MATRIX_DENSE_STRING)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }
  name1 = class_char_pointer(e1);
  if (isvalidstring(name1) < 1)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }


  //
  // does 'name1' already exist in list of named surfaces
  //
  rf1 = r_gts_surface_list_find (name1);
  if (!rf1)
  {
    goto _exit;
  }

  rval = 0;

  // this will store information for vertex in the function call

  gts_surface_tessellate (rf1->s, NULL, NULL);

_exit:

  ent_Clean(e1);

  //
  // return
  //
  return ent_Create_Rlab_Double(rval);
}


static int update_face_color(GtsFace *f, void *data)
{
  GtsColor *c = (GtsColor *) data;
  COLOR_FACE(f)->c.r = c->r;
  COLOR_FACE(f)->c.g = c->g;
  COLOR_FACE(f)->c.b = c->b;
}

static void print_edge (GtsSegment * s, gpointer * data)
{
  fprintf(stderr, "%g,%g,%g   %g,%g,%g\n",
          s->v1->p.x, s->v1->p.y, s->v1->p.z,
          s->v2->p.x, s->v2->p.y, s->v2->p.z);
}


#undef  THIS_SOLVER
#define THIS_SOLVER "gts_surface_boolean"
Ent * ent_gts_surface_boolean(int nargs, Datum args[])
{
  Ent *e1=0, *e2=0, *e3=0, *e4=0, *e5=0;
  FILE *rlab_stderr = (!RLAB_STDERR_DS) ? stderr : RLAB_STDERR_DS;
  char *name1=0, *name2=0, *name3=0, *op=0;
  int rval=-2;

  r_gts_surface *rf1=0, *rf2=0, *rf3=0;
  MDR *color=0;

  if(!r_gts_surface_list)
  {
    init_r_gts_surface_list();
  }

  //
  if (nargs < 4)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_FOUR_ARG_REQUIRED "\n");
    goto _exit;
  }

  //
  // 'name1' is the label of the first surface
  //
  e1 = bltin_get_ent(args[0]);
  if (ent_type(e1) != MATRIX_DENSE_STRING)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }
  name1 = class_char_pointer(e1);
  if (isvalidstring(name1) < 1)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }
  rf1 = r_gts_surface_list_find (name1);
  if (!rf1)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": ARG1: Surface %s: No such surface\n", name1);
    goto _exit;
  }

  //
  // 'name2' is the label of the second surface
  //
  e2 = bltin_get_ent(args[1]);
  if (ent_type(e2) != MATRIX_DENSE_STRING)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG2_STRING_OBJNAME "\n");
    goto _exit;
  }
  name2 = class_char_pointer(e2);
  if (isvalidstring(name2) < 1)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG2_STRING_OBJNAME "\n");
    goto _exit;
  }
  rf2 = r_gts_surface_list_find (name2);
  if (!rf2)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": ARG2: Surface %s: No such surface\n", name2);
    goto _exit;
  }

  //
  // 'name3' is the label of the result surface
  //
  e3 = bltin_get_ent(args[2]);
  if (ent_type(e3) != MATRIX_DENSE_STRING)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG3_STRING_OBJNAME "\n");
    goto _exit;
  }
  name3 = class_char_pointer(e3);
  if (isvalidstring(name3) < 1)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG3_STRING_OBJNAME "\n");
    goto _exit;
  }

  e4 = bltin_get_ent(args[3]);
  if (ent_type(e4) != MATRIX_DENSE_STRING)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG4_STRING_OBJNAME "\n");
    goto _exit;
  }
  op = class_char_pointer(e4);

  if (nargs > 4)  {
    e5 = bltin_get_ent(args[4]);
    if (ent_type(e5) == BTREE)
    {
      ListNode *node;
      // set color of the newly added faces
      RLABCODE_PROCESS_BTREE_ENTRY_MD(e5,node,RLAB_GTS_NEW_SURFACE_COLOR,color,MDR,SIZE,3,NULL);
    }
  }

  //
  // Check for self-intersections in either surface
  //
  if(gts_surface_is_self_intersecting(rf1->s)!= NULL )
  {
    fprintf(rlab_stderr, THIS_SOLVER ": Surface '%s' is self-intersecting. Can't continue!\n",
            name1);
    goto _exit;
  }
  if(gts_surface_is_self_intersecting(rf2->s)!= NULL )
  {
    fprintf(rlab_stderr, THIS_SOLVER ": Surface '%s' is self-intersecting. Can't continue!\n",
            name2);
    goto _exit;
  }

  //
  // Avoid complete self-intersection of two surfaces
  //
  if (      (gts_surface_face_number(rf1->s) == gts_surface_face_number(rf2->s))
         && (gts_surface_edge_number(rf1->s) == gts_surface_edge_number(rf2->s))
         && (gts_surface_vertex_number(rf1->s) == gts_surface_vertex_number(rf2->s))
         && (gts_surface_area(rf1->s) == gts_surface_area(rf2->s)) )
  {
    GtsVector cm1, cm2;
    double area1 = gts_surface_center_of_area(rf1->s,cm1);
    double area2 = gts_surface_center_of_area(rf2->s,cm2);
    if ( (area1==area2) && (cm1[0]==cm2[0]) && (cm1[1]==cm2[1]) && (cm1[2]==cm2[2]) )
    {
      fprintf(rlab_stderr, THIS_SOLVER
          ": Surface '%s' and '%s' are identical/self-intersecting. Can't continue!\n",
          name1, name2);
      goto _exit;
    }
  }

  rval = -1;

  //
  // Get bounding boxes
  //
  gboolean closed;
  GtsSurface *surface=0;
  GNode *tree1 = gts_bb_tree_surface(rf1->s);
  gboolean is_open1 = !gts_surface_is_closed(rf1->s);
  GNode *tree2 = gts_bb_tree_surface(rf2->s);
  gboolean is_open2 = !gts_surface_is_closed(rf2->s);


//   fprintf(stderr, "isopen1,2=%i,%i\n", is_open1, is_open2);

  /* Get the surface intersection object */
  GtsSurfaceInter *si = gts_surface_inter_new(gts_surface_inter_class(), rf1->s, rf2->s,
                                              tree1, tree2, is_open1, is_open2);
//   fprintf(stderr, "si->edges=%p\n", si->edges);
//   g_slist_foreach (si->edges, (GFunc) print_edge, NULL);


  //
  // Check that the surface intersection object is closed
  //
  gts_surface_inter_check(si,&closed);
  if( closed == FALSE )
  {
    fprintf(rlab_stderr, THIS_SOLVER ": Intersection of Surfaces '%s' and '%s' is not closed\n",
            name1, name2);
    goto _gts_cleanup;
  }

  surface = gts_surface_new(gts_surface_class(),
                            GTS_FACE_CLASS(color_face_class()), gts_edge_class(),
                                           gts_vertex_class());

  //
  // Calculate the new surface based on 'op'
  //
  if (!strcmp(op,RLAB_GTS_BOOLEAN_1_OUT_2))
  {
    gts_surface_inter_boolean(si, surface, GTS_1_OUT_2);
  }
  else if (!strcmp(op,RLAB_GTS_BOOLEAN_2_OUT_1))
  {
    gts_surface_inter_boolean(si, surface, GTS_2_OUT_1);
  }
  else if (!strcmp(op,RLAB_GTS_BOOLEAN_1_IN_2))
  {
    gts_surface_inter_boolean(si, surface, GTS_1_IN_2);
  }
  else if (!strcmp(op,RLAB_GTS_BOOLEAN_2_IN_1))
  {
    gts_surface_inter_boolean(si, surface, GTS_2_IN_1);
  }
  else if (!strcmp(op,RLAB_GTS_BOOLEAN_UNION))
  {
    gts_surface_inter_boolean(si, surface, GTS_1_OUT_2);
    gts_surface_inter_boolean(si, surface, GTS_2_OUT_1);
  }
  else if (!strcmp(op,RLAB_GTS_BOOLEAN_INTERSECTION))
  {
    gts_surface_inter_boolean(si, surface, GTS_1_IN_2);
    gts_surface_inter_boolean(si, surface, GTS_2_IN_1);
  }
  else if (!strcmp(op,RLAB_GTS_BOOLEAN_DIFFERENCE))
  {
    gts_surface_inter_boolean(si, surface, GTS_1_OUT_2);
    gts_surface_inter_boolean(si, surface, GTS_2_IN_1);
    gts_surface_foreach_face (si->s2, (GtsFunc) gts_triangle_revert, NULL);
    gts_surface_foreach_face (rf2->s, (GtsFunc) gts_triangle_revert, NULL);
  }
  else
  {
    fprintf(rlab_stderr, THIS_SOLVER ": Unknown boolean operator '%s'\n", op);
    goto _gts_cleanup;
  }

  /* Check for self-intersection */
  if( gts_surface_is_self_intersecting(surface) != NULL )
  {
    fprintf(rlab_stderr, THIS_SOLVER ": Result surface is self intersecting. Can't continue!\n");
    gts_object_destroy(GTS_OBJECT(surface));
    surface = 0;
  }

  rval = 1;

_gts_cleanup:

  // Cleanup si: boolean.c: gts_surface_intersection
  gts_surface_foreach_face (si->s1, (GtsFunc) free_slist, NULL);
  gts_surface_foreach_face (si->s2, (GtsFunc) free_slist, NULL);
  gts_surface_foreach_edge (si->s1, (GtsFunc) free_glist, NULL);
  gts_surface_foreach_edge (si->s2, (GtsFunc) free_glist, NULL);
  gts_object_destroy(GTS_OBJECT(si));
  gts_bb_tree_destroy(tree1, TRUE);
  gts_bb_tree_destroy(tree2, TRUE);

  if (surface)
  {
    /* set the color of all newly added faces */
    if (color)
    {
      GtsColor c;
      c.r = mdrV0(color,0);
      c.g = mdrV0(color,1);
      c.b = mdrV0(color,2);
      gts_surface_foreach_face (surface, (GtsFunc) update_face_color, &c);
    }

    /* Destroy result surface if it exists */
    r_gts_surface_Destroy (name3);
    rf3 = r_gts_surface_Create ();
    rf3->name = cpstr(name3);
    rf3->s = surface;
    surface = 0;
    rval = 0;
  }

_exit:

  ent_Clean(e1);
  ent_Clean(e2);
  ent_Clean(e3);
  ent_Clean(e4);
  ent_Clean(e5);

  //
  // return
  //
  return ent_Create_Rlab_Double(rval);
}

//
// create new surface
//
#undef  THIS_SOLVER
#define THIS_SOLVER "gts_surface_new"
Ent * ent_gts_surface_new (int nargs, Datum args[])
{
  Ent *e1=0, *e2=0, *e3=0;
  FILE *rlab_stderr = (!RLAB_STDERR_DS) ? stderr : RLAB_STDERR_DS;
  char *name=0;
  GtsColor c = GTS_DEFAULT_COLOR;

  r_gts_surface *rf=0;
  r_gts_surface_transform st;
  st.rot_x = 0;
  st.rot_y = 0;
  st.rot_z = 0;
  st.scale = 0;
  st.transl = 0;
  st.center = 0;

  ListNode *node=0;
  MDR *color=0;

  if(!r_gts_surface_list)
  {
    init_r_gts_surface_list();
  }

  //
  if (nargs < 1)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ONE_ARG_REQUIRED "\n");
    goto _exit;
  }

  //
  // get matrix of rows of points in [x,y,z] format
  //
  if (args[0].type == VAR)
  {
    name = (char *) var_key (args[0].u.ptr);
  }

  e1 = bltin_get_ent(args[0]);
  if (ent_type(e1) == BTREE)
  {
    //
    // variable is a list of vertices_i as [x_i,y_i,z_i],
    //    eg_i as [vx_j_1,vx_j_2], and faces_i as [eg_i_1, eg_i_2, eg_i_3]
    //
    MDR *vertex=0 ,*edge=0, *face=0;
    int i, a1, a2, a3, ne=0, nv=0, nf=0;
    // get all the vertices
    RLABCODE_PROCESS_BTREE_ENTRY_MD(e1,node,RLAB_GTS_VERTEX,vertex,MDR,MNC,3,NULL);
    if (!vertex)
    {
      goto _exit_once;
    }
    nv = MNR(vertex);
    if (nv < 1)
    {
      goto _exit_once;
    }
    // get all the edges
    RLABCODE_PROCESS_BTREE_ENTRY_MD(e1,node,RLAB_GTS_EDGE,edge,MDR,MNC,2,NULL);
    if (!edge)
    {
      goto _exit_once;
    }
    ne = MNR(edge);
    if (ne < 3)
    {
      goto _exit_once;
    }
    // get all the faces
    RLABCODE_PROCESS_BTREE_ENTRY_MD(e1,node,RLAB_GTS_FACE,face,MDR,MNC,3,NULL);
    if (!face)
    {
      goto _exit_once;
    }
    nf = MNR(face);
    if (nf < 1)
    {
      goto _exit_once;
    }
    // get the colors
    RLABCODE_PROCESS_BTREE_ENTRY_MD(e1,node,RLAB_GTS_FACE,color,MDR,MNC,3,NULL);

    /* collect the vertices  */
    GtsVertex **vx = GC_MALLOC(nv * sizeof (GtsVertex *));
    for (i=0; i<nv; i++)
    {
      vx[i] = (GtsVertex *) gts_vertex_new(gts_vertex_class(),
               mdr0(vertex,i,0), mdr0(vertex,i,1), mdr0(vertex,i,2));
    }
    /* collect the edges */
    GtsEdge   **eg = GC_MALLOC (ne * sizeof (GtsEdge *));
    for (i=0; i<ne; i++)
    {
      a1 = mdi0(edge,i,0) - 1;
      a2 = mdi0(edge,i,1) - 1;
      eg[i] = gts_edge_new (gts_edge_class(), vx[a1], vx[a2]);
    }

    //
    // does 'name' already exist in list of named surfaces
    //
    rf = r_gts_surface_list_find (name);
    if (rf)
    {
      r_gts_surface_Destroy (name);
      rf = 0;
    }
    if (!rf)
    {
      rf = r_gts_surface_Create ();
      rf->name = cpstr(name);
      rf->s = (GtsSurface *) gts_surface_new (gts_surface_class(),
               GTS_FACE_CLASS(color_face_class()), gts_edge_class(), gts_vertex_class());
    }

    /* collect all the faces */
    for (i=0; i<nf; i++)
    {
      a1 = mdi0(face,i,0) - 1;
      a2 = mdr0(face,i,1) - 1;
      a3 = mdr0(face,i,2) - 1;
      GtsFace * f1 = gts_face_new (GTS_FACE_CLASS(color_face_class()), eg[a1], eg[a2], eg[a3]);
      if (color)
      {
        a1 = MIN(i, MNR(color)-1 );
        COLOR_FACE(f1)->c.r = mdr0(color, a1, 0);
        COLOR_FACE(f1)->c.g = mdr0(color, a1, 1);
        COLOR_FACE(f1)->c.b = mdr0(color, a1, 2);
      }
      else
      {
        COLOR_FACE(f1)->c.r = c.r;
        COLOR_FACE(f1)->c.g = c.g;
        COLOR_FACE(f1)->c.b = c.b;
      }
      gts_surface_add_face (rf->s, f1);
    }

_exit_once:

    if (vx)
      GC_FREE (vx);
    if (eg)
      GC_FREE (eg);

    ent_Clean(e1);
    return ent_Create_Rlab_Success ();
  }

  if (ent_type(e1) != MATRIX_DENSE_STRING)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }
  name = class_char_pointer(e1);
  if (isvalidstring(name) < 1)
  {
    fprintf (rlab_stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_STRING_OBJNAME "\n");
    goto _exit;
  }

  //
  // does 'name' already exist in list of named surfaces
  //
  rf = r_gts_surface_list_find (name);
  if (rf)
  {
    r_gts_surface_Destroy (name);
    rf = 0;
  }
  if (!rf)
  {
    rf = r_gts_surface_Create ();
    rf->name = cpstr(name);
    rf->s = (GtsSurface *) gts_surface_new (gts_surface_class(),
             GTS_FACE_CLASS(color_face_class()), gts_edge_class(), gts_vertex_class());
  }

  if (nargs > 1)
  {
    //
    // can be followed by the surface description and/or parametrization
    //
    e2 = bltin_get_ent(args[1]);
    if (ent_type(e2) == BTREE)
    {
      char *desc=0;
      RLABCODE_PROCESS_BTREE_ENTRY_S(e2,node,RLAB_GTS_NEW_SURFACE_DESCRIPTION,desc,1,0);
      if (desc)
      {
        MDR *transl=0, *center=0, *scale=0, *rot_x=0, *rot_y=0, *rot_z=0;
        int iterate=0;

        RLABCODE_PROCESS_BTREE_ENTRY_MD(e2,node,RLAB_GTS_NEW_SURFACE_TRANSL,transl,MDR,SIZE,3,NULL);
        if (transl)
        {
          st.transl = transl;
          iterate = 1;
        }
        RLABCODE_PROCESS_BTREE_ENTRY_MD(e2,node,RLAB_GTS_NEW_SURFACE_CENTER,center,MDR,SIZE,3,NULL);
        if (center)
        {
          st.center = center;
        }
        RLABCODE_PROCESS_BTREE_ENTRY_MD(e2,node,RLAB_GTS_NEW_SURFACE_SCALE,scale,MDR,SIZE,1,NULL);
        if (scale)
        {
          st.scale = scale;
          iterate = 1;
        }
        RLABCODE_PROCESS_BTREE_ENTRY_MD(e2,node,RLAB_GTS_NEW_SURFACE_ROT_X,rot_x,MDR,SIZE,9,NULL);
        if (rot_x)
        {
          st.rot_x = rot_x;
          iterate = 1;
        }
        RLABCODE_PROCESS_BTREE_ENTRY_MD(e2,node,RLAB_GTS_NEW_SURFACE_ROT_Y,rot_y,MDR,SIZE,9,NULL);
        if (rot_y)
        {
          st.rot_y = rot_y;
          iterate = 1;
        }
        RLABCODE_PROCESS_BTREE_ENTRY_MD(e2,node,RLAB_GTS_NEW_SURFACE_ROT_Z,rot_z,MDR,SIZE,9,NULL);
        if (rot_z)
        {
          st.rot_z = rot_z;
          iterate = 1;
        }

        // we allow a single color for surface definition
        RLABCODE_PROCESS_BTREE_ENTRY_MD(e2,node,RLAB_GTS_NEW_SURFACE_COLOR,color,MDR,MNC,3,NULL);
        if (color)
        {
          c.r = mdrV0(color,0);
          c.g = mdrV0(color,1);
          c.b = mdrV0(color,2);
        }

        if (strstr(desc, "sph")!=0)
        {
          int level = 4;
          RLABCODE_PROCESS_BTREE_ENTRY_F_S(e2,node,RLAB_GTS_NEW_SURFACE_SPHERE_LEVEL,
                                           level,class_int,1,1);
          r_gts_surface_generate_sphere(rf->s, level);
        }
        else if ((strstr(desc, "pla")!=0)||(strstr(desc, "rect")!=0))
        {
          MDR * bbox_x=0, *bbox_y=0, *bbox_z=0;
          double offs=0.0;
          RLABCODE_PROCESS_BTREE_ENTRY_MD(e2,node,RLAB_GTS_NEW_SURFACE_RECT_BBOX_X,bbox_x,
                                          MDR,SIZE,1,NULL);
          RLABCODE_PROCESS_BTREE_ENTRY_MD(e2,node,RLAB_GTS_NEW_SURFACE_RECT_BBOX_Y,bbox_y,
                                          MDR,SIZE,1,NULL);
          RLABCODE_PROCESS_BTREE_ENTRY_MD(e2,node,RLAB_GTS_NEW_SURFACE_RECT_BBOX_Z,bbox_z,
                                          MDR,SIZE,1,NULL);
          // Create four vertices from bounding box data
          GtsVertex *vx1=0, *vx2=0, *vx3=0, *vx4=0;
          if (SIZE(bbox_x)<2 && SIZE(bbox_y)==2 && SIZE(bbox_z)==2)
          {
            if (SIZE(bbox_x)==1)
            {
              offs = mdrV0(bbox_x,0);
            }
            vx1 = gts_vertex_new(gts_vertex_class(), offs, mdrV0(bbox_y,0), mdrV0(bbox_z,0));
            vx2 = gts_vertex_new(gts_vertex_class(), offs, mdrV0(bbox_y,0), mdrV0(bbox_z,1));
            vx3 = gts_vertex_new(gts_vertex_class(), offs, mdrV0(bbox_y,1), mdrV0(bbox_z,1));
            vx4 = gts_vertex_new(gts_vertex_class(), offs, mdrV0(bbox_y,1), mdrV0(bbox_z,0));
          }
          else if (SIZE(bbox_x)==2 && SIZE(bbox_y)<2 && SIZE(bbox_z)==2)
          {
            if (SIZE(bbox_y)==1)
            {
              offs = mdrV0(bbox_y,0);
            }
            vx1 = gts_vertex_new(gts_vertex_class(), mdrV0(bbox_x,0), offs, mdrV0(bbox_z,0));
            vx2 = gts_vertex_new(gts_vertex_class(), mdrV0(bbox_x,1), offs, mdrV0(bbox_z,0));
            vx3 = gts_vertex_new(gts_vertex_class(), mdrV0(bbox_x,1), offs, mdrV0(bbox_z,1));
            vx4 = gts_vertex_new(gts_vertex_class(), mdrV0(bbox_x,0), offs, mdrV0(bbox_z,1));
          }
          else if (SIZE(bbox_x)==2 && SIZE(bbox_y)==2 && SIZE(bbox_z)<2)
          {
            if (SIZE(bbox_z)==1)
            {
              offs = mdrV0(bbox_z,0);
            }
            vx1 = gts_vertex_new(gts_vertex_class(), mdrV0(bbox_x,0), mdrV0(bbox_y,0), offs);
            vx2 = gts_vertex_new(gts_vertex_class(), mdrV0(bbox_x,1), mdrV0(bbox_y,0), offs);
            vx3 = gts_vertex_new(gts_vertex_class(), mdrV0(bbox_x,1), mdrV0(bbox_y,1), offs);
            vx4 = gts_vertex_new(gts_vertex_class(), mdrV0(bbox_x,0), mdrV0(bbox_y,1), offs);
          }
          else
          {
            rerror(THIS_SOLVER ": Help, I need somebody! Help, Just anybody! Heeeelp!\n");
          }
          // create five edges of which 4 are constraints (boundaries)
          GtsEdge   *  e1 = gts_edge_new(gts_edge_class (), vx1, vx2);
          GtsEdge   *  e2 = gts_edge_new(gts_edge_class (), vx2, vx3);
          GtsEdge   *  e3 = gts_edge_new(gts_edge_class (), vx3, vx4);
          GtsEdge   *  e4 = gts_edge_new(gts_edge_class (), vx4, vx1);
          GtsEdge   *  e5 = gts_edge_new(gts_edge_class (), vx1, vx3);
          // Create two triangles
          GtsFace * f1 = gts_face_new (GTS_FACE_CLASS(color_face_class()), e1, e2, e5);
          COLOR_FACE(f1)->c.r = c.r;
          COLOR_FACE(f1)->c.g = c.g;
          COLOR_FACE(f1)->c.b = c.b;
          gts_surface_add_face (rf->s, f1);
          //
          GtsFace * f2 = gts_face_new (GTS_FACE_CLASS(color_face_class()), e5, e3, e4);
          COLOR_FACE(f2)->c.r = c.r;
          COLOR_FACE(f2)->c.g = c.g;
          COLOR_FACE(f2)->c.b = c.b;
          gts_surface_add_face (rf->s, f2);
        }
        else if (!strcmp(desc, "box"))
        {
          MDR * bbox_x=0, *bbox_y=0, *bbox_z=0, *remf=0;
          double offs=0.0;
          RLABCODE_PROCESS_BTREE_ENTRY_MD(e2,node,RLAB_GTS_NEW_SURFACE_RECT_BBOX_X,bbox_x,
                                          MDR,SIZE,1,NULL);
          RLABCODE_PROCESS_BTREE_ENTRY_MD(e2,node,RLAB_GTS_NEW_SURFACE_RECT_BBOX_Y,bbox_y,
                                          MDR,SIZE,1,NULL);
          RLABCODE_PROCESS_BTREE_ENTRY_MD(e2,node,RLAB_GTS_NEW_SURFACE_RECT_BBOX_Z,bbox_z,
                                          MDR,SIZE,1,NULL);
          RLABCODE_PROCESS_BTREE_ENTRY_MD(e2,node,RLAB_GTS_NEW_SURFACE_REMOVE_FACE,remf,
                                          MDR,SIZE,12,NULL);
          surface_add_box (rf->s, bbox_x, bbox_y, bbox_z, color, remf);
        }
        else if (!strcmp(desc, RLAB_GTS_NEW_SURFACE_CART_MESH))
        {
          Ent *params=0, *func=0;
          MDR *x=0, *y=0, *z=0;
          int nx=-1, ny=-1;

          // get x
          RLABCODE_PROCESS_BTREE_ENTRY_MD(e2,node,RLAB_GTS_NEW_SURFACE_CART_X,x,
                                          MDR,SIZE,1,NULL);
          nx = SIZE(x);
          // get y
          RLABCODE_PROCESS_BTREE_ENTRY_MD(e2,node,RLAB_GTS_NEW_SURFACE_CART_Y,y,
                                          MDR,SIZE,1,NULL);
          ny = SIZE(y);
          // get z: assume a data matrix
          RLABCODE_PROCESS_BTREE_ENTRY_MD(e2,node,RLAB_GTS_NEW_SURFACE_CART_Z,z,
                                          MDR,SIZE,1,NULL);
          if (z)
          {
            if ((MNR(z)!=nx)||(MNC(z)!=ny))
            {
              fprintf (rlab_stderr, THIS_SOLVER
                  ": MESH: Dim of 'z' must match Dim of 'x' and Dim of'y'\n");
              goto _exit;
            }
            r_gts_surface_from_cartesian_mesh(rf->s, x, y, z);
          }
        }

        if (iterate)
        {
          // perform all point-transformations specified in the struct 'st'
          MDR *vertex = mdr_Create(3,1);
          MDR *result = mdr_Create(3,1);
          st.vertex = vertex;
          st.result = result;
          gts_surface_foreach_vertex(rf->s, (GtsFunc) r_foreach_transform_vertices, (gpointer) &st);
          mdr_Destroy(vertex);
          mdr_Destroy(result);
        }

      } /*if (desc)*/

    } /*if (ent_type(e1) == BTREE)*/

  } /*if (nargs > 0)*/

_exit:

  ent_Clean(e1);
  ent_Clean(e2);
  ent_Clean(e3);
  return ent_Create_Rlab_Success ();
}




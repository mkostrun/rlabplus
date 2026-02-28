//
// function to be called on each object of the surface:
//    iteration continues irrespectively of function return value
//


#undef  THIS_SOLVER
#define THIS_SOLVER "gts_segment_intersect_surface"
Ent * ent_gts_segment_intersect_surface (int nargs, Datum args[])
{
  Ent *e1=0, *e2=0, *e3=0, *e4=0, *rent=0;
  FILE *rlab_stderr = (!RLAB_STDERR_DS) ? stderr : RLAB_STDERR_DS;
  char *name1=0;
  MDR *F=0, *N=0, *pts=0;
  double scale=1;

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
  e3 = bltin_get_ent(args[2);
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
//       RLABCODE_PROCESS_BTREE_ENTRY_BOOL(e4,node,RLAB_GTS_MODIFY_OBJ,vfn.modify_obj);
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

  //
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

_exit:

  if (pts_count)
  {
    void *f_args[2];
    pts = mdr_Create(pts_count,3);
    mdr_Zero(pts);
    pts_count = 0;
    f_args[0] = (void *) &pts_count;
    f_args[1] = (void *) pts;
    g_slist_foreach (pts_list, (GFunc) copy_slist_pt_to_mdr, f_args);
  }

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
  return ent_Create_Rlab_MDR(pts);
}



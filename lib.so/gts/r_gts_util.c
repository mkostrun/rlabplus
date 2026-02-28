static GtsEdge * new_edge (GtsVertex * v1, GtsVertex * v2)
{
  GtsSegment * s = gts_vertices_are_connected (v1, v2);
  return s == NULL ?
    gts_edge_new (GTS_EDGE_CLASS (gts_constraint_class ()), v1, v2) :
      GTS_EDGE (s);
}

static void copy_vertex_from_s_to_new (GtsPoint * p, gpointer * data)
{
  GtsSurface *snew = (GtsSurface *) data;
  GtsVertex *v  = gts_vertex_new(gts_vertex_class(), p->x, p->y, p->z);
  GtsVertex *v1 = gts_delaunay_add_vertex (snew, v, NULL);
  if (v1 != NULL)
  {
    if (v != v1)
    {
      gts_vertex_replace (v, v1);
    }
  }
  return;
}

static void build_list (gpointer data, GSList ** list)
{
  /* always use O(1) g_slist_prepend instead of O(n) g_slist_append */
  *list = g_slist_prepend (*list, data);
}

static void build_list1 (gpointer data, GList ** list)
{
  /* always use O(1) g_list_prepend instead of O(n) g_list_append */
  *list = g_list_prepend (*list, data);
}

static void build_list2 (GtsVertex * v, GList ** list)
{
  if (gts_vertex_is_boundary (v, NULL))
    *list = g_list_prepend (*list, v);
}

static void vertex_cleanup (GtsVertex * v)
{
  gts_vertex_is_contact (v, TRUE);
}

static void edge_cleanup (GtsSurface * surface)
{
  GSList * edges = NULL;
  GSList * i;

  g_return_if_fail (surface != NULL);

  /* build list of edges */
  gts_surface_foreach_edge (surface, (GtsFunc) build_list, &edges);

  /* remove degenerate and duplicate edges.
  Note: we could use gts_edges_merge() to remove the duplicates and then
  remove the degenerate edges but it is more efficient to do everything
  at once (and it's more pedagogical too ...) */

  /* We want to control manually the destruction of edges */
  gts_allow_floating_edges = TRUE;

  i = edges;
  while (i)
  {
    GtsEdge * e = i->data;
    GtsEdge * duplicate;
    if (GTS_SEGMENT (e)->v1 == GTS_SEGMENT (e)->v2)
    {
      /* edge is degenerate */
      /* destroy e */
      gts_object_destroy (GTS_OBJECT (e));
    }
    else if ((duplicate = gts_edge_is_duplicate (e)))
    {
      /* replace e with its duplicate */
      gts_edge_replace (e, duplicate);
      /* destroy e */
      gts_object_destroy (GTS_OBJECT (e));
    }
    i = i->next;
  }

  /* don't forget to reset to default */
  gts_allow_floating_edges = FALSE;

  /* free list of edges */
  g_slist_free (edges);
}

static void triangle_cleanup (GtsSurface * s)
{
  GSList * triangles = NULL;
  GSList * i;

  g_return_if_fail (s != NULL);

  /* build list of triangles */
  gts_surface_foreach_face (s, (GtsFunc) build_list, &triangles);

  /* remove duplicate triangles */
  i = triangles;
  while (i) {
    GtsTriangle * t = i->data;
    if (gts_triangle_is_duplicate (t))
      /* destroy t, its edges (if not used by any other triangle)
      and its corners (if not used by any other edge) */
      gts_object_destroy (GTS_OBJECT (t));
    i = i->next;
  }

  /* free list of triangles */
  g_slist_free (triangles);
}

static gboolean check_boundaries (GtsVertex * v1, GtsVertex * v2)
{
  return (g_slist_length (v1->segments) < 4 &&
      g_slist_length (v2->segments) < 4);
}

void r_gts_surface_cleanup(GtsSurface *s, gdouble threshold)
{
  /* merge vertices which are close enough */
  /* build list of vertices */
  gboolean verbose = FALSE, sever = FALSE, boundary = FALSE;

  GList * vertices = NULL;

  gts_surface_foreach_vertex (s, boundary ? (GtsFunc) build_list2 : (GtsFunc) build_list1, &vertices);

  /* merge vertices: we MUST update the variable vertices because this function
     modifies the list (i.e. removes the merged vertices). */
//   fprintf(stderr, "before: gts_vertices_merge\n");
  vertices = gts_vertices_merge (vertices, threshold, check_boundaries);

  /* free the list */
//   fprintf(stderr, "before: g_list_free\n");
  g_list_free (vertices);

  /* eliminate degenerate and duplicate edges */
//   fprintf(stderr, "before: edge_cleanup\n");
  edge_cleanup (s);

  /* eliminate duplicate triangles */
//   fprintf(stderr, "before: triangle_cleanup\n");
  triangle_cleanup (s);

//   fprintf(stderr, "done\n");

  return;
}

static gboolean triangle_is_hole (GtsTriangle * t)
{
  GtsEdge * e1, * e2, * e3;
  GtsVertex * v1, * v2, * v3;

  gts_triangle_vertices_edges (t, NULL, &v1, &v2, &v3, &e1, &e2, &e3);

  if ( (GTS_IS_CONSTRAINT (e1) && GTS_SEGMENT (e1)->v1 != v1) ||
       (GTS_IS_CONSTRAINT (e2) && GTS_SEGMENT (e2)->v1 != v2) ||
       (GTS_IS_CONSTRAINT (e3) && GTS_SEGMENT (e3)->v1 != v3))
  {
//     printf("Have a hole!\n");
    return 1;
  }

//   printf("Nope!\n");
  return 0;
}

static guint delaunay_remove_holes (GtsSurface * surface)
{
  g_return_val_if_fail (surface != NULL, 0);
  return gts_surface_foreach_face_remove (surface, (GtsFunc) triangle_is_hole, NULL);
}

static int r_gts_find_face_containing_vertex(gpointer item, gpointer data)
{
  GtsTriangle *t = (GtsTriangle *) item;
  if (!t)
    return 1;

  double d;

  r_gts_void_ptrs * x = (r_gts_void_ptrs *) data;
  if (!x)
    return 1;

  GtsVertex *vx1 = x->vptr1;
  if (!vx1)
    return 1;

  d = gts_point_triangle_distance (GTS_POINT(vx1), t);
  if (d > r_gts_eabs)
  {
    return 0;
  }
}

static int r_gts_remove_face_given_3vertices(gpointer item, gpointer data)
{
  GtsTriangle *t = (GtsTriangle *) item;

  double d1, d2, d3;
  int count=0;
  r_gts_face_vx * x = (r_gts_face_vx * ) data;
  GtsSurface *s =  (GtsSurface *) x->s;
  GtsVertex *vx1 = (GtsVertex *) x->vx1;
  GtsVertex *vx2 = (GtsVertex *) x->vx2;
  GtsVertex *vx3 = (GtsVertex *) x->vx3;

  GtsVertex *tv1, *tv2, *tv3;
  gts_triangle_vertices (t, &tv1, &tv2, &tv3);

  d1 = gts_point_distance(GTS_POINT(vx1), GTS_POINT(tv1));
  d2 = gts_point_distance(GTS_POINT(vx1), GTS_POINT(tv2));
  d3 = gts_point_distance(GTS_POINT(vx1), GTS_POINT(tv3));
  if ((d1 > 1e-3)&&(d2 > 1e-3)&&(d3 > 1e-3))
  {
    return 0;
  }

  d1 = gts_point_distance(GTS_POINT(vx2), GTS_POINT(tv1));
  d2 = gts_point_distance(GTS_POINT(vx2), GTS_POINT(tv2));
  d3 = gts_point_distance(GTS_POINT(vx2), GTS_POINT(tv3));
  if ((d1 > 1e-3)&&(d2 > 1e-3)&&(d3 > 1e-3))
  {
    return 0;
  }

  d1 = gts_point_distance(GTS_POINT(vx3), GTS_POINT(tv1));
  d2 = gts_point_distance(GTS_POINT(vx3), GTS_POINT(tv2));
  d3 = gts_point_distance(GTS_POINT(vx3), GTS_POINT(tv3));
  if ((d1 > 1e-3)&&(d2 > 1e-3)&&(d3 > 1e-3))
  {
    return 0;
  }

//   fprintf(stderr, "vx1: %g,%g,%g\n", vx1->p.x, vx1->p.y, vx1->p.z);
//   fprintf(stderr, "vx2: %g,%g,%g\n", vx2->p.x, vx2->p.y, vx2->p.z);
//   fprintf(stderr, "vx3: %g,%g,%g\n", vx3->p.x, vx3->p.y, vx3->p.z);
//   fprintf(stderr, "tv1: %g,%g,%g\n", tv1->p.x, tv1->p.y, tv1->p.z);
//   fprintf(stderr, "tv2: %g,%g,%g\n", tv2->p.x, tv2->p.y, tv2->p.z);
//   fprintf(stderr, "tv3: %g,%g,%g\n", tv3->p.x, tv3->p.y, tv3->p.z);

  (x->count)++;
  return 1;
}

static int r_gts_remove_face_given_3vertices_old(gpointer item, gpointer data)
{
  GtsTriangle *t = (GtsTriangle *) item;

  double d;
  r_gts_face_vx * x = (r_gts_face_vx * ) data;
  GtsVertex *vx1 = (GtsVertex *) x->vx1;
  GtsVertex *vx2 = (GtsVertex *) x->vx2;
  GtsVertex *vx3 = (GtsVertex *) x->vx3;

  d = gts_point_triangle_distance (GTS_POINT(vx1), t);
  if (d > r_gts_eabs)
  {
    return 0;
  }

  d = gts_point_triangle_distance (GTS_POINT(vx2), t);
  if (d > r_gts_eabs)
  {
    return 0;
  }

  d = gts_point_triangle_distance (GTS_POINT(vx3), t);
  if (d > r_gts_eabs)
  {
    return 0;
  }

  (x->count)++;
  return 1;
}

static int r_copy_vertex_to_mdr (gpointer item, gpointer data)
{
  static int counter;
  MDR *vx = (MDR *) data;
  GtsVertex *v = (GtsVertex *) item;

  if (!v)
  {
    counter = 0;
    return 0;
  }

  Mdr0(vx,counter,0) = v->p.x;
  Mdr0(vx,counter,1) = v->p.y;
  Mdr0(vx,counter,2) = v->p.z;
  counter ++;

  return 0;
}

static int r_replace_mdr_with_mdr (gpointer item, gpointer d)
{
  static int r_row;
  GtsVertex *v = (GtsVertex *) item;
  r_gts_func_vertex *data = (r_gts_func_vertex *) d;
  MDR *old_vx = (MDR *) data->old_vx;
  MDR *new_vx = (MDR *) data->new_vx;
  int *idx_row = (int *) data->idx;
  int i, j;

  if (!v)
  {
    for (i=0; i<MNR(old_vx); i++)
    {
      idx_row[i] = 1;
    }
    r_row = MNR(old_vx);
    return 0;
  }

  // nothing to do: have to wait until g_iterate completes
  if (!r_row)
    return 0;

  //
  // compare vertex with each of the entries in old_vx
  //
  i = 0;
  for (j=0; j < MNR(old_vx); j++)
  {
    //
    // if i-th row is marked as processed, move on to next
    //
    if (!idx_row[i])
    {
      i++;
      continue;
    }

    //
    // if i-th row has been found to match the vertex, replace it
    //
    if ((Mdr0(old_vx,i,0) == v->p.x) && (Mdr0(old_vx,i,1) == v->p.y) && (Mdr0(old_vx,i,2) == v->p.z))
    {
      GtsVertex *with=gts_vertex_new(gts_vertex_class(), Mdr0(new_vx,i,0), Mdr0(new_vx,i,1), Mdr0(new_vx,i,2));
      gts_vertex_replace (v, with);
      idx_row[i] = 0;
      i++;
      r_row--;
    }
  }

  return 0;
}


//
// vertex:
//    get all vertices,edge,faces associated with a surface
//    delete vertices associated with a surface
//
static void export_vertex (GtsPoint * p, gpointer * data)
{
  MDR *vertex = (MDR *) data[0];
  int *ir = (int *) data[1];

  Mdr0(vertex,*ir,0) = p->x;
  Mdr0(vertex,*ir,1) = p->y;
  Mdr0(vertex,*ir,2) = p->z;

  g_hash_table_insert (data[2], p, GUINT_TO_POINTER (++(*((guint *) data[1]))));

  return;
}

static void export_edge(GtsSegment * s, gpointer * data)
{
  MDR *edge = data[0];
  int *ir = data[1];

  Mdr0(edge,*ir,0) = GPOINTER_TO_UINT (g_hash_table_lookup (data[2], s->v1));
  Mdr0(edge,*ir,1) = GPOINTER_TO_UINT (g_hash_table_lookup (data[2], s->v2));

  g_hash_table_insert (data[3], s, GUINT_TO_POINTER (++(*((guint *) data[1]))));
  return;
}

static void export_face (GtsTriangle * t, gpointer * data)
{
  MDR *face = data[0];
  MDR *color = data[4];
  int *ir = data[1];

  Mdr0(face,*ir,0) = GPOINTER_TO_UINT (g_hash_table_lookup (data[3], t->e1));
  Mdr0(face,*ir,1) = GPOINTER_TO_UINT (g_hash_table_lookup (data[3], t->e2));
  Mdr0(face,*ir,2) = GPOINTER_TO_UINT (g_hash_table_lookup (data[3], t->e3));

  GtsFace *f = GTS_FACE(t);
  Mdr0(color,*ir,0) = COLOR_FACE(f)->c.r;
  Mdr0(color,*ir,1) = COLOR_FACE(f)->c.g;
  Mdr0(color,*ir,2) = COLOR_FACE(f)->c.b;

  ++(*ir);

  return;
}

void r_gts_surface_export (GtsSurface * s, MDR **vertex, MDR **edge, MDR **face, MDR **color)
{
  int n;
  void *data[5];


  n = gts_surface_vertex_number(s);
  if (n < 1)
  {
    goto _exit;
  }
  *vertex = mdr_Create(n,3);
  GHashTable *vindex=g_hash_table_new (NULL, NULL);
  GHashTable *eindex=g_hash_table_new (NULL, NULL);
  n = 0;
  data[0] = (void *) *vertex;
  data[1] = &n;
  data[2] = vindex;
  data[3] = eindex;
  gts_surface_foreach_vertex (s, (GtsFunc) export_vertex, data);

  n = gts_surface_edge_number(s);
  *edge = mdr_Create(n,2);
  n = 0;
  data[0] = *edge;
  data[1] = &n;
  data[2] = vindex;
  data[3] = eindex;
  gts_surface_foreach_edge (s, (GtsFunc) export_edge, data);

  n = gts_surface_face_number(s);
  *face  = mdr_Create(n,3);
  *color = mdr_Create(n,3);
  n = 0;
  data[0] = *face;
  data[1] = &n;
  data[2] = vindex;
  data[3] = eindex;
  data[4] = *color;
  gts_surface_foreach_face (s, (GtsFunc) export_face, data);

  g_hash_table_destroy (vindex);
  g_hash_table_destroy (eindex);

_exit:

  return;
}


typedef struct _CartesianGrid CartesianGrid;

struct _CartesianGrid {
  GtsVertex *** vertices;
  guint nx, ny;
  gdouble xmin, xmax, ymin, ymax;
};

static void ** malloc2D (guint nx, guint ny, gulong size)
{
  void ** m = g_malloc (nx*sizeof (void *));
  guint i;

  for (i = 0; i < nx; i++)
    m[i] = g_malloc0 (ny*size);

  return m;
}

static void free2D (void ** m, guint nx)
{
  guint i;

  g_return_if_fail (m != NULL);

  for (i = 0; i < nx; i++)
    g_free (m[i]);
  g_free (m);
}

static CartesianGrid * cartesian_grid_new (guint nx, guint ny)
{
  CartesianGrid * grid;

  grid = g_malloc (sizeof (CartesianGrid));
  grid->vertices = (GtsVertex ***) malloc2D (nx, ny, sizeof (GtsVertex *));
  grid->nx = nx;
  grid->ny = ny;
  grid->xmin = G_MAXDOUBLE;
  grid->xmax = - G_MAXDOUBLE;
  grid->ymin = G_MAXDOUBLE;
  grid->ymax = - G_MAXDOUBLE;

  return grid;
}

static void cartesian_grid_destroy (CartesianGrid * g,
                                    gboolean destroy_vertices)
{
  g_return_if_fail (g != NULL);

  if (destroy_vertices) {
    guint i, j;

    gts_allow_floating_vertices = TRUE;
    for (i = 0; i < g->nx; i++)
      for (j = 0; j < g->ny; j++)
        if (g->vertices[i][j])
          gts_object_destroy (GTS_OBJECT (g->vertices[i][j]));
    gts_allow_floating_vertices = FALSE;
  }

  free2D ((void **) g->vertices, g->nx);
  g_free (g);
}

static CartesianGrid * cartesian_grid_from_mdr (GtsVertexClass * klass,
    MDR *x, MDR *y, MDR *z)
{
  CartesianGrid * grid;
  guint nx, ny, i, j;

  if (!klass)
  {
    return NULL;
  }

  nx = SIZE(x);
  ny = SIZE(y);
  if ((nx != MNR(z)) || (ny != MNC(z)))
  {
    return NULL;
  }

  grid = cartesian_grid_new (nx, ny);
  gdouble vx,vy,vz;
  for (i=0; i<nx; i++)
  {
    for (j=0; j<ny; j++)
    {
      vx = mdrV0(x,i);
      vy = mdrV0(y,j);
      vz = mdr0(z,i,j);
      //
      if (!isnand(vz))
      {
        grid->vertices[i][j] = gts_vertex_new (klass, vx, vy, vz);
      }
      if (vx > grid->xmax)
        grid->xmax = vx;
      if (vx < grid->xmin)
        grid->xmin = vx;
      if (vy > grid->ymax)
        grid->ymax = vy;
      if (vy < grid->ymin)
        grid->ymin = vy;
    }
  }
  return grid;
}

static void cartesian_grid_triangulate (CartesianGrid * g,
                                        GtsSurface * s)
{
  gint i, j;
  GtsVertex *** v;

  g_return_if_fail (g != NULL);
  g_return_if_fail (s != NULL);

  v = g->vertices;
  for (i = 0; i < g->nx - 1; i++)
    for (j = 0; j < g->ny - 1; j++)
      if (v[i][j]) {
    if (v[i][j+1]) {
      if (v[i+1][j+1]) {
        GtsEdge * e1 = new_edge (v[i][j+1], v[i][j]);
        GtsEdge * e2 =
            gts_edge_new (GTS_EDGE_CLASS (gts_constraint_class ()),
                          v[i][j], v[i+1][j+1]);
        GtsEdge * e3 = new_edge (v[i+1][j+1], v[i][j+1]);
        gts_surface_add_face (s, gts_face_new (s->face_class, e1, e2, e3));
        if (v[i+1][j]) {
          e1 = new_edge (v[i+1][j], v[i+1][j+1]);
          e3 = new_edge (v[i][j], v[i+1][j]);
          gts_surface_add_face (s,
                                gts_face_new (s->face_class, e1, e2, e3));
        }
      }
      else if (v[i+1][j]) {
        GtsEdge * e1 = new_edge (v[i][j+1], v[i][j]);
        GtsEdge * e2 = new_edge (v[i][j], v[i+1][j]);
        GtsEdge * e3 =
            gts_edge_new (GTS_EDGE_CLASS (gts_constraint_class ()),
                          v[i+1][j], v[i][j+1]);
        gts_surface_add_face (s, gts_face_new (s->face_class, e1, e2, e3));
      }
    }
    else if (v[i+1][j] && v[i+1][j+1]) {
      GtsEdge * e1 = new_edge (v[i][j], v[i+1][j]);
      GtsEdge * e2 = new_edge (v[i+1][j], v[i+1][j+1]);
      GtsEdge * e3 =
          gts_edge_new (GTS_EDGE_CLASS (gts_constraint_class ()),
                        v[i+1][j+1], v[i][j]);
      gts_surface_add_face (s, gts_face_new (s->face_class, e1, e2, e3));
    }
      }
      else if (v[i][j+1] && v[i+1][j+1] && v[i+1][j]) {
        GtsEdge * e1 = new_edge (v[i+1][j], v[i+1][j+1]);
        GtsEdge * e2 = new_edge (v[i+1][j+1], v[i][j+1]);
        GtsEdge * e3 =
            gts_edge_new (GTS_EDGE_CLASS (gts_constraint_class ()),
                          v[i][j+1], v[i+1][j]);
        gts_surface_add_face (s, gts_face_new (s->face_class, e1, e2, e3));
      }
}

void r_gts_surface_from_cartesian_mesh(GtsSurface *s, MDR *x, MDR *y, MDR *z)
{
  CartesianGrid * grid = cartesian_grid_from_mdr (gts_vertex_class (), x, y, z);
  cartesian_grid_triangulate (grid, s);
  cartesian_grid_destroy (grid, FALSE);
  return;
}

static guint generate_octohedron (GtsSurface * s)
{
  GtsVertex * v01 = gts_vertex_new (s->vertex_class,  1, 0, 0);
  GtsVertex * v02 = gts_vertex_new (s->vertex_class,  0, 1, 0);
  GtsVertex * v03 = gts_vertex_new (s->vertex_class, -1, 0, 0);
  GtsVertex * v04 = gts_vertex_new (s->vertex_class,  0,-1, 0);
  GtsVertex * v05 = gts_vertex_new (s->vertex_class,  0, 0,-1);
  GtsVertex * v06 = gts_vertex_new (s->vertex_class,  0, 0, 1);

  GtsEdge * e01 = gts_edge_new (s->edge_class, v01, v02);
  GtsEdge * e02 = gts_edge_new (s->edge_class, v02, v03);
  GtsEdge * e03 = gts_edge_new (s->edge_class, v03, v04);
  GtsEdge * e04 = gts_edge_new (s->edge_class, v04, v01);
  GtsEdge * e05 = gts_edge_new (s->edge_class, v01, v05);
  GtsEdge * e06 = gts_edge_new (s->edge_class, v02, v05);
  GtsEdge * e07 = gts_edge_new (s->edge_class, v03, v05);
  GtsEdge * e08 = gts_edge_new (s->edge_class, v04, v05);
  GtsEdge * e09 = gts_edge_new (s->edge_class, v01, v06);
  GtsEdge * e10 = gts_edge_new (s->edge_class, v02, v06);
  GtsEdge * e11 = gts_edge_new (s->edge_class, v03, v06);
  GtsEdge * e12 = gts_edge_new (s->edge_class, v04, v06);

  gts_surface_add_face (s, gts_face_new (s->face_class, e01, e09, e10)); /* v1 v2 v6 */
  gts_surface_add_face (s, gts_face_new (s->face_class, e01, e05, e06)); /* v1 v2 v5 */
  gts_surface_add_face (s, gts_face_new (s->face_class, e02, e10, e11)); /* v2 v3 v6 */
  gts_surface_add_face (s, gts_face_new (s->face_class, e02, e06, e07)); /* v2 v3 v5 */
  gts_surface_add_face (s, gts_face_new (s->face_class, e03, e11, e12)); /* v3 v4 v6 */
  gts_surface_add_face (s, gts_face_new (s->face_class, e03, e07, e08)); /* v3 v4 v5 */
  gts_surface_add_face (s, gts_face_new (s->face_class, e04, e12, e09)); /* v4 v1 v6 */
  gts_surface_add_face (s, gts_face_new (s->face_class, e04, e08, e05)); /* v4 v1 v5 */

  return 0;
}

/**
 * gts_surface_generate_sphere:
 * @s: a #GtsSurface.
 * @geodesation_order: a #guint.
 *
 * Add a triangulated unit sphere generated by recursive subdivision to @s.
 * First approximation is an isocahedron; each level of refinement
 * (@geodesation_order) increases the number of triangles by a factor of 4.
 * http://mathworld.wolfram.com/GeodesicDome.html
 *
 * Returns: @s.
 */
void * r_gts_surface_generate_sphere (GtsSurface * s, guint geodesation_order)
{
  guint cgo;

  g_return_val_if_fail (s != NULL, NULL);
  g_return_val_if_fail (geodesation_order != 0, NULL);

  generate_octohedron (s);

  for (cgo = 1; cgo < geodesation_order; cgo++)
    gts_surface_tessellate (s, NULL, NULL);

  return (s);
}





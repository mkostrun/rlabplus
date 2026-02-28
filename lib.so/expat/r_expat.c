// Copyright (C) 2003-2013 Marijan Kostrun
//   part of rlabplus for linux project on rlabplus.sourceforge.net
//
// ngspice wrapper for rlabplus
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



// ngspice specific
#include "r_expat.h"

static void XMLCALL
startElement(void *userData, const XML_Char *name, const XML_Char **atts) {
  int i,nattr=0;
  Btree *bnew=0, *battr=0;
  XML_rlab_list * dataptr = (XML_rlab_list *) userData;

  dataptr->depth += 1;
  if (dataptr->node_name[dataptr->depth])
  {
    GC_FREE(dataptr->node_name[dataptr->depth]);
  }
  dataptr->node_name[dataptr->depth] = cpstr((char *)name);
  ListNode *node;

  // is new node already defined in parent node
  Btree *bt_prev = dataptr->node_btree[dataptr->depth - 1];
  node = btree_FindNode(bt_prev, (char *)name);
  if (!node)
  {
    bnew = btree_Create();
    install(bt_prev, (char *) name, ent_Assign_Rlab_BTREE(bnew));
  }
  else
  {
    bnew = ent_data(var_ent(node));
  }
  dataptr->node_btree[dataptr->depth] = bnew;

  /* do we need to process attributes ?*/
  for (i=0; atts[i]; i += 2)
  {
    nattr++;
  }
  if (nattr == 0)
    return; /* Nothing to do if there is no attributes to add */

  // check if node already has attr attached to it
  node = btree_FindNode(bnew, "attr");
  if (!node)
  {
    battr = btree_Create();
    install(bnew, "attr", ent_Assign_Rlab_BTREE(battr));
  }
  else
  {
#ifdef DEBUG
    printf("Node 'attr' exists already\n");
#endif
    battr = ent_data(var_ent(node));
  }

  // go over attributes and overwrite them or append them
  for (i=0; atts[i]; i += 2)
  {
    node = btree_FindNode(battr, (char *)atts[i]);
    if (!node)
    {
      install(battr, (char *)atts[i], ent_Create_Rlab_String((char *)atts[i+1]));
    }
    else
    {
      MDS *s2 = ent_data(var_ent(node));
      if (MNC(s2)!=1)
      {
        printf(_THIS_LIB ": The attribute has to be vector: Cannot append!");
      }
      else
      {
        mds_Extend(s2,MNR(s2)+1,MNC(s2));
        MdsV0(s2,SIZE(s2)-1) = cpstr((char *)atts[i+1]);
        ent_data(var_ent(node)) = s2;
      }
    }
  }

#ifdef DEBUG
  for (i=0;i<dataptr->depth;i++)
    printf("/%s", dataptr->node_name[i]);
  for (i=0; atts[i]; i += 2)
  {
    printf(" %" XML_FMT_STR "='%" XML_FMT_STR "'", atts[i], atts[i + 1]);
  }
  printf("\n");
#endif
}

static void XMLCALL
endElement(void *userData, const XML_Char *name)
{
  XML_rlab_list * dataptr = (XML_rlab_list *) userData;
  (void) name;
  if (dataptr->node_name[dataptr->depth])
  {
    GC_FREE(dataptr->node_name[dataptr->depth]);
    dataptr->node_name [dataptr->depth] = NULL;
  }
  dataptr->node_btree[dataptr->depth] = NULL;
  dataptr->depth -= 1;
}

#undef  THIS_SOLVER
#define THIS_SOLVER "xml_parse"
Ent * ent_expat_parse (int nargs, Datum args[])
{
  Ent *e1=0, *e2=0;
  int i, j, len, len1, depth=0, min_depth=0;
  MDS *s=0;
  ListNode *node=0;
  char *xml_buffer = (char *) string_buff;

  XML_rlab_list RLABVAR;
  XML_Parser parser=NULL;
  Btree *rval=btree_Create();
  RLABVAR.depth = 0;
  RLABVAR.node_name [0] = (char *) cpstr("ROOT");
  RLABVAR.node_btree[0] = rval;
  for(i=1; i<MAX_DEPTH; i++)
  {
    RLABVAR.node_name [i] = NULL;
    RLABVAR.node_btree[i] = NULL;
  }

  if (nargs < 1)
  {
    fprintf(stderr, THIS_SOLVER ": " RLAB_ERROR_ARG_1 "\n");
    goto _exit;
  }
  e1 = bltin_get_ent (args[0]);
  if (ent_type(e1)!=MATRIX_DENSE_STRING)
  {
    fprintf(stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_MDS "\n");
    goto _exit;
  }
  s = ent_data(e1);
  if (SIZE(s)<1)
  {
    fprintf(stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_MDS "\n");
    goto _exit;
  }

  len = 0;
  for (i=0; i<MNR(s); i++)
  {
    for (j=0; j<MNC(s); j++)
    {
      len1 = isvalidstring(Mds0(s,i,j));
      if (len+len1 >= MAX_STRING_BUFF)
      {
          fprintf(stderr, THIS_SOLVER ": Total length of entries in string matrix has to be" );
          fprintf(stderr, " less than %d. If this is too small change macro MAX_STRING_BUFF",
                  MAX_STRING_BUFF);
          fprintf(stderr, " and recompile rlab\n");
          goto _exit;
      }

      if (len1 >=0)
      {
        if (len1 > 0)
        {
          strncpy(&xml_buffer[len],Mds0(s,i,j), len1);
          len += len1;
        }
        strncpy(&xml_buffer[len],"\n",1);
        len ++;
        xml_buffer[len] = 0;
      }
    }
  }

  if (len < 1)
  {
    fprintf(stderr, THIS_SOLVER ": " RLAB_ERROR_ARG1_MDS);
    goto _exit;
  }

  if (nargs > 1)
  {
    e2 = bltin_get_ent (args[1]);
    if (ent_type(e2)==BTREE)
    {
      RLABCODE_PROCESS_BTREE_ENTRY_D(e2,node,"depth_min",min_depth,class_double,>=,0.0);
    }
  }

  // lets do it!
  parser = (XML_Parser) XML_ParserCreate(NULL);
  XML_SetUserData(parser, &RLABVAR);
  XML_SetElementHandler(parser, startElement, endElement);

  if (XML_Parse(parser, xml_buffer, len, 1) == XML_STATUS_ERROR)
  {
    fprintf(stderr,
            "Parse error at line %" XML_FMT_INT_MOD "u:\n%" XML_FMT_STR "\n",
            XML_GetCurrentLineNumber(parser),
            XML_ErrorString(XML_GetErrorCode(parser)));
    goto _exit;
  }


_exit:

  ent_Clean(e1);
  ent_Clean(e2);

  if (parser)
    XML_ParserFree(parser);

  for(i=0; i<MAX_DEPTH; i++)
  {
    if (RLABVAR.node_name[i])
    {
      GC_FREE(RLABVAR.node_name[i]);
      RLABVAR.node_name[i] = NULL;
    }
  }

  return ent_Assign_Rlab_BTREE(rval);
}



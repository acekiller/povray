/************************************************************************
*  Oct-tree routine prototypes.  Use by Radiosity calculation routies.
*  Implemented by and (c) 1994 Jim McElhiney, mcelhiney@acm.org or cserve 71201,1326
*  All standard POV distribution rights granted.  All other rights reserved.
*
*************************************************************************/

#ifndef OCTREE_H
#define OCTREE_H



/*****************************************************************************
* Global preprocessor defines
******************************************************************************/

#define OT_BIAS 10000000.

#define MAX3(a,b,c) ( ((a)>(b)) ? max((a),(c)) : max((b),(c)) )


/*****************************************************************************
* Global typedefs
******************************************************************************/

typedef struct ot_block_struct OT_BLOCK;
typedef struct ot_id_struct OT_ID;
typedef struct ot_node_struct OT_NODE;

/* Each node in the oct-tree has a (possibly null) linked list of these
   data blocks off it.  */
struct ot_block_struct
{
  OT_BLOCK *next;
  VECTOR Point, S_Normal;
  float  drdx, dgdx, dbdx,  drdy, dgdy, dbdy,  drdz, dgdz, dbdz;
  RGB    Illuminance;
  float  Harmonic_Mean_Distance, Nearest_Distance;
  VECTOR To_Nearest_Surface;
  short  Bounce_Depth;
};

/* This is the information necessary to name an oct-tree node. */
struct ot_id_struct
{
  long x, y, z;
  long Size;
};

/* These are the structures that make up the oct-tree itself, known as nodes */
struct ot_node_struct
{
  OT_ID    Id;
  OT_BLOCK *Values;
  OT_NODE  *Kids[8];
};


/*****************************************************************************
* Global variables
******************************************************************************/



/*****************************************************************************
* Global functions
******************************************************************************/

void ot_ins PARAMS((OT_NODE **root, OT_BLOCK *new_block, OT_ID *new_id));
void ot_list_insert PARAMS((OT_BLOCK **list_ptr, OT_BLOCK *item));
void ot_newroot PARAMS((OT_NODE **root_ptr));
long ot_dist_traverse PARAMS((OT_NODE *subtree, VECTOR point, int bounce_depth,  \
               long (*func)(OT_BLOCK *block, void *handle1), void *handle2));
long ot_point_in_node PARAMS((VECTOR point, OT_ID *node));
void ot_index_sphere PARAMS((VECTOR point, DBL radius, OT_ID *id));
void ot_index_box PARAMS((VECTOR min_point, VECTOR max_point, OT_ID *id));
void ot_parent PARAMS((OT_ID *dad, OT_ID *kid));
long ot_save_tree PARAMS((OT_NODE *rootptr, FILE *fd));
long ot_write_block PARAMS((OT_BLOCK *bl, void * handle));
long ot_free_tree PARAMS((OT_NODE **ppRoot));
long ot_read_file PARAMS((FILE * fd));


/* a trunc function which always returns the floor integer */
long Trunc PARAMS((DBL value));



#endif

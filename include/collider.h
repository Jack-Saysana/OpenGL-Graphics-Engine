#include <glad/glad.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <structs/models/entity_str.h>

#define F_TOP    (0)
#define F_BOTTOM (1)
#define F_RIGHT  (2)
#define F_LEFT   (3)
#define F_FRONT  (4)
#define F_BACK   (5)

typedef struct {
  unsigned int verts[8];
  vec3 norm;
} COL_FACE;

// ====================== INTERNALLY DEFINED FUNCTIONS =======================

static int validate_col_faces(COL_FACE *, size_t);
static void triangulate_col_faces(COLLIDER *, COL_FACE *, unsigned int (*)[4]);
static void triangulate_col_face(COLLIDER *, unsigned int *, vec3);
static void orient_faces(unsigned int (*)[4], unsigned int *);
static int find_face_from_edge(unsigned int (*)[4], unsigned int, ivec2);
static void canonicalize_topology(COLLIDER *col, unsigned int(*)[4],
                                  unsigned int *, vec3 *);

// ====================== EXTERNALLY DEFINED FUNCTIONS =======================


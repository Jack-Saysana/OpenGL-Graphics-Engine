#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define BUFF_STARTING_LEN (10)
#define NUM_PROPS (5)

typedef enum chain_type {
  LOCATION = 0,
  ROTATION = 1,
  SCALE = 2
} C_TYPE;

typedef struct material {
  uint64_t name;
  char *mat_paths[NUM_PROPS];
} MATERIAL;

typedef struct line_buffer {
  char *dir;
  char *filename;
  char **buffer;
  size_t len;
} LINE_BUFFER;

typedef struct face_vert {
  struct face_vert *prev;
  struct face_vert *next;
  int index;
} FACE_VERT;

typedef struct bone {
  float coords[3];
  int num_children;
} BONE;




typedef struct animation {
  K_CHAIN *keyframe_chains;
  size_t num_chains;
} ANIMATION;

typedef struct keyframe_chain {
  KEYFRAME *chain;
  size_t num_frames;
  unsigned int b_id;
  C_TYPE type;
  unsigned int start_frame;
} K_CHAIN;

typedef struct keyframe {
  float offset[4];
  unsigned int frame;
} KEYFRAME;





BONE *bones;
size_t b_buff_len;
size_t b_len;

ANIMATION *animations;
size_t a_buff_len;
size_t a_len;

int (*bone_ids)[4];
float (*bone_weights)[4];

float (*verticies)[3];
size_t v_buff_len;
size_t v_len;

float (*normals)[3];
size_t n_buff_len;
size_t n_len;

float (*tex_coords)[2];
size_t t_buff_len;
size_t t_len;

int (*vbo_index_combos)[3];
size_t vbo_buff_len;
size_t vbo_len;

int (*faces)[3];
size_t face_buff_len;
size_t f_len;

MATERIAL *materials;
size_t mat_buff_len;
size_t mat_len;

int preprocess_lines(LINE_BUFFER *);
int preprocess_face(FILE *, char *);
int triangulate_polygon(FILE *, FACE_VERT *, size_t);
int is_ear(int *, FACE_VERT *, float *);
uint64_t get_hash(char *str);
int parse_mtllib(MATERIAL *materials, size_t *mat_buff_len, size_t *mat_len,
                 char *dir, char *lib);
void free_line_buffer(LINE_BUFFER *);
void free_materials(void *buffer, size_t buf_len);
void free_animations(ANIMATION *animations, size_t a_len);
int double_buffer(void **, size_t *, size_t);

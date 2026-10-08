#ifndef __COL_CHECK_ARGS_STR_H__
#define __COL_CHECK_ARGS_STR_H__

typedef struct collision_update {
  COLLISION col;
  void (*move_cb)(ENTITY *, vec3);
  int (*is_moving_cb)(ENTITY *, size_t);
} COL_UPDATE;

typedef struct col_check_args {
  pthread_mutex_t *col_lock;
  size_t start;
  size_t end;
  SIMULATION *sim;
  COL_UPDATE **collisions;
  size_t *buf_len;
  size_t *buf_size;
  vec3 origin;
  float range;
  int get_col_info;
} C_ARGS;

#endif

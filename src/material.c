#include <material.h>

int parse_mtllib(MATERIAL *materials, size_t *mat_buff_len, size_t *mat_len,
                 char *dir, char *lib) {
  char *lib_path = malloc(strlen(dir) + strlen(lib) + 2);
  if (lib_path == NULL) {
    printf("Unable to allocate mtllib path\n");
    return -1;
  }
  sprintf(lib_path, "%s/%s", dir, lib);

  LINE_BUFFER *lib_lines = get_lines(lib_path);
  free(lib_path);
  if (lib_lines == NULL) {
    printf("Unable to get mtllib line buffer\n");
    return -1;
  }

  MATERIAL *cur_mat = materials + *mat_len;
  char *cur_line = NULL;
  int status = 0;
  for (int i = 0; i < lib_lines->len; i++) {
    cur_line = lib_lines->buffer[i];
    PROP_TYPE op = get_op(&cur_line);

    if (op == NEWMTL) {
      cur_mat = materials + *mat_len;
      cur_mat->name = get_hash(cur_line);
      cur_mat->mat_paths[AMB] = NULL;
      cur_mat->mat_paths[DIFF] = NULL;
      cur_mat->mat_paths[SPEC] = NULL;
      cur_mat->mat_paths[SPEC_EXPONENT] = NULL;
      cur_mat->mat_paths[BUMP] = NULL;

      (*mat_len)++;
      if (*mat_len == *mat_buff_len) {
        status = double_buffer((void **) &materials, mat_buff_len,
                               sizeof(MATERIAL));
        if (status != 0) {
          free_line_buffer(lib_lines);
          printf("Unable to reallocate material buffer\n");
          return -1;
        }
      }
    } else if (op != NO_OP) {
      cur_mat->mat_paths[op] = malloc(strlen(lib_lines->dir) + strlen(cur_line)
                                      + 2);
      sprintf(cur_mat->mat_paths[op], "%s/%s", lib_lines->dir, cur_line);
    }
  }

  free_line_buffer(lib_lines);

  return 0;
}

PROP_TYPE get_op(char **cur_line) {
  int op_len = 0;

  while ((*cur_line)[op_len] != ' ' && (*cur_line)[op_len] != '\0') {
    op_len++;
  }

  if ((*cur_line)[op_len] == '\0') {
    return NO_OP;
  }
  (*cur_line)[op_len] = '\0';

  PROP_TYPE op = NO_OP;
  if (strncmp(*cur_line, "newmtl", op_len) == 0) {
    op = NEWMTL;
  } else if(strncmp(*cur_line, "map_Ka", op_len) == 0) {
    op = AMB;
  } else if(strncmp(*cur_line, "map_Kd", op_len) == 0) {
    op = DIFF;
  } else if(strncmp(*cur_line, "map_Ks", op_len) == 0) {
    op = SPEC;
  } else if(strncmp(*cur_line, "map_Ns", op_len) == 0) {
    op = SPEC_EXPONENT;
  } else if(strncmp(*cur_line, "map_bump", op_len) == 0) {
    op = BUMP;
  }
  (*cur_line) += op_len + 1;

  return op;
}

void free_materials(MATERIAL *buffer, size_t buff_len) {
  MATERIAL *materials = buffer;
  for (int i = 0; i < buff_len; i++) {
    for (int j = 0; j < NUM_PROPS; j++) {
      if (materials[i].mat_paths[j] != NULL) {
        free(materials[i].mat_paths[j]);
      }
    }
  }
  free(materials);
}

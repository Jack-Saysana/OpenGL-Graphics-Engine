#include <material.h>

int parse_mtllib(MATERIAL *materials, size_t *mat_buff_len, size_t *mat_len,
                 char *lib) {
  LINE_BUFFER *lib_lines = get_lines(lib);
  if (lib == NULL) {
    printf("Unable to get mtllib line buffer\n");
    return -1;
  }

  MATERIAL cur_mat = materials[mat_len];
  char *cur_line = NULL;
  int status = 0;
  for (int i = 0; i < lib_lines->len; i++) {
    cur_line = lib_lines->buffer[i];
    PROP_TYPE op = get_op(&cur_line);

    int line_len = strlen(cur_line);
    if (op == NEWMTL) {
      cur_mat = materials[mat_len];
      cur_mat.name = malloc(line_len + 1);
      strncpy(cur_mat.name, cur_line, line_len);
      cur_mat.name[line_len] = '\0';

      (*mat_len)++;
      if (*mat_len == *mat_buff_len) {
        status = double_complex_buffer(&materials, mat_buff_len, *mat_len,
                                       sizeof(MATERIAL), free_materials);
        if (status != 0) {
          printf("Unable to reallocate material buffer\n");
          return -1;
        }
      }
    } else if (op != NO_OP) {
      cur_mat.properties[cur_mat.num_props].type = op;
      if (op == AMB || op == DIFF || op == SPEC || op == SPEC_EXPONENT) {
        // Parse color
      } else {
        cur_mat.properties[cur_mat.num_props].data.path = malloc(line_len + 1);
        if (cur_mat.properties[cur_mat.num_props].data.path == NULL) {
          free_materials(materials, *mat_len);
          printf("Unable to allocate material property\n");
          return -1;
        }

        strncpy(cur_mat.properties[cur_mat.num_props].data.path, cur_line,
                line_len);
        cur_mat.properties[cur_mat.num_props].data.path[line_len] = '\0';
      }
    }
  }

  free_line_buffer(lb);
}

PROP_TYPE get_op(char **cur_line) {
  int op_len = 0;
  while ((*cur_line)[i] != ' ' || (*cur_line)[i] != '\0') {
    op_len++;
  }
  if ((*cur_line)[op_len] == '\0') {
    return no_op;
  }
  (*cur_line)[op_len] = '\0';

  PROP_TYPE op = NO_OP;
  if (strncmp(cur_line, "newmtl", op_len) == 0) {
    op = NEWMTL;
  } else if(strncmp(cur_line, "Ka", op_len) == 0) {
    op = AMB;
  } else if(strncmp(cur_line, "Kd", op_len) == 0) {
    op = DIFF;
  } else if(strncmp(cur_line, "Ks", op_len) == 0) {
    op = SPEC;
  } else if(strncmp(cur_line, "Ns", op_len) == 0) {
    op = SPEC_EXPONENT;
  } else if(strncmp(cur_line, "map_Ka", op_len) == 0) {
    op = MAP_AMB;
  } else if(strncmp(cur_line, "map_Kd", op_len) == 0) {
    op = MAP_DIFF;
  } else if(strncmp(cur_line, "map_Ks", op_len) == 0) {
    op = MAP_SPEC;
  } else if(strncmp(cur_line, "map_Ns", op_len) == 0) {
    op = MAP_SPEC_EXPONENT;
  } else if(strncmp(cur_line, "map_bump", op_len) == 0) {
    op = BUMP;
  }
  (*cur_line) += op_len + 1;

  return op;
}

void free_materials(void *buffer, size_t buf_size) {
  MATERIAL *mats = (MATERIAL *buffer);
  for (size_t i = 0; i < buf_size; i++) {
    free(mats[i].name);
    for (int j = 0; j < mats[i].num_props; j++) {
      free(mats[i].properties[j].data);
    }
  }
  free(buffer);
}

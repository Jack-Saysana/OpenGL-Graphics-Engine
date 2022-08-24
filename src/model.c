#include <model.h>

float (*verticies)[NUM_VERT_COORDS] = NULL;
size_t v_buff_len = VERTEX_BUFF_STARTING_LEN;
size_t v_len = 0;

float (*normals)[3] = NULL;
size_t n_buff_len = NORMAL_BUFF_STARTING_LEN;
size_t n_len = 0;

float (*tex_coords)[NUM_TEX_COORDS] = NULL;
size_t t_buff_len = TEX_COORD_BUFF_STARTING_LEN;
size_t t_len = 0;

int (*vbo_index_combos)[3] = NULL;
VBO *vertex_buffer = NULL;
size_t vbo_buff_len = VBO_STARTING_LEN;
size_t vbo_len = 0;

int (*indicies)[3] = NULL;
//size_t i_buff_len = INDEX_BUFF_STARTING_LEN;
//size_t i_len = 0;

int load_model(char *path) {
  LINE_BUFFER *line_buff = get_lines(path);
  if (line_buff == NULL) {
    printf("Unable to create line buffer\n");
    return -1;
  }
  line_buff->path = path;

  V_DATA *v_data = parse_lines(line_buff);
  if (v_data == NULL) {
    printf("Unable to parse line buffer\n");
    return -1;
  }

  return 0;
}

V_DATA *parse_lines(LINE_BUFFER *lb) {
  verticies = malloc(sizeof(float) * NUM_VERT_COORDS * V_BUFF_STARTING_LEN);
  if (verticies == NULL) {
    printf("Unable to allocate vertex buffer\n");
    return NULL;
  }

  normals = malloc(sizeof(float) * 3 * NORMAL_BUFF_STARTING_LEN);
  if (normals == NULL) {
    free(verticies);
    printf("Unable to allocate normal buffer\n");
    return NULL;
  }

  tex_coords = malloc(sizeof(float) * NUM_TEX_COORDS *
                      TEX_COORD_BUFF_STARTING_LEN);
  if (tex_coords == NULL) {
    free(verticies);
    free(normals);
    printf("Unable to allocate tex coord buffer\n");
    return NULL;
  }

  vbo_index_combos = malloc(sizeof(int) * 3 * VBO_STARTING_LEN);
  if (vbo_index_combos == NULL) {
    free(verticies);
    free(normals);
    free(tex_coords);
    printf("Unable to allocate vbo index combos\n");
    return NULL;
  }

  vertex_buffer = malloc(sizeof(VBO) * VBO_STARTING_LEN);
  if (vertex_buffer == NULL) {
    free(verticies);
    free(normals);
    free(tex_coords);
    free(vbo_index_combos);
    printf("Unable to allocate vbo\n");
    return NULL;
  }

  indicies = malloc(sizeof(int) * 3 * INDEX_BUFF_STARTING_LEN);
  if (indicies == NULL) {
    free(verticies);
    free(normals);
    free(tex_coords);
    free(vbo_index_combos);
    free(vertex_buffer);
    printf("Unable to allocate index data\n");
    return NULL;
  }

  char *cur_line = NULL;
  int status = 0;
  for (int i = 0; i < lb->len; i++) {
    cur_line = lb->buffer[i];
    //printf("%d. %s\n", i + 1, cur_line);
    /*if (strncmp("mtllib", cur_line, 6) == 0) {

    } else if (strncmp("usemtl", cur_line, 6) == 0) {

    } else*/
    if (cur_line[0] == 'v' && cur_line[1] == 't') {
      sscanf(cur_line, "vt %f %f",
            &(tex_coords[t_len][0]),
            &(tex_coords[t_len][1])
          );
      t_len++;
      if (t_len == t_buff_len) {
        status = double_buffer((void **) &tex_coords, &t_buff_len,
                               sizeof(float) * 2);
      }
    } else if (cur_line[0] == 'v' && cur_line[1] == 'n') {
      sscanf(cur_line, "vn %f %f %f",
            &(normals[n_len][0]),
            &(normals[n_len][1]),
            &(normals[n_len][2])
          );
      n_len++;
      if (n_len == n_buff_len) {
        status = double_buffer((void **) &normals, &n_buff_len,
                               sizeof(float) * 3);
      }
    } else if (cur_line[0] == 'v') {
      sscanf(cur_line, "v %f %f %f",
            &(verticies[v_len][0]),
            &(verticies[v_len][1]),
            &(verticies[v_len][2])
          );
      v_len++;
      if (v_len == v_buff_len) {
        status = double_buffer((void **) &verticies, &v_buff_len,
                               sizeof(float) * 3);
      }
    } else if (cur_line[0] == 'f') {
      status = parse_face(cur_line + 2);
    }

    if (status != 0) {
      free(verticies);
      free(normals);
      free(tex_coords);
      free(vbo_index_combos);
      free(vertex_buffer);
      free(indicies);
      printf("Parse error at line %d\n", i);
      return NULL;
    }
  }

  printf("VERTEX LIST:\n");
  for (int i = 0; i < v_len; i++) {
    printf("%d. (%f, %f, %f)\n", i, verticies[i][0], verticies[i][1],
           verticies[i][2]);
  }

  printf("TEX_COORD LIST:\n");
  for (int i = 0; i < t_len; i++) {
    printf("%d. (%f, %f)\n", i, tex_coords[i][0], tex_coords[i][1]);
  }

  printf("NORMAL LIST:\n");
  for (int i = 0; i < n_len; i++) {
    printf("%d. (%f, %f, %f)\n", i, normals[i][0], normals[i][1],
           normals[i][2]);
  }

  printf("INDEX COMBOS:\n");
  for (int i = 0; i < vbo_len; i++) {
    printf("[%d] ==========\n", i + 1);
    printf("%d/%d/%d\n", vbo_index_combos[i][0] + 1,
                         vbo_index_combos[i][1] + 1,
                         vbo_index_combos[i][2] + 1);
  }

  printf("VBO:\n");
  for (int i = 0; i < vbo_len; i++) {
    printf("[%d] ==========\n", i + 1);
    printf("  (%d)[%f, %f, %f]\n  (%d)[%f, %f]\n  (%d)[%f, %f, %f]\n",
           vbo_index_combos[i][0] + 1,
           vertex_buffer[i].vertex[0],
           vertex_buffer[i].vertex[1],
           vertex_buffer[i].vertex[2],
           vbo_index_combos[i][1] + 1,
           vertex_buffer[i].tex_coord[0],
           vertex_buffer[i].tex_coord[1],
           vbo_index_combos[i][2] + 1,
           vertex_buffer[i].normal[0],
           vertex_buffer[i].normal[1],
           vertex_buffer[i].normal[2]);
  }
  return NULL;
}

int parse_face(char *line) {
  int status = 0;
  //                      v   t   n
  int index_combo[3] = { -1, -1, -1 };
  int cur_attrib = 0;
  char *cur_num = line;
  int read_index = 0;
  int i = 0;
  while (line[i] != '\0') {
    if (line[i] == '/') {
      line[i] = '\0';
//      printf("%s\n", cur_num);
      read_index = atoi(cur_num) - 1;
      cur_num = line + i + 1;

      if (read_index >= 0 && cur_attrib == 0) {
        if (read_index > v_len - 1) {
          printf("Invalid vertex index\n");
          return -1;
        }
        index_combo[0] = read_index;
      } else if (cur_attrib == 1) {
        if (read_index > t_len - 1) {
          printf("Invalid tex coord index\n");
          return -1;
        }
        index_combo[1] = read_index;
      } else if(read_index >= 0) {
        printf("Invalid number of vertex attributes\n");
        return -1;
      }
      cur_attrib++;
    } else if (line[i] == ' ') {
      line[i] = '\0';
//      printf("%s\n\n", cur_num);
      read_index = atoi(cur_num) - 1;
      cur_num = line + i + 1;

      if (cur_attrib == 0) {
        if (read_index > v_len - 1) {
          printf("Invalid vertex index\n");
          return -1;
        }
        index_combo[0] = read_index;
      } else {
        if (read_index > n_len - 1) {
          printf("Invalid normal index\n");
          return -1;
        }
        index_combo[2] = read_index;
      }

      int found = -1;
      for (int i = 0; i < vbo_len && found == -1; i++) {
        if (vbo_index_combos[i][0] == index_combo[0] &&
            vbo_index_combos[i][1] == index_combo[1] &&
            vbo_index_combos[i][2] == index_combo[2]) {
          found = i;
        }
      }

      if (found == -1) {
        status = add_vertex_attrib(index_combo);
        if (status != 0) {
          return -1;
        }
        found = vbo_len;
      }

      cur_attrib = 0;
      index_combo[0] = -1;
      index_combo[1] = -1;
      index_combo[2] = -1;
    }
    i++;
  }
//  printf("%s\n\n", cur_num);
  read_index = atoi(cur_num) - 1;

  if (cur_attrib == 0) {
    if (read_index > v_len - 1) {
      printf("Invalid vertex index\n");
      return -1;
    }
    index_combo[0] = read_index;
  } else {
    if (read_index > n_len - 1) {
      printf("%d\n", read_index);
      printf("%ld\n", n_len - 1);
      printf("Invalid normal index\n");
      return -1;
    }
    index_combo[2] = read_index;
  }
  int found = -1;
  for (int i = 0; i < vbo_len && found == -1; i++) {
    if (vbo_index_combos[i][0] == index_combo[0] &&
        vbo_index_combos[i][1] == index_combo[1] &&
        vbo_index_combos[i][2] == index_combo[2]) {
      found = i;
    }
  }

  if (found == -1) {
    status = add_vertex_attrib(index_combo);
    if (status != 0) {
      return -1;
    }
    found = vbo_len;
  }

  return 0;
}

int add_vertex_attrib(int *index_combo) {
  vbo_index_combos[vbo_len][0] = index_combo[0];
  vbo_index_combos[vbo_len][1] = index_combo[1];
  vbo_index_combos[vbo_len][2] = index_combo[2];

  if (index_combo[0] == -1) {
    printf("No vertex data found\n");
    return -1;
  } else {
    vertex_buffer[vbo_len].vertex[0] = verticies[index_combo[0]][0];
    vertex_buffer[vbo_len].vertex[1] = verticies[index_combo[0]][1];
    vertex_buffer[vbo_len].vertex[2] = verticies[index_combo[0]][2];
  }

  if (index_combo[1] == -1) {
    vertex_buffer[vbo_len].tex_coord[0] = 0;
    vertex_buffer[vbo_len].tex_coord[1] = 0;
  } else {
    vertex_buffer[vbo_len].tex_coord[0] = tex_coords[index_combo[1]][0];
    vertex_buffer[vbo_len].tex_coord[1] = tex_coords[index_combo[1]][1];
  }

  if (index_combo[2] == -1) {
    vertex_buffer[vbo_len].normal[0] = 0;
    vertex_buffer[vbo_len].normal[1] = 0;
    vertex_buffer[vbo_len].normal[2] = 0;
  } else {
    vertex_buffer[vbo_len].normal[0] = normals[index_combo[2]][0];
    vertex_buffer[vbo_len].normal[1] = normals[index_combo[2]][1];
    vertex_buffer[vbo_len].normal[2] = normals[index_combo[2]][2];
  }

  vbo_len++;
  if (vbo_len == vbo_buff_len) {
    int status = 0;

    status = double_buffer((void **) &vertex_buffer, &vbo_buff_len,
                           sizeof(VBO));
    vbo_index_combos = realloc(vbo_index_combos, vbo_buff_len *
                               sizeof(int) * 3);
    if (vbo_index_combos == NULL || status != 0) {
      printf("Unable to reallocate vbo index combos\n");
      return -1;
    }
  }

  return 0;
}

/*
 * ========== DOUBLE_BUFFER() ==========
 * DESC
 *
 * ARGS
 * - void** buffer: Pointer to the buffer being doubled
 * - size_t *buff_size: Pointer to the number recording the siZe
 * of the buffer
 *
 * RETURNS
 * 0 if successful
 * -1 if an error has occured
 * =====================================
 */
int double_buffer(void **buffer, size_t *buff_size, size_t unit_size) {
  *buffer = realloc(*buffer, 2 * *buff_size * unit_size);
  if (*buffer == NULL) {
    return -1;
  }
  *buff_size = 2 * *buff_size;
  return 0;
}

/*
 * ========== GET_LINES() ==========
 * DESC: Retrieves an array of the lines of a
 * file
 *
 * ARGS:
 * char *path: Path to file being read
 *
 * RETURNS:
 * A pointer to a LINE_BUFFER struct which holds the individual lines
 * of the file and the number of lines
 *
 * IMPORTANT:
 * The returned LINE_BUFFER * must be freed using
 * free_line_buffer()
 * =================================
 */
LINE_BUFFER *get_lines(char *path) {
  FILE *file = fopen(path, "r");
  if (file == NULL) {
    printf("Unable to open object file\n");
    return NULL;
  }
  fseek(file, 0, SEEK_END);
  size_t file_len = ftell(file) + 1;
  fseek(file, 0, SEEK_SET);

  char *file_contents = malloc(file_len);
  if (file_contents == NULL) {
    printf("Unable to allocate file buffer\n");
    return NULL;
  }
  fread(file_contents, file_len, 1, file);
  fclose(file);
  file_contents[file_len - 1] = '\n';

  LINE_BUFFER *lb = malloc(sizeof(lb));
  if (lb == NULL) {
    free(file_contents);
    printf("Unable to allocate line buffer struct\n");
  }

  lb->buffer = malloc(sizeof(char *) * LINE_BUFF_STARTING_LEN);
  if (lb->buffer == NULL) {
    free(lb);
    free(file_contents);
    printf("Unable to allocate line buffer\n");
    return NULL;
  }
  size_t line_buffer_max = sizeof(char *) * LINE_BUFF_STARTING_LEN;
  lb->len = 0;

  char *cur_line = file_contents;
  for (int i = 0; i < file_len; i++) {
    if (file_contents[i] == '\n') {
      file_contents[i] = '\0';
      lb->buffer[lb->len] = cur_line;
      lb->len++;
      if (lb->len == line_buffer_max) {
        lb->buffer = realloc(lb->buffer, line_buffer_max * 2);
        if (lb->buffer == NULL) {
          free(file_contents);
          free(lb);
          printf("Unable to reallocate line buffer\n");
          return NULL;
        }

        line_buffer_max *= 2;
      }

      cur_line = file_contents + i + 1;
    }
  }

  return lb;
}

/*
 * ========== FREE_LINE_BUFFER ==========
 *
 * Deallocates a given line buffer
 *
 * ======================================
 */
void free_line_buffer(LINE_BUFFER *lb) {
  free(lb->buffer[0]);
  free(lb->buffer);
  free(lb);
}

#include <model_loader.h>

float (*verticies)[3] = NULL;
size_t v_buff_len = 0;
size_t v_len = 0;

float (*normals)[3] = NULL;
size_t n_buff_len = 0;
size_t n_len = 0;

float (*tex_coords)[2] = NULL;
size_t t_buff_len = 0;
size_t t_len = 0;

int (*vbo_index_combos)[3] = NULL;
VBO *vertex_buffer = NULL;
size_t vbo_buff_len = 0;
size_t vbo_len = 0;

int (*indicies)[3] = NULL;
size_t i_buff_len = 0;
size_t i_len = 0;

MODEL *load_model(char *path) {
  LINE_BUFFER *line_buff = get_lines(path);
  if (line_buff == NULL) {
    printf("Unable to create line buffer\n");
    return NULL;
  }
  line_buff->path = path;

  if (strncmp("#PRE", line_buff->buffer[0], 4) != 0) {
    preprocess_lines(line_buff);

    line_buff = get_lines(path);
    if (line_buff == NULL) {
      printf("Unable to create line buffer\n");
      return NULL;
    }
    line_buff->path = path;
  }

  int status = parse_lines(line_buff);
  if (status == -1) {
    printf("Unable to parse line buffer\n");
    return NULL;
  }

  unsigned int VAO;
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  unsigned int VBO;
  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(VBO) * vbo_len, vertex_buffer,
               GL_STATIC_DRAW);

  unsigned int EBO;
  glGenBuffers(1, &EBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * 3 * i_len, indicies,
               GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (void *) 0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (void *) (sizeof(float) * 3));
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (void *) (sizeof(float) * 6));

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);
  glBindVertexArray(0);

  MODEL *model = malloc(sizeof(MODEL));
  model->VAO = VAO;
  model->num_verts = vbo_len;

  free(vertex_buffer);
  free(indicies);

  return model;
}

int parse_lines(LINE_BUFFER *lb) {
  verticies = malloc(sizeof(float) * 3 * VERTEX_BUFF_STARTING_LEN);
  if (verticies == NULL) {
    printf("Unable to allocate vertex buffer\n");
    return -1;
  }
  v_buff_len = VERTEX_BUFF_STARTING_LEN;
  v_len = 0;

  normals = malloc(sizeof(float) * 3 * NORMAL_BUFF_STARTING_LEN);
  if (normals == NULL) {
    free(verticies);
    printf("Unable to allocate normal buffer\n");
    return -1;
  }
  n_buff_len = NORMAL_BUFF_STARTING_LEN;
  n_len = 0;

  tex_coords = malloc(sizeof(float) * 2 *
                      TEX_COORD_BUFF_STARTING_LEN);
  if (tex_coords == NULL) {
    free(verticies);
    free(normals);
    printf("Unable to allocate tex coord buffer\n");
    return -1;
  }
  t_buff_len = TEX_COORD_BUFF_STARTING_LEN;
  t_len = 0;

  vbo_index_combos = malloc(sizeof(int) * 3 * VBO_STARTING_LEN);
  if (vbo_index_combos == NULL) {
    free(verticies);
    free(normals);
    free(tex_coords);
    printf("Unable to allocate vbo index combos\n");
    return -1;
  }

  vertex_buffer = malloc(sizeof(VBO) * VBO_STARTING_LEN);
  if (vertex_buffer == NULL) {
    free(verticies);
    free(normals);
    free(tex_coords);
    free(vbo_index_combos);
    printf("Unable to allocate vbo\n");
    return -1;
  }
  vbo_buff_len = VBO_STARTING_LEN;
  vbo_len = 0;

  indicies = malloc(sizeof(int) * 3 * INDEX_BUFF_STARTING_LEN);
  if (indicies == NULL) {
    free(verticies);
    free(normals);
    free(tex_coords);
    free(vbo_index_combos);
    free(vertex_buffer);
    printf("Unable to allocate index data\n");
    return -1;
  }
  i_buff_len = INDEX_BUFF_STARTING_LEN;
  i_len = 0;

  char *cur_line = NULL;
  int status = 0;
  for (int i = 0; i < lb->len; i++) {
    cur_line = lb->buffer[i];
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
      return -1;
    }
  }

  /*for (int i = 0; i < vbo_len; i++) {
    printf("[%d]==========\n", i + 1);
    printf("[ %f %f %f ]\n[ %f %f %f ]\n[ %f %f ]\n",
           vertex_buffer[i].vertex[0], vertex_buffer[i].vertex[1],
           vertex_buffer[i].vertex[2], vertex_buffer[i].normal[0],
           vertex_buffer[i].normal[1], vertex_buffer[i].normal[2],
           vertex_buffer[i].tex_coord[0], vertex_buffer[i].tex_coord[1]);
  }

  printf("\nEBO:\n");
  for (int i = 0; i < i_len; i++) {
    printf("[ %d %d %d ]\n", indicies[i][0], indicies[i][1], indicies[i][2]);
  }*/

  free_line_buffer(lb);
  free(verticies);
  free(normals);
  free(tex_coords);
  free(vbo_index_combos);
  return 0;
}

/*
 * ========== PARSE_FACE() ==========
 *
 * DESC:
 * Parses an obj files face command by populating
 * the vertex buffer with the appropriate vertex
 * attributes and the index buffer with the
 * appropriate triangle indicies
 *
 * ARGS:
 * char *line: The line of the face command
 *
 * RETURNS:
 * 0 if successful
 * -1 if unsuccessful
 *
 * ==================================
 */
int parse_face(char *line) {
  int status = 0;
  int *face = indicies[i_len];
  int num_verts = 0;
  //                      v   t   n
  int index_combo[3] = { -1, -1, -1 };
  int cur_attrib = 0;
  char *cur_num = line;
  int read_index = 0;

  int line_len = strlen(line) + 1;
  for (int i = 0; i < line_len; i++) {
    if (line[i] == '/') {
      line[i] = '\0';
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
    } else if (line[i] == ' ' || line[i] == '\0') {
      line[i] = '\0';
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
          found = 1;
          if (num_verts < 3) {
            face[num_verts] = i;
            num_verts++;
          } else {
            printf("Too many verticies specified\n");
            return -1;
          }
        }
      }

      if (found == -1) {
        status = add_vertex_attrib(index_combo);
        if (status != 0) {
          return -1;
        }
        if (num_verts < 3) {
          face[num_verts] = vbo_len - 1;
          num_verts++;
        } else {
          printf("Too many verticies specified\n");
          return -1;
        }
      }

      cur_attrib = 0;
      index_combo[0] = -1;
      index_combo[1] = -1;
      index_combo[2] = -1;
    }
  }

  if (num_verts < 3) {
    printf("Too few verticies specified\n");
    return -1;
  } else {
    i_len++;

    if (i_len == i_buff_len) {
      status = double_buffer((void **) &indicies, &i_buff_len,
                             sizeof(int) * 3);
      if (status == -1) {
        printf("Unable to reallocate index buffer\n");
        return -1;
      }
    }
  }

  return 0;
}

/*
 * ========== ADD_VERTEX_ATTRIB ==========
 *
 * DESC:
 * Populate the vertex buffer array with a given
 * group of vertex attributes
 *
 * ARGS:
 * int *index_combo: Representing the structure of
 * the vertex attribute to be added to the buffer.
 *  -Structure:
 *    = Array must be 3 integers long
 *    - First element represents the index of the
 *      vertex array element which corresponds to
 *      the vertex attribute's geometric location
 *    = Second element represents the index of the
 *      tex_coord element which cooresponds to the
 *      vertex attributes texture coordinate
 *    - Third element represents the index of the
 *      normal element which cooresponds to the
 *      vertex attributes normal vector
 *
 * RETURNS:
 * 0 if successful
 * -1 if an error occurs
 * =======================================
 */
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
 * Doubles the given buffer
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
  *buffer = realloc(*buffer, 2 * (*buff_size) * unit_size);
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
  int status = 0;;
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

  LINE_BUFFER *lb = malloc(sizeof(LINE_BUFFER));
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
  size_t line_buffer_max = LINE_BUFF_STARTING_LEN;
  lb->len = 0;

  char *cur_line = file_contents;
  for (int i = 0; i < file_len; i++) {
    if (file_contents[i] == '\n') {
      file_contents[i] = '\0';
      //printf("%s\n", cur_line);
      lb->buffer[lb->len] = cur_line;
      lb->len++;
      if (lb->len == line_buffer_max) {
        status = double_buffer((void **) &(lb->buffer), &line_buffer_max,
                               sizeof(char *));
        if (status != 0) {
          free(file_contents);
          free(lb);
          printf("Unable to reallocate line buffer\n");
          return NULL;
        }
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

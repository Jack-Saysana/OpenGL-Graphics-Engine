#include <model_loader.h>

/*float (*verticies)[3] = NULL;
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
size_t i_len = 0;*/

MODEL *load_model(char *path) {
  char *bin_path = malloc(strlen(path) + 5);
  sprintf(bin_path, "%s.bin", path);
  FILE *file = fopen(bin_path, "rb");

  /*LINE_BUFFER *line_buff = get_lines(path);
  if (line_buff == NULL) {
    printf("Unable to create line buffer\n");
    return NULL;
  }
  line_buff->path = path;*/

  //if (strncmp("#PRE", line_buff->buffer[0], 4) != 0) {
  if (file == NULL) {
    LINE_BUFFER *line_buff = get_lines(path);
    if (line_buff == NULL) {
      printf("Unable to create line buffer\n");
      return NULL;
    }
    line_buff->path = path;

    preprocess_lines(line_buff);

    file = fopen(bin_path, "rb");
    if (file == NULL) {
      printf("Unable to open preproccessed file\n");
      return NULL;
    }
    /*line_buff = get_lines(path);
    if (line_buff == NULL) {
      printf("Unable to create line buffer\n");
      return NULL;
    }
    line_buff->path = path;*/
  }

  size_t v_len;
  size_t i_len;
  fread(&v_len, sizeof(size_t), 1, file);
  fread(&i_len, sizeof(size_t), 1, file);

  //printf("VBOS: %lld\nFaces: %lld\n", v_combo_len, f_len);

  float *verticies = malloc(sizeof(float) * 8 * v_len);
  int *indicies = malloc(sizeof(int) * 3 * i_len);
  fread(verticies, sizeof(float) * 8, v_len, file);
  fread(indicies, sizeof(int) * 3, i_len, file);
  fclose(file);

  /*int status = parse_lines(line_buff);
  if (status == -1) {
    printf("Unable to parse line buffer\n");
    return NULL;
  }*/

  /*printf("\nVERTEX BUFFER: \n");
  float *test = verts;
  for (int i = 0; i < v_combo_len * 8; i++) {
    if (i % 8 == 7) {
      printf("%f,\n", test[i]);
    } else {
      printf("%f, ", test[i]);
    }
  }

  int *test_i = inds;
  for (int i = 0; i < f_len * 3; i++) {
    if (i % 3 == 2) {
      printf("%d,\n", test_i[i]);
    } else {
      printf("%d, ", test_i[i]);
    }
  }
  fflush(stdout);*/

  unsigned int VAO_id;
  glGenVertexArrays(1, &VAO_id);
  glBindVertexArray(VAO_id);

  unsigned int VBO_id;
  glGenBuffers(1, &VBO_id);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_id);
  //glBufferData(GL_ARRAY_BUFFER, sizeof(VBO) * vbo_len, (float *) vertex_buffer,
  //             GL_STATIC_DRAW);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8 * v_len, verticies, GL_STATIC_DRAW);

  unsigned int EBO_id;
  glGenBuffers(1, &EBO_id);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_id);
  //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * 3 * i_len, (int *) indicies,
  //            GL_STATIC_DRAW);
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
  model->VAO = VAO_id;
  model->num_indicies = i_len * 3;

  //free(vertex_buffer);
  free(verticies);
  free(indicies);

  return model;
}

/*int parse_lines(LINE_BUFFER *lb) {
  verticies = malloc(sizeof(float) * 3 * VERTEX_BUFF_STARTING_LEN);
  if (verticies == NULL) {
    free_line_buffer(lb);
    printf("Unable to allocate vertex buffer\n");
    return -1;
  }
  v_buff_len = VERTEX_BUFF_STARTING_LEN;
  v_len = 0;

  normals = malloc(sizeof(float) * 3 * NORMAL_BUFF_STARTING_LEN);
  if (normals == NULL) {
    free_line_buffer(lb);
    free(verticies);
    printf("Unable to allocate normal buffer\n");
    return -1;
  }
  n_buff_len = NORMAL_BUFF_STARTING_LEN;
  n_len = 0;

  tex_coords = malloc(sizeof(float) * 2 *
                      TEX_COORD_BUFF_STARTING_LEN);
  if (tex_coords == NULL) {
    free_line_buffer(lb);
    free(verticies);
    free(normals);
    printf("Unable to allocate tex coord buffer\n");
    return -1;
  }
  t_buff_len = TEX_COORD_BUFF_STARTING_LEN;
  t_len = 0;

  vbo_index_combos = malloc(sizeof(int) * 3 * VBO_STARTING_LEN);
  if (vbo_index_combos == NULL) {
    free_line_buffer(lb);
    free(verticies);
    free(normals);
    free(tex_coords);
    printf("Unable to allocate vbo index combos\n");
    return -1;
  }

  vertex_buffer = malloc(sizeof(VBO) * VBO_STARTING_LEN);
  if (vertex_buffer == NULL) {
    free_line_buffer(lb);
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
    free_line_buffer(lb);
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
    //if (strncmp("mtllib", cur_line, 6) == 0) {
      // Parse lib
    //} else if (strncmp("usemtl", cur_line, 6) == 0) {

    //} else
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

      //printf("%lld [ %f %f %f ]\n", v_len, verticies[v_len][0],
      //       verticies[v_len][1], verticies[v_len][2]);

      v_len++;
      if (v_len == v_buff_len) {
        status = double_buffer((void **) &verticies, &v_buff_len,
                               sizeof(float) * 3);
      }
    } else if (cur_line[0] == 'f') {
      //printf("%s\n", cur_line);
      //fflush(stdout);
      status = parse_face(cur_line + 2);
    }

    if (status != 0) {
      free_line_buffer(lb);
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

  free_line_buffer(lb);
  free(verticies);
  free(normals);
  free(tex_coords);
  free(vbo_index_combos);
  return 0;
}*/

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
/*int parse_face(char *line) {
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
      if (cur_attrib > 1) {
        printf("Invalid number of vertex attributes\n");
        return -1;
      }

      line[i] = '\0';
      read_index = atoi(cur_num) - 1;
      cur_num = line + i + 1;

      if (cur_attrib == 0 && (read_index >= v_len || read_index < 0)) {
          printf("Invalid vertex index\n");
          return -1;
      } else if (cur_attrib == 1 && (read_index >= t_len || read_index < -1)) {
          printf("Invalid tex coord index\n");
          return -1;
      }
      index_combo[cur_attrib] = read_index;

      cur_attrib++;
    } else if (line[i] == ' ' || line[i] == '\0') {
      line[i] = '\0';
      read_index = atoi(cur_num) - 1;
      cur_num = line + i + 1;

      if (cur_attrib == 0 && (read_index >= v_len || read_index < 0)) {
        printf("Invalid vertex index\n");
        return -1;
      } else if (cur_attrib == 1 && (read_index >= t_len || read_index < 0)) {
        printf("Invalid tex coord index\n");
        return -1;
      } else if (cur_attrib == 2 && (read_index >= n_len || read_index < 0)) {
        printf("Invalid normal index\n");
        return -1;
      }
      index_combo[cur_attrib] = read_index;

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
  }

  i_len++;
  if (i_len == i_buff_len) {
    status = double_buffer((void **) &indicies, &i_buff_len,
                           sizeof(int) * 3);
    if (status == -1) {
      printf("Unable to reallocate index buffer\n");
      return -1;
    }
  }

  return 0;
}*/

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
/*int add_vertex_attrib(int *index_combo) {
  vbo_index_combos[vbo_len][0] = index_combo[0];
  vbo_index_combos[vbo_len][1] = index_combo[1];
  vbo_index_combos[vbo_len][2] = index_combo[2];

  if (index_combo[0] < 0 || index_combo[0] >= v_len) {
    printf("No vertex data found\n");
    return -1;
  }

  //printf("\n========== VERTICIES ==========:\n");
  //for (int i = 0; i < v_len; i++) {
  //  printf("%d: [ %f %f %f ]\n", i, verticies[i][0], verticies[i][1], verticies[i][2]);
  //}
  //printf("\nIndex combo: [ %d %d %d ]\n", index_combo[0], index_combo[1], index_combo[2]);

  vertex_buffer[vbo_len].vertex[0] = verticies[index_combo[0]][0];
  vertex_buffer[vbo_len].vertex[1] = verticies[index_combo[0]][1];
  vertex_buffer[vbo_len].vertex[2] = verticies[index_combo[0]][2];

  //printf("Vertex: [ %f %f %f ]\n", vertex_buffer[vbo_len].vertex[0],
  //       vertex_buffer[vbo_len].vertex[1], vertex_buffer[vbo_len].vertex[2]);

  if (index_combo[1] < 0 || index_combo[1] >= t_len) {
    vertex_buffer[vbo_len].tex_coord[0] = 0;
    vertex_buffer[vbo_len].tex_coord[1] = 0;
  } else {
    vertex_buffer[vbo_len].tex_coord[0] = tex_coords[index_combo[1]][0];
    vertex_buffer[vbo_len].tex_coord[1] = tex_coords[index_combo[1]][1];
  }

  if (index_combo[2] < 0 || index_combo[2] >= n_len) {
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
}*/

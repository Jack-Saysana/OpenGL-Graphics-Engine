#include <obj_preprocessor.h>

int preprocess_lines(LINE_BUFFER *lb) {
  char *bin_path = malloc(strlen(lb->dir) + strlen(lb->filename) + 6);
  sprintf(bin_path, "%s/%s.bin", lb->dir, lb->filename);

  printf("%s, %s\n", lb->dir, lb->filename);
  fflush(stdout);

  FILE *file = fopen(bin_path, "wb");
  if (file == NULL) {
    printf("Unable to open preprocessed file\n");
    free_line_buffer(lb);
    return -1;
  }

  verticies = malloc(sizeof(float) * 3 * VERTEX_BUFF_STARTING_LEN);
  if (verticies == NULL) {
    free_line_buffer(lb);
    fclose(file);
    printf("Unable to allocate vertex buffer\n");
    return -1;
  }
  v_buff_len = VERTEX_BUFF_STARTING_LEN;
  v_len = 0;

  normals = malloc(sizeof(float) * 3 * NORMAL_BUFF_STARTING_LEN);
  if (normals == NULL) {
    free_line_buffer(lb);
    fclose(file);
    free(verticies);
    printf("Unable to allocate normal buffer\n");
    return -1;
  }
  n_buff_len = NORMAL_BUFF_STARTING_LEN;
  n_len = 0;

  tex_coords = malloc(sizeof(float) * 2 * TEX_COORD_BUFF_STARTING_LEN);
  if (tex_coords == NULL) {
    free_line_buffer(lb);
    fclose(file);
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
    fclose(file);
    free(verticies);
    free(normals);
    free(tex_coords);
    printf("Unable to allocate vbo index combos\n");
    return -1;
  }
  vbo_buff_len = VBO_STARTING_LEN;
  vbo_len = 0;

  faces = malloc(sizeof(int) * 3 * FACE_BUFF_STARTING_LEN);
  if (faces == NULL) {
    free_line_buffer(lb);
    fclose(file);
    free(verticies);
    free(normals);
    free(tex_coords);
    free(vbo_index_combos);
    printf("Unable to allocate face buffer\n");
    return -1;
  }
  face_buff_len = FACE_BUFF_STARTING_LEN;
  f_len = 0;

  materials = malloc(sizeof(MATERIAL) * MATERIAL_BUFF_STARTING_LEN);
  if (materials == NULL) {
    free_line_buffer(lb);
    fclose(file);
    free(verticies);
    free(normals);
    free(tex_coords);
    free(vbo_index_combos);
    free(faces);
    printf("Unable to allocate material buffer\n");
    return -1;
  }
  mat_buff_len = MATERIAL_BUFF_STARTING_LEN;
  mat_len = 0;

  MATERIAL *cur_mat = NULL;
  char *cur_line = NULL;
  int status = 0;
  for (int i = 0; i < lb->len; i++) {
    cur_line = lb->buffer[i];

    if (cur_line[0] == 'f') {
      status = preprocess_face(file, cur_line + 2);
    } else if (cur_line[0] == 'v' && cur_line[1] == 't' && cur_line[2] == ' ') {
      sscanf(cur_line, "vt %f %f",
            &(tex_coords[t_len][0]),
            &(tex_coords[t_len][1])
          );
      t_len++;
      if (t_len == t_buff_len) {
        status = double_buffer((void **) &tex_coords, &t_buff_len,
                               sizeof(float) * 2);
      }
    } else if (cur_line[0] == 'v' && cur_line[1] == 'n' && cur_line[2] == ' ') {
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
    } else if (cur_line[0] == 'v' && cur_line[1] == ' ') {
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
    } else if (cur_line[0] == 'm' && cur_line[1] == 't' && cur_line[2] == 'l'
               && cur_line[3] == 'l' && cur_line[4] == 'i' && cur_line[5] =='b'
               && cur_line[6] == ' ') {
      status = parse_mtllib(materials, &mat_buff_len, &mat_len, lb->dir,
                            cur_line + 7);
    } else if (cur_line[0] == 'u' && cur_line[1] == 's' && cur_line[2] == 'e'
               && cur_line[3] == 'm' && cur_line[4] == 't' &&
               cur_line[5] == 'l' && cur_line[6] == ' ') {
      cur_mat = NULL;
      uint64_t hash = get_hash(cur_line + 7);
      for (int i = 0; i < mat_len; i++) {
        if (materials[i].name == hash) {
          cur_mat = materials + i;
        }
      }
    }

    if (status != 0) {
      free_line_buffer(lb);
      fclose(file);
      free(verticies);
      free(normals);
      free(tex_coords);
      free(vbo_index_combos);
      free_materials(materials, mat_len);
      printf("Parse error at line %d\n", i);
      return -1;
    }
  }

  int material_flag = cur_mat != NULL;
  int cur_path_len = 0;

  fwrite(&vbo_len, sizeof(size_t), 1, file);
  fwrite(&f_len, sizeof(size_t), 1, file);
  fwrite(&material_flag, sizeof(material_flag), 1, file);
  if (material_flag) {
    for (int i = 0; i < NUM_PROPS; i++) {
      if (cur_mat->mat_paths[i] == NULL) {
        cur_path_len = 0;
      } else {
        cur_path_len = strlen(cur_mat->mat_paths[i]) + 1;
      }
      fwrite(&cur_path_len, sizeof(int), 1, file);
      if (cur_path_len > 0) {
        fwrite(cur_mat->mat_paths[i], sizeof(char), cur_path_len, file);
      }
    }
  }
  for (size_t i = 0; i < vbo_len; i++) {
    fwrite(verticies[vbo_index_combos[i][0]], sizeof(float), 3, file);
    fwrite(normals[vbo_index_combos[i][2]], sizeof(float), 3, file);
    fwrite(tex_coords[vbo_index_combos[i][1]], sizeof(float), 2, file);
  }
  fwrite(faces, sizeof(int) * 3, f_len, file);

  fclose(file);
  free_line_buffer(lb);
  free(verticies);
  free(normals);
  free(tex_coords);
  free(vbo_index_combos);
  free(faces);
  free_materials(materials, mat_len);

  return 0;
}

int preprocess_face(FILE *file, char *line) {
  FACE_VERT *face_list_head = NULL;
  FACE_VERT *face_list_tail = NULL;
  size_t num_verts = 0;

  int status = 0;
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
          printf("Preprocessor Error: Invalid vertex index\n");
          return -1;
        }
        index_combo[0] = read_index;
      } else if (cur_attrib == 1) {
        if (read_index > t_len - 1) {
          printf("Preprocessor Error: Invalid tex coord index\n");
          return -1;
        }
        index_combo[1] = read_index;
      } else if(read_index >= 0) {
        printf("Preprocessor Error: Invalid number of vertex attributes\n");
        return -1;
      }
      cur_attrib++;
    } else if (line[i] == ' ' || line[i] == '\0') {
      line[i] = '\0';
      read_index = atoi(cur_num) - 1;
      cur_num = line + i + 1;

      if (cur_attrib == 0) {
        if (read_index > v_len - 1) {
          printf("Preprocessor Error: Invalid vertex index\n");
          return -1;
        }
        index_combo[0] = read_index;
      } else {
        if (read_index > n_len - 1) {
          printf("Preprocessor Error:Invalid normal index\n");
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
        vbo_index_combos[vbo_len][0] = index_combo[0];
        vbo_index_combos[vbo_len][1] = index_combo[1];
        vbo_index_combos[vbo_len][2] = index_combo[2];
        found = vbo_len;
        vbo_len++;

        if (vbo_len == vbo_buff_len) {
          status = double_buffer((void **) &vbo_index_combos, &vbo_buff_len,
                                 sizeof(int) * 3);
        }

        if (status != 0) {
          printf("Preproccessor Error: Unable to realloc vbo indicies\n");
          return -1;
        }
      }

      if (face_list_head == NULL) {
        face_list_head = malloc(sizeof(FACE_VERT));
        face_list_head->index = found;
        face_list_head->prev = NULL;
        face_list_head->next = NULL;
        face_list_tail = face_list_head;
      } else {
        face_list_tail->next = malloc(sizeof(FACE_VERT));
        face_list_tail->next->prev = face_list_tail;
        face_list_tail = face_list_tail->next;

        face_list_tail->next = NULL;
        face_list_tail->index = found;
      }
      num_verts++;

      cur_attrib = 0;
      index_combo[0] = -1;
      index_combo[1] = -1;
      index_combo[2] = -1;
    }
  }
  face_list_tail->next = face_list_head;
  face_list_head->prev = face_list_tail;

  if (num_verts == 3) {
    faces[f_len][0] = face_list_head->prev->index;
    faces[f_len][1] = face_list_head->index;
    faces[f_len][2] = face_list_head->next->index;
    f_len++;
    if (f_len == face_buff_len) {
      status = double_buffer((void **) &faces, &face_buff_len, sizeof(int) * 3);
      if (status != 0) {
        printf("Unable to reallocate face buffer\n");
        return -1;
      }
    }

    face_list_tail->next = NULL;
    face_list_head->prev = NULL;
    while (face_list_head->next != NULL) {
      face_list_head = face_list_head->next;
      free(face_list_head->prev);
    }
    free(face_list_head);
  } else {
    status = triangulate_polygon(file, face_list_head, num_verts);
  }

  return status;
}

int triangulate_polygon(FILE *file, FACE_VERT *head, size_t num_verts) {
  float poly_normal[3] = {
    normals[vbo_index_combos[head->index][2]][0],
    normals[vbo_index_combos[head->index][2]][1],
    normals[vbo_index_combos[head->index][2]][2]
  };

  int verts_left = num_verts;
  int cur_triangle[3] = { -1, -1, -1 };

  int status = 0;

  FACE_VERT *cur_vert = head;
  FACE_VERT *temp = NULL;
  while (verts_left > 3) {
    cur_triangle[0] = cur_vert->next->index;
    cur_triangle[1] = cur_vert->index;
    cur_triangle[2] = cur_vert->prev->index;

    if (is_ear(cur_triangle, cur_vert, poly_normal) == 0) {
      faces[f_len][0] = cur_triangle[0];
      faces[f_len][1] = cur_triangle[1];
      faces[f_len][2] = cur_triangle[2];
      f_len++;
      if (f_len == face_buff_len) {
        status = double_buffer((void **) &faces, &face_buff_len, sizeof(int) * 3);
        if (status != 0) {
          printf("Unable to reallocate face buffer\n");
          return -1;
        }
      }

      cur_vert->prev->next = cur_vert->next;
      cur_vert->next->prev = cur_vert->prev;
      temp = cur_vert;
      cur_vert = cur_vert->next;
      free(temp);
      verts_left--;
    } else {
      cur_vert = cur_vert->next;
    }
  }

  faces[f_len][0] = cur_vert->next->index;
  faces[f_len][1] = cur_vert->index;
  faces[f_len][2] = cur_vert->prev->index;
  f_len++;
  if (f_len == face_buff_len) {
    status = double_buffer((void **) &faces, &face_buff_len, sizeof(int) * 3);
    if (status != 0) {
      printf("Unable to reallocate face buffer\n");
      return -1;
    }
  }

  free(cur_vert->prev);
  free(cur_vert->next);
  free(cur_vert);

  return 0;
}

int is_ear(int *triangle, FACE_VERT *ref_vert, float *polygon_normal) {
  // Triangle[0]: Vertex A
  // Triangle[1]: Focus of triangle
  // Triangle[2]: Vertex B

  float origin[3] = { 0.0, 0.0, 0.0 };
  // Translate Vertex A as if focus was at the origin
  float first_vert[3] = {
    verticies[vbo_index_combos[triangle[0]][0]][0]-verticies[vbo_index_combos[triangle[1]][0]][0],
    verticies[vbo_index_combos[triangle[0]][0]][1]-verticies[vbo_index_combos[triangle[1]][0]][1],
    verticies[vbo_index_combos[triangle[0]][0]][2]-verticies[vbo_index_combos[triangle[1]][0]][2]
  };
  // Translate Vertex B as if focus was at origin
  float second_vert[3] = {
    verticies[vbo_index_combos[triangle[2]][0]][0]-verticies[vbo_index_combos[triangle[1]][0]][0],
    verticies[vbo_index_combos[triangle[2]][0]][1]-verticies[vbo_index_combos[triangle[1]][0]][1],
    verticies[vbo_index_combos[triangle[2]][0]][2]-verticies[vbo_index_combos[triangle[1]][0]][2]
  };

  float *coords[3] = {
    origin,
    first_vert,
    second_vert
  };

  float u[3] = {
    coords[1][0] - coords[0][0],
    coords[1][1] - coords[0][1],
    coords[1][2] - coords[0][2]
  };

  float v[3] = {
    coords[2][0] - coords[0][0],
    coords[2][1] - coords[0][1],
    coords[2][2] - coords[0][2]
  };

  float u_cross_v[3] = {
    (u[1] * v[2]) - (u[2] * v[1]),
    (u[2] * v[0]) - (u[0] * v[2]),
    (u[0] * v[1]) - (u[1] * v[0])
  };

  /*float mult = 0;
  if (u_cross_v[0] != 0) {
    mult = polygon_normal[0] / u_cross_v[0];
  } else if (u_cross_v[1] != 0) {
    mult = polygon_normal[1] / u_cross_v[1];
  } else if (u_cross_v[2] != 0) {
    mult = polygon_normal[2] / u_cross_v[2];
  } else {
    // U x V IS (0, 0, 0) NEED SPECIAL CONSIDERATION
    return -1;
  }

  for (int i = 0; i < 3; i++) {
    if (u_cross_v[i] * mult != polygon_normal[i]) {
      return -1;
    }
  }*/

  float a = 0.0;
  float b = 0.0;
  float c = 0.0;
  float p[3] = { 0.0, 0.0, 0.0 };

  FACE_VERT *cur_vert = ref_vert->next->next;
  while (cur_vert != ref_vert->prev) {
    p[0] = verticies[vbo_index_combos[cur_vert->index][0]][0] - verticies[vbo_index_combos[triangle[1]][0]][0] - coords[0][0];
    p[1] = verticies[vbo_index_combos[cur_vert->index][0]][1] - verticies[vbo_index_combos[triangle[1]][0]][1] - coords[0][1];
    p[2] = verticies[vbo_index_combos[cur_vert->index][0]][2] - verticies[vbo_index_combos[triangle[1]][0]][2] - coords[0][2];

    c = ((u[0]*v[1]*p[2])-(u[0]*v[2]*p[1])+(v[0]*u[2]*p[1])-(v[0]*u[1]*p[2])-(u[2]*v[1]*p[0])+(v[2]*u[1]*p[0])) /
        ((u[0]*u_cross_v[2]*v[1])-(u[0]*v[2]*u_cross_v[1])+(v[0]*u[2]*u_cross_v[1])-(v[0]*u_cross_v[2]*u[1])-(u_cross_v[0]*u[2]*v[1])+(u_cross_v[0]*v[2]*u[1]));

    a = ((v[0]*u_cross_v[2]*p[1])+(v[0]*u_cross_v[1]*p[2])+(u_cross_v[0]*v[2]*p[1])-(u_cross_v[0]*v[1]*p[2])-(v[2]*u_cross_v[1]*p[0])+(u_cross_v[2]*v[1]*p[0])) /
        ((u[0]*u_cross_v[2]*v[1])-(u[0]*v[2]*u_cross_v[1])+(v[0]*u[2]*u_cross_v[1])-(v[0]*u_cross_v[2]*u[1])-(u_cross_v[0]*u[2]*v[1])+(u_cross_v[0]*v[2]*u[1]));

    b = ((u[0]*u_cross_v[2]*p[1])-(u[0]*u_cross_v[1]*p[2])-(u_cross_v[0]*u[2]*p[1])+(u_cross_v[0]*u[1]*p[2])+(u[2]*u_cross_v[1]*p[0])-(u_cross_v[2]*u[1]*p[0])) /
        ((u[0]*u_cross_v[2]*v[1])-(u[0]*v[2]*u_cross_v[1])+(v[0]*u[2]*u_cross_v[1])-(v[0]*u_cross_v[2]*u[1])-(u_cross_v[0]*u[2]*v[1])+(u_cross_v[0]*v[2]*u[1]));

    if (c == 0.0 && a >= 0.0 && a <= 1.0 && b >= 0.0 && b <= 1.0 &&
        a + b <= 1.0) {
      return 0;
    }

    cur_vert = cur_vert->next;
  }

  return 0;
}

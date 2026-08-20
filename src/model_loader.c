#include <model_loader.h>

MODEL_DATA *load_model_data(char *path) {
  char *bin_path = malloc(strlen(path) + 5);
  sprintf(bin_path, "%s.bin", path);
  FILE *file = fopen(bin_path, "rb");

  if (file == NULL) {
    LINE_BUFFER *line_buff = get_lines(path);
    if (line_buff == NULL) {
      return NULL;
    }

    preprocess_lines(line_buff);

    file = fopen(bin_path, "rb");
    if (file == NULL) {
      return NULL;
    }
  }
  free(bin_path);

  size_t b_len = 0;
  size_t col_len = 0;
  size_t v_len = 0;
  size_t i_len = 0;
  size_t a_len = 0;
  size_t total_chains = 0;
  size_t total_keyframes = 0;
  size_t total_frames = 0;
  fread(&b_len, sizeof(size_t), 1, file);
  fread(&col_len, sizeof(size_t), 1, file);
  fread(&v_len, sizeof(size_t), 1, file);
  fread(&i_len, sizeof(size_t), 1, file);
  fread(&a_len, sizeof(size_t), 1, file);
  fread(&total_chains, sizeof(size_t), 1, file);
  fread(&total_keyframes, sizeof(size_t), 1, file);
  fread(&total_frames, sizeof(size_t), 1, file);

  int material_flag = 0;
  int path_len = 0;
  fread(&material_flag, sizeof(int), 1, file);
  char *mat_paths[NUM_PROPS];
  memset(mat_paths, 0, sizeof(char *) * NUM_PROPS);
  if (material_flag) {
    for (int i = 0; i < NUM_PROPS; i++) {
      fread(&path_len, sizeof(int), 1, file);
      if (path_len > 0) {
        mat_paths[i] = malloc(path_len);
        fread(mat_paths[i], sizeof(char), path_len, file);
      } else {
        mat_paths[i] = NULL;
      }
    }
  }

  BONE *bones = NULL;
  int *collider_links = NULL;
  if (b_len) {
    bones = malloc(sizeof(BONE) * b_len);
    if (bones == NULL) {
      goto ERR_BONES;
    }
    collider_links = malloc(sizeof(int) * b_len);
    if (collider_links == NULL) {
      goto ERR_COL_LINKS;
    }
  }

  VBO *vertices = NULL;
  if (v_len) {
    vertices = malloc(sizeof(VBO) * v_len);
    if (vertices == NULL) {
      goto ERR_VERTS;
    }
  }

  int *indicies = NULL;
  if (i_len) {
    indicies = malloc(sizeof(int) * 3 * i_len);
    if (indicies == NULL) {
      goto ERR_INDS;
    }
  }

  ANIMATION *animations = NULL;
  if (a_len) {
    animations = malloc(sizeof(ANIMATION) * a_len);
    if (animations == NULL) {
      goto ERR_ANIMS;
    }
  }

  MODEL_DATA *md = malloc(sizeof(MODEL_DATA));
  if (md == NULL) {
    goto ERR_MD;
  }

  K_CHAIN *k_chain_block = NULL;
  size_t next_chain = 0;
  if (total_chains) {
    k_chain_block = malloc(sizeof(K_CHAIN) * total_chains);
    if (k_chain_block == NULL) {
      goto ERR_K_CHAIN;
    }
  }

  KEYFRAME *keyframe_block = NULL;
  size_t next_keyframe = 0;
  if (total_keyframes) {
    keyframe_block = malloc(sizeof(KEYFRAME) * total_keyframes);
    if (keyframe_block == NULL) {
      goto ERR_KF_BLOCK;
    }
  }

  int *sled_block = NULL;
  size_t next_sled = 0;
  if (total_frames) {
    sled_block = malloc(sizeof(int) * total_frames);
    if (sled_block == NULL) {
      goto ERR_SLED_BLK;
    }
  }

  COLLIDER *colliders = NULL;
  int *bone_links = NULL;
  if (col_len) {
    colliders = malloc(sizeof(COLLIDER) * col_len);
    if (colliders == NULL) {
      goto ERR_COLS;
    }
    bone_links = malloc(sizeof(int) * col_len);
    if (bone_links == NULL) {
      goto ERR_BL;
    }
  }

  if (bones) {
    fread(bones, sizeof(BONE), b_len, file);
  }
  if (collider_links) {
    fread(collider_links, sizeof(int), b_len, file);
  }
  if (colliders) {
    fread(colliders, sizeof(COLLIDER), col_len, file);
  }
  if (bone_links) {
    fread(bone_links, sizeof(int), col_len, file);
  }
  if (vertices) {
    fread(vertices, sizeof(VBO), v_len, file);
  }
  if (indicies) {
    fread(indicies, sizeof(int) * 3, i_len, file);
  }

  for (int i = 0; i < a_len; i++) {
    fread(&(animations[i].num_chains), sizeof(size_t), 1, file);
    fread(&(animations[i].duration), sizeof(size_t), 1, file);

    animations[i].keyframe_chains = k_chain_block + next_chain;
    next_chain += animations[i].num_chains;

    for (int j = 0; j < animations[i].num_chains; j++) {
      K_CHAIN *cur = animations[i].keyframe_chains + j;
      fread(&(cur->b_id), sizeof(unsigned int), 1, file);
      fread(&(cur->type), sizeof(C_TYPE), 1, file);
      fread(&(cur->num_frames), sizeof(size_t), 1, file);

      cur->chain = keyframe_block + next_keyframe;
      next_keyframe += cur->num_frames;

      for (int k = 0; k < cur->num_frames; k++) {
        fread(cur->chain[k].offset, sizeof(float), 4, file);
        fread(&(cur->chain[k].frame), sizeof(int), 1, file);
      }

      cur->sled = sled_block + next_sled;
      next_sled += animations[i].duration;

      int cur_frame = -1;
      if (cur->chain[0].frame == 0) {
        cur_frame = 0;
      }
      for (int k = 0; k < animations[i].duration; k++) {
        cur->sled[k] = cur_frame;
        if (cur_frame + 1 < cur->num_frames &&
            k == cur->chain[cur_frame + 1].frame) {
          cur_frame++;
        }
      }
    }
  }

  // Ensure read colliders have valid winding order and faces
  for (int i = 0; i < col_len; i++) {
    if (!validate_collider(colliders + i)) {
      fprintf(stderr, "ERR: Invalid collider (id:%d) for model: %s\n", i, path);
      goto ERR_BL;
    }
  }

  md->animations = animations;
  md->k_chain_block = k_chain_block;
  md->keyframe_block = keyframe_block;
  md->sled_block = sled_block;
  md->bones = bones;
  md->bone_collider_links = collider_links;
  md->colliders = colliders;
  md->collider_bone_links = bone_links;
  md->vertices = vertices;
  md->indices = indicies;
  md->num_animations = a_len;
  md->num_bones = b_len;
  md->num_colliders = col_len;
  md->num_indices = 3 * i_len;
  md->num_vertices = v_len;
  for (int i = 0; i < NUM_PROPS; i++) {
    md->mat_paths[i] = mat_paths[i];
  }

  fclose(file);
  return md;

ERR_BL:
  free(colliders);
ERR_COLS:
  free(sled_block);
ERR_SLED_BLK:
  free(keyframe_block);
ERR_KF_BLOCK:
  free(k_chain_block);
ERR_K_CHAIN:
  free(md);
ERR_MD:
  free(animations);
ERR_ANIMS:
  free(indicies);
ERR_INDS:
  free(vertices);
ERR_VERTS:
  free(collider_links);
ERR_COL_LINKS:
  free(bones);
ERR_BONES:
  fclose(file);
  return NULL;
}

void init_model_vao(MODEL *model) {
  int error = 0;
  unsigned int VAO_id;
  glGenVertexArrays(1, &VAO_id);
  if ((error = glGetError()) != GL_NO_ERROR) {
    fprintf(stderr, "FAILED TO ALLOCATE VAO: %d\n", error);
  }
  glBindVertexArray(VAO_id);
  if ((error = glGetError()) != GL_NO_ERROR) {
    fprintf(stderr, "FAILED TO BIND VAO: %d\n", error);
  }
  glBindBuffer(GL_ARRAY_BUFFER, model->VBO);
  if ((error = glGetError()) != GL_NO_ERROR) {
    fprintf(stderr, "FAILED TO BIND VBO: %d\n", error);
  }
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->EBO);
  if ((error = glGetError()) != GL_NO_ERROR) {
    fprintf(stderr, "FAILED TO BIND EBO: %d\n", error);
  }

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VBO),
                        (void *) 0);
  if ((error = glGetError()) != GL_NO_ERROR) {
    fprintf(stderr, "FAILED TO SET ATTRIB 0: %d\n", error);
  }
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VBO),
                        (void *) (sizeof(float) * 3));
  if ((error = glGetError()) != GL_NO_ERROR) {
    fprintf(stderr, "FAILED TO SET ATTRIB 1: %d\n", error);
  }
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VBO),
                        (void *) (sizeof(float) * 6));
  if ((error = glGetError()) != GL_NO_ERROR) {
    fprintf(stderr, "FAILED TO SET ATTRIB 2: %d\n", error);
  }
  glVertexAttribIPointer(3, 4, GL_INT, sizeof(VBO),
                        (void *) (sizeof(float) * 8));
  if ((error = glGetError()) != GL_NO_ERROR) {
    fprintf(stderr, "FAILED TO SET ATTRIB 3: %d\n", error);
  }
  glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(VBO),
                        (void *) ((sizeof(float) * 8) + (sizeof(int) * 4)));
  if ((error = glGetError()) != GL_NO_ERROR) {
    fprintf(stderr, "FAILED TO SET ATTRIB 4: %d\n", error);
  }

  glEnableVertexAttribArray(0);
  if ((error = glGetError()) != GL_NO_ERROR) {
    fprintf(stderr, "FAILED TO ENABLE ATTRIB 0: %d\n", error);
  }
  glEnableVertexAttribArray(1);
  if ((error = glGetError()) != GL_NO_ERROR) {
    fprintf(stderr, "FAILED TO ENABLE ATTRIB 1: %d\n", error);
  }
  glEnableVertexAttribArray(2);
  if ((error = glGetError()) != GL_NO_ERROR) {
    fprintf(stderr, "FAILED TO ENABLE ATTRIB 2: %d\n", error);
  }
  glEnableVertexAttribArray(3);
  if ((error = glGetError()) != GL_NO_ERROR) {
    fprintf(stderr, "FAILED TO ENABLE ATTRIB 3: %d\n", error);
  }
  glEnableVertexAttribArray(4);
  if ((error = glGetError()) != GL_NO_ERROR) {
    fprintf(stderr, "FAILED TO ENABLE ATTRIB 4: %d\n", error);
  }
  glBindVertexArray(0);
  if ((error = glGetError()) != GL_NO_ERROR) {
    fprintf(stderr, "FAILED TO UNBIND VAO: %d\n", error);
  }

  model->VAO = VAO_id;
}

MODEL *gen_model(MODEL_DATA *md, int gen_vao) {
  MODEL *model = malloc(sizeof(MODEL));
  if (model == NULL) {
    printf("Unable to allocate model\n");
    return NULL;
  }

  int error = 0;
  unsigned int VBO_id;
  glGenBuffers(1, &VBO_id);
  if ((error = glGetError()) != GL_NO_ERROR) {
    fprintf(stderr, "FAILED TO ALLOCATE VBO: %d\n", error);
  }
  glBindBuffer(GL_ARRAY_BUFFER, VBO_id);
  if ((error = glGetError()) != GL_NO_ERROR) {
    fprintf(stderr, "FAILED TO BIND VBO: %d\n", error);
  }
  glBufferData(GL_ARRAY_BUFFER, sizeof(VBO) * md->num_vertices, md->vertices,
               GL_STATIC_DRAW);
  if ((error = glGetError()) != GL_NO_ERROR) {
    fprintf(stderr, "FAILED TO POPULATE VBO: %d\n", error);
  }

  unsigned int EBO_id;
  glGenBuffers(1, &EBO_id);
  if ((error = glGetError()) != GL_NO_ERROR) {
    fprintf(stderr, "FAILED TO ALLOCATE EBO: %d\n", error);
  }
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_id);
  if ((error = glGetError()) != GL_NO_ERROR) {
    fprintf(stderr, "FAILED TO BIND EBO: %d\n", error);
  }
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * md->num_indices,
               md->indices, GL_STATIC_DRAW);
  if ((error = glGetError()) != GL_NO_ERROR) {
    fprintf(stderr, "FAILED TO POPULATE EBO: %d\n", error);
  }

  if (!md->num_bones) {
    // Create root bone of model if one does not exist already
    md->num_bones = 1;
    md->bones = malloc(sizeof(BONE));
    if (!md->bones) {
      fprintf(stderr, "Failed to allocate root bone\n");
      return NULL;
    }

    md->bone_collider_links = malloc(sizeof(int));
    if (!md->bone_collider_links) {
      fprintf(stderr, "Failed to allocate bone to collider linkage\n");
      return NULL;
    }
    md->bone_collider_links[0] = -1;

    if (md->num_colliders) {
      md->bone_collider_links[0] = 0;
      for (size_t i = 0; i < md->num_colliders; i++) {
        md->collider_bone_links[i] = 0;
      }
    }

    glm_mat3_identity(md->bones[0].coordinate_matrix);
    glm_vec3_copy((vec3) {0.0, 1.0, 0.0}, md->bones[0].head);
    glm_vec3_zero(md->bones[0].base);
    md->bones[0].parent = -1;
    md->bones[0].num_children = 0;
  }

  model->VAO = INVALID_INDEX;
  model->VBO = VBO_id;
  model->EBO = EBO_id;
  model->animations = md->animations;
  model->k_chain_block = md->k_chain_block;
  model->keyframe_block = md->keyframe_block;
  model->sled_block = md->sled_block;
  model->bones = md->bones;
  model->bone_collider_links = md->bone_collider_links;
  model->colliders = md->colliders;
  model->collider_bone_links = md->collider_bone_links;
  model->num_animations = md->num_animations;
  model->num_bones = md->num_bones;
  model->num_colliders = md->num_colliders;
  model->num_indicies = md->num_indices;

  for (int i = 0; i < NUM_PROPS; i++) {
    model->textures[i] = INVALID_TEX;
  }
  for (int i = 0; i < NUM_PROPS; i++) {
    if (md->mat_paths[i] != NULL) {
      gen_texture_id(md->mat_paths[i], model->textures + i);
      free(md->mat_paths[i]);
    }
  }

  if (gen_vao) {
    init_model_vao(model);
  }

  return model;
}

MODEL *load_model(char *path) {
  MODEL_DATA *md = load_model_data(path);
  if (md == NULL) {
    return NULL;
  }
  MODEL *model = gen_model(md, 1);
  free(md->vertices);
  free(md->indices);
  free(md);

  return model;
}

MODEL *load_model_vaoless(char *path) {
  MODEL_DATA *md = load_model_data(path);
  if (md == NULL) {
    return NULL;
  }
  MODEL *model = gen_model(md, 0);
  free(md->vertices);
  free(md->indices);
  free(md);

  return model;
}

int export_model_data_obj(MODEL_DATA *d, char *path) {
  FILE *file = fopen(path, "w");
  if (file == NULL) {
    fprintf(stderr, "Failed to open obj file for writing.\n");
    goto ERR_FILE;
  }

  vec3 *v = malloc(sizeof(vec3) * BUFF_STARTING_LEN);
  size_t v_len = 0;
  size_t v_size = BUFF_STARTING_LEN;
  if (!v) {
    goto ERR_V;
  }

  vec2 *vt = malloc(sizeof(vec2) * BUFF_STARTING_LEN);
  size_t vt_len = 0;
  size_t vt_size = BUFF_STARTING_LEN;
  if (!vt) {
    goto ERR_VT;
  }

  vec3 *vn = malloc(sizeof(vec3) * BUFF_STARTING_LEN);
  size_t vn_len = 0;
  size_t vn_size = BUFF_STARTING_LEN;
  if (!vn) {
    goto ERR_VN;
  }

  int status = 0;

  fprintf(stderr, "Exporting Vertex Coordinates...\n");
  fprintf(file, "# Vertex Coordinates\n");
  for (size_t i = 0; i < d->num_vertices; i++) {
    int found = 0;
    for (size_t j = 0; j < v_len; j++) {
      if (v[j][X] == d->vertices[i].vertex[X] &&
          v[j][Y] == d->vertices[i].vertex[Y] &&
          v[j][Z] == d->vertices[i].vertex[Z]) {
        found = 1;
        break;
      }
    }
    if (!found) {
      glm_vec3_copy(d->vertices[i].vertex, v[v_len]);
      fprintf(file, "v %f %f %f\n", v[v_len][X], v[v_len][Y], v[v_len][Z]);
      v_len++;
      if (v_len == v_size) {
        status = double_buffer((void **) &v, &v_size, sizeof(vec3));
        if (status) {
          goto ERR_REALLOC;
        }
      }
    }
  }

  fprintf(stderr, "Exporting Texture Coordinates...\n");
  fprintf(file, "# Texture Coordinates\n");
  for (size_t i = 0; i < d->num_vertices; i++) {
    int found = 0;
    for (size_t j = 0; j < vt_len; j++) {
      if (vt[j][X] == d->vertices[i].tex_coord[X] &&
          vt[j][Y] == d->vertices[i].tex_coord[Y]) {
        found = 1;
        break;
      }
    }
    if (!found) {
      glm_vec2_copy(d->vertices[i].tex_coord, vt[vt_len]);
      fprintf(file, "vt %f %f\n", vt[vt_len][X], vt[vt_len][Y]);
      vt_len++;
      if (vt_len == vt_size) {
        status = double_buffer((void **) &vt, &vt_size, sizeof(vec2));
        if (status) {
          goto ERR_REALLOC;
        }
      }
    }
  }

  fprintf(stderr, "Exporting Normals...\n");
  fprintf(file, "# Normals\n");
  for (size_t i = 0; i < d->num_vertices; i++) {
    int found = 0;
    for (size_t j = 0; j < vn_len; j++) {
      if (vn[j][X] == d->vertices[i].tex_coord[X] &&
          vn[j][Y] == d->vertices[i].tex_coord[Y]) {
        found = 1;
        break;
      }
    }
    if (!found) {
      glm_vec3_copy(d->vertices[i].normal, vn[vn_len]);
      fprintf(file, "vn %f %f %f\n", vn[vn_len][X], vn[vn_len][Y],
              vn[vn_len][Z]);
      vn_len++;
      if (vn_len == vn_size) {
        status = double_buffer((void **) &vn, &vn_size, sizeof(vec3));
        if (status) {
          goto ERR_REALLOC;
        }
      }
    }
  }

  fprintf(stderr, "Exporting Facess...\n");
  for (size_t i = 0; i < d->num_indices / 3; i++) {
    ivec3 f = { d->indices[(i*3)], d->indices[(i*3)+1], d->indices[(i*3)+2] };
    fprintf(file, "f");
    for (int j = 0; j < 3; j++) {
      vec3 vert = { d->vertices[f[j]].vertex[X],
                    d->vertices[f[j]].vertex[Y],
                    d->vertices[f[j]].vertex[Z] };
      vec2 tex = { d->vertices[f[j]].tex_coord[X],
                   d->vertices[f[j]].tex_coord[Y] };
      vec3 norm = { d->vertices[f[j]].normal[X],
                    d->vertices[f[j]].normal[Y],
                    d->vertices[f[j]].normal[Z] };

      for (size_t k = 0; k < v_len; k++) {
        if (v[k][X] == vert[X] && v[k][Y] == vert[Y] && v[k][Z] == vert[Z]) {
          fprintf(file, " %ld/", k + 1);
          break;
        }
      }
      for (size_t k = 0; k < vt_len; k++) {
        if (vt[k][X] == tex[X] && vt[k][Y] == tex[Y] && vt[k][Z] == tex[Z]) {
          fprintf(file, "%ld", k + 1);
          break;
        }
      }
      for (size_t k = 0; k < vn_len; k++) {
        if (vn[k][X] == norm[X] && vn[k][Y] == norm[Y] &&
            vn[k][Z] == norm[Z]) {
          fprintf(file, "/%ld", k + 1);
          break;
        }
      }
    }
    fprintf(file, "\n");
  }

  fprintf(stderr, "Wavefront OBJ Export Finished.\n");
  free(v);
  free(vt);
  free(vn);
  fclose(file);
  return 0;

  /*
  for (size_t i = 0; i < d->num_vertices; i++) {
    fprintf(file, "v %f %f %f\n", d->vertices[i].vertex[X],
                                  d->vertices[i].vertex[Y],
                                  d->vertices[i].vertex[Z]);
  }

  for (size_t i = 0; i < d->num_vertices; i++) {
    fprintf(file, "vt %f %f\n", d->vertices[i].tex_coord[X],
                                d->vertices[i].tex_coord[Y]);
  }

  for (size_t i = 0; i < d->num_vertices; i++) {
    fprintf(file, "vn %f %f %f\n", d->vertices[i].normal[X],
                                   d->vertices[i].normal[Y],
                                   d->vertices[i].normal[Z]);
  }

  for (size_t i = 0; i < d->num_indices / 3; i++) {
    fprintf(file, "f %d/%d/%d %d/%d/%d %d/%d/%d\n",
            d->indices[(i*3)]+1, d->indices[(i*3)]+1, d->indices[(i*3)]+1,
            d->indices[(i*3)+1]+1, d->indices[(i*3)+1]+1,
            d->indices[(i*3)+1]+1, d->indices[(i*3)+2]+1,
            d->indices[(i*3)+2]+1, d->indices[(i*3)+2]+1);
  }
  fclose(file);
  return 0;
  */

ERR_REALLOC:
  free(vn);
ERR_VN:
  free(vt);
ERR_VT:
  free(v);
ERR_V:
  fclose(file);
ERR_FILE:
  return -1;
}

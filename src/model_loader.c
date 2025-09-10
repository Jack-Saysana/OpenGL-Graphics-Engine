#include <model_loader.h>

MODEL_DATA *load_model_data(char *path) {
  char *bin_path = malloc(strlen(path) + 5);
  sprintf(bin_path, "%s.bin", path);
  FILE *file = fopen(bin_path, "rb");

  if (file == NULL) {
    LINE_BUFFER *line_buff = get_lines(path);
    if (line_buff == NULL) {
      printf("Unable to create line buffer\n");
      return NULL;
    }

    preprocess_lines(line_buff);

    file = fopen(bin_path, "rb");
    if (file == NULL) {
      printf("Unable to open preproccessed file\n");
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
      fclose(file);
      printf("Unable to allocate bone buffer\n");
      return NULL;
    }
    collider_links = malloc(sizeof(int) * b_len);
    if (collider_links == NULL) {
      fclose(file);
      free(bones);
      printf("Unable to allocate collider_links\n");
      return NULL;
    }
  }

  VBO *vertices = NULL;
  if (v_len) {
    vertices = malloc(sizeof(VBO) * v_len);
    if (vertices == NULL) {
      fclose(file);
      free(bones);
      free(collider_links);
      printf("Unable to allocate vertex buffer\n");
      return NULL;
    }
  }

  int *indicies = NULL;
  if (i_len) {
    indicies = malloc(sizeof(int) * 3 * i_len);
    if (indicies == NULL) {
      fclose(file);
      free(bones);
      free(collider_links);
      free(vertices);
      printf("Unable to allocate indicies buffer\n");
      return NULL;
    }
  }

  ANIMATION *animations = NULL;
  if (a_len) {
    animations = malloc(sizeof(ANIMATION) * a_len);
    if (animations == NULL) {
      fclose(file);
      free(bones);
      free(collider_links);
      free(vertices);
      free(indicies);
      printf("Unable to allocate animation buffer\n");
      return NULL;
    }
  }

  MODEL_DATA *md = malloc(sizeof(MODEL_DATA));
  if (md == NULL) {
    fclose(file);
    free(bones);
    free(collider_links);
    free(vertices);
    free(indicies);
    free(animations);
    printf("Unable to allocate model data\n");
    return NULL;
  }

  K_CHAIN *k_chain_block = NULL;
  size_t next_chain = 0;
  if (total_chains) {
    k_chain_block = malloc(sizeof(K_CHAIN) * total_chains);
    if (k_chain_block == NULL) {
      fclose(file);
      free(bones);
      free(collider_links);
      free(vertices);
      free(indicies);
      free(animations);
      free(md);
      printf("Unable to allocate keyframe chains\n");
      return NULL;
    }
  }

  KEYFRAME *keyframe_block = NULL;
  size_t next_keyframe = 0;
  if (total_keyframes) {
    keyframe_block = malloc(sizeof(KEYFRAME) * total_keyframes);
    if (keyframe_block == NULL) {
      fclose(file);
      free(bones);
      free(collider_links);
      free(vertices);
      free(indicies);
      free(animations);
      free(md);
      free(k_chain_block);
      printf("Unable to allocate keyframes\n");
      return NULL;
    }
  }

  int *sled_block = NULL;
  size_t next_sled = 0;
  if (total_frames) {
    sled_block = malloc(sizeof(int) * total_frames);
    if (sled_block == NULL) {
      fclose(file);
      free(bones);
      free(collider_links);
      free(vertices);
      free(indicies);
      free(animations);
      free(md);
      free(k_chain_block);
      free(keyframe_block);
      printf("Unable to allocate keyframe sled\n");
      return NULL;
    }
  }

  COLLIDER *colliders = NULL;
  int *bone_links = NULL;
  if (col_len) {
    colliders = malloc(sizeof(COLLIDER) * col_len);
    if (colliders == NULL) {
      fclose(file);
      free(bones);
      free(collider_links);
      free(vertices);
      free(indicies);
      free(animations);
      free(md);
      free(k_chain_block);
      free(keyframe_block);
      free(sled_block);
      printf("Unable to allocate colliders\n");
      return NULL;
    }
    bone_links = malloc(sizeof(int) * col_len);
    if (bone_links == NULL) {
      fclose(file);
      free(bones);
      free(collider_links);
      free(vertices);
      free(indicies);
      free(animations);
      free(md);
      free(k_chain_block);
      free(keyframe_block);
      free(sled_block);
      free(colliders);
      printf("Unable to allocate bone links\n");
      return NULL;
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
  fclose(file);

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

  return md;
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

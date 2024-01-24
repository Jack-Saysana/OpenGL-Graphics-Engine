#include <model_loader.h>

MODEL *load_model(char *path) {
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
  MATERIAL *obj_mat = NULL;
  fread(&material_flag, sizeof(int), 1, file);
  if (material_flag) {
    obj_mat = malloc(sizeof(MATERIAL));
    for (int i = 0; i < NUM_PROPS; i++) {
      fread(&path_len, sizeof(int), 1, file);
      if (path_len > 0) {
        obj_mat->mat_paths[i] = malloc(path_len);
        fread(obj_mat->mat_paths[i], sizeof(char), path_len, file);
      } else {
        obj_mat->mat_paths[i] = NULL;
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

  MODEL *model = malloc(sizeof(MODEL));
  if (model == NULL) {
    fclose(file);
    free(bones);
    free(collider_links);
    free(vertices);
    free(indicies);
    free(animations);
    printf("Unable to allocate model\n");
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
      free(model);
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
      free(model);
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
      free(model);
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
      free(model);
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
      free(model);
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

  unsigned int VAO_id;
  glGenVertexArrays(1, &VAO_id);
  glBindVertexArray(VAO_id);

  unsigned int VBO_id;
  glGenBuffers(1, &VBO_id);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_id);
  glBufferData(GL_ARRAY_BUFFER, sizeof(VBO) * v_len, vertices, GL_STATIC_DRAW);

  unsigned int EBO_id;
  glGenBuffers(1, &EBO_id);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_id);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * 3 * i_len, indicies,
               GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VBO),
                        (void *) 0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VBO),
                        (void *) (sizeof(float) * 3));
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VBO),
                        (void *) (sizeof(float) * 6));
  glVertexAttribIPointer(3, 4, GL_INT, sizeof(VBO),
                        (void *) (sizeof(float) * 8));
  glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(VBO),
                        (void *) ((sizeof(float) * 8) + (sizeof(int) * 4)));

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);
  glEnableVertexAttribArray(3);
  glEnableVertexAttribArray(4);
  glBindVertexArray(0);

  model->VAO = VAO_id;
  model->VBO = VBO_id;
  model->EBO = EBO_id;
  model->animations = animations;
  model->k_chain_block = k_chain_block;
  model->keyframe_block = keyframe_block;
  model->sled_block = sled_block;
  model->bones = bones;
  model->bone_collider_links = collider_links;
  model->colliders = colliders;
  model->collider_bone_links = bone_links;
  model->num_animations = a_len;
  model->num_bones = b_len;
  model->num_colliders = col_len;
  model->num_indicies = i_len * 3;

  if (obj_mat != NULL) {
    for (int i = 0; i < NUM_PROPS; i++) {
      if (obj_mat->mat_paths[i] != NULL) {
        gen_texture_id(obj_mat->mat_paths[i], model->textures + i);
      } else {
        model->textures[i] = 0xBAADF00D;
      }
    }

    free_materials(obj_mat, 1);
  } else {
    for (int i = 0; i < NUM_PROPS; i++) {
      model->textures[i] = 0xBAADF00D;
    }
  }

  free(bin_path);
  free(vertices);
  free(indicies);

  return model;
}

int gen_texture_id(char *tex_path, unsigned int *dest) {
  unsigned int texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  int width;
  int height;
  int nrChannels;
  stbi_set_flip_vertically_on_load(1);
  unsigned char *data = stbi_load(tex_path, &width, &height, &nrChannels, 0);
  if (data) {
    int format = GL_RGBA;
    if (nrChannels == 1) {
      format = GL_RED;
    } else if (nrChannels == 2) {
      format = GL_RG;
    } else if (nrChannels == 3) {
      format = GL_RGB;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
  } else {
    fprintf(stderr, "Failed to load texture at: %s\n", tex_path);
    return -1;
  }
  stbi_image_free(data);

  *dest = texture;
  return 0;
}

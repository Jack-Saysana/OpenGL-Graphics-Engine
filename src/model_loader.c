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
  free(bin_path);

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

MODEL *gen_model(MODEL_DATA *md) {
  MODEL *model = malloc(sizeof(MODEL));
  if (model == NULL) {
    printf("Unable to allocate model\n");
    return NULL;
  }

  unsigned int VAO_id;
  glGenVertexArrays(1, &VAO_id);
  glBindVertexArray(VAO_id);

  unsigned int VBO_id;
  glGenBuffers(1, &VBO_id);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_id);
  glBufferData(GL_ARRAY_BUFFER, sizeof(VBO) * md->num_vertices, md->vertices,
               GL_STATIC_DRAW);

  unsigned int EBO_id;
  glGenBuffers(1, &EBO_id);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_id);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * md->num_indices,
               md->indices, GL_STATIC_DRAW);

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

  return model;
}

MODEL *load_model(char *path) {
  MODEL_DATA *md = load_model_data(path);
  if (md == NULL) {
    return NULL;
  }
  MODEL *model = gen_model(md);
  free(md->vertices);
  free(md->indices);
  free(md);

  return model;
}

int write_model_obj(MODEL_DATA *md, char *path) {
  // TODO Implement material saving
  FILE *file = fopen(path, "w");
  if (!file) {
    return -1;
  }

  for (size_t i = 1; i < md->num_bones; i++) {
    int parent = md->bones[i].parent;
    if (parent != -1) {
      parent--;
    }
    // Bones
    fprintf(file, "b %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %d %d\n",
                     md->bones[i].head[X],
                     md->bones[i].head[Y],
                     md->bones[i].head[Z],
                     md->bones[i].base[X],
                     md->bones[i].base[Y],
                     md->bones[i].base[Z],
                     md->bones[i].coordinate_matrix[0][0],
                     md->bones[i].coordinate_matrix[0][1],
                     md->bones[i].coordinate_matrix[0][2],
                     md->bones[i].coordinate_matrix[1][0],
                     md->bones[i].coordinate_matrix[1][1],
                     md->bones[i].coordinate_matrix[1][2],
                     md->bones[i].coordinate_matrix[2][0],
                     md->bones[i].coordinate_matrix[2][1],
                     md->bones[i].coordinate_matrix[2][2],
                     parent, md->bones[i].num_children);
  }
  fprintf(file, "\n");

  // Verticies
  for (size_t i = 0; i < md->num_vertices; i++) {
    fprintf(file, "v %f %f %f %d:%f %d:%f %d:%f %d:%f\n",
            md->vertices[i].vertex[X],
            md->vertices[i].vertex[Y],
            md->vertices[i].vertex[Z],
            md->vertices[i].bone_ids[X],
            md->vertices[i].weights[X],
            md->vertices[i].bone_ids[Y],
            md->vertices[i].weights[Y],
            md->vertices[i].bone_ids[Z],
            md->vertices[i].weights[Z],
            md->vertices[i].bone_ids[W],
            md->vertices[i].weights[W]);
  }
  for (size_t i = 0; i < md->num_vertices; i++) {
    fprintf(file, "vt %f %f\n",
            md->vertices[i].tex_coord[X],
            md->vertices[i].tex_coord[Y]);
  }
  for (size_t i = 0; i < md->num_vertices; i++) {
    fprintf(file, "vn %f %f %f\n",
            md->vertices[i].normal[X],
            md->vertices[i].normal[Y],
            md->vertices[i].normal[Z]);
  }

  // Face indicies
  for (size_t i = 0; i < md->num_indices / 3; i++) {
    fprintf(file, "f %d/%d/%d %d/%d/%d %d/%d/%d\n",
            md->indices[(i*3)] + 1,
            md->indices[(i*3)] + 1,
            md->indices[(i*3)] + 1,
            md->indices[(i*3)+1] + 1,
            md->indices[(i*3)+1] + 1,
            md->indices[(i*3)+1] + 1,
            md->indices[(i*3)+2] + 1,
            md->indices[(i*3)+2] + 1,
            md->indices[(i*3)+2] + 1);
  }
  fprintf(file, "\n");

  // Colliders
  for (size_t i = 0; i < md->num_colliders; i++) {
    if (md->colliders[i].type == POLY) {
      vec3 verts[8];
      int root_bone = md->collider_bone_links[i];
      // Collider verts in .obj files are given in entity space, but are stored
      // in bone face in engine, so we must convert them back to entity space
      if (root_bone != -1) {
        mat4 bone_to_entity = GLM_MAT4_IDENTITY_INIT;
        glm_mat4_ins3(md->bones[root_bone].coordinate_matrix, bone_to_entity);
        glm_vec4(md->colliders[i].data.center_of_mass, 1.0, bone_to_entity[3]);

        // Convert collider verticies to bone space
        for (int j = 0; j < 8; j++) {
          glm_mat4_mulv3(bone_to_entity, md->colliders[i].data.verts[j], 1.0,
                         verts[j]);
        }
      }

      fprintf(file, "hp %d %d %d %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f\n",
              md->colliders[i].category,
              md->collider_bone_links[i]-1,
              md->colliders[i].data.num_used,
              verts[0][X],
              verts[0][Y],
              verts[0][Z],
              verts[1][X],
              verts[1][Y],
              verts[1][Z],
              verts[2][X],
              verts[2][Y],
              verts[2][Z],
              verts[3][X],
              verts[3][Y],
              verts[3][Z],
              verts[4][X],
              verts[4][Y],
              verts[4][Z],
              verts[5][X],
              verts[5][Y],
              verts[5][Z],
              verts[6][X],
              verts[6][Y],
              verts[6][Z],
              verts[7][X],
              verts[7][Y],
              verts[7][Z]);
    } else {
      fprintf(file, "hs %d %d %f %f %f %f\n",
              md->colliders[i].category,
              md->collider_bone_links[i]-1,
              md->colliders[i].data.center[X],
              md->colliders[i].data.center[Y],
              md->colliders[i].data.center[Z],
              md->colliders[i].data.radius);
    }
    for (int j = 0; j < md->colliders[i].num_dofs; j++) {
      int dof_type = md->colliders[i].dofs[j][W];
#ifdef __linux__
      fprintf(file, "dof %ld %d %f %f %f\n",
#else
      fprintf(file, "dof %lld %d %f %f %f\n",
#endif
              i, dof_type,
              md->colliders[i].dofs[j][X],
              md->colliders[i].dofs[j][Y],
              md->colliders[i].dofs[j][Z]);
    }
  }
  fprintf(file, "\n");

  // Animations
  for (size_t i = 0; i < md->num_animations; i++) {
#ifdef __linux__
    fprintf(file, "a %ld\n", md->animations[i].duration);
#else
    fprintf(file, "a %lld\n", md->animations[i].duration);
#endif
    K_CHAIN *chains = md->animations[i].keyframe_chains;
    for (size_t j = 0; j < md->animations[i].num_chains; j++) {
      if (chains[j].type == LOCATION) {
        fprintf(file, "cl %d\n", chains[j].b_id);
      } else if (chains[j].type == ROTATION) {
        fprintf(file, "cr %d\n", chains[j].b_id);
      } else {
        fprintf(file, "cs %d\n", chains[j].b_id);
      }
      for (size_t k = 0; k < chains[j].num_frames; k++) {
        if (chains[j].type == ROTATION) {
          fprintf(file, "kp %d %f %f %f %f\n",
                  chains[j].chain[k].frame,
                  chains[j].chain[k].offset[X],
                  chains[j].chain[k].offset[Y],
                  chains[j].chain[k].offset[Z],
                  chains[j].chain[k].offset[W]);
        } else {
          fprintf(file, "kp %d %f %f %f\n",
                  chains[j].chain[k].frame,
                  chains[j].chain[k].offset[X],
                  chains[j].chain[k].offset[Y],
                  chains[j].chain[k].offset[Z]);
        }
      }
    }
    fprintf(file, "\n");
  }

  fclose(file);
  return 0;
}

int write_model_bin(MODEL_DATA *md, char *path) {
  FILE *file = fopen(path, "wb");
  if (!file) {
    return -1;
  }
  size_t total_chains = 0;
  size_t total_keyframes = 0;
  size_t total_frames = 0;
  // Compute totals for animation
  for (size_t i = 0; i < md->num_animations; i++) {
    total_chains += md->animations[i].num_chains;
    total_frames += (md->animations[i].duration *
                     md->animations[i].num_chains);
    for (int j = 0; j < md->animations[i].num_chains; j++) {
      total_keyframes += md->animations[i].keyframe_chains[j].num_frames;
    }
  }

  fwrite(&md->num_bones, sizeof(size_t), 1, file);
  fwrite(&md->num_colliders, sizeof(size_t), 1, file);
  fwrite(&md->num_vertices, sizeof(size_t), 1, file);
  fwrite(&md->num_indices, sizeof(size_t), 1, file);
  fwrite(&md->num_animations, sizeof(size_t), 1, file);

  fwrite(&total_chains, sizeof(size_t), 1, file);
  fwrite(&total_keyframes, sizeof(size_t), 1, file);
  fwrite(&total_frames, sizeof(size_t), 1, file);

  // TODO Implement material saving
  int material_flag = 0;
  fwrite(&material_flag, sizeof(material_flag), 1, file);

  fwrite(md->bones, sizeof(BONE), md->num_bones, file);
  fwrite(md->bone_collider_links, sizeof(int), md->num_bones, file);

  fwrite(md->colliders, sizeof(COLLIDER), md->num_colliders, file);
  fwrite(md->collider_bone_links, sizeof(int), md->num_colliders, file);

  fwrite(md->vertices, sizeof(VBO), md->num_vertices, file);
  fwrite(md->indices, sizeof(int) * 3, md->num_indices, file);

  for (size_t i = 0; i < md->num_animations; i++) {
    fwrite(&(md->animations[i].num_chains), sizeof(size_t), 1, file);
    fwrite(&(md->animations[i].duration), sizeof(size_t), 1, file);
    for (size_t j = 0; j < md->animations[i].num_chains; j++) {
      K_CHAIN cur = md->animations[i].keyframe_chains[j];
      fwrite(&(cur.b_id), sizeof(unsigned int), 1, file);
      fwrite(&(cur.type), sizeof(C_TYPE), 1, file);
      fwrite(&(cur.num_frames), sizeof(size_t), 1, file);
      for (size_t k = 0; k < cur.num_frames; k++) {
        fwrite(cur.chain[k].offset, sizeof(float), 4, file);
        fwrite(&(cur.chain[k].frame), sizeof(int), 1, file);
      }
    }
  }

  fclose(file);
  return 0;
}

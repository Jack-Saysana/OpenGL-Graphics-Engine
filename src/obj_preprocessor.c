#include <obj_preprocessor.h>

int preprocess_lines(LINE_BUFFER *lb) {
  char *bin_path = malloc(strlen(lb->dir) + strlen(lb->filename) + 6);
  if (bin_path == NULL) {
    fprintf(stderr, "Unable to allocate line buffer path\n");
    return -1;
  }
  sprintf(bin_path, "%s/%s.bin", lb->dir, lb->filename);

  printf("%s, %s\n", lb->dir, lb->filename);
  fflush(stdout);

  FILE *file = fopen(bin_path, "wb");
  if (file == NULL) {
    fprintf(stderr, "Unable to open preprocessed file\n");
    free(bin_path);
    free_line_buffer(lb);
    return -1;
  }
  free(bin_path);

  bones = malloc(sizeof(BONE) * BUFF_STARTING_LEN);
  if (bones == NULL) {
    free_line_buffer(lb);
    fclose(file);
    fprintf(stderr, "Unable to allocate bone buffer\n");
    return -1;
  }
  b_buff_len = BUFF_STARTING_LEN;
  b_len = 0;

  bone_ids = malloc(sizeof(int) * 4 * BUFF_STARTING_LEN);
  if (bone_ids == NULL) {
    free_line_buffer(lb);
    fclose(file);
    free(bones);
    fprintf(stderr, "Unable to allocate bone id buffer\n");
    return -1;
  }

  bone_weights = malloc(sizeof(float) * 4 * BUFF_STARTING_LEN);
  if (bone_weights == NULL) {
    free_line_buffer(lb);
    fclose(file);
    free(bones);
    free(bone_ids);
    fprintf(stderr, "Unable to allocate bone weight buffer\n");
    return -1;
  }

  collider_links = malloc(sizeof(int) * 4 * BUFF_STARTING_LEN);
  if (collider_links == NULL) {
    free_line_buffer(lb);
    fclose(file);
    free(bones);
    free(bone_ids);
    free(bone_weights);
    fprintf(stderr, "Unable to allocate collider link buffer\n");
    return -1;
  }

  verticies = malloc(sizeof(float) * 3 * BUFF_STARTING_LEN);
  if (verticies == NULL) {
    free_line_buffer(lb);
    fclose(file);
    free(bones);
    free(bone_ids);
    free(bone_weights);
    free(collider_links);
    fprintf(stderr, "Unable to allocate vertex buffer\n");
    return -1;
  }
  v_buff_len = BUFF_STARTING_LEN;
  v_len = 0;

  normals = malloc(sizeof(float) * 3 * BUFF_STARTING_LEN);
  if (normals == NULL) {
    free_line_buffer(lb);
    fclose(file);
    free(bones);
    free(bone_ids);
    free(bone_weights);
    free(collider_links);
    free(verticies);
    fprintf(stderr, "Unable to allocate normal buffer\n");
    return -1;
  }
  n_buff_len = BUFF_STARTING_LEN;
  n_len = 0;

  tex_coords = malloc(sizeof(float) * 2 * BUFF_STARTING_LEN);
  if (tex_coords == NULL) {
    free_line_buffer(lb);
    fclose(file);
    free(bones);
    free(bone_ids);
    free(bone_weights);
    free(collider_links);
    free(verticies);
    free(normals);
    fprintf(stderr, "Unable to allocate tex coord buffer\n");
    return -1;
  }
  t_buff_len = BUFF_STARTING_LEN;
  t_len = 0;

  vbo_index_combos = malloc(sizeof(int) * 3 * BUFF_STARTING_LEN);
  if (vbo_index_combos == NULL) {
    free_line_buffer(lb);
    fclose(file);
    free(bones);
    free(bone_ids);
    free(bone_weights);
    free(collider_links);
    free(verticies);
    free(normals);
    free(tex_coords);
    fprintf(stderr, "Unable to allocate vbo index combos\n");
    return -1;
  }
  vbo_buff_len = BUFF_STARTING_LEN;
  vbo_len = 0;

  faces = malloc(sizeof(int) * 3 * BUFF_STARTING_LEN);
  if (faces == NULL) {
    free_line_buffer(lb);
    fclose(file);
    free(bones);
    free(bone_ids);
    free(bone_weights);
    free(collider_links);
    free(verticies);
    free(normals);
    free(tex_coords);
    free(vbo_index_combos);
    fprintf(stderr, "Unable to allocate face buffer\n");
    return -1;
  }
  face_buff_len = BUFF_STARTING_LEN;
  f_len = 0;

  materials = malloc(sizeof(MATERIAL) * BUFF_STARTING_LEN);
  if (materials == NULL) {
    free_line_buffer(lb);
    fclose(file);
    free(bones);
    free(bone_ids);
    free(bone_weights);
    free(collider_links);
    free(verticies);
    free(normals);
    free(tex_coords);
    free(vbo_index_combos);
    free(faces);
    fprintf(stderr, "Unable to allocate material buffer\n");
    return -1;
  }
  mat_buff_len = BUFF_STARTING_LEN;
  mat_len = 0;

  animations = malloc(sizeof(MATERIAL) * BUFF_STARTING_LEN);
  if (animations == NULL) {
    free_line_buffer(lb);
    fclose(file);
    free(bones);
    free(bone_ids);
    free(bone_weights);
    free(collider_links);
    free(verticies);
    free(normals);
    free(tex_coords);
    free(vbo_index_combos);
    free(faces);
    free(materials);
    fprintf(stderr, "Unable to allocate animations buffer\n");
    return -1;
  }
  a_buff_len = BUFF_STARTING_LEN;
  a_len = 0;

  colliders = malloc(sizeof(COLLIDER) * BUFF_STARTING_LEN);
  if (colliders == NULL) {
    free_line_buffer(lb);
    fclose(file);
    free(bones);
    free(bone_ids);
    free(bone_weights);
    free(collider_links);
    free(verticies);
    free(normals);
    free(tex_coords);
    free(vbo_index_combos);
    free(faces);
    free(materials);
    free(animations);
    fprintf(stderr, "Unable to allocate colliders buffer\n");
    return -1;
  }
  bone_links = malloc(sizeof(int) * BUFF_STARTING_LEN);
  if (bone_links == NULL) {
    free_line_buffer(lb);
    fclose(file);
    free(bones);
    free(bone_ids);
    free(bone_weights);
    free(collider_links);
    free(verticies);
    free(normals);
    free(tex_coords);
    free(vbo_index_combos);
    free(faces);
    free(materials);
    free(animations);
    free(colliders);
    fprintf(stderr, "Unable to allocate bone_links buffer\n");
    return -1;
  }
  col_buff_len = BUFF_STARTING_LEN;
  col_len = 0;

  size_t total_chains = 0;
  size_t total_keyframes = 0;
  size_t total_frames = 0;

  ANIMATION *cur_anim = NULL;
  size_t cur_anim_buff_len = 0;

  K_CHAIN *cur_chain = NULL;
  size_t cur_chain_buff_len = 0;

  MATERIAL *cur_mat = NULL;
  char *cur_line = NULL;
  int status = 0;
  for (int i = 0; i < lb->len; i++) {
    cur_line = lb->buffer[i];

    if (cur_line[0] == 'f') {
      status = preprocess_face(file, cur_line + 2);
    } else if (cur_line[0] == 'b') {
      memset(bones + b_len, 0, sizeof(BONE));
      sscanf(cur_line, "b %f %f %f \
                          %f %f %f \
                          %f %f %f \
                          %f %f %f \
                          %f %f %f \
                          %d %d",
                          bones[b_len].head,
                          bones[b_len].head + 1,
                          bones[b_len].head + 2,
                          bones[b_len].base,
                          bones[b_len].base + 1,
                          bones[b_len].base + 2,
                          bones[b_len].coordinate_matrix[0],
                          bones[b_len].coordinate_matrix[0] + 1,
                          bones[b_len].coordinate_matrix[0] + 2,
                          bones[b_len].coordinate_matrix[1],
                          bones[b_len].coordinate_matrix[1] + 1,
                          bones[b_len].coordinate_matrix[1] + 2,
                          bones[b_len].coordinate_matrix[2],
                          bones[b_len].coordinate_matrix[2] + 1,
                          bones[b_len].coordinate_matrix[2] + 2,
                          &(bones[b_len].parent),
                          &(bones[b_len].num_children));
      b_len++;
      if (b_len == b_buff_len) {
        size_t old_buff_len = b_buff_len;
        status = double_buffer((void **) &bones, &b_buff_len, sizeof(BONE));
        if (status == 0) {
          status = double_buffer((void **) &collider_links, &old_buff_len,
                                 sizeof(int));
        }
      }
    } else if (cur_line[0] == 'h' && cur_line[1] == 'p' &&
               cur_line[2] == ' ') {
      memset(colliders + col_len, 0, sizeof(COLLIDER));
      colliders[col_len].type = POLY;
      colliders[col_len].children_offset = -1;
      colliders[col_len].num_children = 0;
      sscanf(cur_line, "hp %d %d %d %f %f %f \
                                    %f %f %f \
                                    %f %f %f \
                                    %f %f %f \
                                    %f %f %f \
                                    %f %f %f \
                                    %f %f %f \
                                    %f %f %f",
                                    &(colliders[col_len].category),
                                    bone_links + col_len,
                                    &(colliders[col_len].data.num_used),
                                    colliders[col_len].data.verts[0],
                                    colliders[col_len].data.verts[0]+1,
                                    colliders[col_len].data.verts[0]+2,
                                    colliders[col_len].data.verts[1],
                                    colliders[col_len].data.verts[1]+1,
                                    colliders[col_len].data.verts[1]+2,
                                    colliders[col_len].data.verts[2],
                                    colliders[col_len].data.verts[2]+1,
                                    colliders[col_len].data.verts[2]+2,
                                    colliders[col_len].data.verts[3],
                                    colliders[col_len].data.verts[3]+1,
                                    colliders[col_len].data.verts[3]+2,
                                    colliders[col_len].data.verts[4],
                                    colliders[col_len].data.verts[4]+1,
                                    colliders[col_len].data.verts[4]+2,
                                    colliders[col_len].data.verts[5],
                                    colliders[col_len].data.verts[5]+1,
                                    colliders[col_len].data.verts[5]+2,
                                    colliders[col_len].data.verts[6],
                                    colliders[col_len].data.verts[6]+1,
                                    colliders[col_len].data.verts[6]+2,
                                    colliders[col_len].data.verts[7],
                                    colliders[col_len].data.verts[7]+1,
                                    colliders[col_len].data.verts[7]+2);

      vec3 *verts = colliders[col_len].data.verts;
      vec3 center_of_mass = GLM_VEC3_ZERO_INIT;
      unsigned int num_used = colliders[col_len].data.num_used;
      for (unsigned int i = 0; i < num_used; i++) {
        glm_vec3_add(verts[i], center_of_mass, center_of_mass);
      }
      center_of_mass[0] /= num_used;
      center_of_mass[1] /= num_used;
      center_of_mass[2] /= num_used;
      glm_vec3_copy(center_of_mass, colliders[col_len].data.center_of_mass);

      col_len++;
      if (col_len == col_buff_len) {
        status = double_buffer((void **) &colliders, &col_buff_len,
                               sizeof(COLLIDER));
        if (status == 0) {
          col_buff_len /= 2;
          status = double_buffer((void **) &bone_links, &col_buff_len,
                                 sizeof(int));
        }
      }
    } else if (cur_line[0] == 'h' && cur_line[1] == 's' &&
               cur_line[2] == ' ') {
      memset(colliders + col_len, 0, sizeof(COLLIDER));
      colliders[col_len].type = SPHERE;
      colliders[col_len].children_offset = -1;
      colliders[col_len].num_children = 0;
      sscanf(cur_line, "hs %d %d %f %f %f %f",
             &(colliders[col_len].category),
             bone_links + col_len,
             colliders[col_len].data.center,
             colliders[col_len].data.center + 1,
             colliders[col_len].data.center + 2,
             &(colliders[col_len].data.radius));

      col_len++;
      if (col_len == col_buff_len) {
        status = double_buffer((void **) &colliders, &col_buff_len,
                               sizeof(COLLIDER));
        if (status == 0) {
          col_buff_len /= 2;
          status = double_buffer((void **) &bone_links, &col_buff_len,
                                 sizeof(int));
        }
      }
    } else if (cur_line[0] == 'v' && cur_line[1] == 't' &&
               cur_line[2] == ' ') {
      sscanf(cur_line, "vt %f %f",
            tex_coords[t_len],
            tex_coords[t_len] + 1
          );
      t_len++;
      if (t_len == t_buff_len) {
        status = double_buffer((void **) &tex_coords, &t_buff_len,
                               sizeof(float) * 2);
      }
    } else if (cur_line[0] == 'v' && cur_line[1] == 'n' && cur_line[2] == ' ') {
      sscanf(cur_line, "vn %f %f %f",
            normals[n_len],
            normals[n_len] + 1,
            normals[n_len] + 2
          );
      n_len++;
      if (n_len == n_buff_len) {
        status = double_buffer((void **) &normals, &n_buff_len,
                               sizeof(float) * 3);
      }
    } else if (cur_line[0] == 'v' && cur_line[1] == ' ') {
      sscanf(cur_line, "v %f %f %f %d:%f %d:%f %d:%f %d:%f",
            verticies[v_len],
            verticies[v_len] + 1,
            verticies[v_len] + 2,
            bone_ids[v_len],
            bone_weights[v_len],
            bone_ids[v_len] + 1,
            bone_weights[v_len] + 1,
            bone_ids[v_len] + 2,
            bone_weights[v_len] + 2,
            bone_ids[v_len] + 3,
            bone_weights[v_len] + 3
          );
      v_len++;
      if (v_len == v_buff_len) {
        status = double_buffer((void **) &verticies, &v_buff_len,
                               sizeof(float) * 3);
        if (status == 0) {
          v_buff_len /= 2;
          status = double_buffer((void **) &bone_ids, &v_buff_len,
                                 sizeof(int) * 4);
        }

        if (status == 0) {
          v_buff_len /= 2;
          status = double_buffer((void **) &bone_weights, &v_buff_len,
                                 sizeof(float) * 4);
        }
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
    } else if (cur_line[0] == 'a') {
      cur_anim = animations + a_len;
      memset(cur_anim, 0, sizeof(ANIMATION));
      sscanf(cur_line, "a %ld", &(cur_anim->duration));
      cur_anim->keyframe_chains = malloc(sizeof(K_CHAIN) * BUFF_STARTING_LEN);
      cur_anim->num_chains = 0;
      cur_anim_buff_len = BUFF_STARTING_LEN;
      if (cur_anim->keyframe_chains == NULL) {
        fprintf(stderr, "Unable to allocate keyframe chains for animation\n");
        status = -1;
      }

      if (status == 0) {
        a_len++;
        if (a_len == a_buff_len) {
          status = double_buffer((void **) &animations, &a_buff_len,
                                 sizeof(ANIMATION));
          if (status == 0) {
            cur_anim = animations + a_len - 1;
          }
        }
      }
    } else if (cur_line[0] == 'c' && (cur_line[1] == 'l' || cur_line[1] == 'r'
               || cur_line[1] == 's')) {
      if (cur_anim == NULL) {
        fprintf(stderr, "No animation defined\n");
        status = -1;
      }

      if (status == 0) {
        cur_chain = cur_anim->keyframe_chains + cur_anim->num_chains;
        memset(cur_chain, 0, sizeof(K_CHAIN));

        cur_chain->type = LOCATION;
        if (cur_line[1] == 'r') {
          cur_chain->type = ROTATION;
          sscanf(cur_line, "cr %d", &cur_chain->b_id);
        } else if (cur_line[1] == 's') {
          cur_chain->type = SCALE;
          sscanf(cur_line, "cs %d", &cur_chain->b_id);
        } else {
          sscanf(cur_line, "cl %d", &cur_chain->b_id);
        }

        cur_chain->chain = malloc(sizeof(KEYFRAME) * BUFF_STARTING_LEN);
        cur_chain->num_frames = 0;
        cur_chain_buff_len = BUFF_STARTING_LEN;
        if (cur_chain->chain == NULL) {
          fprintf(stderr, "Unable to allocate keyframes of current chain\n");
          status = -1;
        }
      }

      if (status == 0) {
        (cur_anim->num_chains)++;
        if (cur_anim->num_chains == cur_anim_buff_len) {
          status = double_buffer((void **) &(cur_anim->keyframe_chains),
                                 &cur_anim_buff_len, sizeof(K_CHAIN));
          if (status == 0) {
            cur_chain = cur_anim->keyframe_chains + cur_anim->num_chains - 1;
          }
        }
      }
    } else if (cur_line[0] == 'k' && cur_line[1] == 'p') {
      if (cur_chain == NULL) {
        fprintf(stderr, "No keyframe chain defined\n");
        status = -1;
      }

      if (status == 0) {
        size_t frame_index = cur_chain->num_frames;
        memset(cur_chain->chain + frame_index, 0, sizeof(KEYFRAME));
        if (cur_chain->type == ROTATION) {
          sscanf(cur_line, "kp %d %f %f %f %f",
                 &(cur_chain->chain[frame_index].frame),
                 cur_chain->chain[frame_index].offset,
                 cur_chain->chain[frame_index].offset + 1,
                 cur_chain->chain[frame_index].offset + 2,
                 cur_chain->chain[frame_index].offset + 3);
        } else {
          sscanf(cur_line, "kp %d %f %f %f",
                 &(cur_chain->chain[frame_index].frame),
                 cur_chain->chain[frame_index].offset,
                 cur_chain->chain[frame_index].offset + 1,
                 cur_chain->chain[frame_index].offset + 2);
        }

        (cur_chain->num_frames)++;
        if (cur_chain->num_frames == cur_chain_buff_len) {
          status = double_buffer((void **) &(cur_chain->chain),
                                 &cur_chain_buff_len, sizeof(KEYFRAME));
        }
      }
    }

    if (status != 0) {
      free_line_buffer(lb);
      fclose(file);
      free(bones);
      free(bone_ids);
      free(bone_weights);
      free(collider_links);
      free(verticies);
      free(normals);
      free(tex_coords);
      free(vbo_index_combos);
      free(faces);
      free_materials(materials, mat_len);
      free(colliders);
      free(bone_links);

      for (int i = 0; i < a_len; i++) {
        for (int j = 0; j < animations[i].num_chains; j++) {
          free(animations[i].keyframe_chains[j].chain);
        }
        free(animations[i].keyframe_chains);
      }
      free(animations);

      fprintf(stderr, "Parse error at line %d\n", i);
      return -1;
    }
  }

  // Compute collider links for each bone since bones and colliders aren't
  // neccessariy 1:1
  for (int i = 0; i < b_len; i++) {
    collider_links[i] = -1;
    for (int j = 0; j < col_len; j++) {
      if (bone_links[j] == i) {
        collider_links[i] = j;
        break;
      }
    }
  }
  for (int i = 0; i < b_len; i++) {
    if (collider_links[i] == -1) {
      continue;
    }
    for (int j = 0; j < b_len; j++) {
      if (bones[j].parent == i && collider_links[j] == -1) {
        collider_links[j] = collider_links[i];
      }
    }
  }

  status = sort_colliders();
  if (status != 0) {
    fclose(file);
    free_line_buffer(lb);
    free(bones);
    free(collider_links);
    free(bone_ids);
    free(bone_weights);
    free(verticies);
    free(normals);
    free(tex_coords);
    free(vbo_index_combos);
    free(faces);
    free_materials(materials, mat_len);
    free(colliders);
    free(bone_links);

    for (int i = 0; i < a_len; i++) {
      for (int j = 0; j < animations[i].num_chains; j++) {
        free(animations[i].keyframe_chains[j].chain);
      }
      free(animations[i].keyframe_chains);
    }
    free(animations);
    fprintf(stderr, "Collider sorting error\n");
    return -1;
  }

  // Convert colliders to be given in bone space and "sort" the verticies for
  // polygonal colliders
  vec3 dirs[8] = {
    { 1.0, 1.0, 1.0 },
    { -1.0, 1.0, 1.0 },
    { -1.0, 1.0, -1.0 },
    { 1.0, 1.0, -1.0 },
    { 1.0, -1.0, -1.0 },
    { -1.0, -1.0, -1.0 },
    { -1.0, -1.0, 1.0 },
    { 1.0, -1.0, 1.0}
  };
  vec3 unsorted[8];
  for (int i = 0; i < col_len; i++) {
    int root_bone = bone_links[i];
    if (root_bone != -1 && colliders[i].type == POLY) {
      mat4 entity_to_bone = GLM_MAT4_IDENTITY_INIT;
      glm_mat4_ins3(bones[root_bone].coordinate_matrix, entity_to_bone);
      glm_vec4(colliders[i].data.center_of_mass, 1.0,
               entity_to_bone[3]);
      glm_mat4_inv(entity_to_bone, entity_to_bone);

      // Convert collider verticies to bone space
      for (int j = 0; j < colliders[i].data.num_used; j++) {
        glm_mat4_mulv3(entity_to_bone, colliders[i].data.verts[j], 1.0,
                       colliders[i].data.verts[j]);
      }

    }

    // Sort verticies to the appropriate winding order
    if (colliders[i].type == POLY && colliders[i].data.num_used == 8) {
      for (int j = 0; j < 8; j++) {
        glm_vec3_sub(colliders[i].data.verts[j],
                     colliders[i].data.center_of_mass, unsorted[j]);
        glm_vec3_normalize(unsorted[j]);
      }

      vec3 temp = GLM_VEC3_ZERO_INIT;
      int best = 0;
      for (int j = 0; j < 8; j++) {
        best = max_dot(unsorted, colliders[i].data.num_used, dirs[j]);
        if (best != j) {
          glm_vec3_copy(colliders[i].data.verts[j], temp);
          glm_vec3_copy(colliders[i].data.verts[best],
                        colliders[i].data.verts[j]);
          glm_vec3_copy(temp, colliders[i].data.verts[best]);

          glm_vec3_copy(unsorted[j], temp);
          glm_vec3_copy(unsorted[best], unsorted[j]);
          glm_vec3_copy(temp, unsorted[best]);
        }
      }
    }
  }

  // Compute totals for animation
  for (size_t i = 0; i < a_len; i++) {
    total_chains += animations[i].num_chains;
    total_frames += (animations[i].duration * animations[i].num_chains);
    for (int j = 0; j < animations[i].num_chains; j++) {
      total_keyframes += animations[i].keyframe_chains[j].num_frames;
    }
  }

  int material_flag = cur_mat != NULL;
  int cur_path_len = 0;

  fwrite(&b_len, sizeof(size_t), 1, file);
  fwrite(&col_len, sizeof(size_t), 1, file);
  fwrite(&vbo_len, sizeof(size_t), 1, file);
  fwrite(&f_len, sizeof(size_t), 1, file);

  fwrite(&a_len, sizeof(size_t), 1, file);

  fwrite(&total_chains, sizeof(size_t), 1, file);
  fwrite(&total_keyframes, sizeof(size_t), 1, file);
  fwrite(&total_frames, sizeof(size_t), 1, file);

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

  fwrite(bones, sizeof(BONE), b_len, file);
  fwrite(collider_links, sizeof(int), b_len, file);

  fwrite(colliders, sizeof(COLLIDER), col_len, file);
  fwrite(bone_links, sizeof(int), col_len, file);

  for (size_t i = 0; i < vbo_len; i++) {
    if (vbo_index_combos[i][0] != -1) {
      fwrite(verticies[vbo_index_combos[i][0]], sizeof(float), 3, file);
    } else {
      fwrite(GLM_VEC3_ZERO, sizeof(float), 3, file);
    }
    if (vbo_index_combos[i][2] != -1) {
      fwrite(normals[vbo_index_combos[i][2]], sizeof(float), 3, file);
    } else {
      fwrite(GLM_VEC3_ZERO, sizeof(float), 3, file);
    }
    if (vbo_index_combos[i][1] != -1) {
      fwrite(tex_coords[vbo_index_combos[i][1]], sizeof(float), 2, file);
    } else {
      fwrite((vec2) { 0.0, 0.0 }, sizeof(float), 2, file);
    }
    if (vbo_index_combos[i][0] != -1) {
      fwrite(bone_ids[vbo_index_combos[i][0]], sizeof(int), 4, file);
      fwrite(bone_weights[vbo_index_combos[i][0]], sizeof(float), 4, file);
    } else {
      fwrite((ivec4) { 0, 0, 0, 0 }, sizeof(int), 4, file);
      fwrite(GLM_VEC4_ZERO, sizeof(float), 4, file);
    }
  }
  fwrite(faces, sizeof(int) * 3, f_len, file);

  for (size_t i = 0; i < a_len; i++) {
    fwrite(&(animations[i].num_chains), sizeof(size_t), 1, file);
    fwrite(&(animations[i].duration), sizeof(size_t), 1, file);
    for (size_t j = 0; j < animations[i].num_chains; j++) {
      K_CHAIN cur = animations[i].keyframe_chains[j];
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

  // =============== EXPORTS .OBJ INFO ONLY ===================
  char *simple_bin_path = malloc(strlen(lb->dir) + strlen(lb->filename) + 7);
  sprintf(simple_bin_path, "%s/%s.bin2", lb->dir, lb->filename);
  FILE *simple_file = fopen(simple_bin_path, "wb");
  fwrite(&vbo_len, sizeof(size_t), 1, simple_file);
  fwrite(&f_len, sizeof(size_t), 1, simple_file);
  for (size_t i = 0; i < vbo_len; i++) {
    if (vbo_index_combos[i][0] != -1) {
      fwrite(verticies[vbo_index_combos[i][0]], sizeof(float), 3, simple_file);
    } else {
      fwrite(GLM_VEC3_ZERO, sizeof(float), 3, simple_file);
    }
    if (vbo_index_combos[i][2] != -1) {
      fwrite(normals[vbo_index_combos[i][2]], sizeof(float), 3, simple_file);
    } else {
      fwrite(GLM_VEC3_ZERO, sizeof(float), 3, simple_file);
    }
    if (vbo_index_combos[i][1] != -1) {
      fwrite(tex_coords[vbo_index_combos[i][1]], sizeof(float), 2, simple_file);
    } else {
      fwrite((ivec2) { 0, 0 }, sizeof(float), 2, simple_file);
    }
  }
  fwrite(faces, sizeof(int) * 3, f_len, simple_file);
  fclose(simple_file);
  // ==========================================================

  free_line_buffer(lb);
  free(bones);
  free(collider_links);
  free(bone_ids);
  free(bone_weights);
  free(verticies);
  free(normals);
  free(tex_coords);
  free(vbo_index_combos);
  free(faces);
  free_materials(materials, mat_len);
  free(colliders);
  free(bone_links);

  for (int i = 0; i < a_len; i++) {
    for (int j = 0; j < animations[i].num_chains; j++) {
      free(animations[i].keyframe_chains[j].chain);
    }
    free(animations[i].keyframe_chains);
  }
  free(animations);

  return 0;
}

int sort_colliders() {
  size_t cur_pos = 0;
  // Bring all non-skeletal colliders to the front of the array
  for (size_t cur_col = 0; cur_col < col_len; cur_col++) {
    if (colliders[cur_col].category != HURT_BOX || bone_links[cur_col] == -1) {
      swap_colliders(cur_col, cur_pos);
      cur_pos++;
    }
  }

  // Queue used for traversing and sorting the collider tree in a breadth-first
  // manner (BFS must be used here because the collider array will be expected
  // to be organized in a heap-like manner, requiring a breadth-first ordering)
  size_t front = 0;
  size_t end = 0;
  int dequeued_col = 0;
  int *collider_queue = malloc(sizeof(int) * col_len);
  if (collider_queue == NULL) {
    return -1;
  }

  // Stack used for traversing the sub-bone-tree of each collider in a depth
  // first manner (the usage of DFS vs BFS doesn't actually matter here)
  size_t top = 0;
  int popped_bone = 0;
  int *bone_stack = malloc(sizeof(int) * b_len);
  if (bone_stack == NULL) {
    free(collider_queue);
    return -1;
  }

  // Write sorted collider trees of each parent collider in the armature
  for (size_t cur_bone = 0; cur_bone < b_len; cur_bone++) {
    // Find root bone of root collider
    if (bones[cur_bone].parent == -1) {
      // Add root collider to sorted list
      int cur_col = collider_links[cur_bone];
      swap_colliders(cur_col, cur_pos);
      cur_pos++;

      // Enqueue root bone of collider
      collider_queue[end] = cur_bone;
      end = (end + 1) % col_len;

      while (front != end) {
        // Dequeue collider root to be travered
        dequeued_col = collider_queue[front];
        front = (front + 1) % col_len;

        // Put root bone of collider on the stack
        bone_stack[top] = dequeued_col;
        top++;

        // Traverse the sub-tree extending from the collider's root bone,
        // finding and enqueuing all root bones of child colliders
        while (top) {
          // Pop top of stack
          top--;
          popped_bone = bone_stack[top];
          for (size_t cur_child = 0; cur_child < b_len; cur_child++) {
            // Find all children to popped bone
            if (bones[cur_child].parent == popped_bone) {
              // If child is root bone of a child collider, add the child
              // collider to the sorted list and enqueue the root bone of the
              // child collider
              if (bone_links[collider_links[cur_child]] == cur_child &&
                  collider_links[cur_child] != collider_links[popped_bone]) {
                cur_col = collider_links[cur_child];
                swap_colliders(cur_col, cur_pos);

                // Update child info of popped bone
                int parent_col = collider_links[popped_bone];
                if (colliders[parent_col].children_offset == -1) {
                  colliders[parent_col].children_offset = cur_pos;
                }
                colliders[parent_col].num_children++;

                cur_pos++;

                collider_queue[end] = cur_child;
                end = (end + 1) % col_len;
              } else if (collider_links[cur_child] ==
                         collider_links[popped_bone]) {
                // The child bone is still apart of the current collider, so
                // continue traversing the tree by pushing the child bone onto
                // the stack
                bone_stack[top] = cur_child;
                top++;
              }
            }
          }
        }
      }
    }
  }

  return 0;
}

void swap_colliders(size_t cur, size_t dest) {
  if (cur != dest) {
    COLLIDER temp_collider;
    memcpy(&temp_collider, colliders + dest, sizeof(COLLIDER));
    memcpy(colliders + dest, colliders + cur, sizeof(COLLIDER));
    memcpy(colliders + cur, &temp_collider, sizeof(COLLIDER));

    int temp_bone_link = bone_links[dest];
    bone_links[dest] = bone_links[cur];
    bone_links[cur] = temp_bone_link;

    for (int i = 0; i < b_len; i++) {
      if (collider_links[i] == dest) {
        collider_links[i] = cur;
      } else if (collider_links[i] == cur) {
        collider_links[i] = dest;
      }
    }
  }
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
          fprintf(stderr, "Preprocessor Error: Invalid vertex index\n");
          return -1;
        }
        index_combo[0] = read_index;
      } else if (cur_attrib == 1) {
        if (read_index > t_len - 1) {
          fprintf(stderr, "Preprocessor Error: Invalid tex coord index\n");
          return -1;
        }
        index_combo[1] = read_index;
      } else if(read_index >= 0) {
        fprintf(stderr,
                "Preprocessor Error: Invalid number of vertex attributes\n");
        return -1;
      }
      cur_attrib++;
    } else if (line[i] == ' ' || line[i] == '\0') {
      line[i] = '\0';
      read_index = atoi(cur_num) - 1;
      cur_num = line + i + 1;

      if (cur_attrib == 0) {
        if (read_index > v_len - 1) {
          fprintf(stderr, "Preprocessor Error: Invalid vertex index\n");
          return -1;
        }
        index_combo[0] = read_index;
      } else {
        if (read_index > n_len - 1) {
          fprintf(stderr, "Preprocessor Error:Invalid normal index\n");
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
          fprintf(stderr,
                  "Preproccessor Error: Unable to realloc vbo indicies\n");
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
      status = double_buffer((void **) &faces, &face_buff_len,
                             sizeof(int) * 3);
      if (status != 0) {
        fprintf(stderr, "Unable to reallocate face buffer\n");
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
        status = double_buffer((void **) &faces, &face_buff_len,
                               sizeof(int) * 3);
        if (status != 0) {
          fprintf(stderr, "Unable to reallocate face buffer\n");
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
      fprintf(stderr, "Unable to reallocate face buffer\n");
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

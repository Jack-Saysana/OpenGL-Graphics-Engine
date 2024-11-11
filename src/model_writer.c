#include <model_writer.h>

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
            md->bones[i].head[X], md->bones[i].head[Y], md->bones[i].head[Z],
            md->bones[i].base[X], md->bones[i].base[Y], md->bones[i].base[Z],
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

  // Verticies of a given face may have the same vertex, normal and/or
  // tex coord, so consolidate them before writing to the file
  size_t *v_idx = malloc(sizeof(size_t) * BUFF_STARTING_LEN);
  if (!v_idx) {
    fclose(file);
    return -1;
  }
  size_t v_len = 0;
  size_t v_size = BUFF_STARTING_LEN;

  size_t *t_idx = malloc(sizeof(size_t) * BUFF_STARTING_LEN);
  if (!t_idx) {
    free(v_idx);
    fclose(file);
    return -1;
  }
  size_t t_len = 0;
  size_t t_size = BUFF_STARTING_LEN;

  size_t *n_idx = malloc(sizeof(size_t) * BUFF_STARTING_LEN);
  if (!n_idx) {
    free(t_idx);
    free(v_idx);
    fclose(file);
    return -1;
  }
  size_t n_len = 0;
  size_t n_size = BUFF_STARTING_LEN;

  int status = 0;
  for (size_t i = 0; i < md->num_vertices; i++) {
    // TODO Maybe use a map in the future here for faster searching. However,
    // its not a huge deal for this use case
    int found = 0;
    for (size_t j = 0; j < v_len; j++) {
      if (comp_vec3(md->vertices[v_idx[j]].vertex, md->vertices[i].vertex)) {
        found = 1;
        break;
      }
    }
    if (!found) {
      v_idx[v_len] = i;
      v_len++;
      if (v_len == v_size) {
        status = double_buffer((void **) &v_idx, &v_size, sizeof(size_t));
        if (status) {
          free(n_idx);
          free(t_idx);
          free(v_idx);
          fclose(file);
          return -1;
        }
      }
    }

    found = 0;
    for (size_t j = 0; j < t_len; j++) {
      if (comp_vec2(md->vertices[t_idx[j]].tex_coord,
                    md->vertices[i].tex_coord)) {
        found = 1;
        break;
      }
    }
    if (!found) {
      t_idx[t_len] = i;
      t_len++;
      if (t_len == t_size) {
        status = double_buffer((void **) &t_idx, &t_size, sizeof(size_t));
        if (status) {
          free(n_idx);
          free(t_idx);
          free(v_idx);
          fclose(file);
          return -1;
        }
      }
    }

    found = 0;
    for (size_t j = 0; j < n_len; j++) {
      if (comp_vec3(md->vertices[n_idx[j]].normal, md->vertices[i].normal)) {
        found = 1;
        break;
      }
    }
    if (!found) {
      n_idx[n_len] = i;
      n_len++;
      if (n_len == n_size) {
        status = double_buffer((void **) &n_idx, &n_size, sizeof(size_t));
        if (status) {
          free(n_idx);
          free(t_idx);
          free(v_idx);
          fclose(file);
          return -1;
        }
      }
    }
  }

  // Verticies
  for (size_t i = 0; i < v_len; i++) {
    fprintf(file, "v %f %f %f %d:%f %d:%f %d:%f %d:%f\n",
            md->vertices[v_idx[i]].vertex[X],
            md->vertices[v_idx[i]].vertex[Y],
            md->vertices[v_idx[i]].vertex[Z],
            md->vertices[v_idx[i]].bone_ids[X],
            md->vertices[v_idx[i]].weights[X],
            md->vertices[v_idx[i]].bone_ids[Y],
            md->vertices[v_idx[i]].weights[Y],
            md->vertices[v_idx[i]].bone_ids[Z],
            md->vertices[v_idx[i]].weights[Z],
            md->vertices[v_idx[i]].bone_ids[W],
            md->vertices[v_idx[i]].weights[W]);
  }
  for (size_t i = 0; i < t_len; i++) {
    fprintf(file, "vt %f %f\n",
            md->vertices[t_idx[i]].tex_coord[X],
            md->vertices[t_idx[i]].tex_coord[Y]);
  }
  for (size_t i = 0; i < n_len; i++) {
    fprintf(file, "vn %f %f %f\n",
            md->vertices[n_idx[i]].normal[X],
            md->vertices[n_idx[i]].normal[Y],
            md->vertices[n_idx[i]].normal[Z]);
  }

  // Face indicies
  for (size_t i = 0; i < md->num_indices / 3; i++) {
    fprintf(file, "f ");
    for (int j = 0; j < 3; j++) {
      int vert = md->indices[(i*3)+j];
      int compressed_idx = 0;
      for (size_t k = 0; k < v_len; k++) {
        if (comp_vec3(md->vertices[v_idx[k]].vertex,
                      md->vertices[vert].vertex)) {
          compressed_idx = k;
          break;
        }
      }
      fprintf(file, "%d/", compressed_idx + 1);
      for (size_t k = 0; k < t_len; k++) {
        if (comp_vec2(md->vertices[t_idx[k]].tex_coord,
                      md->vertices[vert].tex_coord)) {
          compressed_idx = k;
          break;
        }
      }
      fprintf(file, "%d/", compressed_idx + 1);
      for (size_t k = 0; k < v_len; k++) {
        if (comp_vec3(md->vertices[n_idx[k]].normal,
                      md->vertices[vert].normal)) {
          compressed_idx = k;
          break;
        }
      }
      fprintf(file, "%d", compressed_idx + 1);
      if (j != 2) {
        fprintf(file, " ");
      }
    }
    fprintf(file, "\n");
  }
  free(v_idx);
  free(t_idx);
  free(n_idx);
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
              verts[0][X], verts[0][Y], verts[0][Z],
              verts[1][X], verts[1][Y], verts[1][Z],
              verts[2][X], verts[2][Y], verts[2][Z],
              verts[3][X], verts[3][Y], verts[3][Z],
              verts[4][X], verts[4][Y], verts[4][Z],
              verts[5][X], verts[5][Y], verts[5][Z],
              verts[6][X], verts[6][Y], verts[6][Z],
              verts[7][X], verts[7][Y], verts[7][Z]);
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
        fprintf(file, "cl %d\n", chains[j].b_id - 1);
      } else if (chains[j].type == ROTATION) {
        fprintf(file, "cr %d\n", chains[j].b_id - 1);
      } else {
        fprintf(file, "cs %d\n", chains[j].b_id - 1);
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

#include <model.h>

void draw_model(unsigned int shader, MODEL *model) {
  if (model == NULL) {
    return;
  }

  int uniform_loc = -1;
  char *uniform_names[5] = { "material.amb_map", "material.diff_map",
                             "material.spec_map", "material.spec_exponent",
                             "material.bump_map" };

  for (int i = 0; i < NUM_PROPS; i++) {
    glActiveTexture(GL_TEXTURE0 + i);
    glBindTexture(GL_TEXTURE_2D, model->textures[i]);
    uniform_loc = glGetUniformLocation(shader, uniform_names[i]);
    if (uniform_loc != -1) {
      glUniform1i(uniform_loc, i);
    }
  }

  glBindVertexArray(model->VAO);
  glDrawElements(GL_TRIANGLES, model->num_indicies, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}

void draw_bones(MODEL *model) {
  if (model == NULL) {
    return;
  }

  L_VBO *bone_data = malloc(sizeof(L_VBO) * model->num_bones * 2);
  for (int i = 0; i < model->num_bones; i++) {
    // bone vertices
    bone_data[(i*2)].coords[0] = model->bones[i].base[0];
    bone_data[(i*2)].coords[1] = model->bones[i].base[1];
    bone_data[(i*2)].coords[2] = model->bones[i].base[2];
    bone_data[(i*2)].id = i;

    bone_data[(i*2)+1].coords[0] = model->bones[i].head[0];
    bone_data[(i*2)+1].coords[1] = model->bones[i].head[1];
    bone_data[(i*2)+1].coords[2] = model->bones[i].head[2];
    bone_data[(i*2)+1].id = i;
  }

  draw_lines(bone_data, model->num_bones);
  free(bone_data);
}

void draw_axes(unsigned int shader, MODEL *model) {
  if (model == NULL) {
    return;
  }

  L_VBO *axis_data = malloc(sizeof(L_VBO) * 2 * model->num_bones);
  // One draw call for each coordinate axis (red x, green y, blue z)
  // Could theoretically do this in one draw call with a separate shader, but
  // this allows us to render the bone axes using the same shader as the bones
  for (int j = 0; j < 3; j++) {
    for (int i = 0; i < model->num_bones; i++) {
      axis_data[(i*2)].coords[0] = model->bones[i].base[0];
      axis_data[(i*2)].coords[1] = model->bones[i].base[1];
      axis_data[(i*2)].coords[2] = model->bones[i].base[2];
      axis_data[(i*2)].id = i;

      vec3 axis_head = GLM_VEC3_ZERO_INIT;
      glm_vec3_copy(model->bones[i].coordinate_matrix[j], axis_head);
      glm_vec3_scale(axis_head, 0.1, axis_head);
      glm_vec3_add(axis_data[i*2].coords, axis_head,
                   axis_data[(i*2)+1].coords);
      axis_data[(i*2)+1].id = i;
    }
    vec3 col = GLM_VEC3_ZERO_INIT;
    col[j] = 1.0;
    set_vec3("col", col, shader);

    draw_lines(axis_data, model->num_bones);
  }
  free(axis_data);
}

void free_model(MODEL *model) {
  if (model == NULL) {
    return;
  }

  if (model->VAO != INVALID_INDEX) {
    glDeleteVertexArrays(1, &(model->VAO));
  }
  glDeleteBuffers(1, &(model->VBO));
  glDeleteBuffers(1, &(model->EBO));
  free(model->k_chain_block);
  free(model->keyframe_block);
  free(model->sled_block);
  free(model->animations);
  free(model->bones);
  free(model->bone_collider_links);
  free(model->colliders);
  free(model->collider_bone_links);
  free(model);
}

#include <animation.h>

int animate(ENTITY *entity, unsigned int animation_index, unsigned int frame) {
  if (entity == NULL) {
    fprintf(stderr, "Invalid animated entity\n");
    return -1;
  }

  if (animation_index >= entity->model->num_animations) {
    fprintf(stderr, "Invalid animation target\n");
    return -1;
  }

  ANIMATION animation = entity->model->animations[animation_index];

  for (int i = 0; i < entity->model->num_bones; i++) {
    glm_mat4_identity(entity->bone_mats[i][0]);
    glm_mat4_identity(entity->bone_mats[i][1]);
    glm_mat4_identity(entity->bone_mats[i][2]);
  }

  apply_animation(entity, animation, frame);
  apply_bone_mats(entity);

  return 0;
}

int blend_anim(ENTITY *entity, unsigned int a1_idx, unsigned int a2_idx,
               unsigned int f1, unsigned int f2, float ratio) {
  if (entity == NULL) {
    fprintf(stderr, "Invalid animated entity\n");
    return -1;
  }

  if (a1_idx >= entity->model->num_animations ||
      a2_idx >= entity->model->num_animations) {
    fprintf(stderr, "Invalid animation target\n");
    return -1;
  }

  ANIMATION a1 = entity->model->animations[a1_idx];
  ANIMATION a2 = entity->model->animations[a2_idx];

  for (int i = 0; i < entity->model->num_bones; i++) {
    glm_mat4_identity(entity->bone_mats[i][0]);
    glm_mat4_identity(entity->bone_mats[i][1]);
    glm_mat4_identity(entity->bone_mats[i][2]);
  }

  apply_blend(entity, a1, a2, f1, f2, ratio);
  apply_bone_mats(entity);

  return 0;
}

int list_animate(ENTITY *entity, A_INPUT *anims, size_t num_anims) {
  if (entity == NULL) {
    fprintf(stderr, "Invalid animated entity\n");
    return -1;
  }

  if (!anims) {
    fprintf(stderr, "Invalid animation target(s)\n");
    return -1;
  }

  for (int i = 0; i < entity->model->num_bones; i++) {
    glm_mat4_identity(entity->bone_mats[i][0]);
    glm_mat4_identity(entity->bone_mats[i][1]);
    glm_mat4_identity(entity->bone_mats[i][2]);
  }

  for (int i = num_anims - 1; i >= 0; i--) {
    if (anims[i].type == A_TYPE_SINGLE) {
      apply_animation(entity, entity->model->animations[anims[i].index],
                      anims[i].frame);
    } else {
      apply_blend(entity, entity->model->animations[anims[i].a1],
                  entity->model->animations[anims[i].a2], anims[i].f1,
                  anims[i].f2, anims[i].ratio);
    }
  }
  apply_bone_mats(entity);

  return 0;
}

void apply_animation(ENTITY *entity, ANIMATION animation, unsigned int frame) {
  for (int i = 0; i < animation.num_chains; i++) {
    K_CHAIN cur_chain = animation.keyframe_chains[i];
    int prev = cur_chain.num_frames - 1;
    if (frame < animation.duration) {
      prev = cur_chain.sled[frame];
    }

    if (prev != -1 && prev < cur_chain.num_frames - 1) {
      calc_bone_mats(entity->bone_mats, cur_chain.b_id, cur_chain.type,
                     frame, cur_chain.chain + prev,
                     cur_chain.chain + (prev + 1));
    } else if (prev != -1 && cur_chain.type == ROTATION) {
      versor quat = GLM_QUAT_IDENTITY_INIT;
      glm_quat_init(quat, cur_chain.chain[prev].offset[0],
                    cur_chain.chain[prev].offset[1],
                    cur_chain.chain[prev].offset[2],
                    cur_chain.chain[prev].offset[3]);
      glm_quat_normalize(quat);
      glm_quat_mat4(quat, entity->bone_mats[cur_chain.b_id][cur_chain.type]);
    } else if (prev != -1) {
      vec3 offset = { cur_chain.chain[prev].offset[0],
                      cur_chain.chain[prev].offset[1],
                      cur_chain.chain[prev].offset[2] };
      if (cur_chain.type == LOCATION) {
        glm_translate(entity->bone_mats[cur_chain.b_id][cur_chain.type],
                      offset);
      } else {
        glm_scale(entity->bone_mats[cur_chain.b_id][cur_chain.type],
                  offset);
      }
    }
  }
}

void apply_blend(ENTITY *entity, ANIMATION a1, ANIMATION a2,
                 unsigned int f1, unsigned int f2, float ratio) {
  K_CHAIN c1;
  K_CHAIN c2;
  KEYFRAME *temp = NULL;
  KEYFRAME k_default_l = { GLM_VEC4_ZERO_INIT, 0 };
  KEYFRAME k_default_r = { GLM_QUAT_IDENTITY_INIT, 0 };
  KEYFRAME k_default_s = { { 1, 1, 1, 0 }, 0 };
  int found = 0;
  int cur_kframe = 0;
  for (int i = 0; i < a1.num_chains; i++) {
    c1 = a1.keyframe_chains[i];
    cur_kframe = c1.num_frames - 1;
    if (f1 < a1.duration) {
      cur_kframe = c1.sled[f1];
    }
    temp = c1.chain + cur_kframe;

    found = 0;
    for (int j = 0; j < a2.num_chains; j++) {
      c2 = a2.keyframe_chains[j];
      if (c1.b_id == c2.b_id && c1.type == c2.type) {
        cur_kframe = c2.num_frames - 1;
        if (f2 < a2.duration) {
          cur_kframe = c2.sled[f2];
        }

        // Interpolate keyframes
        blend_keyframes(entity->bone_mats, c1.b_id, c1.type, temp,
                        c2.chain + cur_kframe, ratio);

        found = 1;
        break;
      }
    }
    if (!found) {
      // Interpolate c1 with identity
      if (c1.type == LOCATION) {
        blend_keyframes(entity->bone_mats, c1.b_id, c1.type, temp,
                        &k_default_l, ratio);
      } else if (c1.type == ROTATION) {
        blend_keyframes(entity->bone_mats, c1.b_id, c1.type, temp,
                        &k_default_r, ratio);
      } else {
        blend_keyframes(entity->bone_mats, c1.b_id, c1.type, temp,
                        &k_default_s, ratio);
      }
    }
  }
  for (int i = 0; i < a2.num_chains; i++) {
    c2 = a2.keyframe_chains[i];
    cur_kframe = c2.num_frames - 1;
    if (f2 < a2.duration) {
      cur_kframe = c2.sled[f2];
    }
    temp = c2.chain + cur_kframe;

    found = 0;
    for (int j = 0; j < a1.num_chains; j++) {
      c1 = a1.keyframe_chains[j];
      if (c1.b_id == c2.b_id && c1.type == c2.type) {
        found = 1;
        break;
      }
    }
    if (!found) {
      // Interpolate c2 with identitiy
      if (c2.type == LOCATION) {
        blend_keyframes(entity->bone_mats, c2.b_id, c2.type, &k_default_l,
                        temp, ratio);
      } else if (c2.type == ROTATION) {
        blend_keyframes(entity->bone_mats, c2.b_id, c2.type, &k_default_r,
                        temp, ratio);
      } else {
        blend_keyframes(entity->bone_mats, c2.b_id, c2.type, &k_default_s,
                        temp, ratio);
      }
    }
  }
}

void calc_bone_mats(mat4 (*bone_mats)[3], unsigned int bone_id, C_TYPE type,
                    unsigned int frame, KEYFRAME *prev, KEYFRAME *next) {
  float ratio = (float) (frame - prev->frame) /
                (float) (next->frame - prev->frame);
  blend_keyframes(bone_mats, bone_id, type, prev, next, ratio);
}

void blend_keyframes(mat4 (*dest)[3], unsigned int bone_id, C_TYPE type,
                     KEYFRAME *prev, KEYFRAME *next, float ratio) {
  mat4 temp = GLM_MAT4_IDENTITY_INIT;
  if (type == ROTATION) {
    versor quat_next = GLM_QUAT_IDENTITY_INIT;
    versor quat_prev = GLM_QUAT_IDENTITY_INIT;
    versor quat = GLM_QUAT_IDENTITY_INIT;

    glm_quat_init(quat_next, next->offset[0], next->offset[1], next->offset[2],
                  next->offset[3]);
    glm_quat_init(quat_prev, prev->offset[0], prev->offset[1], prev->offset[2],
                  prev->offset[3]);

    glm_quat_slerp(quat_prev, quat_next, ratio, quat);

    glm_quat_mat4(quat, temp);
  } else {
    vec3 offset_next = { next->offset[0], next->offset[1], next->offset[2] };
    vec3 offset_prev = { prev->offset[0], prev->offset[1], prev->offset[2] };

    vec3 offset_lerp = GLM_VEC3_ZERO_INIT;
    glm_vec3_lerp(offset_prev, offset_next, ratio, offset_lerp);

    if (type == LOCATION) {
      glm_translate(temp, offset_lerp);
    } else {
      glm_scale(temp, offset_lerp);
    }
  }
  glm_mat4_copy(temp, dest[bone_id][type]);
}

void apply_bone_mats(ENTITY *entity) {
  int parent = -1;
  mat4 (*bone_mats)[3] = entity->bone_mats;
  mat4 *final_mats = entity->final_b_mats;

  vec3 temp = { 0.0, 0.0, 0.0 };
  mat4 to_bone = GLM_MAT4_IDENTITY_INIT;
  mat4 from_bone = GLM_MAT4_IDENTITY_INIT;
  for (size_t i = 0; i < entity->model->num_bones; i++) {
    parent = entity->model->bones[i].parent;
    glm_vec3_negate_to(entity->model->bones[i].base, temp);
    glm_translate(to_bone, entity->model->bones[i].base);
    glm_translate(from_bone, temp);

    glm_mat4_mul(from_bone, bone_mats[i][SCALE], final_mats[i]);
    glm_mat4_mul(bone_mats[i][ROTATION], final_mats[i], final_mats[i]);
    glm_mat4_mul(to_bone, final_mats[i], final_mats[i]);
    glm_mat4_mul(bone_mats[i][LOCATION], final_mats[i], final_mats[i]);

    glm_mat4_identity(to_bone);
    glm_mat4_identity(from_bone);

    if (parent != -1) {
      glm_mat4_mul(final_mats[parent], final_mats[i], final_mats[i]);
    }
  }
}

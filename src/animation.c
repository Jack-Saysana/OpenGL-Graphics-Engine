#include <animation.h>

int animate(ENTITY *entity, unsigned int animation_index, unsigned int frame) {
  if (entity == NULL) {
    printf("Invalid animated entity\n");
    return -1;
  }

  if (animation_index >= entity->model->num_animations) {
    printf("Invalid animation target\n");
    return -1;
  }
  ANIMATION animation = entity->model->animations[animation_index];

  for (int i = 0; i < entity->model->num_bones; i++) {
    glm_mat4_identity(entity->bone_mats[i][0]);
    glm_mat4_identity(entity->bone_mats[i][1]);
    glm_mat4_identity(entity->bone_mats[i][2]);
  }

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

  int parent = -1;
  mat4 (*bone_mats)[3] = entity->bone_mats;
  mat4 *final_mats = entity->final_b_mats;

  vec3 temp = { 0.0, 0.0, 0.0 };
  mat4 to_bone = GLM_MAT4_IDENTITY_INIT;
  mat4 from_bone = GLM_MAT4_IDENTITY_INIT;
  for (size_t i = 0; i < entity->model->num_bones; i++) {
    parent = entity->model->bones[i].parent;
    glm_vec3_negate_to(entity->model->bones[i].coords, temp);
    glm_translate(to_bone, entity->model->bones[i].coords);
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

  return 0;
}

void calc_bone_mats(mat4 (*bone_mats)[3], unsigned int bone_id, C_TYPE type,
                    unsigned int frame, KEYFRAME *prev, KEYFRAME *next) {
  float ratio = (float) (frame - prev->frame) / (float) (next->frame - prev->frame);

  if (type == ROTATION) {
    versor quat_next = GLM_QUAT_IDENTITY_INIT;
    versor quat_prev = GLM_QUAT_IDENTITY_INIT;
    versor quat = GLM_QUAT_IDENTITY_INIT;

    glm_quat_init(quat_next, next->offset[0], next->offset[1], next->offset[2],
                  next->offset[3]);
    glm_quat_init(quat_prev, prev->offset[0], prev->offset[1], prev->offset[2],
                  prev->offset[3]);

    glm_quat_slerp(quat_prev, quat_next, ratio, quat);

    glm_quat_mat4(quat, bone_mats[bone_id][type]);
  } else {
    vec3 offset_next = { next->offset[0], next->offset[1], next->offset[2] };
    vec3 offset_prev = { prev->offset[0], prev->offset[1], prev->offset[2] };

    vec3 offset_lerp = GLM_VEC3_ZERO_INIT;
    glm_vec3_lerp(offset_prev, offset_next, ratio, offset_lerp);

    if (type == LOCATION) {
      glm_translate(bone_mats[bone_id][type], offset_lerp);
    } else {
      glm_scale(bone_mats[bone_id][type], offset_lerp);
    }
  }
}

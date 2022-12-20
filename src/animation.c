#include <animation.h>

int animate(MODEL *model, unsigned int animation_index, unsigned int frame) {
  if (model == NULL) {
    printf("Invalid animated model\n");
    return -1;
  }

  if (animation_index >= model->num_animations) {
    printf("Invalid animation target\n");
    return -1;
  }
  ANIMATION animation = model->animations[animation_index];

  for (int i = 0; i < model->num_bones; i++) {
    glm_mat4_identity(model->bone_mats[i][0]);
    glm_mat4_identity(model->bone_mats[i][1]);
    glm_mat4_identity(model->bone_mats[i][2]);
  }

  for (int i = 0; i < animation.num_chains; i++) {
    K_CHAIN cur_chain = animation.keyframe_chains[i];
    int prev = cur_chain.num_frames - 1;
    if (frame < animation.duration) {
      prev = cur_chain.sled[frame];
    }
    if (prev != -1 && prev < cur_chain.num_frames - 1) {
      calc_bone_mats(model->bone_mats, cur_chain.b_id, cur_chain.type,
                     frame, cur_chain.chain + prev,
                     cur_chain.chain + (prev + 1));
    } else if (prev != -1 && cur_chain.type == ROTATION) {
      versor quat = GLM_QUAT_IDENTITY_INIT;
      glm_quat_init(quat, cur_chain.chain[prev].offset[0],
                    cur_chain.chain[prev].offset[1],
                    cur_chain.chain[prev].offset[2],
                    cur_chain.chain[prev].offset[3]);
    } else if (prev != -1) {
      vec3 offset = { cur_chain.chain[prev].offset[0],
                      cur_chain.chain[prev].offset[1],
                      cur_chain.chain[prev].offset[2] };
      if (cur_chain.type == LOCATION) {
        glm_translate(model->bone_mats[cur_chain.b_id][cur_chain.type],
                      offset);
      } else {
        glm_scale(model->bone_mats[cur_chain.b_id][cur_chain.type],
                  offset);
      }
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

#include <stdlib.h>
#include <stdio.h>
#include <cglm/quat.h>
#include <cglm/affine.h>
#include <structs/models/entity_str.h>

// ====================== INTERNALLY DEFINED FUNCTIONS =======================

void apply_animation(ENTITY *entity, ANIMATION animation, unsigned int frame);
void blend_keyframes(mat4 (*dest)[3], unsigned int bone_id, C_TYPE type,
                     KEYFRAME *prev, KEYFRAME *next, float ratio);
void calc_bone_mats(mat4 (*bone_mats)[3], unsigned int bone_id, C_TYPE type,
                    unsigned int frame, KEYFRAME *prev, KEYFRAME *next);
void apply_blend(ENTITY *entity, ANIMATION a1, ANIMATION a2, unsigned int f1,
                 unsigned int f2, float ratio);
void apply_bone_mats(ENTITY *entity);

// ====================== EXTERNALLY DEFINED FUNCTIONS =======================

int double_buffer(void **buffer, size_t *buff_size, size_t unit_size);

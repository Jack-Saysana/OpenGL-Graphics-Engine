#include <stdlib.h>
#include <stdio.h>
#include <cglm/quat.h>
#include <cglm/affine.h>
#include <structs/models/entity_str.h>

int animate(ENTITY *entity, unsigned int animation_index, unsigned int frame);
void calc_bone_mats(mat4 (*bone_mats)[3], unsigned int bone_id, C_TYPE type,
                    unsigned int frame, KEYFRAME *prev, KEYFRAME *next);
int double_buffer(void **buffer, size_t *buff_size, size_t unit_size);

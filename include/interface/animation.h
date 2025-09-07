#ifndef __ENGINE_ANIMATION_H__
#define __ENGINE_ANIMATION_H__

#include "./structs/models/entity_str.h"

int animate(ENTITY *entity, unsigned int animation_index, unsigned int frame);
int blend_anim(ENTITY *entity, unsigned int a1_idx, unsigned int a2_idx,
               unsigned int f1, unsigned int f2, float ratio);
int list_animate(ENTITY *entity, A_INPUT *anims, size_t num_anims);

#endif

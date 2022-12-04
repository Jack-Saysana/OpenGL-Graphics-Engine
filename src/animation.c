#include <animation.h>

void free_animations(ANIMATION *animations, size_t a_len) {
  for (int i = 0; i < a_len; i++) {
    for (int j = 0; j < animations[i].num_chains; j++) {
      free(animations[i].keyframe_chains[j].chain);
    }
    free(animations[i].keyframe_chains);
  } 
  free(animations);
}

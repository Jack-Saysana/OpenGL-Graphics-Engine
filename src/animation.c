#include <animation.h>

/*int begin_animation(K_CHAIN **queue, size_t *queue_size, size_t *queue_len,
                    size_t ANIMATION *anim) {

}

int enqueue_chain(K_CHAIN **queue, K_CHAIN *chain, size_t *queue_size,
                  size_t *queue_len) {
  queue[*queue_len] = chain;
  size_t index = *queue_len;
  size_t parent = (index - 1) / 2;
  while (index > 0 && queue[parent]->start_frame < queue[index]->start_frame) {
    queue[index] = queue[parent];
    queue[parent] = chain;
    index = parent;
    parent = (index - 1) / 2;
  }

  *(queue_len)++;
  if (*(queue_len) == *(queue_size)) {
    int status = double_buffer((void **) &queue, queue_size, sizeof(K_CHAIN *));
    if (status != 0) {
      printf("Unable to reallocate chain queue\n");
      return -1;
    }
  }

  return 0;
}

K_CHAIN *dequeue_chain(K_CHAIN **queue, size_t *queue_len) {
  K_CHAIN *chain = queue[0];
  queue[0] = queue[*queue_len];
  (*queue_len)--;

  size_t index = 0;
  size_t left_child = (2 * index) + 1;
  size_t right_child = (2 * index) + 2;
}*/

void free_animations(ANIMATION *animations, size_t a_len) {
  for (int i = 0; i < a_len; i++) {
    for (int j = 0; j < animations[i].num_chains; j++) {
      free(animations[i].keyframe_chains[j].chain);
    }
    free(animations[i].keyframe_chains);
  }
  free(animations);
}

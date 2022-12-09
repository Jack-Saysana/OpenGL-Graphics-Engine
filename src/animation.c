#include <animation.h>

C_QUEUE *begin_animation(ANIMATION *anim) {
  C_QUEUE *queue = malloc(sizeof(C_QUEUE));
  if (queue == NULL) {
    printf("Unable to allocate animation queue\n");
    return NULL;
  }
  queue->buffer = malloc(anim->num_chains * sizeof(K_CHAIN *));
  if (queue->buffer == NULL) {
    printf("Unable to allocate queue buffer\n");
    return NULL;
  }
  queue->queue_len = 0;
  queue->queue_size = anim->num_chains;

  int status = 0;
  for (int i = 0; i < anim->num_chains; i++) {
    status = enqueue_chain(queue, anim->keyframe_chains + i);
    if (status != 0) {
      free_queue(queue);
      return NULL;
    }
  }

  // TEST QUEUE FUNCTIONALITY
  /*for (int i = 0; i < queue->queue_size; i++) {
    if (i >= queue->queue_len) {
      printf("NULL ");
      fflush(stdout);
    } else {
      printf("%d ", (queue->buffer)[i]->start_frame);
      fflush(stdout);
    }
  }
  printf("\n");
  fflush(stdout);

  size_t len = queue->queue_len;
  for (int i = 0; i < len; i++) {
    dequeue_chain(queue);
    for (int j = 0; j < queue->queue_size; j++) {
      if (j >= queue->queue_len) {
        printf("NULL ");
        fflush(stdout);
      } else {
        printf("%d ", (queue->buffer)[j]->start_frame);
        fflush(stdout);
      }
    }
    printf("\n");
  }*/

  return queue;
}

int enqueue_chain(C_QUEUE *queue, K_CHAIN *chain) {
  queue->buffer[queue->queue_len] = chain;
  size_t index = queue->queue_len;
  size_t parent = (index - 1) / 2;
  while (index > 0 && queue->buffer[index]->start_frame <
         queue->buffer[parent]->start_frame) {
    queue->buffer[index] = queue->buffer[parent];
    queue->buffer[parent] = chain;
    index = parent;
    parent = (index - 1) / 2;
  }

  (queue->queue_len)++;
  if (queue->queue_len == queue->queue_size) {
    int status = double_buffer((void **) &(queue->buffer), &(queue->queue_size),
                               sizeof(K_CHAIN *));
    if (status != 0) {
      printf("Unable to reallocate chain queue\n");
      return -1;
    }
  }

  return 0;
}

K_CHAIN *dequeue_chain(C_QUEUE *queue) {
  K_CHAIN *chain = queue->buffer[0];

  if (queue->queue_len >= 1) {
    queue->queue_len--;
    queue->buffer[0] = queue->buffer[queue->queue_len];
    queue->buffer[queue->queue_len] = NULL;

    size_t index = 0;
    size_t left_child = (2 * index) + 1;
    size_t right_child = (2 * index) + 2;
    K_CHAIN *temp = NULL;
    while ((left_child < queue->queue_len &&
            queue->buffer[left_child]->start_frame <
            queue->buffer[index]->start_frame) ||
           (right_child < queue->queue_len &&
            queue->buffer[right_child]->start_frame <
            queue->buffer[index]->start_frame)) {
      if(left_child < queue->queue_len && right_child < queue->queue_len) {
        if (queue->buffer[left_child]->start_frame <
            queue->buffer[right_child]->start_frame) {
          temp = queue->buffer[left_child];
          queue->buffer[left_child] = queue->buffer[index];
        } else {
          temp = queue->buffer[right_child];
          queue->buffer[right_child] = queue->buffer[index];
        }
      } else if (left_child < queue->queue_len) {
        temp = queue->buffer[left_child];
        queue->buffer[left_child] = queue->buffer[index];
      } else {
        temp = queue->buffer[right_child];
        queue->buffer[right_child] = queue->buffer[index];
      }
      queue->buffer[index] = temp;
    }
  }

  return chain;
}

void calc_bone_mats(MODEL *model, mat4 *bone_mats, unsigned int bone_id,
                    unsigned int frame, KEYFRAME *prev, KEYFRAME *next) {
  BONE *stack = malloc(sizeof(BONE) * model->num_bones);
  if (stack == NULL) {
    printf("Unable to allocate stack\n");
    return;
  }
}

void free_queue(C_QUEUE *queue) {
  free(queue->buffer);
  free(queue);
}

void free_animations(ANIMATION *animations, size_t a_len) {
  for (int i = 0; i < a_len; i++) {
    for (int j = 0; j < animations[i].num_chains; j++) {
      free(animations[i].keyframe_chains[j].chain);
    }
    free(animations[i].keyframe_chains);
  }
  free(animations);
}

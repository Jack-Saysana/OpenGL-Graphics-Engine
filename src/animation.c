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
      printf("%d:%d ", queue->buffer[i]->start_frame, queue->buffer[i]->b_id);
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
        printf("%d:%d ", queue->buffer[j]->start_frame, queue->buffer[j]->b_id);
        fflush(stdout);
      }
    }
    printf("\n");
    fflush(stdout);
  }*/

  return queue;
}

int enqueue_chain(C_QUEUE *queue, K_CHAIN *chain) {
  queue->buffer[queue->queue_len] = chain;
  size_t index = queue->queue_len;
  size_t parent = (index - 1) / 2;
  while (index > 0 && (queue->buffer[index]->start_frame <
         queue->buffer[parent]->start_frame ||
         (queue->buffer[index]->start_frame ==
          queue->buffer[parent]->start_frame && queue->buffer[index]->b_id <
          queue->buffer[parent]->b_id))) {
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
    size_t right_child = left_child + 1;

    unsigned int cur_start = -1;
    unsigned int cur_id = -1;
    if (queue->queue_len > 0) {
      cur_start = queue->buffer[index]->start_frame;
      cur_id = queue->buffer[index]->b_id;
    }

    unsigned int left_start = -1;
    unsigned int left_id = -1;
    if (left_child < queue->queue_len) {
      left_start = queue->buffer[left_child]->start_frame;
      left_id = queue->buffer[left_child]->b_id;
    }

    unsigned int right_start = -1;
    unsigned int right_id = -1;
    if (right_child < queue->queue_len) {
      right_start = queue->buffer[right_child]->start_frame;
      right_id = queue->buffer[right_child]->b_id;
    }

    size_t next = -1;
    K_CHAIN *temp = NULL;
    while ((left_child < queue->queue_len && (left_start < cur_start ||
           (left_start == cur_start && left_id < cur_id))) ||
           (right_child < queue->queue_len && (right_start < cur_start ||
           (right_start == cur_start && right_id < cur_id)))) {
      if(left_child < queue->queue_len && right_child < queue->queue_len) {
        if (left_start < right_start || (left_start == right_start &&
            left_id < right_id)) {
          temp = queue->buffer[left_child];
          queue->buffer[left_child] = queue->buffer[index];
          next = left_child;
        } else {
          temp = queue->buffer[right_child];
          queue->buffer[right_child] = queue->buffer[index];
          next = right_child;
        }
      } else if (left_child < queue->queue_len) {
        temp = queue->buffer[left_child];
        queue->buffer[left_child] = queue->buffer[index];
        next = left_child;
      } else {
        temp = queue->buffer[right_child];
        queue->buffer[right_child] = queue->buffer[index];
        next = right_child;
      }
      queue->buffer[index] = temp;

      index = next;
      left_child = (2 * index) + 1;
      right_child = left_child + 1;

      cur_start = queue->buffer[index]->start_frame;
      cur_id = queue->buffer[index]->b_id;

      if (left_child < queue->queue_len) {
        left_start = queue->buffer[left_child]->start_frame;
        left_id = queue->buffer[left_child]->b_id;
      }

      if (right_child < queue->queue_len) {
        right_start = queue->buffer[right_child]->start_frame;
        right_id = queue->buffer[right_child]->b_id;
      }
    }
  }

  return chain;
}

void calc_bone_mats(mat4 (*bone_mats)[3], unsigned int bone_id, C_TYPE type,
                    unsigned int frame, KEYFRAME *prev, KEYFRAME *next) {
  versor quat = GLM_QUAT_IDENTITY_INIT;
  vec4 offset_next = { next->offset[0], next->offset[1], next->offset[2],
                       next->offset[3] };
  vec4 offset_prev = { prev->offset[0], prev->offset[1], prev->offset[2],
                       prev->offset[3] };
  float ratio = (float) (frame - prev->frame) / (float) (next->frame - prev->frame);
  vec4 offset_lerp = GLM_VEC4_ZERO_INIT;
  vec3 offset_vec3_lerp = GLM_VEC3_ZERO_INIT;
  glm_vec4_lerp(offset_prev, offset_next, ratio, offset_lerp);
  glm_vec3(offset_lerp, offset_vec3_lerp);

  if (type == LOCATION) {
    glm_translate(bone_mats[bone_id][type], offset_vec3_lerp);
  } else if (type == ROTATION) {
    quat[0] += offset_lerp[0];
    quat[1] += offset_lerp[1];
    quat[2] += offset_lerp[2];
    quat[3] += offset_lerp[3];
    glm_quat_mat4(quat, bone_mats[bone_id][type]);
  } else if (type == SCALE) {
    glm_scale(bone_mats[bone_id][type], offset_vec3_lerp);
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

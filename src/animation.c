#include <animation.h>

int add_keyframe(ANIMATION *animation, unsigned int id, KEY_TYPE type,
                 unsigned int time, float *offset) {
  ANIM_BONE *bone = animation->bones + id;
  if (bone->attrib_chains[type] != NULL && time <= bone->tails[type]->time) {
    printf("Invalid keyframe time\n");
    return -1;
  }

  FRAME *frame = malloc(sizeof(FRAME));
  if (frame == NULL) {
    printf("Unable to allocate new keyframe\n");
    return -1;
  }
  frame->time = time;
  frame->next = NULL;
  frame->offset[0] = offset[0];
  frame->offset[1] = offset[1];
  frame->offset[2] = offset[2];
  frame->offset[3] = offset[3];

  if (bone->attrib_chains[type] == NULL) {
    bone->attrib_chains[type] = frame;
  } else {
    bone->tails[type]->next = frame;
  }
  bone->tails[type] = frame;

  return 0;
}

void free_animation(ANIMATION *animation) {
  for (int i = 0; i < num_bones; i++) {
    for (int j = LOCATION; j <= SCALE; j++) {
      FRAME *cur = animation->bones[i][j];
      FRAME *temp = NULL;
      while (cur != NULL) {
        temp = cur->next;
        free(cur);
        cur = temp;
      }
    }
  }
  free(animation->attrib_chains);
  free(bone_ids);
  free(animation);
}

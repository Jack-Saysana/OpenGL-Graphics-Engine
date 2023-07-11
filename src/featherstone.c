#include <featherstone.h>

int featherstone_abm(ENTITY *body) {
  size_t num_links = body->model->num_colliders;
  COLLIDER *links = body->model->colliders;
  // Calculate spatial velocities from inbound to outbound
  int root_bone = -1;
  int parent = -1;
  for (int i = 0; i < num_links; i++) {
    if (links[i].category != HIT_BOX) {
      continue;
    }

    root_bone = collider_bone_links[i];
    parent = -1;
    if (root_bone != -1) {
      parent = bone_collider_links[body->model->bones[root_bone].parent];
    }

  }
  // Calculate I-hat and Z-hat from inbound to outbound
  // Calculate I-hat-A and Z-hat-A from outbound to inbound
  // Calculate q** and spatial acceleration from inbound to outbound
}

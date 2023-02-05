#include <simulation.h>

int init_simulation() {
  if (dynamic_ents != NULL || static_ents != NULL) {
    printf("Simulation already initialized."\
           "Simulation initialization failed\n");
    return -1;
  }

  dynamic_ents = malloc(sizeof(ENTITY *) * BUFF_STARTING_LEN);
  if (dynamic_ents == NULL) {
    printf("Failed to allocate dynamic entities buffer. "\
           "Simulation initialization failed\n");
    return -1;
  }

  static_ents = malloc(sizeof(ENTITY *) * BUFF_STARTING_LEN);
  if (static_ents == NULL) {
    free(dynamic_ents);
    printf("Failed to allocate static entities buffer. "\
           "Simulation initialization failed\n");
    return -1;
  }

  physics_tree = init_tree();
  if (physics_tree == NULL) {
    free(dynamic_ents);
    free(static_ents);
    printf("Failed to initialized physics oct-tree."\
           "Simulation initialization failed\n");
    return -1;
  }

  d_ent_buff_len = 0;
  d_ent_buff_size = BUFF_STARTING_LEN;
  s_ent_buff_len = 0;
  s_ent_buff_size = BUFF_STARTING_LEN;

  return 0;
}

int insert_entity(ENTITY *entity) {
  if (entity == NULL || dynamic_ents == NULL || static_ents == NULL) {
    return -1;
  }

  int status = 0;
  if (entity->velocity[0] != 0.0 || entity->velocity[1] != 0.0 ||
      entity->velocity[2] != 0.0) {
    dynamic_ents[d_ent_buff_len] = entity;
    d_ent_buff_len++;
    if (d_ent_buff_len == d_ent_buff_size) {
      status = double_buffer((void **) &dynamic_ents, &d_ent_buff_size,
                             sizeof(ENTITY *));
      if (status != 0) {
        printf("Unable to reallocate dynamic entity buffer\n");
        end_simulation();
        return -1;
      }
    }
  } else {
    static_ents[s_ent_buff_len] = entity;
    s_ent_buff_len++;
    if (s_ent_buff_len == s_ent_buff_size) {
      status = double_buffer((void **) &static_ents, &s_ent_buff_size,
                             sizeof(ENTITY *));
      if (status != 0) {
        printf("Unable to reallocate static entity buffer\n");
        end_simulation();
        return -1;
      }
    }
  }

  for (int i = 0; i < entity->model->num_colliders; i++) {
    status = oct_tree_insert(physics_tree, entity, i);
    if (status != 0) {
      printf("Unable to insert entity into physics oct-tree\n");
      end_simulation();
      return -1;
    }
  }

  return 0;
}

void end_simulation() {
  free(dynamic_ents);
  free(static_ents);
  free_oct_tree(physics_tree);

  dynamic_ents = NULL;
  static_ents = NULL;
  physics_tree = NULL;
}

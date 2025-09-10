#ifndef __ENGINE_SIM_LEDGER_STR_H__
#define __ENGINE_SIM_LEDGER_STR_H__

typedef struct simulation_collider {
  ENTITY *entity;
  void *data;
  size_t collider_offset;
  size_t index;
  int status;
  int to_delete;
} SIM_COLLIDER;

typedef struct simulation_entity {
  ENTITY *entity;
  void *data;
  size_t index;
  int status;
  int to_delete;
} SIM_ENTITY;

typedef union simulation_item {
  SIM_COLLIDER col;
  SIM_ENTITY ent;
} SIM_ITEM;

typedef struct simulation_ledger {
  SIM_ITEM *map;
  size_t *list;
  size_t num_items;
  size_t map_size;
  size_t list_size;
} SIM_LEDGER;

typedef union ledger_input {
  struct c_data{
    ENTITY *ent;
    size_t col;
    void *data;
  } collider;
  struct e_data {
    ENTITY *ent;
    void *data;
  } entity;
} LEDGER_INPUT;

#endif

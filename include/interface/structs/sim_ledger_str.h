#ifndef __SIM_LEDGER_STR_H__
#define __SIM_LEDGER_STR_H__

typedef struct simulation_collider {
  ENTITY *entity;
  void *data;
  size_t collider_offset;
  size_t index;
  int status;
  int to_delete;
} SIM_COLLIDER;

typedef struct simulation_collider_2d {
  ENTITY_2D *entity;
  void *data;
  size_t collider_offset;
  size_t index;
  int status;
  int to_delete;
} SIM_COLLIDER_2D;

typedef struct simulation_entity {
  ENTITY *entity;
  void *data;
  size_t index;
  int status;
  int to_delete;
} SIM_ENTITY;

typedef struct simulation_entity_2d {
  ENTITY_2D *entity;
  void *data;
  size_t index;
  int status;
  int to_delete;
} SIM_ENTITY_2D;

typedef union simulation_item {
  SIM_COLLIDER col;
  SIM_COLLIDER_2D col_2d;
  SIM_ENTITY ent;
  SIM_ENTITY_2D ent_2d;
} SIM_ITEM;

typedef struct simulation_ledger {
  SIM_ITEM *map;
  size_t *list;
  size_t num_items;
  size_t map_size;
  size_t list_size;
} SIM_LEDGER;

typedef union ledger_input {
  struct c_data {
    ENTITY *ent;
    size_t col;
    void *data;
  } collider;
  struct e_data {
    ENTITY *ent;
    void *data;
  } entity;
  struct c_data_2d {
    ENTITY_2D *ent;
    size_t col;
    void *data;
  } collider_2d;
  struct e_data_2d {
    ENTITY_2D *ent;
    void *data;
  } entity_2d;
} LEDGER_INPUT;

#endif

#include <sim_ledger.h>

int ledger_init(SIM_LEDGER *ledger) {
  ledger->map = malloc(sizeof(SIM_ITEM) * HASH_MAP_STARTING_LEN);
  if (ledger->map == NULL) {
    goto MAP_ERR;
  }
  memset(ledger->map, 0, sizeof(SIM_ITEM) * HASH_MAP_STARTING_LEN);

  ledger->list = malloc(sizeof(size_t) * BUFF_STARTING_LEN);
  if(ledger->list == NULL) {
    goto LIST_ERR;
  }

  ledger->num_items = 0;
  ledger->map_size = HASH_MAP_STARTING_LEN;
  ledger->list_size = BUFF_STARTING_LEN;
  return 0;

LIST_ERR:
  free(ledger->map);
MAP_ERR:
  return -1;
}

void free_ledger(SIM_LEDGER *ledger) {
  free(ledger->map);
  free(ledger->list);
}

size_t hash_item(double key, size_t i, size_t size) {
  size_t ret = size * ((key * HASH_MAGIC_NUM) - floor(key * HASH_MAGIC_NUM));
  return (ret + i) % size;
}

int ledger_add(SIM_LEDGER *ledger, LEDGER_INPUT l_data, int l_type) {
  size_t index = ledger_search(ledger, l_data, l_type);
  if (index != INVALID_INDEX) {
    return 0;
  }

  size_t i = 0;
  double key = 0.0;
  while (1) {
    if (l_type == L_TYPE_ENTITY) {
      key = (size_t) l_data.entity.ent;
    } else {
      key = ((size_t) l_data.collider.ent) + l_data.collider.col;
    }
    index = hash_item(key, i, ledger->map_size);
    if (l_type == L_TYPE_ENTITY) {
      if (ledger->map[index].ent.status != LEDGER_OCCUPIED) {
        ledger->map[index].ent.entity = l_data.entity.ent;
        ledger->map[index].ent.data = l_data.entity.data;
        ledger->map[index].ent.index = ledger->num_items;
        ledger->map[index].ent.status = LEDGER_OCCUPIED;
        ledger->map[index].ent.to_delete = 0;
        break;
      }
    } else {
      if (ledger->map[index].col.status != LEDGER_OCCUPIED) {
        ledger->map[index].col.entity = l_data.collider.ent;
        ledger->map[index].col.data = l_data.collider.data;
        ledger->map[index].col.collider_offset = l_data.collider.col;
        ledger->map[index].col.index = ledger->num_items;
        ledger->map[index].col.status = LEDGER_OCCUPIED;
        ledger->map[index].col.to_delete = 0;
        break;
      }
    }
    i++;
  }

  int status = 0;
  ledger->list[ledger->num_items] = index;
  ledger->num_items++;
  if (ledger->num_items == ledger->list_size) {
    status = double_buffer((void **) &ledger->list, &ledger->list_size,
                           sizeof(size_t));
    if (status) {
      if (l_type == L_TYPE_ENTITY) {
        ledger->map[index].ent.status = LEDGER_FREE;
      } else {
        ledger->map[index].col.status = LEDGER_FREE;
      }
      ledger->num_items--;
      return -1;
    }
  }

  double load_factor = ((double) ledger->num_items) / ((double) ledger->map_size);
  if (load_factor > 0.5) {
    status = resize_ledger(ledger, l_type);
    if (status) {
      if (l_type == L_TYPE_ENTITY) {
        ledger->map[index].ent.status = LEDGER_FREE;
      } else {
        ledger->map[index].col.status = LEDGER_FREE;
      }
      ledger->num_items--;
      return -1;
    }
  }

  return 0;
}

size_t ledger_search(SIM_LEDGER *ledger, LEDGER_INPUT l_data, int l_type) {
  double key = 0;
  if (l_type == L_TYPE_ENTITY) {
    key = (size_t) l_data.entity.ent;
  } else {
    key = ((size_t) l_data.collider.ent) + l_data.collider.col;
  }

  size_t i = 0;
  size_t index = 0;
  SIM_ITEM *map = ledger->map;
  while (1) {
    index = hash_item(key, i, ledger->map_size);
    if (l_type == L_TYPE_ENTITY) {
      if (map[index].ent.status == LEDGER_FREE) {
        break;
      } else if (map[index].ent.status == LEDGER_OCCUPIED &&
                 map[index].ent.entity == l_data.entity.ent) {
        return index;
      }
    } else {
      if (map[index].col.status == LEDGER_FREE) {
        break;
      } else if (map[index].col.status == LEDGER_OCCUPIED &&
                 map[index].col.entity == l_data.collider.ent &&
                 map[index].col.collider_offset == l_data.collider.col) {
        return index;
      }
    }
    i++;
  }
  return INVALID_INDEX;
}

void ledger_delete(SIM_LEDGER *ledger, LEDGER_INPUT l_data, int l_type) {
  size_t index = ledger_search(ledger, l_data, l_type);
  SIM_ITEM *map = ledger->map;
  size_t *l_list = ledger->list;
  if (index != INVALID_INDEX) {
    ledger->num_items--;
    if (l_type == L_TYPE_ENTITY) {
      map[index].ent.status = LEDGER_DELETED;
      l_list[map[index].ent.index] = l_list[ledger->num_items];
      map[l_list[ledger->num_items]].ent.index = map[index].ent.index;
    } else {
      map[index].col.status = LEDGER_DELETED;
      l_list[map[index].col.index] = l_list[ledger->num_items];
      map[l_list[ledger->num_items]].col.index = map[index].col.index;
    }
  }
}

void ledger_delete_direct(SIM_LEDGER *ledger, size_t index, int l_type) {
  SIM_ITEM *map = ledger->map;
  size_t *l_list = ledger->list;
  ledger->num_items--;
  if (l_type == L_TYPE_ENTITY) {
    map[l_list[index]].ent.status = LEDGER_DELETED;
    l_list[index] = l_list[ledger->num_items];
    map[l_list[ledger->num_items]].ent.index = index;
  } else {
    map[l_list[index]].col.status = LEDGER_DELETED;
    l_list[index] = l_list[ledger->num_items];
    map[l_list[ledger->num_items]].col.index = index;
  }
}

int resize_ledger(SIM_LEDGER *ledger, int l_type) {
  SIM_ITEM *new_map = malloc(sizeof(SIM_ITEM) * 2 * ledger->map_size);
  if (new_map == NULL) {
    return -1;
  }
  ledger->map_size *= 2;
  memset(new_map, 0, sizeof(SIM_ITEM) * ledger->map_size);

  size_t j = 0;
  size_t index = 0;
  ENTITY *cur_ent = NULL;
  size_t cur_col = 0;
  void *data = NULL;
  int cur_del = 0;
  double key = 0.0;
  size_t *l_list = ledger->list;
  for (size_t i = 0; i < ledger->num_items; i++) {
    if (l_type == L_TYPE_ENTITY) {
      cur_ent = ledger->map[l_list[i]].ent.entity;
      data = ledger->map[l_list[i]].ent.data;
      cur_del = ledger->map[l_list[i]].ent.to_delete;
      key = (size_t) cur_ent;
    } else {
      cur_ent = ledger->map[l_list[i]].col.entity;
      data = ledger->map[l_list[i]].col.data;
      cur_col = ledger->map[l_list[i]].col.collider_offset;
      cur_del = ledger->map[l_list[i]].col.to_delete;
      key = ((size_t) cur_ent) + cur_col;
    }

    j = 0;
    while (1) {
      index = hash_item(key, j, ledger->map_size);
      if (l_type == L_TYPE_ENTITY) {
        if (new_map[index].ent.status != LEDGER_OCCUPIED) {
          new_map[index].ent.entity = cur_ent;
          new_map[index].ent.data = data;
          new_map[index].ent.index = i;
          new_map[index].ent.status = LEDGER_OCCUPIED;
          new_map[index].ent.to_delete = cur_del;
          break;
        }
      } else {
        if (new_map[index].col.status != LEDGER_OCCUPIED) {
          new_map[index].col.entity = cur_ent;
          new_map[index].col.data = data;
          new_map[index].col.collider_offset = cur_col;
          new_map[index].col.index = i;
          new_map[index].col.status = LEDGER_OCCUPIED;
          new_map[index].col.to_delete = cur_del;
          break;
        }
      }
      j++;
    }
    l_list[i] = index;
  }

  free(ledger->map);
  ledger->map = new_map;
  return 0;
}


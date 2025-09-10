#include <stdio.h>
#include <string.h>
#include <structs/models/entity_str.h>
#include <structs/sim_ledger_str.h>

// ====================== INTERNALLY DEFINED FUNCTIONS =======================

size_t ledger_search(SIM_LEDGER *ledger, LEDGER_INPUT l_data, int l_type);
int resize_ledger(SIM_LEDGER *ledger, int l_type);

// ====================== EXTERNALLY DEFINED FUNCTIONS =======================

int double_buffer(void **buffer, size_t *buff_size, size_t unit_size);

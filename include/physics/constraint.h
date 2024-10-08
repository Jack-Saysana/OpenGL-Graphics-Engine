#include <structs/entity_str.h>
#include <structs/physics/constraint_str.h>
#include <structs/math/amat_str.h>

// ====================== INTERNALLY DEFINED FUNCTIONS =======================

void calc_world_accel(ENTITY *, vec3, size_t, vec3);
void calc_force_vec(ENTITY *, vec3, size_t, vec3, vec6);

// ====================== EXTERNALLY DEFINED FUNCTIONS =======================

int featherstone_abm(ENTITY *, vec3);
void solve_system(amat, amat, amat);
void lcp(amat, amat, amat);

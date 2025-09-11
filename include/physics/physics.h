#include <cglm/cglm.h>
#include <globals.h>
#include <structs/models/entity_str.h>
#include <structs/2d/models/entity_2d_str.h>
#include <structs/simulation_str.h>

static vec3 U_DIR = { 0.0, 1.0, 0.0 };
static vec3 D_DIR = { 0.0, -1.0, 0.0 };
static vec3 L_DIR = { 1.0, 0.0, 0.0 };
static vec3 R_DIR = { -1.0, 1.0, 0.0 };
static vec3 F_DIR = { 0.0, 1.0, 1.0 };
static vec3 B_DIR = { 0.0, 1.0, -1.0 };

// ====================== INTERNALLY DEFINED FUNCTIONS =======================

// ====================== EXTERNALLY DEFINED FUNCTIONS =======================

void apply_constraints(ENTITY *ent, J_CONS *cons, size_t num_constr,
                       vec3 gravity);
int featherstone_abm(ENTITY *ent, vec3 grav);
void vec3_remove_noise(vec3 vec, float threshold);
float remove_noise(float val, float threshold);
int max_dot(vec3 *verts, unsigned int len, vec3 dir);

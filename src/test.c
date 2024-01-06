#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <entity_str.h>

#define DEFAULT (0)
#define HIT_BOX (1)
#define HURT_BOX (2)

extern MODEL *cube;
extern MODEL *rect_prism;
extern MODEL *dude;
extern MODEL *test;
extern MODEL *floor_model;
extern MODEL *platform;
extern MODEL *sphere;
extern MODEL *vector;

extern ENTITY *player;
extern ENTITY *obstacle;
extern ENTITY *floor_entity;
extern ENTITY *box_entity;
extern ENTITY *sphere_entity;
extern ENTITY **boxes;
extern ENTITY **spheres;
extern ENTITY **rects;
extern const int NUM_BOXES;
extern const int NUM_SPHERES;
extern const int NUM_RECTS;
extern ENTITY *ragdoll;

vec3 m_box_pos = { 7.0, 4.0, -2.0 };
vec3 m_box_scale = { 0.5, 0.5, 0.5 };
vec3 m_sphere_pos = { -3.0, 3.0, -3.0 };
vec3 m_sphere_scale = { 0.5, 0.5, 0.5 };
vec3 m_rect_pos = { 1.0, 3.0, 3.0 };
vec3 m_rect_scale = { 0.5, 0.5, 0.5 };
vec3 ob_pos = { 3.0, 0.0, -3.0 };
vec3 floor_scale = { 50.0, 1.0, 50.0 };
vec3 cube_pos = { 3.0, 2.0, 3.0 };
vec3 s_pos = { -3.0, 2.0, 3.0 };
vec3 ragdoll_pos = { 0.0, 1.0, -3.0 };

ENTITY *init_entity(MODEL *);
void featherstone_abm(ENTITY *body);
void free_entity(ENTITY *);
void free_model(MODEL *);
int init_scene();
void get_model_mat(ENTITY *entity, mat4 model);
void calc_inertia_tensor(ENTITY *ent, size_t col);
void print_mat6(mat6);
void print_mat3(mat3);
void print_vec6(vec6);
void print_vec3(vec3);

// Not used but defition needed
vec3 col_point = { 0.0, 0.0, 0.0 };
int enable_gravity = 1;

void print_mat4 (mat4 m) {
  for (int i = 0; i < 4; i++) {
    printf("|%.2f %.2f %.2f %.2f|\n",
           m[0][i], m[1][i], m[2][i], m[3][i]);
  }
}

void print_p_data(ENTITY *ent) {
  mat4 global_ent_to_world = GLM_MAT4_IDENTITY_INIT;
  get_model_mat(ent, global_ent_to_world);

  printf("Num colliders: %ld\nNum bones: %ld\n", ent->model->num_colliders,
         ent->model->num_bones);
  printf("Bone -> collider relations:\n");
  for (size_t i = 0; i < ent->model->num_bones; i++) {
    printf("  %ld -> %d\n", i, ent->model->bone_collider_links[i]);
  }
  printf("Collider -> bone relations:\n");
  for (size_t i = 0; i < ent->model->num_colliders; i++) {
    printf("  %ld -> %d\n", i, ent->model->collider_bone_links[i]);
  }
  printf("Physics data:\n");
  for (size_t i = 0; i < ent->model->num_colliders; i++) {
    if (ent->model->colliders[i].category != HURT_BOX) {
      continue;
    }

    int root_bone = ent->model->collider_bone_links[i];

    vec3 cur_coords = GLM_VEC3_ZERO_INIT;
    mat4 cur_ent_to_world = GLM_MAT3_IDENTITY_INIT;
    glm_mat4_mul(ent->final_b_mats[root_bone], global_ent_to_world,
                 cur_ent_to_world);
    if (ent->model->colliders[i].type == POLY) {
      glm_mat4_mulv3(cur_ent_to_world,
                     ent->model->colliders[i].data.center_of_mass, 1.0,
                     cur_coords);
    } else {
      glm_mat4_mulv3(cur_ent_to_world, ent->model->colliders[i].data.center,
                     1.0, cur_coords);
    }

    mat3 cur_bone_to_world_rot = GLM_MAT3_IDENTITY_INIT;
    mat4 cur_bone_to_world = GLM_MAT4_IDENTITY_INIT;
    glm_mat4_ins3(ent->model->bones[root_bone].coordinate_matrix,
                  cur_bone_to_world);
    glm_mat4_mul(cur_ent_to_world, cur_bone_to_world, cur_bone_to_world);
    glm_mat4_pick3(cur_bone_to_world, cur_bone_to_world_rot);

    vec3 cur_joint = GLM_VEC3_ZERO_INIT;
    glm_mat4_mulv3(cur_ent_to_world, ent->model->bones[root_bone].base, 1.0,
                   cur_joint);

    int parent_bone = ent->model->bones[root_bone].parent;

    vec3 axis_of_rot = { 1.0, 0.0, 0.0 };
    glm_mat3_mulv(cur_bone_to_world_rot, axis_of_rot, axis_of_rot);
    glm_vec3_normalize(axis_of_rot);

    printf("  collider[%ld]:\n", i);
    if (parent_bone != -1) {
      printf("    Parent: %d\n", ent->model->bone_collider_links[parent_bone]);
    }
    printf("    axis of rotation: %f %f %f\n", axis_of_rot[0], axis_of_rot[1],
           axis_of_rot[2]);
    printf("    COM:\n");
    printf("    %f %f %f\n", cur_coords[0], cur_coords[1], cur_coords[2]);
    printf("    joint:\n");
    printf("    %f %f %f\n", cur_joint[0], cur_joint[1], cur_joint[2]);
    printf("    to parent:\n");
    print_mat6(ent->np_data[i].ST_to_parent);
    printf("    from parent:\n");
    print_mat6(ent->np_data[i].ST_from_parent);
    printf("    I_hat:\n");
    print_mat6(ent->np_data[i].I_hat);
    printf("    Z_hat:\n");
    print_vec6(ent->np_data[i].Z_hat);
    printf("    s_hat:\n");
    print_vec6(ent->np_data[i].s_hat[0]);
    printf("    Coriolis:\n");
    print_vec6(ent->np_data[i].coriolis_vector[0]);
    printf("    v-hat:\n");
    print_vec6(ent->np_data[i].v_hat);
    printf("    a-hat:\n");
    print_vec6(ent->np_data[i].a_hat);
    printf("\n");
  }
}

void sa_test() {
  mat6 m1 = {
    {1.0, 7.0,  13.0, 19.0, 25.0, 31.0},
    {2.0, 8.0,  14.0, 20.0, 26.0, 32.0},
    {3.0, 9.0,  15.0, 21.0, 27.0, 33.0},
    {4.0, 10.0, 16.0, 22.0, 28.0, 34.0},
    {5.0, 11.0, 17.0, 23.0, 29.0, 35.0},
    {6.0, 12.0, 18.0, 24.0, 30.0, 36.0}
  };
  printf("m1:\n");
  print_mat6(m1);
  printf("\n");


  mat6 copy = MAT6_ZERO_INIT;
  mat6_copy(m1, copy);
  printf("copy:\n");
  print_mat6(copy);
  printf("\n");

  mat6_zero(copy);
  printf("zeroed copy:\n");
  print_mat6(copy);
  printf("\n");

  mat3 m2 = {
    {1.0, 4.0, 7.0},
    {2.0, 5.0, 8.0},
    {3.0, 6.0, 9.0}
  };
  mat3 m3 = {
    {10.0, 13.0, 16.0},
    {11.0, 14.0, 17.0},
    {12.0, 15.0, 18.0}
  };
  mat3 m4 = {
    {19.0, 22.0, 25.0},
    {20.0, 23.0, 26.0},
    {21.0, 24.0, 27.0}
  };
  mat3 m5 = {
    {28.0, 31.0, 34.0},
    {29.0, 32.0, 35.0},
    {30.0, 33.0, 36.0}
  };
  mat6 m6 = MAT6_ZERO_INIT;
  mat6_compose(m2, m3, m4, m5, m6);
  printf("Composition:\n");
  print_mat6(m6);
  printf("\n");

  vec6 res = VEC6_ZERO_INIT;
  vec6 v1 = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
  vec6_spatial_transpose_mulm(v1, m1, res);
  printf("v1'm1:\n");
  print_vec6(res);
  printf("\n");

  printf("\n");
  print_mat6(m1);
  mat3 m7 = {
    {36.0, 30.0, 24.0},
    {35.0, 29.0, 23.0},
    {34.0, 28.0, 22.0}
  };
  mat3 m8 = {
    {33.0, 27.0, 21.0},
    {32.0, 26.0, 20.0},
    {31.0, 25.0, 19.0}
  };
  mat3 m9 = {
    {18.0, 12.0, 6.0},
    {17.0, 11.0, 5.0},
    {16.0, 10.0, 4.0}
  };
  mat3 m10 = {
    {15.0, 9.0, 3.0},
    {14.0, 8.0, 2.0},
    {13.0, 7.0, 1.0}
  };
  mat6 m11 = MAT6_ZERO_INIT;
  mat6_compose(m7, m8, m9, m10, m11);
  printf("\n");
  print_mat6(m11);
  mat6_mul(m1, m11, m1);
  printf("m1*m11:\n");
  print_mat6(m1);
  printf("\n");

  print_vec6(v1);
  printf("\n");
  printf("m1*v1:\n");
  mat6_mulv(m1, v1, v1);
  print_vec6(v1);
  printf("\n");

  mat6_compose(m2, m3, m4, m5, m6);
  mat6_scale(m6, 2.0, m6);
  printf("scale\n");
  print_mat6(m6);
  printf("\n");

  mat3 r = {
    {1.0, 4.0, 7.0},
    {2.0, 5.0, 8.0},
    {3.0, 6.0, 9.0}
  };
  mat3 t = {
    {1.0, 0.0, 0.0},
    {0.0, 1.0, 0.0},
    {0.0, 0.0, 1.0}
  };
  mat6_spatial_transform(r, t, m1);
  printf("transform:\n");
  print_mat6(m1);
  printf("\n");

  vec6 v_copy = VEC6_ZERO_INIT;
  vec6_copy(v1, v_copy);
  printf("copy:\n");
  print_vec6(v_copy);
  printf("\n");

  vec3 a = {1.0, 2.0, 3.0};
  vec3 b = {4.0, 5.0, 6.0};
  vec6_compose(a, b, v1);
  printf("compose:\n");
  print_vec6(v1);
  printf("\n");

  vec6_add(v1, v1, v1);
  printf("add:\n");
  print_vec6(v1);
  printf("\n");

  vec6_sub(v1, v1, v1);
  printf("sub:\n");
  print_vec6(v1);
  printf("\n");

  vec6 v2 = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
  vec6_scale(v2, 2.0, v2);
  printf("scale:\n");
  print_vec6(v2);
  printf("\n");

  vec6 v3 = {4.0, 5.0, 6.0, 1.0, 2.0, 3.0};
  mat6_compose(m2, m3, m4, m5, m6);
  vec6 v4 = VEC6_ZERO_INIT;
  vec6_spatial_transpose_mulm(v3, m6, v4);
  printf("vec6'mat6:\n");
  print_vec6(v4);
  printf("\n");

  vec6 v5 = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
  vec6_spatial_transpose_mulv(v5, v3, m6);
  printf("ab':\n");
  print_mat6(m6);
  printf("\n");

  vec6 v6 = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
  float d = vec6_dot(v6, v6);
  printf("dot:\n");
  printf("%f\n", d);
  d = vec6_inner_product(v6, v6);
  printf("inner product:\n");
  printf("%f\n", d);

  printf("Misc. tests:\n");
  mat6 m12 = {
    {1.0, 0.0, 0.0, 0.0, 0.0, 1.5},
    {0.0, 1.0, 0.0, 0.0, 0.0, 0.0},
    {0.0, 0.0, 1.0, -1.5, 0.0, 0.0},
    {0.0, 0.0, 0.0, 1.0, 0.0, 0.0},
    {0.0, 0.0, 0.0, 0.0, 1.0, 0.0},
    {0.0, 0.0, 0.0, 0.0, 0.0, 1.0}
  };
  vec6 v7 = {-1.0, 0.0, 0.0, 0.0, 0.0, -1.0};
  mat6_mulv(m12, v7, v6);
  printf("  m:\n");
  print_mat6(m12);
  printf("  v:\n");
  print_vec6(v7);
  printf("  mv:\n");
  print_vec6(v6);
}

void init_dual_test(MODEL *t_mod, ENTITY **t_entity) {
  t_mod->animations = NULL;
  t_mod->k_chain_block = NULL;
  t_mod->keyframe_block = NULL;
  t_mod->sled_block = NULL;

  t_mod->num_bones = 7;
  t_mod->num_colliders = 5;
  t_mod->bones = malloc(sizeof(BONE) * t_mod->num_bones);
  t_mod->colliders = malloc(sizeof(COLLIDER) * t_mod->num_colliders);
  t_mod->collider_bone_links = malloc(sizeof(int) * t_mod->num_colliders);
  t_mod->bone_collider_links = malloc(sizeof(int) * t_mod->num_bones);
  t_mod->num_animations = 0;

  for (int i = 0; i < NUM_PROPS; i++) {
    t_mod->textures[i] = 0;
  }
  t_mod->VAO = 0;
  t_mod->VBO = 0;
  t_mod->EBO = 0;
  t_mod->num_indicies = 0;

  // ============================= COLLIDER SET-UP ===========================
  for (size_t i = 0; i < t_mod->num_colliders; i++) {
    COLLIDER *cur_col = t_mod->colliders + i;
    cur_col->num_children = 0;

    cur_col->type = POLY;
    cur_col->category = HURT_BOX;

    cur_col->data.num_used = 0;
  }
  t_mod->colliders[0].children_offset = 1;
  t_mod->colliders[0].num_children = 1;
  t_mod->colliders[1].children_offset = 2;
  t_mod->colliders[1].num_children = 1;
  t_mod->colliders[2].children_offset = 3;
  t_mod->colliders[2].num_children = 2;
  t_mod->colliders[3].children_offset = 0;
  t_mod->colliders[4].children_offset = 0;

  t_mod->collider_bone_links[0] = 0;
  t_mod->collider_bone_links[1] = 1;
  t_mod->collider_bone_links[2] = 2;
  t_mod->collider_bone_links[3] = 5;
  t_mod->collider_bone_links[4] = 6;

  glm_vec3_copy((vec3) { 1.0, 0.0, 0.0 },
                t_mod->colliders[0].data.center_of_mass);
  glm_vec3_copy((vec3) { 4.0, 0.0, 0.0 },
                t_mod->colliders[1].data.center_of_mass);
  glm_vec3_copy((vec3) { 7.0, 0.0, 0.0 },
                t_mod->colliders[2].data.center_of_mass);
  glm_vec3_copy((vec3) { 9.0, 2.0, 0.0 },
                t_mod->colliders[3].data.center_of_mass);
  glm_vec3_copy((vec3) { 10.0, -2.0, 0.0 },
                t_mod->colliders[4].data.center_of_mass);

  // =============================== BONE SET-UP =============================
  for (size_t i = 0; i < t_mod->num_bones; i++) {
    BONE *cur_bone = t_mod->bones + i;
    cur_bone->num_children = 0;

    glm_vec3_copy((vec3) { 0.0, -1.0, 0.0 }, cur_bone->coordinate_matrix[0]);
    glm_vec3_copy((vec3) { 1.0, 0.0, 0.0 }, cur_bone->coordinate_matrix[1]);
    glm_vec3_copy((vec3) { 0.0, 0.0, 1.0 }, cur_bone->coordinate_matrix[2]);
  }

  glm_vec3_copy((vec3) { 0.0, 0.0, 0.0 }, t_mod->bones[0].base);
  glm_vec3_copy((vec3) { 2.0, 0.0, 0.0 }, t_mod->bones[0].head);
  glm_vec3_copy((vec3) { 2.0, 0.0, 0.0 }, t_mod->bones[1].base);
  glm_vec3_copy((vec3) { 6.0, 0.0, 0.0 }, t_mod->bones[1].head);
  glm_vec3_copy((vec3) { 6.0, 0.0, 0.0 }, t_mod->bones[2].base);
  glm_vec3_copy((vec3) { 7.0, 0.0, 0.0 }, t_mod->bones[2].head);
  glm_vec3_copy((vec3) { 7.0, 0.0, 0.0 }, t_mod->bones[3].base);
  glm_vec3_copy((vec3) { 8.0, 2.0, 0.0 }, t_mod->bones[3].head);
  glm_vec3_copy((vec3) { 7.0, 0.0, 0.0 }, t_mod->bones[4].base);
  glm_vec3_copy((vec3) { 8.0, -2.0, 0.0 }, t_mod->bones[4].head);
  glm_vec3_copy((vec3) { 8.0, 2.0, 0.0 }, t_mod->bones[5].base);
  glm_vec3_copy((vec3) { 10.0, 2.0, 0.0 }, t_mod->bones[5].head);
  glm_vec3_copy((vec3) { 8.0, -2.0, 0.0 }, t_mod->bones[6].base);
  glm_vec3_copy((vec3) { 12.0, -2.0, 0.0 }, t_mod->bones[6].head);

  t_mod->bones[0].parent = -1;
  t_mod->bones[1].parent = 0;
  t_mod->bones[2].parent = 1;
  t_mod->bones[3].parent = 2;
  t_mod->bones[4].parent = 2;
  t_mod->bones[5].parent = 3;
  t_mod->bones[6].parent = 4;

  t_mod->bones[0].num_children = 1;
  t_mod->bones[1].num_children = 1;
  t_mod->bones[2].num_children = 2;
  t_mod->bones[3].num_children = 1;
  t_mod->bones[4].num_children = 1;

  t_mod->bone_collider_links[0] = 0;
  t_mod->bone_collider_links[1] = 1;
  t_mod->bone_collider_links[2] = 2;
  t_mod->bone_collider_links[3] = 2;
  t_mod->bone_collider_links[4] = 2;
  t_mod->bone_collider_links[5] = 3;
  t_mod->bone_collider_links[6] = 4;

  *t_entity = init_entity(t_mod);

  // ============================ PHYS DATA SET-UP ===========================
  for (size_t i = 0; i < t_mod->num_colliders; i++) {
    P_DATA *p_data = (*t_entity)->np_data;
    glm_mat4_identity(p_data[i].inv_inertia);
    p_data[i].inv_mass = 1.0;
    p_data[i].num_dofs = 1;
    glm_vec3_copy((vec3) { 0.0, 0.0, 1.0 }, p_data[i].dofs[0]);
  }
  mat3 inertia_0 = {
    { 2.0 / 3.0, 0.0, 0.0 },
    { 0.0, 2.0 / 3.0, 0.0 },
    { 0.0, 0.0, 2.0 / 3.0 },
  };
  glm_mat4_ins3(inertia_0, (*t_entity)->np_data[0].inv_inertia);
  glm_mat4_inv((*t_entity)->np_data[0].inv_inertia,
               (*t_entity)->np_data[0].inv_inertia);

  mat3 inertia_1 = {
    { 5.0 / 3.0, 0.0, 0.0 },
    { 0.0, 2.0 / 3.0, 0.0 },
    { 0.0, 0.0, 5.0 / 3.0 },
  };
  glm_mat4_ins3(inertia_1, (*t_entity)->np_data[1].inv_inertia);
  glm_mat4_inv((*t_entity)->np_data[1].inv_inertia,
               (*t_entity)->np_data[1].inv_inertia);

  mat3 inertia_2 = {
    { 2.0 / 3.0, 0.0, 0.0 },
    { 0.0, 10.0 / 3.0, 0.0 },
    { 0.0, 0.0, 10.0 / 3.0 },
  };
  glm_mat4_ins3(inertia_2, (*t_entity)->np_data[2].inv_inertia);
  glm_mat4_inv((*t_entity)->np_data[2].inv_inertia,
               (*t_entity)->np_data[2].inv_inertia);

  glm_mat4_ins3(inertia_0, (*t_entity)->np_data[3].inv_inertia);
  glm_mat4_inv((*t_entity)->np_data[3].inv_inertia,
               (*t_entity)->np_data[3].inv_inertia);

  glm_mat4_ins3(inertia_1, (*t_entity)->np_data[4].inv_inertia);
  glm_mat4_inv((*t_entity)->np_data[4].inv_inertia,
               (*t_entity)->np_data[4].inv_inertia);
}

void init_moving_test(MODEL *t_mod, ENTITY **t_entity) {
  init_dual_test(t_mod, t_entity);

  P_DATA *p_data = (*t_entity)->np_data;
  p_data[1].vel_angles[0] = -167988.0 / 618010.0;
  p_data[2].vel_angles[0] = 176514.0 / 618010.0;
  p_data[3].vel_angles[0] = 13608.0 / 618010.0;
  p_data[4].vel_angles[0] = 4494.0 / 618010.0;

  /*
  vec6_copy((vec6) { 0.0, 0.0, -167988.0/618010.0, 335976.0/618010.0, 0.0, 0.0 },
            p_data[1].v_hat);
  vec6_copy((vec6) { 0.0, 0.0, 8526.0/618010.0, 663426.0/618010.0, 0.0, 0.0 },
            p_data[2].v_hat);
  vec6_copy((vec6) { 0.0, 0.0, 22134.0/618010.0, 632766.0/618010.0, -17052.0/618010.0, 0.0 },
            p_data[3].v_hat);
  vec6_copy((vec6) { 0.0, 0.0, 13020.0/618010.0, 628860.0/618010.0, 17052.0/618010.0, 0.0 },
            p_data[4].v_hat);

  printf("BASE LINE:\n");
  print_vec6(p_data[1].v_hat);
  printf("\n");
  */
}

void init_basic_test(MODEL *t_mod, ENTITY **t_entity) {
  t_mod->animations = NULL;
  t_mod->k_chain_block = NULL;
  t_mod->keyframe_block = NULL;
  t_mod->sled_block = NULL;

  t_mod->bones = malloc(sizeof(BONE) * 3);
  t_mod->colliders = malloc(sizeof(COLLIDER) * 3);
  t_mod->collider_bone_links = malloc(sizeof(int) * 3);
  t_mod->bone_collider_links = malloc(sizeof(int) * 3);
  t_mod->num_animations = 0;
  t_mod->num_bones = 3;
  t_mod->num_colliders = 3;

  for (int i = 0; i < NUM_PROPS; i++) {
    t_mod->textures[i] = 0;
  }
  t_mod->VAO = 0;
  t_mod->VBO = 0;
  t_mod->EBO = 0;
  t_mod->num_indicies = 0;

  for (size_t i = 0; i < t_mod->num_colliders; i++) {
    COLLIDER *cur_col = t_mod->colliders + i;
    if (i < t_mod->num_colliders - 1) {
      cur_col->children_offset = i + 1;
      cur_col->num_children = 1;
    } else {
      cur_col->children_offset = 0;
      cur_col->num_children = 0;
    }
    cur_col->type = POLY;
    cur_col->category = HURT_BOX;

    glm_vec3_copy((vec3) { 0.5 + i, 0.0, 0.0 }, cur_col->data.center_of_mass);
    cur_col->data.num_used = 0;

    t_mod->collider_bone_links[i] = i;
  }

  for (size_t i = 0; i < t_mod->num_bones; i++) {
    BONE *cur_bone = t_mod->bones + i;
    glm_vec3_copy((vec3) { 0.0 + i, 0.0, 0.0 }, cur_bone->base);
    glm_vec3_copy((vec3) { 1.0 + i, 0.0, 0.0 }, cur_bone->head);
    if (i > 0) {
      cur_bone->parent = i - 1;
    } else {
      cur_bone->parent = -1;
    }

    if (i < t_mod->num_bones - 1) {
      cur_bone->num_children = 1;
    } else {
      cur_bone->num_children = 0;
    }

    glm_vec3_copy((vec3) { 0.0, -1.0, 0.0 }, cur_bone->coordinate_matrix[0]);
    glm_vec3_copy((vec3) { 1.0, 0.0, 0.0 }, cur_bone->coordinate_matrix[1]);
    glm_vec3_copy((vec3) { 0.0, 0.0, 1.0 }, cur_bone->coordinate_matrix[2]);

    t_mod->bone_collider_links[i] = i;
  }

  *t_entity = init_entity(t_mod);

  for (size_t i = 0; i < t_mod->num_colliders; i++) {
    P_DATA *p_data = (*t_entity)->np_data;
    mat3 inertia = {
      {1.0 / 6.0, 0.0, 0.0},
      {0.0, 1.0 / 6.0, 0.0},
      {0.0, 0.0, 1.0 / 6.0},
    };
    glm_mat4_ins3(inertia, p_data[i].inv_inertia);
    glm_mat4_inv(p_data[i].inv_inertia, p_data[i].inv_inertia);
    p_data[i].inv_mass = 1.0;
    p_data[i].num_dofs = 1;
    glm_vec3_copy((vec3) { 0.0, 0.0, 1.0 }, p_data[i].dofs[0]);
  }
}

int main() {
  // Test spatial algebra functions
  //sa_test();

  MODEL t_mod;
  ENTITY *t_ent;
  init_moving_test(&t_mod, &t_ent);
  featherstone_abm(t_ent);

  return 0;
}

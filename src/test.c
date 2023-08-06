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

void featherstone_abm(ENTITY *body);
void free_entity(ENTITY *);
void free_model(MODEL *);
int init_scene();
void get_model_mat(ENTITY *entity, mat4 model);
void calc_inertia_tensor(ENTITY *ent, size_t col);

// Not used but defition needed
vec3 col_point = { 0.0, 0.0, 0.0 };
int enable_gravity = 1;

void print_mat6(mat6 m) {
  for (int i = 0; i < 6; i++) {
    printf("|%.2f %.2f %.2f %.2f %.2f %.2f|\n",
           m[0][i], m[1][i], m[2][i], m[3][i], m[4][i], m[5][i]);
  }
}

void print_mat3 (mat3 m) {
  for (int i = 0; i < 3; i++) {
    printf("|%.2f %.2f %.2f|\n",
           m[0][i], m[1][i], m[2][i]);
  }
}

void print_mat4 (mat4 m) {
  for (int i = 0; i < 4; i++) {
    printf("|%.2f %.2f %.2f %.2f|\n",
           m[0][i], m[1][i], m[2][i], m[3][i]);
  }
}

void print_vec6(vec6 v) {
  printf("[%f %f %f %f %f %f]\n", v[0], v[1], v[2], v[3], v[4], v[5]);
}

void print_vec3(vec3 v) {
  printf("[%f %f %f]\n", v[0], v[1], v[2]);
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
    printf("    v-hat:\n");
    print_vec6(ent->np_data[i].v_hat);
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

  mat6_mul(m1, m1, m1);
  printf("m1*m1:\n");
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
}

void calc_final_b_mats() {
  mat4 model_mat = GLM_MAT4_IDENTITY_INIT;
  get_model_mat(ragdoll, model_mat);

  for (int i = 0; i < ragdoll->model->num_bones; i++) {
    vec3 temp = GLM_VEC3_ZERO_INIT;
    mat4 from_base = GLM_MAT4_IDENTITY_INIT;
    mat4 to_base = GLM_MAT4_IDENTITY_INIT;
    glm_mat4_mulv3(model_mat, ragdoll->model->bones[i].base, 1.0, temp);
    glm_translate(to_base, temp);
    glm_vec3_negate(temp);
    glm_translate(from_base, temp);

    glm_mat4_identity(ragdoll->final_b_mats[i]);
    glm_mat4_mul(from_base, ragdoll->final_b_mats[i],
                 ragdoll->final_b_mats[i]);
    glm_mat4_mul(ragdoll->bone_mats[i][SCALE],
                 ragdoll->final_b_mats[i], ragdoll->final_b_mats[i]);
    glm_mat4_mul(ragdoll->bone_mats[i][ROTATION],
                 ragdoll->final_b_mats[i], ragdoll->final_b_mats[i]);
    glm_mat4_mul(to_base, ragdoll->final_b_mats[i],
                 ragdoll->final_b_mats[i]);
    glm_mat4_mul(ragdoll->bone_mats[i][LOCATION],
                 ragdoll->final_b_mats[i], ragdoll->final_b_mats[i]);

    int parent = ragdoll->model->bones[i].parent;
    if (parent != -1) {
      glm_mat4_mul(ragdoll->final_b_mats[parent], ragdoll->final_b_mats[i],
                   ragdoll->final_b_mats[i]);
    }
  }
}

int main() {
  GLFWwindow *window;

  if (!glfwInit()) {
    return -1;
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  window = glfwCreateWindow(640.0, 400.0, "Jack", NULL, NULL);
  if (window == NULL) {
    printf("Failed to create GLFW window\n");
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    printf("Failed to initialize GLAD\n");
    glfwTerminate();
    return -1;
  }
  int status = init_scene();
  if (status) {
    return -1;
  }

  //ragdoll->type |= T_DRIVING;
  ragdoll->inv_mass = 1.0;
  ragdoll->scale[0] = 2.0;
  ragdoll->scale[1] = 2.0;
  ragdoll->scale[2] = 2.0;
  for (size_t i = 0; i < ragdoll->model->num_colliders; i++) {
    ragdoll->np_data[i].inv_mass = 1.0;
    calc_inertia_tensor(ragdoll, i);
  }

  // Test spatial algebra functions
  // sa_test();

  // Complex featherstone abm test
  // Bone 1 normal
  // Bone 2 -45 degrees
  // Bone 3 -45 degrees
  versor quat = GLM_QUAT_IDENTITY_INIT;
  glm_quatv(quat, glm_rad(-45.0), ragdoll->model->bones[1].coordinate_matrix[0]);
  glm_quat_mat4(quat, ragdoll->bone_mats[1][ROTATION]);
  glm_quatv(quat, glm_rad(-45.0), ragdoll->model->bones[3].coordinate_matrix[0]);
  glm_quat_mat4(quat, ragdoll->bone_mats[3][ROTATION]);
  calc_final_b_mats();

  printf("\n\nBefore:\n");
  print_p_data(ragdoll);

  ragdoll->np_data[0].joint_angle_vels[3] = 1.0;
  featherstone_abm(ragdoll);

  printf("\n\nAfter:\n");
  print_p_data(ragdoll);



  free_entity(player);
  free_entity(ragdoll);
  free_entity(box_entity);
  free_entity(sphere_entity);
  free_entity(floor_entity);
  free_entity(obstacle);
  for (int i = 0; i < NUM_BOXES; i++) {
    free_entity(boxes[i]);
  }
  free(boxes);
  for (int i = 0; i < NUM_SPHERES; i++) {
    free_entity(spheres[i]);
  }
  free(spheres);
  for (int i = 0; i < NUM_RECTS; i++) {
    free_entity(rects[i]);
  }
  free(rects);

  free_model(platform);
  free_model(cube);
  free_model(rect_prism);
  free_model(test);
  free_model(dude);
  free_model(floor_model);
  free_model(sphere);
  free_model(vector);

  glfwTerminate();
  return 0;
}

#include <init.h>

unsigned int shader;
unsigned int u_shader;
unsigned int bone_shader;
unsigned int basic_shader;
unsigned int test_shader;

MODEL *cube;
MODEL *dude;
MODEL *test;
MODEL *floor_model;
MODEL *platform;
MODEL *sphere;
MODEL *vector;

ENTITY *player;
ENTITY *obstacle;
ENTITY *floor_entity;
ENTITY *box_entity;
ENTITY *sphere_entity;
ENTITY **boxes;
ENTITY **spheres;
const int NUM_BOXES = 1;
const int NUM_SPHERES = 1;

int init_scene() {
  shader = init_shader_prog(
      DIR"/src/shaders/cell_shader/shader.vs",
      NULL,
      DIR"/src/shaders/cell_shader/shader.fs"
      );
  if (shader == -1) {
    printf("Error loading shaders\n");
    return -1;
  }

  u_shader = init_shader_prog(
      DIR"/src/shaders/unanimated/shader.vs",
      NULL,
      DIR"/src/shaders/cell_shader/shader.fs"
      );
  if (u_shader == -1) {
    printf("Error loading shaders\n");
    return -1;
  }

  bone_shader = init_shader_prog(
      DIR"/src/shaders/bone/shader.vs",
      NULL,
      DIR"/src/shaders/basic/shader.fs"
      );
  if (bone_shader == -1) {
    printf("Error loading bone shaders\n");
    return -1;
  }

  basic_shader = init_shader_prog(
      DIR"/src/shaders/basic/shader.vs",
      NULL,
      DIR"/src/shaders/basic/shader.fs"
      );
  if (basic_shader == -1) {
    printf("Error loading basic shaders\n");
    return -1;
  }

  test_shader = init_shader_prog(
      DIR"/src/shaders/unanimated/shader.vs",
      NULL,
      DIR"/src/shaders/test/shader.fs"
      );
  if (test_shader == -1) {
    printf("Error loading test shaders\n");
    return -1;
  }

  cube = load_model(
      DIR"/resources/cube/cube.obj"
      );
  if (cube == NULL) {
    printf("Unable to load cube model\n");
    return -1;
  }

  dude = load_model(
      DIR"/resources/low_poly_new/low_poly_new.obj"
      );
  if (dude == NULL) {
    printf("Unable to load dude model\n");
    return -1;
  }

  test = load_model(
      DIR"/resources/test/test.obj"
      );
  if (test == NULL) {
    printf("Unable to load test model\n");
    return -1;
  }

  floor_model = load_model(
      DIR"/resources/floor/floor.obj"
      );
  if (floor_model == NULL) {
    printf("Unable to load floor model\n");
    return -1;
  }

  platform = load_model(
      DIR"/resources/platform/platform.obj"
      );
  if (platform == NULL) {
    printf("Unable to load platform model\n");
    return -1;
  }

  sphere = load_model(
      DIR"/resources/sphere/sphere.obj"
      );
  if (sphere == NULL) {
    printf("Unable to load spehere model\n");
    return -1;
  }

  vector = load_model(
      DIR"/resources/vector/vector.obj"
      );
  if (vector == NULL) {
    printf("Unable to load vector model\n");
    return -1;
  }

  player = init_entity(dude);
  if (player == NULL) {
    printf("Unable to load player\n");
    return -1;
  }

  obstacle = init_entity(platform);
  if (obstacle == NULL) {
    printf("Unable to load obstacle\n");
    return -1;
  }
  vec3 ob_pos = { 3.0, 0.0, -3.0 };
  glm_vec3_copy(ob_pos, obstacle->translation);

  floor_entity = init_entity(floor_model);
  if (floor_entity == NULL) {
    printf("Unable to load floor entity\n");
    return -1;
  }
  vec3 floor_scale = { 50.0, 1.0, 50.0 };
  glm_vec3_copy(floor_scale, floor_entity->scale);

  vec3 cube_pos = { 3.0, 2.0, 3.0 };
  box_entity = init_entity(cube);
  if (box_entity == NULL) {
    printf("Unable to load box entity\n");
    return -1;
  }
  glm_vec3_copy(cube_pos, box_entity->translation);

  vec3 s_pos = { -3.0, 2.0, 3.0 };
  sphere_entity = init_entity(sphere);
  if (sphere_entity == NULL) {
    printf("Unable to load sphere entity\n");
    return -1;
  }
  glm_vec3_copy(s_pos, sphere_entity->translation);

  boxes = malloc(sizeof(ENTITY *) * NUM_BOXES);
  vec3 m_box_pos = { -1.0, 3.0, -3.0 };
  vec3 m_box_scale = { 0.5, 0.5, 0.5 };
  for (int i = 0; i < NUM_BOXES; i++) {
    boxes[i] = init_entity(cube);
    if (boxes[i] == NULL) {
      printf("Unable to load moveable box: %d\n", i);
      return -1;
    }
    m_box_pos[2] += 1.0;
    glm_vec3_copy(m_box_pos, boxes[i]->translation);
    glm_vec3_copy(m_box_scale, boxes[i]->scale);
  }

  spheres = malloc(sizeof(ENTITY *) * NUM_SPHERES);
  vec3 m_sphere_pos = { 1.0, 3.0, -3.0 };
  vec3 m_sphere_scale = { 0.5, 0.5, 0.5 };
  for (int i = 0; i < NUM_SPHERES; i++) {
    spheres[i] = init_entity(sphere);
    if (spheres[i] == NULL) {
      fprintf(stderr, "Unable to load moveable sphere: %d\n", i);
      return -1;
    }
    m_sphere_pos[2] += 1.0;
    glm_vec3_copy(m_sphere_pos, spheres[i]->translation);
    glm_vec3_copy(m_sphere_scale, spheres[i]->scale);
  }

  return 0;
}

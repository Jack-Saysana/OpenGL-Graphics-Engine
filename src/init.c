#include <init.h>
#include <string.h>

unsigned int shader;
unsigned int u_shader;
unsigned int bone_shader;
unsigned int basic_shader;
unsigned int test_shader;

MODEL *cube;
MODEL *rect_prism;
MODEL *dude;
MODEL *test;
MODEL *floor_model;
MODEL *platform;
MODEL *sphere;
MODEL *vector;
MODEL *quad;

ENTITY *player;
ENTITY *obstacle;
ENTITY *floor_entity;
ENTITY *box_entity;
ENTITY *sphere_entity;
ENTITY **boxes;
ENTITY **spheres;
ENTITY **rects;
const int NUM_BOXES = 0;
const int NUM_SPHERES = 0;
const int NUM_RECTS = 0;
ENTITY *ragdoll;

extern vec3 m_box_pos;
extern vec3 m_box_scale;
extern vec3 m_sphere_pos;
extern vec3 m_sphere_scale;
extern vec3 m_rect_pos;
extern vec3 m_rect_scale;
extern vec3 ob_pos;
extern vec3 floor_scale;
extern vec3 cube_pos;
extern vec3 s_pos;
extern vec3 ragdoll_pos;

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

  rect_prism = load_model(
      DIR"/resources/rect_prism/rect_prism.obj"
      );
  if (rect_prism == NULL) {
    printf("Unable to load rect prism model\n");
  }

  dude = load_model(
      DIR"/resources/low_poly_new/low_poly_new.obj"
      );
  if (dude == NULL) {
    printf("Unable to load dude model\n");
    return -1;
  }

  test = load_model(
      //DIR"/resources/three_link/three_link_2.obj"
      //DIR"/resources/three_link/three_link.obj"
      //DIR"/resources/two_link/two_link.obj"
      //DIR"/resources/dual_link/dual_link.obj"
      DIR"/resources/three_link/three_link_4.obj"
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

  quad = load_model(
      DIR"/resources/quad/quad.obj"
      );
  if (quad == NULL) {
    printf("Unable to load quad model\n");
    return -1;
  }

  load_model(DIR"/resources/EOW/medium_ship/medium_ship.obj");
  load_model(DIR"/resources/EOW/large_ship/large_ship.obj");
  load_model(DIR"/resources/EOW/custom_ship/custom_ship.obj");
  load_model(DIR"/resources/EOW/trade_ship/trade_ship.obj");
  load_model(DIR"/resources/EOW/enemy_ship/enemy_ship.obj");
  load_model(DIR"/resources/EOW/enemy/enemy.obj");
  load_model(DIR"/resources/EOW/merchant/merchant.obj");
  load_model(DIR"/resources/EOW/player/player.obj");
  load_model(DIR"/resources/quad/quad.obj");
  load_model(DIR"/resources/circle/circle.obj");
  load_model(DIR"/resources/chest/chest.obj");
  load_model(DIR"/resources/planet/planet.obj");
  load_model(DIR"/resources/EOW_frames/player_still.obj");
  load_model(DIR"/resources/EOW_frames/player_walk_1.obj");
  load_model(DIR"/resources/EOW_frames/player_walk_2.obj");
  load_model(DIR"/resources/EOW_frames/player_walk_3.obj");
  load_model(DIR"/resources/EOW_frames/enemy_still.obj");
  load_model(DIR"/resources/EOW_frames/enemy_walk_1.obj");
  load_model(DIR"/resources/EOW_frames/enemy_walk_2.obj");
  load_model(DIR"/resources/EOW_frames/enemy_walk_3.obj");
  char path[100] = DIR"/resources/Dinklebitmap/x.obj";
  for (char cur = ' '; cur < 127; cur++) {
    if (cur == '/') {
      //printf("%s\n", DIR"/resources/Dinklebitmap/fs.obj");
      load_model(DIR"/resources/Dinklebitmap/fs.obj");
    } else if (cur == ' ') {
      load_model(DIR"/resources/Dinklebitmap/space.obj");
    } else if (cur == '\\') {
      load_model(DIR"/resources/Dinklebitmap/bs.obj");
    } else if (cur == '.') {
      load_model(DIR"/resources/Dinklebitmap/period.obj");
    }else {
      path[strlen(path) - 5] = cur;
      //printf("%s\n", path);
      load_model(path);
    }
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
  glm_vec3_copy(ob_pos, obstacle->translation);

  floor_entity = init_entity(floor_model);
  if (floor_entity == NULL) {
    printf("Unable to load floor entity\n");
    return -1;
  }
  glm_vec3_copy(floor_scale, floor_entity->scale);

  box_entity = init_entity(cube);
  if (box_entity == NULL) {
    printf("Unable to load box entity\n");
    return -1;
  }
  glm_vec3_copy(cube_pos, box_entity->translation);

  sphere_entity = init_entity(sphere);
  if (sphere_entity == NULL) {
    printf("Unable to load sphere entity\n");
    return -1;
  }
  glm_vec3_copy(s_pos, sphere_entity->translation);

  boxes = malloc(sizeof(ENTITY *) * NUM_BOXES);
  for (int i = 0; i < NUM_BOXES; i++) {
    boxes[i] = init_entity(cube);
    if (boxes[i] == NULL) {
      printf("Unable to load moveable box: %d\n", i);
      return -1;
    }
    m_box_pos[2] -= 1.0;
    glm_vec3_copy(m_box_pos, boxes[i]->translation);
    glm_vec3_copy(m_box_scale, boxes[i]->scale);
  }

  spheres = malloc(sizeof(ENTITY *) * NUM_SPHERES);
  for (int i = 0; i < NUM_SPHERES; i++) {
    spheres[i] = init_entity(sphere);
    if (spheres[i] == NULL) {
      fprintf(stderr, "Unable to load moveable sphere: %d\n", i);
      return -1;
    }
    m_sphere_pos[0] += 1.0;
    glm_vec3_copy(m_sphere_pos, spheres[i]->translation);
    glm_vec3_copy(m_sphere_scale, spheres[i]->scale);
  }

  rects = malloc(sizeof(ENTITY *) * NUM_RECTS);
  for (int i = 0; i < NUM_RECTS; i++) {
    rects[i] = init_entity(rect_prism);
    if (rects[i] == NULL) {
      fprintf(stderr, "Unable to load moveable rect prism: %d\n", i);
      return -1;
    }
    m_rect_pos[2] += 1.0;
    glm_vec3_copy(m_rect_pos, rects[i]->translation);
    glm_vec3_copy(m_rect_scale, rects[i]->scale);
  }

  ragdoll = init_entity(test);
  //ragdoll = init_entity(dude);
  glm_vec3_copy(ragdoll_pos, ragdoll->translation);
  if (ragdoll == NULL) {
    printf("Unable to load ragdoll\n");
    return -1;
  }

  return 0;
}

#include <main.h>

#define ARENA_WIDTH (100)
#define RENDER_DISTANCE (10)

// SCENE ELEMENTS
extern unsigned int shader;
extern unsigned int u_shader;
extern unsigned int bone_shader;
extern unsigned int basic_shader;
extern unsigned int test_shader;

extern MODEL *cube;
extern MODEL *rect_prism;
extern MODEL *dude;
extern MODEL *test;
extern MODEL *floor_model;
extern MODEL *platform;
extern MODEL *sphere;
extern MODEL *vector;
extern MODEL *quad;
extern MODEL *four_mod;

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
extern ENTITY *four_ent[ARENA_WIDTH * ARENA_WIDTH];
extern ENTITY *render_sphere;

extern F_GLYPH *font;
extern int font_len;

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

// CAMERA INFO
vec3 up = { 0.0, 1.0, 0.0 };
vec3 camera_offset = { 0.0, 0.0, -5.0 };
vec3 camera_front = { 0.0, 0.0, -1.0 };
vec3 camera_pos = { 0.0, 0.0, 0.0 };
vec3 camera_model_pos = { 0.0, 0.5, 0.0 };
float camera_model_rot = 0.0;
float pitch = 0;
float yaw = 0;
vec3 movement = { 0.0, 0.0, 0.0 };

// CONTROLS
int firstMouse = 1;
float lastX = 400;
float lastY = 300;
float last_push = 0.0;
int toggled = 1;
int space_pressed = 0;
int draw = 0;

// MISC DATA
mat4 persp_proj = GLM_MAT4_IDENTITY_INIT;
int cur_frame = 0;
vec3 col_point = { 0.0, 0.0, 0.0 };
int enable_gravity = 1;

int featherstone_abm(ENTITY *body);
void integrate_ragdoll(ENTITY *subject);

int main() {
  GLFWwindow *window = init_gl("Jack");
  register_fb_size_callback(framebuffer_size_callback);
  register_mouse_movement_callback(mouse_input);
  register_scroll_callback(scroll_callback);

  int status = init_scene();
  if (status) {
    cleanup_gl();
    return 1;
  }
  glm_vec3_copy((vec3) {RENDER_DISTANCE, RENDER_DISTANCE, RENDER_DISTANCE },
                render_sphere->scale);
  // Ensure collision always checked on render sphere
  render_sphere->velocity[X] = 0.01;

  vec3 cube_col = GLM_VEC3_ONE_INIT;
  vec3 s_col = GLM_VEC3_ONE_INIT;
  glm_quatv(player->rotation, camera_model_rot, up);

  mat4 ortho_proj = GLM_MAT4_IDENTITY_INIT;
  mat4 model = GLM_MAT4_IDENTITY_INIT;
  mat4 view = GLM_MAT4_IDENTITY_INIT;

  glm_ortho(-1.0, 1.0, -1.0, 1.0, 0.0, 100.0, ortho_proj);
  glm_perspective(glm_rad(45.0f), RES_X / RES_Y, 0.1f, 100.0f, persp_proj);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  /* Origin VAO setup */

  float point[] = { 0.0, 0.0, 0.0 };
  unsigned int pt_VAO;
  glGenVertexArrays(1, &pt_VAO);
  glBindVertexArray(pt_VAO);
  unsigned int pt_VBO;
  glGenBuffers(1, &pt_VBO);
  glBindBuffer(GL_ARRAY_BUFFER, pt_VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3, point, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                        (void *) 0);
  glEnableVertexAttribArray(0);
  glBindVertexArray(0);

  // UI SET UP
  init_ui("resources/quad/quad.obj", "src/shaders/ui/shader.vs",
          "src/shaders/ui/shader.fs", "src/shaders/font/shader.vs",
          "src/shaders/font/shader.fs");
  UI_COMP *c1 = add_ui_comp(UI_ROOT_COMP, (vec2) { 0.5, -0.5 }, 0.75, 0.5,
                            ABSOLUTE_POS | POS_UNIT_RATIO |
                            WIDTH_UNIT_RATIO_Y | HEIGHT_UNIT_RATIO_Y);
  set_ui_pivot(c1, PIVOT_CENTER);
  set_ui_texture(c1, "resources/ui/ui_bg.png");
  //set_ui_on_click(c1, test_callback_click, NULL);
  //set_ui_on_release(c1, test_callback_release, NULL);

  UI_COMP *c2 = add_ui_comp(c1, (vec2) { 0.0, 0.0 }, 128.0, 64.0,
                            RELATIVE_POS | POS_UNIT_PIXEL | SIZE_UNIT_PIXEL);
  set_ui_pivot(c2, PIVOT_TOP_LEFT);
  set_ui_text(c2, "Hello!\nmy name is\nJack!!!", 16.0, T_RIGHT, font,
              GLM_VEC3_ZERO);
  set_ui_texture(c2, "resources/ui/ui_bg.png");
  //set_ui_on_click(c2, test_callback_click, NULL);

  UI_COMP *c3 = add_ui_comp(c1, (vec2) { 0.99, -0.01 }, 16.0, 16.0,
                            ABSOLUTE_POS | POS_UNIT_RATIO_X | SIZE_UNIT_PIXEL);
  set_ui_pivot(c3, PIVOT_TOP_RIGHT);
  set_ui_texture(c3, "resources/ui/ui_close_unpressed.png");
  //set_ui_on_click(c3, test_callback_click, NULL);

  UI_COMP *tool_tip = add_ui_comp(UI_ROOT_COMP, (vec2) { 0.0, 0.0 }, 64.0,
                                  64.0, ABSOLUTE_POS | POS_UNIT_PIXEL |
                                  SIZE_UNIT_PIXEL);
  set_ui_display(tool_tip, 0);
  set_ui_on_hover(c1, test_callback_hover, (void *) tool_tip);
  set_ui_no_hover(c1, test_callback_no_hover, (void *) tool_tip);
  set_manual_layer(c1, 0.2);
  //set_ui_enabled(c1, 0);

  /*
  UI_COMP *c4 = add_ui_comp(c1, (vec2) { 0.0, 0.0 }, 32.0, 32.0,
                            RELATIVE_POS | POS_UNIT_PIXEL | SIZE_UNIT_PIXEL);
  set_ui_pivot(c4, PIVOT_TOP_LEFT);

  UI_COMP *c5 = add_ui_comp(c1, (vec2) { 0.1, -15.0 }, 64.0, 32.0,
                            RELATIVE_POS | POS_X_UNIT_RATIO_X |
                            POS_Y_UNIT_PIXEL | SIZE_UNIT_PIXEL);
  set_ui_pivot(c5, PIVOT_TOP_LEFT);

  add_ui_comp(c1, (vec2) { 0.0, 0.0 }, 32.0, 32.0, RELATIVE_POS |
              POS_X_UNIT_RATIO_X | POS_Y_UNIT_PIXEL | SIZE_UNIT_PIXEL);
  add_ui_comp(c1, (vec2) { 0.0, 0.0 }, 64.0, 32.0, RELATIVE_POS |
              POS_X_UNIT_RATIO_X | POS_Y_UNIT_PIXEL | SIZE_UNIT_PIXEL);
  add_ui_comp(c1, (vec2) { 0.0, 0.0 }, 64.0, 32.0, RELATIVE_POS |
              POS_X_UNIT_RATIO_X | POS_Y_UNIT_PIXEL | SIZE_UNIT_PIXEL);
  add_ui_comp(c1, (vec2) { 0.0, 0.0 }, 64.0, 32.0, RELATIVE_POS |
              POS_X_UNIT_RATIO_X | POS_Y_UNIT_PIXEL | SIZE_UNIT_PIXEL);
  */

  // SIMULATION SET UP
  float max_extents = 1024.0;
  unsigned int max_depth = 9;
  SIMULATION *render_sim = init_sim(max_extents, max_depth);
  SIMULATION *sim = init_sim(max_extents, max_depth);
  if (status) {
    return 1;
  }

  sim_add_force(sim, (vec3) { 0.0, -GRAVITY, 0.0 });

  player->type |= T_DRIVING;
  player->inv_mass = 1.0;
  status = sim_add_entity(sim, player, ALLOW_DEFAULT);
  if (status != 0) {
    cleanup_gl();
    return 1;
  }

  render_sphere->type |= T_DRIVING;
  status = sim_add_entity(render_sim, render_sphere, ALLOW_DEFAULT);
  if (status != 0) {
    cleanup_gl();
    return 1;
  }

  //ragdoll->type |= T_DRIVING;
  ragdoll->inv_mass = 1.0;
  ragdoll->scale[0] = 2.0;
  ragdoll->scale[1] = 2.0;
  ragdoll->scale[2] = 2.0;
  for (size_t i = 0; i < ragdoll->model->num_colliders; i++) {
    ragdoll->np_data[i].inv_mass = 1.0;
    if (i == ragdoll->model->num_colliders - 1) {
      ragdoll->np_data[i].inv_mass = 0.5;
    }

    vec3 dof = { 1.0, 0.0, 0.0 };

    glm_vec3_copy(dof, ragdoll->np_data[i].dof);
  }
  //ragdoll->np_data[0].vel_angles[0] = 1.0;
  /*
  status = sim_insert_entity(sim, ragdoll, ALLOW_HURT_BOXES);
  if (status != 0) {
    glfwTerminate();
  }
  */

  /*
  // Insertion of test entities into the physics simulation
  obstacle->type |= T_IMMUTABLE;
  status = sim_add_entity(sim, obstacle, ALLOW_DEFAULT);
  if (status != 0) {
    glfwTerminate();
  }

  sphere_entity->type |= T_IMMUTABLE;
  status = sim_add_entity(sim, sphere_entity,
                          ALLOW_DEFAULT | ALLOW_HURT_BOXES);
  if (status != 0) {
    glfwTerminate();
  }

  box_entity->type |= T_IMMUTABLE;
  status = sim_add_entity(sim, box_entity, ALLOW_DEFAULT);
  if (status != 0) {
    glfwTerminate();
  }
  */

  for (int i = 0; i < ARENA_WIDTH * ARENA_WIDTH; i++) {
    four_ent[i]->type |= T_IMMUTABLE;
    status = sim_add_entity(sim, four_ent[i], ALLOW_HURT_BOXES);
    if (status != 0) {
      glfwTerminate();
    }
    status = sim_add_entity(render_sim, four_ent[i], ALLOW_DEFAULT);
    if (status != 0) {
      glfwTerminate();
    }
  }

  for (int i = 0; i < NUM_BOXES; i++) {
    boxes[i]->inv_mass = 1.0;
    vec3 init_vel = { 0.0, -0.05, 0.0 };
    glm_vec3_copy(init_vel, boxes[i]->velocity);
    //vec3 init_ang_vel = { -0.168430775, 0.00429611094, 0.00221232418 };
    //glm_vec3_copy(init_ang_vel, boxes[i]->ang_velocity);
    //versor init_rot = { 0.701721907, 0.477072179, -0.519831359, 0.0988073647 };
    //glm_quat_normalize(init_rot);
    //glm_quat_copy(init_rot, boxes[i]->rotation);

    glm_mat4_identity(boxes[i]->inv_inertia);
    boxes[i]->inv_inertia[0][0] = (12.0 * boxes[i]->inv_mass) / 2.0;
    boxes[i]->inv_inertia[1][1] = (12.0 * boxes[i]->inv_mass) / 2.0;
    boxes[i]->inv_inertia[2][2] = (12.0 * boxes[i]->inv_mass) / 2.0;

    status = sim_add_entity(sim, boxes[i], ALLOW_HURT_BOXES);
    if (status) {
      cleanup_gl();
      return 1;
    }
  }

  for (int i = 0; i < NUM_SPHERES; i++) {
    spheres[i]->inv_mass = 1.0;
    vec3 init_vel = { 0.0, -0.05, 0.0 };
    glm_vec3_copy(init_vel, spheres[i]->velocity);

    glm_mat4_identity(spheres[i]->inv_inertia);
    spheres[i]->inv_inertia[0][0] = spheres[i]->inv_mass / 0.1;
    spheres[i]->inv_inertia[1][1] = spheres[i]->inv_mass / 0.1;
    spheres[i]->inv_inertia[2][2] = spheres[i]->inv_mass / 0.1;

    status = sim_add_entity(sim, spheres[i], ALLOW_HURT_BOXES);
    if (status) {
      cleanup_gl();
      return 1;
    }
  }

  for (int i = 0; i < NUM_RECTS; i++) {
    rects[i]->inv_mass = 1.0;
    vec3 init_vel = { 0.0, 0.05, 0.0 };
    glm_vec3_copy(init_vel, rects[i]->velocity);

    glm_mat4_identity(rects[i]->inv_inertia);
    rects[i]->inv_inertia[0][0] = (12.0 * rects[i]->inv_mass) / 1.25;
    rects[i]->inv_inertia[1][1] = (12.0 * rects[i]->inv_mass) / 5.0;
    rects[i]->inv_inertia[2][2] = (12.0 * rects[i]->inv_mass) / 4.25;

    status = sim_add_entity(sim, rects[i], ALLOW_HURT_BOXES);
    if (status) {
      cleanup_gl();
      return 1;
    }
  }

  /*
  floor_entity->type |= T_IMMUTABLE;
  glm_mat4_zero(floor_entity->inv_inertia);
  status = sim_add_entity(sim, floor_entity, ALLOW_DEFAULT);
  if (status != 0) {
    cleanup_gl();
    return 1;
  }
  */

  while (!glfwWindowShouldClose(window)) {
    if (CURSOR_ENABLED) {
      glfwSetInputMode(window, GLFW_CURSOR,GLFW_CURSOR_NORMAL);
    } else {
      glfwSetInputMode(window, GLFW_CURSOR,GLFW_CURSOR_DISABLED);
    }

    float current_time = glfwGetTime();
    DELTA_TIME = current_time - LAST_FRAME;
    LAST_FRAME = current_time;

    if (DELTA_TIME > 0.25) {
      DELTA_TIME = 0.01;
    }

    keyboard_input(window);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shader);
    set_mat4("projection", persp_proj, shader);

    glUseProgram(u_shader);
    set_mat4("projection", persp_proj, u_shader);

    glUseProgram(bone_shader);
    set_mat4("projection", persp_proj, bone_shader);

    glUseProgram(basic_shader);
    set_mat4("projection", persp_proj, basic_shader);

    glUseProgram(test_shader);
    set_mat4("projection", persp_proj, test_shader);

    /* Animation */

    animate(player, 0, cur_frame);

    /* Physics */

    glm_vec3_add(movement, player->velocity, player->velocity);
    vec3 displacement = { 0.0, 0.0, 0.0 };
    glm_vec3_copy(player->translation, displacement);
    glm_vec3_copy(player->translation, render_sphere->translation);

    integrate_sim(sim);
    COLLISION *collisions = NULL;
    size_t num_collisions = get_sim_collisions(sim, &collisions);
    for (size_t i = 0; i < num_collisions; i++) {
      impulse_resolution(sim, collisions[i]);
    }
    free(collisions);

    //featherstone_abm(ragdoll);
    //integrate_ragdoll(ragdoll);

    glm_vec3_sub(player->translation, displacement, displacement);
    glm_vec3_add(displacement, camera_model_pos, camera_model_pos);

    /* Camera */

    vec3 translation = { -camera_model_pos[0], -camera_model_pos[1],
                         -camera_model_pos[2] };

    glm_mat4_identity(view);
    camera_offset[1] = -dude->bones[18].base[1];
    glm_translate(view, camera_offset);
    glm_rotate_x(view, glm_rad(pitch), view);
    glm_rotate_y(view, glm_rad(yaw), view);
    glm_translate(view, translation);

    mat4 rot_mat = GLM_MAT4_IDENTITY_INIT;
    glm_mat4_inv_fast(view, rot_mat);
    glm_vec3_zero(camera_pos);
    glm_mat4_mulv3(rot_mat, camera_pos, 1.0, camera_pos);

    /* Models */

    glm_mat4_identity(model);

    /* Origin */

    glPointSize(10.0);
    glUseProgram(basic_shader);
    set_mat4("model", model, basic_shader);
    set_mat4("view", view, basic_shader);
    set_vec3("test_col", (vec3) { 0.0, 1.0, 1.0 }, basic_shader);
    glBindVertexArray(pt_VAO);
    glDrawArrays(GL_POINTS, 0, 1);

    /* Test collision point */

    glm_translate(model, col_point);
    set_mat4("model", model, basic_shader);
    set_vec3("test_col", (vec3) { 1.0, 0.0, 0.0 }, basic_shader);
    glDrawArrays(GL_POINTS, 0, 1);
    glBindVertexArray(0);

    /* Skeleton */

    glm_vec3_copy(camera_model_pos, player->translation);
    glm_quatv(player->rotation, camera_model_rot, up);

    glUseProgram(bone_shader);
    set_vec3("test_col", (vec3) { 0.0, 0.0, 1.0 }, bone_shader);
    set_mat4("view", view, bone_shader);

    draw_skeleton(bone_shader, player);
    //draw_skeleton(bone_shader, ragdoll);

    /* Colliders */

    glPointSize(5.0);
    glUseProgram(basic_shader);
    set_vec3("test_col", (vec3) { 1.0, 0.0, 1.0 }, basic_shader);
    glBindVertexArray(pt_VAO);
    draw_colliders(basic_shader, player, sphere);
    //draw_colliders(basic_shader, ragdoll, sphere);
    //draw_colliders(basic_shader, obstacle, sphere);
    //draw_colliders(basic_shader, floor_entity, sphere);
    for (int i = 0; i < NUM_BOXES; i++) {
      draw_colliders(basic_shader, boxes[i], sphere);
    }

    // Only render within render distance
    num_collisions = get_sim_collisions(render_sim, &collisions);
    for (size_t i = 0; i < num_collisions; i++) {
      if (collisions[i].a_ent == render_sphere ||
          collisions[i].b_ent == render_sphere) {
        if (collisions[i].a_ent != render_sphere) {
          draw_colliders(basic_shader, collisions[i].a_ent, sphere);
        } else if (collisions[i].b_ent != render_sphere) {
          draw_colliders(basic_shader, collisions[i].b_ent, sphere);
        }
      }
    }
    free(collisions);

    set_vec3("test_col", cube_col, basic_shader);
    //draw_colliders(basic_shader, box_entity, sphere);

    set_vec3("test_col", s_col, basic_shader);
    //draw_colliders(basic_shader, sphere_entity, sphere);

    draw_colliders(basic_shader, render_sphere, sphere);

    /* Player */

    glUseProgram(shader);
    set_mat4("view", view, shader);
    set_vec3("camera_pos", camera_pos, shader);
    if (draw) {
      draw_entity(shader, player);
      //draw_entity(shader, ragdoll);
    }

    /* Objects */

    glUseProgram(test_shader);
    set_mat4("view", view, test_shader);
    set_vec3("col", (vec3) { 1.0, 1.0, 1.0 }, test_shader);
    //draw_entity(test_shader, box_entity);
    //draw_entity(test_shader, obstacle);
    //draw_entity(test_shader, floor_entity);
    for (int i = 0; i < NUM_BOXES; i++) {
      draw_entity(test_shader, boxes[i]);
    }
    for (int i = 0; i < NUM_SPHERES; i++) {
      draw_entity(test_shader, spheres[i]);
    }
    for (int i = 0; i < NUM_RECTS; i++) {
      draw_entity(test_shader, rects[i]);
    }
    /*
    for (int i = 0; i < ARENA_WIDTH * ARENA_WIDTH; i++) {
      draw_entity(test_shader, four_ent[i]);
    }
    */

    //vec3 pos = { 0.0, 0.0, 0.0 };
    //glUniform3f(glGetUniformLocation(test_shader, "col"), 1.0, 1.0, 0.0);
    //draw_oct_tree(cube, sim->oct_tree, pos, sim->oct_tree->max_extent,
    //              test_shader, 0, 1);

    /* Misc */
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    render_ui();
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Swap Buffers and Poll Events
    glfwSwapBuffers(window);

    glfwPollEvents();
  }

  free_sim(sim);
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
  free_ui();

  cleanup_gl();

  return 0;
}

void integrate_ragdoll(ENTITY *subject) {
  float increment = 0.001;

  for (int cur_bone = 0; cur_bone < subject->model->num_bones; cur_bone++) {
    int cur_col = subject->model->bone_collider_links[cur_bone];
    int collider_root_bone = subject->model->collider_bone_links[cur_col];

    if (collider_root_bone == cur_bone) {
      // Integrate acceleration
      float delta = subject->np_data[cur_col].accel_angle * increment;
      subject->np_data[cur_col].vel_angle *= 0.999;
      subject->np_data[cur_col].vel_angle += delta;
      remove_noise(subject->np_data[cur_col].vel_angle, 0.0001);

      vec6 delta_vec6 = VEC6_ZERO_INIT;
      vec6_scale(subject->np_data[cur_col].a_hat, increment, delta_vec6);
      vec6_scale(subject->np_data[cur_col].v_hat, 0.999,
                 subject->np_data[cur_col].v_hat);
      vec6_add(subject->np_data[cur_col].v_hat, delta_vec6,
               subject->np_data[cur_col].v_hat);
      vec6_remove_noise(subject->np_data[cur_col].v_hat, 0.0001);

      // Convert velocity from bone to world space
      mat3 bone_to_entity = GLM_MAT3_IDENTITY_INIT;
      glm_mat3_copy(subject->model->bones[cur_bone].coordinate_matrix,
                    bone_to_entity);

      mat3 entity_to_world = GLM_MAT3_IDENTITY_INIT;
      glm_mat4_pick3(subject->final_b_mats[cur_bone], entity_to_world);

      mat3 bone_to_world = GLM_MAT3_IDENTITY_INIT;
      glm_mat3_mul(entity_to_world, bone_to_entity, bone_to_world);

      float *vel = ((float *) subject->np_data[cur_col].v_hat) + 3;
      float *ang_vel = subject->np_data[cur_col].v_hat;

      vec3 world_vel = GLM_VEC3_ZERO_INIT;
      glm_mat3_mulv(bone_to_world, vel, world_vel);

      vec3 world_ang_vel = GLM_VEC3_ZERO_INIT;
      glm_mat3_mulv(bone_to_world, ang_vel, world_ang_vel);

      // Integrate velocity
      delta = subject->np_data[cur_col].vel_angle * increment;
      subject->np_data[cur_col].joint_angle += delta;

      vec3 delta_vec3 = GLM_VEC3_ZERO_INIT;
      glm_vec3_scale(world_vel, increment, delta_vec3);
      glm_translate(subject->bone_mats[cur_bone][LOCATION], delta_vec3);

      vec3 delta_rot = GLM_VEC3_ZERO_INIT;
      glm_vec3_scale(world_ang_vel, increment, delta_rot);
      versor rot_quat = GLM_QUAT_IDENTITY_INIT;
      glm_quatv(rot_quat, glm_vec3_norm(delta_rot), delta_rot);
      glm_quat_normalize(rot_quat);
      versor temp_quat = GLM_QUAT_IDENTITY_INIT;
      glm_mat4_quat(subject->bone_mats[cur_bone][ROTATION], temp_quat);
      glm_quat_normalize(temp_quat);
      glm_quat_mul(rot_quat, temp_quat, temp_quat);
      glm_quat_normalize(temp_quat);
      glm_quat_mat4(temp_quat, subject->bone_mats[cur_bone][ROTATION]);

      // Combine rotation, location and scale into final bone matrix
      vec3 temp = GLM_VEC3_ZERO_INIT;
      mat4 from_center = GLM_MAT4_IDENTITY_INIT;
      mat4 to_center = GLM_MAT4_IDENTITY_INIT;
      if (subject->model->colliders[cur_col].type == SPHERE) {
        glm_vec3_copy(subject->model->colliders[cur_col].data.center, temp);
      } else {
        glm_vec3_copy(subject->model->colliders[cur_col].data.center_of_mass,
                      temp);
      }
      glm_translate(to_center, temp);
      glm_vec3_negate(temp);
      glm_translate(from_center, temp);

      glm_mat4_identity(subject->final_b_mats[cur_bone]);
      glm_mat4_mul(from_center, subject->final_b_mats[cur_bone],
                   subject->final_b_mats[cur_bone]);
      glm_mat4_mul(subject->bone_mats[cur_bone][SCALE],
                   subject->final_b_mats[cur_bone],
                   subject->final_b_mats[cur_bone]);
      glm_mat4_mul(subject->bone_mats[cur_bone][ROTATION],
                   subject->final_b_mats[cur_bone],
                   subject->final_b_mats[cur_bone]);
      glm_mat4_mul(to_center, subject->final_b_mats[cur_bone],
                   subject->final_b_mats[cur_bone]);
      glm_mat4_mul(subject->bone_mats[cur_bone][LOCATION],
                   subject->final_b_mats[cur_bone],
                   subject->final_b_mats[cur_bone]);

      int parent_bone = subject->model->bones[cur_bone].parent;
      if (parent_bone != -1) {
        vec3 base_loc = GLM_VEC3_ZERO_INIT;
        glm_mat4_mulv3(subject->final_b_mats[cur_bone],
                       subject->model->bones[cur_bone].base, 1.0, base_loc);

        vec3 p_head_loc = GLM_VEC3_ZERO_INIT;
        glm_mat4_mulv3(subject->final_b_mats[parent_bone],
                       subject->model->bones[parent_bone].head, 1.0,
                       p_head_loc);

        vec3 anchor = GLM_VEC3_ZERO_INIT;
        glm_vec3_sub(p_head_loc, base_loc, anchor);
        mat4 anchor_mat = GLM_MAT4_IDENTITY_INIT;
        glm_translate(anchor_mat, anchor);
        glm_mat4_mul(anchor_mat, subject->final_b_mats[cur_bone],
                     subject->final_b_mats[cur_bone]);
        glm_mat4_mul(anchor_mat, subject->bone_mats[cur_bone][LOCATION],
                     subject->bone_mats[cur_bone][LOCATION]);
      }
    } else {
      glm_mat4_copy(subject->final_b_mats[collider_root_bone],
                    subject->final_b_mats[cur_bone]);
    }
  }
}

void keyboard_input(GLFWwindow *window) {
  float cam_speed = 4.0 * DELTA_TIME;
  glm_vec3_zero(movement);
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    camera_model_rot = glm_rad(-yaw + 180.0);

    glm_vec3_scale(camera_front, cam_speed, movement);
    glm_vec3_add(camera_model_pos, movement, camera_model_pos);
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    camera_model_rot = glm_rad(-yaw);

    glm_vec3_scale(camera_front, -cam_speed, movement);
    glm_vec3_add(camera_model_pos, movement, camera_model_pos);
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    camera_model_rot = glm_rad(-yaw -90.0);

    glm_vec3_cross(up, camera_front, movement);
    glm_vec3_normalize(movement);
    glm_vec3_scale(movement, cam_speed, movement);
    glm_vec3_add(camera_model_pos, movement, camera_model_pos);
  }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    camera_model_rot = glm_rad(-yaw + 90.0);

    glm_vec3_cross(camera_front, up, movement);
    glm_vec3_normalize(movement);
    glm_vec3_scale(movement, cam_speed, movement);
    glm_vec3_add(camera_model_pos, movement, camera_model_pos);
  }
  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
    if (space_pressed == 0) {
      movement[1] = 5.0;
      space_pressed = 1;
    }
  } else {
    space_pressed = 0;
  }
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, 1);
  }
  if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS &&
      glfwGetKey(window, GLFW_KEY_E) != GLFW_PRESS) {
    if (glfwGetTime() - last_push >= 0.05) {
      if (cur_frame == 59) {
        cur_frame = 0;
      }
      cur_frame++;
      last_push = glfwGetTime();
    }
  } else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS &&
             glfwGetKey(window, GLFW_KEY_Q) != GLFW_PRESS &&
             cur_frame > 0) {
    if (glfwGetTime() - last_push >= 0.05) {
      cur_frame--;
      last_push = glfwGetTime();
    }
  }
  if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) {
    if (toggled) {
      if (draw == 1) {
        draw = 0;
        CURSOR_ENABLED = 1;
        enable_gravity = 0;
      } else {
        draw = 1;
        CURSOR_ENABLED = 0;
        enable_gravity = 1;
      }
      toggled = 0;
    }
  } else {
    toggled = 1;
  }
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glm_perspective(glm_rad(45.0f), RES_X / RES_Y, 0.1f, 100.0f, persp_proj);
}

void mouse_input(GLFWwindow *window, double xpos, double ypos) {
  if (CURSOR_ENABLED == 0) {
    if (firstMouse) {
      lastX = xpos;
      lastY = yaw;
      firstMouse = 0;
    }

    float xOffset = xpos -lastX;
    float yOffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    const float sensitivity = 0.1f;
    xOffset *= sensitivity;
    yOffset *= sensitivity;

    yaw += xOffset;
    pitch += yOffset;

    if (pitch > 89.0f) {
      pitch = 89.0f;
    } else if (pitch < -89.0f) {
      pitch = -89.0f;
    }

    camera_front[0] = sin(glm_rad(yaw));
    camera_front[2] = -cos(glm_rad(yaw));
    glm_vec3_normalize(camera_front);
  }
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
  camera_offset[2] += yoffset;
  if (camera_offset[2] < -8.0) {
    camera_offset[2] = -8.0;
  }
  if (camera_offset[2] > -3.0) {
    camera_offset[2] = -3.0;
  }
}

void test_callback_click(UI_COMP *comp, void *args) {
  fprintf(stderr, "Clicked (%f, %f), %f, %f\n", comp->pos[X], comp->pos[Y],
          comp->width, comp->height);
}

void test_callback_release(UI_COMP *comp, void *args) {
  fprintf(stderr, "Released (%f, %f), %f, %f\n", comp->pos[X], comp->pos[Y],
          comp->width, comp->height);
}

void test_callback_hover(UI_COMP *comp, void *args) {
  UI_COMP *tool_tip = (UI_COMP *) args;
  set_ui_display(tool_tip, 1);
  set_ui_pos(tool_tip, (vec2) { MOUSE_POS[X], -MOUSE_POS[Y] });
}

void test_callback_no_hover(UI_COMP *comp, void *args) {
  UI_COMP *tool_tip = (UI_COMP *) args;
  set_ui_display(tool_tip, 0);
}

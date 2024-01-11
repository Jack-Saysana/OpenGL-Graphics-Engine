#include <main.h>

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
int cursor_on = 1;
int draw = 0;

// MISC DATA
int cur_frame = 0;
vec3 col_point = { 0.0, 0.0, 0.0 };
int enable_gravity = 1;

int featherstone_abm(ENTITY *body);
void integrate_ragdoll(ENTITY *subject);

int main() {
  GLFWwindow *window;

  if (!glfwInit()) {
    return -1;
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  window = glfwCreateWindow(RES_X, RES_Y, "Jack", NULL, NULL);
  if (window == NULL) {
    printf("Failed to create GLFW window\n");
    glfwTerminate();
    return -1;
  }

  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  glfwSetCursorPosCallback(window, mouse_input);
  glfwSetScrollCallback(window, scroll_callback);

  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    printf("Failed to initialize GLAD\n");
    glfwTerminate();
    return -1;
  }

  glViewport(0, 0, RES_X, RES_Y);

  int status = init_scene();
  if (status) {
    glfwTerminate();
    return -1;
  }

  vec3 cube_col = GLM_VEC3_ONE_INIT;
  vec3 s_col = GLM_VEC3_ONE_INIT;
  glm_quatv(player->rotation, camera_model_rot, up);

  mat4 ortho_proj = GLM_MAT4_IDENTITY_INIT;
  mat4 persp_proj = GLM_MAT4_IDENTITY_INIT;
  mat4 model = GLM_MAT4_IDENTITY_INIT;
  mat4 view = GLM_MAT4_IDENTITY_INIT;

  glm_ortho(-1.0, 1.0, -1.0, 1.0, 0.0, 100.0, ortho_proj);
  glm_perspective(glm_rad(45.0f), RES_X / RES_Y, 0.1f, 100.0f, persp_proj);

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

  glEnable(GL_DEPTH_TEST);

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
  init_ui(RES_X, RES_Y);
  add_ui_comp(UI_ROOT_COMP, (vec2) { 0.0, 0.0 }, 0.1, 0.1,
              RELATIVE_POS | POS_UNIT_RATIO | SIZE_UNIT_RATIO);
  add_ui_comp(UI_ROOT_COMP, (vec2) { 0.0, 0.0 }, 0.1, 0.1,
              RELATIVE_POS | POS_UNIT_RATIO | SIZE_UNIT_RATIO);
  add_ui_comp(UI_ROOT_COMP, (vec2) { 0.0, 0.0 }, 0.1, 0.1,
              RELATIVE_POS | POS_UNIT_RATIO | SIZE_UNIT_RATIO);

  // SIMULATION SET UP

  status = init_simulation();
  if (status != 0) {
    glfwTerminate();
  }

  player->type |= T_DRIVING;
  player->inv_mass = 1.0;
  status = insert_entity(player);
  if (status != 0) {
    glfwTerminate();
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
  status = insert_entity(ragdoll);
  if (status != 0) {
    glfwTerminate();
  }
  */

  // Insertion of test entities into the physics simulation
  /*
  obstacle->type |= T_DRIVING;// | T_IMMUTABLE;
  status = insert_entity(obstacle);
  if (status != 0) {
    glfwTerminate();
  }

  sphere_entity->type |= T_DRIVING;// | T_IMMUTABLE;
  status = insert_entity(sphere_entity);
  if (status != 0) {
    glfwTerminate();
  }

  box_entity->type |= T_DRIVING;// | T_IMMUTABLE;
  status = insert_entity(box_entity);
  if (status != 0) {
    glfwTerminate();
  }
  */

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

    status = insert_entity(boxes[i]);
    if (status) {
      glfwTerminate();
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

    status = insert_entity(spheres[i]);
    if (status) {
      glfwTerminate();
    }
  }

  for (int i = 0; i < NUM_RECTS; i++) {
    rects[i]->inv_mass = 1.0;
    vec3 init_vel = { 0.0, 0.0, 0.0 };
    glm_vec3_copy(init_vel, rects[i]->velocity);

    glm_mat4_identity(rects[i]->inv_inertia);
    rects[i]->inv_inertia[0][0] = (12.0 * rects[i]->inv_mass) / 1.25;
    rects[i]->inv_inertia[1][1] = (12.0 * rects[i]->inv_mass) / 5.0;
    rects[i]->inv_inertia[2][2] = (12.0 * rects[i]->inv_mass) / 4.25;

    status = insert_entity(rects[i]);
    if (status) {
      glfwTerminate();
    }
  }

  floor_entity->type |= T_DRIVING;// | T_IMMUTABLE;
  glm_mat4_zero(floor_entity->inv_inertia);
  status = insert_entity(floor_entity);
  if (status != 0) {
    glfwTerminate();
  }

  while (!glfwWindowShouldClose(window)) {
    if (cursor_on) {
      glfwSetInputMode(window, GLFW_CURSOR,GLFW_CURSOR_NORMAL);
    } else {
      glfwSetInputMode(window, GLFW_CURSOR,GLFW_CURSOR_DISABLED);
    }

    keyboard_input(window);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /* Animation */

    animate(player, 0, cur_frame);

    /* Physics */

    glm_vec3_add(movement, player->velocity, player->velocity);
    vec3 displacement = { 0.0, 0.0, 0.0 };
    glm_vec3_copy(player->translation, displacement);

    status = simulate_frame();
    if (status != 0) {
      break;
    }

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
    draw_colliders(basic_shader, floor_entity, sphere);
    for (int i = 0; i < NUM_BOXES; i++) {
      draw_colliders(basic_shader, boxes[i], sphere);
    }

    set_vec3("test_col", cube_col, basic_shader);
    //draw_colliders(basic_shader, box_entity, sphere);

    set_vec3("test_col", s_col, basic_shader);
    //draw_colliders(basic_shader, sphere_entity, sphere);

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
    set_vec3("test_col", (vec3) { 1.0, 1.0, 1.0 }, test_shader);
    //draw_entity(test_shader, box_entity);
    //draw_entity(test_shader, obstacle);
    draw_entity(test_shader, floor_entity);
    for (int i = 0; i < NUM_BOXES; i++) {
      draw_entity(test_shader, boxes[i]);
    }
    for (int i = 0; i < NUM_SPHERES; i++) {
      draw_entity(test_shader, spheres[i]);
    }
    for (int i = 0; i < NUM_RECTS; i++) {
      draw_entity(test_shader, rects[i]);
    }

    /* Misc */
    render_ui();

    // Swap Buffers and Poll Events
    glfwSwapBuffers(window);

    glfwPollEvents();
  }

  end_simulation();
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

vec3 quad_translate[8] = {
                       { 1.0, 1.0, 1.0 }, //  X, Y, Z
                       { 1.0, 1.0, -1.0 }, //  X, Y,-Z
                       { 1.0, -1.0, 1.0 }, //  X,-Y, Z
                       { 1.0, -1.0, -1.0 }, //  X,-Y,-Z
                       { -1.0, 1.0, 1.0 }, // -X, Y, Z
                       { -1.0, 1.0, -1.0 }, // -X, Y,-Z
                       { -1.0, -1.0, 1.0 }, // -X,-Y, Z
                       { -1.0, -1.0, -1.0 }  // -X,-Y,-Z
                      };
void draw_oct_tree(MODEL *cube, OCT_TREE *tree, vec3 pos, float scale,
                   unsigned int shader, size_t offset, int depth) {
  mat4 model = GLM_MAT4_IDENTITY_INIT;
  glm_translate(model, pos);
  glm_scale_uni(model, scale);
  glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1,
                     GL_FALSE, (float *) model);
  draw_model(shader, cube);

  vec3 temp = { 0.0, 0.0, 0.0 };
  if (tree->node_buffer[offset].next_offset != -1 && depth < 5) {
    for (int i = 0; i < 8; i++) {
      glUniform3f(glGetUniformLocation(shader, "test_col"), 1.0, 1.0, 0.0);
      glm_vec3_scale(quad_translate[i], scale / 2.0, temp);
      glm_vec3_add(pos, temp, temp);
      draw_oct_tree(cube, tree, temp, scale / 2.0, shader,
                    tree->node_buffer[offset].next_offset + i, depth + 1);
    }
  }
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

void keyboard_input(GLFWwindow *window) {
  float cam_speed = 4.0 * delta_time;
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
        cursor_on = 1;
        enable_gravity = 0;
      } else {
        draw = 1;
        cursor_on = 0;
        enable_gravity = 1;
      }
      toggled = 0;
    }
  } else {
    toggled = 1;
  }
}

void mouse_input(GLFWwindow *window, double xpos, double ypos) {
  if (cursor_on == 0) {
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

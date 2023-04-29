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

vec3 m_box_pos = { 7.0, 4.0, -2.0 };
vec3 m_box_scale = { 0.5, 0.5, 0.5 };
vec3 m_sphere_pos = { 1.0, 3.0, -3.0 };
vec3 m_sphere_scale = { 0.5, 0.5, 0.5 };
vec3 m_rect_pos = { 1.0, 3.0, 3.0 };
vec3 m_rect_scale = { 0.5, 0.5, 0.5 };
vec3 ob_pos = { 3.0, 0.0, -3.0 };
vec3 floor_scale = { 50.0, 1.0, 50.0 };
vec3 cube_pos = { 3.0, 2.0, 3.0 };
vec3 s_pos = { -3.0, 2.0, 3.0 };

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

  mat4 projection = GLM_MAT4_IDENTITY_INIT;
  mat4 model = GLM_MAT4_IDENTITY_INIT;
  mat4 view = GLM_MAT4_IDENTITY_INIT;

  glm_perspective(glm_rad(45.0f), RES_X / RES_Y, 0.1f, 100.0f, projection);
  glUseProgram(shader);
  glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1,
                     GL_FALSE, (float *)projection);

  glUseProgram(u_shader);
  glUniformMatrix4fv(glGetUniformLocation(u_shader, "projection"), 1,
                     GL_FALSE, (float *)projection);

  glUseProgram(bone_shader);
  glUniformMatrix4fv(glGetUniformLocation(bone_shader, "projection"), 1,
                     GL_FALSE, (float *)projection);

  glUseProgram(basic_shader);
  glUniformMatrix4fv(glGetUniformLocation(basic_shader, "projection"), 1,
                     GL_FALSE, (float *)projection);

  glUseProgram(test_shader);
  glUniformMatrix4fv(glGetUniformLocation(test_shader, "projection"), 1,
                     GL_FALSE, (float *)projection);

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

  /*m_box_entity->inv_mass = 4.0;
  vec3 init_vel = { 0.0, 0.001, 0.0 };
  glm_vec3_copy(init_vel, m_box_entity->velocity);
  //versor init_rot = { 0.0, 0.0, 0.5, 1.0 };
  //glm_quat_normalize(init_rot);
  //glm_quat_copy(init_rot, m_box_entity->rotation);
  //vec3 init_ang_vel = { 0.015, 0.0, 0.015 };
  //glm_vec3_copy(init_ang_vel, m_box_entity->ang_velocity);
  status = insert_entity(m_box_entity);
  if (status != 0) {
    glfwTerminate();
  }*/
  for (int i = 0; i < NUM_BOXES; i++) {
    boxes[i]->inv_mass = 2.0;
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

    animate(player, 1, cur_frame);

    /* Physics */

    glm_vec3_add(movement, player->velocity, player->velocity);
    vec3 displacement = { 0.0, 0.0, 0.0 };
    glm_vec3_copy(player->translation, displacement);

    status = simulate_frame();
    if (status != 0) {
      break;
    }
    /*fprintf(stderr, "%f %f %f\n",
            boxes[0]->ang_velocity[0],
            boxes[0]->ang_velocity[1],
            boxes[0]->ang_velocity[2]);
    fflush(stderr);*/
    //fprintf(stderr, "%f\n", glm_vec3_norm(boxes[0]->ang_velocity));
    //fflush(stderr);

    glm_vec3_sub(player->translation, displacement, displacement);
    glm_vec3_add(displacement, camera_model_pos, camera_model_pos);

    /* Camera */

    vec3 translation = { -camera_model_pos[0], -camera_model_pos[1],
                         -camera_model_pos[2] };

    glm_mat4_identity(view);
    camera_offset[1] = -dude->bones[18].coords[1];
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
    glUniformMatrix4fv(glGetUniformLocation(basic_shader, "model"), 1,
                       GL_FALSE, (float *) model);
    glUniformMatrix4fv(glGetUniformLocation(basic_shader, "view"), 1,
                       GL_FALSE, (float *) view);
    glUniform3f(glGetUniformLocation(basic_shader, "test_col"), 0.0, 1.0, 1.0);
    glBindVertexArray(pt_VAO);
    glDrawArrays(GL_POINTS, 0, 1);

    /* Test collision point */

    glm_translate(model, col_point);
    glUniformMatrix4fv(glGetUniformLocation(basic_shader, "model"), 1,
                       GL_FALSE, (float *) model);
    glUniform3f(glGetUniformLocation(basic_shader, "test_col"), 1.0, 0.0, 0.0);
    glDrawArrays(GL_POINTS, 0, 1);
    glBindVertexArray(0);

    /* Skeleton */

    glm_vec3_copy(camera_model_pos, player->translation);
    glm_quatv(player->rotation, camera_model_rot, up);

    glUseProgram(bone_shader);
    glUniform3f(glGetUniformLocation(bone_shader, "test_col"), 0.0, 0.0, 1.0);
    glUniformMatrix4fv(glGetUniformLocation(bone_shader, "view"), 1,
                       GL_FALSE, (float *) view);

    draw_skeleton(bone_shader, player);

    /* Colliders */

    glPointSize(5.0);
    glUseProgram(basic_shader);
    glUniform3f(glGetUniformLocation(basic_shader, "test_col"), 1.0, 0.0, 1.0);
    glBindVertexArray(pt_VAO);
    draw_colliders(basic_shader, player, sphere);
    draw_colliders(basic_shader, obstacle, sphere);
    draw_colliders(basic_shader, floor_entity, sphere);
    for (int i = 0; i < NUM_BOXES; i++) {
      draw_colliders(basic_shader, boxes[i], sphere);
    }

    glUniform3f(glGetUniformLocation(basic_shader, "test_col"), cube_col[0],
                cube_col[1], cube_col[2]);
    draw_colliders(basic_shader, box_entity, sphere);

    glUniform3f(glGetUniformLocation(basic_shader, "test_col"), s_col[0],
                s_col[1], s_col[2]);
    draw_colliders(basic_shader, sphere_entity, sphere);

    /* Player */

    glUseProgram(shader);
    glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1,
                       GL_FALSE, (float *) view);
    glUniform3f(glGetUniformLocation(shader, "camera_pos"), camera_pos[0],
                camera_pos[1], camera_pos[2]);
    if (draw) {
      draw_entity(shader, player);
    }

    /* Objects */

    glUseProgram(test_shader);
    glUniformMatrix4fv(glGetUniformLocation(test_shader, "view"), 1, GL_FALSE,
                       (float *) view);

    glUniform3f(glGetUniformLocation(test_shader, "test_col"), 1.0, 1.0, 1.0);
    draw_entity(test_shader, box_entity);
    draw_entity(test_shader, obstacle);
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

    // Swap Buffers and Poll Events
    glfwSwapBuffers(window);

    glfwPollEvents();
  }

  end_simulation();
  free_entity(player);
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

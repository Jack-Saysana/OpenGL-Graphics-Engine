#include <main.h>

#define LINUX (1)
#define LAPTOP (0)
#define PC (0)

#if LINUX == 1
#define DIR "/home/jbs/Documents/C/OpenGL-Graphics-Engine"
#elif LAPTOP == 1
#define DIR "C:/Users/jackm/Documents/C/OpenGL-Graphics-Engine"
#elif PC == 1
#define DIR "C:/Users/Jack/Documents/C/OpenGL-Graphics-Engine"
#else
#define DIR ""
#endif


vec3 up = { 0.0, 1.0, 0.0 };

float delta_time = 0.0;
float last_frame = 0.0;

vec3 camera_offset = { 0.0, 0.0, -5.0 };
vec3 camera_front = { 0.0, 0.0, -1.0 };
vec3 camera_pos = { 0.0, 0.0, 0.0 };
vec3 camera_model_pos = { 5.0, 0.0, 5.0 };
float camera_model_rot = 0.0;

float lastX = 400;
float lastY = 300;

float pitch = 0;
float yaw = 0;

int firstMouse = 1;

int cur_frame = 0;
float last_push = 0.0;

int toggled = 1;
int draw = 0;

int main() {
  GLFWwindow *window;

  if (!glfwInit()) {
    return -1;
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  window = glfwCreateWindow(640, 480, "Jack", NULL, NULL);
  if (window == NULL) {
    printf("Failed to create GLFW window\n");
    glfwTerminate();
    return -1;
  }

  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  glfwSetInputMode(window, GLFW_CURSOR,GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(window, mouse_input);
  glfwSetScrollCallback(window, scroll_callback);

  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    printf("Failed to initialize GLAD\n");
    glfwTerminate();
    return -1;
  }

  glViewport(0, 0, 640, 480);

  unsigned int shader = init_shader_prog(
      DIR"/src/shaders/cell_shader/shader.vs",
      NULL,
      DIR"/src/shaders/cell_shader/shader.fs"
      );
  if (shader == -1) {
    printf("Error loading shaders\n");
    glfwTerminate();
    return -1;
  }

  unsigned int u_shader = init_shader_prog(
      DIR"/src/shaders/unanimated/shader.vs",
      NULL,
      DIR"/src/shaders/unanimated/shader.fs"
      );
  if (u_shader == -1) {
    printf("Error loading shaders\n");
    glfwTerminate();
    return -1;
  }

  unsigned int b_shader = init_shader_prog(
      DIR"/src/shaders/bone/shader.vs",
      NULL,
      DIR"/src/shaders/bone/shader.fs"
      );
  if (b_shader == -1) {
    printf("Error loading bone shaders\n");
    glfwTerminate();
    return -1;
  }

  unsigned int origin_shader = init_shader_prog(
      DIR"/src/shaders/origin/shader.vs",
      NULL,
      DIR"/src/shaders/origin/shader.fs"
      );
  if (origin_shader == -1) {
    printf("Error loading origin shaders\n");
    glfwTerminate();
    return -1;
  }

  unsigned int test_shader = init_shader_prog(
      DIR"/src/shaders/unanimated/shader.vs",
      NULL,
      DIR"/src/shaders/test/shader.fs"
      );
  if (test_shader == -1) {
    printf("Error loading test shaders\n");
    glfwTerminate();
    return -1;
  }

  MODEL *cube = load_model(
      DIR"/resources/cube/cube.obj"
      );
  if (cube == NULL) {
    printf("Unable to load cube model\n");
    glfwTerminate();
    return -1;
  }

  MODEL *dude = load_model(
      DIR"/resources/low_poly_new/low_poly_new.obj"
      );
  if (dude == NULL) {
    printf("Unable to load dude model\n");
    glfwTerminate();
    return -1;
  }

  MODEL *test = load_model(
      DIR"/resources/test/test.obj"
      );
  if (test == NULL) {
    printf("Unable to load test model\n");
    glfwTerminate();
    return -1;
  }

  MODEL *floor = load_model(
      DIR"/resources/floor/floor.obj"
      );
  if (floor == NULL) {
    printf("Unable to load floor model\n");
    glfwTerminate();
    return -1;
  }

  OCT_TREE *tree = init_tree();
  if (tree == NULL) {
    printf("Unable to load oct-tree\n");
    glfwTerminate();
    return -1;
  }

  ENTITY *player = init_entity(dude);
  if (player == NULL) {
    printf("Unable to load player\n");
    glfwTerminate();
    return -1;
  }

  COLLIDER dude_hb = {{{ 0.20, 1.75, 0.1 },
                       { 0.20, 1.75, -0.1 },
                       { -0.20, 1.75, 0.1 },
                       { -0.20, 1.75, -0.1 },
                       { 0.20, 0.0, 0.1 },
                       { 0.20, 0.0, -0.1 },
                       { -0.20, 0.0, 0.1 },
                       { -0.20, 0.0, -0.1}
                      }, 8};

  dude->colliders[0] = dude_hb;

  glm_translate(player->model_mat, camera_model_pos);
  glm_rotate_y(player->model_mat, camera_model_rot, player->model_mat);
  int status = oct_tree_insert(tree, player, 0);
  if (status != 0) {
    printf("Failed to insert dude1\n");
  }

  mat4 projection = GLM_MAT4_IDENTITY_INIT;
  mat4 model = GLM_MAT4_IDENTITY_INIT;
  mat4 view = GLM_MAT4_IDENTITY_INIT;


  glm_perspective(glm_rad(45.0f), 800.0f / 600.0f, 0.1f, 100.0f, projection);
  glUseProgram(shader);
  glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1,
                     GL_FALSE, (float *)projection);

  glUseProgram(u_shader);
  glUniformMatrix4fv(glGetUniformLocation(u_shader, "projection"), 1,
                     GL_FALSE, (float *)projection);

  glUseProgram(b_shader);
  glUniformMatrix4fv(glGetUniformLocation(b_shader, "projection"), 1,
                     GL_FALSE, (float *)projection);

  glUseProgram(origin_shader);
  glUniformMatrix4fv(glGetUniformLocation(origin_shader, "projection"), 1,
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

  float until_next = 0.0;
  while (!glfwWindowShouldClose(window)) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float current_time = glfwGetTime();
    delta_time = current_time - last_frame;
    last_frame = current_time;

    until_next += delta_time;

    keyboard_input(window);

    /* Animation */

    animate(player, 0, cur_frame);

    /* Collision */

    oct_tree_delete(tree, player->tree_offsets[0]);
    oct_tree_insert(tree, player, 0);

    // Render

    /* Camera */

    vec3 translation = { -camera_model_pos[0], 0.0, -camera_model_pos[2] };

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
    glUseProgram(origin_shader);
    glUniformMatrix4fv(glGetUniformLocation(origin_shader, "model"), 1,
                       GL_FALSE, (float *) model);
    glUniformMatrix4fv(glGetUniformLocation(origin_shader, "view"), 1,
                       GL_FALSE, (float *) view);
    glBindVertexArray(pt_VAO);
    glDrawArrays(GL_POINTS, 0, 1);
    glBindVertexArray(0);

    /* Oct-Tree */

    /* Skeleton */

    glm_translate(model, camera_model_pos);
    glm_rotate_y(model, camera_model_rot, model);

    glUseProgram(b_shader);
    glUniformMatrix4fv(glGetUniformLocation(b_shader, "view"), 1,
                       GL_FALSE, (float *) view);

    glm_mat4_copy(model, player->model_mat);
    draw_skeleton(b_shader, player);

    /* Skin */

    glUseProgram(shader);
    glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1,
                       GL_FALSE, (float *) view);
    glUniform3f(glGetUniformLocation(shader, "camera_pos"), camera_pos[0],
                camera_pos[1], camera_pos[2]);
    if (draw) {
      draw_entity(shader, player);
    }

    /* Unanimated models */

    glUseProgram(u_shader);
    glUniformMatrix4fv(glGetUniformLocation(u_shader, "view"), 1,
                       GL_FALSE, (float *) view);
    glUniform3f(glGetUniformLocation(u_shader, "camera_pos"), camera_pos[0],
                camera_pos[1], camera_pos[2]);

    glm_mat4_identity(model);
    glm_vec3_zero(translation);
    translation[0] = 5.0;
    translation[1] = 10.0;
    translation[2] = -15.0;
    glm_translate(model, translation);
    glUniformMatrix4fv(glGetUniformLocation(u_shader, "model"), 1,
                       GL_FALSE, (float *) model);
    draw_model(u_shader, dude);

    glm_mat4_identity(model);
    glm_vec3_zero(translation);
    translation[0] = 50.0;
    translation[1] = 50.0;
    translation[2] = 50.0;
    glm_scale(model, translation);
    glUniformMatrix4fv(glGetUniformLocation(u_shader, "model"), 1,
                       GL_FALSE, (float *) model);
    draw_model(u_shader, floor);

    /* Tests */

    glUseProgram(test_shader);
    glUniformMatrix4fv(glGetUniformLocation(test_shader, "view"), 1, GL_FALSE,
                       (float *) view);
    glUniform3f(glGetUniformLocation(test_shader, "test_col"), 1.0, 1.0, 0.0);

    vec3 pos = { 0.0, 0.0, 0.0 };
    draw_oct_tree(cube, tree, pos, 16.0, test_shader, 0, 1);
    
    // Swap Buffers and Poll Events
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  free_entity(player);

  free_model(cube);
  free_model(test);
  free_model(dude);
  free_model(floor);

  free_oct_tree(tree);

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
  float cam_speed = 5.0 * delta_time;
  vec3 movement = { 0.0, 0.0, 0.0 };
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
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, 1);
  }
  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS &&
      glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) != GLFW_PRESS) {
    if (glfwGetTime() - last_push >= 0.125) {
      if (cur_frame == 39) {
        cur_frame = 0;
      }
      cur_frame++;
      last_push = glfwGetTime();
    }
  } else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS &&
             glfwGetKey(window, GLFW_KEY_SPACE) != GLFW_PRESS &&
             cur_frame > 0) {
    if (glfwGetTime() - last_push >= 0.125) {
      cur_frame--;
      last_push = glfwGetTime();
    }
  }
  if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) {
    if (toggled) {
      if (draw == 1) {
        draw = 0;
      } else {
        draw = 1;
      }
      toggled = 0;
    }
  } else {
    toggled = 1;
  }
}

void mouse_input(GLFWwindow *window, double xpos, double ypos) {
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

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
  camera_offset[2] += yoffset;
  if (camera_offset[2] < -8.0) {
    camera_offset[2] = -8.0;
  }
  if (camera_offset[2] > -3.0) {
    camera_offset[2] = -3.0;
  }
}

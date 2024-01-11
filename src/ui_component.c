#include <ui_component.h>

/*
  Initializes root component of the UI. Must me called only once before
  further creation/usage of UI components.

  Parameters:
  float res_x: Screen width in pixels
  float res_y: Screen height in pixels

  Returns:
  0 if successful
  -1 if error has occured
*/
int init_ui(float res_x, float res_y) {
  // Initialize root ui component
  int status = init_ui_comp(&ui_root, (vec2) {0.0, 0.0}, res_x, res_y,
                            PIVOT_CENTER, T_CENTER,
                            ABSOLUTE_POS | POS_UNIT_PIXEL | SIZE_UNIT_PIXEL,
                            UI_ENABLED);
  if (status) {
    fprintf(stderr, "Error initializing root ui component.\n");
    return -1;
  }

  glm_vec2_copy((vec2) { 0.0, 0.0 }, ui_root.pix_pos);
  ui_root.pix_width = res_x;
  ui_root.pix_height = res_y;

  // Initialized base quad model used for rendering ui components
  ui_quad = load_model("resources/quad/quad.obj");
  if (ui_quad == NULL) {
    free_ui_comp(&ui_root);
    fprintf(stderr, "Error loading ui quad models\n");
    return -1;
  }

  // Initialize shader used for rendering ui components
  ui_shader = init_shader_prog(
      "src/shaders/ui/shader.vs",
      NULL,
      "src/shaders/ui/shader.fs"
      );
  if (ui_shader == -1) {
    free_ui_comp(&ui_root);
    free_model(ui_quad);
    fprintf(stderr, "Error loading ui shaders\n");
    return -1;
  }

  // Initialize render stack
  render_stack = malloc(sizeof(UI_COMP *) * STK_SIZE_INIT);
  if (render_stack == NULL) {
    return -1;
  }
  render_stk_top = 0;
  render_stk_size = STK_SIZE_INIT;

  // Initialize matrix used for rendering ui components
  mat4 ui_proj = GLM_MAT4_IDENTITY_INIT;
  glm_ortho(-1.0, 1.0, -1.0, 1.0, 0.0, 100.0, ui_proj);

  glUseProgram(ui_shader);
  set_mat4("proj", ui_proj, ui_shader);

  return 0;
}

/*
  Main API function for adding components to the rendering list

  Parameters:
  - UI_COMP *parent: Pointer to UI component which will be the parent of the
    new component
  - vec2 pos: Position of the new UI component
  - float width: Width of the new UI component
  - float height: Height of the new UI component
  - int options: A bit string specifying the interpretation of pos, width and
    height
    - Option 1: Absolute vs Relative positioning
      - ABSOLUTE_POS: Pos represents an offset from the parent component's top
        left corner
      - RELATIVE_POS: Pos represents an offset from the automatically
        calculated position of the component, which places the component next
        to it's child components, where children are arranged from left to
        right

    - Option 2: Ratio vs Pixel position unit
      - POS_UNIT_RATIO: Pos[X] is a ratio of the parent component's width
                        Pos[Y] is a ratio of the parent component's height
      - POS_UNIT_PIXEL: Pos[X] and pos[Y] are pixel offsets

    - Option 3: Ratio vs pixel size unit
      - SIZE_UNIT_RATIO: width is a ratio of the parent component's width
                         height is a ratio of the parent component's height
      - SIZE_UNIT_PIXEL: width and height are in units of pixels
  - PIVOT pivot: Point on the new component which is located at pos
*/
int add_ui_comp(UI_COMP *parent, vec2 pos, float width, float height,
                int options) {
  int status = init_ui_comp(parent->children + parent->num_children, pos,
                            width, height, PIVOT_TOP_LEFT, T_CENTER, options,
                            UI_ENABLED);
  if (status) {
    fprintf(stderr, "Unable to initialize ui component\n");
    return -1;
  }

  parent->num_children++;
  if (parent->num_children == parent->child_buf_size) {
    status = double_buffer((void **) parent->children, &parent->child_buf_size,
                           sizeof(UI_COMP));
    if (status) {
      parent->num_children--;
      free_ui_comp(parent->children + parent->num_children);
      fprintf(stderr, "Unable to allocate ui component\n");
      return -1;
    }
  }

  return 0;
}

/*
  Initialize a new UI component given the position, width, height, pivot, text
  anchor, numerical options and enabled variables.
*/
int init_ui_comp(UI_COMP *comp, vec2 pos, float width, float height,
                  PIVOT pivot, TEXT_ANCHOR txt_anc, int opts, int enabled) {
  comp->children = malloc(sizeof(UI_COMP) * CHILD_BUF_SIZE_INIT);
  if (comp->children == NULL) {
    return -1;
  }
  comp->num_children = 0;
  comp->child_buf_size = CHILD_BUF_SIZE_INIT;

  glm_vec2_copy(pos, comp->pos);
  comp->width = width;
  comp->height = height;
  comp->pivot = pivot;
  comp->txt_anc = txt_anc;
  comp->numerical_options = opts;
  comp->enabled = enabled;
  return 0;
}

void free_ui_comp(UI_COMP *comp) {
  free(comp->children);
}

/*
  Render a singular UI component to the screen. Assumes the pixel information
  regarding the components size and position are already calculated
*/
void render_comp(UI_COMP *comp) {
  float screen_width = comp->pix_width / ui_root.pix_width;
  float screen_height = comp->pix_height / ui_root.pix_height;

  vec2 pivot_offset = {
                        UI_PIVOT_OFFSETS[comp->pivot][X] * screen_width * 0.5,
                        UI_PIVOT_OFFSETS[comp->pivot][Y] * screen_height * 0.5
                      };
  vec2 screen_pos = {
                      ((comp->pix_pos[X] / ui_root.pix_width) +
                       pivot_offset[X]) * 2.0,
                      ((comp->pix_pos[Y] / ui_root.pix_height) +
                       pivot_offset[Y]) * 2.0
                    };

  mat4 comp_model_mat = GLM_MAT4_IDENTITY_INIT;
  // Scale quad to component width
  glm_translate(comp_model_mat, (vec3) { screen_pos[X], screen_pos[Y], 0.0 });
  glm_scale(comp_model_mat, (vec3) { screen_width, screen_height, 1.0 });

  // Draw quad
  glUseProgram(ui_shader);
  set_mat4("model", comp_model_mat, ui_shader);
  draw_model(ui_shader, ui_quad);
}

/*
  Traverses the tree starting at the root ui component, calculating the pixel
  information of each component before rendering them.
*/
int render_ui() {
  render_stk_top = 1;

  render_comp(&ui_root);
  render_stack[0] = &ui_root;

  UI_COMP *cur_comp = NULL;
  UI_COMP *cur_child = NULL;
  vec2 top_left = GLM_VEC2_ZERO_INIT;
  vec2 next_rel_pos = GLM_VEC2_ZERO_INIT;
  vec2 cur_offset = GLM_VEC2_ZERO_INIT;
  while (render_stk_top) {
    render_stk_top--;
    cur_comp = render_stack[render_stk_top];

    // Get pixel coordinate of parent's center
    vec2 pivot_offset = {
                          UI_PIVOT_OFFSETS[cur_comp->pivot][X] *
                          cur_comp->pix_width * 0.5,
                          UI_PIVOT_OFFSETS[cur_comp->pivot][Y] *
                          cur_comp->pix_height * 0.5
                        };
    glm_vec2_sub(cur_comp->pix_pos, pivot_offset,
                 top_left);

    // Get pixel coordinate of top left of parent
    glm_vec2_add(top_left, (vec2) { -cur_comp->pix_width * 0.5,
                 cur_comp->pix_height * 0.5 }, top_left);
    glm_vec2_copy(top_left, next_rel_pos);

    for (size_t i = 0; i < cur_comp->num_children; i++) {
      cur_child = cur_comp->children + i;
      if (!cur_child->enabled) {
        continue;
      }

      // Calculate child pixel sizing
      if (cur_child->numerical_options & SIZE_UNIT_PIXEL) {
        // Component is sized based on pixels
        cur_child->pix_width = cur_child->width;
        cur_child->pix_height = cur_child->height;
      } else {
        // Component is sized relative to parent
        cur_child->pix_width = cur_comp->pix_width * cur_child->width;
        cur_child->pix_height = cur_comp->pix_height * cur_child->height;
      }

      // Calculate child pixel offset
      if (cur_child->numerical_options & POS_UNIT_PIXEL) {
        // Component offset is measured in pixels
        glm_vec2_copy(cur_child->pos, cur_offset);
      } else {
        // Component offset is measured relative to size of parent
        cur_offset[X] = cur_comp->pix_width * cur_child->pos[X];
        cur_offset[Y] = cur_comp->pix_height * cur_child->pos[Y];
      }

      // Calculate child pixel position
      if (cur_child->numerical_options & ABSOLUTE_POS) {
        // Component is offset from top left of parent
        glm_vec2_copy(top_left, cur_child->pix_pos);
      } else {
        // Component is offset from default relative placement
        glm_vec2_copy(next_rel_pos, cur_child->pix_pos);
        next_rel_pos[X] += cur_child->pix_width;
        next_rel_pos[X] += cur_offset[X];
      }
      glm_vec2_add(cur_child->pix_pos, cur_offset, cur_child->pix_pos);

      // Render child component
      render_comp(cur_child);

      // Push child component on stack
      render_stack[render_stk_top] = cur_child;
      render_stk_top++;
      if (render_stk_top == render_stk_size) {
        int status = double_buffer((void **) render_stack, &render_stk_size,
                                   sizeof(UI_COMP *));
        if (status) {
          free(render_stack);
          fprintf(stderr, "UI Error: Unable to grow ui render stack\n");
          return -1;
        }
      }
    }
  }
  //render_comp(&ui_root);

  return 0;
}

#include <ui_component.h>

// ============================== INITIALIZATION =============================

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
int init_ui() {
  // Initialize root ui component
  int status = init_ui_comp(&ui_root, "", GLM_VEC3_ZERO, GLM_VEC2_ZERO, RES_X,
                            RES_Y, PIVOT_CENTER, T_CENTER,
                            ABSOLUTE_POS | POS_UNIT_PIXEL | SIZE_UNIT_PIXEL,
                            UI_TRUE, UI_FALSE);
  if (status) {
    fprintf(stderr, "Error initializing root ui component.\n");
    return -1;
  }

  glm_vec3_copy((vec3) { 0.0, 0.0, 0.0 }, ui_root.pix_pos);
  ui_root.pix_width = RES_X;
  ui_root.pix_height = RES_Y;

  // Initialized base quad model used for rendering ui components
  ui_quad = load_model("resources/quad/quad.obj");
  if (ui_quad == NULL) {
    free_ui_comp(&ui_root);
    fprintf(stderr, "Error loading ui quad models\n");
    return -1;
  }

  // Initialize shaders used for rendering ui components
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

  text_shader = init_shader_prog(
      "src/shaders/font/shader.vs",
      NULL,
      "src/shaders/font/shader.fs"
      );
  if (text_shader == -1) {
    free_ui_comp(&ui_root);
    free_model(ui_quad);
    glDeleteProgram(text_shader);
    fprintf(stderr, "Error loading ui text shaders\n");
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

  glUseProgram(text_shader);
  set_mat4("proj", ui_proj, text_shader);

  return 0;
}

/*
  Initialize a new UI component given the position, width, height, pivot, text
  anchor, numerical options and enabled variables.
*/
int init_ui_comp(UI_COMP *comp, char *text, vec3 text_col, vec2 pos,
                 float width, float height, PIVOT pivot, TEXT_ANCHOR txt_anc,
                 int opts, int enabled, int display) {
  comp->children = malloc(sizeof(UI_COMP) * CHILD_BUF_SIZE_INIT);
  if (comp->children == NULL) {
    return -1;
  }
  comp->num_children = 0;
  comp->child_buf_size = CHILD_BUF_SIZE_INIT;

  comp->text = text;
  comp->text_len = strlen(text);
  glm_vec3_copy(text_col, comp->text_col);

  glm_vec2_copy(pos, comp->pos);
  comp->width = width;
  comp->height = height;
  comp->pivot = pivot;
  comp->txt_anc = txt_anc;
  comp->numerical_options = opts;
  comp->display = display;
  comp->enabled = enabled;
  return 0;
}

// ================================= CLEANUP =================================

int free_ui() {
  render_stack[0] = &ui_root;
  render_stk_top = 1;

  UI_COMP *cur_comp = NULL;
  while (render_stk_top) {
    cur_comp = render_stack[render_stk_top - 1];
    if (cur_comp->num_children) {
      for (size_t i = 0; i < cur_comp->num_children; i++) {
        render_stack[render_stk_top] = cur_comp->children + i;
        render_stk_top++;
        if (render_stk_top == render_stk_size) {
          int status = double_buffer((void **) &render_stack, &render_stk_size,
                                     sizeof(UI_COMP *));
          if (status) {
            fprintf(stderr, "UI Error: Unable to grow ui free stack\n");
            return -1;
          }
        }
      }

      cur_comp->num_children = 0;
    } else {
      free_ui_comp(cur_comp);
      render_stk_top--;
    }
  }

  free(render_stack);
  free_model(ui_quad);
  glDeleteProgram(ui_shader);
  glDeleteProgram(text_shader);

  ui_quad = NULL;
  render_stack = NULL;

  return 0;
}

void free_ui_comp(UI_COMP *comp) {
  free(comp->children);
}

// ============================ ADDING COMPONENTS ============================

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
      - POS_X_UNIT_RATIO_X: Pos[X] is a ratio of the parent component's width
      - POS_X_UNIT_RATIO_Y: Pos[X] is a ratio of the parent component's height
      - POS_X_UNIT_PIXEL: Pos[Y] is a pixel offset

      - POS_Y_UNIT_RATIO_X: Pos[Y] is a ratio of the parent component's width
      - POS_Y_UNIT_RATIO_Y: Pos[Y] is a ratio of the parent component's height
      - POS_Y_UNIT_PIXEL: Pos[Y] is a pixel offset

      - POS_UNIT_RATIO_X: Pos[X] is a ratio of the parent component's width
                          Pos[Y] is a ratio of the parent component's width
                       (equivalent to POS_X_UNIT_RATIO_X | POS_Y_UNIT_RATIO_X)

      - POS_UNIT_RATIO_Y: Pos[X] is a ratio of the parent component's height
                          Pos[Y] is a ratio of the parent component's height
                       (equivalent to POS_X_UNIT_RATIO_Y | POS_Y_UNIT_RATIO_Y)

      - POS_UNIT_PIXEL: Pos[X] and pos[Y] are pixel offsets
                       (equivalent to POS_X_UNIT_PIXEL | POS_Y_UNIT_PIXEL)

      - POS_UNIT_RATIO: Pos[X] is a ratio of the parent component's width
                        Pos[Y] is a ratio of the parent component's height
                       (equivalent to POS_X_UNIT_RATIO_X | POS_Y_UNIT_RATIO_Y)

    - Option 3: Ratio vs pixel size unit
      - WIDTH_UNIT_RATIO_X: Width is a ratio of the parent component's width
      - WIDTH_UNIT_RATIO_Y: Width is a ratio of the parent component's height
      - WIDTH_UNIT_PIXEL: Width is in units of pixels

      - HEIGHT_UNIT_RATIO_X: Height is a ratio of the parent component's width
      - HEIGHT_UNIT_RATIO_Y: Height is a ratio of the parent component's height
      - HEIGHT_UNIT_PIXEL: Height is in units of pixels

      - LINE_UNIT_RATIO_X: Text line height is a ratio of the component's width
      - LINE_UNIT_RATIO_Y: Text line height is a ratio of the component's
                           height
      - LINE_UNIT_RATIO_PIXEL: Text line height is in units of pixels

      - SIZE_UNIT_RATIO_X: width is a ratio of the parent component's width
                           height is a ratio of the parent component's width
                           line height is a ratio of the component's width
        (equivalent to WIDTH_UNIT_RATIO_X | HEIGHT_UNIT_RATIO_X |
                       LINE_UNIT_RATIO_X)

      - SIZE_UNIT_RATIO_Y: width is a ratio of the parent component's height
                           height is a ratio of the parent component's height
                           line height is a ratio of the component's height
        (equivalent to WIDTH_UNIT_RATIO_Y | HEIGHT_UNIT_RATIO_Y |
                       LINE_UNIT_RATIO_Y)

      - SIZE_UNIT_PIXEL: width, height and line height are in units of pixels
        (equivalent to WIDTH_UNIT_PIXEL | HEIGHT_UNIT_PIXEL |
                       LINE_UNIT_RATIO_PIXEL)

      - SIZE_UNIT_RATIO: width is a ratio of the parent component's width
                         height is a ratio of the parent component's height
                         text line height is a ratio of component's height
        (equivalent to WIDTH_UNIT_RATIO_X | HEIGHT_UNIT_RATIO_Y |
                       LINE_UNIT_RATIO_Y)

  - PIVOT pivot: Point on the new component which is located at pos
*/
UI_COMP *add_ui_comp(UI_COMP *parent, vec2 pos, float width, float height,
                     int options) {
  UI_COMP *comp = parent->children + parent->num_children;
  int status = init_ui_comp(comp, "", GLM_VEC3_ZERO, pos, width, height,
                            PIVOT_TOP_LEFT, T_CENTER, options, UI_TRUE,
                            UI_TRUE);
  if (status) {
    fprintf(stderr, "Unable to initialize ui component\n");
    return NULL;
  }

  parent->num_children++;
  if (parent->num_children == parent->child_buf_size) {
    status = double_buffer((void **) &parent->children,
                           &parent->child_buf_size, sizeof(UI_COMP));
    if (status) {
      parent->num_children--;
      free_ui_comp(parent->children + parent->num_children);
      fprintf(stderr, "Unable to allocate ui component\n");
      return NULL;
    }
  }

  return comp;
}

// ============================ EDITING COMPONENTS ===========================

void set_pivot(UI_COMP *comp, PIVOT pivot) {
  comp->pivot = pivot;
}

void set_display(UI_COMP *comp, int display) {
  comp->display = display;
}

void set_text(UI_COMP *comp, char *str, float line_height, vec3 col) {
  comp->text = str;
  comp->text_len = strlen(str);
  comp->line_height = line_height;
  glm_vec3_copy(col, comp->text_col);
}

void set_text_col(UI_COMP *comp, vec3 col) {
  glm_vec3_copy(col, comp->text_col);
}

// ================================ RENDERING ================================

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
                      (comp->pix_pos[X] / ui_root.pix_width) +
                      pivot_offset[X],
                      (comp->pix_pos[Y] / ui_root.pix_height) +
                      pivot_offset[Y]
                    };

  mat4 comp_model_mat = GLM_MAT4_IDENTITY_INIT;
  // Scale quad to component width
  glm_translate(comp_model_mat, (vec3) { screen_pos[X] * 2.0,
                                         screen_pos[Y] * 2.0,
                                         -comp->pix_pos[Z]});
  glm_scale(comp_model_mat, (vec3) { screen_width * 2.0, screen_height * 2.0,
                                     1.0 });

  // Draw quad
  glUseProgram(ui_shader);
  set_mat4("model", comp_model_mat, ui_shader);
  draw_model(ui_shader, ui_quad);

  // Draw text
  if (comp->text_len) {
    draw_text(comp->text, comp->text_len, comp->text_col, comp->txt_anc,
              (vec2) { screen_pos[X] * ui_root.pix_width,
                       screen_pos[Y] * ui_root.pix_height }, ui_root.pix_width,
              ui_root.pix_height, comp->pix_width, comp->pix_line_height,
              text_shader);
  }
}

/*
  Traverses the tree starting at the root ui component, calculating the pixel
  information of each component before rendering them.
*/
int render_ui() {
  if (!ui_root.enabled) {
    return 0;
  }

  render_stk_top = 1;

  ui_root.pix_width = RES_X;
  ui_root.pix_height = RES_Y;
  if (ui_root.display) {
    render_comp(&ui_root);
  }
  render_stack[0] = &ui_root;

  UI_COMP *cur_comp = NULL;
  UI_COMP *cur_child = NULL;
  vec2 top_left = GLM_VEC2_ZERO_INIT;
  vec2 next_rel_pos = GLM_VEC2_ZERO_INIT;
  float next_line_y = 0.0;
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

    // Get pixel coordinate of top left of parent
    glm_vec2_sub(cur_comp->pix_pos, pivot_offset,
                 top_left);
    glm_vec2_add(top_left, (vec2) { -cur_comp->pix_width * 0.5,
                 cur_comp->pix_height * 0.5 }, top_left);
    glm_vec2_copy(top_left, next_rel_pos);
    next_line_y = top_left[Y];

    for (size_t i = 0; i < cur_comp->num_children; i++) {
      cur_child = cur_comp->children + i;
      if (!cur_child->enabled) {
        continue;
      }

      // Calculate pixelized position and size of child component
      calc_pix_stats(cur_comp, cur_child, top_left, next_rel_pos,
                     &next_line_y);

      // Render child component
      if (cur_child->display) {
        render_comp(cur_child);
      }

      // Push child component on stack
      render_stack[render_stk_top] = cur_child;
      render_stk_top++;
      if (render_stk_top == render_stk_size) {
        int status = double_buffer((void **) &render_stack, &render_stk_size,
                                   sizeof(UI_COMP *));
        if (status) {
          fprintf(stderr, "UI Error: Unable to grow ui render stack\n");
          return -1;
        }
      }
    }
  }

  return 0;
}

// ================================= HELPERS =================================

void calc_pix_stats(UI_COMP *parent, UI_COMP *child, vec2 top_left,
                    vec2 next_rel_pos, float *next_line_y) {
  vec2 cur_offset = GLM_VEC2_ZERO_INIT;

  // Calculate child pixel sizing
  int width_opt = child->numerical_options & WIDTH_UNIT_PIXEL;
  if (width_opt == WIDTH_UNIT_RATIO_X) {
    // Width measured relative to width of parent
    child->pix_width = parent->pix_width * child->width;
  } else if (width_opt == WIDTH_UNIT_RATIO_Y) {
    // Width measured relative to height of parent
    child->pix_width = parent->pix_height * child->width;
  } else {
    // Width measured in pixels
    child->pix_width = child->width;
  }

  int height_opt = child->numerical_options & HEIGHT_UNIT_PIXEL;
  if (height_opt == HEIGHT_UNIT_RATIO_X) {
    // Height measured relative to width of parent
    child->pix_height = parent->pix_width * child->height;
  } else if (height_opt == HEIGHT_UNIT_RATIO_Y) {
    // Height measured relative to height of parent
    child->pix_height = parent->pix_height * child->height;
  } else {
    // Height measured in pixels
    child->pix_height = child->height;
  }

  int line_height_opt = child->numerical_options & LINE_UNIT_PIXEL;
  if (line_height_opt == LINE_UNIT_RATIO_X) {
    // Line height measured relative to width of child
    child->pix_line_height = child->pix_width * child->line_height;
  } else if (line_height_opt == LINE_UNIT_RATIO_Y) {
    // Line height measured relative to height of child
    child->pix_line_height = child->pix_height * child->line_height;
  } else {
    // Line height measured in pixels
    child->pix_line_height = child->line_height;
  }

  // Calculate child pixel offset
  int pos_x_opt = child->numerical_options & POS_X_UNIT_PIXEL;
  if (pos_x_opt == POS_X_UNIT_RATIO_X) {
    // X offset is measured relative to width of parent
    cur_offset[X] = parent->pix_width * child->pos[X];
  } else if (pos_x_opt == POS_X_UNIT_RATIO_Y) {
    // X offset is measured relative to height of parent
    cur_offset[X] = parent->pix_height * child->pos[X];
  } else {
    // X offset is measured in pixels
    cur_offset[X] = child->pos[X];
  }

  int pos_y_opt = child->numerical_options & POS_Y_UNIT_PIXEL;
  if (pos_y_opt == POS_Y_UNIT_RATIO_X) {
    // Y offset measured relative to width of parent
    cur_offset[Y] = parent->pix_width * child->pos[Y];
  } else if (pos_y_opt == POS_Y_UNIT_RATIO_Y) {
    // Y offset measured relative to height of parent
    cur_offset[Y] = parent->pix_height * child->pos[Y];
  } else {
    // Y offset measured in pixels
    cur_offset[Y] = child->pos[Y];
  }

  // Calculate child pixel position
  if (child->numerical_options & ABSOLUTE_POS) {
    // Component is offset from top left of parent
    glm_vec2_copy(top_left, child->pix_pos);
  } else {
    // Component is offset from default relative placement
    float parent_max_x = ((UI_PIVOT_OFFSETS[parent->pivot][X] -
                           UI_PIVOT_OFFSETS[PIVOT_RIGHT][X]) *
                          parent->pix_width * 0.5) + parent->pix_pos[X];

    vec2 child_b_right = GLM_VEC2_ZERO_INIT;
    glm_vec2_sub(UI_PIVOT_OFFSETS[child->pivot],
                 UI_PIVOT_OFFSETS[PIVOT_BOTTOM_RIGHT], child_b_right);
    float child_max_x = (child_b_right[X] * child->pix_width * 0.5) +
                        next_rel_pos[X] + cur_offset[X];
    if (child_max_x > parent_max_x) {
      next_rel_pos[X] = top_left[X];
      next_rel_pos[Y] = *next_line_y;
    }

    glm_vec2_copy(next_rel_pos, child->pix_pos);

    float child_min_y = (child_b_right[Y] * child->pix_height * 0.5) +
                        child->pix_pos[Y] + cur_offset[Y];
    if (child_min_y < *next_line_y) {
      *next_line_y = child_min_y;
    }

    // Calculate pixel offset from pix_pos to center of the component
    vec2 pivot_compensation = {
      UI_PIVOT_OFFSETS[child->pivot][X] * child->width * 0.5,
      UI_PIVOT_OFFSETS[child->pivot][Y] * child->height * 0.5
    };

    // Update default position for next sibling component
    next_rel_pos[X] += cur_offset[X] + (child->pix_width * 0.5) +
                       pivot_compensation[X];
  }
  glm_vec2_add(child->pix_pos, cur_offset, child->pix_pos);
  child->pix_pos[Z] = parent->pix_pos[Z] + 0.001;
}

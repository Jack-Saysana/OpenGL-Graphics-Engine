#include <font.h>

/*
  Main API function for importing font binaries.

  Parameters:
  - char *bin_path: Path to font binary to import
  - char *tex_path: Path to font texture image
  - F_GLYPH **dest: Pointer to buffer to be generated and populated with font
    data

  Returns:
  number of glyphs generated from font binary
*/
int import_font(char *bin_path, char *tex_path, F_GLYPH **dest) {
  FILE *in_file = fopen(bin_path, "rb");
  if (in_file == NULL) {
    fprintf(stderr, "Error: Unable to import font at \"%s\"\n", bin_path);
    return -1;
  }

  int num_glyphs = 0;
  float base = 0.0;
  fread(&num_glyphs, sizeof(int), 1, in_file);
  fread(&base, sizeof(float), 1, in_file);

  // Read number of glyphs to read
  *dest = malloc(sizeof(F_GLYPH) * num_glyphs);
  if (*dest == NULL) {
    fprintf(stderr, "Error: Unable to allocate font buffer\n");
    return -1;
  }

  vec3 v[4] = { {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0},
                {0.0, 0.0, 0.0} };
  vec2 t[4] = { {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0} };
  ivec3 i[2] = { {0, 0, 0}, {0, 0, 0} };

  // Read each glyph
  for (int k = 0; k < num_glyphs; k++) {
    F_GLYPH *cur_char = (*dest) + k;

    fread(&v, sizeof(v), 1, in_file);
    fread(&t, sizeof(t), 1, in_file);
    fread(&i, sizeof(i), 1, in_file);
    fread(&cur_char->height, sizeof(float), 1, in_file);
    fread(&cur_char->width, sizeof(float), 1, in_file);
    fread(&cur_char->advance, sizeof(float), 1, in_file);
    fread(&cur_char->x_offset, sizeof(float), 1, in_file);
    fread(&cur_char->y_offset, sizeof(float), 1, in_file);
    fread(&cur_char->id, sizeof(int), 1, in_file);
    cur_char->base = base;

    // Structure VBO format
    struct vertex {
      vec3 coords;
      vec2 tex_coords;
    } vertices[4] = {
      { { v[0][X], v[0][Y], v[0][Z] }, { t[0][X], t[0][Y] } },
      { { v[1][X], v[1][Y], v[1][Z] }, { t[1][X], t[1][Y] } },
      { { v[2][X], v[2][Y], v[2][Z] }, { t[2][X], t[2][Y] } },
      { { v[3][X], v[3][Y], v[3][Z] }, { t[3][X], t[3][Y] } }
    };

    /*
    fprintf(stderr, "%d (%c):\n", cur_char->id, cur_char->id);
    fprintf(stderr, "  { %f, %f, %f } { %f, %f }\n", vertices[0].coords[0],
            vertices[0].coords[1], vertices[0].coords[2],
            vertices[0].tex_coords[0], vertices[0].tex_coords[1]);
    fprintf(stderr, "  { %f, %f, %f } { %f, %f }\n", vertices[1].coords[0],
            vertices[1].coords[1], vertices[1].coords[2],
            vertices[1].tex_coords[0], vertices[1].tex_coords[1]);
    fprintf(stderr, "  { %f, %f, %f } { %f, %f }\n", vertices[2].coords[0],
            vertices[2].coords[1], vertices[2].coords[2],
            vertices[2].tex_coords[0], vertices[2].tex_coords[1]);
    fprintf(stderr, "  { %f, %f, %f } { %f, %f }\n", vertices[3].coords[0],
            vertices[3].coords[1], vertices[3].coords[2],
            vertices[3].tex_coords[0], vertices[3].tex_coords[1]);
    fprintf(stderr, "  height: %f, width: %f, advance: %f\n", cur_char->height,
            cur_char->width, cur_char->advance);
    */

    // Generate glyph mesh
    glGenVertexArrays(1, &cur_char->VAO);
    glBindVertexArray(cur_char->VAO);

    glGenBuffers(1, &cur_char->VBO);
    glBindBuffer(GL_ARRAY_BUFFER, cur_char->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &cur_char->EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cur_char->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(i), i, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct vertex),
                          (void *) 0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(struct vertex),
                          (void *) (sizeof(float) * 3));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    gen_texture_id(tex_path, &cur_char->texture);
  }

  return 0;
}

/*
  Frees a font buffer
*/
void free_font(F_GLYPH *glyphs, int num_glyphs) {
  for (int i = 0; i < num_glyphs; i++) {
    glDeleteVertexArrays(1, &(glyphs[i].VAO));
    glDeleteBuffers(1, &(glyphs[i].VBO));
    glDeleteBuffers(1, &(glyphs[i].EBO));
  }
  free(glyphs);
}

// ================================ RENDERING ================================

int draw_text(char *str, size_t str_len, vec3 col, TEXT_ANCHOR txt_anc,
              vec3 pos, float screen_width, float screen_height, float width,
              float line_height, F_GLYPH* font, unsigned int shader) {
  // Calculate the number of lines in the text and the length of each line
  float *line_widths = malloc(sizeof(float) * BUFF_START_LEN);
  if (line_widths == NULL) {
    fprintf(stderr, "Error: Failed to allocate text line buffer\n");
    return -1;
  }
  size_t line_buf_size = BUFF_START_LEN;
  size_t num_lines = 0;
  line_widths[num_lines] = 0.0;

  F_GLYPH *cur_char = NULL;
  F_GLYPH *prev_char = NULL;
  float cur_width = 0.0;
  float cur_x_off = 0.0;
  float cur_adv = 0.0;
  float prev_width = 0.0;
  float prev_x_off = 0.0;
  float prev_adv = 0.0;
  for (int i = 0; i < str_len; i++) {
    cur_char = font + (str[i] - ' ');
    if (str[i] == '\n') {
      num_lines++;
      if (num_lines == line_buf_size) {
        int status = double_buffer((void **) &line_widths, &line_buf_size,
                                   sizeof(float));
        if (status) {
          fprintf(stderr, "Error: Failed to relllocate text line buffer\n");
          return -1;
        }
      }

      if (line_widths[num_lines - 1] != 0.0) {
        prev_char = font + (str[i - 1] - ' ');
        prev_width = prev_char->width * line_height;
        prev_x_off = prev_char->x_offset * line_height;
        prev_adv = prev_char->advance * line_height;
        line_widths[num_lines - 1] += (prev_width - prev_x_off - prev_adv);
      }

      line_widths[num_lines] = 0.0;

      continue;
    } else if (str[i] < ' ') {
      cur_char = font + ' ';
    }

    cur_width = cur_char->width * line_height;
    cur_x_off = cur_char->x_offset * line_height;
    cur_adv = cur_char->advance * line_height;

    float delta_width = cur_adv;
    if (line_widths[num_lines] == 0.0) {
      delta_width += cur_x_off;
    }

    if (line_widths[num_lines] + (cur_width - cur_x_off) > width) {
      num_lines++;
      if (num_lines == line_buf_size) {
        int status = double_buffer((void **) &line_widths, &line_buf_size,
                                   sizeof(float));
        if (status) {
          fprintf(stderr, "Error: Failed to relllocate text line buffer\n");
          return -1;
        }
      }

      if (line_widths[num_lines - 1] != 0.0) {
        prev_char = font + (str[i - 1] - ' ');
        prev_width = prev_char->width * line_height;
        prev_x_off = prev_char->x_offset * line_height;
        prev_adv = prev_char->advance * line_height;
        line_widths[num_lines - 1] += (prev_width - prev_x_off - prev_adv);
        delta_width += cur_x_off;
      }

      line_widths[num_lines] = 0.0;
    }

    line_widths[num_lines] += delta_width;
  }
  if (line_widths[num_lines]) {
    if (cur_adv > cur_width - cur_x_off) {
      line_widths[num_lines] -= (cur_adv - (cur_width - cur_x_off));
    } else if (cur_adv < cur_width - cur_x_off) {
      line_widths[num_lines] += (cur_width - cur_x_off);
    }
  }
  //line_widths[num_lines] += (cur_width - cur_x_off - cur_adv);
  num_lines++;

  // Render characters
  float start_y = pos[Y];
  if (num_lines % 2 == 0) {
    start_y += (((num_lines / 2) - 1) * line_height);
  } else {
    start_y += (((num_lines / 2) - 0.5) * line_height);
  }

  mat4 glyph_model_mat = GLM_MAT4_IDENTITY_INIT;
  size_t cur_line = 0;
  float cur_len = 0.0;
  vec2 cur_pos = { 0.0, start_y };
  for (int i = 0; i < str_len; i++) {
    cur_char = font + (str[i] - ' ');
    if (str[i] == '\n') {
      cur_line++;
      cur_pos[Y] -= line_height;
      cur_len = 0.0;
      continue;
    } else if (str[i] < ' ') {
      cur_char = font + ' ';
    }

    cur_width = cur_char->width * line_height;
    cur_x_off = cur_char->x_offset * line_height;
    cur_adv = cur_char->advance * line_height;

    float delta_width = cur_adv;
    if (cur_len == 0.0) {
      delta_width += cur_x_off;

      cur_pos[X] = get_next_x(pos, line_widths[cur_line], width, cur_x_off,
                              txt_anc);
    }

    if (cur_len + (cur_width - cur_x_off) > width) {
      cur_line++;
      if (cur_len != 0.0) {
        delta_width += cur_x_off;
      }

      cur_pos[Y] -= line_height;
      cur_pos[X] = get_next_x(pos, line_widths[cur_line], width, cur_x_off,
                              txt_anc);

      cur_len = 0.0;
    }

    cur_len += delta_width;

    glUseProgram(shader);
    glm_mat4_identity(glyph_model_mat);
    glm_translate(glyph_model_mat,
                  (vec3) { 2.0 * (cur_pos[X] / screen_width),
                           2.0 * (cur_pos[Y] / screen_height),
                           pos[Z] + 0.001 });
    glm_scale(glyph_model_mat,
              (vec3) { 2.0 * (line_height / screen_width),
                       2.0 * (line_height / screen_height), 1.0 });
    set_mat4("model", glyph_model_mat, shader);
    set_vec3("text_col", col, shader);
    draw_glyph(cur_char, shader);

    cur_pos[X] += cur_adv;
  }

  free(line_widths);
  return 0;
}

/*
  Draw a single character from a font
*/
void draw_glyph(F_GLYPH *glyph, unsigned int shader) {
  glUseProgram(shader);
  glBindVertexArray(glyph->VAO);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, glyph->texture);
  set_int("font_map", 0, shader);

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}

// ================================= HELPERS =================================

float get_next_x(vec2 pos, float line_width, float comp_width, float x_off,
                 TEXT_ANCHOR txt_anc) {
  if (txt_anc == T_CENTER) {
    return pos[X] - (line_width * 0.5) + x_off;
  } else if (txt_anc == T_LEFT) {
    return pos[X] - (comp_width * 0.5) + x_off;
  } else {
    return pos[X] + (comp_width * 0.5) - line_width + x_off;
  }
}

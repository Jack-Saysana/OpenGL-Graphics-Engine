#include <helpers.h>

/*
 * ========== DOUBLE_BUFFER() ==========
 *
 * IMPORTANT! IF THE FUNCTION FAILS, THE BUFFER IS NOT DEALLOCATED
 *
 * DESC
 * Doubles the given buffer
 *
 * ARGS
 * - void** buffer: Pointer to the buffer being doubled
 * - size_t *buff_size: Pointer to the number recording the siZe
 * of the buffer
 *
 * RETURNS
 * 0 if successful
 * -1 if an error has occured
 * =====================================
 */
int double_buffer(void **buffer, size_t *buff_size, size_t unit_size) {
  void *new_buff = realloc(*buffer, 2 * (*buff_size) * unit_size);
  if (new_buff == NULL) {
    return -1;
  }
  (*buffer) = new_buff;
  *buff_size = 2 * *buff_size;
  return 0;
}

/*
 * ========== GET_LINES() ==========
 * DESC: Retrieves an array of the lines of a
 * file
 *
 * ARGS:
 * char *path: Path to file being read
 *
 * RETURNS:
 * A pointer to a LINE_BUFFER struct which holds the individual lines
 * of the file and the number of lines
 *
 * IMPORTANT:
 * The returned LINE_BUFFER * must be freed using
 * free_line_buffer()
 * =================================
 */
LINE_BUFFER *get_lines(char *path) {
  int status = 0;
  FILE *file = fopen(path, "r");
  if (file == NULL) {
    printf("Unable to open file at path: %s\n", path);
    return NULL;
  }

  char *file_contents = malloc(FILE_CONTENTS_STARTING_LEN);
  if (file_contents == NULL) {
    printf("Unable to allocate file contents\n");
    fclose(file);
    return NULL;
  }
  size_t contents_len = 0;
  size_t contents_buffer_len = FILE_CONTENTS_STARTING_LEN;

  int next = fgetc(file);
  while (next != EOF) {
    file_contents[contents_len] = next;
    contents_len++;
    if (contents_len == contents_buffer_len) {
      status = double_buffer((void **) &file_contents, &contents_buffer_len,
                             sizeof(char));
      if (status != 0) {
        free(file_contents);
        fclose(file);
        printf("Unable to allocate file contents\n");
        return NULL;
      }
    }
    next = fgetc(file);
  }
  file_contents[contents_len] = '\n';
  contents_len++;
  fclose(file);

  LINE_BUFFER *lb = malloc(sizeof(LINE_BUFFER));
  if (lb == NULL) {
    free(file_contents);
    printf("Unable to allocate line buffer struct\n");
    return NULL;
  }

  lb->dir = malloc(strlen(path) + 1);
  if (lb->dir == NULL) {
    free(lb);
    free(file_contents);
    return NULL;
  }
  strncpy(lb->dir, path, strlen(path) + 1);
  for (int i = strlen(lb->dir) - 1; i >= 0; i--) {
    if (lb->dir[i] == '\\' || lb->dir[i] == '/') {
      lb->dir[i] = '\0';
      lb->filename = lb->dir + i + 1;
      break;
    }
  }

  lb->buffer = malloc(sizeof(char *) * LINE_BUFF_STARTING_LEN);
  if (lb->buffer == NULL) {
    free(lb->dir);
    free(lb);
    free(file_contents);
    printf("Unable to allocate line buffer\n");
    return NULL;
  }
  size_t line_buffer_max = LINE_BUFF_STARTING_LEN;
  lb->len = 0;

 char *cur_line = file_contents;
  for (int i = 0; i < contents_len; i++) {
    if (file_contents[i] == '\n') {
      file_contents[i] = '\0';
      lb->buffer[lb->len] = cur_line;
      lb->len++;
      if (lb->len == line_buffer_max) {
        status = double_buffer((void **) &(lb->buffer), &line_buffer_max,
                               sizeof(char *));
        if (status != 0) {
          free(lb->buffer);
          free(lb->dir);
          free(lb);
          free(file_contents);
          printf("Unable to reallocate line buffer\n");
          return NULL;
        }
      }

      cur_line = file_contents + i + 1;
    }
  }

  return lb;
}

/*
 * ========== FREE_LINE_BUFFER ==========
 *
 * Deallocates a given line buffer
 *
 * ======================================
 */
void free_line_buffer(LINE_BUFFER *lb) {
  free(lb->dir);
  free(lb->buffer[0]);
  free(lb->buffer);
  free(lb);
}

/*
 * ========= GET_STR_HASH ==========
 *
 * Hash string to an integer
 *
 * =============================
 */
size_t get_str_hash(char *str) {
  int weight = 17;
  int str_len = strlen(str);

  uint64_t hash = 0;
  for (int i = 0; i < str_len && i < 10; i++) {
    hash += (weight * str[i]);
    weight *= weight;
  }

  uint64_t y;
  uint64_t x = hash;
  int n = 64;
  y = x >> 32;
  if (y != 0) {
    n -= 32;
    x = y;
  }
  y = x >> 16;
  if (y != 0) {
    n -= 16;
    x = y;
  }
  y = x >> 8;
  if (y != 0) {
    n -= 8;
    x = y;
  }
  y = x >> 4;
  if (y != 0) {
    n -= 4;
    x = y;
  }
  y = x >> 2;
  if (y != 0) {
    n -= 2;
    x = y;
  }
  y = x >> 1;
  if (y != 0) {
    n -= 1;
  }

  hash = hash << n;
  hash = hash | str_len;
  hash = hash & ~1;

  return hash;
}

char *remove_double_dot(char *path) {
  char *buffer = malloc(sizeof(char) * strlen(path) + 1);
  size_t index = 0;
  char prev[2] = { '\0', '\0' };
  char cur = '\0';
  for (size_t i = 0; i < strlen(path); i++) {
    cur = path[i];
    if (cur == '/' && prev[0] == '.' && prev[1] == '.') {
      for (size_t j = index - 4; j >= 0; j--) {
        if (j == 0 || buffer[j] == '/') {
          index = j;
          break;
        }
      }
    }
    buffer[index] = cur;
    index++;

    prev[1] = prev[0];
    prev[0] = cur;
  }
  buffer[index] = '\0';

  return buffer;
}

/*
 * ================ VEC3_REMOVE_NOISE ====================
 *
 * Zeros out members of a vec3 given a specific threshold
 *
 * =======================================================
 */
void vec3_remove_noise(vec3 vec, float threshold) {
  if (vec[0] < threshold && vec[0] > -threshold) {
    vec[0] = 0.0;
  }
  if (vec[1] < threshold && vec[1] > -threshold) {
    vec[1] = 0.0;
  }
  if (vec[2] < threshold && vec[2] > -threshold) {
    vec[2] = 0.0;
  }
}

/*
 * ================== REMOVE_NOISE =======================
 *
 * Zeros out a float given a specific threshold
 *
 * =======================================================
 */
float remove_noise(float val, float threshold) {
  if (val < threshold && val > -threshold) {
    return 0;
  } else {
    return val;
  }
}

/*
 * ================= GET_LSB ================
 *
 * Returns number with all bits but its least significant set bit set to 0
 *
 * ==========================================
 */
int get_lsb(int x) {
  return x & -x;
}

/*
  The following functions are helpers to consicely set up uniform variables in
  shaders
*/
void set_mat4(char *name, mat4 matrix, unsigned int shader) {
  glUniformMatrix4fv(glGetUniformLocation(shader, name), 1, GL_FALSE,
                     (float *) matrix);
}

void set_mat3(char *name, mat3 matrix, unsigned int shader) {
  glUniformMatrix3fv(glGetUniformLocation(shader, name), 1, GL_FALSE,
                     (float *) matrix);
}

void set_vec4(char *name, vec4 matrix, unsigned int shader) {
  glUniform4fv(glGetUniformLocation(shader, name), 1, (float *) matrix);
}

void set_vec3(char *name, vec3 v, unsigned int shader) {
  glUniform3fv(glGetUniformLocation(shader, name), 1, (float *) v);
}

void set_vec2(char *name, vec2 v, unsigned int shader) {
  glUniform2fv(glGetUniformLocation(shader, name), 1, (float *) v);
}

void set_float(char *name, float val, unsigned int shader) {
  glUniform1f(glGetUniformLocation(shader, name), val);
}

void set_int(char *name, int val, unsigned int shader) {
  glUniform1i(glGetUniformLocation(shader, name), val);
}

void set_uint(char *name, unsigned int val, unsigned int shader) {
  glUniform1ui(glGetUniformLocation(shader, name), val);
}

void set_iarr(char *name, int *arr, size_t count, unsigned int shader) {
  glUniform1iv(glGetUniformLocation(shader, name), count, arr);
}

void set_texture(char *name, unsigned int tex, unsigned int shader, int unit) {
  glUseProgram(shader);
  glActiveTexture(GL_TEXTURE0 + unit);
  glBindTexture(GL_TEXTURE_2D, tex);
  glUniform1i(glGetUniformLocation(shader, name), unit);
}

/*
  The following functions are helpers to concisely print out the vector and
  matrix types provided by cglm
*/
void print_vec3(vec3 v) {
  printf("%f %f %f\n", v[0], v[1], v[2]);
}

void print_vec4(vec4 v) {
  printf("%f %f %f %f\n", v[0], v[1], v[2], v[3]);
}

void print_mat3(mat3 m) {
  for (int i = 0; i < 3; i++) {
    printf("|%f %f %f|\n", m[0][i], m[1][i], m[2][i]);
  }
}

void print_mat4(mat4 m) {
  for (int i = 0; i < 4; i++) {
    printf("|%f %f %f %f|\n", m[0][i], m[1][i], m[2][i], m[3][i]);
  }
}

/*
  The following functions are helpers to concisely compare vectors
*/

int comp_vec2(vec2 a, vec2 b) {
  return a[X] == b[X] && a[Y] == b[Y];
}

int comp_vec3(vec3 a, vec3 b) {
  return a[X] == b[X] && a[Y] == b[Y] && a[Z] == b[Z];
}

int comp_vec4(vec4 a, vec4 b) {
  return a[X] == b[X] && a[Y] == b[Y] && a[Z] == b[Z] && a[W] == b[W];
}

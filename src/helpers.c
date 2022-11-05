#include <helpers.h>

/*
 * ========== DOUBLE_BUFFER() ==========
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
    free(*buffer);
    return -1;
  }
  (*buffer) = new_buff;
  *buff_size = 2 * *buff_size;
  return 0;
}

/*
 * ========== DOUBLE_COMPLEX_BUFFER() ==========
 * DESC
 * Doubles the given buffer
 *
 * ARGS
 * - void** buffer: Pointer to the buffer being doubled
 * - size_t *buff_size: Pointer to the number recording the siZe
 * of the buffer
 * - size_t buff_len: Number of elements inside buffer
 * - void free_buff(void *, size_t): Function to be called when
 * freeing the buffer upon failure. Best used when needing to also
 * free individual elements of the buffer. First argument will take
 * (*buffer) and second will take buff_len
 *
 * RETURNS
 * 0 if successful
 * -1 if an error has occured
 * =====================================
 */
int double_complex_buffer(void **buffer, size_t *buff_size, size_t buff_len,
                          size_t unit_size, void free_buff(void *, size_t)) {
  void *new_buff = realloc(*buffer, 2 * (*buff_size) * unit_size);
  if (new_buff == NULL) {
    free_buff(*buffer, buff_len);
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
    printf("Unable to open object file\n");
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
          free(file_contents);
          free(lb);
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
 * ========= GET_HASH ==========
 *
 * Hash string to an integer
 *
 * =============================
 */
uint64_t get_hash(char *str) {
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

  return hash;
}

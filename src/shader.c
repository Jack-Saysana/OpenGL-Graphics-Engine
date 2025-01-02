#include <shader.h>

unsigned int init_shader_prog(char *v_path, char *g_path, char *f_path) {
  long vs = -1;
  long gs = -1;
  long fs = -1;

  char *v_source = load_source(v_path);
  if (v_source == NULL) {
    fprintf(stderr, "Invalid vertex shader path\n");
    return -1;
  }
  fprintf(stderr, "%s:\n", v_path);
  vs = gen_shader(v_source, GL_VERTEX_SHADER);
  if (vs == -1) {
    free(v_source);
    return -1;
  }
  free(v_source);

  char *f_source = load_source(f_path);
  if (f_source == NULL) {
    fprintf(stderr, "Invalid fragment shader path\n");
    return -1;
  }
  fprintf(stderr, "%s:\n", f_path);
  fs = gen_shader(f_source, GL_FRAGMENT_SHADER);
  if (fs == -1) {
    free(f_source);
    return -1;
  }
  free(f_source);

  if (g_path) {
    char *g_source = load_source(g_path);
    if (g_source == NULL) {
      fprintf(stderr, "Invalid geometry shader path\n");
      return -1;
    }
    fprintf(stderr, "%s:\n", g_path);
    gs = gen_shader(g_source, GL_GEOMETRY_SHADER);
    if (gs == -1) {
      free(g_source);
      return -1;
    }
    free(g_source);
  }

  unsigned int program = glCreateProgram();
  if (!program) {
    fprintf(stderr, "Failed to create shader program\n");
    return -1;
  }

  glAttachShader(program, vs);
  GLenum err = glGetError();
  if (err != GL_NO_ERROR) {
    fprintf(stderr, "Failed to attach vertex shader\n");
  }

  glAttachShader(program, fs);
  err = glGetError();
  if (err != GL_NO_ERROR) {
    fprintf(stderr, "Failed to attach fragment shader\n");
  }

  if (gs != -1) {
    glAttachShader(program, gs);
    err = glGetError();
    if (err != GL_NO_ERROR) {
      fprintf(stderr, "Failed to attach geometry shader\n");
    }
  }

  glLinkProgram(program);
  err = glGetError();
  if (err != GL_NO_ERROR) {
    fprintf(stderr, "Failed to link shader program\n");
  }

  glDeleteShader(vs);
  glDeleteShader(fs);
  if (gs != -1) {
    glDeleteShader(gs);
  }

  return program;
}

long gen_shader(const char *source, GLenum type) {
  if (source == NULL) {
    fprintf(stderr, "Invalid shader source code\n");
    return -1;
  }

  int source_len = strlen(source);
  unsigned int shader = glCreateShader(type);
  glShaderSource(shader, 1, &source, &source_len);
  glCompileShader(shader);

  int compilation_status = -1;
  char *info_log = NULL;
  int info_log_len = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compilation_status);
  if (compilation_status == GL_FALSE) {
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_log_len);
    info_log = malloc(info_log_len);
    if (info_log == NULL) {
      fprintf(stderr, "Shader error. Unable to allocate info log.\n");
      return -1;
    }

    glGetShaderInfoLog(shader, info_log_len, &info_log_len, info_log);
    fprintf(stderr, "%s\n", info_log);
    free(info_log);
    return -1;
  } else {
    return shader;
  }
}

char *load_source(char *path) {
  if (path == NULL) {
    return NULL;
  }

  FILE *file = fopen(path, "r");
  if (file == NULL) {
    return NULL;
  }

  long int file_size = 0;
  int c = fgetc(file);
  while (c != EOF) {
    file_size++;
    c = fgetc(file);
  }
  fseek(file, 0, SEEK_SET);

  char *source = malloc(file_size + 1);
  if (source == NULL) {
    fprintf(stderr, "Unable to allocate shader program memory\n");
    return NULL;
  }
  fread(source, file_size, 1, file);
  source[file_size] = '\0';

  fclose(file);

  return source;
}

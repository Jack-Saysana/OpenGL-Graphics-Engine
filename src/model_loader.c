#include <model_loader.h>

MODEL *load_model(char *path) {
  char *bin_path = malloc(strlen(path) + 5);
  sprintf(bin_path, "%s.bin", path);
  FILE *file = fopen(bin_path, "rb");

  if (file == NULL) {
    LINE_BUFFER *line_buff = get_lines(path);
    if (line_buff == NULL) {
      printf("Unable to create line buffer\n");
      return NULL;
    }

    preprocess_lines(line_buff);

    file = fopen(bin_path, "rb");
    if (file == NULL) {
      printf("Unable to open preproccessed file\n");
      return NULL;
    }
  }

  size_t b_len;
  size_t v_len;
  size_t i_len;
  size_t a_len;
  fread(&b_len, sizeof(size_t), 1, file);
  fread(&v_len, sizeof(size_t), 1, file);
  fread(&i_len, sizeof(size_t), 1, file);
  fread(&a_len, sizeof(size_t), 1, file);
  printf("a_len: %lld\n", a_len);
  fflush(stdout);

  int material_flag;
  int path_len;
  MATERIAL *obj_mat = NULL;
  fread(&material_flag, sizeof(int), 1, file);
  if (material_flag) {
    obj_mat = malloc(sizeof(MATERIAL));
    for (int i = 0; i < NUM_PROPS; i++) {
      fread(&path_len, sizeof(int), 1, file);
      if (path_len > 0) {
        obj_mat->mat_paths[i] = malloc(path_len);
        fread(obj_mat->mat_paths[i], sizeof(char), path_len, file);
      } else {
        obj_mat->mat_paths[i] = NULL;
      }
    }
  }

  BONE *bones = malloc(sizeof(BONE) * b_len);
  VBO *vertices = malloc(sizeof(VBO) * v_len);
  int *indicies = malloc(sizeof(int) * 3 * i_len);
  ANIMATION *animations = malloc(sizeof(ANIMATION) * a_len);
  fread(bones, sizeof(BONE), b_len, file);
  fread(vertices, sizeof(VBO), v_len, file);
  fread(indicies, sizeof(int) * 3, i_len, file);

  for (int i = 0; i < a_len; i++) {
    fread(&(animations[i].num_chains), sizeof(size_t), 1, file);
    printf("Animation %d with %lld chains:\n", i + 1, animations[i].num_chains);
    fflush(stdout);
    animations[i].keyframe_chains = malloc(sizeof(K_CHAIN) * animations[i].num_chains);
    for (int j = 0; j < animations[i].num_chains; j++) {
      K_CHAIN *cur = animations[i].keyframe_chains + j;
      fread(&(cur->b_id), sizeof(unsigned int), 1, file);
      fread(&(cur->start_frame), sizeof(unsigned int), 1, file);
      fread(&(cur->type), sizeof(C_TYPE), 1, file);
      fread(&(cur->num_frames), sizeof(size_t), 1, file);
      cur->chain = malloc(sizeof(KEYFRAME) * cur->num_frames);
      printf("  Chain on bone %d of type %d with %lld frames:\n", cur->b_id, cur->type, cur->num_frames);
      fflush(stdout);
      for (int k = 0; k < cur->num_frames; k++) {
        fread(cur->chain[k].offset, sizeof(float), 4, file);
        fread(&(cur->chain[k].frame), sizeof(unsigned int), 1, file);
        printf("    Frame: %d, offset: %f %f %f %f\n", cur->chain[k].frame,
               cur->chain[k].offset[0],
               cur->chain[k].offset[1],
               cur->chain[k].offset[2],
               cur->chain[k].offset[3]);
        fflush(stdout);
      }
    }
  }
  fclose(file);

  unsigned int VAO_id;
  glGenVertexArrays(1, &VAO_id);
  glBindVertexArray(VAO_id);

  unsigned int VBO_id;
  glGenBuffers(1, &VBO_id);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_id);
  glBufferData(GL_ARRAY_BUFFER, sizeof(VBO) * v_len, vertices, GL_STATIC_DRAW);

  unsigned int EBO_id;
  glGenBuffers(1, &EBO_id);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_id);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * 3 * i_len, indicies,
              GL_STATIC_DRAW);

  int stride = (12 * sizeof(float)) + (4 * sizeof(int));
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride,
                        (void *) 0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride,
                        (void *) (sizeof(float) * 3));
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride,
                        (void *) (sizeof(float) * 6));
  glVertexAttribPointer(3, 4, GL_INT, GL_FALSE, stride,
                        (void *) (sizeof(float) * 8));
  glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, stride,
                        (void *) ((sizeof(float) * 8) + (sizeof(int) * 4)));

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);
  glBindVertexArray(0);

  MODEL *model = malloc(sizeof(MODEL));
  model->VAO = VAO_id;
  model->bones = bones;
  model->num_bones = b_len;
  model->num_indicies = i_len * 3;

  if (obj_mat != NULL) {
    for (int i = 0; i < NUM_PROPS; i++) {
      if (obj_mat->mat_paths[i] != NULL) {
        model->textures[i] = genTextureId(obj_mat->mat_paths[i]);
      }
    }

    free_materials(obj_mat, 1);
  }
  free(bin_path);
  free(vertices);
  free(indicies);

  return model;
}

unsigned int genTextureId(char *tex_path) {
  unsigned int texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  int width;
  int height;
  int nrChannels;
  unsigned char *data = stbi_load(tex_path, &width, &height, &nrChannels, 0);
  if (data) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
  } else {
    printf("Failed to load texture at: %s\n", tex_path);
  }
  stbi_image_free(data);

  return texture;
}

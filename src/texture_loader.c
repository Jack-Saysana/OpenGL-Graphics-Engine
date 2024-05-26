#include <texture_loader.h>

int gen_texture_id(char *tex_path, unsigned int *dest) {
  char *hash_path = remove_double_dot(tex_path);
  size_t tex_index = tex_tab_search(hash_path);
  if (tex_index != INVALID_INDEX) {
    *dest = tex_tab[tex_index].texture;
    free(hash_path);
    return 0;
  }

  unsigned int texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  int width;
  int height;
  int nrChannels;
  stbi_set_flip_vertically_on_load(1);
  unsigned char *data = stbi_load(tex_path, &width, &height, &nrChannels, 0);
  if (data) {
    int format = GL_RGBA;
    if (nrChannels == 1) {
      format = GL_RED;
    } else if (nrChannels == 2) {
      format = GL_RG;
    } else if (nrChannels == 3) {
      format = GL_RGB;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
  } else {
    fprintf(stderr, "Failed to load texture at: %s\n", tex_path);
    *dest = INVALID_TEX;
    return -1;
  }
  stbi_image_free(data);

  tex_index = tex_tab_add(hash_path);
  tex_tab[tex_index].texture = texture;
  *dest = texture;
  free(hash_path);
  return 0;
}

int gen_cubemap(char **paths, unsigned int *dest) {
  unsigned int texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

  int width;
  int height;
  int nrChannels;
  stbi_set_flip_vertically_on_load(1);
  for (int i = 0; i < 6; i++) {
    unsigned char *data = stbi_load(paths[i], &width, &height, &nrChannels, 0);
    if (data) {
      int format = GL_RGBA;
      if (nrChannels == 1) {
        format = GL_RED;
      } else if (nrChannels == 2) {
        format = GL_RG;
      } else if (nrChannels == 3) {
        format = GL_RGB;
      }

      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width,
                   height, 0, format, GL_UNSIGNED_BYTE, data);
      glGenerateMipmap(GL_TEXTURE_2D);
    } else {
      fprintf(stderr, "Failed to load texture at: %s\n", paths[i]);
      *dest = INVALID_TEX;
      return -1;
    }
    stbi_image_free(data);
    data = NULL;
  }

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  *dest = texture;

  return 0;
}

// ========================== TEXTURE TABLE HELPERS ==========================

size_t hash_tex(char *path, size_t i) {
  size_t key = get_str_hash(path);
  double mult = key * HASH_MAGIC_NUM - floor(key * HASH_MAGIC_NUM);
  size_t ret = tex_tab_size * mult;
  return (ret + i) % tex_tab_size;
}

int init_tex_tab() {
  tex_tab = malloc(sizeof(TEX_TAB) * TEX_TAB_STARTING_LEN);
  if (tex_tab == NULL) {
    return -1;
  }
  memset(tex_tab, 0, sizeof(TEX_TAB) * TEX_TAB_STARTING_LEN);
  tex_tab_len = 0;
  tex_tab_size = TEX_TAB_STARTING_LEN;

  return 0;
}

void free_textures() {
  for (size_t i = 0; i < tex_tab_size; i++) {
    if (tex_tab[i].status == LEDGER_OCCUPIED) {
      free(tex_tab[i].path);
    }
  }

  free(tex_tab);
  tex_tab = NULL;
  tex_tab_len = 0;
  tex_tab_size = 0;
}

size_t tex_tab_add(char *path) {
  if (!tex_tab) {
    init_tex_tab();
  }

  size_t index = tex_tab_search(path);
  if (index != INVALID_INDEX) {
    return index;
  }

  size_t i = 0;
  while (1) {
    index = hash_tex(path, i);
    if (tex_tab[index].status != LEDGER_OCCUPIED) {
      tex_tab[index].status = LEDGER_OCCUPIED;
      tex_tab[index].texture = INVALID_TEX;
      tex_tab[index].path = malloc(sizeof(char) * strlen(path) + 1);
      memcpy(tex_tab[index].path, path, strlen(path) + 1);
      break;
    }
    i++;
  }
  tex_tab_len++;

  double load_factor = ((double) tex_tab_len) / ((double) tex_tab_size);
  if (load_factor > 0.5) {
    int status = resize_tex_tab();
    if (status) {
      tex_tab[index].status = LEDGER_FREE;
      tex_tab_len--;
      return INVALID_INDEX;
    }
  }

  return tex_tab_search(path);
}

size_t tex_tab_search(char *path) {
  if (!tex_tab) {
    init_tex_tab();
  }

  size_t i = 0;
  size_t index = 0;
  while (1) {
    index = hash_tex(path, i);
    if (tex_tab[index].status == LEDGER_FREE) {
      break;
    } else if (tex_tab[index].status == LEDGER_OCCUPIED &&
               strncmp(path, tex_tab[index].path, strlen(path)) == 0) {
      return index;
    }
    i++;
  }
  return INVALID_INDEX;
}

void tex_tab_delete(char *path) {
  if (!tex_tab) {
    init_tex_tab();
  }

  size_t index = tex_tab_search(path);
  if (index != INVALID_INDEX) {
    tex_tab_len--;
    tex_tab[index].status = LEDGER_DELETED;
    glDeleteTextures(1, &tex_tab[index].texture);
    free(tex_tab[index].path);
  }
}

int resize_tex_tab() {
  if (!tex_tab) {
    init_tex_tab();
  }

  TEX_TAB *new_tab = malloc(sizeof(TEX_TAB) * 2 * tex_tab_size);
  if (new_tab == NULL) {
    return -1;
  }
  memset(new_tab, 0, sizeof(TEX_TAB) * tex_tab_size * 2);

  size_t j = 0;
  size_t index = 0;
  char *cur_path = 0;
  unsigned int cur_tex = 0;
  for (size_t i = 0; i < tex_tab_size; i++) {
    if (tex_tab[i].status != LEDGER_OCCUPIED) {
      continue;
    }

    cur_path = tex_tab[i].path;
    cur_tex = tex_tab[i].texture;
    j = 0;
    while (1) {
      index = hash_tex(cur_path, j);
      if (new_tab[index].status != LEDGER_OCCUPIED) {
        new_tab[index].status = LEDGER_OCCUPIED;
        new_tab[index].path = cur_path;
        new_tab[index].texture = cur_tex;
        break;
      }
      j++;
    }
  }
  tex_tab_size *= 2;

  free(tex_tab);
  tex_tab = new_tab;
  return 0;
}


#include <texture_loader.h>

int gen_texture_id(char *tex_path, unsigned int *dest) {
  char *hash_path = remove_double_dot(tex_path);
  size_t tex_index = tex_tab_search(hash_path);
  if (tex_index != INVALID_INDEX) {
    *dest = tex_tab[tex_index].texture;
    tex_tab[tex_index].ref_count++;
    free(hash_path);
    return 0;
  }

  unsigned int texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  if (glGetError() != GL_NO_ERROR) {
    fprintf(stderr, "Failed to bind texture: %s\n", tex_path);
    goto ERR_ALL;
  }

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  if (glGetError() != GL_NO_ERROR) {
    fprintf(stderr, "Failed to set texture params: %s\n", tex_path);
    goto ERR_ALL;
  }

  int width;
  int height;
  int nrChannels;
  stbi_set_flip_vertically_on_load(1);
  unsigned char *data = stbi_load(tex_path, &width, &height, &nrChannels, 0);
  if (data) {
    GLenum format = GL_RGBA;
    GLenum internal_format = GL_RGBA8;
    if (nrChannels == 1) {
      format = GL_RED;
      internal_format = GL_R8;
    } else if (nrChannels == 2) {
      format = GL_RG;
      internal_format = GL_RG8;
    } else if (nrChannels == 3) {
      format = GL_RGB;
      internal_format = GL_RGB8;
    }

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format,
                 GL_UNSIGNED_BYTE, data);
    if (glGetError() != GL_NO_ERROR) {
      fprintf(stderr, "Failed to gen texImage2D: %s\n", tex_path);
      stbi_image_free(data);
      goto ERR_ALL;
    }
    glGenerateMipmap(GL_TEXTURE_2D);
    if (glGetError() != GL_NO_ERROR) {
      fprintf(stderr, "Failed to gen mipmap: %s\n", tex_path);
      stbi_image_free(data);
      goto ERR_ALL;
    }
  } else {
    fprintf(stderr, "Failed to load texture at: %s\n", tex_path);
    goto ERR_ALL;
  }
  stbi_image_free(data);

  tex_index = tex_tab_add(hash_path);
  if (tex_index == INVALID_INDEX) {
    goto ERR_ALL;
  }

  // fprintf(stderr, "New texture: %s -> %d, %ld of %ld\n", hash_path, texture,
          // tex_tab_len, tex_tab_size);
  tex_tab[tex_index].texture = texture;
  tex_tab[tex_index].ref_count = 1;
  *dest = texture;
  free(hash_path);
  return 0;

ERR_ALL:
  glDeleteTextures(1, &texture);
  *dest = INVALID_INDEX;
  free(hash_path);
  return -1;
}

void del_texture_id(char *tex_path) {
  char *hash_path = remove_double_dot(tex_path);
  size_t tex_index = tex_tab_search(hash_path);
  if (tex_index == INVALID_INDEX) {
    free(hash_path);
    return;
  }

  tex_tab[tex_index].ref_count--;
  if (!tex_tab[tex_index].ref_count) {
    tex_tab_delete(tex_tab[tex_index].path);
  }
  free(hash_path);
}

int gen_cubemap(char **paths, unsigned int *dest) {
  unsigned int texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

  int width;
  int height;
  int nrChannels;
  stbi_set_flip_vertically_on_load(0);
  for (int i = 0; i < 6; i++) {
    unsigned char *data = stbi_load(paths[i], &width, &height, &nrChannels, 0);
    if (data) {
      GLenum format = GL_RGBA;
      GLenum internal_format = GL_RGBA8;
      if (nrChannels == 1) {
        format = GL_RED;
        internal_format = GL_R8;
      } else if (nrChannels == 2) {
        format = GL_RG;
        internal_format = GL_RG8;
      } else if (nrChannels == 3) {
        format = GL_RGB;
        internal_format = GL_RGB8;
      }

      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internal_format,
                   width, height, 0, format, GL_UNSIGNED_BYTE, data);
      glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
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

void del_cubemap(unsigned int cm) {
  glDeleteTextures(1, &cm);
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

  for (size_t i = 0; i < tex_tab_size; i++) {
    tex_tab[i].texture = -1;
  }
  return 0;
}

void free_textures() {
  for (size_t i = 0; i < tex_tab_size; i++) {
    if (tex_tab[i].status == LEDGER_OCCUPIED) {
      tex_tab[i].status = LEDGER_DELETED;
      glDeleteTextures(1, &tex_tab[i].texture);
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
    // fprintf(stderr, "Resizing tex tab.\n");
    int status = resize_tex_tab();
    if (status) {
      tex_tab[index].status = LEDGER_FREE;
      free(tex_tab[index].path);
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
               strcmp(path, tex_tab[index].path) == 0) {
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
    // fprintf(stderr, "Deleting texture: %d\n", tex_tab[index].texture);
    glDeleteTextures(1, &tex_tab[index].texture);
    free(tex_tab[index].path);
    tex_tab[index].texture = -1;
    tex_tab[index].path = NULL;
  }
}

int resize_tex_tab() {
  if (!tex_tab) {
    init_tex_tab();
  }

  size_t old_size = tex_tab_size;
  size_t new_size = tex_tab_size * 2;

  TEX_TAB *new_tab = malloc(sizeof(TEX_TAB) * new_size);
  if (new_tab == NULL) {
    return -1;
  }
  memset(new_tab, 0, sizeof(TEX_TAB) * new_size);

  TEX_TAB *old_tab = tex_tab;
  tex_tab = new_tab;
  tex_tab_size = new_size;

  size_t j = 0;
  size_t index = 0;
  for (size_t i = 0; i < old_size; i++) {
    if (old_tab[i].status != LEDGER_OCCUPIED) {
      continue;
    }

    j = 0;
    while (1) {
      index = hash_tex(old_tab[i].path, j);
      if (tex_tab[index].status != LEDGER_OCCUPIED) {
        tex_tab[index] = old_tab[index];
        break;
      }
      j++;
    }
  }
  free(old_tab);
  return 0;
}


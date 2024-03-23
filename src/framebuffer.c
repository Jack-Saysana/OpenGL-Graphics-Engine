#include <framebuffer.h>

FRAMEBUFFER framebuffer_init(float res_x, float res_y) {
  FRAMEBUFFER fb;

  glGenFramebuffers(1, &fb.FBO);
  glBindFramebuffer(GL_FRAMEBUFFER, fb.FBO);

  glGenTextures(1, &fb.color_texture);
  glBindTexture(GL_TEXTURE_2D, fb.color_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, res_x, res_y, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);

  glGenRenderbuffers(1, &fb.depth_stencil_rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, fb.depth_stencil_rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, res_x, res_y);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         fb.color_texture, 0);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                            GL_RENDERBUFFER, fb.depth_stencil_rbo);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  return fb;
}

void framebuffer_delete(FRAMEBUFFER fb) {
  glDeleteTextures(1, &fb.color_texture);
  glDeleteRenderbuffers(1, &fb.depth_stencil_rbo);
  glDeleteFramebuffers(1, &fb.FBO);
}

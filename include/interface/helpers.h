#ifndef __ENGINE_HELPERS_H__
#define __ENGINE_HELPERS_H__

int double_buffer(void **buffer, size_t *buff_size, size_t unit_size);

int max_dot(vec3 *vecs, unsigned int len, vec3 dir);
void vec3_remove_noise(vec3 vec, float threshold);
float remove_noise(float flt, float threshold);

void set_mat4(char *name, mat4 mat, unsigned int shader);
void set_mat3(char *name, mat3 mat, unsigned int shader);
void set_vec4(char *name, vec4 vec, unsigned int shader);
void set_vec3(char *name, vec3 vec, unsigned int shader);
void set_vec2(char *name, vec2 vec, unsigned int shader);
void set_float(char *name, float flt, unsigned int shader);
void set_int(char *name, int val, unsigned int shader);
void set_uint(char *name, unsigned int val, unsigned int shader);
void set_iarr(char *name, int *arr, size_t arr_len, unsigned int shader);

#endif

# Helpers

The engine also provides a variety of helper functions that are useful for in many cases

## Functions

### Shader manipulation

These functions will automatically set uniform variables of shaders

```void set_mat4(char *name, mat4 mat, unsigned int shader)```

**Arguments**

- `char *name`: Name of variable in shader

- `mat4 mat`: 4x4 matrix to be passed to shader

- `unsigned int shader`: ID of shader program

```void set_mat4(char *name, mat3 mat, unsigned int shader)```

**Arguments**

- `char *name`: Name of variable in shader

- `mat3 mat`: 3x3 matrix to be passed to shader

- `unsigned int shader`: ID of shader program

```void set_vec4(char *name, vec4 vec, unsigned int shader)```

**Arguments**

- `char *name`: Name of variable in shader

- `vec4 vec3`: 4-float vector to be passed to shader

- `unsigned int shader`: ID of shader program

```void set_vec3(char *name, vec3 vec, unsigned int shader)```

**Arguments**

- `char *name`: Name of variable in shader

- `vec3 vector`: 3-float vector to be passed to shader

- `unsigned int shader`: ID of shader program

```void set_vec2(char *name, vec2 vec, unsigned int shader)```

**Arguments**

- `char *name`: Name of variable in shader

- `vec2 vec`: 2-float vector to be passed to shader

- `unsigned int shader`: ID of shader program

```void set_float(char *name, float f, unsigned int shader)```

**Arguments**

- `char *name`: Name of variable in shader

- `float f`: float to be passed to shader

- `unsigned int shader`: ID of shader program

```void set_int(char *name, int i, unsigned int shader)```

**Arguments**

- `char *name`: Name of variable in shader

- `int i`: integer to be passed to shader

- `unsigned int shader`: ID of shader program

```void set_uint(char *name, unsigned int ui, unsigned int shader)```

**Arguments**

- `char *name`: Name of variable in shader

- `unsigned int ui`: unsigned integer to be passed to shader

- `unsigned int shader`: ID of shader program

```void set_i_arr(char *name, int *iarr, size_t len, unsigned int shader)```

**Arguments**

- `char *name`: Name of variable in shader

- `int *iarr`: Buffer of integers to be passed to shader

- `size_t len`: Number of elements in iarr to pass to the shader

- `unsigned int shader`: ID of shader program

### Math

```int max_dot(vec3 *verts, unsigned int len, vec3 dir)```

Given a set of vertices and an arbitrary direction, dir, max_dot will find which vertex in verts has the highest dot product with dir.

**Arguments**

- `vec3 *verts`: List of vertices to search

- `unsigned int len`: Number of vertices in `verts`

- `vec3 dir`: Vector to test against

**Returns**

The index of the vertex in `verts` whose dot product is the maximum with `dir`

```void vec3_remove_noise(vec3 v, float threshold)```

Each element in `v` is checked. If it falls between -threshold and threshold, it is simply set to 0

**Arguments**

- `vec3 v`: 3-float vector to evaluate

- `float threshold`: Threshold value for zeroing

```void remove_noise(float f, float threshold)```

If `f` falls between `-threshold` and `threshold`, zero is returned. Otherwise, the value of f is returned

**Arguments**

- `float f`: float to evaluate

- `float threshold`: Threshold value for zeroing

**Returns**

0.0 if `f` falls within the threshold, `f` if not

### Misc

```int double_buffer(void **buffer, size_t *buff_size, size_t unit_size)```

Reallocates a dynamically allocated buffer to twice its size

**Arguments**

- `void **buffer`: Pointer to buffer to reallocate

- `size_t *buff_size`: Pointer to size value of buffer

- `size_t unit_size`: Size of each element in the buffer

**Returns**

0 if successful, nonzero if error has occured

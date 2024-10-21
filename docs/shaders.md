# Shaders

For ease of development, shader file importing functions are provided for glsl shaders.

## Functions

### `unsigned int init_shader_prog(char *vs_path, char *gs_path, char *fs_path)`

Creates a shader program given inputs to vertex, geometry and fragment shader files

**Arguments**

- `char *vs_path`: Path to vertex shader

- `char *gs_path`: Path to geometry shader. If no geometry shader is used, `NULL` is a valid argument.

- `char *fs_path`: Path to fragment shader

**Returns**

The ID of the new shader program or -1 if an error has occured

#version 460

in vec4 fragPos;
in vec2 texCoords;
in vec3 normal;
in vec3 viewPos;

struct Material {
  sampler2D diff_map;
  sampler2D spec_map;
  int shine;
} material;

struct DirLight {
  vec3 col;
  vec3 dir;
}

struct PointLight {
  vec3 col;
  vec3 pos;
  int a_consts[3];
}

vec3 calc_dir_light(struct DirLight);
vec3 calc_point_light(struct PointLight);

void main() {
  // TODO: MAKE SCENE
}

vec3 calc_dir_light(struct DirLight light) {
  vec3 col = texture(material.diff_map, texCoords);

  vec3 ambient = light.col * 0.05;

  // TODO: MAKE NORMAL A UNIT VECTOR
  vec3 diffuse = max(normal * light.dir, 0) * light.col;

  vec3 viewDir = viewPos - fragPos;
  // TODO: CALC SPECULAR (BLINN PHONG)
  vec3 specular = vec3(0, 0, 0);

  return (ambient + diffuse + specular) * light.col;
}

vec3 calc_point_light(struct PointLight light) {
  vec3 col = texture(material.diff_map, texCoords);

  vec3 ambient = light.col * 0.05;

  vec3 lightDir = light.pos - vec3(fragPos);
  // TODO: MAKE NORMAL A UNIT VECTOR
  vec3 diffuse = max(normal * light.dir, 0) * light.col;

  vec3 viewDir = viewPos - fragPos;
  // TODO: CALC SPECULAR (BLINN PHONG)
  vec3 specular = vec3(0, 0, 0);

  // TODO: ADD ATTENUATION

  return (ambient + diffuse + specular) * light.col;
}

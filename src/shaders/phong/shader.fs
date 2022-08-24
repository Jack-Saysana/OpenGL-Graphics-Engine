#version 460

in vec4 fragPos;
in vec2 texCoords;
in vec3 normal;
in vec3 viewPos;

out vec4 FragCol;

struct Material {
  sampler2D diff_map;
  sampler2D spec_map;
  int shine;
} material;

struct DirLight {
  vec3 col;
  vec3 dir;
} d_light;

struct PointLight {
  vec3 col;
  vec3 pos;
  int a_consts[3];
} p_lights[1];

vec3 calc_dir_light(struct DirLight);
vec3 calc_point_light(struct PointLight);

void main() {
  FragCol = vec4(calc_dir_light(d_light) + calc_point_light(p_lights[0]), 1.0);
}

vec3 calc_dir_light(struct DirLight light) {
  vec3 col = texture(material.diff_map, texCoords);

  vec3 ambient = light.col * 0.05;

  vec3 norm = normalize(normal);
  vec3 diffuse = max(dot(norm, light.dir), 0) * light.col;

  vec3 viewDir = viewPos - fragPos;
  vec3 reflectDir = reflect(light.dir, norm);
  float spec_strn = pow(max(dot(viewDir, reflectDir), 0), material.shine);
  vec3 specular = texture(material.spec_map, texCoords) * spec_strn;

  return (ambient + diffuse + specular) * light.col;
}

vec3 calc_point_light(struct PointLight light) {
  vec3 col = texture(material.diff_map, texCoords);

  vec3 ambient = light.col * 0.05;

  vec3 lightDir = light.pos - vec3(fragPos);
  vec3 norm = normalize(normal);
  vec3 diffuse = max(dot(norm, light.dir), 0) * light.col;

  vec3 viewDir = viewPos - fragPos;
  vec3 reflectDir = reflect(lightDir, norm);
  float spec_strn = pow(max(dot(viewDir, reflectDir), 0), material.shine);
  vec3 specular = texture(material.spec_map, texCoords) * spec_strn;

  return (ambient + diffuse + specular) * light.col;
}

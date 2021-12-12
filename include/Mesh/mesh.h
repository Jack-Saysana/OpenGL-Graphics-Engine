#ifndef MESH_H
#define MESH_H

#include <vector>
#include <string>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "Shader/shader.h"

struct Vertex
{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

struct Texture
{
    unsigned int id;
    std::string type;
    std::string path;
};

class Mesh
{
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    glm::mat4* modelMatrices;

    Mesh(std::vector<Vertex> vert, std::vector<unsigned int> ind, std::vector<Texture> tex);
    Mesh(std::vector<Vertex> vert, std::vector<unsigned int> ind, std::vector<Texture> tex, glm::mat4* modelMats, unsigned int amount);

    void Draw(Shader &shader);
    void DrawInstanced(Shader &shader);
    void updateModelMats(glm::mat4* modelMats);
private:
    bool instanced;
    unsigned int VAO, VBO, IVBO, EBO;
    unsigned int instances;
    void setUpMesh();
};

#endif
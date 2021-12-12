#ifndef MODEL_H
#define MODEL_H

#include <string>
#include <vector>
#include <Mesh/mesh.h>
#include <Shader/shader.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "stb/stb_image.h"

//Reads a texture map, loads the corresponding texture map to an object, then returns the object via it's id
unsigned int TextureFromFile(const char* path, const std::string &directory, aiTextureType type);

class Model
{
public:
    Model(std::string path);
    Model(std::string path, unsigned int numInstances, glm::mat4* modelMats);
    
    void Draw(Shader &shader);
    void updateInstancedModels(glm::mat4* modelMats);
private:
    std::vector<Texture> loaded_textures;
    std::vector<Mesh> meshes;
    std::string directory;

    bool instanced;
    unsigned int instances;
    glm::mat4* modelMatrices;

    //Loads 3d model into a root "scene" object
    void loadModel(std::string path);
    //Recursively processes a node and all of it's children
    void processNode(aiNode* node, const aiScene* scene);
    //Takes mesh data and processes it to return Mesh object
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
    //Finds and loads corresponding texture maps for a given texture type (diffuse, specular, etc...) 
    std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
};

#endif
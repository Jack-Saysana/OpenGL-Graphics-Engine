#include <Model/model.h>

Model::Model(std::string path)
{
    instances = 1;
    instanced = false;

    loadModel(path);
}

//New model constructor for models that will be instanced
Model::Model(std::string path, unsigned int numInstances, glm::mat4* modelMats)
{
    //Stores the number of instances we will render
    instances = numInstances;
    instanced = true;
    //Also stores the array of model matrices that correspond to each instance of the model
    modelMatrices = modelMats;
    loadModel(path);
}

void Model::Draw(Shader &shader)
{
    for(int i = 0; i < meshes.size(); i++)
        meshes[i].Draw(shader);
}

void Model::updateInstancedModels(glm::mat4* modelMats)
{
    if(instanced)
    {
        for(int i = 0; i < meshes.size(); i++)
            meshes[i].updateModelMats(modelMats);
    }
    else
    {
        std::cout << "Model is not an instanced. Cannot update model matrices." << std::endl;
    }
}

void Model::loadModel(std::string path)
{
    //Imports and loads 3d model data into a root "scene" object
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    //Error Handling
    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }

    //Retrieve Directory of 3d model
    directory = path.substr(0, path.find_last_of('/'));

    //Process root node
    processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode* node, const aiScene* scene)
{
    //Process all of the node's meshes
    for(unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }

    //Do same for each child node, will not run if node has no children
    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    //Process each vertex in the mesh's position, normal and texture coordinates into vertex object
    //Then add each vertex into vertices vector array
    for(unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        vertex.Position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        vertex.Normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        if(mesh->mTextureCoords[0])
        {
            vertex.TexCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        }
        else
        {
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);
        }
        vertices.push_back(vertex);
    }

    //Iterate over each face in the mesh then store each of the face's indices into the indicies vector array
    for(unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for(unsigned int j = 0; j < face.mNumIndices; j++)
        {
            indices.push_back(face.mIndices[j]);
        }
    }

    if(mesh->mMaterialIndex >= 0)
    {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        //Creates texture object(s) from the material object then adds them to texture vector array
        std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        //Inserts vector array containing diffuse maps to overall texture vector array
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        
        //Same thing here, but for specualr maps
        std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

        //Normal maps
        std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
    }

    //Determines if the Model is instanced, and process the meshes accordingly
    if(instanced)
    {
        return Mesh(vertices, indices, textures, modelMatrices, instances);
    }
    else
    {
        return Mesh(vertices, indices, textures);
    }
}

std::vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
{
    std::vector<Texture> textures;
    for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        bool skip = false;
        //Check if texture is already loaded
        for(unsigned int j = 0; j < loaded_textures.size(); j++)
        {
            if(std::strcmp(loaded_textures[j].path.data(), str.C_Str()) == 0)
            {
                textures.push_back(loaded_textures[j]);
                skip = true;
                break;
            }
        }
        if(!skip)
        {
            Texture texture;
            texture.id = TextureFromFile(str.C_Str(), directory, type);
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            loaded_textures.push_back(texture);
        }
    }
    return textures;
}

unsigned int TextureFromFile(const char* path, const std::string &directory, aiTextureType type)
{
    std::string fileName = std::string(path);
    fileName = directory + "/" + fileName;
    
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(fileName.c_str(), &width, &height, &nrComponents, 0);
    if(data)
    {
        GLenum internalFormat;
        GLenum format;
        if(nrComponents == 1)
        {
            internalFormat = GL_RED;
            format = GL_RED;
        }
        else if(nrComponents == 3)
        {
            if(type == aiTextureType_DIFFUSE)
            {
                internalFormat = GL_SRGB;
            } else
            {
                internalFormat = GL_RGB;
            }
            format = GL_RGB;
        }
        else if(nrComponents == 4)
        {
            if(type == aiTextureType_DIFFUSE)
            {
                internalFormat = GL_SRGB_ALPHA;
            } else {
                internalFormat = GL_RGBA;
            }
            format = GL_RGBA;
        }
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
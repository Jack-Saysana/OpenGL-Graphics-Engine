#include <Mesh/mesh.h>

Mesh::Mesh(std::vector<Vertex> vert, std::vector<unsigned int> ind, std::vector<Texture> tex)
{
    vertices = vert;
    indices = ind;
    textures = tex;

    instanced = false;

    setUpMesh();
}

//New mesh constructor for Meshes that will be instanced
Mesh::Mesh(std::vector<Vertex> vert, std::vector<unsigned int> ind, std::vector<Texture> tex, glm::mat4* modelMats, unsigned int amount)
{
    vertices = vert;
    indices = ind;
    textures = tex;

    instanced = true;
    //Stores the array containing the model matrices corresponding to each instance of the mesh
    modelMatrices = modelMats;
    //Stores the number of instances we will render
    instances = amount;

    setUpMesh();
}

void Mesh::setUpMesh()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Normal));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, TexCoords));
    glEnableVertexAttribArray(2);

    //If the mesh is instanced, an instanced array will be created and used to store the instanced model matrices
    if(instanced)
    {
        //Creation of a new array buffer to store instanced model matrices
        glGenBuffers(1, &IVBO);
        glBindBuffer(GL_ARRAY_BUFFER, IVBO);
        //Stores the array of model matrices into the buffer
        glBufferData(GL_ARRAY_BUFFER, instances * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);

        //In OPENGL the max amount of data we can store in a single vertex attribute is a vec4
        //Since we're storing a mat4 (which is essentially 4 vec4s) for each vertex, we use 4 vertex attributes that each point to a vec4
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*) 0);
        glEnableVertexAttribArray(3);
        glVertexAttribDivisor(3, 1);

        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void *)sizeof(glm::vec4));
        glEnableVertexAttribArray(4);
        glVertexAttribDivisor(4, 1);

        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void *)(2 * sizeof(glm::vec4)));
        glEnableVertexAttribArray(5);
        glVertexAttribDivisor(5, 1);

        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void *)(3 * sizeof(glm::vec4)));
        glEnableVertexAttribArray(6);
        glVertexAttribDivisor(6, 1);
    }

    glBindVertexArray(0);
}

void Mesh::Draw(Shader &shader)
{
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    unsigned int normalNr = 1;
    for(unsigned int i = 0; i < textures.size(); i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        std::string number;
        std::string type = textures[i].type;
        if(type == "texture_diffuse")
        {
            number = std::to_string(diffuseNr++);
        }
        else if(type == "texture_specular")
        {
            number = std::to_string(specularNr++);
        }
        else if(type == "texture_normal")
        {
            number = std::to_string(normalNr++);
        }
        shader.setInt((type + number).c_str(), i);
        glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }
    glActiveTexture(GL_TEXTURE0);

    glBindVertexArray(VAO);
    shader.setBool("instanced", instanced);

    //Determines if the mesh is instanced and draws it accordingly
    if(instanced)
    {
        //Similar to glDrawElements, but takes an additional parameter representing the number of instances to draw
        glDrawElementsInstanced(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0, instances);
    }
    else
    {
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    }
    glBindVertexArray(0);
}

void Mesh::updateModelMats(glm::mat4 *modelMats)
{
    modelMatrices = modelMats;
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, IVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, instances * sizeof(glm::mat4), &modelMatrices[0]);
    glBindVertexArray(0);
}
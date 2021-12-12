#version 460 core
layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexCoords;
layout (location = 3) in mat4 inModelMatrix;

out vec2 TexCoords;
out vec3 FragPos;
out vec3 norm;
out vec4 FragPosLightSpace;

uniform bool instanced;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

void main()
{
    //Determines if the vertex is instanced and defines the model matrix to be used in the shader accordingly
    mat4 modelMatrix = instanced ? inModelMatrix : model;

    gl_Position = projection * view * modelMatrix * vec4(inPos, 1.0);

    TexCoords = inTexCoords;
    FragPos = vec3(modelMatrix * vec4(inPos, 1.0));
    //Using the lightSpaceMatrix, we pass the current fragment's coordinates from the light's
    //point of view into the fragment shader
    FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
    norm = mat3(transpose(inverse(modelMatrix))) * inNormal;
}
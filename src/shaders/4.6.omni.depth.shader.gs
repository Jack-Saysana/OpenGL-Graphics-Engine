#version 460 core
layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

uniform mat4 shadowMatrices[6];

out vec4 FragPos;

void main() {
    //Render the triangle from the perspective of each face of the cubemap
    for (int face = 0; face < 6; face++)
    {
        //When a cubemap is attached to the framebuffer, the variable gl_layer determines
        //which face of the cubemap we emit vertices to
        gl_Layer = face;
        //Render the triangle vertex in accordance to the face's transformation matrix
        for(int i = 0; i < 3; i++)
        {
            FragPos = gl_in[i].gl_Position;
            gl_Position = shadowMatrices[face] * FragPos;
            EmitVertex();
        }
        EndPrimitive();
    }
}
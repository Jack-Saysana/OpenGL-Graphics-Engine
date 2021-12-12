#version 460 core
in vec4 FragPos;

uniform vec3 lightPos;
uniform float far_plane;

void main()
{
    //Get distance between the fragment and light source
    float lightDistance = length(FragPos.xyz - lightPos);

    //Map the distance to the range [0, 1]
    lightDistance = lightDistance / far_plane;

    //Modify the fragment depth as this distance
    gl_FragDepth = lightDistance;
}
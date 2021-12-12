#version 460 core
in vec2 TexCoords;
in vec3 FragPos;
in vec4 FragPosLightSpace;
in vec3 norm;

out vec4 FragColor;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;
uniform sampler2D depthMap;
uniform samplerCube depthCubeMap;

struct PointLight
{
    vec3 position;
    vec4 color;

    //Intensities for each respective light effect
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;

    //Attenuation constants
    float constant;
    float linear;
    float quadratic;
};
uniform int numPointLights;
uniform PointLight pointLights[1];

struct DirLight
{
    vec3 direction;
    vec4 color;

    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
};
uniform DirLight dirLight;

uniform vec3 viewPos;

uniform float far_plane;

vec4 calcPointLight(PointLight light, vec3 norm);
vec4 calcDirLight(DirLight light, vec3 norm);
float calcDirShadow(vec3 norm, vec3 lightDir);
float calcOmniShadow(vec3 lightPos);

void main()
{
    vec3 norm = normalize(norm);

    vec4 result;

    //Calculate Directional Light
    result += calcDirLight(dirLight, norm);

    //Calculate Point Light
    for(int i = 0; i < numPointLights; i++)
    {
        result += calcPointLight(pointLights[i], norm);
    }

    FragColor = result;
    
    float gamma = 2.2;
    FragColor.rgb = pow(FragColor.rgb, vec3(1.0/gamma));
}

vec4 calcPointLight(PointLight light, vec3 norm)
{
    //Creates a vector projecting FROM the fragment TO the light source
    //The reasoning for this direction, is that it allows for the usage of a dot product
    //to calculate the diffuse lighting, as a vector projecting TOWARDS the fragment would
    //cancel out with the fragment's normal vector
    vec3 lightDir = normalize(light.position - FragPos);

    //Represents a vector pointing FROM the fragment TO the viewers position
    vec3 viewDir = normalize(viewPos - FragPos);

    //BLINN - PHONG MODEL
    //We utilize a vector halfway between the light dir and view dir to avoid the specular cut off
    //from the phong model
    vec3 halfwayDir = normalize(lightDir + viewDir);


    //AMBIENT LIGHTING

    vec4 ambient = light.ambient * texture(texture_diffuse1, TexCoords) * light.color;


    //DIFFUSE LIGHTING

    //Calculates the strength of diffused light on the fragment
    
    //The larger the angle between the light's direction and the fragment's normal vector,
    //the weaker the diffuse effect.
    float diffStr = max(dot(lightDir, norm), 0.0);
    vec4 diffuse = light.diffuse * diffStr * texture(texture_diffuse1, TexCoords) * light.color;

    //SPECULAR LIGHTING
    
    //Calculates the strength of the specular light on the fragment
    
    //The larger the angle between the fragment's normal vector and the vector halfway between
    //the camera vector and light direction vector, the weaker the specular effect.
    
    //The dot product is then raised to a degree of shininess.
    //Larger shininess values make it so the specular effect is much more sensitive
    //to the angle between the view direction and reflect direction, in turn,
    //giving a more exact and mirror like reflection.
    float specStr = pow(max(dot(norm, halfwayDir), 0.0), 256);
    vec4 specular = light.specular * specStr * texture(texture_specular1, TexCoords) * light.color;

    //ATTENTUATION - reduction of light intensity as fragments get further away from light

    float distance = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + (light.linear * distance) + (light.quadratic * (distance * distance)));

    //SHADOW CALCULATION
    float shadow = calcOmniShadow(light.position);

    //Multiply the diffuse and specular values by the shadow value
    //We omit the ambient values because if the fragment is inside a shadow, it should
    //still have a small amount of color
    vec4 result = attenuation * (ambient + (shadow * (diffuse + specular)));

    return(vec4(vec3(result), texture(texture_diffuse1, TexCoords).a));
}

vec4 calcDirLight(DirLight light, vec3 norm)
{
    vec3 lightDir = normalize(-light.direction);

    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 halfwayDir = normalize(lightDir + viewDir);

    //Ambient
    vec4 ambient = light.ambient * texture(texture_diffuse1, TexCoords) * light.color;

    //Diffuse
    float diffStr = max(dot(lightDir, norm), 0.0);
    vec4 diffuse = light.diffuse * diffStr * texture(texture_diffuse1, TexCoords) * light.color;

    //Specular
    float specStr = pow(max(dot(norm, halfwayDir), 0.0), 256);
    vec4 specular = light.specular * specStr * texture(texture_specular1, TexCoords) * light.color;

    //Shadow
    float shadow = calcDirShadow(norm, lightDir);

    vec4 result = ambient + (shadow * (diffuse + specular));

    return(vec4(vec3(result), texture(texture_diffuse1, TexCoords).a));
}

float calcDirShadow(vec3 norm, vec3 lightDir)
{
    //DIRECTIONAL SHADOW CALCULATION
    
    //Calculates whether or not the fragment is overshadowed or not

    //Normalize the coordinates to the range [-1, 1]
    vec3 projCoords = FragPosLightSpace.xyz / FragPosLightSpace.w;
    //Transform the coordinates to the range [0, 1], since the values from the depth map
    //are also mapped in the range [0, 1]
    projCoords = (projCoords * 0.5) + 0.5;
    //Returns the depth value of the current fragment from the light's point of view
    float currentDepth = projCoords.z;
    float bias = max(0.05 * (1.0 - dot(norm, lightDir)), 0.005);
    
    //Samples from the depthMap multiple times with slightly different texture coordinates,
    //checks if each of the samples is inside a shadow and averages the shadow value of all
    //the samples, which results in a softer looking shadow
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(depthMap, 0);
    for(int x = -1; x <= 1; x++)
    {
        for(int y = -1; y <= 1; y++)
        {
            //With the current fragment's coordinates represented from the perspective of the light,
            //we sample from the depthMap at the same coordinate, which returns the closest depth 
            //value at this point from the lights point of view
            float closestDepth = texture(depthMap, projCoords.xy + (vec2(x, y) * texelSize)).r;
            //If the current fragment's depth value from the lights pov is less than the closest
            //depth value from the lights pov, the fragment is inside a shadow
            shadow += (currentDepth - bias < closestDepth ? 1.0 : 0.0);
        }
    }

    return shadow / 9.0;
}

float calcOmniShadow(vec3 lightPos)
{
    //Remember, Cubemaps are sampled from the middle of the cube, pointed out toward the fragments
    //Therefore, in order to get the closest depth values from the depthcubemap, we create a vector
    //pointing from the light (the center of the cube) to the fragment
    vec3 lightToFrag = FragPos - lightPos;
    //Gets the closest depth value from the cubemap
    float closestDepth = texture(depthCubeMap, lightToFrag);
    //maps the depth value from [0, 1] to [0, far_plane]
    closestDepth *= far_plane;
    //Retrieve the length between the current fragment and the light source,
    //which will be compared to the depthvalue sampled previously to determine
    //if the current fragment is inside a shadow
    float lengthToLight = length(lightToFrag);

    float bias = 0.05;
    //Actual comparison to see if fragment is inside shadow
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
    return shadow;
}
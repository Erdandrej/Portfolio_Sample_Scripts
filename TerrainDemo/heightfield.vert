#version 420

///////////////////////////////////////////////////////////////////////////////
// In Variables
///////////////////////////////////////////////////////////////////////////////
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 color;

///////////////////////////////////////////////////////////////////////////////
// Out Variables
///////////////////////////////////////////////////////////////////////////////
flat out vec3 flatColor;
out vec3 Color;
out vec3 viewSpacePosition;

///////////////////////////////////////////////////////////////////////////////
// Uniforms
///////////////////////////////////////////////////////////////////////////////
struct Light {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Light light;
uniform vec3 viewPosition;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

uniform mat4 modelViewMatrix;
uniform mat4 modelViewProjectionMatrix;

vec3 calculateLighting(vec3 Normal, vec3 FragPos) {
    // Ambient lighting
    vec3 ambient = light.ambient;
    
    // Diffuse lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(lightDir, norm), 0.0);
    vec3 diffuse = light.diffuse * diff;

    // Specular lighting
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPosition - FragPos);
    vec3 reflectDir = reflect(-lightDir, Normal);

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16);
    vec3 specular = light.specular * spec;
    
    return (ambient + diffuse + specular);
}

void main()
{
    vec3 FragPos = vec3(mat4(1.0) * vec4(position, 1.0));
    vec3 Normal = normal;

    Color = color * calculateLighting(Normal, FragPos);
    flatColor = Color;

    viewSpacePosition = (modelViewMatrix * vec4(position, 1)).xyz;

    gl_Position = modelViewProjectionMatrix * vec4(position, 1.0);
}

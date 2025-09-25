// fragment shader
#version 330 core
out vec4 FragColor;

in vec4 FragPosLightSpace;
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

in vec3 TangentLightDir;
in vec3 TangentViewPos;
in vec3 TangentFragPos;

struct Light {
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct Material {
	sampler2D diffuse;
	sampler2D roughness;
	sampler2D normal;
	float shininess;
};

uniform Light light;
uniform Material material;
//uniform vec3 viewPos;
uniform sampler2D shadowMap;

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
	// perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
	float bias = max(0.01 * (1.0 - dot(normal, lightDir)), 0.005);

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    
    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1.0)
        shadow = 0.0;

    return shadow;
}

void main()
{
	// normal
	vec3 normal = texture(material.normal, TexCoords).rgb;
	normal = normalize(normal * 2.0 - 1.0);

	// ambient
	vec3 ambient = light.ambient * texture(material.diffuse, TexCoords).rgb;

	// diffuse
	//vec3 lightDir = normalize(-light.direction);
	vec3 lightDir = TangentLightDir;
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoords).rgb;

	// specular
	vec3 viewDir = normalize(TangentViewPos - TangentFragPos);
	//vec3 reflectDir = reflect(-lightDir, norm);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
	vec3 specularMap = vec3(1.0) - texture(material.roughness, TexCoords).rgb;
	//vec3 specular = light.specular * spec * specularMap;
	vec3 specular = light.specular * spec * vec3(0.2);

	// shadows
	vec3 worldLightDir = normalize(-light.direction);
	vec3 worldNormal = Normal;
	float shadow = ShadowCalculation(FragPosLightSpace, worldNormal, worldLightDir);
 
    vec3 result = (ambient + (1.0 - shadow) * (diffuse + specular));

	result = pow(result, vec3(1.0/2.2));
    FragColor = vec4(result, 1.0);
}
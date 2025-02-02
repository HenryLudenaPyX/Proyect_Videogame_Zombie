#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_emissive1;
uniform sampler2D texture_specular1;
struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform Light light;
uniform vec3 viewPos; // Posición de la cámara
uniform bool useLighting; 

// Linterna
uniform bool flashlightOn;
uniform vec3 flashlightPos;
uniform vec3 flashlightDir;
uniform float cutoff;
uniform float outerCutoff;

void main()
{    
    // 1. Obtener texturas
    vec3 diffuseTexture = texture(texture_diffuse1, TexCoords).rgb;
    vec3 specularTexture = texture(texture_specular1, TexCoords).rgb;

    // 2. Normalizar vectores
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);

    // 3. Luz ambiente
    vec3 ambient = light.ambient * diffuseTexture;

    // 4. Luz difusa (Lambert)
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * diffuseTexture;

    // 5. Luz especular (Phong)
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = light.specular * spec * specularTexture;

    vec3 flashlightEffect = vec3(0.0);

    // 6. Iluminación de la linterna (spotlight)
    if (flashlightOn) {
        vec3 fragToFlashlight = normalize(flashlightPos - FragPos);
        float theta = dot(normalize(flashlightDir), fragToFlashlight);

        if (theta > cutoff) {
            float intensity = smoothstep(outerCutoff, cutoff, theta);

            // Luz difusa de la linterna
            float diffFlashlight = max(dot(norm, fragToFlashlight), 0.0);
            vec3 flashlightDiffuse = vec3(1.0, 1.0, 0.8) * intensity * diffFlashlight * diffuseTexture;

            // Luz especular de la linterna
            vec3 reflectFlashlight = reflect(-fragToFlashlight, norm);
            float specFlashlight = pow(max(dot(viewDir, reflectFlashlight), 0.0), 32);
            vec3 flashlightSpecular = vec3(1.0) * intensity * specFlashlight * specularTexture;

            // Atenuación por distancia
            float distance = length(flashlightPos - FragPos);
            float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * (distance * distance));

            flashlightEffect = (flashlightDiffuse + flashlightSpecular) * attenuation;
        }
    }

    // 7. Combinar todas las luces
    vec3 result = ambient + diffuse + specular + flashlightEffect;
    
    // 8. Si no hay iluminación, usar solo el mapa emisivo
    if (!useLighting) {
        result = texture(texture_emissive1, TexCoords).rgb;
    }

    FragColor = vec4(result, 1.0);

}

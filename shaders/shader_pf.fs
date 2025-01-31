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

void main()
{    
    // Si no hay iluminación, solo usamos el mapa emisivo (para el cielo)
    if (!useLighting) {
        vec3 emissiveColor = texture(texture_emissive1, TexCoords).rgb;
        FragColor = vec4(emissiveColor, 1.0);
        return;
    }

    // 1. Obtener texturas
    vec3 diffuseTexture = texture(texture_diffuse1, TexCoords).rgb;
    vec3 specularTexture = texture(texture_specular1, TexCoords).rgb; // Mapa especular

    // 2. Calcular iluminación
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

    // 6. Resultado final
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);

}

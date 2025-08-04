#include "DefaultShaders.h"
#include <iostream>

const char* vertexShaderSource = R"glsl(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in mat4 aInstanceMatrix;
layout (location = 6) in vec3 aNormal;     // Normal del modelo (opcional)
layout (location = 7) in vec3 aTangent;    // Tangente del modelo (opcional)  
layout (location = 8) in vec3 aBitangent;  // Bitangente del modelo (opcional)

uniform mat4 view;
uniform mat4 projection;
uniform bool uUseModelNormals; // Flag para usar normales del modelo o calcular del cubo

out vec2 TexCoord;
out vec3 FragPos;
out vec3 Normal;
out mat3 TBN;

void main() {
    TexCoord = aTexCoord;
    
    // Transformar posición
    vec4 worldPos = aInstanceMatrix * vec4(aPos, 1.0);
    FragPos = worldPos.xyz;
    
    // Matriz normal para transformar normales correctamente
    mat3 normalMatrix = mat3(transpose(inverse(aInstanceMatrix)));
    
    vec3 normal, tangent, bitangent;
    
    if (uUseModelNormals) {
        // Usar normales del modelo 3D
        normal = normalize(normalMatrix * aNormal);
        tangent = normalize(normalMatrix * aTangent);
        bitangent = normalize(normalMatrix * aBitangent);
    } else {
        // Calcular normales del cubo basadas en la posición local
        normal = normalize(normalMatrix * normalize(aPos));
        tangent = normalize(normalMatrix * vec3(1.0, 0.0, 0.0));
        bitangent = normalize(normalMatrix * vec3(0.0, 1.0, 0.0));
    }
    
    Normal = normal;
    
    // Crear TBN matrix para normal mapping (Gram-Schmidt orthogonalization)
    vec3 T = normalize(tangent - dot(tangent, normal) * normal);
    vec3 B = normalize(cross(normal, T));
    
    TBN = mat3(T, B, normal);
    
    gl_Position = projection * view * worldPos;
}
)glsl";

const char* fragmentShaderSource = R"glsl(
#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;
in mat3 TBN;

// Material properties
uniform vec3 uAlbedo;
uniform float uMetallic;
uniform float uRoughness;
uniform vec3 uEmissive;
uniform vec2 uTiling;
uniform float uNormalStrength;

// Textures
uniform sampler2D uAlbedoTexture;
uniform sampler2D uNormalTexture;
uniform sampler2D uMetallicTexture;
uniform sampler2D uRoughnessTexture;
uniform sampler2D uEmissiveTexture;
uniform sampler2D uAOTexture;

// Texture flags
uniform bool uHasAlbedoTexture;
uniform bool uHasNormalTexture;
uniform bool uHasMetallicTexture;
uniform bool uHasRoughnessTexture;
uniform bool uHasEmissiveTexture;
uniform bool uHasAOTexture;

// Lighting
uniform vec3 uViewPos;
uniform vec3 uAmbientLight;
uniform bool uUsePBR; // Flag to switch between PBR and Blinn-Phong

// Post-processing uniforms
uniform float uExposure = 1.0;
uniform float uSaturation = 1.0;
uniform float uSmoothness = 1.0;

// Directional Light
uniform bool uHasDirLight;
uniform vec3 uDirLightDirection;
uniform vec3 uDirLightColor;
uniform float uDirLightIntensity;

// Point Lights (máximo 4)
uniform int uNumPointLights;
uniform vec3 uPointLightPositions[4];
uniform vec3 uPointLightColors[4];
uniform float uPointLightIntensities[4];
uniform vec3 uPointLightAttenuations[4];
uniform float uPointLightMinDistances[4];
uniform float uPointLightMaxDistances[4];

// Spot Lights (máximo 2)
uniform int uNumSpotLights;
uniform vec3 uSpotLightPositions[2];
uniform vec3 uSpotLightDirections[2];
uniform vec3 uSpotLightColors[2];
uniform float uSpotLightIntensities[2];
uniform float uSpotLightCutOffs[2];
uniform float uSpotLightOuterCutOffs[2];
uniform float uSpotLightRanges[2];

// Constants
const float PI = 3.14159265359;

// PBR Functions
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / max(denom, 0.0001); // Prevenir división por cero
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / max(denom, 0.0001); // Prevenir división por cero
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// PBR Lighting
vec3 CalculatePBRLighting(vec3 albedo, float metallic, float roughness, vec3 N, vec3 V, vec3 L, vec3 lightColor) {
    // Clamp y remap roughness para evitar valores extremos
    roughness = clamp(roughness, 0.04, 1.0);
    metallic = clamp(metallic, 0.0, 1.0);
    
    vec3 H = normalize(V + L);
    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.0);
    float HdotV = max(dot(H, V), 0.0);
    
    // Early exit if light doesn't hit surface
    if (NdotL <= 0.0) return vec3(0.0);
    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);
    
    float NDF = DistributionGGX(N, H, roughness);   
    float G = GeometrySmith(N, V, L, roughness);      
    vec3 F = FresnelSchlick(HdotV, F0);
           
    vec3 numerator = NDF * G * F; 
    float denominator = 4.0 * NdotV * NdotL + 0.0001;
    vec3 specular = numerator / denominator;
    
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;	  

    return (kD * albedo / PI + specular) * lightColor * NdotL;
}

// Blinn-Phong Lighting
vec3 CalculateBlinnPhongLighting(vec3 albedo, float shininess, vec3 N, vec3 V, vec3 L, vec3 lightColor) {
    // Diffuse
    float NdotL = max(dot(N, L), 0.0);
    vec3 diffuse = albedo * NdotL;
    
    // Specular (Blinn-Phong)
    vec3 H = normalize(V + L);
    float NdotH = max(dot(N, H), 0.0);
    float spec = pow(NdotH, shininess);
    
    // Better specular color that varies with material
    vec3 specularColor = mix(vec3(0.04), albedo, 0.3); // Mix of white and material color
    vec3 specular = specularColor * spec;
    
    return (diffuse + specular) * lightColor;
}

void main() {
    vec2 texCoord = TexCoord * uTiling;

    
    // Sample material properties
    float alpha = 1.0;
    vec3 albedo = uAlbedo;
    if (uHasAlbedoTexture) {
        vec4 albedoSample = texture(uAlbedoTexture, texCoord);
        albedo *= albedoSample.rgb;
        alpha = albedoSample.a;
    }

    
    float metallic = uMetallic;
    float roughness = uRoughness;
    if (uHasMetallicTexture) {
        metallic *= texture(uMetallicTexture, texCoord).r;
    }
    if (uHasRoughnessTexture) {
        roughness *= texture(uRoughnessTexture, texCoord).r;
    }
    
    // Clamp valores para prevenir artefactos
    metallic = clamp(metallic, 0.0, 1.0);
    roughness = clamp(roughness, 0.04, 1.0);
    
    // Sample normal map
    vec3 N = normalize(Normal);
    if (uHasNormalTexture) {
        vec3 normalMap = texture(uNormalTexture, texCoord).rgb;
        normalMap = normalMap * 2.0 - 1.0;
        normalMap.xy *= uNormalStrength;
        N = normalize(TBN * normalMap);
    }
    
    // Sample AO
    float ao = 1.0;
    if (uHasAOTexture) {
        ao = texture(uAOTexture, texCoord).r;
    }
    
    // Sample emissive
    vec3 emissive = uEmissive;
    if (uHasEmissiveTexture) {
        emissive *= texture(uEmissiveTexture, texCoord).rgb;
    }
    
    // View direction
    vec3 V = normalize(uViewPos - FragPos);
    
    // Verificar que V es válido
    if (length(V) < 0.1) V = vec3(0.0, 0.0, 1.0);
    
    // Calculate lighting
    vec3 Lo = vec3(0.0);
    
    // Convert roughness to shininess for Blinn-Phong
    float shininess = (1.0 - roughness) * 128.0 + 1.0;
    
    // Directional Light
    if (uHasDirLight) {
        vec3 L = normalize(-uDirLightDirection);
        vec3 lightContrib;
        
        if (uUsePBR) {
            // Ajustar intensidad basada en el ángulo de incidencia
            float NdotL = max(dot(N, L), 0.0);
            vec3 adjustedColor = uDirLightColor * uDirLightIntensity * NdotL;
            lightContrib = CalculatePBRLighting(albedo, metallic, roughness, N, V, L, adjustedColor);
        } else {
            lightContrib = CalculateBlinnPhongLighting(albedo, shininess, N, V, L, uDirLightColor * uDirLightIntensity);
        }
        
        Lo += lightContrib;
    }
    
    // Point Lights
    for (int i = 0; i < uNumPointLights; i++) {
        vec3 L = normalize(uPointLightPositions[i] - FragPos);
        float distance = length(uPointLightPositions[i] - FragPos);
        
        // Early exit si está fuera del rango
        if (distance > uPointLightMaxDistances[i]) continue;
        
        // Atenuación física más realista con rango configurable
        float constant = uPointLightAttenuations[i].x;
        float linear = uPointLightAttenuations[i].y;
        float quadratic = uPointLightAttenuations[i].z;
        
        // Calcular atenuación base
        float attenuation = 1.0 / (constant + linear * distance + quadratic * distance * distance);
        
        // Aplicar rango mínimo y máximo
        float rangeAttenuation = 1.0;
        if (distance < uPointLightMinDistances[i]) {
            // Suavizar transición cerca de la luz
            rangeAttenuation = smoothstep(0.0, uPointLightMinDistances[i], distance);
        } else if (distance > uPointLightMaxDistances[i] * 0.75) {
            // Suavizar transición en el borde máximo
            rangeAttenuation = smoothstep(uPointLightMaxDistances[i], uPointLightMaxDistances[i] * 0.75, distance);
        }
        
        attenuation *= rangeAttenuation;
        
        vec3 lightColor = uPointLightColors[i] * uPointLightIntensities[i] * attenuation;
        vec3 lightContrib;
        
        if (uUsePBR) {
            lightContrib = CalculatePBRLighting(albedo, metallic, roughness, N, V, L, lightColor);
        } else {
            lightContrib = CalculateBlinnPhongLighting(albedo, shininess, N, V, L, lightColor);
        }
        
        Lo += lightContrib;
    }
    
    // Spot Lights
    for (int i = 0; i < uNumSpotLights; i++) {
        vec3 lightDir = uSpotLightPositions[i] - FragPos;
        float distance = length(lightDir);
        
        // Early exit si está fuera del rango
        if (distance > uSpotLightRanges[i]) continue;
        
        vec3 L = normalize(lightDir);
        
        // Calcular atenuación por distancia con rango máximo
        float distanceRatio = distance / uSpotLightRanges[i];
        float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);
        attenuation *= 1.0 - smoothstep(0.75, 1.0, distanceRatio);
        
        // Calcular ángulo entre dirección de la luz y dirección al fragmento
        float theta = dot(L, normalize(-uSpotLightDirections[i]));
        float epsilon = cos(uSpotLightCutOffs[i]) - cos(uSpotLightOuterCutOffs[i]);
        float spotIntensity = clamp((theta - cos(uSpotLightOuterCutOffs[i])) / epsilon, 0.0, 1.0);
        
        // Suavizar los bordes del cono de luz
        spotIntensity = smoothstep(0.0, 1.0, spotIntensity);
        
        // Agregar atenuación radial desde el centro del cono
        float radialFalloff = 1.0 - length(cross(L, normalize(-uSpotLightDirections[i])));
        radialFalloff = smoothstep(0.0, 0.5, radialFalloff);
        
        // Combinar atenuaciones
        float finalIntensity = spotIntensity * attenuation * radialFalloff * uSpotLightIntensities[i];
        
        // Early exit si la luz no afecta a este fragmento
        if (finalIntensity <= 0.001) continue;
        
        vec3 lightColor = uSpotLightColors[i] * finalIntensity;
        vec3 lightContrib;
        
        if (uUsePBR) {
            lightContrib = CalculatePBRLighting(albedo, metallic, roughness, N, V, L, lightColor);
        } else {
            lightContrib = CalculateBlinnPhongLighting(albedo, shininess, N, V, L, lightColor);
        }
        
        Lo += lightContrib;
    }
    
    // Ambient lighting (mejorado para PBR)
    vec3 ambient;
    if (uUsePBR) {
        // Ambient basado en metallic/roughness para PBR
        vec3 ambientFactor = mix(vec3(0.03), albedo, metallic);
        ambient = uAmbientLight * albedo * ao * (1.0 - metallic * 0.5);
    } else {
        // Ambient mejorado para preservar saturación
        vec3 ambientColor = uAmbientLight * albedo * ao;
        // Aplicar saturación preservada
        float luminance = dot(ambientColor, vec3(0.299, 0.587, 0.114));
        ambient = mix(ambientColor, albedo * luminance, 0.3); // Preservar 30% del color original
    }
    
    // Final color
    vec3 color = ambient + Lo + emissive;
    
    // Exposure tone mapping mejorado
    color = color / (color + vec3(1.0)); // Reinhard tone mapping
    
    // Aplicar exposure
    color = color * uExposure;
    
    // Aplicar saturación
    float luminance = dot(color, vec3(0.299, 0.587, 0.114));
    color = mix(vec3(luminance), color, uSaturation);
    
    // Gamma correction con smoothness
    color = pow(color, vec3(1.0/uSmoothness)); // Smoothness controla la gamma

    if (alpha < 0.1)
        discard;
    
    FragColor = vec4(color, 1.0);
}
)glsl";

DefaultShaders::DefaultShaders() {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Verificar errores de enlace
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Error al enlazar shaders: " << infoLog << std::endl;
    }
    

}

DefaultShaders::~DefaultShaders() {
    glDeleteProgram(program);
}

GLuint DefaultShaders::getProgram() const {
    return program;
}

GLuint DefaultShaders::compileShader(GLenum type, const std::string& source) {
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    // Verificar errores de compilaci�n
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Error al compilar shader: " << infoLog << std::endl;
    }

    return shader;
}

// Post-processing uniform setters
void DefaultShaders::setExposure(float exposure) {
    glUseProgram(program);
    GLint location = glGetUniformLocation(program, "uExposure");
    if (location != -1) {
        glUniform1f(location, exposure);
    }
}

void DefaultShaders::setSaturation(float saturation) {
    glUseProgram(program);
    GLint location = glGetUniformLocation(program, "uSaturation");
    if (location != -1) {
        glUniform1f(location, saturation);
    }
}

void DefaultShaders::setSmoothness(float smoothness) {
    glUseProgram(program);
    GLint location = glGetUniformLocation(program, "uSmoothness");
    if (location != -1) {
        glUniform1f(location, smoothness);
    }
}

// Post-processing uniform getters
float DefaultShaders::getExposure() const {
    GLint location = glGetUniformLocation(program, "uExposure");
    if (location != -1) {
        float value;
        glGetUniformfv(program, location, &value);
        return value;
    }
    return 1.0f; // Default value
}



float DefaultShaders::getSaturation() const {
    GLint location = glGetUniformLocation(program, "uSaturation");
    if (location != -1) {
        float value;
        glGetUniformfv(program, location, &value);
        return value;
    }
    return 1.0f; // Default value
}

float DefaultShaders::getSmoothness() const {
    GLint location = glGetUniformLocation(program, "uSmoothness");
    if (location != -1) {
        float value;
        glGetUniformfv(program, location, &value);
        return value;
    }
    return 1.0f; // Default value
}



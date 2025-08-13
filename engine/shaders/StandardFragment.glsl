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

    return nom / max(denom, 0.0001);
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / max(denom, 0.0001);
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
    roughness = clamp(roughness, 0.04, 1.0);
    metallic = clamp(metallic, 0.0, 1.0);
    
    vec3 H = normalize(V + L);
    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.0);
    float HdotV = max(dot(H, V), 0.0);
    
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
    float NdotL = max(dot(N, L), 0.0);
    vec3 diffuse = albedo * NdotL;
    
    vec3 H = normalize(V + L);
    float NdotH = max(dot(N, H), 0.0);
    float spec = pow(NdotH, shininess);
    
    vec3 specularColor = mix(vec3(0.04), albedo, 0.3);
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
        alpha = albedoSample.a; // Usar el canal alpha de la textura
    }

    float metallic = uMetallic;
    float roughness = uRoughness;
    if (uHasMetallicTexture) {
        metallic *= texture(uMetallicTexture, texCoord).r;
    }
    if (uHasRoughnessTexture) {
        roughness *= texture(uRoughnessTexture, texCoord).r;
    }
    
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
        
        if (distance > uPointLightMaxDistances[i]) continue;
        
        float constant = uPointLightAttenuations[i].x;
        float linear = uPointLightAttenuations[i].y;
        float quadratic = uPointLightAttenuations[i].z;
        
        float attenuation = 1.0 / (constant + linear * distance + quadratic * distance * distance);
        
        float rangeAttenuation = 1.0;
        if (distance < uPointLightMinDistances[i]) {
            rangeAttenuation = smoothstep(0.0, uPointLightMinDistances[i], distance);
        } else if (distance > uPointLightMaxDistances[i] * 0.75) {
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
        
        if (distance > uSpotLightRanges[i]) continue;
        
        vec3 L = normalize(lightDir);
        
        float distanceRatio = distance / uSpotLightRanges[i];
        float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);
        attenuation *= 1.0 - smoothstep(0.75, 1.0, distanceRatio);
        
        float theta = dot(L, normalize(-uSpotLightDirections[i]));
        float epsilon = cos(uSpotLightCutOffs[i]) - cos(uSpotLightOuterCutOffs[i]);
        float spotIntensity = clamp((theta - cos(uSpotLightOuterCutOffs[i])) / epsilon, 0.0, 1.0);
        
        spotIntensity = smoothstep(0.0, 1.0, spotIntensity);
        
        float radialFalloff = 1.0 - length(cross(L, normalize(-uSpotLightDirections[i]))); 
        radialFalloff = smoothstep(0.0, 0.5, radialFalloff);
        
        float finalIntensity = spotIntensity * attenuation * radialFalloff * uSpotLightIntensities[i];
        
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
    
    // Ambient lighting
    vec3 ambient;
    if (uUsePBR) {
        ambient = uAmbientLight * albedo * ao * (1.0 - metallic * 0.5);
    } else {
        vec3 ambientColor = uAmbientLight * albedo * ao;
        float luminance = dot(ambientColor, vec3(0.299, 0.587, 0.114));
        ambient = mix(ambientColor, albedo * luminance, 0.3);
    }
    
    // Final color
    vec3 color = ambient + Lo + emissive;
    
    // Tone mapping y post-processing
    color = color / (color + vec3(1.0));
    color = color * uExposure;
    
    float luminance = dot(color, vec3(0.299, 0.587, 0.114));
    color = mix(vec3(luminance), color, uSaturation);
    
    color = pow(color, vec3(1.0/uSmoothness));

    if (!uHasAlbedoTexture) {
        color = mix(color, uAlbedo, 0.3);
    }
    
    float minBrightness = 0.05;
    float currentBrightness = max(max(color.r, color.g), color.b);
    if (currentBrightness < minBrightness) {
        color = color * (minBrightness / currentBrightness);
    }
    
    FragColor = vec4(color, alpha); // ← Transparencia aplicada
}

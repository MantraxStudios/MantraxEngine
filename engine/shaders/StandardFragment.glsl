#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;
in mat3 TBN;
in vec4 FragPosLightSpace; // Light space position for simple shadows

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

// Point Lights (m�ximo 4)
uniform int uNumPointLights;
uniform vec3 uPointLightPositions[4];
uniform vec3 uPointLightColors[4];
uniform float uPointLightIntensities[4];
uniform vec3 uPointLightAttenuations[4];
uniform float uPointLightMinDistances[4];
uniform float uPointLightMaxDistances[4];

// Spot Lights (m�ximo 2)
uniform int uNumSpotLights;
uniform vec3 uSpotLightPositions[2];
uniform vec3 uSpotLightDirections[2];
uniform vec3 uSpotLightColors[2];
uniform float uSpotLightIntensities[2];
uniform float uSpotLightCutOffs[2];
uniform float uSpotLightOuterCutOffs[2];
uniform float uSpotLightRanges[2];

// Simple Shadow Mapping
uniform bool uEnableShadows = true;
uniform sampler2DShadow uShadowMap;
uniform float uShadowBias = 0.001;
uniform float uShadowStrength = 0.7;

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

    return nom / max(denom, 0.0001); // Prevenir divisi�n por cero
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / max(denom, 0.0001); // Prevenir divisi�n por cero
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

// CORREGIDO: PCF mejorado para sombras m�s suaves
float SampleShadowMapPCF(vec3 projCoords, float bias) {
    vec2 texelSize = 1.0 / textureSize(uShadowMap, 0);
    float shadow = 0.0;
    int samples = 0;
    
    // Aplicar bias a la coordenada Z una sola vez
    float biasedDepth = projCoords.z - bias;
    
    // CORREGIDO: PCF adaptivo - menos muestras, mejor rendimiento y claridad
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            vec2 offset = vec2(x, y) * texelSize * 0.25; // Sampling m�s fino para evitar blur
            vec3 sampleCoords = vec3(projCoords.xy + offset, biasedDepth);
            shadow += texture(uShadowMap, sampleCoords);
            samples++;
        }    
    }
    
    return shadow / float(samples);
}

// Versi�n m�nima - casi id�ntica a tu c�digo original
float CalculateShadow(vec3 lightDir) {
    if (!uEnableShadows || !uHasDirLight) {
        return 1.0;
    }
    
    // CORREGIDO: Verificar distancia desde la c�mara para sombras adaptivas
    float distanceFromCamera = length(uViewPos - FragPos);
    if (distanceFromCamera > 20.0) { // M�xima distancia de sombras
        return 1.0; // Sin sombras muy lejos
    }
    
    // Perform perspective divide
    vec3 projCoords = FragPosLightSpace.xyz / FragPosLightSpace.w;
    
    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    
    // Check if we're outside the shadow map bounds
    if (projCoords.x < 0.0 || projCoords.x > 1.0 || 
        projCoords.y < 0.0 || projCoords.y > 1.0 || 
        projCoords.z > 1.0) {
        return 1.0; // Outside shadow map
    }
    
    // CORREGIDO: Bias adaptivo basado en distancia y �ngulo
    vec3 normal = normalize(Normal);
    float NdotL = max(dot(normal, lightDir), 0.0);
    
    // Bias m�s peque�o para objetos cercanos, m�s grande para lejanos
    float distanceFactor = smoothstep(0.5, 10.0, distanceFromCamera);
    float baseBias = mix(uShadowBias * 0.2, uShadowBias, distanceFactor);
    
    float dynamicBias = baseBias * (1.0 + (1.0 - NdotL) * 0.3);
    float bias = clamp(dynamicBias, baseBias * 0.5, baseBias * 2.0);
    
    // Use PCF for smoother shadows (igual que tu c�digo original)
    float shadow = SampleShadowMapPCF(projCoords, bias);
    
    // CORREGIDO: Apply shadow strength con desvanecimiento suave por distancia
    shadow = smoothstep(0.0, 1.0, shadow);
    
    // Desvanecimiento suave de sombras basado en distancia
    float fadeStart = 15.0;
    float fadeEnd = 20.0;
    float fadeFactor = smoothstep(fadeStart, fadeEnd, distanceFromCamera);
    
    // Mezclar entre sombra normal y sin sombra basado en distancia
    float finalShadow = mix(shadow, 1.0, fadeFactor);
    return mix(1.0 - uShadowStrength, 1.0, finalShadow);
}

// Mant�n tu funci�n SampleShadowMapPCF original tal como estaba

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
    
    // Verificar que V es v�lido
    if (length(V) < 0.1) V = vec3(0.0, 0.0, 1.0);
    
    // Calculate lighting
    vec3 Lo = vec3(0.0);
    
    // Convert roughness to shininess for Blinn-Phong
    float shininess = (1.0 - roughness) * 128.0 + 1.0;
    
    // Directional Light
    if (uHasDirLight) {
        vec3 L = normalize(-uDirLightDirection);
        vec3 lightContrib;
        
        // Calculate shadow factor
        float shadowFactor = CalculateShadow(L);
        
        if (uUsePBR) {
            // Ajustar intensidad basada en el �ngulo de incidencia
            float NdotL = max(dot(N, L), 0.0);
            vec3 adjustedColor = uDirLightColor * uDirLightIntensity * NdotL;
            lightContrib = CalculatePBRLighting(albedo, metallic, roughness, N, V, L, adjustedColor);
        } else {
            lightContrib = CalculateBlinnPhongLighting(albedo, shininess, N, V, L, uDirLightColor * uDirLightIntensity);
        }
        
        // Apply shadow to directional light contribution
        Lo += lightContrib * shadowFactor;
    }
    
    // Point Lights
    for (int i = 0; i < uNumPointLights; i++) {
        vec3 L = normalize(uPointLightPositions[i] - FragPos);
        float distance = length(uPointLightPositions[i] - FragPos);
        
        // Early exit si est� fuera del rango
        if (distance > uPointLightMaxDistances[i]) continue;
        
        // Atenuaci�n f�sica m�s realista con rango configurable
        float constant = uPointLightAttenuations[i].x;
        float linear = uPointLightAttenuations[i].y;
        float quadratic = uPointLightAttenuations[i].z;
        
        // Calcular atenuaci�n base
        float attenuation = 1.0 / (constant + linear * distance + quadratic * distance * distance);
        
        // Aplicar rango m�nimo y m�ximo
        float rangeAttenuation = 1.0;
        if (distance < uPointLightMinDistances[i]) {
            // Suavizar transici�n cerca de la luz
            rangeAttenuation = smoothstep(0.0, uPointLightMinDistances[i], distance);
        } else if (distance > uPointLightMaxDistances[i] * 0.75) {
            // Suavizar transici�n en el borde m�ximo
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
        
        // Early exit si est� fuera del rango
        if (distance > uSpotLightRanges[i]) continue;
        
        vec3 L = normalize(lightDir);
        
        // Calcular atenuaci�n por distancia con rango m�ximo
        float distanceRatio = distance / uSpotLightRanges[i];
        float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);
        attenuation *= 1.0 - smoothstep(0.75, 1.0, distanceRatio);
        
        // Calcular �ngulo entre direcci�n de la luz y direcci�n al fragmento
        float theta = dot(L, normalize(-uSpotLightDirections[i]));
        float epsilon = cos(uSpotLightCutOffs[i]) - cos(uSpotLightOuterCutOffs[i]);
        float spotIntensity = clamp((theta - cos(uSpotLightOuterCutOffs[i])) / epsilon, 0.0, 1.0);
        
        // Suavizar los bordes del cono de luz
        spotIntensity = smoothstep(0.0, 1.0, spotIntensity);
        
        // Agregar atenuaci�n radial desde el centro del cono
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
        // Ambient mejorado para preservar saturaci�n
        vec3 ambientColor = uAmbientLight * albedo * ao;
        // Aplicar saturaci�n preservada
        float luminance = dot(ambientColor, vec3(0.299, 0.587, 0.114));
        ambient = mix(ambientColor, albedo * luminance, 0.3); // Preservar 30% del color original
    }
    
    // Final color
    vec3 color = ambient + Lo + emissive;
    
    // Exposure tone mapping mejorado
    color = color / (color + vec3(1.0)); // Reinhard tone mapping
    
    // Aplicar exposure
    color = color * uExposure;
    
    // Aplicar saturaci�n
    float luminance = dot(color, vec3(0.299, 0.587, 0.114));
    color = mix(vec3(luminance), color, uSaturation);
    
    // Gamma correction con smoothness
    color = pow(color, vec3(1.0/uSmoothness)); // Smoothness controla la gamma

    if (alpha < 0.1)
        discard;
    
    FragColor = vec4(color, 1.0);
}

# CSM (Cascaded Shadow Maps) - Guía de Uso

## Introducción

Cascaded Shadow Maps (CSM) es una técnica avanzada de shadow mapping que proporciona sombras de alta calidad tanto para objetos cercanos como lejanos, optimizando la resolución de los shadow maps mediante múltiples cascadas.

## Características Implementadas

✅ **4 Cascadas Configurables**  
✅ **Distribución Logarítmica-Uniforme** de distancias  
✅ **PCF (Percentage Closer Filtering)** para suavizado  
✅ **Bias Adaptativos** basados en el ángulo de superficie  
✅ **Integración Completa** con PBR y Blinn-Phong  
✅ **Control de Fuerza** de sombras configurable

## Uso Básico

### 1. Configuración Automática

El CSM se inicializa automáticamente cuando se crea el RenderPipeline:

```cpp
// CSM se inicializa automáticamente con:
// - 4 cascadas
// - 2048x2048 shadow maps por cascade
// - Sombras habilitadas por defecto
RenderPipeline pipeline(camera, shaders);
```

### 2. Control de Sombras

```cpp
// Habilitar/deshabilitar sombras
pipeline.enableShadows(true);

// Configurar calidad de shadow maps
pipeline.setShadowMapSize(4096); // Más alta calidad (más costoso)
pipeline.setShadowMapSize(1024); // Menor calidad (más rápido)

// Ajustar bias para prevenir shadow acne
pipeline.setShadowBias(0.005f); // Valor por defecto
pipeline.setShadowBias(0.001f); // Menos bias (más detalle, posible acne)
pipeline.setShadowBias(0.01f);  // Más bias (menos acne, menos detalle)

// Controlar fuerza de las sombras
pipeline.setShadowStrength(0.8f); // 80% de intensidad (por defecto)
pipeline.setShadowStrength(1.0f); // Sombras completamente negras
pipeline.setShadowStrength(0.5f); // Sombras más suaves

// Habilitar/deshabilitar PCF (suavizado)
pipeline.setEnablePCF(true); // Sombras suaves (por defecto)
pipeline.setEnablePCF(false); // Sombras duras (más rápido)
```

### 3. Configuración de Luz Direccional

Para que CSM funcione, necesitas al menos una luz direccional:

```cpp
// Crear luz direccional para sombras
auto sunLight = std::make_shared<Light>(LightType::Directional);
sunLight->setDirection(glm::vec3(-0.5f, -1.0f, -0.3f)); // Sol desde arriba
sunLight->setColor(glm::vec3(1.0f, 0.95f, 0.8f)); // Luz cálida
sunLight->setIntensity(3.0f);
sunLight->setEnabled(true);

// Agregar al pipeline
pipeline.AddLight(sunLight);
```

### 4. Configuración Avanzada

```cpp
// Acceso directo al ShadowManager para configuración avanzada
ShadowManager* shadowMgr = pipeline.getShadowManager();

// Configurar distancias de cascadas personalizadas
std::vector<float> customDistances = {0.1f, 5.0f, 20.0f, 50.0f, 100.0f};
shadowMgr->setCascadeDistances(customDistances);

// Configurar parámetros de luz
shadowMgr->setLightSize(15.0f); // Tamaño de la fuente de luz para sombras más suaves
```

## Configuración Recomendada por Escenario

### Escenas Interiores

```cpp
pipeline.setShadowMapSize(1024);  // Menor resolución suficiente
pipeline.setShadowBias(0.002f);   // Bias menor para más detalle
pipeline.setShadowStrength(0.6f); // Sombras más suaves
```

### Escenas Exteriores Grandes

```cpp
pipeline.setShadowMapSize(2048);  // Resolución estándar
pipeline.setShadowBias(0.005f);   // Bias estándar
pipeline.setShadowStrength(0.8f); // Sombras más definidas
```

### Máxima Calidad

```cpp
pipeline.setShadowMapSize(4096);  // Alta resolución
pipeline.setShadowBias(0.001f);   // Mínimo bias
pipeline.setShadowStrength(0.9f); // Sombras fuertes
pipeline.setEnablePCF(true);      // Suavizado activado
```

### Máximo Rendimiento

```cpp
pipeline.setShadowMapSize(512);   // Resolución mínima
pipeline.setShadowBias(0.01f);    // Bias alto
pipeline.setShadowStrength(0.5f); // Sombras suaves
pipeline.setEnablePCF(false);     // Sin suavizado
```

## Debug y Visualización

### Información de Estado

```cpp
// Verificar estado de CSM
bool shadowsEnabled = pipeline.getShadowsEnabled();
int shadowMapSize = pipeline.getShadowMapSize();
float bias = pipeline.getShadowBias();
float strength = pipeline.getShadowStrength();
bool pcfEnabled = pipeline.getPCFEnabled();

std::cout << "Shadows: " << (shadowsEnabled ? "ON" : "OFF") << std::endl;
std::cout << "Shadow Map Size: " << shadowMapSize << "x" << shadowMapSize << std::endl;
std::cout << "Bias: " << bias << std::endl;
std::cout << "Strength: " << strength << std::endl;
std::cout << "PCF: " << (pcfEnabled ? "ON" : "OFF") << std::endl;
```

### Cascade Debugging

En el shader, puedes visualizar las cascadas usando colores:

```glsl
// En el fragment shader, después del cálculo de sombras
if (uEnableShadows) {
    int cascadeIndex = SelectCascade(FragDepth);

    // Debug: colorear según cascade (opcional)
    vec3 debugColors[4] = vec3[](
        vec3(1.0, 0.0, 0.0), // Cascade 0: Rojo
        vec3(0.0, 1.0, 0.0), // Cascade 1: Verde
        vec3(0.0, 0.0, 1.0), // Cascade 2: Azul
        vec3(1.0, 1.0, 0.0)  // Cascade 3: Amarillo
    );

    // Mezclar con color de debug para visualizar cascades
    // color = mix(color, debugColors[cascadeIndex], 0.3);
}
```

## Integración con Editor

Si estás usando el sistema de editor de Mantrax Engine, puedes agregar controles UI para CSM:

```cpp
// En tu código de editor UI (ImGui)
if (ImGui::CollapsingHeader("Shadow Settings")) {
    bool shadowsEnabled = pipeline.getShadowsEnabled();
    if (ImGui::Checkbox("Enable Shadows", &shadowsEnabled)) {
        pipeline.enableShadows(shadowsEnabled);
    }

    int shadowMapSize = pipeline.getShadowMapSize();
    if (ImGui::SliderInt("Shadow Map Size", &shadowMapSize, 512, 4096)) {
        pipeline.setShadowMapSize(shadowMapSize);
    }

    float bias = pipeline.getShadowBias();
    if (ImGui::SliderFloat("Shadow Bias", &bias, 0.0001f, 0.1f, "%.4f")) {
        pipeline.setShadowBias(bias);
    }

    float strength = pipeline.getShadowStrength();
    if (ImGui::SliderFloat("Shadow Strength", &strength, 0.0f, 1.0f)) {
        pipeline.setShadowStrength(strength);
    }

    bool pcfEnabled = pipeline.getPCFEnabled();
    if (ImGui::Checkbox("Enable PCF", &pcfEnabled)) {
        pipeline.setEnablePCF(pcfEnabled);
    }
}
```

## Notas de Rendimiento

- **Shadow Map Size**: Es el factor más importante. 2048x2048 es un buen balance.
- **PCF**: Agrega costo pero mejora mucho la calidad visual.
- **Número de Cascadas**: 4 cascadas es óptimo para la mayoría de escenas.
- **Frustum Culling**: CSM respeta el frustum culling del pipeline principal.

## Troubleshooting

### Sombras no aparecen

1. Verificar que hay una luz direccional habilitada
2. Verificar que `uEnableShadows` está en true
3. Verificar que los objetos están en el rango de las cascadas

### Shadow Acne (artefactos)

1. Aumentar `shadowBias`
2. Verificar orientación de normales
3. Aumentar resolución de shadow map

### Sombras muy suaves

1. Disminuir `shadowBias`
2. Aumentar resolución de shadow map
3. Ajustar `lightSize` en ShadowManager

### Rendimiento bajo

1. Reducir resolución de shadow map
2. Deshabilitar PCF
3. Reducir número de objetos que proyectan sombras

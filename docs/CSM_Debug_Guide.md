# Guía de Debug para CSM - Solución de Problemas

## Pasos para Diagnosticar Sombras

### 1. Verificar Logs de Consola

Ahora que agregamos logging extensivo, ejecuta tu aplicación y busca estos mensajes en la consola:

#### ✅ **Logs Esperados (Todo Funciona):**

```
ShadowManager initialized successfully
RenderPipeline: Executing shadow pass (shadows enabled)
RenderPipeline: Looking for directional lights (X total lights)
RenderPipeline: Checking light - Type: 0, Enabled: 1  // Type 0 = Directional
RenderPipeline: Found directional light for shadows
ShadowManager: Beginning shadow pass with 4 cascades
ShadowManager: Light direction: (-0.5, -1, -0.3)
ShadowManager: Shadow shader program: XXXXX
RenderPipeline: Rendering 4 cascades with X scene objects
ShadowManager: Rendering cascade 0 (FB: XXXXX, Size: 2048)
RenderPipeline: Setting up shadow uniforms...
RenderPipeline: Shadows enabled in shader
```

#### ❌ **Problemas Comunes y Sus Logs:**

**Problema 1: No hay luz direccional**

```
RenderPipeline: Looking for directional lights (0 total lights)
RenderPipeline: No directional light found for shadows!
```

**Problema 2: Sombras deshabilitadas**

```
RenderPipeline: Skipping shadow pass (shadows disabled)
```

**Problema 3: ShadowManager no inicializado**

```
RenderPipeline: Cannot render shadows - shadowManager: null
```

### 2. Código de Configuración Mínimo

Si no ves las sombras, prueba este código básico en tu escena:

```cpp
// 1. Asegurar que el pipeline tiene sombras habilitadas
pipeline.enableShadows(true);

// 2. Crear una luz direccional simple
auto sunLight = std::make_shared<Light>(LightType::Directional);
sunLight->setDirection(glm::vec3(-0.5f, -1.0f, -0.3f)); // Sol desde arriba
sunLight->setColor(glm::vec3(1.0f, 1.0f, 1.0f));       // Luz blanca
sunLight->setIntensity(2.0f);                           // Intensidad moderada
sunLight->setEnabled(true);                             // ¡IMPORTANTE!

// 3. Agregar la luz al pipeline
pipeline.AddLight(sunLight);

// 4. Verificar configuración
std::cout << "Shadows enabled: " << pipeline.getShadowsEnabled() << std::endl;
std::cout << "Shadow map size: " << pipeline.getShadowMapSize() << std::endl;
std::cout << "Shadow bias: " << pipeline.getShadowBias() << std::endl;
```

### 3. Verificar Objetos en la Escena

```cpp
// En tu loop de render, agrega esto para debug:
std::cout << "Scene objects count: " << sceneObjects.size() << std::endl;

// Para cada objeto, verifica:
for (GameObject* obj : sceneObjects) {
    if (obj->hasGeometry()) {
        std::cout << "Object has geometry: " << obj->getName() << std::endl;
    }
}
```

### 4. Configuraciones de Debugging

#### Debug Rápido - Sombras Fuertes

```cpp
pipeline.setShadowBias(0.001f);      // Bias muy bajo
pipeline.setShadowStrength(1.0f);    // Sombras 100% negras
pipeline.setEnablePCF(false);        // Sin suavizado para ver bordes claros
```

#### Debug de Cascadas (En el Shader)

Puedes temporalmente colorear las cascadas. En `DefaultShaders.cpp`, encuentra la función `CalculateShadow` y agrega al final:

```glsl
// Debug: Colorear según cascade (temporal)
int cascadeIndex = SelectCascade(FragDepth);
vec3 debugColors[4] = vec3[](
    vec3(1.0, 0.5, 0.5), // Cascade 0: Rojizo
    vec3(0.5, 1.0, 0.5), // Cascade 1: Verdoso
    vec3(0.5, 0.5, 1.0), // Cascade 2: Azulado
    vec3(1.0, 1.0, 0.5)  // Cascade 3: Amarillo
);

// Mezclar con color de debug
FragColor.rgb = mix(FragColor.rgb, debugColors[cascadeIndex], 0.3);
```

### 5. Checklist de Problemas Comunes

- [ ] **¿Hay una luz direccional?** - Debe ser `LightType::Directional`
- [ ] **¿La luz está habilitada?** - `light->setEnabled(true)`
- [ ] **¿Las sombras están habilitadas?** - `pipeline.enableShadows(true)`
- [ ] **¿Hay objetos en la escena?** - Verificar `sceneObjects.size() > 0`
- [ ] **¿Los objetos tienen geometría?** - `obj->hasGeometry()` debe ser true
- [ ] **¿La dirección de luz es correcta?** - Debe apuntar hacia abajo (Y negativo)
- [ ] **¿El bias es apropiado?** - Muy alto = no sombras, muy bajo = artefactos

### 6. Configuraciones según el Problema

#### No aparecen sombras en absoluto:

```cpp
pipeline.setShadowStrength(1.0f);    // Máxima fuerza
pipeline.setShadowBias(0.0f);        // Sin bias (puede causar artefactos)
sunLight->setIntensity(5.0f);        // Luz muy intensa
```

#### Sombras muy claras:

```cpp
pipeline.setShadowStrength(0.9f);    // Incrementar fuerza
pipeline.setLowAmbient(true);        // Reducir luz ambiental
pipeline.setAmbientIntensity(0.1f);  // Luz ambiental mínima
```

#### Artefactos de sombra (shadow acne):

```cpp
pipeline.setShadowBias(0.01f);       // Incrementar bias
```

### 7. Test de Iluminación Base

Antes de probar sombras, asegúrate de que la iluminación básica funciona:

```cpp
// Deshabilitar sombras temporalmente
pipeline.enableShadows(false);

// Verificar que los objetos se ven diferentes con/sin la luz direccional
// Con luz: objetos bien iluminados
// Sin luz: objetos muy oscuros (solo ambient)
```

### 8. Comando de Debug para ImGui (Opcional)

Si usas ImGui en tu editor:

```cpp
if (ImGui::Begin("Shadow Debug")) {
    ImGui::Text("Shadows Enabled: %s", pipeline.getShadowsEnabled() ? "YES" : "NO");
    ImGui::Text("Shadow Map Size: %d", pipeline.getShadowMapSize());
    ImGui::Text("Lights Count: %zu", lights.size());
    ImGui::Text("Scene Objects: %zu", sceneObjects.size());

    if (ImGui::Button("Toggle Shadows")) {
        pipeline.enableShadows(!pipeline.getShadowsEnabled());
    }

    float bias = pipeline.getShadowBias();
    if (ImGui::SliderFloat("Shadow Bias", &bias, 0.0f, 0.1f, "%.4f")) {
        pipeline.setShadowBias(bias);
    }

    float strength = pipeline.getShadowStrength();
    if (ImGui::SliderFloat("Shadow Strength", &strength, 0.0f, 1.0f)) {
        pipeline.setShadowStrength(strength);
    }
}
ImGui::End();
```

## Próximos Pasos

1. **Ejecuta tu aplicación** y revisa los logs de consola
2. **Identifica en qué paso falla** usando los logs
3. **Aplica la solución correspondiente** según el problema detectado
4. **Ajusta parámetros** hasta ver las sombras
5. **Optimiza configuración** una vez que funcionen

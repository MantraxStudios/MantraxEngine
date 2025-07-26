# Flujo de Objetos y RenderPipeline

## Diagrama del Flujo

```
┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐
│   TestScene     │    │  SceneManager    │    │ RenderPipeline  │
│                 │    │                  │    │                 │
│ initialize()    │───▶│ setupRenderPipe- │───▶│ AddGameObject() │
│                 │    │ line()           │    │                 │
│ addGameObject() │    │                  │    │                 │
└─────────────────┘    └──────────────────┘    └─────────────────┘
         │                       │                       │
         │                       │                       │
         ▼                       ▼                       ▼
┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐
│ Scene Objects   │    │ Sync Objects     │    │ Pipeline Objects│
│ List            │    │ to Pipeline      │    │ List            │
│                 │    │                  │    │                 │
│ - RedCube       │    │ For each obj:    │    │ - RedCube       │
│ - BlueCube      │    │ pipeline.Add-    │    │ - BlueCube      │
│ - GreenCube     │    │ GameObject(obj)  │    │ - GreenCube     │
└─────────────────┘    └──────────────────┘    └─────────────────┘
```

## Flujo Detallado

### 1. **Inicialización de Escenas (Editor)**

```cpp
// Editor.cpp - main()
sceneManager.initializeAllScenes();           // 1. Inicializa todas las escenas
sceneManager.setupRenderPipeline(pipeline);   // 2. Sincroniza objetos con RenderPipeline
```

**¿Qué hace `initializeAllScenes()`?**

- Llama a `TestScene::initialize()`
- TestScene crea objetos y los agrega con `addGameObject()`
- Los objetos quedan en la lista de la escena

**¿Qué hace `setupRenderPipeline()`?**

- Toma todos los objetos de la escena activa
- Los agrega al RenderPipeline con `pipeline.AddGameObject()`
- Los objetos quedan en la lista del RenderPipeline

### 2. **Inicialización de Escenas (CoreWrapper)**

```cpp
// CoreWrapper.cpp - Initialize()
sceneManager.setupRenderPipeline(*m_pipeline);  // Sincroniza objetos con RenderPipeline
```

### 3. **Cambio de Escenas**

```cpp
// CoreWrapper.cpp - Update()
if (newActiveScene != m_activeScene) {
    m_activeScene = newActiveScene;
    m_pipeline->clearGameObjects();              // Limpia objetos anteriores
    m_pipeline->clearLights();                   // Limpia luces anteriores
    sceneManager.setupRenderPipeline(*m_pipeline); // Sincroniza nuevos objetos
}
```

## Comparación: Escenas Estáticas vs Runtime

### **Escenas Estáticas (TestScene, TexturedScene, ModelScene)**

```cpp
// TestScene::initialize()
auto* redCube = new GameObject(cubeGeometry);
redCube->Name = "RedCube";
redCube->setLocalPosition({ -2.0f, 0.0f, 0.0f });
redCube->setMaterial(redMaterial);
addGameObject(redCube);  // ✅ Solo esto es suficiente

// SceneManager::setupRenderPipeline() se llama automáticamente
for (auto* obj : activeScene->getGameObjects()) {
    pipeline.AddGameObject(obj);  // ✅ Sincronización automática
}
```

### **Objetos en Runtime (MainBar)**

**Antes (Manual):**

```cpp
// MainBar.cpp
activeScene->addGameObject(NewObject);        // ✅ Agregar a escena
pipeline->AddGameObject(NewObject);           // ✅ Agregar manualmente al pipeline
```

**Ahora (Automático):**

```cpp
// MainBar.cpp
activeScene->addGameObject(NewObject);        // ✅ Sincronización automática

// Scene.h - addGameObject() mejorado
void addGameObject(GameObject* object) {
    if (object) {
        gameObjects.push_back(object);

        // Sincronizar automáticamente con RenderPipeline
        if (renderPipeline) {
            renderPipeline->AddGameObject(object);  // ✅ Automático
        }
    }
}
```

## Puntos Clave

### **¿Por qué se ven los objetos de TestScene?**

1. **TestScene::initialize()** crea objetos y los agrega a la escena
2. **SceneManager::setupRenderPipeline()** se llama automáticamente
3. **setupRenderPipeline()** toma todos los objetos de la escena y los agrega al RenderPipeline
4. **Los objetos se ven** porque están en ambos lugares

### **¿Por qué NO se veían los objetos de MainBar?**

1. **MainBar** creaba objetos y los agregaba solo a la escena
2. **NO se llamaba setupRenderPipeline()** automáticamente
3. **Los objetos NO se veían** porque solo estaban en la escena, no en el RenderPipeline

### **¿Por qué ahora SÍ se ven?**

1. **addGameObject() mejorado** sincroniza automáticamente con el RenderPipeline
2. **Los objetos se ven** porque se agregan a ambos lugares automáticamente

## Métodos Importantes

### **SceneManager::setupRenderPipeline()**

```cpp
void SceneManager::setupRenderPipeline(RenderPipeline& pipeline) {
    if (!activeScene) return;

    // Agrega TODOS los objetos de la escena al RenderPipeline
    for (auto* obj : activeScene->getGameObjects()) {
        pipeline.AddGameObject(obj);
    }

    // Agrega TODAS las luces de la escena al RenderPipeline
    for (const auto& light : activeScene->getLights()) {
        pipeline.AddLight(light);
    }
}
```

### **Scene::addGameObject() (Mejorado)**

```cpp
void addGameObject(GameObject* object) {
    if (object) {
        gameObjects.push_back(object);

        // Sincronizar automáticamente con RenderPipeline si está disponible
        if (renderPipeline) {
            renderPipeline->AddGameObject(object);
        }
    }
}
```

## Casos de Uso

### **Crear Objetos en Escenas Estáticas**

```cpp
// En TestScene::initialize()
auto* obj = new GameObject(geometry);
obj->setLocalPosition({ 0.0f, 0.0f, 0.0f });
addGameObject(obj);  // ✅ Automático
```

### **Crear Objetos en Runtime**

```cpp
// En MainBar.cpp
auto* obj = new GameObject(geometry);
obj->setLocalPosition({ 0.0f, 0.0f, 0.0f });
activeScene->addGameObject(obj);  // ✅ Automático
```

### **Crear Objetos sin Sincronización (Casos Especiales)**

```cpp
// Para objetos que no deben renderizarse
activeScene->addGameObjectNoSync(obj);  // ✅ Solo en escena
```

## Ventajas del Sistema Mejorado

1. **Consistencia**: Todos los objetos se manejan igual
2. **Simplicidad**: No hay que recordar llamar `pipeline->AddGameObject()`
3. **Robustez**: Menos errores de objetos no visibles
4. **Flexibilidad**: `addGameObjectNoSync()` para casos especiales

# Creación de Objetos en Runtime

## Descripción

El sistema permite crear objetos dinámicamente durante la ejecución del programa a través de la interfaz del editor. Los objetos se pueden crear con diferentes tipos de geometría y materiales.

## Funcionalidades Disponibles

### 1. **New Object** - Objeto Básico

- Crea un objeto con geometría nativa (cubo)
- Usa material rojo por defecto
- Se posiciona en el origen (0, 0, 0)
- **Se renderiza inmediatamente**

### 2. **New Empty Object** - Objeto Vacío

- Crea un GameObject sin geometría
- No se renderiza (objeto invisible)
- Útil para contenedores o puntos de referencia
- Se posiciona en (2, 0, 0)

### 3. **New Object from Model** - Objeto con Modelo

- Intenta cargar un modelo desde archivo
- Si falla, crea un objeto de respaldo
- Usa material azul si se carga correctamente
- Se posiciona en (-2, 0, 0)

## Cómo Funciona

### Flujo de Creación

```cpp
// 1. Obtener referencias
auto& sceneManager = SceneManager::getInstance();
Scene* activeScene = sceneManager.getActiveScene();
RenderPipeline* pipeline = activeScene->getRenderPipeline();

// 2. Crear el objeto
GameObject* NewObject = new GameObject(geometry);

// 3. Configurar propiedades
NewObject->Name = "New Object";
NewObject->setLocalPosition({ 0.0f, 0.0f, 0.0f });
NewObject->setMaterial(material);

// 4. Agregar a la escena
activeScene->addGameObject(NewObject);

// 5. Agregar al RenderPipeline (IMPORTANTE)
pipeline->AddGameObject(NewObject);
```

### ¿Por qué es Importante `pipeline->AddGameObject()`?

El RenderPipeline mantiene su propia lista de objetos para renderizar. Cuando agregas un objeto solo a la escena con `addGameObject()`, este se agrega a la lista de la escena pero **NO** se agrega a la lista de renderizado del RenderPipeline.

**Sin `pipeline->AddGameObject()`:**

- [ ] Objeto existe en la escena
- [x] Objeto NO se renderiza
- [x] Objeto NO es visible

**Con `pipeline->AddGameObject()`:**

- [ ] Objeto existe en la escena
- [ ] Objeto se renderiza
- [ ] Objeto es visible

## Herramientas de Depuración

### Show Object Count

Muestra información sobre los objetos en la escena:

- **Scene Objects**: Objetos en la lista de la escena
- **Pipeline Objects**: Objetos en la lista del RenderPipeline
- **Visible Objects**: Objetos visibles (pasaron frustum culling)

### Clear All Objects

Limpia todos los objetos de la escena y los vuelve a crear desde cero.

## Casos de Uso

### 1. Crear Objetos Interactivos

```cpp
// Crear objeto que responde a input
GameObject* interactiveObject = new GameObject(geometry);
interactiveObject->Name = "Interactive";
interactiveObject->setLocalPosition({ 0.0f, 0.0f, 0.0f });
activeScene->addGameObject(interactiveObject);
pipeline->AddGameObject(interactiveObject);
```

### 2. Crear Efectos Visuales

```cpp
// Crear partículas o efectos
for (int i = 0; i < 10; i++) {
    GameObject* particle = new GameObject(particleGeometry);
    particle->setLocalPosition({
        static_cast<float>(rand() % 10),
        0.0f,
        static_cast<float>(rand() % 10)
    });
    activeScene->addGameObject(particle);
    pipeline->AddGameObject(particle);
}
```

### 3. Crear UI 3D

```cpp
// Crear elementos de UI en 3D
GameObject* uiElement = new GameObject(uiGeometry);
uiElement->setLocalPosition({ 0.0f, 2.0f, -5.0f });
uiElement->setLocalScale({ 2.0f, 1.0f, 0.1f });
activeScene->addGameObject(uiElement);
pipeline->AddGameObject(uiElement);
```

## Consideraciones de Rendimiento

### Objetos Vacíos

- No consumen recursos de renderizado
- Útiles para lógica y organización
- No afectan el FPS

### Objetos con Geometría

- Consumen recursos de GPU
- Afectan el rendimiento según la complejidad
- Se benefician del frustum culling

### Límites Recomendados

- **Objetos simples**: Hasta 10,000 objetos
- **Objetos complejos**: Hasta 1,000 objetos
- **Objetos vacíos**: Sin límite práctico

## Solución de Problemas

### Objeto No Visible

1. Verificar que se llamó `pipeline->AddGameObject()`
2. Verificar que el objeto tiene geometría (`hasGeometry()`)
3. Verificar que el material existe
4. Verificar la posición (puede estar fuera de la vista)

### Objeto en Posición Incorrecta

1. Verificar `setLocalPosition()`
2. Verificar si hay transformaciones parent-child
3. Verificar la escala del objeto

### Objeto con Material Incorrecto

1. Verificar que el material existe en el pipeline
2. Verificar `setMaterial()` se llamó correctamente
3. Verificar que el material se cargó desde la configuración

## Ejemplo Completo

```cpp
void CreateRuntimeObject() {
    auto& sceneManager = SceneManager::getInstance();
    Scene* activeScene = sceneManager.getActiveScene();
    RenderPipeline* pipeline = activeScene->getRenderPipeline();

    if (pipeline) {
        // Crear objeto
        GameObject* obj = new GameObject(pipeline->createNativeGeometry());
        obj->Name = "Runtime Object";
        obj->setLocalPosition({ 5.0f, 0.0f, 0.0f });
        obj->setLocalRotationEuler({ 0.0f, 45.0f, 0.0f });
        obj->setLocalScale({ 1.5f, 1.5f, 1.5f });
        obj->setMaterial(pipeline->getMaterial("gold_material"));

        // Agregar a ambos sistemas
        activeScene->addGameObject(obj);
        pipeline->AddGameObject(obj);

        std::cout << "Created: " << obj->Name << std::endl;
    }
}
```

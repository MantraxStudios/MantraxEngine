# GameObject - Carga Automática de Modelos

## Descripción

El sistema de GameObject ahora soporta la carga automática de modelos 3D desde archivos. Esto permite crear objetos que cargan automáticamente su geometría desde un path especificado, simplificando el proceso de creación de objetos con modelos.

## Características

- **Carga automática**: Los modelos se cargan automáticamente al crear el GameObject
- **Cache inteligente**: Utiliza el sistema de cache del ModelLoader para evitar cargas duplicadas
- **Flexibilidad**: Permite cargar modelos en cualquier momento durante la vida del objeto
- **Manejo de errores**: Proporciona feedback detallado sobre el éxito o fracaso de la carga

## Constructores con Carga Automática

### Constructor con Path de Modelo

```cpp
// Crear GameObject y cargar modelo automáticamente
GameObject* obj = new GameObject("models/character.fbx");
obj->setLocalPosition({ 0.0f, 0.0f, 0.0f });
```

### Constructor con Path y Material

```cpp
// Crear GameObject con modelo y material
auto material = std::make_shared<Material>();
GameObject* obj = new GameObject("models/weapon.obj", material);
obj->setLocalPosition({ 1.0f, 0.0f, 0.0f });
```

## Métodos de Carga

### Establecer Path del Modelo

```cpp
GameObject* obj = new GameObject();
obj->setModelPath("models/enemy.fbx");
```

### Cargar Modelo desde Path

```cpp
// Cargar desde el path establecido
bool success = obj->loadModelFromPath();

// Cargar desde un path específico
bool success = obj->loadModelFromPath("models/vehicle.obj");
```

### Verificar Estado

```cpp
// Verificar si tiene geometría cargada
if (obj->hasGeometry()) {
    std::cout << "Modelo cargado correctamente" << std::endl;
}

// Obtener el path del modelo
std::string path = obj->getModelPath();
```

## Ejemplos de Uso

### 1. Carga Inmediata

```cpp
// El modelo se carga automáticamente al crear el objeto
GameObject* player = new GameObject("models/player.fbx");
player->Name = "Player";
player->setLocalPosition({ 0.0f, 0.0f, 0.0f });

// Verificar si se cargó correctamente
if (player->hasGeometry()) {
    std::cout << "Player model loaded successfully" << std::endl;
} else {
    std::cout << "Failed to load player model" << std::endl;
}
```

### 2. Carga Retrasada

```cpp
// Crear objeto vacío
GameObject* enemy = new GameObject();
enemy->Name = "Enemy";
enemy->setLocalPosition({ 5.0f, 0.0f, 0.0f });

// Cargar modelo más tarde
enemy->setModelPath("models/enemy.fbx");
if (enemy->loadModelFromPath()) {
    std::cout << "Enemy model loaded" << std::endl;
}
```

### 3. Carga Condicional

```cpp
GameObject* object = new GameObject();
object->Name = "DynamicObject";

// Cargar modelo basado en condiciones
std::string modelPath;
if (isHighQualityMode) {
    modelPath = "models/high_quality.obj";
} else {
    modelPath = "models/low_quality.obj";
}

object->loadModelFromPath(modelPath);
```

### 4. Carga con Material

```cpp
// Crear material
auto material = std::make_shared<Material>();
material->setAlbedo({ 1.0f, 0.0f, 0.0f }); // Rojo

// Crear objeto con modelo y material
GameObject* redCube = new GameObject("models/cube.obj", material);
redCube->setLocalPosition({ 0.0f, 0.0f, 0.0f });
```

## Formatos Soportados

El sistema utiliza Assimp para cargar modelos, por lo que soporta múltiples formatos:

- **FBX** (.fbx)
- **OBJ** (.obj)
- **3DS** (.3ds)
- **DAE** (.dae) - Collada
- **BLEND** (.blend) - Blender
- **PLY** (.ply)
- **STL** (.stl)
- **GLTF/GLB** (.gltf, .glb)
- Y muchos más...

## Sistema de Cache

### Beneficios del Cache

- **Rendimiento**: Los modelos se cargan una sola vez
- **Memoria**: Evita duplicación de datos en memoria
- **Velocidad**: Cargas subsecuentes son instantáneas

### Gestión del Cache

```cpp
// Obtener instancia del ModelLoader
auto& modelLoader = ModelLoader::getInstance();

// Ver información del cache
modelLoader.listLoadedModels();
std::cout << "Cache size: " << modelLoader.getCacheSize() << std::endl;

// Limpiar cache si es necesario
modelLoader.clearCache();
```

## Manejo de Errores

### Verificación de Carga

```cpp
GameObject* obj = new GameObject("models/nonexistent.fbx");

if (!obj->hasGeometry()) {
    std::cerr << "Failed to load model: " << obj->getModelPath() << std::endl;
    // Manejar el error apropiadamente
}
```

### Logs Detallados

El sistema proporciona logs detallados:

```
Loading model for GameObject 'Player' from path: models/player.fbx
Model loaded from cache: models/player.fbx
Successfully loaded model for GameObject 'Player'
```

## Consideraciones de Rendimiento

### Carga Asíncrona

Para cargas pesadas, considera usar carga asíncrona:

```cpp
// Crear objeto vacío
GameObject* heavyObject = new GameObject();
heavyObject->setModelPath("models/large_model.fbx");

// Cargar en un hilo separado
std::thread loadThread([heavyObject]() {
    heavyObject->loadModelFromPath();
});
```

### Optimización de Memoria

- Los modelos se comparten automáticamente entre objetos
- Usa el cache del ModelLoader para evitar cargas duplicadas
- Considera limpiar el cache periódicamente si usas muchos modelos únicos

## Integración con el Sistema Existente

### Compatibilidad

- Totalmente compatible con GameObjects existentes
- No afecta el rendimiento del sistema de renderizado
- Mantiene todas las funcionalidades de transform y jerarquía

### Migración

```cpp
// Código anterior
auto geometry = modelLoader.loadModel("models/cube.obj");
GameObject* obj = new GameObject(geometry);

// Nuevo código
GameObject* obj = new GameObject("models/cube.obj");
```

## Casos de Uso Avanzados

### 1. Carga Dinámica de Niveles

```cpp
void loadLevel(const std::string& levelName) {
    // Cargar objetos del nivel desde archivo de configuración
    for (const auto& objectData : levelObjects) {
        GameObject* obj = new GameObject(objectData.modelPath);
        obj->setLocalPosition(objectData.position);
        obj->setLocalRotationEuler(objectData.rotation);
        addGameObject(obj);
    }
}
```

### 2. Sistema de LOD (Level of Detail)

```cpp
GameObject* createLODObject(float distance) {
    GameObject* obj = new GameObject();

    if (distance < 10.0f) {
        obj->loadModelFromPath("models/high_detail.obj");
    } else if (distance < 50.0f) {
        obj->loadModelFromPath("models/medium_detail.obj");
    } else {
        obj->loadModelFromPath("models/low_detail.obj");
    }

    return obj;
}
```

### 3. Carga de Recursos por Demanda

```cpp
class ResourceManager {
    std::unordered_map<std::string, GameObject*> objectTemplates;

public:
    GameObject* createObject(const std::string& templateName) {
        if (objectTemplates.find(templateName) == objectTemplates.end()) {
            // Cargar template desde archivo
            auto* template = new GameObject("templates/" + templateName + ".obj");
            objectTemplates[templateName] = template;
        }

        // Clonar el template
        return new GameObject(*objectTemplates[templateName]);
    }
};
```

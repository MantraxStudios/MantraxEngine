# Limpieza Completa al Cargar Escenas con SceneSaver

## Descripción

Se ha implementado un sistema de limpieza completa que se ejecuta automáticamente cuando se usa `SceneSaver::LoadScene()`. Este sistema asegura que no queden residuos de la escena anterior al cargar una nueva escena.

## Funcionalidad Implementada

### 1. Limpieza Automática de Escena Anterior

Cuando se llama a `SceneSaver::LoadScene()`, el sistema:

1. **Detecta la escena actual**: Identifica si existe una escena activa
2. **Limpia componentes de física**: Ejecuta `SceneManager::cleanupPhysicsComponents()`
3. **Limpia la escena**: Ejecuta `Scene::cleanup()` para liberar todos los recursos
4. **Limpia el TileEditor**: Borra todos los tiles y resetea el estado del editor
5. **Remueve la escena anterior**: Elimina completamente la escena del SceneManager

### 2. Nuevos Métodos Agregados

#### SceneManager
- **`removeScene(const std::string& sceneName)`**: Remueve completamente una escena del manager

#### TileEditor
- **`clearAllTiles()`**: Limpia todos los tiles y resetea el estado del editor

## Flujo de Limpieza

### Antes de Cargar la Nueva Escena:

```
1. SceneSaver::LoadScene() se ejecuta
2. Se obtiene la escena actual del SceneManager
3. Si existe una escena actual:
   a. Se limpian los componentes de física
   b. Se ejecuta cleanup() en la escena
   c. Se limpia el TileEditor
   d. Se remueve la escena del manager
4. Se procede a cargar la nueva escena
```

### Durante la Carga:

```
1. Se lee el archivo JSON de la nueva escena
2. Se crea una nueva instancia de Scene
3. Se configuran todos los componentes
4. Se agrega la nueva escena al SceneManager
5. Se activa la nueva escena
```

## Beneficios

### 1. Prevención de Memory Leaks
- Todos los recursos de la escena anterior se liberan correctamente
- No hay objetos huérfanos en memoria
- Los componentes de física se limpian completamente

### 2. Estado Limpio
- El TileEditor se resetea completamente
- No hay tiles residuales de la escena anterior
- El cache de texturas se limpia

### 3. Estabilidad del Sistema
- No hay conflictos entre escenas
- Los recursos se gestionan correctamente
- El sistema mantiene un estado consistente

## Logs de Ejemplo

```
SceneSaver: Cleaning up previous scene: MenuScene
SceneManager: Cleaning up physics components for scene: MenuScene
PhysicsManager: Cleaning up scene physics components...
PhysicsManager: Removing all actors from scene...
PhysicsManager: Scene physics components cleanup completed.
SceneManager: Cleaning up Collider for GameObject: MenuButton
SceneManager: Cleaning up Rigidbody for GameObject: MenuButton
SceneManager: Physics components cleanup completed for scene: MenuScene
SceneSaver: Clearing TileEditor data from previous scene
TileEditor: Clearing all tiles and resetting editor state
TileEditor: All tiles cleared successfully
SceneSaver: Previous scene cleaned up successfully
SceneSaver: Removing existing scene with same name: GameScene
SceneManager: Removing scene: GameScene
SceneManager: Scene 'GameScene' removed successfully
Scene loaded successfully. Total objects: 15
```

## Uso

### Automático
La limpieza se ejecuta automáticamente al usar:
```cpp
SceneSaver::LoadScene("path/to/scene.json");
```

### Manual
Si necesitas limpiar manualmente:
```cpp
// Limpiar componentes de física de una escena
SceneManager::getInstance().cleanupPhysicsComponents(scene);

// Remover completamente una escena
SceneManager::getInstance().removeScene("SceneName");

// Limpiar el TileEditor
TileEditor* tileEditor = RenderWindows::getInstance().GetWindow<TileEditor>();
if (tileEditor) {
    tileEditor->clearAllTiles();
}
```

## Consideraciones Técnicas

### 1. Orden de Limpieza
1. **Componentes de física**: Se limpian primero para evitar referencias inválidas
2. **Escena**: Se ejecuta cleanup() para liberar recursos
3. **TileEditor**: Se limpia el estado del editor
4. **SceneManager**: Se remueve la escena del manager

### 2. Manejo de Errores
- Se usan try-catch para manejar excepciones durante la limpieza
- Los errores no detienen el proceso de carga
- Se registran todos los errores en el log

### 3. Compatibilidad
- Compatible con el sistema existente de gestión de escenas
- No interfiere con la funcionalidad existente
- Mantiene la API pública sin cambios

## Casos de Uso

### 1. Cambio de Nivel en Juego
```cpp
// Al cambiar de nivel, se limpia automáticamente todo
SceneSaver::LoadScene("levels/level2.json");
```

### 2. Recarga de Escena
```cpp
// Al recargar la escena actual, se limpia y se vuelve a cargar
SceneSaver::LoadScene("current_scene.json");
```

### 3. Cambio de Escena en Editor
```cpp
// En el editor, al cambiar de escena se limpia todo
SceneSaver::LoadScene("editor_scene.json");
```

## Troubleshooting

### Problema: Escena anterior no se limpia completamente
**Solución**: Verificar que el método `Scene::cleanup()` esté implementado correctamente

### Problema: Tiles residuales en TileEditor
**Solución**: Verificar que `TileEditor::clearAllTiles()` se ejecute correctamente

### Problema: Componentes de física no se limpian
**Solución**: Verificar que `SceneManager::cleanupPhysicsComponents()` funcione

### Problema: Memory leaks al cambiar escenas
**Solución**: Verificar que todos los recursos se liberen en el orden correcto

## Futuras Mejoras

1. **Limpieza asíncrona**: Implementar limpieza en background para escenas grandes
2. **Limpieza selectiva**: Permitir limpiar solo ciertos tipos de recursos
3. **Métricas de limpieza**: Agregar estadísticas sobre el tiempo de limpieza
4. **Limpieza de otros sistemas**: Extender la limpieza a otros sistemas del engine
5. **Rollback de limpieza**: Implementar sistema de rollback en caso de error durante la carga

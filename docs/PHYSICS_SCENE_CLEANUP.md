# Limpieza de Componentes de Física al Cambiar de Escena

## Descripción

Se ha implementado un sistema automático de limpieza de componentes de física (Collider, Rigidbody, PhysicalObject) al cambiar de escena en el MantraxEngine. Esto asegura que no haya conflictos entre escenas y que los recursos de física se liberen correctamente.

## Componentes Afectados

### 1. Collider.cpp
- **Método `destroy()` mejorado**: Ahora incluye logging detallado y limpieza más robusta
- **Limpieza automática**: Se desvincula del Rigidbody antes de liberar recursos
- **Estado reseteado**: Se resetean todas las banderas de estado

### 2. Rigidbody.cpp
- **Método `destroy()` mejorado**: Incluye logging detallado y limpieza robusta
- **Limpieza de actores**: Se remueven correctamente de la escena de PhysX
- **Estado reseteado**: Se resetean todas las banderas de estado

### 3. SceneManager.cpp
- **Método `cleanupPhysicsComponents()`**: Limpia todos los componentes de física de una escena
- **Método `reinitializePhysicsComponents()`**: Reinicializa los componentes de física de la escena activa
- **Integración automática**: Se ejecuta automáticamente al cambiar de escena

### 4. PhysicsManager.cpp
- **Método `cleanupScenePhysicsComponents()`**: Limpia todos los objetos de física de la escena
- **Manejo de eventos**: Limpia el event handler para prevenir callbacks durante la limpieza

## Flujo de Limpieza

### Al Cambiar de Escena:

1. **Limpieza de la escena anterior**:
   - Se llama a `cleanupPhysicsComponents(activeScene)`
   - Se ejecuta `PhysicsManager::cleanupScenePhysicsComponents()`
   - Se limpian todos los componentes de física individualmente

2. **Cambio de escena**:
   - Se establece la nueva escena activa
   - Se inicializa si es necesario

3. **Reinicialización de la nueva escena**:
   - Se llama a `reinitializePhysicsComponents()`
   - Se reinicializan todos los componentes de física

## Beneficios

### 1. Prevención de Memory Leaks
- Los recursos de PhysX se liberan correctamente
- No hay actores huérfanos en la escena de física

### 2. Estabilidad entre Escenas
- No hay conflictos de colisiones entre escenas
- Los eventos de física se limpian correctamente

### 3. Rendimiento Mejorado
- La escena de física se mantiene limpia
- No hay objetos de física innecesarios

### 4. Debugging Mejorado
- Logging detallado de todas las operaciones de limpieza
- Fácil identificación de problemas de limpieza

## Uso

### Automático
La limpieza se ejecuta automáticamente al cambiar de escena usando:
```cpp
SceneManager::getInstance().setActiveScene("NuevaEscena");
```

### Manual
Si necesitas limpiar manualmente los componentes de física:
```cpp
// Limpiar componentes de una escena específica
SceneManager::getInstance().cleanupPhysicsComponents(scene);

// Reinicializar componentes de la escena activa
SceneManager::getInstance().reinitializePhysicsComponents();
```

## Consideraciones Técnicas

### 1. Orden de Limpieza
1. Se limpia el event handler primero
2. Se remueven todos los actores de la escena
3. Se liberan los recursos de PhysX
4. Se resetean los estados de los componentes

### 2. Seguridad
- Se verifica que los objetos existan antes de limpiarlos
- Se manejan excepciones durante la limpieza
- Se resetean todas las referencias a nullptr

### 3. Compatibilidad
- Compatible con el sistema existente de gestión de escenas
- No interfiere con la funcionalidad existente
- Mantiene la API pública sin cambios

## Logs de Ejemplo

```
SceneManager: Switching from scene: MenuScene to: GameScene
SceneManager: Cleaning up physics components for scene: MenuScene
PhysicsManager: Cleaning up scene physics components...
PhysicsManager: Removing all actors from scene...
PhysicsManager: Scene physics components cleanup completed.
SceneManager: Cleaning up Collider for GameObject: MenuButton
SceneManager: Cleaning up Rigidbody for GameObject: MenuButton
SceneManager: Physics components cleanup completed for scene: MenuScene
SceneManager: Reinitializing physics components for scene: GameScene
SceneManager: Reinitializing Collider for GameObject: Player
SceneManager: Reinitializing Rigidbody for GameObject: Player
SceneManager: Physics components reinitialization completed for scene: GameScene
Scene switched successfully. Objects in scene: 5
```

## Troubleshooting

### Problema: Componentes no se limpian correctamente
**Solución**: Verificar que el GameObject tenga los componentes correctos y que el método `destroy()` esté implementado

### Problema: Errores de PhysX durante la limpieza
**Solución**: Asegurar que la escena de PhysX esté activa antes de la limpieza

### Problema: Componentes no se reinicializan
**Solución**: Verificar que el método `start()` esté implementado en los componentes

## Futuras Mejoras

1. **Limpieza asíncrona**: Implementar limpieza en background para escenas grandes
2. **Pooling de componentes**: Reutilizar componentes de física para mejor rendimiento
3. **Limpieza selectiva**: Permitir limpiar solo ciertos tipos de componentes
4. **Métricas de limpieza**: Agregar estadísticas sobre el tiempo de limpieza y recursos liberados

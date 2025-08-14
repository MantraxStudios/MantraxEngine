# Recarga de Escena en Gizmos

## Descripción

El sistema de Gizmos incluye funcionalidad para recargar automáticamente la escena cuando se presiona el botón "Stop" durante el modo de juego. Esto asegura que la escena vuelva a su estado original después de la ejecución. También incluye un botón de recarga manual para debugging.

## Funcionalidad Implementada

### 1. Botón Stop con Recarga Automática

Cuando se presiona el botón "Stop" en la ventana de Gizmos:

1. **Verifica la ruta de escena**: Comprueba que `EditorInfo::currentScenePath` esté disponible
2. **Detiene el modo de juego**: `EditorInfo::IsPlaying = false`
3. **Recarga la escena**: Usa `SceneSaver::LoadScene()` con la ruta actual
4. **Limpieza automática**: Se ejecuta todo el sistema de limpieza implementado
5. **Logging detallado**: Registra todo el proceso para debugging

### 2. Botón de Recarga Manual

Nuevo botón "Reload Scene" que permite recargar la escena manualmente:
- Útil para debugging y desarrollo
- No requiere estar en modo de juego
- Incluye las mismas verificaciones de seguridad

### 3. Información de Estado

La ventana de Gizmos ahora muestra:
- **Ruta de escena actual**: Muestra la ruta del archivo de escena cargado
- **Estado del juego**: Indica si está en modo "Playing" o "Editing"

## Flujo de Recarga

### Al Presionar Stop:

```
Botón Stop presionado →
  ↓
1. Verificar ruta de escena disponible
2. Detener modo de juego
3. Ejecutar SceneSaver::LoadScene()
   - Limpiar escena anterior automáticamente
   - Cargar escena desde archivo
   - Reinicializar todos los componentes
4. Confirmar éxito/fallo
5. Mostrar mensajes de estado
```

### Al Presionar Reload Manual:

```
Botón Reload Scene presionado →
  ↓
1. Verificar ruta de escena disponible
2. Ejecutar SceneSaver::LoadScene()
3. Confirmar éxito/fallo
4. Mostrar mensajes de estado
```

## Código Implementado

### Gizmos.cpp - Botón Stop

```cpp
if (ImGui::Button("Stop")) {
    std::cout << "Gizmos: Stop button pressed - Reloading scene..." << std::endl;
    
    // Verificar que tenemos una ruta de escena válida
    if (EditorInfo::currentScenePath.empty()) {
        std::cerr << "Gizmos: ERROR - No current scene path available for reload!" << std::endl;
        std::cerr << "Gizmos: Please save the scene first or load a scene file" << std::endl;
        return;
    }
    
    std::cout << "Gizmos: Current scene path: " << EditorInfo::currentScenePath << std::endl;
    
    // Detener el modo de juego PRIMERO
    EditorInfo::IsPlaying = false;
    std::cout << "Gizmos: Game mode stopped" << std::endl;
    
    // Recargar la escena usando SceneSaver (que ya incluye limpieza automática)
    std::cout << "Gizmos: Starting scene reload..." << std::endl;
    bool success = SceneSaver::LoadScene(EditorInfo::currentScenePath);
    
    if (success) {
        std::cout << "Gizmos: Scene reloaded successfully!" << std::endl;
        std::cout << "Gizmos: Scene is now ready for editing" << std::endl;
    } else {
        std::cerr << "Gizmos: ERROR - Failed to reload scene!" << std::endl;
        std::cerr << "Gizmos: The scene may be in an inconsistent state" << std::endl;
    }
}
```

### Gizmos.cpp - Botón de Recarga Manual

```cpp
if (ImGui::Button("Reload Scene")) {
    std::cout << "Gizmos: Manual reload button pressed..." << std::endl;
    
    if (EditorInfo::currentScenePath.empty()) {
        std::cerr << "Gizmos: ERROR - No current scene path available for reload!" << std::endl;
        std::cerr << "Gizmos: Please save the scene first or load a scene file" << std::endl;
        return;
    }
    
    std::cout << "Gizmos: Manually reloading scene from: " << EditorInfo::currentScenePath << std::endl;
    bool success = SceneSaver::LoadScene(EditorInfo::currentScenePath);
    
    if (success) {
        std::cout << "Gizmos: Manual scene reload successful!" << std::endl;
    } else {
        std::cerr << "Gizmos: ERROR - Manual scene reload failed!" << std::endl;
    }
}
```

### Gizmos.cpp - Información de Estado

```cpp
// Mostrar información de la escena actual
ImGui::Separator();
ImGui::Text("Current Scene: %s", EditorInfo::currentScenePath.empty() ? "None" : EditorInfo::currentScenePath.c_str());
ImGui::Text("Game Mode: %s", EditorInfo::IsPlaying ? "Playing" : "Editing");
```

## Beneficios

### 1. Restauración Automática del Estado
- La escena vuelve exactamente a como estaba antes de ejecutar
- No hay residuos de la ejecución del juego
- Todos los objetos se restauran a sus posiciones originales

### 2. Integración con Sistema de Limpieza
- Usa el sistema de limpieza automática implementado
- Limpia componentes de física correctamente
- Resetea el TileEditor y otros sistemas

### 3. Logging y Debugging Mejorado
- Registra todo el proceso de recarga
- Identifica errores si la recarga falla
- Facilita el debugging de problemas
- Muestra información de estado en tiempo real

### 4. Funcionalidad de Recarga Manual
- Permite recargar la escena sin estar en modo de juego
- Útil para debugging y desarrollo
- Mantiene las mismas verificaciones de seguridad

## Casos de Uso

### 1. Desarrollo y Testing
```cpp
// Durante el desarrollo, puedes:
// 1. Hacer cambios en la escena
// 2. Presionar Play para probar
// 3. Presionar Stop para volver al estado original
// 4. Hacer más cambios y repetir
```

### 2. Debugging de Juego
```cpp
// Si algo sale mal durante la ejecución:
// 1. Presionar Stop
// 2. La escena se recarga automáticamente
// 3. Puedes investigar el problema
// 4. Hacer correcciones
```

### 3. Iteración Rápida
```cpp
// Para iteración rápida en el desarrollo:
// 1. Modificar escena
// 2. Play → Test → Stop → Reload
// 3. Repetir hasta que funcione correctamente
```

### 4. Recarga Manual para Debugging
```cpp
// Cuando necesites recargar sin estar en modo de juego:
// 1. Presionar "Reload Scene"
// 2. La escena se recarga manualmente
// 3. Útil para probar cambios de configuración
```

## Logs de Ejemplo

### Recarga Exitosa con Stop
```
Gizmos: Stop button pressed - Reloading scene...
Gizmos: Current scene path: C:/Projects/MyGame/scenes/level1.json
Gizmos: Game mode stopped
Gizmos: Starting scene reload...
SceneSaver: Cleaning up previous scene: Level1
SceneManager: Cleaning up physics components for scene: Level1
PhysicsManager: Cleaning up scene physics components...
SceneManager: Physics components cleanup completed for scene: Level1
SceneSaver: Clearing TileEditor data from previous scene
TileEditor: Clearing all tiles and resetting editor state
SceneSaver: Previous scene cleaned up successfully
Scene loaded successfully. Total objects: 25
Gizmos: Scene reloaded successfully!
Gizmos: Scene is now ready for editing
```

### Recarga Manual Exitosa
```
Gizmos: Manual reload button pressed...
Gizmos: Manually reloading scene from: C:/Projects/MyGame/scenes/level1.json
SceneSaver: Cleaning up previous scene: Level1
SceneManager: Cleaning up physics components for scene: Level1
Scene loaded successfully. Total objects: 25
Gizmos: Manual scene reload successful!
```

### Error - Sin Ruta de Escena
```
Gizmos: Stop button pressed - Reloading scene...
Gizmos: ERROR - No current scene path available for reload!
Gizmos: Please save the scene first or load a scene file
```

## Consideraciones Técnicas

### 1. Dependencias
- **SceneSaver**: Maneja la carga y limpieza de escenas
- **SceneManager**: Gestiona las escenas activas
- **EditorInfo**: Mantiene el estado del editor y la ruta de escena

### 2. Manejo de Errores
- Verifica que la ruta de escena esté disponible antes de intentar recargar
- Maneja fallos en la carga de escena
- Registra errores para debugging
- Previene operaciones en estado inconsistente

### 3. Estado del Editor
- Mantiene sincronizado `EditorInfo::IsPlaying`
- Preserva la ruta de escena actual
- No interfiere con otras funcionalidades del editor
- Muestra información de estado en tiempo real

### 4. Orden de Operaciones
- Verifica la ruta de escena PRIMERO
- Detiene el modo de juego ANTES de recargar
- Ejecuta la recarga con todas las verificaciones
- Confirma el resultado al usuario

## Troubleshooting

### Problema: La escena no se recarga
**Solución**: Verificar que `EditorInfo::currentScenePath` contenga una ruta válida

### Problema: Error al recargar
**Solución**: Verificar que el archivo de escena existe y es válido

### Problema: Componentes no se restauran
**Solución**: Verificar que `SceneSaver::LoadScene()` funcione correctamente

### Problema: Estado del editor inconsistente
**Solución**: Verificar que `EditorInfo::IsPlaying` se actualice correctamente

### Problema: No hay ruta de escena disponible
**Solución**: Guardar la escena primero o cargar un archivo de escena existente

## Futuras Mejoras

1. **Recarga selectiva**: Permitir recargar solo ciertos objetos
2. **Historial de cambios**: Mantener un historial de modificaciones
3. **Recarga automática**: Recargar automáticamente al detectar cambios en archivos
4. **Backup automático**: Crear backups antes de ejecutar el juego
5. **Recarga en background**: Implementar recarga asíncrona para escenas grandes
6. **Hot reload**: Recargar solo componentes modificados sin recargar toda la escena

## Integración con Otros Sistemas

### SceneSaver
- Usa el sistema de limpieza automática implementado
- Maneja la carga de todos los componentes
- Preserva la configuración de la escena

### SceneManager
- Gestiona el cambio de escenas
- Mantiene la escena activa actualizada
- Coordina la limpieza de componentes

### EditorInfo
- Mantiene el estado del editor
- Preserva la ruta de escena actual
- Coordina el modo de juego

### Sistema de Logging
- Registra todas las operaciones de recarga
- Facilita el debugging y troubleshooting
- Mantiene un historial de operaciones

# Canvas Buffer Integration Guide

## Descripción

Esta funcionalidad permite que el UI del canvas use el width y height del buffer de la cámara en lugar del tamaño de la ventana. Esto es especialmente útil para:

- Renderizado a textura
- Efectos de post-procesamiento
- UI que debe coincidir con el tamaño del render target
- Aplicaciones que requieren resolución fija independiente del tamaño de ventana

## Cambios Implementados

### 1. Nuevos métodos en Camera.h

Se agregaron métodos para obtener el tamaño del buffer de la cámara:

```cpp
// Buffer size getters
int getBufferWidth() const { return framebufferEnabled ? framebufferWidth : 0; }
int getBufferHeight() const { return framebufferEnabled ? framebufferHeight : 0; }
std::pair<int, int> getBufferSize() const { return { getBufferWidth(), getBufferHeight() }; }
```

### 2. Modificaciones en RenderPipeline

- **updateCanvasSize()**: Ahora verifica si la cámara tiene un framebuffer habilitado y usa su tamaño
- **updateCanvasFromCameraBuffer()**: Nuevo método que actualiza el canvas específicamente con el tamaño del buffer de la cámara
- **renderFrame()**: Ahora llama automáticamente a `updateCanvasFromCameraBuffer()` antes de renderizar la UI

### 3. Comportamiento Automático

El canvas se actualiza automáticamente en cada frame para usar:

- El tamaño del buffer de la cámara si está habilitado
- El tamaño de la ventana como fallback si no hay buffer de cámara

## Uso

### Desde C++

```cpp
// Obtener la cámara
Camera* camera = scene->getCamera();

// Habilitar framebuffer con tamaño personalizado
camera->enableFramebuffer(true);
camera->setFramebufferSize(1024, 768);

// El canvas se actualizará automáticamente en el próximo frame
```

### Desde Lua

```lua
-- Obtener la cámara activa
local camera = getActiveCamera()

-- Habilitar framebuffer
camera:enableFramebuffer(true)
camera:setFramebufferSize(1024, 768)

-- Verificar estado
local isEnabled = camera:isFramebufferEnabled()
print("Framebuffer habilitado: " .. tostring(isEnabled))
```

## Ejemplo Completo

Ver el archivo `src/CanvasBufferTest.lua` para un ejemplo completo de uso.

## Ventajas

1. **Flexibilidad**: El UI puede usar un tamaño diferente al de la ventana
2. **Consistencia**: El canvas siempre coincide con el render target activo
3. **Automatización**: No requiere actualización manual del canvas
4. **Fallback**: Funciona correctamente incluso sin framebuffer de cámara

## Consideraciones

- El canvas se actualiza automáticamente en cada frame
- Si no hay framebuffer de cámara, usa el tamaño de la ventana
- Los cambios en el tamaño del buffer se reflejan inmediatamente en el canvas
- El sistema es compatible con el código existente

## Casos de Uso

1. **Renderizado a textura**: UI que debe coincidir con el tamaño de la textura renderizada
2. **Efectos de post-procesamiento**: UI que se renderiza en el mismo buffer que los efectos
3. **Resolución fija**: Aplicaciones que requieren una resolución específica independiente del tamaño de ventana
4. **Debugging**: UI de debug que debe mostrar información específica del render target

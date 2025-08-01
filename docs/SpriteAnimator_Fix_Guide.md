# Guía para Solucionar el Problema del SpriteAnimator

## Problema

El SpriteAnimator funciona correctamente al principio pero luego se queda en negro.

## Causas Identificadas

### 1. **Manejo de Texturas Inválidas**

- Las texturas pueden tener IDs inválidos (0) después de ciertas operaciones
- No se verifica la validez de las texturas antes de usarlas
- Las texturas pueden corromperse durante la animación

### 2. **Problemas de Material**

- El material puede no estar correctamente asignado al GameObject
- Las propiedades del material pueden no actualizarse correctamente
- Problemas en la sincronización entre el material del SpriteAnimator y el GameObject

### 3. **Problemas de Estado**

- Los estados pueden no estar correctamente validados
- Cambios de estado sin verificación de integridad
- Estados sin texturas válidas

## Soluciones Implementadas

### 1. **Mejoras en el Manejo de Texturas**

```cpp
// Verificación de texturas válidas antes de usarlas
if (state.texture[currentFrame].getID() != 0) {
    auto currentTexture = std::make_shared<Texture>(state.texture[currentFrame]);
    if (spriteMaterial && currentTexture && currentTexture->getID() != 0) {
        spriteMaterial->setAlbedoTexture(currentTexture);
        // Log de éxito
    } else {
        // Log de error
    }
}
```

### 2. **Validación de Material**

```cpp
// Verificar que el GameObject tenga el material correcto
if (getOwner() && getOwner()->getMaterial() != this->getMaterial()) {
    getOwner()->setMaterial(this->getMaterial());
}
```

### 3. **Sistema de Debugging**

```cpp
// Habilitar modo debug
spriteAnimator:enableDebugMode(true)

// Obtener información de debug
spriteAnimator:printDebugInfo()

// Verificar integridad
spriteAnimator:isMaterialValid()
spriteAnimator:isValidState("StateName")
spriteAnimator:hasValidTextures("StateName")
```

### 4. **Forzar Actualizaciones**

```cpp
// Forzar actualización del material
spriteAnimator:forceUpdate()
```

## Scripts de Prueba

### 1. **SpriteAnimatorDebugTest.lua**

Script básico para diagnosticar problemas.

### 2. **SpriteAnimatorFinalTest.lua**

Script completo con todas las validaciones y debugging.

## Cómo Usar

### Paso 1: Ejecutar el Script de Debug

```lua
-- Cargar el script de debug
dofile("src/components/SpriteAnimatorFinalTest.lua")
```

### Paso 2: Revisar la Consola

El script mostrará información detallada sobre:

- Estado del material
- Validación de estados
- Verificación de texturas
- Información de debugging

### Paso 3: Interpretar los Resultados

**Si el sprite se ve rojo y brillante:**

- El problema está en las texturas
- Las texturas no se están cargando correctamente
- Solución: Verificar rutas de texturas y formato de archivos

**Si el sprite se ve negro:**

- El problema está en el material o renderizado
- Posibles causas:
  - Configuración de iluminación
  - Configuración de shaders
  - Posición de la cámara
  - Configuración de renderizado

**Si el sprite no se ve:**

- El problema está en la configuración del GameObject
- Posibles causas:
  - GameObject no tiene material asignado
  - GameObject no está siendo renderizado
  - Problemas de transformación

## Nuevas Funcionalidades

### Métodos de Debugging

- `enableDebugMode(bool)`: Habilitar/deshabilitar modo debug
- `isDebugModeEnabled()`: Verificar si el modo debug está activo
- `forceUpdate()`: Forzar actualización del material
- `printDebugInfo()`: Mostrar información de debug en consola

### Métodos de Validación

- `isValidState(string)`: Verificar si un estado es válido
- `hasValidTextures(string)`: Verificar si un estado tiene texturas válidas
- `isMaterialValid()`: Verificar si el material es válido

### Métodos de Información

- `getDebugInfo()`: Obtener información de debug como string
- `printDebugInfo()`: Imprimir información de debug en consola

## Configuración Recomendada

### Para Sprites Básicos

```lua
spriteAnimator:createMaterial("BasicSpriteMaterial")
spriteAnimator:setSpriteAlbedo(vec3(1.0, 1.0, 1.0)) -- Blanco
spriteAnimator:setSpriteMetallic(0.0) -- No metálico
spriteAnimator:setSpriteRoughness(0.5) -- Semi-rugoso
spriteAnimator:setSpriteEmissive(vec3(0.1, 0.1, 0.1)) -- Ligera emisión
```

### Para Sprites con Emisión

```lua
spriteAnimator:setSpriteEmissive(vec3(0.3, 0.0, 0.0)) -- Emisión roja fuerte
```

### Para Debugging

```lua
spriteAnimator:enableDebugMode(true)
spriteAnimator:printDebugInfo()
```

## Solución de Problemas Comunes

### Problema: Sprite se ve negro después de funcionar

**Solución:**

1. Habilitar modo debug
2. Verificar integridad del material
3. Forzar actualización del material
4. Verificar texturas válidas

### Problema: Animación no funciona

**Solución:**

1. Verificar estados válidos
2. Verificar texturas en cada estado
3. Verificar configuración de animación

### Problema: Material no se actualiza

**Solución:**

1. Usar `forceUpdate()`
2. Verificar sincronización con GameObject
3. Verificar propiedades del material

## Notas Importantes

1. **Siempre verificar la integridad** antes de usar el SpriteAnimator
2. **Usar el modo debug** para diagnosticar problemas
3. **Forzar actualizaciones** cuando sea necesario
4. **Verificar texturas** antes de usarlas en animaciones
5. **Monitorear la consola** para mensajes de error y advertencias

## Archivos Modificados

1. `src/components/SpriteAnimator.h` - Agregados métodos de debugging
2. `src/components/SpriteAnimator.cpp` - Mejorado manejo de errores
3. `src/wrapper/CoreWrapper.cpp` - Registrados nuevos métodos en Lua
4. `src/components/SpriteAnimatorFinalTest.lua` - Script de prueba completo
5. `src/components/SpriteAnimatorDebugTest.lua` - Script de debug básico

## Próximos Pasos

1. Ejecutar el script de prueba final
2. Revisar la consola para identificar problemas específicos
3. Aplicar las correcciones necesarias según el diagnóstico
4. Verificar que el SpriteAnimator funcione correctamente
5. Deshabilitar el modo debug en producción

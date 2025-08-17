# SpriteAnimator Inspector Guide

## 📋 **Descripción General**

El `SpriteAnimator` ahora tiene su propio material que se puede editar completamente desde el inspector. El material se actualiza automáticamente con las texturas de animación y permite configurar todas las propiedades PBR. **✅ Ahora incluye serialización completa para guardar/cargar configuraciones.**

> **✅ Actualización Importante**: El `SpriteAnimator` se puede agregar a GameObjects desde Lua usando `gameObject:addComponent("SpriteAnimator")` y ahora soporta serialización completa.

## 🎮 **Funciones Disponibles en el Inspector**

### **Gestión de Material**

```lua
-- Crear un nuevo material
spriteAnimator:createMaterial("NombreDelMaterial")

-- Obtener el material actual
local material = spriteAnimator:getMaterial()

-- Asignar un material externo
spriteAnimator:setMaterial(miMaterial)
```

### **Configuración de Propiedades del Material**

```lua
-- Color base (albedo)
spriteAnimator:setSpriteAlbedo(vec3(1.0, 1.0, 1.0)) -- Blanco

-- Propiedades metálicas
spriteAnimator:setSpriteMetallic(0.0) -- 0.0 = no metálico, 1.0 = muy metálico

-- Rugosidad de la superficie
spriteAnimator:setSpriteRoughness(0.3) -- 0.0 = espejo, 1.0 = muy rugoso

-- Emisión (brillo propio)
spriteAnimator:setSpriteEmissive(vec3(0.1, 0.1, 0.1)) -- Ligera emisión

-- Repetición de textura
spriteAnimator:setSpriteTiling(vec2(1.0, 1.0)) -- Sin repetición
```

### **Control de Animación**

```lua
-- Cambiar estado de animación
spriteAnimator.currentState = "Walk"

-- Actualizar manualmente la textura del material
spriteAnimator:updateMaterialTexture()

-- Obtener información del estado actual
print("Estado: " .. spriteAnimator.currentState)
```

### **🆕 Serialización y Persistencia**

```lua
-- Guardar configuración completa
local configuracion = spriteAnimator:serializeComponent()

-- Cargar configuración guardada
spriteAnimator:deserialize(configuracion)

-- Funciones de utilidad para el inspector
guardarConfiguracion() -- Guarda y muestra la configuración
cargarConfiguracion(configuracion) -- Carga una configuración
```

## 🎨 **Presets de Material Predefinidos**

### **Preset Brillante**

```lua
configurarPresetMaterial("brillante")
-- Albedo: Blanco puro
-- Metallic: 0.8 (muy metálico)
-- Roughness: 0.1 (muy suave)
-- Emissive: Ligera emisión
```

### **Preset Mate**

```lua
configurarPresetMaterial("mate")
-- Albedo: Gris medio
-- Metallic: 0.0 (no metálico)
-- Roughness: 0.9 (muy rugoso)
-- Emissive: Sin emisión
```

### **Preset Emissivo**

```lua
configurarPresetMaterial("emissivo")
-- Albedo: Gris oscuro
-- Metallic: 0.0 (no metálico)
-- Roughness: 0.5 (medio)
-- Emissive: Brillo azul fuerte
```

### **🆕 Preset Metálico**

```lua
configurarPresetMaterial("metalico")
-- Albedo: Gris claro
-- Metallic: 1.0 (muy metálico)
-- Roughness: 0.2 (suave)
-- Emissive: Sin emisión
```

## 🆕 **Configuración por Tipo de Sprite**

```lua
-- Configurar para personajes
configurarMaterialPorTipo("personaje")

-- Configurar para UI
configurarMaterialPorTipo("ui")

-- Configurar para efectos
configurarMaterialPorTipo("efecto")
```

## 🔧 **Ejemplo Completo de Uso**

```lua
-- 1. Crear GameObject con SpriteAnimator
local spriteObject = GameObject.create("MiSprite")
local spriteAnimator = spriteObject:addComponent("SpriteAnimator")

-- 2. Configurar material
spriteAnimator:createMaterial("SpriteMaterial")
spriteAnimator:setSpriteAlbedo(vec3(1.0, 1.0, 1.0))
spriteAnimator:setSpriteMetallic(0.0)
spriteAnimator:setSpriteRoughness(0.3)
spriteAnimator:setSpriteEmissive(vec3(0.1, 0.1, 0.1))

-- 3. Configurar estados de animación
local idleState = {
    state_name = "Idle",
    texture = {}
}
-- Agregar texturas aquí

local walkState = {
    state_name = "Walk",
    texture = {}
}
-- Agregar texturas aquí

spriteAnimator.SpriteStates:push_back(idleState)
spriteAnimator.SpriteStates:push_back(walkState)

-- 4. Establecer estado inicial
spriteAnimator.currentState = "Idle"

-- 5. Inicializar
spriteAnimator:start()

-- 6. 🆕 Guardar configuración
local configuracion = spriteAnimator:serializeComponent()
```

## 🆕 **Funciones Avanzadas del Inspector**

### **Creación Rápida de Sprites**

```lua
-- Crear sprite con configuración básica
local sprite = crearSpriteRapido("MiPersonaje", "Idle")
```

### **Gestión de Estados**

```lua
-- Obtener todos los estados disponibles
local estados = obtenerEstadosDisponibles()

-- Verificar si un estado existe
if estadoExiste("Walk") then
    cambiarEstado("Walk")
end
```

### **Información Completa**

```lua
-- Obtener información detallada del componente
obtenerInfoCompleta()
```

## 🎯 **Casos de Uso Comunes**

### **Sprite con Efecto Brillante**

```lua
spriteAnimator:setSpriteAlbedo(vec3(1.0, 1.0, 1.0))
spriteAnimator:setSpriteMetallic(0.8)
spriteAnimator:setSpriteRoughness(0.1)
spriteAnimator:setSpriteEmissive(vec3(0.2, 0.2, 0.2))
```

### **Sprite con Efecto Emissivo**

```lua
spriteAnimator:setSpriteAlbedo(vec3(0.5, 0.5, 0.5))
spriteAnimator:setSpriteMetallic(0.0)
spriteAnimator:setSpriteRoughness(0.5)
spriteAnimator:setSpriteEmissive(vec3(0.8, 0.8, 1.0))
```

### **Sprite Mate para UI**

```lua
spriteAnimator:setSpriteAlbedo(vec3(0.8, 0.8, 0.8))
spriteAnimator:setSpriteMetallic(0.0)
spriteAnimator:setSpriteRoughness(0.9)
spriteAnimator:setSpriteEmissive(vec3(0.0, 0.0, 0.0))
```

### **🆕 Sprite Metálico para Efectos**

```lua
spriteAnimator:setSpriteAlbedo(vec3(0.8, 0.8, 0.8))
spriteAnimator:setSpriteMetallic(1.0)
spriteAnimator:setSpriteRoughness(0.2)
spriteAnimator:setSpriteEmissive(vec3(0.0, 0.0, 0.0))
```

## 🔄 **Actualización Automática**

El material se actualiza automáticamente cuando:

- Cambias el `currentState`
- Se llama al método `update()`
- Se llama manualmente a `updateMaterialTexture()`
- Se deserializa una configuración guardada

## 📊 **Información del Material**

```lua
-- Obtener información completa del material
function obtenerInfoMaterial()
    local material = spriteAnimator:getMaterial()
    if material then
        print("Material: " .. material:getName())
        print("Albedo: " .. tostring(material:getAlbedo()))
        print("Metallic: " .. material:getMetallic())
        print("Roughness: " .. material:getRoughness())
        print("Emissive: " .. tostring(material:getEmissive()))
        print("Tiling: " .. tostring(material:getTiling()))
    end
end
```

## ⚡ **Funciones Rápidas**

```lua
-- Cambiar estado
cambiarEstado("Walk")

-- Configurar material básico
configurarMaterial()

-- Crear material personalizado
crearMaterialPersonalizado()

-- Cambiar entre animaciones
cambiarAnimacion()

-- Obtener información del estado
obtenerInfoEstado()

-- Aplicar preset
configurarPresetMaterial("brillante")

-- 🆕 Funciones avanzadas
guardarConfiguracion()
crearSpriteRapido("MiSprite", "Idle")
configurarMaterialPorTipo("personaje")
obtenerInfoCompleta()
```

## 🎨 **Consejos de Uso**

1. **Para sprites 2D**: Usa `metallic = 0.0` y `roughness = 0.3`
2. **Para efectos brillantes**: Aumenta `metallic` y reduce `roughness`
3. **Para efectos emissivos**: Usa `emissive` para dar brillo propio
4. **Para UI**: Usa `roughness = 0.9` para un look mate
5. **Para animaciones**: El material se actualiza automáticamente con cada frame
6. **🆕 Para persistencia**: Usa `serializeComponent()` para guardar configuraciones
7. **🆕 Para configuración rápida**: Usa `configurarMaterialPorTipo()` para presets específicos

## 🚀 **Ventajas del Sistema**

- ✅ **Material propio**: Cada SpriteAnimator tiene su material independiente
- ✅ **Actualización automática**: Se actualiza con las animaciones
- ✅ **Configuración completa**: Todas las propiedades PBR disponibles
- ✅ **Presets predefinidos**: Configuraciones rápidas para casos comunes
- ✅ **Integración con inspector**: Fácil edición desde la interfaz
- ✅ **Optimizado para sprites**: Configuración por defecto optimizada
- ✅ **🆕 Serialización completa**: Guardar/cargar configuraciones
- ✅ **🆕 Funciones avanzadas**: Creación rápida y gestión de estados
- ✅ **🆕 Configuración por tipo**: Presets específicos para diferentes usos

## 🆕 **Nuevas Funcionalidades**

### **Serialización**

- Guardar configuraciones completas
- Cargar configuraciones guardadas
- Persistencia entre sesiones

### **Creación Rápida**

- Crear sprites con configuración básica
- Configuración automática por tipo
- Presets optimizados

### **Gestión Avanzada**

- Verificación de estados
- Información completa del componente
- Funciones de utilidad

¡Tu SpriteAnimator ahora tiene un material completamente editable desde el inspector con serialización completa! 🎉

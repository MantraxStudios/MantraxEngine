-- ===== SPRITE ANIMATOR INSPECTOR EXAMPLE =====
-- Este script muestra cómo usar el SpriteAnimator desde el inspector

-- Crear un GameObject con SpriteAnimator
local spriteObject = GameObject.create("MiSprite")
local spriteAnimator = spriteObject:addComponent("SpriteAnimator")

-- ===== CONFIGURACIÓN BÁSICA DEL MATERIAL =====
spriteAnimator:createMaterial("SpriteMaterialPersonalizado")

-- Configurar propiedades del material desde el inspector
spriteAnimator:setSpriteAlbedo(vec3(1.0, 1.0, 1.0)) -- Color blanco
spriteAnimator:setSpriteMetallic(0.0) -- No metálico
spriteAnimator:setSpriteRoughness(0.3) -- Poco rugoso para sprites
spriteAnimator:setSpriteEmissive(vec3(0.1, 0.1, 0.1)) -- Ligera emisión
spriteAnimator:setSpriteTiling(vec2(1.0, 1.0)) -- Sin tiling

-- ===== CONFIGURACIÓN DE ESTADOS DE ANIMACIÓN =====
-- Crear estados de animación
local idleState = {
    state_name = "Idle",
    texture = {}
}
-- Aquí agregarías las texturas para el estado idle
-- idleState.texture:push_back(Texture("path/to/idle1.png"))
-- idleState.texture:push_back(Texture("path/to/idle2.png"))

local walkState = {
    state_name = "Walk", 
    texture = {}
}
-- Aquí agregarías las texturas para el estado walk
-- walkState.texture:push_back(Texture("path/to/walk1.png"))
-- walkState.texture:push_back(Texture("path/to/walk2.png"))

local runState = {
    state_name = "Run",
    texture = {}
}
-- walkState.texture:push_back(Texture("path/to/run1.png"))
-- walkState.texture:push_back(Texture("path/to/run2.png"))

-- Agregar estados al SpriteAnimator
spriteAnimator.SpriteStates:push_back(idleState)
spriteAnimator.SpriteStates:push_back(walkState)
spriteAnimator.SpriteStates:push_back(runState)

-- Establecer estado inicial
spriteAnimator.currentState = "Idle"

-- Inicializar el componente
spriteAnimator:start()

-- ===== FUNCIONES PARA EL INSPECTOR =====

-- Función para cambiar de estado (puede ser llamada desde el inspector)
function cambiarEstado(nuevoEstado)
    spriteAnimator.currentState = nuevoEstado
    print("Estado cambiado a: " .. nuevoEstado)
end

-- Función para configurar el material desde el inspector
function configurarMaterial()
    -- Configurar albedo (color base)
    spriteAnimator:setSpriteAlbedo(vec3(0.8, 0.8, 1.0)) -- Azul claro
    
    -- Configurar propiedades físicas
    spriteAnimator:setSpriteMetallic(0.1) -- Ligero brillo metálico
    spriteAnimator:setSpriteRoughness(0.2) -- Muy suave
    
    -- Configurar emisión (brillo propio)
    spriteAnimator:setSpriteEmissive(vec3(0.2, 0.2, 0.3)) -- Brillo azul
    
    -- Configurar tiling (repetición de textura)
    spriteAnimator:setSpriteTiling(vec2(1.0, 1.0)) -- Sin repetición
    
    print("Material configurado exitosamente")
end

-- Función para obtener información del material
function obtenerInfoMaterial()
    local material = spriteAnimator:getMaterial()
    if material then
        print("Material activo: " .. material:getName())
        print("Albedo: " .. tostring(material:getAlbedo()))
        print("Metallic: " .. material:getMetallic())
        print("Roughness: " .. material:getRoughness())
    else
        print("No hay material asignado")
    end
end

-- Función para crear un material personalizado
function crearMaterialPersonalizado()
    -- Crear un nuevo material
    spriteAnimator:createMaterial("MaterialSpriteAnimado")
    
    -- Configurar para sprites animados
    spriteAnimator:setSpriteAlbedo(vec3(1.0, 1.0, 1.0)) -- Blanco puro
    spriteAnimator:setSpriteMetallic(0.0) -- Sin metálico
    spriteAnimator:setSpriteRoughness(0.1) -- Muy suave
    spriteAnimator:setSpriteEmissive(vec3(0.0, 0.0, 0.0)) -- Sin emisión
    spriteAnimator:setSpriteTiling(vec2(1.0, 1.0)) -- Sin tiling
    
    print("Material personalizado creado para sprites animados")
end

-- Función para actualizar manualmente la textura del material
function actualizarTextura()
    spriteAnimator:updateMaterialTexture()
    print("Textura del material actualizada")
end

-- Función para cambiar entre estados de animación
function cambiarAnimacion()
    if spriteAnimator.currentState == "Idle" then
        spriteAnimator.currentState = "Walk"
        print("Cambiando a estado Walk")
    elseif spriteAnimator.currentState == "Walk" then
        spriteAnimator.currentState = "Run"
        print("Cambiando a estado Run")
    else
        spriteAnimator.currentState = "Idle"
        print("Cambiando a estado Idle")
    end
end

-- Función para obtener información del estado actual
function obtenerInfoEstado()
    print("Estado actual: " .. spriteAnimator.currentState)
    print("Número de estados: " .. spriteAnimator.SpriteStates:size())
    
    for i = 1, spriteAnimator.SpriteStates:size() do
        local estado = spriteAnimator.SpriteStates[i]
        print("Estado " .. i .. ": " .. estado.state_name)
        print("  Texturas: " .. estado.texture:size())
    end
end

-- Función para configurar material con diferentes presets
function configurarPresetMaterial(preset)
    if preset == "brillante" then
        spriteAnimator:setSpriteAlbedo(vec3(1.0, 1.0, 1.0))
        spriteAnimator:setSpriteMetallic(0.8)
        spriteAnimator:setSpriteRoughness(0.1)
        spriteAnimator:setSpriteEmissive(vec3(0.3, 0.3, 0.3))
        print("Preset brillante aplicado")
        
    elseif preset == "mate" then
        spriteAnimator:setSpriteAlbedo(vec3(0.7, 0.7, 0.7))
        spriteAnimator:setSpriteMetallic(0.0)
        spriteAnimator:setSpriteRoughness(0.9)
        spriteAnimator:setSpriteEmissive(vec3(0.0, 0.0, 0.0))
        print("Preset mate aplicado")
        
    elseif preset == "emissivo" then
        spriteAnimator:setSpriteAlbedo(vec3(0.5, 0.5, 0.5))
        spriteAnimator:setSpriteMetallic(0.0)
        spriteAnimator:setSpriteRoughness(0.5)
        spriteAnimator:setSpriteEmissive(vec3(0.8, 0.8, 1.0))
        print("Preset emissivo aplicado")
        
    elseif preset == "metalico" then
        spriteAnimator:setSpriteAlbedo(vec3(0.8, 0.8, 0.8))
        spriteAnimator:setSpriteMetallic(1.0)
        spriteAnimator:setSpriteRoughness(0.2)
        spriteAnimator:setSpriteEmissive(vec3(0.0, 0.0, 0.0))
        print("Preset metálico aplicado")
        
    else
        print("Preset no reconocido. Usar: brillante, mate, emissivo, metalico")
    end
end

-- ===== FUNCIONES AVANZADAS PARA EL INSPECTOR =====

-- Función para serializar el componente (guardar configuración)
function guardarConfiguracion()
    local configuracion = spriteAnimator:serializeComponent()
    print("Configuración serializada:")
    print(configuracion)
    return configuracion
end

-- Función para cargar configuración
function cargarConfiguracion(configuracion)
    spriteAnimator:deserialize(configuracion)
    print("Configuración cargada exitosamente")
end

-- Función para crear un sprite con configuración rápida
function crearSpriteRapido(nombre, estadoInicial)
    local objeto = GameObject.create(nombre)
    local animator = objeto:addComponent("SpriteAnimator")
    
    -- Configuración básica
    animator:createMaterial(nombre .. "Material")
    animator:setSpriteAlbedo(vec3(1.0, 1.0, 1.0))
    animator:setSpriteMetallic(0.0)
    animator:setSpriteRoughness(0.3)
    animator:setSpriteEmissive(vec3(0.0, 0.0, 0.0))
    
    -- Crear estado básico
    local estado = {
        state_name = estadoInicial or "Default",
        texture = {}
    }
    animator.SpriteStates:push_back(estado)
    animator.currentState = estadoInicial or "Default"
    
    animator:start()
    print("Sprite creado: " .. nombre)
    return objeto
end

-- Función para configurar material para diferentes tipos de sprites
function configurarMaterialPorTipo(tipo)
    if tipo == "personaje" then
        spriteAnimator:setSpriteAlbedo(vec3(1.0, 1.0, 1.0))
        spriteAnimator:setSpriteMetallic(0.0)
        spriteAnimator:setSpriteRoughness(0.2)
        spriteAnimator:setSpriteEmissive(vec3(0.0, 0.0, 0.0))
        print("Material configurado para personaje")
        
    elseif tipo == "ui" then
        spriteAnimator:setSpriteAlbedo(vec3(0.9, 0.9, 0.9))
        spriteAnimator:setSpriteMetallic(0.0)
        spriteAnimator:setSpriteRoughness(0.9)
        spriteAnimator:setSpriteEmissive(vec3(0.0, 0.0, 0.0))
        print("Material configurado para UI")
        
    elseif tipo == "efecto" then
        spriteAnimator:setSpriteAlbedo(vec3(0.5, 0.5, 0.5))
        spriteAnimator:setSpriteMetallic(0.0)
        spriteAnimator:setSpriteRoughness(0.5)
        spriteAnimator:setSpriteEmissive(vec3(0.8, 0.8, 1.0))
        print("Material configurado para efecto")
        
    else
        print("Tipo no reconocido. Usar: personaje, ui, efecto")
    end
end

-- ===== FUNCIONES DE UTILIDAD =====

-- Función para obtener todos los estados disponibles
function obtenerEstadosDisponibles()
    local estados = {}
    for i = 1, spriteAnimator.SpriteStates:size() do
        local estado = spriteAnimator.SpriteStates[i]
        table.insert(estados, estado.state_name)
    end
    return estados
end

-- Función para verificar si un estado existe
function estadoExiste(nombreEstado)
    for i = 1, spriteAnimator.SpriteStates:size() do
        local estado = spriteAnimator.SpriteStates[i]
        if estado.state_name == nombreEstado then
            return true
        end
    end
    return false
end

-- Función para obtener información completa del componente
function obtenerInfoCompleta()
    print("=== INFORMACIÓN COMPLETA DEL SPRITE ANIMATOR ===")
    print("Estado actual: " .. spriteAnimator.currentState)
    print("Estados disponibles: " .. spriteAnimator.SpriteStates:size())
    
    local material = spriteAnimator:getMaterial()
    if material then
        print("Material: " .. material:getName())
        print("Albedo: " .. tostring(material:getAlbedo()))
        print("Metallic: " .. material:getMetallic())
        print("Roughness: " .. material:getRoughness())
        print("Emissive: " .. tostring(material:getEmissive()))
        print("Tiling: " .. tostring(material:getTiling()))
    else
        print("No hay material asignado")
    end
    
    print("Componente habilitado: " .. tostring(spriteAnimator:isActive()))
    print("================================================")
end

print("=== SPRITE ANIMATOR INSPECTOR EXAMPLE CARGADO ===")
print("Funciones básicas disponibles:")
print("- cambiarEstado(nuevoEstado)")
print("- configurarMaterial()")
print("- obtenerInfoMaterial()")
print("- crearMaterialPersonalizado()")
print("- actualizarTextura()")
print("- cambiarAnimacion()")
print("- obtenerInfoEstado()")
print("- configurarPresetMaterial(preset)")
print("")
print("Funciones avanzadas:")
print("- guardarConfiguracion()")
print("- cargarConfiguracion(configuracion)")
print("- crearSpriteRapido(nombre, estadoInicial)")
print("- configurarMaterialPorTipo(tipo)")
print("- obtenerEstadosDisponibles()")
print("- estadoExiste(nombreEstado)")
print("- obtenerInfoCompleta()")
print("")
print("Presets disponibles: brillante, mate, emissivo, metalico")
print("Tipos disponibles: personaje, ui, efecto") 
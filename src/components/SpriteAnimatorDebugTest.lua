-- ===== SPRITE ANIMATOR DEBUG TEST =====
-- Script para diagnosticar el problema del SpriteAnimator que se queda en negro

print("=== INICIANDO DIAGNÓSTICO DEL SPRITE ANIMATOR ===")

-- Crear GameObject de prueba
local testObject = GameObject.create("DebugSprite")
local spriteAnimator = testObject:addComponent("SpriteAnimator")

if not spriteAnimator then
    print("❌ ERROR: No se pudo crear el SpriteAnimator")
    return
end

print("✅ SpriteAnimator creado exitosamente")

-- Probar configuración básica del material
spriteAnimator:createMaterial("DebugMaterial")
spriteAnimator:setSpriteAlbedo(vec3(1.0, 0.0, 0.0)) -- Rojo brillante para detectar problemas
spriteAnimator:setSpriteMetallic(0.0)
spriteAnimator:setSpriteRoughness(0.5)
spriteAnimator:setSpriteEmissive(vec3(0.2, 0.0, 0.0)) -- Emisión roja para visibilidad
spriteAnimator:setSpriteTiling(vec2(1.0, 1.0))

print("✅ Material configurado con color rojo y emisión")

-- Crear estados de prueba con texturas simples
local idleState = {
    state_name = "Idle",
    texture = {}
}

local walkState = {
    state_name = "Walk", 
    texture = {}
}

-- Intentar cargar texturas de prueba
print("Intentando cargar texturas de prueba...")

-- Crear texturas de prueba simples (puedes ajustar las rutas según tu proyecto)
local texturePaths = {
    "Sprites/Characters/player_idle.png",
    "Sprites/Characters/player_walk.png", 
    "Sprites/Characters/player_run.png"
}

for i, path in ipairs(texturePaths) do
    print("Probando cargar textura: " .. path)
    -- Aquí podrías intentar cargar la textura si existe
end

spriteAnimator.SpriteStates:push_back(idleState)
spriteAnimator.SpriteStates:push_back(walkState)

spriteAnimator.currentState = "Idle"

print("✅ Estados de animación configurados")

-- Inicializar el componente
spriteAnimator:start()

print("✅ Componente inicializado")

-- Función para monitorear el estado del SpriteAnimator
local function monitorSpriteAnimator()
    print("=== MONITOREO DEL SPRITE ANIMATOR ===")
    print("Estado actual: " .. spriteAnimator.currentState)
    print("¿Está reproduciendo? " .. tostring(spriteAnimator:getIsPlaying()))
    print("Frame actual: " .. spriteAnimator:getCurrentFrame())
    print("Velocidad de animación: " .. spriteAnimator.animationSpeed)
    print("Estado de reproducción: " .. spriteAnimator.playbackState)
    
    -- Verificar material
    local material = spriteAnimator:getMaterial()
    if material then
        print("✅ Material disponible")
        print("  - Nombre: " .. material:getName())
        print("  - Albedo: " .. tostring(material:getAlbedo()))
        print("  - Metálico: " .. material:getMetallic())
        print("  - Rugosidad: " .. material:getRoughness())
        print("  - Emisivo: " .. tostring(material:getEmissive()))
    else
        print("❌ ERROR: No hay material disponible")
    end
    
    -- Verificar GameObject
    if testObject then
        print("✅ GameObject disponible")
        local gameObjectMaterial = testObject:getMaterial()
        if gameObjectMaterial then
            print("✅ GameObject tiene material asignado")
        else
            print("❌ ERROR: GameObject no tiene material asignado")
        end
    else
        print("❌ ERROR: GameObject no disponible")
    end
end

-- Ejecutar monitoreo inicial
monitorSpriteAnimator()

-- Función para probar cambios de estado
local function testStateChanges()
    print("=== PROBANDO CAMBIOS DE ESTADO ===")
    
    local states = {"Idle", "Walk"}
    for i, state in ipairs(states) do
        print("Cambiando a estado: " .. state)
        spriteAnimator.currentState = state
        spriteAnimator:update() -- Forzar actualización
        
        -- Esperar un poco para ver si hay cambios
        print("Esperando 1 segundo...")
        -- En un entorno real, esto sería manejado por el sistema de tiempo
        
        monitorSpriteAnimator()
    end
end

-- Función para probar animación
local function testAnimation()
    print("=== PROBANDO ANIMACIÓN ===")
    
    spriteAnimator:playAnimation("Idle")
    print("Animación iniciada")
    
    -- Simular algunos frames
    for i = 1, 5 do
        spriteAnimator:setCurrentFrame(i)
        spriteAnimator:update()
        print("Frame " .. i .. " establecido")
        monitorSpriteAnimator()
    end
    
    spriteAnimator:stopAnimation()
    print("Animación detenida")
end

-- Ejecutar pruebas
testStateChanges()
testAnimation()

-- Función para verificar integridad del material
local function verifyMaterialIntegrity()
    print("=== VERIFICANDO INTEGRIDAD DEL MATERIAL ===")
    
    local material = spriteAnimator:getMaterial()
    if material then
        -- Intentar cambiar propiedades del material
        spriteAnimator:setSpriteAlbedo(vec3(0.0, 1.0, 0.0)) -- Verde
        print("Color cambiado a verde")
        
        spriteAnimator:setSpriteEmissive(vec3(0.0, 0.3, 0.0)) -- Emisión verde
        print("Emisión cambiada a verde")
        
        monitorSpriteAnimator()
        
        -- Cambiar de vuelta a rojo
        spriteAnimator:setSpriteAlbedo(vec3(1.0, 0.0, 0.0))
        spriteAnimator:setSpriteEmissive(vec3(0.2, 0.0, 0.0))
        print("Color restaurado a rojo")
    end
end

verifyMaterialIntegrity()

print("=== DIAGNÓSTICO COMPLETADO ===")
print("Si el sprite se ve rojo y brillante, el problema está en las texturas")
print("Si el sprite se ve negro, el problema está en el material o el renderizado")
print("Si el sprite no se ve, el problema está en la configuración del GameObject")

-- Función para limpiar
local function cleanup()
    print("=== LIMPIEZA ===")
    if spriteAnimator then
        spriteAnimator:stopAnimation()
    end
    print("✅ Limpieza completada")
end

cleanup()

print("🎯 DIAGNÓSTICO FINALIZADO - Revisa la consola para ver los resultados") 
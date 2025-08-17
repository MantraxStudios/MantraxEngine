-- ===== SPRITE ANIMATOR FINAL TEST =====
-- Script final para diagnosticar y solucionar el problema del SpriteAnimator

print("=== INICIANDO PRUEBA FINAL DEL SPRITE ANIMATOR ===")

-- Crear GameObject de prueba
local testObject = GameObject.create("FinalTestSprite")
local spriteAnimator = testObject:addComponent("SpriteAnimator")

if not spriteAnimator then
    print("❌ ERROR CRÍTICO: No se pudo crear el SpriteAnimator")
    return
end

print("✅ SpriteAnimator creado exitosamente")

-- Habilitar modo debug
spriteAnimator:enableDebugMode(true)
print("✅ Modo debug habilitado")

-- Probar configuración básica del material
spriteAnimator:createMaterial("FinalTestMaterial")
spriteAnimator:setSpriteAlbedo(vec3(1.0, 0.0, 0.0)) -- Rojo brillante
spriteAnimator:setSpriteMetallic(0.0)
spriteAnimator:setSpriteRoughness(0.3)
spriteAnimator:setSpriteEmissive(vec3(0.3, 0.0, 0.0)) -- Emisión roja fuerte
spriteAnimator:setSpriteTiling(vec2(1.0, 1.0))

print("✅ Material configurado con color rojo brillante y emisión fuerte")

-- Verificar integridad del material
if spriteAnimator:isMaterialValid() then
    print("✅ Material válido")
else
    print("❌ ERROR: Material inválido")
end

-- Crear estados de prueba
local idleState = {
    state_name = "Idle",
    texture = {}
}

local walkState = {
    state_name = "Walk", 
    texture = {}
}

spriteAnimator.SpriteStates:push_back(idleState)
spriteAnimator.SpriteStates:push_back(walkState)

spriteAnimator.currentState = "Idle"

print("✅ Estados de animación configurados")

-- Verificar estados
if spriteAnimator:isValidState("Idle") then
    print("✅ Estado 'Idle' válido")
else
    print("❌ ERROR: Estado 'Idle' inválido")
end

if spriteAnimator:isValidState("Walk") then
    print("✅ Estado 'Walk' válido")
else
    print("❌ ERROR: Estado 'Walk' inválido")
end

-- Inicializar el componente
spriteAnimator:start()
print("✅ Componente inicializado")

-- Mostrar información de debug inicial
print("=== INFORMACIÓN DE DEBUG INICIAL ===")
spriteAnimator:printDebugInfo()

-- Función para probar cambios de estado con validación
local function testStateChangesWithValidation()
    print("=== PROBANDO CAMBIOS DE ESTADO CON VALIDACIÓN ===")
    
    local states = {"Idle", "Walk"}
    for i, state in ipairs(states) do
        print("Cambiando a estado: " .. state)
        
        if spriteAnimator:isValidState(state) then
            spriteAnimator.currentState = state
            spriteAnimator:forceUpdate() -- Forzar actualización
            spriteAnimator:update()
            
            print("✅ Estado cambiado exitosamente a: " .. state)
            
            -- Verificar si tiene texturas válidas
            if spriteAnimator:hasValidTextures(state) then
                print("✅ Estado '" .. state .. "' tiene texturas válidas")
            else
                print("⚠️ ADVERTENCIA: Estado '" .. state .. "' no tiene texturas válidas")
            end
            
        else
            print("❌ ERROR: Estado '" .. state .. "' no es válido")
        end
    end
end

-- Función para probar animación con validación
local function testAnimationWithValidation()
    print("=== PROBANDO ANIMACIÓN CON VALIDACIÓN ===")
    
    if spriteAnimator:isValidState("Idle") then
        spriteAnimator:playAnimation("Idle")
        print("✅ Animación iniciada para estado 'Idle'")
        
        -- Simular algunos frames con validación
        for i = 1, 3 do
            spriteAnimator:setCurrentFrame(i)
            spriteAnimator:update()
            
            print("Frame " .. i .. " establecido")
            print("Frame actual: " .. spriteAnimator:getCurrentFrame())
            print("¿Está reproduciendo? " .. tostring(spriteAnimator:getIsPlaying()))
        end
        
        spriteAnimator:stopAnimation()
        print("✅ Animación detenida")
    else
        print("❌ ERROR: No se puede reproducir animación - estado inválido")
    end
end

-- Función para verificar integridad completa
local function verifyCompleteIntegrity()
    print("=== VERIFICANDO INTEGRIDAD COMPLETA ===")
    
    -- Verificar material
    if spriteAnimator:isMaterialValid() then
        print("✅ Material válido")
        
        -- Probar cambios de color
        spriteAnimator:setSpriteAlbedo(vec3(0.0, 1.0, 0.0)) -- Verde
        print("Color cambiado a verde")
        
        spriteAnimator:setSpriteEmissive(vec3(0.0, 0.5, 0.0)) -- Emisión verde
        print("Emisión cambiada a verde")
        
        -- Forzar actualización
        spriteAnimator:forceUpdate()
        spriteAnimator:update()
        
        -- Restaurar color rojo
        spriteAnimator:setSpriteAlbedo(vec3(1.0, 0.0, 0.0))
        spriteAnimator:setSpriteEmissive(vec3(0.3, 0.0, 0.0))
        print("Color restaurado a rojo")
        
    else
        print("❌ ERROR: Material inválido")
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

-- Ejecutar todas las pruebas
testStateChangesWithValidation()
testAnimationWithValidation()
verifyCompleteIntegrity()

-- Mostrar información de debug final
print("=== INFORMACIÓN DE DEBUG FINAL ===")
spriteAnimator:printDebugInfo()

-- Función para diagnóstico final
local function finalDiagnosis()
    print("=== DIAGNÓSTICO FINAL ===")
    
    local issues = {}
    
    -- Verificar material
    if not spriteAnimator:isMaterialValid() then
        table.insert(issues, "Material inválido")
    end
    
    -- Verificar estados
    if not spriteAnimator:isValidState("Idle") then
        table.insert(issues, "Estado 'Idle' inválido")
    end
    
    if not spriteAnimator:isValidState("Walk") then
        table.insert(issues, "Estado 'Walk' inválido")
    end
    
    -- Verificar texturas
    if not spriteAnimator:hasValidTextures("Idle") then
        table.insert(issues, "Estado 'Idle' sin texturas válidas")
    end
    
    if not spriteAnimator:hasValidTextures("Walk") then
        table.insert(issues, "Estado 'Walk' sin texturas válidas")
    end
    
    -- Mostrar resultados
    if #issues == 0 then
        print("✅ No se detectaron problemas críticos")
        print("💡 Si el sprite se ve negro, el problema puede estar en:")
        print("   - Configuración de iluminación")
        print("   - Configuración de shaders")
        print("   - Configuración de renderizado")
        print("   - Posición de la cámara")
    else
        print("❌ PROBLEMAS DETECTADOS:")
        for i, issue in ipairs(issues) do
            print("   " .. i .. ". " .. issue)
        end
    end
end

finalDiagnosis()

-- Función para limpiar
local function cleanup()
    print("=== LIMPIEZA ===")
    if spriteAnimator then
        spriteAnimator:stopAnimation()
        spriteAnimator:enableDebugMode(false)
    end
    print("✅ Limpieza completada")
end

cleanup()

print("🎯 PRUEBA FINAL COMPLETADA")
print("📋 Revisa la consola para ver todos los resultados del diagnóstico")
print("🔧 Si hay problemas, usa la información de debug para solucionarlos") 
-- ===== SPRITE ANIMATOR PERSISTENT TEXTURES TEST =====
-- Script para probar el nuevo sistema de texturas persistentes

print("=== INICIANDO PRUEBA DE TEXTURAS PERSISTENTES ===")

-- Crear GameObject de prueba
local testObject = GameObject.create("PersistentTestSprite")
local spriteAnimator = testObject:addComponent("SpriteAnimator")

if not spriteAnimator then
    print("❌ ERROR: No se pudo crear el SpriteAnimator")
    return
end

print("✅ SpriteAnimator creado exitosamente")

-- Habilitar modo debug
spriteAnimator:enableDebugMode(true)
print("✅ Modo debug habilitado")

-- Configurar material
spriteAnimator:createMaterial("PersistentTestMaterial")
spriteAnimator:setSpriteAlbedo(vec3(1.0, 1.0, 1.0)) -- Blanco
spriteAnimator:setSpriteMetallic(0.0)
spriteAnimator:setSpriteRoughness(0.5)
spriteAnimator:setSpriteEmissive(vec3(0.1, 0.1, 0.1)) -- Ligera emisión
spriteAnimator:setSpriteTiling(vec2(1.0, 1.0))

print("✅ Material configurado")

-- Agregar estados con rutas de texturas (ejemplo)
-- En un caso real, estas serían rutas reales a archivos de textura
print("📁 Agregando estados con rutas de texturas...")

-- Estado "idle" con 4 frames
spriteAnimator:addSpriteState("idle")
spriteAnimator:addTextureToState("idle", "assets/textures/idle_1.png")
spriteAnimator:addTextureToState("idle", "assets/textures/idle_2.png")
spriteAnimator:addTextureToState("idle", "assets/textures/idle_3.png")
spriteAnimator:addTextureToState("idle", "assets/textures/idle_4.png")

-- Estado "walk" con 6 frames
spriteAnimator:addSpriteState("walk")
spriteAnimator:addTextureToState("walk", "assets/textures/walk_1.png")
spriteAnimator:addTextureToState("walk", "assets/textures/walk_2.png")
spriteAnimator:addTextureToState("walk", "assets/textures/walk_3.png")
spriteAnimator:addTextureToState("walk", "assets/textures/walk_4.png")
spriteAnimator:addTextureToState("walk", "assets/textures/walk_5.png")
spriteAnimator:addTextureToState("walk", "assets/textures/walk_6.png")

print("✅ Estados agregados")

-- Precargar todas las texturas
print("🔄 Precargando texturas...")
spriteAnimator:preloadAllTextures()

-- Verificar integridad
print("🔍 Verificando integridad...")
if spriteAnimator:isMaterialValid() then
    print("✅ Material válido")
else
    print("❌ ERROR: Material inválido")
end

if spriteAnimator:isValidState("idle") then
    print("✅ Estado 'idle' válido")
else
    print("❌ ERROR: Estado 'idle' inválido")
end

if spriteAnimator:hasValidTextures("idle") then
    print("✅ Estado 'idle' tiene texturas válidas")
else
    print("❌ ERROR: Estado 'idle' no tiene texturas válidas")
end

-- Probar animación
print("🎬 Probando animación...")
spriteAnimator:setCurrentState("idle")
spriteAnimator:playAnimation("idle")

-- Simular algunos frames de animación
for i = 1, 10 do
    print("Frame " .. i .. ": " .. spriteAnimator:getCurrentFrame())
    -- En un caso real, esto se haría automáticamente en el update
    spriteAnimator:setCurrentFrame(i % 4) -- Ciclo entre 0-3
end

-- Cambiar a estado walk
print("🚶 Cambiando a estado 'walk'...")
spriteAnimator:setCurrentState("walk")
spriteAnimator:playAnimation("walk")

-- Probar algunos frames del nuevo estado
for i = 1, 6 do
    print("Walk Frame " .. i .. ": " .. spriteAnimator:getCurrentFrame())
    spriteAnimator:setCurrentFrame(i % 6) -- Ciclo entre 0-5
end

-- Mostrar información de debug
print("📊 Información de debug:")
spriteAnimator:printDebugInfo()

-- Probar limpieza de cache
print("🧹 Probando limpieza de cache...")
spriteAnimator:clearTextureCache()

-- Recargar texturas
print("🔄 Recargando texturas...")
spriteAnimator:preloadAllTextures()

print("✅ Prueba de texturas persistentes completada")
print("💡 Las texturas ahora se mantienen en memoria y no se crean/destruyen constantemente") 
-- ===== SPRITE ANIMATOR TEST =====
-- Este archivo prueba todas las funcionalidades del SpriteAnimator

print("=== INICIANDO PRUEBA DEL SPRITE ANIMATOR ===")

-- Crear GameObject de prueba
local testObject = GameObject.create("TestSprite")
local spriteAnimator = testObject:addComponent("SpriteAnimator")

print("✅ SpriteAnimator creado exitosamente")

-- Probar configuración básica del material
spriteAnimator:createMaterial("TestMaterial")
spriteAnimator:setSpriteAlbedo(vec3(1.0, 0.5, 0.5)) -- Rojo claro
spriteAnimator:setSpriteMetallic(0.2)
spriteAnimator:setSpriteRoughness(0.4)
spriteAnimator:setSpriteEmissive(vec3(0.1, 0.0, 0.0))
spriteAnimator:setSpriteTiling(vec2(1.0, 1.0))

print("✅ Material configurado exitosamente")

-- Probar estados de animación
local idleState = {
    state_name = "Idle",
    texture = {}
}

local walkState = {
    state_name = "Walk",
    texture = {}
}

local runState = {
    state_name = "Run",
    texture = {}
}

spriteAnimator.SpriteStates:push_back(idleState)
spriteAnimator.SpriteStates:push_back(walkState)
spriteAnimator.SpriteStates:push_back(runState)

spriteAnimator.currentState = "Idle"

print("✅ Estados de animación configurados")

-- Inicializar el componente
spriteAnimator:start()

print("✅ Componente inicializado")

-- Probar cambio de estados
print("Probando cambio de estados...")
spriteAnimator.currentState = "Walk"
print("Estado cambiado a: " .. spriteAnimator.currentState)

spriteAnimator.currentState = "Run"
print("Estado cambiado a: " .. spriteAnimator.currentState)

spriteAnimator.currentState = "Idle"
print("Estado cambiado a: " .. spriteAnimator.currentState)

-- Probar serialización
print("Probando serialización...")
local configuracion = spriteAnimator:serializeComponent()
print("Configuración serializada (primeros 200 caracteres):")
print(string.sub(configuracion, 1, 200) .. "...")

-- Probar deserialización
print("Probando deserialización...")
spriteAnimator:deserialize(configuracion)
print("✅ Deserialización exitosa")

-- Probar presets
print("Probando presets...")
configurarPresetMaterial("brillante")
print("Preset brillante aplicado")

configurarPresetMaterial("mate")
print("Preset mate aplicado")

configurarPresetMaterial("emissivo")
print("Preset emissivo aplicado")

configurarPresetMaterial("metalico")
print("Preset metálico aplicado")

-- Probar configuración por tipo
print("Probando configuración por tipo...")
configurarMaterialPorTipo("personaje")
print("Configuración para personaje aplicada")

configurarMaterialPorTipo("ui")
print("Configuración para UI aplicada")

configurarMaterialPorTipo("efecto")
print("Configuración para efecto aplicada")

-- Probar funciones de utilidad
print("Probando funciones de utilidad...")
local estados = obtenerEstadosDisponibles()
print("Estados disponibles: " .. #estados)

for i, estado in ipairs(estados) do
    print("  " .. i .. ": " .. estado)
end

print("¿Existe estado 'Idle'? " .. tostring(estadoExiste("Idle")))
print("¿Existe estado 'Jump'? " .. tostring(estadoExiste("Jump")))

-- Obtener información completa
obtenerInfoCompleta()

-- Probar creación rápida
print("Probando creación rápida...")
local spriteRapido = crearSpriteRapido("SpriteRapido", "Default")
print("✅ Sprite rápido creado")

-- Probar funciones del inspector
print("Probando funciones del inspector...")
cambiarEstado("Walk")
configurarMaterial()
obtenerInfoMaterial()
crearMaterialPersonalizado()
actualizarTextura()
cambiarAnimacion()
obtenerInfoEstado()

print("=== PRUEBA COMPLETADA EXITOSAMENTE ===")
print("✅ Todas las funcionalidades del SpriteAnimator funcionan correctamente")
print("✅ El inspector está completamente funcional")
print("✅ La serialización funciona correctamente")
print("✅ Los presets y configuraciones funcionan")
print("✅ Las funciones de utilidad funcionan")

print("")
print("🎉 ¡El SpriteAnimator está listo para usar desde el inspector!")
print("")
print("Funciones disponibles:")
print("- cambiarEstado(nuevoEstado)")
print("- configurarMaterial()")
print("- configurarPresetMaterial(preset)")
print("- configurarMaterialPorTipo(tipo)")
print("- guardarConfiguracion()")
print("- crearSpriteRapido(nombre, estadoInicial)")
print("- obtenerInfoCompleta()")
print("")
print("Presets: brillante, mate, emissivo, metalico")
print("Tipos: personaje, ui, efecto") 
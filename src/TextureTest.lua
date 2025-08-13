-- Texture Test Script
-- Este script prueba la carga de texturas con el sistema actual

print("=== Texture Test Script ===")

-- Crear un material de prueba
local testMaterial = Material("TestMaterial")
print("Material creado: " .. testMaterial:getName())

-- Intentar cargar una textura de albedo (ruta relativa)
local texturePath = "Sprites\\Backgrounds\\tile_0009.png"
print("Intentando cargar textura con ruta relativa: " .. texturePath)

testMaterial:setAlbedoTexture(texturePath)

-- Verificar el estado del material
print("\n=== Estado del Material ===")
print("Nombre: " .. testMaterial:getName())
print("Tiene textura de albedo: " .. (testMaterial:hasAlbedoTexture() and "SÍ" or "NO"))

if testMaterial:hasAlbedoTexture() then
    local albedoTexture = testMaterial:getAlbedoTexture()
    print("ID de textura: " .. albedoTexture:getID())
    print("Ruta de archivo: " .. albedoTexture:getFilePath())
    print("Dimensiones: " .. albedoTexture:getWidth() .. "x" .. albedoTexture:getHeight())
    
    -- Verificar que la textura sea válida
    if albedoTexture:getID() > 0 then
        print("✓ Textura válida con ID: " .. albedoTexture:getID())
    else
        print("✗ Textura inválida - ID es 0")
    end
else
    print("✗ No se pudo cargar la textura de albedo")
end

-- Debug del estado de texturas
print("\n=== Debug de Texturas ===")
testMaterial:debugTextureState()

print("\n=== Fin del Test ===")

-- Resumen del problema
print("\n=== RESUMEN ===")
if testMaterial:hasAlbedoTexture() and testMaterial:getAlbedoTexture():getID() > 0 then
    print("✓ Las texturas se están cargando correctamente")
    print("✓ El sistema de rutas está funcionando")
    print("✓ El problema podría estar en el renderizado o shader")
else
    print("✗ El problema está en la carga de texturas")
    print("  - Verificar que los archivos existan en Content/")
    print("  - Verificar que las rutas en materials_config.json sean correctas")
    print("  - Verificar que STB_Image pueda cargar los archivos")
end

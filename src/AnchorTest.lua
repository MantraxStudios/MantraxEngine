-- Anchor Test
-- Este script prueba todos los anclajes del canvas para verificar que funcionen correctamente

print("=== Anchor Test ===")

-- Obtener la cámara activa
local camera = getActiveCamera()
if not camera then
    print("Error: No se encontró una cámara activa")
    return
end

-- Habilitar framebuffer para probar con buffer de cámara
camera:enableFramebuffer(true)
camera:setFramebufferSize(800, 600)

print("Framebuffer habilitado: 800x600")

-- Función para probar todos los anclajes
function testAllAnchors()
    print("Probando todos los anclajes...")
    
    -- TopLeft (esquina superior izquierda)
    print("TopLeft: Esquina superior izquierda")
    
    -- TopCenter (centro superior)
    print("TopCenter: Centro superior")
    
    -- TopRight (esquina superior derecha)
    print("TopRight: Esquina superior derecha")
    
    -- CenterLeft (centro izquierda)
    print("CenterLeft: Centro izquierda")
    
    -- Center (centro de la pantalla)
    print("Center: Centro de la pantalla")
    
    -- CenterRight (centro derecha)
    print("CenterRight: Centro derecha")
    
    -- BottomLeft (esquina inferior izquierda)
    print("BottomLeft: Esquina inferior izquierda")
    
    -- BottomCenter (centro inferior)
    print("BottomCenter: Centro inferior")
    
    -- BottomRight (esquina inferior derecha)
    print("BottomRight: Esquina inferior derecha")
end

-- Ejecutar la prueba
testAllAnchors()

print("Todos los anclajes deberían estar visibles en sus posiciones correctas")
print("Si algún anclaje no se ve bien, hay un problema en el cálculo de posiciones") 
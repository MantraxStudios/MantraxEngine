-- Canvas Buffer Test
-- Este script demuestra cómo hacer que el UI del canvas use el width y height del buffer de la cámara

print("=== Canvas Buffer Test ===")

-- Obtener la cámara activa
local camera = getActiveCamera()
if not camera then
    print("Error: No se encontró una cámara activa")
    return
end

print("Cámara encontrada")

-- Configurar el framebuffer de la cámara
-- Esto hará que el canvas use el tamaño del buffer en lugar del tamaño de la ventana
camera:enableFramebuffer(true)
camera:setFramebufferSize(1024, 768)  -- Tamaño personalizado del buffer

print("Framebuffer habilitado con tamaño: 1024x768")

-- Verificar el estado del framebuffer
local isEnabled = camera:isFramebufferEnabled()
print("Framebuffer habilitado: " .. tostring(isEnabled))

-- El RenderPipeline automáticamente actualizará el canvas con el tamaño del buffer
-- en el próximo frame de renderizado

-- También puedes cambiar el tamaño del buffer dinámicamente
function updateBufferSize()
    local newWidth = 800
    local newHeight = 600
    camera:setFramebufferSize(newWidth, newHeight)
    print("Buffer size actualizado a: " .. newWidth .. "x" .. newHeight)
end

-- Ejemplo de cómo cambiar el tamaño del buffer después de un tiempo
-- (descomenta las líneas siguientes para probar)
--[[
local timer = 0
function update(deltaTime)
    timer = timer + deltaTime
    if timer > 5.0 then  -- Después de 5 segundos
        updateBufferSize()
        timer = 0
    end
end
--]]

print("El canvas ahora usará el tamaño del buffer de la cámara en lugar del tamaño de la ventana")
print("Esto es útil para renderizado a textura o efectos de post-procesamiento") 
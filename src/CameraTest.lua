-- Camera Test Script
-- Este script demuestra cómo acceder y modificar la cámara del nivel desde Lua

function start()
    log("=== Camera Test Script Started ===")
    
    -- Obtener la cámara activa
    local camera = getActiveCamera()
    if not camera then
        log("Error: No se pudo obtener la cámara activa")
        return
    end
    
    log("Cámara obtenida exitosamente")
    
    -- Mostrar información inicial de la cámara
    local pos = camera:getPosition()
    log("Posición inicial: " .. pos.x .. ", " .. pos.y .. ", " .. pos.z)
    
    local fov = camera:getFOV()
    log("FOV inicial: " .. fov)
    
    local aspect = camera:getAspectRatio()
    log("Aspect Ratio inicial: " .. aspect)
    
    -- Cambiar posición de la cámara
    camera:setPosition(vector3(0, 5, 10))
    log("Posición cambiada a (0, 5, 10)")
    
    -- Cambiar rotación de la cámara
    camera:setRotation(-90, 0)  -- Mirar hacia adelante
    log("Rotación cambiada a (-90, 0)")
    
    -- Cambiar FOV
    camera:setAspectRatio(16.0 / 9.0)
    log("Aspect Ratio cambiado a 16:9")
    
    -- Cambiar tipo de proyección
    camera:setProjectionType(ProjectionType.Perspective, true)
    log("Proyección cambiada a Perspective")
    
    -- Mostrar vectores de orientación
    local forward = camera:getForward()
    local right = camera:getRight()
    local up = camera:getUp()
    
    log("Forward: " .. forward.x .. ", " .. forward.y .. ", " .. forward.z)
    log("Right: " .. right.x .. ", " .. right.y .. ", " .. right.z)
    log("Up: " .. up.x .. ", " .. up.y .. ", " .. up.z)
end

function update(deltaTime)
    local camera = getActiveCamera()
    if not camera then
        return
    end
    
    -- Ejemplo de movimiento de cámara en tiempo real
    -- Esto se ejecutará cada frame
    
    -- Obtener input (ejemplo con teclas WASD)
    local moveSpeed = 5.0 * deltaTime
    
    -- Mover hacia adelante con W
    if getAction("MoveForward") and getAction("MoveForward"):getValue() > 0 then
        camera:moveForward(moveSpeed)
    end
    
    -- Mover hacia atrás con S
    if getAction("MoveBackward") and getAction("MoveBackward"):getValue() > 0 then
        camera:moveForward(-moveSpeed)
    end
    
    -- Mover hacia la derecha con D
    if getAction("MoveRight") and getAction("MoveRight"):getValue() > 0 then
        camera:moveRight(moveSpeed)
    end
    
    -- Mover hacia la izquierda con A
    if getAction("MoveLeft") and getAction("MoveLeft"):getValue() > 0 then
        camera:moveRight(-moveSpeed)
    end
    
    -- Mover hacia arriba con Espacio
    if getAction("MoveUp") and getAction("MoveUp"):getValue() > 0 then
        camera:moveUp(moveSpeed)
    end
    
    -- Mover hacia abajo con Shift
    if getAction("MoveDown") and getAction("MoveDown"):getValue() > 0 then
        camera:moveUp(-moveSpeed)
    end
    
    -- Rotar cámara con el mouse
    local mouseX = getAction("MouseX")
    local mouseY = getAction("MouseY")
    
    if mouseX and mouseY then
        local sensitivity = 0.1
        local deltaX = mouseX:getValue() * sensitivity
        local deltaY = mouseY:getValue() * sensitivity
        
        camera:rotate(deltaX, -deltaY)  -- Invertir Y para movimiento natural
    end
end

function onTriggerEnter(other)
    local camera = getActiveCamera()
    if not camera then
        return
    end
    
    -- Ejemplo: cambiar FOV cuando el jugador entra en un trigger
    log("Trigger entered! Cambiando FOV...")
    camera:setAspectRatio(21.0 / 9.0)  -- Cambiar a ultra-wide
end

function onTriggerExit(other)
    local camera = getActiveCamera()
    if not camera then
        return
    end
    
    -- Restaurar FOV cuando el jugador sale del trigger
    log("Trigger exited! Restaurando FOV...")
    camera:setAspectRatio(16.0 / 9.0)  -- Restaurar aspect ratio normal
end

-- Función de utilidad para crear una cámara de seguimiento
function createFollowCamera(targetObject, offset)
    local camera = getActiveCamera()
    if not camera then
        return
    end
    
    -- Obtener posición del objeto objetivo
    local targetPos = targetObject:getPosition()
    
    -- Calcular posición de la cámara con offset
    local cameraPos = vector3(
        targetPos.x + offset.x,
        targetPos.y + offset.y,
        targetPos.z + offset.z
    )
    
    -- Posicionar cámara
    camera:setPosition(cameraPos)
    
    -- Hacer que la cámara mire al objetivo
    camera:setTarget(targetPos)
    
    log("Cámara de seguimiento configurada")
end

-- Función de utilidad para crear una cámara orbital
function createOrbitalCamera(targetObject, distance, height)
    local camera = getActiveCamera()
    if not camera then
        return
    end
    
    local targetPos = targetObject:getPosition()
    
    -- Calcular posición orbital
    local cameraPos = vector3(
        targetPos.x,
        targetPos.y + height,
        targetPos.z + distance
    )
    
    camera:setPosition(cameraPos)
    camera:setTarget(targetPos)
    
    log("Cámara orbital configurada")
end

-- Función para cambiar entre proyección perspectiva y ortográfica
function toggleProjection()
    local camera = getActiveCamera()
    if not camera then
        return
    end
    
    local currentType = camera:getProjectionType()
    
    if currentType == ProjectionType.Perspective then
        camera:setProjectionType(ProjectionType.Orthographic, false)
        log("Cambiando a proyección ortográfica")
    else
        camera:setProjectionType(ProjectionType.Perspective, false)
        log("Cambiando a proyección perspectiva")
    end
end

log("Camera Test Script loaded successfully!") 
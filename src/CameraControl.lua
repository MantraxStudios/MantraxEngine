-- Camera Control Script
-- Script simple para controlar la cámara del nivel

function start()
    log("=== Camera Control Script Started ===")
    
    -- Obtener la cámara activa
    local camera = getActiveCamera()
    if not camera then
        log("Error: No se pudo obtener la cámara activa")
        return
    end
    
    log("Cámara obtenida exitosamente")
    
    -- Mostrar información inicial
    local pos = camera:getPosition()
    log("Posición inicial: " .. pos.x .. ", " .. pos.y .. ", " .. pos.z)
    
    -- Cambiar posición de la cámara
    camera:setPosition(vector3(0, 3, 8))
    log("Posición cambiada a (0, 3, 8)")
    
    -- Cambiar rotación para mirar hacia el origen
    camera:setRotation(-90, 0)
    log("Rotación ajustada")
end

function update(deltaTime)
    local camera = getActiveCamera()
    if not camera then
        return
    end
    
    -- Movimiento básico de cámara
    local speed = 3.0 * deltaTime
    
    -- Mover hacia adelante/atrás
    if getAction("MoveForward") and getAction("MoveForward"):getValue() > 0 then
        camera:moveForward(speed)
    end
    
    if getAction("MoveBackward") and getAction("MoveBackward"):getValue() > 0 then
        camera:moveForward(-speed)
    end
    
    -- Mover lateralmente
    if getAction("MoveRight") and getAction("MoveRight"):getValue() > 0 then
        camera:moveRight(speed)
    end
    
    if getAction("MoveLeft") and getAction("MoveLeft"):getValue() > 0 then
        camera:moveRight(-speed)
    end
    
    -- Mover verticalmente
    if getAction("MoveUp") and getAction("MoveUp"):getValue() > 0 then
        camera:moveUp(speed)
    end
    
    if getAction("MoveDown") and getAction("MoveDown"):getValue() > 0 then
        camera:moveUp(-speed)
    end
end

-- Función para cambiar la posición de la cámara
function setCameraPosition(x, y, z)
    local camera = getActiveCamera()
    if camera then
        camera:setPosition(vector3(x, y, z))
        log("Cámara movida a: " .. x .. ", " .. y .. ", " .. z)
    end
end

-- Función para cambiar la rotación de la cámara
function setCameraRotation(yaw, pitch)
    local camera = getActiveCamera()
    if camera then
        camera:setRotation(yaw, pitch)
        log("Rotación cambiada a: " .. yaw .. ", " .. pitch)
    end
end

-- Función para cambiar el FOV
function setCameraFOV(fov)
    local camera = getActiveCamera()
    if camera then
        -- Nota: setFOV no está implementado en la clase Camera aún
        log("FOV actual: " .. camera:getFOV())
    end
end

-- Función para cambiar el tipo de proyección
function setProjectionType(type)
    local camera = getActiveCamera()
    if camera then
        camera:setProjectionType(type, true)
        log("Tipo de proyección cambiado")
    end
end

log("Camera Control Script loaded!") 
-- Script de prueba para verificar que la dirección de la luz se aplique correctamente
-- Este script debe ejecutarse en un GameObject que tenga un LightComponent

function onStart()
    print("=== LIGHT TEST SCRIPT STARTED ===")
    
    -- Obtener el componente de luz
    local lightComponent = gameObject:getComponent("LightComponent")
    if not lightComponent then
        print("ERROR: No se encontró LightComponent en este GameObject")
        return
    end
    
    print("LightComponent encontrado:")
    print("  Tipo: " .. tostring(lightComponent:getType()))
    print("  Color: " .. tostring(lightComponent:getColor().x) .. ", " .. tostring(lightComponent:getColor().y) .. ", " .. tostring(lightComponent:getColor().z))
    print("  Intensidad: " .. tostring(lightComponent:getIntensity()))
    print("  Posición: " .. tostring(lightComponent:getPosition().x) .. ", " .. tostring(lightComponent:getPosition().y) .. ", " .. tostring(lightComponent:getPosition().z))
    print("  Dirección: " .. tostring(lightComponent:getDirection().x) .. ", " .. tostring(lightComponent:getDirection().y) .. ", " .. tostring(lightComponent:getDirection().z))
    
    -- Forzar actualización de transformación
    lightComponent:forceTransformUpdate()
    
    print("Transformación actualizada")
end

function onUpdate(deltaTime)
    -- Obtener el componente de luz
    local lightComponent = gameObject:getComponent("LightComponent")
    if not lightComponent then
        return
    end
    
    -- Mostrar información en tiempo real (cada segundo)
    if math.floor(Time.time * 10) % 10 == 0 then
        local pos = gameObject:getWorldPosition()
        local rot = gameObject:getWorldRotationQuat()
        local lightDir = lightComponent:getDirection()
        local lightPos = lightComponent:getPosition()
        
        print("=== LIGHT STATUS UPDATE ===")
        print("GameObject:")
        print("  Posición: (" .. tostring(pos.x) .. ", " .. tostring(pos.y) .. ", " .. tostring(pos.z) .. ")")
        print("  Rotación: (" .. tostring(rot.x) .. ", " .. tostring(rot.y) .. ", " .. tostring(rot.z) .. ", " .. tostring(rot.w) .. ")")
        print("Luz:")
        print("  Posición: (" .. tostring(lightPos.x) .. ", " .. tostring(lightPos.y) .. ", " .. tostring(lightPos.z) .. ")")
        print("  Dirección: (" .. tostring(lightDir.x) .. ", " .. tostring(lightDir.y) .. ", " .. tostring(lightDir.z) .. ")")
        print("========================")
    end
end

function onKeyPress(key)
    local lightComponent = gameObject:getComponent("LightComponent")
    if not lightComponent then
        return
    end
    
    if key == "R" then
        -- Rotar el GameObject 90 grados en Y
        local currentRot = gameObject:getLocalRotationQuat()
        local newRot = currentRot * glm.quat(glm.radians(90.0), glm.vec3(0, 1, 0))
        gameObject:setLocalRotationQuat(newRot)
        
        -- Forzar actualización de la luz
        lightComponent:forceTransformUpdate()
        
        print("GameObject rotado 90° en Y, luz actualizada")
        
    elseif key == "T" then
        -- Establecer dirección manual de la luz
        local newDir = glm.vec3(0, -1, 0) -- Hacia abajo
        lightComponent:setDirection(newDir)
        
        print("Dirección de luz establecida manualmente a (0, -1, 0)")
        
    elseif key == "Y" then
        -- Establecer dirección manual de la luz
        local newDir = glm.vec3(1, 0, 0) -- Hacia la derecha
        lightComponent:setDirection(newDir)
        
        print("Dirección de luz establecida manualmente a (1, 0, 0)")
        
    elseif key == "U" then
        -- Establecer dirección manual de la luz
        local newDir = glm.vec3(0, 0, -1) -- Hacia adelante
        lightComponent:setDirection(newDir)
        
        print("Dirección de luz establecida manualmente a (0, 0, -1)")
    end
end

print("LightTest script loaded successfully")

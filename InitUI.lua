-- Inicialización automática de UI básica
print("=== Auto UI Initialization ===")

-- Función para inicializar UI básica
function InitializeBasicUI()
    local sceneManager = SceneManager.getInstance()
    local activeScene = sceneManager:getActiveScene()
    
    if not activeScene then
        print("ERROR: No active scene for UI initialization")
        return false
    end
    
    local renderPipeline = activeScene:getRenderPipeline()
    if not renderPipeline then
        print("ERROR: No render pipeline for UI initialization")
        return false
    end
    
    -- Verificar si ya existe un canvas
    local canvasCount = renderPipeline:getCanvasCount()
    print("[InitUI] Current canvas count: " .. canvasCount)
    
    local canvas = nil
    if canvasCount == 0 then
        print("[InitUI] Creating basic UI canvas...")
        canvas = renderPipeline:addCanvas(1920, 1080)
        if not canvas then
            print("[InitUI] ERROR: Failed to create canvas!")
            return false
        end
        print("[InitUI] Canvas created successfully!")
    else
        canvas = renderPipeline:getCanvas(0)
        print("[InitUI] Using existing canvas")
    end
    
    -- Crear texto básico de prueba
    print("[InitUI] Creating test UI elements...")
    
    local testText = canvas:MakeNewText("UI System Active!")
    if testText then
        testText:setAnchor(Anchor.TopLeft)
        testText:setPosition(20, 30)
        print("[InitUI] Test text created successfully")
    else
        print("[InitUI] ERROR: Failed to create test text")
        return false
    end
    
    -- Texto adicional para verificar diferentes posiciones
    local centerText = canvas:MakeNewText("Center Text")
    if centerText then
        centerText:setAnchor(Anchor.Center)
        print("[InitUI] Center text created")
    end
    
    print("[InitUI] UI initialization complete!")
    print("[InitUI] Canvas size: " .. canvas:getWidth() .. "x" .. canvas:getHeight())
    print("[InitUI] Total elements: " .. #canvas.RenderElements)
    
    return true
end

-- Ejecutar inicialización
local success = InitializeBasicUI()
if success then
    print("UI initialization successful!")
else
    print("UI initialization failed!")
end

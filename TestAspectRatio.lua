-- Script para probar las mejoras de aspect ratio
print("=== Testing Aspect Ratio Improvements ===")

-- Obtener el scene manager y render pipeline
local sceneManager = SceneManager.getInstance()
local activeScene = sceneManager:getActiveScene()

if not activeScene then
    print("ERROR: No active scene found!")
    return
end

local renderPipeline = activeScene:getRenderPipeline()
if not renderPipeline then
    print("ERROR: No render pipeline found!")
    return
end

print("Scene and RenderPipeline found successfully")

-- Verificar si ya existe un canvas
local canvasCount = renderPipeline:getCanvasCount()
print("Current canvas count: " .. canvasCount)

local canvas = nil
if canvasCount == 0 then
    print("Creating new canvas for aspect ratio testing...")
    canvas = renderPipeline:addCanvas(1920, 1080)
    if not canvas then
        print("ERROR: Failed to create canvas!")
        return
    end
    print("Canvas created successfully!")
else
    canvas = renderPipeline:getCanvas(0)
    print("Using existing canvas")
end

-- Test 1: Configurar modo adaptativo (por defecto)
print("\n=== Test 1: Adaptive Mode ===")
canvas:setAspectRatioMode(AspectRatioMode.Adaptive)

-- Crear elementos de prueba
local testTexts = {}

-- Texto en cada esquina para probar el escalado
testTexts[1] = canvas:MakeNewText("Top-Left Corner")
testTexts[1]:setAnchor(Anchor.TopLeft)
testTexts[1]:setPosition(20, 20)

testTexts[2] = canvas:MakeNewText("Top-Right Corner")
testTexts[2]:setAnchor(Anchor.TopRight)
testTexts[2]:setPosition(20, 20)

testTexts[3] = canvas:MakeNewText("Bottom-Left Corner")
testTexts[3]:setAnchor(Anchor.BottomLeft)
testTexts[3]:setPosition(20, 20)

testTexts[4] = canvas:MakeNewText("Bottom-Right Corner")
testTexts[4]:setAnchor(Anchor.BottomRight)
testTexts[4]:setPosition(20, 20)

testTexts[5] = canvas:MakeNewText("CENTER TEXT")
testTexts[5]:setAnchor(Anchor.Center)

print("Created " .. #testTexts .. " test elements")

-- Simular cambio de resolución para probar el aspect ratio
print("\n=== Testing Resolution Changes ===")

-- Obtener tamaño actual
local currentWidth = canvas:getWidth()
local currentHeight = canvas:getHeight()
print("Current canvas size: " .. currentWidth .. "x" .. currentHeight)

-- Simular diferentes resoluciones
local testResolutions = {
    {1920, 1080, "Full HD 16:9"},
    {1366, 768, "HD 16:9"},
    {1280, 720, "HD Ready 16:9"}, 
    {1600, 900, "HD+ 16:9"},
    {1440, 900, "WXGA+ 16:10"},
    {1280, 1024, "SXGA 5:4"}
}

for i, resolution in ipairs(testResolutions) do
    local width, height, name = resolution[1], resolution[2], resolution[3]
    print("Testing resolution: " .. name .. " (" .. width .. "x" .. height .. ")")
    
    -- Actualizar aspect ratio (esto normalmente se haría automáticamente)
    canvas:updateAspectRatio(width, height)
    
    -- Obtener la escala UI resultante
    local uiScale = canvas:getUIScale()
    print("  UI Scale: " .. uiScale.x .. ", " .. uiScale.y)
    print("  Canvas size after update: " .. canvas:getWidth() .. "x" .. canvas:getHeight())
end

-- Test 2: Modo Letterbox
print("\n=== Test 2: Letterbox Mode ===")
canvas:setAspectRatioMode(AspectRatioMode.Letterbox)
canvas:updateAspectRatio(1280, 1024) -- Resolución 5:4 para ver letterbox
print("Letterbox mode applied for 5:4 resolution")

-- Test 3: Modo Stretch
print("\n=== Test 3: Stretch Mode ===")
canvas:setAspectRatioMode(AspectRatioMode.Stretch)
canvas:updateAspectRatio(1280, 1024)
print("Stretch mode applied")

-- Restaurar modo adaptativo
print("\n=== Restoring Adaptive Mode ===")
canvas:setAspectRatioMode(AspectRatioMode.Adaptive)
canvas:updateAspectRatio(1920, 1080)
print("Restored to adaptive mode with Full HD resolution")

print("\n=== Aspect Ratio Test Complete ===")
print("Canvas final size: " .. canvas:getWidth() .. "x" .. canvas:getHeight())
print("Total UI elements: " .. #canvas.RenderElements)
print("Final UI scale: " .. canvas:getUIScale().x .. ", " .. canvas:getUIScale().y)

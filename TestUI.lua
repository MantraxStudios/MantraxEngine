-- Script para probar UI básica
print("=== Testing UI System ===")

-- Get the current scene and render pipeline
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

-- Check if we already have canvases
local canvasCount = renderPipeline:getCanvasCount()
print("Current canvas count: " .. canvasCount)

-- Create a canvas if none exists
local canvas = nil
if canvasCount == 0 then
    print("Creating new canvas...")
    canvas = renderPipeline:addCanvas(1920, 1080)
    if canvas then
        print("Canvas created successfully!")
    else
        print("ERROR: Failed to create canvas!")
        return
    end
else
    print("Using existing canvas...")
    canvas = renderPipeline:getCanvas(0)
    if not canvas then
        print("ERROR: Failed to get existing canvas!")
        return
    end
end

-- Create test text elements
print("Creating UI text elements...")

-- Test 1: Simple top-left text
local text1 = canvas:MakeNewText("Hello UI System!")
if text1 then
    text1:setAnchor(Anchor.TopLeft)
    text1:setPosition(50, 50)
    print("Created text1: Top-left positioned")
else
    print("ERROR: Failed to create text1")
end

-- Test 2: Center text
local text2 = canvas:MakeNewText("Center Text Test")
if text2 then
    text2:setAnchor(Anchor.Center)
    print("Created text2: Center positioned")
else
    print("ERROR: Failed to create text2")
end

-- Test 3: Bottom-right text
local text3 = canvas:MakeNewText("Bottom Right!")
if text3 then
    text3:setAnchor(Anchor.BottomRight)
    print("Created text3: Bottom-right positioned")
else
    print("ERROR: Failed to create text3")
end

print("UI test setup complete!")
print("Canvas width: " .. canvas:getWidth())
print("Canvas height: " .. canvas:getHeight())
print("Total UI elements: " .. #canvas.RenderElements)

-- Script para probar el CanvasManager mejorado con edición en tiempo real
print("=== Testing Enhanced Canvas Manager Editor ===")

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

-- Crear o obtener canvas
local canvas = nil
local canvasCount = renderPipeline:getCanvasCount()

if canvasCount == 0 then
    print("Creating canvas for editor testing...")
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

-- Crear varios elementos de prueba para el editor
print("\n=== Creating Test Elements for Canvas Manager ===")

local testElements = {}

-- Texto 1: Título principal
testElements[1] = canvas:MakeNewText("Main Title - Click to Select!")
testElements[1]:setAnchor(Anchor.TopCenter)
testElements[1]:setPosition(960, 50)
testElements[1]:enableDrag(true)
print("Created main title element")

-- Texto 2: Subtítulo
testElements[2] = canvas:MakeNewText("Subtitle - Use WASD to Move")
testElements[2]:setAnchor(Anchor.TopCenter)
testElements[2]:setPosition(960, 100)
testElements[2]:enableDrag(true)
print("Created subtitle element")

-- Texto 3: Menú izquierdo
testElements[3] = canvas:MakeNewText("Menu Item 1")
testElements[3]:setAnchor(Anchor.CenterLeft)
testElements[3]:setPosition(100, 300)
testElements[3]:enableDrag(true)
print("Created menu item 1")

-- Texto 4: Menú izquierdo 2
testElements[4] = canvas:MakeNewText("Menu Item 2")
testElements[4]:setAnchor(Anchor.CenterLeft)
testElements[4]:setPosition(100, 350)
testElements[4]:enableDrag(true)
print("Created menu item 2")

-- Texto 5: Información derecha
testElements[5] = canvas:MakeNewText("Info Panel")
testElements[5]:setAnchor(Anchor.CenterRight)
testElements[5]:setPosition(1820, 300)
testElements[5]:enableDrag(true)
print("Created info panel")

-- Texto 6: Footer
testElements[6] = canvas:MakeNewText("Footer Text - Edit Me!")
testElements[6]:setAnchor(Anchor.BottomCenter)
testElements[6]:setPosition(960, 1000)
testElements[6]:enableDrag(true)
print("Created footer element")

-- Texto 7: Estado dinámico
testElements[7] = canvas:MakeNewText("Dynamic Status: Ready")
testElements[7]:setAnchor(Anchor.TopLeft)
testElements[7]:setPosition(50, 150)
testElements[7]:enableDrag(true)
print("Created status element")

print("\n=== Canvas Manager Editor Setup Complete ===")
print("Total elements created: " .. #testElements)
print("Canvas size: " .. canvas:getWidth() .. "x" .. canvas:getHeight())

print("\n=== How to Use the Canvas Manager Editor ===")
print("1. SELECTION:")
print("   - Open the 'Canvas Manager' window in the editor")
print("   - Click on any element in the 'UI Elements' list to select it")
print("   - Selected elements are highlighted in green")

print("\n2. MOVEMENT CONTROLS:")
print("   - WASD or Arrow Keys: Move selected element")
print("   - Hold Shift: Fast movement (3x speed)")
print("   - Manual buttons: Use directional buttons for precise movement")
print("   - Real-time toggle: Enable/disable continuous movement")

print("\n3. POSITION EDITING:")
print("   - Use Position sliders to set exact coordinates")
print("   - Drag & Drop: Elements are draggable by default")
print("   - Current position is displayed in real-time")

print("\n4. TEXT EDITING:")
print("   - Select a text element")
print("   - Edit text in the 'Text Editing' section")
print("   - Use quick presets or type custom text")
print("   - Changes apply immediately")

print("\n5. VISUAL PROPERTIES:")
print("   - Color picker: Change text color with RGBA controls")
print("   - Anchor selection: Choose from 9 anchor points")
print("   - Drag toggle: Enable/disable drag functionality")

print("\n=== Advanced Features ===")
print("- Multi-canvas support: Switch between different canvases")
print("- Element properties: Real-time property editing")
print("- Input integration: Uses your InputSystem for smooth controls")
print("- Visual feedback: Selected elements are clearly highlighted")

print("\n=== Test Scenarios ===")
print("Try these interactions:")
print("1. Select 'Main Title' and move it with WASD")
print("2. Edit the footer text to something custom")
print("3. Change the color of menu items")
print("4. Switch anchors to see positioning changes")
print("5. Use Shift+WASD for fast movement")
print("6. Try the manual movement buttons")

-- Función para crear elementos adicionales dinámicamente
function CreateTestElement(text, x, y, anchor)
    local newElement = canvas:MakeNewText(text or "New Test Element")
    newElement:setAnchor(anchor or Anchor.Center)
    newElement:setPosition(x or 500, y or 500)
    newElement:enableDrag(true)
    
    print("Created new test element: '" .. (text or "New Test Element") .. "' at (" .. (x or 500) .. ", " .. (y or 500) .. ")")
    return newElement
end

-- Función para crear un layout de menú completo
function CreateMenuLayout()
    print("\n=== Creating Complete Menu Layout ===")
    
    -- Limpiar elementos existentes (opcional)
    -- canvas:ClearElements() -- Si esta función existe
    
    -- Header
    local header = CreateTestElement("GAME TITLE", 960, 100, Anchor.TopCenter)
    
    -- Menú principal
    local startBtn = CreateTestElement("START GAME", 960, 300, Anchor.Center)
    local settingsBtn = CreateTestElement("SETTINGS", 960, 350, Anchor.Center)
    local creditsBtn = CreateTestElement("CREDITS", 960, 400, Anchor.Center)
    local exitBtn = CreateTestElement("EXIT", 960, 450, Anchor.Center)
    
    -- Footer
    local footer = CreateTestElement("Press any menu item to select", 960, 950, Anchor.BottomCenter)
    
    print("Menu layout created! Use Canvas Manager to edit each element.")
end

-- Función para simular texto dinámico
function UpdateDynamicStatus()
    if testElements[7] then
        local statuses = {
            "Dynamic Status: Ready",
            "Dynamic Status: Loading...",
            "Dynamic Status: Connected",
            "Dynamic Status: Processing",
            "Dynamic Status: Complete!"
        }
        
        local randomStatus = statuses[math.random(#statuses)]
        testElements[7].Text = randomStatus
        print("Updated status to: " .. randomStatus)
    end
end

print("\n=== Additional Functions Available ===")
print("- Call CreateTestElement(text, x, y, anchor) to create new elements")
print("- Call CreateMenuLayout() to create a complete menu")
print("- Call UpdateDynamicStatus() to change the status text")

print("\n=== Canvas Manager Editor is Ready! ===")
print("Open the Canvas Manager window and start editing your UI elements!")
print("All elements are created with drag enabled and ready for editing.")

-- Auto-actualizar el estado cada pocos segundos (simulación)
local updateTimer = 0
function UpdateCanvasManager(deltaTime)
    updateTimer = updateTimer + (deltaTime or 0.016)
    
    -- Actualizar estado cada 5 segundos
    if updateTimer > 5.0 then
        UpdateDynamicStatus()
        updateTimer = 0
    end
end

print("\nThe Canvas Manager Editor test is now active!")
print("Check the 'Canvas Manager' window in your editor interface.")

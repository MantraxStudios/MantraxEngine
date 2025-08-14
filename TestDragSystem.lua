-- Script para probar el sistema de arrastre de UI
print("=== Testing UI Drag System ===")

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
    print("Creating new canvas for drag testing...")
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

-- Crear elementos de texto arrastrables
print("\n=== Creating Draggable Text Elements ===")

local draggableTexts = {}

-- Texto 1: Draggable básico
draggableTexts[1] = canvas:MakeNewText("Drag Me! (Basic)")
draggableTexts[1]:setAnchor(Anchor.TopLeft)
draggableTexts[1]:setPosition(50, 50)
draggableTexts[1]:enableDrag(true)
print("Created basic draggable text")

-- Texto 2: Draggable con restricciones
draggableTexts[2] = canvas:MakeNewText("Drag Me! (Constrained)")
draggableTexts[2]:setAnchor(Anchor.TopLeft)
draggableTexts[2]:setPosition(50, 100)
draggableTexts[2]:enableDrag(true)
draggableTexts[2]:setDragConstraints(glm.vec2(10, 10), glm.vec2(500, 300))
print("Created constrained draggable text")

-- Texto 3: Draggable con callbacks
draggableTexts[3] = canvas:MakeNewText("Drag Me! (With Events)")
draggableTexts[3]:setAnchor(Anchor.TopLeft)
draggableTexts[3]:setPosition(50, 150)
draggableTexts[3]:enableDrag(true)

-- Configurar callbacks para el texto 3
draggableTexts[3]:setOnDragStart(function(event)
    print("Drag started on 'With Events' text at position: " .. event.startPosition.x .. ", " .. event.startPosition.y)
end)

draggableTexts[3]:setOnDragUpdate(function(event)
    print("Dragging 'With Events' text to: " .. event.currentPosition.x .. ", " .. event.currentPosition.y)
end)

draggableTexts[3]:setOnDragEnd(function(event)
    print("Drag ended on 'With Events' text at final position: " .. event.currentPosition.x .. ", " .. event.currentPosition.y)
end)

print("Created event-enabled draggable text")

-- Texto 4: No draggable (para comparación)
draggableTexts[4] = canvas:MakeNewText("I'm NOT Draggable")
draggableTexts[4]:setAnchor(Anchor.TopLeft)
draggableTexts[4]:setPosition(50, 200)
-- No llamamos enableDrag() - por defecto no es draggable
print("Created non-draggable text for comparison")

-- Texto 5: Draggable limitado al padre
draggableTexts[5] = canvas:MakeNewText("Drag Me! (Parent Bounds)")
draggableTexts[5]:setAnchor(Anchor.Center)
draggableTexts[5]:enableDrag(true)
draggableTexts[5]:setConstrainToParent(true)
print("Created parent-constrained draggable text")

print("\n=== Drag System Configuration ===")

-- Configurar el sistema de arrastre
-- Note: En C++, estas funciones serían llamadas automáticamente
-- Aquí las listamos para mostrar la funcionalidad disponible

print("Drag threshold: 5 pixels (default)")
print("All draggable elements registered with UIDragSystem")
print("Canvas linked to drag system for coordinate conversion")

print("\n=== Usage Instructions ===")
print("1. Click and drag on any 'Drag Me!' text to move it around")
print("2. The 'Basic' text can be dragged anywhere")
print("3. The 'Constrained' text is limited to a specific area")
print("4. The 'With Events' text prints debug messages when dragged")
print("5. The 'NOT Draggable' text cannot be moved")
print("6. The 'Parent Bounds' text is constrained to the canvas area")

print("\n=== Test Cases ===")
print("Try these interactions:")
print("- Drag different texts to test movement")
print("- Try to drag the non-draggable text (should not move)")
print("- Drag the constrained text beyond its limits")
print("- Watch console for drag event messages")

print("\n=== Drag System Test Setup Complete ===")
print("Canvas size: " .. canvas:getWidth() .. "x" .. canvas:getHeight())
print("Total UI elements: " .. #canvas.RenderElements)
print("Draggable elements created: " .. #draggableTexts)

-- Función helper para testear el sistema programáticamente
function TestDragProgrammatically()
    print("\n=== Programmatic Drag Test ===")
    
    -- Simular arrastre en el primer elemento
    local testElement = draggableTexts[1]
    if testElement then
        print("Moving first draggable text programmatically...")
        testElement:setPosition(200, 200)
        print("Text moved to (200, 200)")
    end
end

-- Función para deshabilitar/habilitar drag en tiempo real
function ToggleDragOnAllElements()
    print("\n=== Toggling Drag on All Elements ===")
    
    for i, text in ipairs(draggableTexts) do
        if text.isDragEnabled then -- Si tiene la función
            local isCurrentlyEnabled = text:isDragEnabled()
            text:enableDrag(not isCurrentlyEnabled)
            print("Element " .. i .. " drag " .. (isCurrentlyEnabled and "disabled" or "enabled"))
        end
    end
end

print("\n=== Additional Functions Available ===")
print("- Call TestDragProgrammatically() to test programmatic movement")
print("- Call ToggleDragOnAllElements() to toggle drag on all elements")
print("\nDrag system is now active! Try dragging the UI elements with your mouse.")

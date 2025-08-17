-- UI System Example
-- Demonstrates how to use the UI system with OpenGL

-- Global variables for UI elements
local uiRenderer = nil
local buttons = {}
local panels = {}

function OnStart()
    print("=== UI System Example Started ===")
    
    -- Initialize UI renderer (this would be done in C++)
    -- uiRenderer = UIRenderer.new(800, 600)
    
    -- Create some example UI elements
    createExampleUI()
    
    print("UI system initialized successfully!")
end

function OnTick(deltaTime)
    -- Update UI elements
    updateUI(deltaTime)
    
    -- Handle input (this would be done in C++)
    handleInput()
end

function createExampleUI()
    print("Creating example UI elements...")
    
    -- Create a main panel
    local mainPanel = {
        x = 50, y = 50, width = 300, height = 400,
        color = {0.1, 0.1, 0.1, 0.8},
        children = {}
    }
    
    -- Create buttons
    local button1 = {
        x = 20, y = 20, width = 120, height = 40,
        text = "Button 1",
        normalColor = {0.2, 0.2, 0.2, 1.0},
        hoverColor = {0.3, 0.3, 0.3, 1.0},
        pressedColor = {0.1, 0.1, 0.1, 1.0},
        onClick = function()
            print("Button 1 clicked!")
            -- Example: Move camera
            local camera = getActiveCamera()
            if camera then
                camera:setPosition(vector3.new(0, 5, 10))
                camera:setRotation(0, 0)
                print("Camera reset!")
            end
        end
    }
    
    local button2 = {
        x = 160, y = 20, width = 120, height = 40,
        text = "Button 2",
        normalColor = {0.2, 0.4, 0.2, 1.0},
        hoverColor = {0.3, 0.5, 0.3, 1.0},
        pressedColor = {0.1, 0.3, 0.1, 1.0},
        onClick = function()
            print("Button 2 clicked!")
            -- Example: Change camera position
            local camera = getActiveCamera()
            if camera then
                camera:setPosition(vector3.new(5, 3, 8))
                print("Camera moved!")
            end
        end
    }
    
    local button3 = {
        x = 20, y = 80, width = 260, height = 40,
        text = "Toggle Movement",
        normalColor = {0.4, 0.2, 0.2, 1.0},
        hoverColor = {0.5, 0.3, 0.3, 1.0},
        pressedColor = {0.3, 0.1, 0.1, 1.0},
        onClick = function()
            print("Toggle Movement clicked!")
            -- Example: Toggle continuous movement
            if not continuousMovement then
                continuousMovement = true
                print("Continuous movement enabled!")
            else
                continuousMovement = false
                print("Continuous movement disabled!")
            end
        end
    }
    
    -- Add buttons to panel
    table.insert(mainPanel.children, button1)
    table.insert(mainPanel.children, button2)
    table.insert(mainPanel.children, button3)
    
    -- Store UI elements
    table.insert(panels, mainPanel)
    table.insert(buttons, button1)
    table.insert(buttons, button2)
    table.insert(buttons, button3)
    
    print("Created " .. #buttons .. " buttons and " .. #panels .. " panels")
end

function updateUI(deltaTime)
    -- Update UI animations or effects
    for i, button in ipairs(buttons) do
        -- Example: Add hover effect
        if button.hovered then
            button.alpha = 0.8 + 0.2 * math.sin(Time.getTime() * 5)
        else
            button.alpha = 1.0
        end
    end
end

function handleInput()
    -- This would be handled by the C++ input system
    -- For now, just demonstrate the concept
    
    -- Example: Check for mouse clicks
    -- local mouseX, mouseY = getMousePosition()
    -- for i, button in ipairs(buttons) do
    --     if isPointInRect(mouseX, mouseY, button.x, button.y, button.width, button.height) then
    --         if not button.hovered then
    --             button.hovered = true
    --             print("Hovering over: " .. button.text)
    --         end
    --     else
    --         if button.hovered then
    --             button.hovered = false
    --         end
    --     end
    -- end
end

-- Utility functions
function isPointInRect(x, y, rectX, rectY, rectWidth, rectHeight)
    return x >= rectX and x <= rectX + rectWidth and
           y >= rectY and y <= rectY + rectHeight
end

-- Example functions that could be called from UI
function moveCameraForward()
    local camera = getActiveCamera()
    if camera then
        camera:moveForward(2.0)
        print("Camera moved forward")
    end
end

function moveCameraBackward()
    local camera = getActiveCamera()
    if camera then
        camera:moveForward(-2.0)
        print("Camera moved backward")
    end
end

function rotateCameraLeft()
    local camera = getActiveCamera()
    if camera then
        camera:rotate(-10, 0)
        print("Camera rotated left")
    end
end

function rotateCameraRight()
    local camera = getActiveCamera()
    if camera then
        camera:rotate(10, 0)
        print("Camera rotated right")
    end
end

-- Global variables for demo
continuousMovement = false

-- Demo functions
function demoUI()
    print("=== UI Demo Functions ===")
    print("Available functions:")
    print("- moveCameraForward()")
    print("- moveCameraBackward()")
    print("- rotateCameraLeft()")
    print("- rotateCameraRight()")
    print("- createExampleUI()")
end 
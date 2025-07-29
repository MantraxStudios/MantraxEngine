-- Example Lua script for ScriptExecutor component
-- This script demonstrates the basic functions that can be used

local ExampleScript = {}

-- Called when the script starts
function ExampleScript.OnInit()
    print("ExampleScript: OnInit() called")
    
    -- You can access the GameObject through the CoreWrapper
    -- For example, you could get the GameObject's position:
    -- local pos = GameObject.getPosition()
    -- print("GameObject position: " .. tostring(pos.x) .. ", " .. tostring(pos.y) .. ", " .. tostring(pos.z))
end

-- Called every frame
function ExampleScript.OnTick()
    -- This function is called every frame
    -- You can add your game logic here
    
    -- Example: Move the GameObject
    -- GameObject.moveForward(1.0)
    
    -- Example: Rotate the GameObject
    -- GameObject.rotate(0, 1, 0)
end

-- Called when the script is destroyed
function ExampleScript.onDestroy()
    print("ExampleScript: onDestroy() called")
end

-- Return the table to make it available to the ScriptExecutor
return ExampleScript 
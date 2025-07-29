-- Player Controller Lua script
-- This script demonstrates more advanced functionality

local PlayerController = {}

-- Variables to track player state
local isMoving = false
local moveSpeed = 5.0
local rotationSpeed = 2.0

-- Called when the script starts
function PlayerController.OnInit()
    print("PlayerController: OnInit() called")
    print("Player controller initialized with move speed: " .. moveSpeed)
    
    -- Initialize player state
    isMoving = false
end

-- Called every frame
function PlayerController.OnTick()
    -- Handle player input and movement
    -- This is where you would check for input and move the player
    
    -- Example: Check for movement input
    -- if InputSystem.isKeyPressed("W") then
    --     GameObject.moveForward(moveSpeed * deltaTime)
    --     isMoving = true
    -- elseif InputSystem.isKeyPressed("S") then
    --     GameObject.moveForward(-moveSpeed * deltaTime)
    --     isMoving = true
    -- else
    --     isMoving = false
    -- end
    
    -- Example: Handle rotation
    -- if InputSystem.isKeyPressed("A") then
    --     GameObject.rotate(0, -rotationSpeed * deltaTime, 0)
    -- elseif InputSystem.isKeyPressed("D") then
    --     GameObject.rotate(0, rotationSpeed * deltaTime, 0)
    -- end
    
    -- Example: Handle jumping
    -- if InputSystem.isKeyPressed("SPACE") and not isJumping then
    --     GameObject.addForce(0, 10.0, 0)
    --     isJumping = true
    -- end
end

-- Called when the script is destroyed
function PlayerController.onDestroy()
    print("PlayerController: onDestroy() called")
    print("Player controller cleanup completed")
end

-- Return the table to make it available to the ScriptExecutor
return PlayerController 
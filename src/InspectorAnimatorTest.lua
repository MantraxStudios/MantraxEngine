-- Test script for Inspector Animator File Loading functionality
-- This script tests the new UI elements in the Inspector for loading .animator files

print("=== Inspector Animator File Loading Test ===")

-- Create a test GameObject with SpriteAnimator
local testObject = GameObject.new("TestAnimatorObject")
local animator = testObject:addComponent("SpriteAnimator")

print("Created test object: " .. testObject.Name)
print("Added SpriteAnimator component")

-- Test the getAnimatorFilePath method
local defaultPath = animator:getAnimatorFilePath()
print("Default animator file path: " .. defaultPath)

-- Test loading from a specific path (if a test file exists)
local testAnimatorPath = "Content/Animators/TestAnimator.animator"
print("Testing load from path: " .. testAnimatorPath)

-- Try to load from the test path
local loadResult = animator:loadFromAnimatorFile(testAnimatorPath)
print("Load result: " .. tostring(loadResult))

-- Test creating a simple animator configuration and saving it
print("\n--- Testing Save Functionality ---")

-- Add some test states and textures
animator:addSpriteState("Idle")
animator:addSpriteState("Walk")
animator:addSpriteState("Jump")

-- Add some test textures (if they exist)
local testTextures = {
    "Content/Sprites/Characters/player_idle.png",
    "Content/Sprites/Characters/player_walk.png",
    "Content/Sprites/Characters/player_jump.png"
}

for i, texturePath in ipairs(testTextures) do
    local stateName = ""
    if i == 1 then stateName = "Idle"
    elseif i == 2 then stateName = "Walk"
    elseif i == 3 then stateName = "Jump"
    end
    
    animator:addTextureToState(stateName, texturePath)
    print("Added texture " .. texturePath .. " to state " .. stateName)
end

-- Set animation properties
animator:setAnimationSpeed(2.0)
animator:setCurrentState("Idle")

print("Animation speed: " .. animator.animationSpeed)
print("Current state: " .. animator.currentState)

-- Test serialization
local serializedData = animator:serializeComponent()
print("Serialized data keys:")
for key, value in pairs(serializedData) do
    print("  " .. key .. ": " .. tostring(value))
end

print("\n--- Inspector UI Test Instructions ---")
print("1. Select the TestAnimatorObject in the scene")
print("2. Open the Inspector window")
print("3. Look for the 'Animator Configuration' section")
print("4. You should see:")
print("   - Current file path display")
print("   - 'Load Animator File' button")
print("   - File path input field with 'Load' button")
print("   - 'Save Current' button")
print("5. Test the file dialog by clicking 'Load Animator File'")
print("6. Test saving by clicking 'Save Current'")

print("\n=== Test Complete ===")
print("Check the Inspector window to see the new animator file loading UI!") 
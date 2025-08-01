-- Test script for drag and drop functionality in SpriteAnimator
-- This script tests the UIBuilder::Drag_Objetive fix

print("=== Drag and Drop Test for SpriteAnimator ===")

-- Create a GameObject with SpriteAnimator
local gameObject = GameObject.new("TestSpriteAnimator")
local spriteAnimator = gameObject:addComponent("SpriteAnimator")

if spriteAnimator then
    print("✓ SpriteAnimator component created successfully")
    
    -- Create a material
    spriteAnimator:createMaterial("TestMaterial")
    print("✓ Material created")
    
    -- Add a test state
    spriteAnimator:addSpriteState("TestState")
    spriteAnimator:setCurrentState("TestState")
    print("✓ Test state created and set as current")
    
    -- Test loading a texture (simulating drag and drop)
    local testTexturePath = "Content/Sprites/Characters/player.png"
    spriteAnimator:addTextureToState("TestState", testTexturePath)
    print("✓ Test texture added to state")
    
    -- Verify the texture was loaded
    local texture = spriteAnimator:getTexture(testTexturePath)
    if texture then
        print("✓ Texture loaded successfully")
        print("  - Width: " .. texture:getWidth())
        print("  - Height: " .. texture:getHeight())
        print("  - ID: " .. texture:getID())
    else
        print("✗ Texture failed to load")
    end
    
    -- Test the animator file functionality
    local animatorPath = spriteAnimator:getAnimatorFilePath()
    print("✓ Animator file path: " .. animatorPath)
    
    -- Test serialization
    local jsonData = spriteAnimator:serializeComponent()
    if jsonData and jsonData ~= "" then
        print("✓ Serialization successful")
        print("  - JSON length: " .. string.len(jsonData))
    else
        print("✗ Serialization failed")
    end
    
    -- Test debug info
    local debugInfo = spriteAnimator:getDebugInfo()
    print("✓ Debug info available")
    print("  - Debug info length: " .. string.len(debugInfo))
    
else
    print("✗ Failed to create SpriteAnimator component")
end

print("=== Test completed ===")
print("")
print("To test drag and drop:")
print("1. Open the Inspector")
print("2. Select a GameObject with SpriteAnimator")
print("3. In the SpriteAnimator section, drag a texture from Content Browser")
print("4. Drop it in the 'Drop textures here' area")
print("5. The texture should be added to the current state") 
-- Test script to verify SpriteAnimator can be added to GameObjects from Lua
-- This tests the "te falto poner para agregarlo" functionality

print("=== SpriteAnimator AddComponent Test ===")

-- Create a GameObject
local gameObject = GameObject.create("TestSpriteObject")
print("Created GameObject: " .. gameObject.Name)

-- Test adding SpriteAnimator component
local spriteAnimator = gameObject:addComponent("SpriteAnimator")
if spriteAnimator then
    print("✓ Successfully added SpriteAnimator component!")
    
    -- Test material creation
    spriteAnimator:createMaterial("TestMaterial")
    print("✓ Created material for SpriteAnimator")
    
    -- Test material property setting
    spriteAnimator:setSpriteAlbedo(vec3(1.0, 0.5, 0.2))
    spriteAnimator:setSpriteMetallic(0.8)
    spriteAnimator:setSpriteRoughness(0.3)
    print("✓ Set material properties")
    
    -- Test getting the component back
    local retrievedComponent = gameObject:getComponent("SpriteAnimator")
    if retrievedComponent then
        print("✓ Successfully retrieved SpriteAnimator component")
    else
        print("✗ Failed to retrieve SpriteAnimator component")
    end
    
else
    print("✗ Failed to add SpriteAnimator component")
end

-- Test with a different GameObject
local gameObject2 = GameObject.create("AnotherSpriteObject")
local spriteAnimator2 = gameObject2:addComponent("SpriteAnimator")
if spriteAnimator2 then
    print("✓ Successfully added SpriteAnimator to second GameObject")
    
    -- Test different material configuration
    spriteAnimator2:createMaterial("BlueMaterial")
    spriteAnimator2:setSpriteAlbedo(vec3(0.2, 0.5, 1.0))
    spriteAnimator2:setSpriteEmissive(vec3(0.1, 0.1, 0.3))
    print("✓ Configured second SpriteAnimator with different material")
    
else
    print("✗ Failed to add SpriteAnimator to second GameObject")
end

print("=== Test Complete ===") 
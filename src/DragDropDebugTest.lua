-- DragDropDebugTest.lua
-- Test script to debug drag-and-drop functionality

print("=== Drag & Drop Debug Test ===")

-- Create a GameObject with SpriteAnimator
local gameObject = GameObject.new("DragDropTest")
local spriteAnimator = gameObject:addComponent("SpriteAnimator")

if spriteAnimator then
    print("✓ SpriteAnimator component created successfully")
    
    -- Test basic functionality
    print("Testing basic SpriteAnimator methods...")
    
    -- Test adding a state
    spriteAnimator:addSpriteState("TestState")
    print("✓ Added test state")
    
    -- Test setting current state
    spriteAnimator:setCurrentState("TestState")
    print("✓ Set current state")
    
    -- Test manual texture loading (this should work)
    print("Testing manual texture loading...")
    local testTexturePath = "Sprites/Characters/player.png"  -- Adjust path as needed
    spriteAnimator:loadTexture(testTexturePath)
    print("✓ Attempted to load texture: " .. testTexturePath)
    
    -- Test getting texture
    local texture = spriteAnimator:getTexture(testTexturePath)
    if texture then
        print("✓ Texture loaded successfully")
        print("  - Texture ID: " .. texture:getID())
        print("  - Width: " .. texture:getWidth())
        print("  - Height: " .. texture:getHeight())
    else
        print("✗ Texture failed to load")
    end
    
    -- Test adding texture to state
    spriteAnimator:addTextureToState("TestState", testTexturePath)
    print("✓ Added texture to state")
    
    -- Test updating material texture
    spriteAnimator:updateMaterialTexture()
    print("✓ Updated material texture")
    
    -- Test debug info
    local debugInfo = spriteAnimator:getDebugInfo()
    print("Debug Info:")
    print(debugInfo)
    
    print("\n=== Drag & Drop Instructions ===")
    print("1. Open the Inspector")
    print("2. Select this GameObject (DragDropTest)")
    print("3. In the SpriteAnimator section, try to drag a texture from Content Browser")
    print("4. If drag doesn't work, use the manual input field")
    print("5. Check the console for any error messages")
    
else
    print("✗ Failed to create SpriteAnimator component")
end

print("\n=== Test Complete ===") 
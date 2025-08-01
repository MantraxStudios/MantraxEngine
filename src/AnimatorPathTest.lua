-- AnimatorPathTest.lua
-- Test script to verify the animator file path includes Content directory

print("=== Animator Path Test ===")

-- Create a GameObject with SpriteAnimator
local gameObject = GameObject.new("PathTestObject")
local spriteAnimator = gameObject:addComponent("SpriteAnimator")

if spriteAnimator then
    print("✓ SpriteAnimator component created successfully")
    
    -- Test the getAnimatorFilePath method
    local animatorPath = spriteAnimator:getAnimatorFilePath()
    print("Animator file path: " .. animatorPath)
    
    -- Check if the path contains the correct structure
    if animatorPath:find("\\Content\\Animators\\") then
        print("✓ Path correctly includes \\Content\\Animators\\")
    else
        print("✗ Path does not include \\Content\\Animators\\")
    end
    
    -- Check if the path ends with .animator
    if animatorPath:find("%.animator$") then
        print("✓ Path correctly ends with .animator")
    else
        print("✗ Path does not end with .animator")
    end
    
    -- Test adding a state and saving
    spriteAnimator:addSpriteState("TestState")
    spriteAnimator:setCurrentState("TestState")
    
    -- Test loading a texture
    local testTexturePath = "Sprites/Characters/player.png"
    spriteAnimator:loadTexture(testTexturePath)
    spriteAnimator:addTextureToState("TestState", testTexturePath)
    
    print("\n=== Manual Testing Instructions ===")
    print("1. Open the Inspector")
    print("2. Select this GameObject (PathTestObject)")
    print("3. In the SpriteAnimator section, click 'Save Current'")
    print("4. Check that the file is saved in: projects\\Content\\Animators\\PathTestObject.animator")
    print("5. Try loading the saved file using 'Load Animator File'")
    
else
    print("✗ Failed to create SpriteAnimator component")
end

print("\n=== Test Complete ===") 
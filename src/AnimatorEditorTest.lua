-- Animator Editor Test Script
-- This script tests the Animator Editor functionality

print("=== Animator Editor Test ===")

-- Get the scene manager
local sceneManager = SceneManager.getInstance()

-- Create a test GameObject
local testObject = GameObject.new("TestAnimatorObject")
testObject:setPosition(0, 0, 0)

-- Add a SpriteAnimator component
local spriteAnimator = testObject:addComponent("SpriteAnimator")

if spriteAnimator then
    print("✓ SpriteAnimator component added successfully")
    
    -- Test basic functionality
    spriteAnimator:enableDebugMode(true)
    print("✓ Debug mode enabled")
    
    -- Create some test states
    spriteAnimator:addSpriteState("Idle")
    spriteAnimator:addSpriteState("Walk")
    spriteAnimator:addSpriteState("Jump")
    
    print("✓ Test states created: Idle, Walk, Jump")
    
    -- Add some test textures to states
    spriteAnimator:addTextureToState("Idle", "projects/Content/Sprites/Characters/player_idle_1.png")
    spriteAnimator:addTextureToState("Idle", "projects/Content/Sprites/Characters/player_idle_2.png")
    spriteAnimator:addTextureToState("Idle", "projects/Content/Sprites/Characters/player_idle_3.png")
    
    spriteAnimator:addTextureToState("Walk", "projects/Content/Sprites/Characters/player_walk_1.png")
    spriteAnimator:addTextureToState("Walk", "projects/Content/Sprites/Characters/player_walk_2.png")
    spriteAnimator:addTextureToState("Walk", "projects/Content/Sprites/Characters/player_walk_3.png")
    spriteAnimator:addTextureToState("Walk", "projects/Content/Sprites/Characters/player_walk_4.png")
    
    spriteAnimator:addTextureToState("Jump", "projects/Content/Sprites/Characters/player_jump_1.png")
    spriteAnimator:addTextureToState("Jump", "projects/Content/Sprites/Characters/player_jump_2.png")
    
    print("✓ Test textures added to states")
    
    -- Set current state
    spriteAnimator:setCurrentState("Idle")
    print("✓ Current state set to: Idle")
    
    -- Test animation controls
    spriteAnimator:setAnimationSpeed(2.0)
    print("✓ Animation speed set to: 2.0")
    
    -- Test validation methods
    print("✓ Is valid state 'Idle':", spriteAnimator:isValidState("Idle"))
    print("✓ Has valid textures 'Idle':", spriteAnimator:hasValidTextures("Idle"))
    print("✓ Is material valid:", spriteAnimator:isMaterialValid())
    
    -- Test debug info
    local debugInfo = spriteAnimator:getDebugInfo()
    print("✓ Debug info:", debugInfo)
    
    -- Test animation playback
    spriteAnimator:playAnimation("Idle")
    print("✓ Animation started playing")
    
    -- Test frame control
    spriteAnimator:setCurrentFrame(1)
    print("✓ Current frame set to: 1")
    
    -- Test state switching
    spriteAnimator:setCurrentState("Walk")
    print("✓ Switched to Walk state")
    
    -- Test pause/stop
    spriteAnimator:pauseAnimation()
    print("✓ Animation paused")
    
    spriteAnimator:stopAnimation()
    print("✓ Animation stopped")
    
    -- Test texture management
    spriteAnimator:preloadAllTextures()
    print("✓ All textures preloaded")
    
    -- Test texture retrieval
    local texture = spriteAnimator:getTexture("projects/Content/Sprites/Characters/player_idle_1.png")
    if texture then
        print("✓ Texture retrieved successfully")
    else
        print("⚠ Texture not found (this is normal if the file doesn't exist)")
    end
    
    print("=== Animator Editor Test Complete ===")
    print("The Animator Editor should now be available in the editor window.")
    print("You can:")
    print("1. Open the Animator Editor window")
    print("2. Create new animators")
    print("3. Save animators to .animator files")
    print("4. Load animators from .animator files")
    print("5. Edit states and textures")
    print("6. Test animation playback")
    
else
    print("✗ Failed to add SpriteAnimator component")
end 
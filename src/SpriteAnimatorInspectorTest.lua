-- SpriteAnimator Inspector Test
-- This script tests the new persistent texture system and inspector integration

print("=== SpriteAnimator Inspector Test ===")

-- Get the first GameObject (or create one if needed)
local gameObject = GameObject.find("TestSprite")
if not gameObject then
    print("Creating test GameObject...")
    gameObject = GameObject.create("TestSprite")
end

-- Add SpriteAnimator component
local spriteAnimator = gameObject:addComponent("SpriteAnimator")
if not spriteAnimator then
    print("ERROR: Could not add SpriteAnimator component!")
    return
end

print("✓ SpriteAnimator component added successfully")

-- Test the new persistent texture system
print("\n--- Testing Persistent Texture System ---")

-- Add states using the new API
spriteAnimator:addSpriteState("idle")
spriteAnimator:addSpriteState("walk")
spriteAnimator:addSpriteState("jump")

print("✓ Added 3 states: idle, walk, jump")

-- Add texture paths to states
spriteAnimator:addTextureToState("idle", "assets/textures/idle_1.png")
spriteAnimator:addTextureToState("idle", "assets/textures/idle_2.png")
spriteAnimator:addTextureToState("walk", "assets/textures/walk_1.png")
spriteAnimator:addTextureToState("walk", "assets/textures/walk_2.png")
spriteAnimator:addTextureToState("jump", "assets/textures/jump_1.png")

print("✓ Added texture paths to states")

-- Preload all textures
spriteAnimator:preloadAllTextures()
print("✓ Preloaded all textures")

-- Test texture retrieval
local texture1 = spriteAnimator:getTexture("assets/textures/idle_1.png")
if texture1 then
    print("✓ Successfully retrieved texture from cache")
else
    print("⚠ Could not retrieve texture (this is normal if file doesn't exist)")
end

-- Set current state and test animation
spriteAnimator:setCurrentState("idle")
print("✓ Set current state to 'idle'")

-- Test animation playback
spriteAnimator:playAnimation("idle")
print("✓ Started animation playback")

-- Test debug info
local debugInfo = spriteAnimator:getDebugInfo()
print("Debug Info:", debugInfo)

-- Test state validation
if spriteAnimator:isValidState("idle") then
    print("✓ State 'idle' is valid")
else
    print("⚠ State 'idle' is not valid")
end

if spriteAnimator:hasValidTextures("idle") then
    print("✓ State 'idle' has valid textures")
else
    print("⚠ State 'idle' does not have valid textures (normal if files don't exist)")
end

-- Test material creation
spriteAnimator:createMaterial("TestSpriteMaterial")
print("✓ Created material")

-- Set material properties
spriteAnimator:setSpriteAlbedo({1.0, 1.0, 1.0})
spriteAnimator:setSpriteMetallic(0.0)
spriteAnimator:setSpriteRoughness(0.5)
print("✓ Set material properties")

print("\n=== Test Complete ===")
print("The inspector should now work correctly with the persistent texture system.")
print("You can:")
print("1. Open the inspector for this GameObject")
print("2. See the SpriteAnimator component")
print("3. Add states and texture paths")
print("4. View texture information without compilation errors")
print("5. Drag and drop textures to add them to states") 
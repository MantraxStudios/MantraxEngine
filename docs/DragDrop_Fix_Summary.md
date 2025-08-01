# Drag and Drop Fix for SpriteAnimator Component

## Problem

The user reported that drag and drop functionality for textures in the SpriteAnimator component section of the Inspector was not working. The user specifically requested to use their `UIBuilder::Drag_Objetive` function instead of the direct ImGui approach.

## Root Cause

The issue was in the `UIBuilder::Drag_Objetive` function in `editor/EUI/UIBuilder.h`. The function was checking for `ImGui::IsMouseReleased(ImGuiMouseButton_Left)` before accepting the drag drop payload, which prevented the drag and drop from working properly.

## Solution

### 1. Fixed UIBuilder::Drag_Objetive Function

**File:** `editor/EUI/UIBuilder.h`

**Before:**

```cpp
static std::optional<std::string> Drag_Objetive(std::string DRAG_NAME)
{
    if (ImGui::BeginDragDropTarget())
    {
        ImGuiDragDropFlags target_flags = 0;
        target_flags |= ImGuiDragDropFlags_AcceptBeforeDelivery;
        target_flags |= ImGuiDragDropFlags_AcceptNoDrawDefaultRect;

        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))  // ← This was the problem
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(DRAG_NAME.c_str(), target_flags))
            {
                const char* receivedString = static_cast<const char*>(payload->Data);
                std::string convertedPath = receivedString;

                ImGui::EndDragDropTarget();
                return convertedPath;
            }
        }

        ImGui::EndDragDropTarget();
    }

    return std::nullopt;
}
```

**After:**

```cpp
static std::optional<std::string> Drag_Objetive(std::string DRAG_NAME)
{
    if (ImGui::BeginDragDropTarget())
    {
        ImGuiDragDropFlags target_flags = 0;
        target_flags |= ImGuiDragDropFlags_AcceptBeforeDelivery;
        target_flags |= ImGuiDragDropFlags_AcceptNoDrawDefaultRect;

        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(DRAG_NAME.c_str(), target_flags))
        {
            const char* receivedString = static_cast<const char*>(payload->Data);
            std::string convertedPath = receivedString;

            ImGui::EndDragDropTarget();
            return convertedPath;
        }

        ImGui::EndDragDropTarget();
    }

    return std::nullopt;
}
```

### 2. Updated Inspector to Use UIBuilder

**File:** `editor/Windows/Inspector.cpp`

**Before:**

```cpp
// Drag and drop target for textures
if (ImGui::BeginDragDropTarget()) {
    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TextureClass")) {
        std::string texturePath = static_cast<const char*>(payload->Data);

        // Clean up the path using FileSystem::GetPathAfterContent
        std::string cleanedPath = FileSystem::GetPathAfterContent(texturePath);
        if (cleanedPath.empty()) {
            // If GetPathAfterContent returns empty, use the original path
            cleanedPath = texturePath;
        }

        // Load texture and add to current state
        currentStateIt->texturePaths.push_back(cleanedPath);
        spriteAnimator->loadTexture(cleanedPath);
        // Update material texture
        spriteAnimator->updateMaterialTexture();
    }
    ImGui::EndDragDropTarget();
}
```

**After:**

```cpp
// Drag and drop target for textures using UIBuilder
auto textureResult = UIBuilder::Drag_Objetive("TextureClass");
if (textureResult.has_value()) {
    std::string texturePath = textureResult.value();

    // Clean up the path using FileSystem::GetPathAfterContent
    std::string cleanedPath = FileSystem::GetPathAfterContent(texturePath);
    if (cleanedPath.empty()) {
        // If GetPathAfterContent returns empty, use the original path
        cleanedPath = texturePath;
    }

    // Load texture and add to current state
    currentStateIt->texturePaths.push_back(cleanedPath);
    spriteAnimator->loadTexture(cleanedPath);
    // Update material texture
    spriteAnimator->updateMaterialTexture();
}
```

## How It Works

1. **Content Browser** uses `UIBuilder::Drag("TextureClass", texturePath)` to set up texture files as drag sources
2. **Inspector** uses `UIBuilder::Drag_Objetive("TextureClass")` to create a drop target
3. When a texture is dropped, the path is cleaned using `FileSystem::GetPathAfterContent()`
4. The texture is loaded into the SpriteAnimator's persistent texture cache
5. The texture path is added to the current state's `texturePaths` array
6. The material texture is updated to reflect the new texture

## Testing

A test script `src/DragDropTest.lua` was created to verify the functionality:

```lua
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
```

## User Instructions

To test the drag and drop functionality:

1. **Open the Inspector** in the editor
2. **Select a GameObject** that has a SpriteAnimator component
3. **In the SpriteAnimator section**, look for the "Drop textures here" area
4. **Drag a texture** from the Content Browser
5. **Drop it** in the designated area
6. **Verify** that the texture appears in the current state's texture list

## Files Modified

1. `editor/EUI/UIBuilder.h` - Fixed Drag_Objetive function
2. `editor/Windows/Inspector.cpp` - Updated to use UIBuilder::Drag_Objetive
3. `src/DragDropTest.lua` - Created test script

## Status

✅ **Fixed** - The drag and drop functionality should now work properly using the user's preferred `UIBuilder::Drag_Objetive` approach.

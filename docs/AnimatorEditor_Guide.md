# Animator Editor Guide

## Overview

The Animator Editor is a powerful tool for creating, editing, and managing SpriteAnimator configurations. It allows you to create complex sprite animations with multiple states and save them as `.animator` files for reuse across your projects.

## Features

### 1. Animator Management
- **Create New Animators**: Start with a blank animator and build from scratch
- **Load Existing Animators**: Open previously saved `.animator` files
- **Save Animators**: Save your work to `.animator` files with JSON format
- **Delete Animators**: Remove unwanted animators from the project

### 2. State Management
- **Add States**: Create animation states (e.g., "Idle", "Walk", "Jump")
- **Remove States**: Delete unwanted states
- **State Selection**: Choose which state to edit
- **State Properties**: View and edit state-specific properties

### 3. Texture Management
- **Add Textures**: Add texture files to animation states
- **Remove Textures**: Remove textures from states
- **Texture Preview**: See thumbnail previews of textures
- **Texture Validation**: Check if textures are properly loaded

### 4. Animation Controls
- **Play/Pause**: Control animation playback
- **Stop**: Stop animation and reset to first frame
- **Reset**: Reset to the first frame
- **Frame Slider**: Manually control current frame
- **Speed Control**: Adjust animation speed

### 5. Export/Import
- **Save to File**: Export animator to `.animator` JSON file
- **Load from File**: Import animator from `.animator` file
- **File Format**: Uses JSON format for easy editing and version control

## File Structure

### `.animator` File Format
```json
{
    "name": "PlayerAnimator",
    "animationSpeed": 1.0,
    "isPlaying": false,
    "currentFrame": 0,
    "currentState": "Idle",
    "playbackState": "None",
    "states": [
        {
            "name": "Idle",
            "texturePaths": [
                "projects/Content/Sprites/Characters/player_idle_1.png",
                "projects/Content/Sprites/Characters/player_idle_2.png",
                "projects/Content/Sprites/Characters/player_idle_3.png"
            ]
        },
        {
            "name": "Walk",
            "texturePaths": [
                "projects/Content/Sprites/Characters/player_walk_1.png",
                "projects/Content/Sprites/Characters/player_walk_2.png",
                "projects/Content/Sprites/Characters/player_walk_3.png",
                "projects/Content/Sprites/Characters/player_walk_4.png"
            ]
        }
    ]
}
```

### Directory Structure
```
projects/
└── Content/
    └── Animators/
        ├── PlayerAnimator.animator
        ├── EnemyAnimator.animator
        └── UIAnimator.animator
```

## Usage Instructions

### 1. Opening the Animator Editor
1. Launch the MantraxEngine editor
2. Look for the "Animator Editor" window in the editor interface
3. If not visible, it may be docked or minimized - check the window menu

### 2. Creating a New Animator
1. Click "Create New" in the Animator Editor
2. Enter a name for your animator in the "Name" field
3. The animator will be created with default settings

### 3. Adding States
1. Enter a state name in the "New State Name" field
2. Click "Add State"
3. The state will appear in the state list
4. Select the state to edit its properties

### 4. Adding Textures to States
1. Select a state from the state list
2. Enter the texture file path in the "Texture Path" field
3. Click "Add Texture"
4. The texture will be added to the selected state
5. You can add multiple textures to create frame-by-frame animation

### 5. Testing Animation
1. Select a state as the "Current State"
2. Use the animation controls:
   - **Play**: Start animation playback
   - **Pause**: Pause animation
   - **Stop**: Stop and reset animation
   - **Frame Slider**: Manually control frame position

### 6. Saving Your Work
1. Click "Save Animator" to save to the default location
2. Click "Export to File" to save to a specific location
3. Files are saved as `.animator` JSON files

### 7. Loading Animators
1. Click "Import from File" to load an existing `.animator` file
2. Enter the file path or use the file browser
3. The animator will be loaded with all its states and textures

## Integration with SpriteAnimator Component

The Animator Editor works seamlessly with the SpriteAnimator component:

### Loading Animator to GameObject
```lua
-- Create a GameObject
local gameObject = GameObject.new("AnimatedSprite")

-- Add SpriteAnimator component
local animator = gameObject:addComponent("SpriteAnimator")

-- Load animator from file
-- (This would be done through the editor or script)
```

### Runtime Animation Control
```lua
-- Play animation
animator:playAnimation("Walk")

-- Set animation speed
animator:setAnimationSpeed(2.0)

-- Switch states
animator:setCurrentState("Jump")

-- Check if playing
if animator:getIsPlaying() then
    print("Animation is playing")
end
```

## Best Practices

### 1. File Organization
- Keep all animator files in `projects/Content/Animators/`
- Use descriptive names for animator files
- Organize textures in logical folders

### 2. State Naming
- Use clear, descriptive state names (e.g., "Idle", "Walk", "Run", "Jump")
- Avoid spaces in state names
- Use consistent naming conventions

### 3. Texture Management
- Use relative paths from the project root
- Ensure texture files exist before adding them
- Use consistent texture sizes for smooth animation
- Optimize texture sizes for performance

### 4. Animation Design
- Plan your animation states before creating them
- Consider frame rate and animation speed
- Test animations in the editor before finalizing
- Use appropriate frame counts for smooth animation

## Troubleshooting

### Common Issues

1. **Textures not loading**
   - Check file paths are correct
   - Ensure texture files exist
   - Verify file format is supported

2. **Animation not playing**
   - Check if a state is selected
   - Verify textures are loaded
   - Check animation speed settings

3. **Editor not showing**
   - Check if Animator Editor window is docked
   - Look in the window menu
   - Restart the editor if needed

4. **Save/Load errors**
   - Check file permissions
   - Verify directory exists
   - Check JSON format is valid

### Debug Information
The Animator Editor provides debug information:
- Number of states
- Current frame
- Playing status
- Texture loading status
- Material validation

## Advanced Features

### 1. Texture Caching
The SpriteAnimator uses a persistent texture cache to improve performance:
- Textures are loaded once and reused
- Reduces memory usage and loading times
- Automatic texture management

### 2. Debug Mode
Enable debug mode for detailed information:
```lua
animator:enableDebugMode(true)
local debugInfo = animator:getDebugInfo()
print(debugInfo)
```

### 3. Validation Methods
Check animator integrity:
```lua
-- Check if state exists
if animator:isValidState("Walk") then
    print("Walk state is valid")
end

-- Check if textures are loaded
if animator:hasValidTextures("Walk") then
    print("Walk state has valid textures")
end

-- Check material status
if animator:isMaterialValid() then
    print("Material is valid")
end
```

## Future Enhancements

Planned features for future versions:
- Drag and drop texture import
- Animation timeline editor
- Keyframe-based animation
- Animation blending
- Export to different formats
- Animation preview window
- Batch texture import
- Animation templates 
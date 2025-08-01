# Inspector Animator File Loading Guide

## Overview

The Inspector now includes a new "Animator Configuration" section that allows you to load and save `.animator` files directly from the Inspector window. This provides a convenient way to manage SpriteAnimator configurations without needing to use the Animator Editor.

## Location in Inspector

When you select a GameObject with a `SpriteAnimator` component, you'll find the new section in the Inspector:

1. **Material Settings** (existing)
2. **Animator Configuration** (NEW) ← This is where you assign `.animator` files
3. **Material Preview** (existing)
4. **Sprite States** (existing)
5. **Animation Controls** (existing)

## Animator Configuration Section

### Current File Display

- Shows the path to the currently assigned `.animator` file
- Displays "No animator file assigned" if no file is loaded

### Load Animator File Button

- **Function**: Opens a file dialog to browse and select `.animator` files
- **Location**: Searches in `Content/Animators/` directory
- **Behavior**:
  - Automatically creates the `Content/Animators/` directory if it doesn't exist
  - Shows a file browser with only `.animator` files
  - Loads the selected file and applies the configuration to the SpriteAnimator
  - Shows success/error popup messages

### Manual File Path Input

- **Input Field**: Type the path to a `.animator` file
- **Load Button**: Loads the file from the specified path
- **Supports**: Both relative and absolute paths
- **Relative Paths**: Automatically converted to absolute paths relative to the project root

### Save Current Button

- **Function**: Saves the current SpriteAnimator configuration as a `.animator` file
- **Filename**: Uses the GameObject's name + `.animator` extension
- **Location**: Saves to `Content/Animators/` directory
- **Behavior**:
  - Creates the directory if it doesn't exist
  - Serializes all current states, textures, and settings
  - Shows success/error popup messages

## How to Use

### Loading an Existing Animator File

1. **Select** a GameObject with a SpriteAnimator component
2. **Open** the Inspector window
3. **Find** the "Animator Configuration" section
4. **Click** "Load Animator File" button
5. **Browse** to your `.animator` file in the file dialog
6. **Select** the file and click "Open"
7. **Verify** the configuration loaded successfully

### Creating and Saving a New Animator Configuration

1. **Select** a GameObject with a SpriteAnimator component
2. **Configure** the animator using the existing Inspector controls:
   - Add states in the "Sprite States" section
   - Add textures to states
   - Set animation properties
3. **Go to** the "Animator Configuration" section
4. **Click** "Save Current" button
5. **Verify** the file was saved successfully

### Using Manual File Path

1. **Type** the path to your `.animator` file in the input field
2. **Click** the "Load" button
3. **Verify** the configuration loaded successfully

## File Structure

The `.animator` files are JSON files that contain:

```json
{
  "animationSpeed": 1.0,
  "isPlaying": false,
  "currentFrame": 0,
  "currentState": "Idle",
  "playbackState": "Stopped",
  "SpriteStates": [
    {
      "state_name": "Idle",
      "texturePaths": ["Content/Sprites/player_idle.png"]
    }
  ]
}
```

## Error Handling

The Inspector provides feedback through popup messages:

- **"Animator configuration loaded successfully!"** - File loaded correctly
- **"Error loading animator configuration!"** - File couldn't be loaded
- **"Animator configuration saved successfully!"** - File saved correctly
- **"Error saving animator configuration!"** - File couldn't be saved

## Integration with Animator Editor

- **Animator Editor**: Use for creating and editing complex animator configurations
- **Inspector**: Use for quick loading and saving of configurations
- **Both tools** work with the same `.animator` file format
- **Files created** in one tool can be loaded in the other

## Tips

1. **Use the Animator Editor** for initial creation and complex editing
2. **Use the Inspector** for quick loading and saving during development
3. **Keep your `.animator` files** in the `Content/Animators/` directory for consistency
4. **Use descriptive GameObject names** as they become the default filename when saving
5. **Test your configurations** by loading them back to ensure they work correctly

## Troubleshooting

### File Not Found

- Ensure the `.animator` file exists at the specified path
- Check that the file has the correct `.animator` extension
- Verify the file contains valid JSON data

### Load Errors

- Check that all texture paths in the `.animator` file are valid
- Ensure texture files exist in the specified locations
- Verify the JSON structure is correct

### Save Errors

- Check write permissions for the `Content/Animators/` directory
- Ensure the GameObject has a valid name
- Verify there's enough disk space

## Example Workflow

1. **Create** a new GameObject with SpriteAnimator
2. **Configure** the animator with states and textures using the Inspector
3. **Save** the configuration using "Save Current" button
4. **Test** by loading the saved file back
5. **Iterate** and refine the configuration as needed
6. **Use** the Animator Editor for more complex modifications

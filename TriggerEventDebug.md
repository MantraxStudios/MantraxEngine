# Trigger Event Debug

## Problem

The `PhysicsEventCallback` is not being executed, but the `customFilterShader` is working correctly. This indicates that the PhysX event system is not properly configured or the triggers are not set up correctly.

## Debug Information Added

### 1. PhysicsManager Debug

- Added `verifyTriggerConfiguration()` call in `initialize()`
- Added frame counter in `update()` method
- Added `debugTriggerSetup()` method to verify trigger configuration

### 2. PhysicalObject Debug

- Added call to `debugTriggerSetup()` when setting trigger state
- This will verify that the actor and shapes are configured correctly

### 3. PhysicsEventCallback Debug

- Already has debug logs in `onTrigger()` method
- Should show when trigger events are detected

## Expected Behavior

### When Trigger is Set

1. `setTrigger()` should be called from inspector
2. `debugTriggerSetup()` should show actor configuration
3. `customFilterShader` should detect trigger collision
4. `onTrigger()` should be called in `PhysicsEventCallback`
5. Debug logs should appear in console

### Console Output Expected

```
=== DEBUG TRIGGER SETUP ===
Actor: 0x12345678
Is Trigger: YES
Actor Type: STATIC
Number of shapes: 1
Shape 0:
  TRIGGER_SHAPE flag: YES
  SIMULATION_SHAPE flag: NO
  Filter Word0: 4
  Filter Word1: 2
  Filter Word2: 1
==========================

=== FILTER SHADER DEBUG ===
Object 0 - Group: 4, Mask: 2, Trigger: true
Object 1 - Group: 2, Mask: 4, Trigger: false
TRIGGER COLLISION DETECTED - Setting trigger flags
==========================

=== TRIGGER EVENT DETECTED ===
Number of trigger pairs: 1
Trigger Pair 0:
  Trigger Actor: 0x12345678
  Other Actor: 0x87654321
  Status: 1
  Event Type: ENTER
=== END TRIGGER EVENT ===
```

## Troubleshooting Steps

### Step 1: Check Trigger Configuration

1. Set a trigger in the inspector
2. Look for `=== DEBUG TRIGGER SETUP ===` output
3. Verify that `TRIGGER_SHAPE flag: YES` and `SIMULATION_SHAPE flag: NO`

### Step 2: Check Filter Shader

1. Move objects to trigger collision
2. Look for `=== FILTER SHADER DEBUG ===` output
3. Verify that trigger collision is detected

### Step 3: Check Event Callback

1. If filter shader works but no trigger events
2. Check if `=== TRIGGER EVENT DETECTED ===` appears
3. If not, the issue is in the event callback registration

### Step 4: Check Scene Configuration

1. Look for `=== SCENE TRIGGER CONFIGURATION ===` output
2. Verify that `Simulation Event Callback: CONFIGURED`

## Possible Issues

### Issue 1: Trigger Flags Not Set

If `TRIGGER_SHAPE flag: NO`, the shape is not configured as trigger.

**Solution**: Check `setTrigger()` method and shape recreation.

### Issue 2: Filter Data Incorrect

If `Filter Word2: 0`, the trigger flag is not set in filter data.

**Solution**: Check `setupCollisionFilter()` method.

### Issue 3: Event Callback Not Registered

If no trigger events appear, the callback might not be registered.

**Solution**: Check scene configuration and callback registration.

### Issue 4: Objects Not Colliding

If filter shader doesn't detect collision, objects might not be in the same layer.

**Solution**: Check collision layers and masks.

## Code Changes Made

### PhysicsManager.cpp

```cpp
// Added in initialize()
verifyTriggerConfiguration();

// Added in update()
static int frameCount = 0;
frameCount++;
if (frameCount % 60 == 0) {
    std::cout << "Physics simulation frame: " << frameCount << std::endl;
}

// Added debug method
void PhysicsManager::debugTriggerSetup(physx::PxActor* actor, bool isTrigger) {
    // Debug implementation
}
```

### PhysicalObject.cpp

```cpp
// Added in setTrigger()
if (rigidActor) {
    auto& physicsManager = PhysicsManager::getInstance();
    physicsManager.debugTriggerSetup(rigidActor, isTriggerShape);
}
```

## Next Steps

1. **Test the debug version** to identify where the issue occurs
2. **Check console output** for each step of the process
3. **Verify trigger configuration** is correct
4. **Check if events are being generated** by PhysX
5. **Verify callback registration** is working

## Expected Result

After applying these debug changes, you should see:

- ✅ Trigger setup debug information
- ✅ Filter shader detecting trigger collisions
- ✅ Trigger events being generated
- ✅ Event callback being executed
- ✅ Trigger callbacks being called

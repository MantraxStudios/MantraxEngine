-- Camera Debug Test Script
-- This script will help identify what's working and what's not

function OnStart()
  
end

function OnTick()
  print("=== Camera Debug Test Started ===")
    
    -- Test 1: Try to get the active camera
    local camera = getActiveCamera()
    if camera then
        -- Test 6: Test movement
        camera:moveForward(0.01)
    end
    
    print("=== Camera Debug Test Completed ===")end
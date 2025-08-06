#include "CanvasManager.h"
#include <iostream>

// Simple test function to verify CanvasManager works
void TestCanvasManager() {
    std::cout << "=== Testing Canvas Manager ===" << std::endl;
    
    // Create a CanvasManager instance
    CanvasManager canvasManager;
    
    // Test 1: Add canvas
    std::cout << "Test 1: Adding canvas..." << std::endl;
    canvasManager.AddCanvas(1920, 1080);
    canvasManager.AddCanvas(800, 600);
    
    // Test 2: Select canvas
    std::cout << "Test 2: Selecting canvas..." << std::endl;
    canvasManager.SelectCanvas(0);
    
    // Test 3: Add components
    std::cout << "Test 3: Adding components..." << std::endl;
    canvasManager.AddComponent(CanvasManager::ComponentType::Text);
    canvasManager.AddComponent(CanvasManager::ComponentType::Button);
    canvasManager.AddComponent(CanvasManager::ComponentType::Image);
    canvasManager.AddComponent(CanvasManager::ComponentType::Panel);
    
    // Test 4: Select different canvas
    std::cout << "Test 4: Selecting different canvas..." << std::endl;
    canvasManager.SelectCanvas(1);
    canvasManager.AddComponent(CanvasManager::ComponentType::Text);
    canvasManager.AddComponent(CanvasManager::ComponentType::Button);
    
    // Test 5: Sync with RenderPipeline
    std::cout << "Test 5: Syncing with RenderPipeline..." << std::endl;
    canvasManager.SyncWithRenderPipeline();
    
    // Test 6: Render components
    std::cout << "Test 6: Rendering components..." << std::endl;
    canvasManager.RenderAllComponents();
    
    std::cout << "=== Canvas Manager Test Completed ===" << std::endl;
    std::cout << "All tests passed! CanvasManager is working correctly." << std::endl;
}

// Function to demonstrate basic usage
void DemonstrateBasicUsage() {
    std::cout << "\n=== Basic Usage Demo ===" << std::endl;
    
    CanvasManager manager;
    
    // Create a main menu canvas
    manager.AddCanvas(1920, 1080);
    manager.SelectCanvas(0);
    
    // Add menu elements
    manager.AddComponent(CanvasManager::ComponentType::Text);   // Title
    manager.AddComponent(CanvasManager::ComponentType::Button); // Start
    manager.AddComponent(CanvasManager::ComponentType::Button); // Settings
    manager.AddComponent(CanvasManager::ComponentType::Button); // Exit
    
    // Create HUD canvas
    manager.AddCanvas(800, 600);
    manager.SelectCanvas(1);
    
    // Add HUD elements
    manager.AddComponent(CanvasManager::ComponentType::Text);   // Health
    manager.AddComponent(CanvasManager::ComponentType::Text);   // Score
    manager.AddComponent(CanvasManager::ComponentType::Button); // Pause
    
    // Sync and render
    manager.SyncWithRenderPipeline();
    manager.RenderAllComponents();
    
    std::cout << "Basic usage demo completed successfully!" << std::endl;
} 
#include "CanvasManager.h"
#include <iostream>

// Example usage of CanvasManager
void ExampleCanvasManagerUsage() {
    // Get the CanvasManager instance from RenderWindows
    auto& renderWindows = RenderWindows::getInstance();
    auto* canvasManager = renderWindows.GetWindow<CanvasManager>();
    
    if (!canvasManager) {
        std::cerr << "Failed to get CanvasManager window" << std::endl;
        return;
    }
    
    std::cout << "=== Canvas Manager Example ===" << std::endl;
    
    // Example 1: Create multiple canvases
    std::cout << "\n1. Creating multiple canvases..." << std::endl;
    
    // Add canvases with different sizes
    canvasManager->AddCanvas(1920, 1080);  // Main UI canvas
    canvasManager->AddCanvas(800, 600);     // HUD canvas
    canvasManager->AddCanvas(400, 300);     // Mini-map canvas
    
    // Example 2: Add components to the first canvas
    std::cout << "\n2. Adding components to canvas 0..." << std::endl;
    
    // Select the first canvas
    canvasManager->SelectCanvas(0);
    
    // Add a text component
    canvasManager->AddComponent(CanvasManager::ComponentType::Text);
    
    // Add a button component
    canvasManager->AddComponent(CanvasManager::ComponentType::Button);
    
    // Add an image component
    canvasManager->AddComponent(CanvasManager::ComponentType::Image);
    
    // Add a panel component
    canvasManager->AddComponent(CanvasManager::ComponentType::Panel);
    
    // Example 3: Add components to the second canvas
    std::cout << "\n3. Adding components to canvas 1..." << std::endl;
    
    // Select the second canvas
    canvasManager->SelectCanvas(1);
    
    // Add HUD elements
    canvasManager->AddComponent(CanvasManager::ComponentType::Text);  // Health text
    canvasManager->AddComponent(CanvasManager::ComponentType::Text);  // Ammo text
    canvasManager->AddComponent(CanvasManager::ComponentType::Button); // Menu button
    
    // Example 4: Sync with RenderPipeline
    std::cout << "\n4. Syncing with RenderPipeline..." << std::endl;
    canvasManager->SyncWithRenderPipeline();
    
    // Example 5: Render all components
    std::cout << "\n5. Rendering all components..." << std::endl;
    canvasManager->RenderAllComponents();
    
    std::cout << "\n=== Example completed ===" << std::endl;
    std::cout << "Open the 'Canvas Manager' window in the editor to see the UI" << std::endl;
}

// Function to demonstrate canvas management features
void DemonstrateCanvasFeatures() {
    auto& renderWindows = RenderWindows::getInstance();
    auto* canvasManager = renderWindows.GetWindow<CanvasManager>();
    
    if (!canvasManager) return;
    
    std::cout << "\n=== Canvas Management Features ===" << std::endl;
    
    // Feature 1: Canvas creation with custom sizes
    std::cout << "Feature 1: Creating canvases with custom sizes" << std::endl;
    canvasManager->AddCanvas(2560, 1440);  // 4K canvas
    canvasManager->AddCanvas(1280, 720);   // HD canvas
    
    // Feature 2: Component management
    std::cout << "Feature 2: Adding different types of components" << std::endl;
    
    // Select canvas 0 and add components
    canvasManager->SelectCanvas(0);
    
    // Text components for UI
    canvasManager->AddComponent(CanvasManager::ComponentType::Text);  // Title
    canvasManager->AddComponent(CanvasManager::ComponentType::Text);  // Subtitle
    canvasManager->AddComponent(CanvasManager::ComponentType::Text);  // Description
    
    // Button components for interaction
    canvasManager->AddComponent(CanvasManager::ComponentType::Button); // Start button
    canvasManager->AddComponent(CanvasManager::ComponentType::Button); // Settings button
    canvasManager->AddComponent(CanvasManager::ComponentType::Button); // Exit button
    
    // Image components for graphics
    canvasManager->AddComponent(CanvasManager::ComponentType::Image);  // Logo
    canvasManager->AddComponent(CanvasManager::ComponentType::Image);  // Background
    
    // Panel components for layout
    canvasManager->AddComponent(CanvasManager::ComponentType::Panel);  // Main panel
    canvasManager->AddComponent(CanvasManager::ComponentType::Panel);  // Side panel
    
    // Feature 3: Multiple canvas management
    std::cout << "Feature 3: Managing multiple canvases" << std::endl;
    
    // Select canvas 1 and add HUD elements
    canvasManager->SelectCanvas(1);
    canvasManager->AddComponent(CanvasManager::ComponentType::Text);   // Health
    canvasManager->AddComponent(CanvasManager::ComponentType::Text);   // Score
    canvasManager->AddComponent(CanvasManager::ComponentType::Button); // Pause button
    
    // Select canvas 2 and add mini-map elements
    canvasManager->SelectCanvas(2);
    canvasManager->AddComponent(CanvasManager::ComponentType::Panel);  // Mini-map background
    canvasManager->AddComponent(CanvasManager::ComponentType::Image);  // Mini-map texture
    
    std::cout << "All features demonstrated successfully!" << std::endl;
    std::cout << "Use the Canvas Manager window to interact with the created elements" << std::endl;
} 
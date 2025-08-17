#pragma once
#include <glm/glm.hpp>
#include <functional>
#include <vector>
#include <memory>
#include "../core/CoreExporter.h"

// Forward declarations
class UIBehaviour;
class Canvas2D;

// Estados del drag
enum class MANTRAXCORE_API DragState {
    None,           // No está siendo arrastrado
    Hover,          // Mouse encima pero no arrastrándo
    Dragging,       // Siendo arrastrado actualmente
    Dropped         // Acabado de soltar
};

// Información del evento de drag
struct MANTRAXCORE_API UIDragEvent {
    UIBehaviour* element;       // Elemento siendo arrastrado
    glm::vec2 startPosition;    // Posición inicial del drag
    glm::vec2 currentPosition;  // Posición actual
    glm::vec2 deltaPosition;    // Delta desde la última actualización
    glm::vec2 offset;           // Offset desde el punto donde se hizo clic
    DragState state;            // Estado actual del drag
    bool consumed = false;      // Si el evento fue consumido
};

// Callbacks para drag events
using DragStartCallback = std::function<void(const UIDragEvent&)>;
using DragUpdateCallback = std::function<void(const UIDragEvent&)>;
using DragEndCallback = std::function<void(const UIDragEvent&)>;
using DragHoverCallback = std::function<void(const UIDragEvent&)>;

// Información de un elemento draggable
struct MANTRAXCORE_API DraggableInfo {
    UIBehaviour* element = nullptr;
    bool isDraggable = true;
    bool constrainToParent = true;
    glm::vec2 minPosition = glm::vec2(-10000.0f);
    glm::vec2 maxPosition = glm::vec2(10000.0f);
    
    DragStartCallback onDragStart;
    DragUpdateCallback onDragUpdate;
    DragEndCallback onDragEnd;
    DragHoverCallback onDragHover;
};

class MANTRAXCORE_API UIDragSystem {
public:
    static UIDragSystem& getInstance();
    
    // Registrar/desregistrar elementos
    void registerDraggable(UIBehaviour* element, const DraggableInfo& info);
    void unregisterDraggable(UIBehaviour* element);
    void updateDraggableInfo(UIBehaviour* element, const DraggableInfo& info);
    
    // Configurar límites de arrastre
    void setDragConstraints(UIBehaviour* element, const glm::vec2& minPos, const glm::vec2& maxPos);
    void setConstrainToParent(UIBehaviour* element, bool constrain);
    
    // Métodos principales del sistema
    void update(float deltaTime);
    void handleMouseInput(const glm::vec2& mousePos, bool mousePressed, bool mouseReleased);
    
    // Estado del drag actual
    UIBehaviour* getCurrentDragTarget() const { return currentDragTarget; }
    DragState getCurrentDragState() const { return currentDragState; }
    bool isDragging() const { return currentDragState == DragState::Dragging; }
    
    // Configuración global
    void setDragThreshold(float threshold) { dragThreshold = threshold; }
    float getDragThreshold() const { return dragThreshold; }
    
    // Canvas asociado (para conversiones de coordenadas)
    void setCanvas(Canvas2D* canvas) { this->canvas = canvas; }
    Canvas2D* getCanvas() const { return canvas; }
    
private:
    UIDragSystem() = default;
    ~UIDragSystem() = default;
    UIDragSystem(const UIDragSystem&) = delete;
    UIDragSystem& operator=(const UIDragSystem&) = delete;
    
    // Estado interno
    std::vector<DraggableInfo> draggableElements;
    UIBehaviour* currentDragTarget = nullptr;
    UIBehaviour* currentHoverTarget = nullptr;
    DragState currentDragState = DragState::None;
    
    glm::vec2 dragStartPosition = glm::vec2(0.0f);
    glm::vec2 lastMousePosition = glm::vec2(0.0f);
    glm::vec2 dragOffset = glm::vec2(0.0f);
    
    float dragThreshold = 5.0f;  // Píxeles que debe moverse para considerar drag
    Canvas2D* canvas = nullptr;
    
    // Métodos internos
    DraggableInfo* getDraggableInfo(UIBehaviour* element);
    UIBehaviour* getElementUnderMouse(const glm::vec2& mousePos);
    bool isPointInElement(const glm::vec2& point, UIBehaviour* element);
    glm::vec2 constrainPosition(const glm::vec2& position, const DraggableInfo& info);
    void updateDragState(const glm::vec2& mousePos, bool mousePressed, bool mouseReleased);
    void fireDragEvent(const UIDragEvent& event, const DraggableInfo& info);
};

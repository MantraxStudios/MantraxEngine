#include "UIDragSystem.h"
#include "UIBehaviour.h"
#include "Canvas.h"
#include "../components/EventSystem.h"
#include <algorithm>
#include <iostream>
#include <SDL.h>

UIDragSystem& UIDragSystem::getInstance() {
    static UIDragSystem instance;
    return instance;
}

void UIDragSystem::registerDraggable(UIBehaviour* element, const DraggableInfo& info) {
    if (!element) return;
    
    // Verificar si ya está registrado
    auto it = std::find_if(draggableElements.begin(), draggableElements.end(),
        [element](const DraggableInfo& info) { return info.element == element; });
    
    if (it != draggableElements.end()) {
        // Actualizar info existente
        *it = info;
        it->element = element; // Asegurar que el elemento esté correcto
    } else {
        // Agregar nuevo
        DraggableInfo newInfo = info;
        newInfo.element = element;
        draggableElements.push_back(newInfo);
    }
    
    std::cout << "[UIDragSystem] Registered draggable element" << std::endl;
}

void UIDragSystem::unregisterDraggable(UIBehaviour* element) {
    if (!element) return;
    
    auto it = std::remove_if(draggableElements.begin(), draggableElements.end(),
        [element](const DraggableInfo& info) { return info.element == element; });
    
    if (it != draggableElements.end()) {
        draggableElements.erase(it, draggableElements.end());
        
        // Si era el elemento siendo arrastrado, cancelar drag
        if (currentDragTarget == element) {
            currentDragTarget = nullptr;
            currentDragState = DragState::None;
        }
        if (currentHoverTarget == element) {
            currentHoverTarget = nullptr;
        }
        
        std::cout << "[UIDragSystem] Unregistered draggable element" << std::endl;
    }
}

void UIDragSystem::updateDraggableInfo(UIBehaviour* element, const DraggableInfo& info) {
    registerDraggable(element, info); // La misma lógica
}

void UIDragSystem::setDragConstraints(UIBehaviour* element, const glm::vec2& minPos, const glm::vec2& maxPos) {
    auto* info = getDraggableInfo(element);
    if (info) {
        info->minPosition = minPos;
        info->maxPosition = maxPos;
    }
}

void UIDragSystem::setConstrainToParent(UIBehaviour* element, bool constrain) {
    auto* info = getDraggableInfo(element);
    if (info) {
        info->constrainToParent = constrain;
    }
}

void UIDragSystem::update(float deltaTime) {
    // Obtener posición actual del mouse
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    glm::vec2 mousePos(static_cast<float>(mouseX), static_cast<float>(mouseY));
    
    // Convertir a coordenadas del canvas si es necesario
    if (canvas) {
        // Aquí podrías hacer conversiones de coordenadas específicas del canvas
        // Por ahora usamos coordenadas de pantalla directamente
    }
    
    // Obtener estado de los botones del mouse
    Uint32 mouseState = SDL_GetMouseState(nullptr, nullptr);
    bool mousePressed = (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
    
    // Detectar si se acaba de soltar
    static bool wasPressed = false;
    bool mouseReleased = wasPressed && !mousePressed;
    wasPressed = mousePressed;
    
    // Actualizar estado del drag
    updateDragState(mousePos, mousePressed, mouseReleased);
    
    lastMousePosition = mousePos;
}

void UIDragSystem::handleMouseInput(const glm::vec2& mousePos, bool mousePressed, bool mouseReleased) {
    updateDragState(mousePos, mousePressed, mouseReleased);
    lastMousePosition = mousePos;
}

DraggableInfo* UIDragSystem::getDraggableInfo(UIBehaviour* element) {
    auto it = std::find_if(draggableElements.begin(), draggableElements.end(),
        [element](const DraggableInfo& info) { return info.element == element; });
    
    return (it != draggableElements.end()) ? &(*it) : nullptr;
}

UIBehaviour* UIDragSystem::getElementUnderMouse(const glm::vec2& mousePos) {
    // Iterar en orden inverso para verificar elementos de arriba hacia abajo
    for (auto it = draggableElements.rbegin(); it != draggableElements.rend(); ++it) {
        if (it->isDraggable && isPointInElement(mousePos, it->element)) {
            return it->element;
        }
    }
    return nullptr;
}

bool UIDragSystem::isPointInElement(const glm::vec2& point, UIBehaviour* element) {
    if (!element) return false;
    
    // Obtener bounds del elemento basado en su tipo
    // Por ahora usamos un enfoque simple basado en UIText
    // Esto se puede expandir para otros tipos de UI
    
    // Convertir coordenadas de pantalla a coordenadas UI si es necesario
    glm::vec2 uiPoint = point;
    
    if (canvas) {
        // Aquí se haría la conversión de coordenadas apropiada
        // Por simplicidad, asumimos que las coordenadas ya están en espacio UI
    }
    
    // Para UIText, verificar si el punto está dentro del área de texto
    // Esto es una implementación básica - se puede mejorar
    float elementWidth = element->width * 0.1f;  // Estimación básica
    float elementHeight = 30.0f; // Altura básica de texto
    
    // Obtener posición del elemento (esto depende del tipo específico)
    glm::vec2 elementPos(0.0f); // Se necesitaría obtener la posición real del elemento
    
    return (uiPoint.x >= elementPos.x && uiPoint.x <= elementPos.x + elementWidth &&
            uiPoint.y >= elementPos.y && uiPoint.y <= elementPos.y + elementHeight);
}

glm::vec2 UIDragSystem::constrainPosition(const glm::vec2& position, const DraggableInfo& info) {
    glm::vec2 constrainedPos = position;
    
    // Aplicar límites mínimos y máximos
    constrainedPos.x = std::max(info.minPosition.x, std::min(info.maxPosition.x, constrainedPos.x));
    constrainedPos.y = std::max(info.minPosition.y, std::min(info.maxPosition.y, constrainedPos.y));
    
    // Aplicar restricción al padre si está habilitada
    if (info.constrainToParent && canvas) {
        constrainedPos.x = std::max(0.0f, std::min(canvas->getWidth() - 50.0f, constrainedPos.x));
        constrainedPos.y = std::max(0.0f, std::min(canvas->getHeight() - 30.0f, constrainedPos.y));
    }
    
    return constrainedPos;
}

void UIDragSystem::updateDragState(const glm::vec2& mousePos, bool mousePressed, bool mouseReleased) {
    glm::vec2 delta = mousePos - lastMousePosition;
    
    switch (currentDragState) {
        case DragState::None: {
            // Verificar hover
            UIBehaviour* elementUnderMouse = getElementUnderMouse(mousePos);
            
            if (elementUnderMouse != currentHoverTarget) {
                // Cambio de hover
                currentHoverTarget = elementUnderMouse;
                
                if (currentHoverTarget) {
                    currentDragState = DragState::Hover;
                    
                    // Disparar evento de hover
                    auto* info = getDraggableInfo(currentHoverTarget);
                    if (info) {
                        UIDragEvent event;
                        event.element = currentHoverTarget;
                        event.currentPosition = mousePos;
                        event.state = DragState::Hover;
                        fireDragEvent(event, *info);
                    }
                }
            }
            
            // Iniciar drag si se presiona el mouse sobre un elemento
            if (mousePressed && currentHoverTarget) {
                auto* info = getDraggableInfo(currentHoverTarget);
                if (info && info->isDraggable) {
                    currentDragTarget = currentHoverTarget;
                    dragStartPosition = mousePos;
                    dragOffset = glm::vec2(0.0f); // Calcular offset real aquí
                    currentDragState = DragState::Dragging;
                    
                    // Disparar evento de inicio de drag
                    UIDragEvent event;
                    event.element = currentDragTarget;
                    event.startPosition = dragStartPosition;
                    event.currentPosition = mousePos;
                    event.deltaPosition = delta;
                    event.offset = dragOffset;
                    event.state = DragState::Dragging;
                    fireDragEvent(event, *info);
                    
                    std::cout << "[UIDragSystem] Started dragging element" << std::endl;
                }
            }
            break;
        }
        
        case DragState::Hover: {
            // Verificar si aún está en hover
            UIBehaviour* elementUnderMouse = getElementUnderMouse(mousePos);
            if (elementUnderMouse != currentHoverTarget) {
                currentHoverTarget = nullptr;
                currentDragState = DragState::None;
            }
            
            // Iniciar drag si se presiona
            if (mousePressed && currentHoverTarget) {
                auto* info = getDraggableInfo(currentHoverTarget);
                if (info && info->isDraggable) {
                    currentDragTarget = currentHoverTarget;
                    dragStartPosition = mousePos;
                    currentDragState = DragState::Dragging;
                    
                    std::cout << "[UIDragSystem] Started dragging from hover" << std::endl;
                }
            }
            break;
        }
        
        case DragState::Dragging: {
            if (currentDragTarget) {
                auto* info = getDraggableInfo(currentDragTarget);
                if (info) {
                    // Actualizar posición
                    glm::vec2 newPosition = mousePos - dragOffset;
                    newPosition = constrainPosition(newPosition, *info);
                    
                    // Disparar evento de actualización
                    UIDragEvent event;
                    event.element = currentDragTarget;
                    event.startPosition = dragStartPosition;
                    event.currentPosition = newPosition;
                    event.deltaPosition = delta;
                    event.offset = dragOffset;
                    event.state = DragState::Dragging;
                    fireDragEvent(event, *info);
                }
            }
            
            // Terminar drag si se suelta el mouse
            if (mouseReleased) {
                if (currentDragTarget) {
                    auto* info = getDraggableInfo(currentDragTarget);
                    if (info) {
                        // Disparar evento de fin de drag
                        UIDragEvent event;
                        event.element = currentDragTarget;
                        event.startPosition = dragStartPosition;
                        event.currentPosition = mousePos;
                        event.deltaPosition = delta;
                        event.offset = dragOffset;
                        event.state = DragState::Dropped;
                        fireDragEvent(event, *info);
                        
                        std::cout << "[UIDragSystem] Ended dragging element" << std::endl;
                    }
                }
                
                currentDragTarget = nullptr;
                currentDragState = DragState::None;
            }
            break;
        }
        
        case DragState::Dropped: {
            // Estado transitorio, volver a None
            currentDragState = DragState::None;
            break;
        }
    }
}

void UIDragSystem::fireDragEvent(const UIDragEvent& event, const DraggableInfo& info) {
    switch (event.state) {
        case DragState::Hover:
            if (info.onDragHover) {
                info.onDragHover(event);
            }
            break;
        case DragState::Dragging:
            if (info.onDragStart && event.startPosition == event.currentPosition) {
                info.onDragStart(event);
            } else if (info.onDragUpdate) {
                info.onDragUpdate(event);
            }
            break;
        case DragState::Dropped:
            if (info.onDragEnd) {
                info.onDragEnd(event);
            }
            break;
        default:
            break;
    }
}

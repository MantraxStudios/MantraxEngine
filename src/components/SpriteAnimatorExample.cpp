#include "SpriteAnimator.h"
#include <glm/glm.hpp>

// Ejemplo de uso del SpriteAnimator con material propio
void ejemploUsoSpriteAnimator() {
    // Crear un SpriteAnimator
    SpriteAnimator* spriteAnimator = new SpriteAnimator();
    
    // El material se crea automáticamente en el constructor
    // Pero puedes crear uno personalizado si quieres
    spriteAnimator->createMaterial("MiSpriteMaterial");
    
    // Configurar propiedades del material
    spriteAnimator->setSpriteAlbedo(glm::vec3(1.0f, 1.0f, 1.0f)); // Color blanco
    spriteAnimator->setSpriteMetallic(0.0f); // No metálico
    spriteAnimator->setSpriteRoughness(0.3f); // Poco rugoso para sprites
    spriteAnimator->setSpriteEmissive(glm::vec3(0.1f, 0.1f, 0.1f)); // Ligera emisión
    spriteAnimator->setSpriteTiling(glm::vec2(1.0f, 1.0f)); // Sin tiling
    
    // Agregar estados de animación
    SpriteArray idleState;
    idleState.state_name = "Idle";
    // Aquí agregarías las texturas para el estado idle
    // idleState.texture.push_back(Texture("path/to/idle1.png"));
    // idleState.texture.push_back(Texture("path/to/idle2.png"));
    
    SpriteArray walkState;
    walkState.state_name = "Walk";
    // Aquí agregarías las texturas para el estado walk
    // walkState.texture.push_back(Texture("path/to/walk1.png"));
    // walkState.texture.push_back(Texture("path/to/walk2.png"));
    
    spriteAnimator->SpriteStates.push_back(idleState);
    spriteAnimator->SpriteStates.push_back(walkState);
    
    // Establecer el estado inicial
    spriteAnimator->currentState = "Idle";
    
    // Inicializar el componente
    spriteAnimator->start();
    
    // En el bucle de juego, el update() se llamará automáticamente
    // y actualizará la textura del material según el estado actual
    
    // Para cambiar de estado:
    // spriteAnimator->currentState = "Walk";
    // El material se actualizará automáticamente en el próximo update()
    
    // Para obtener el material y usarlo en el renderizado:
    auto material = spriteAnimator->getMaterial();
    if (material) {
        // Usar el material en el pipeline de renderizado
        material->bindTextures();
        // ... renderizar el sprite ...
        material->unbindTextures();
    }
    
    // Limpieza
    delete spriteAnimator;
}

// Ejemplo de cómo crear un material personalizado y asignarlo
void ejemploMaterialPersonalizado() {
    SpriteAnimator* spriteAnimator = new SpriteAnimator();
    
    // Crear un material personalizado
    auto materialPersonalizado = std::make_shared<Material>("SpriteMaterialPersonalizado");
    materialPersonalizado->setAlbedo(glm::vec3(0.8f, 0.8f, 1.0f)); // Color azul claro
    materialPersonalizado->setMetallic(0.1f);
    materialPersonalizado->setRoughness(0.2f);
    materialPersonalizado->setEmissive(glm::vec3(0.2f, 0.2f, 0.3f));
    
    // Asignar el material personalizado al SpriteAnimator
    spriteAnimator->setMaterial(materialPersonalizado);
    
    // El SpriteAnimator ahora usará este material personalizado
    // y lo actualizará automáticamente con las texturas de los sprites
    
    delete spriteAnimator;
} 
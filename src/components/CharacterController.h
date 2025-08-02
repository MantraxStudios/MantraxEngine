#pragma once
#include <physx/PxPhysicsAPI.h>
#include <physx/extensions/PxRigidBodyExt.h>
#include <physx/characterkinematic/PxController.h>
#include <physx/characterkinematic/PxCapsuleController.h>
#include <physx/characterkinematic/PxBoxController.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Component.h"
#include "../core/CoreExporter.h"
#include "../core/PhysicsManager.h"
#include <functional>

enum class MANTRAXCORE_API CharacterControllerType {
    Capsule,
    Box
};

enum class MANTRAXCORE_API MovementMode {
    Walking,
    Running,
    Crouching,
    Jumping,
    Flying
};

class MANTRAXCORE_API GameObject;

class MANTRAXCORE_API CharacterController : public Component {
private:
    std::string getComponentName() const override {
        return "Character Controller";
    }

    // PhysX Character Controller
    physx::PxController* controller;
    physx::PxCapsuleController* capsuleController;
    physx::PxBoxController* boxController;
    
    // Character properties
    CharacterControllerType controllerType;
    float height;
    float radius;
    float stepOffset;
    float slopeLimit;
    float contactOffset;
    
    // Movement properties
    float walkSpeed;
    float runSpeed;
    float crouchSpeed;
    float jumpForce;
    float gravity;
    float airControl;
    
    // Current state
    MovementMode currentMovementMode;
    bool isGrounded;
    bool isCrouching;
    bool isJumping;
    glm::vec3 velocity;
    glm::vec3 inputDirection;
    
    // Callbacks
    std::function<void(const glm::vec3&)> onMoveCallback;
    std::function<void(bool)> onJumpCallback;
    std::function<void(bool)> onRunCallback;
    std::function<void(bool)> onCrouchCallback;
    std::function<void(bool)> onGroundedCallback;
    
    // Internal state
    bool initialized;
    
    // Helper methods
    void updateMovement(float deltaTime);
    void updateGravity(float deltaTime);
    void syncTransformFromController();
    void syncTransformToController();

public:
    CharacterController() = default;
    CharacterController(GameObject* obj);
    ~CharacterController();
    
    // Component overrides
    void start() override;
    void update() override;
    void destroy() override;
    std::string serializeComponent() const override;
    void deserialize(const std::string& data) override;


    // Initialization
    void initializeController();
    
    // Movement methods
    void move(const glm::vec3& direction);
    void jump();
    void setMovementMode(MovementMode mode);
    
    // Character controller properties
    void setHeight(float newHeight);
    float getHeight() const { return height; }
    
    void setRadius(float newRadius);
    float getRadius() const { return radius; }
    
    void setStepOffset(float offset);
    float getStepOffset() const { return stepOffset; }
    
    void setSlopeLimit(float limit);
    float getSlopeLimit() const { return slopeLimit; }
    
    void setContactOffset(float offset);
    float getContactOffset() const { return contactOffset; }
    
    // Movement properties
    void setWalkSpeed(float speed);
    float getWalkSpeed() const { return walkSpeed; }
    
    void setRunSpeed(float speed);
    float getRunSpeed() const { return runSpeed; }
    
    void setCrouchSpeed(float speed);
    float getCrouchSpeed() const { return crouchSpeed; }
    
    void setJumpForce(float force);
    float getJumpForce() const { return jumpForce; }
    
    void setGravity(float gravityValue);
    float getGravity() const { return gravity; }
    
    void setAirControl(float control);
    float getAirControl() const { return airControl; }
    
    // State getters
    bool isGroundedState() const { return isGrounded; }
    bool isCrouchingState() const { return isCrouching; }
    bool isJumpingState() const { return isJumping; }
    MovementMode getMovementMode() const { return currentMovementMode; }
    glm::vec3 getVelocity() const { return velocity; }
    glm::vec3 getInputDirection() const { return inputDirection; }
    
    // Callback setters
    void setOnMoveCallback(std::function<void(const glm::vec3&)> callback) { onMoveCallback = callback; }
    void setOnJumpCallback(std::function<void(bool)> callback) { onJumpCallback = callback; }
    void setOnRunCallback(std::function<void(bool)> callback) { onRunCallback = callback; }
    void setOnCrouchCallback(std::function<void(bool)> callback) { onCrouchCallback = callback; }
    void setOnGroundedCallback(std::function<void(bool)> callback) { onGroundedCallback = callback; }
    
    // Utility methods
    bool isInitialized() const { return initialized; }
    physx::PxController* getController() const { return controller; }
    
    // Manual movement (for AI or external control)
    void setVelocity(const glm::vec3& newVelocity);
    void addForce(const glm::vec3& force);
    
    // Collision detection
    bool isOnGround() const;
    bool isOnSlope() const;
    float getSlopeAngle() const;
    
    // Teleportation
    void teleport(const glm::vec3& position);
    void teleport(const glm::vec3& position, const glm::quat& rotation);
}; 
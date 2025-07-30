#include "CharacterController.h"
#include "GameObject.h"
#include "../core/PhysicsManager.h"
#include "../core/Time.h"
#include <iostream>
#include <cmath>
#include <algorithm>

using json = nlohmann::json;

CharacterController::CharacterController(GameObject* obj) {
    setOwner(obj);

    // Initialize PhysX controller pointers
    controller = nullptr;
    capsuleController = nullptr;
    boxController = nullptr;

    // Initialize character properties
    controllerType = CharacterControllerType::Capsule;
    height = 2.0f;
    radius = 0.5f;
    stepOffset = 0.3f;
    slopeLimit = 45.0f; // In degrees, not radians
    contactOffset = 0.01f; // Must be smaller than radius

    // Initialize movement properties
    walkSpeed = 5.0f;
    runSpeed = 8.0f;
    crouchSpeed = 2.5f;
    jumpForce = 8.0f;
    gravity = -9.81f;
    airControl = 0.3f;

    // Initialize state
    currentMovementMode = MovementMode::Walking;
    isGrounded = false;
    isCrouching = false;
    isJumping = false;
    velocity = glm::vec3(0.0f);
    inputDirection = glm::vec3(0.0f);

    // Initialize internal state
    initialized = false; // Changed to false initially
}

CharacterController::~CharacterController() {
    destroy();
}

void CharacterController::start() {
    // Auto-initialize the controller when the component starts
    initializeController();
}

void CharacterController::update() {
    if (!initialized || !controller) return;

    float deltaTime = Time::getDeltaTime();

    // Update gravity first
    updateGravity(deltaTime);

    // Then update movement
    updateMovement(deltaTime);

    // Sync transform from controller to GameObject
    syncTransformFromController();
}

void CharacterController::destroy() {
    if (controller) {
        controller->release();
        controller = nullptr;
        capsuleController = nullptr;
        boxController = nullptr;
    }

    initialized = false;
}

void CharacterController::initializeController() {
    if (initialized) return;

    auto& physicsManager = PhysicsManager::getInstance();
    if (!physicsManager.getPhysics()) {
        std::cerr << "CharacterController::initializeController() - PhysicsManager not initialized!" << std::endl;
        return;
    }

    // Ensure we have an owner GameObject
    if (!owner) {
        std::cerr << "CharacterController::initializeController() - No owner GameObject!" << std::endl;
        return;
    }

    // Get transform from GameObject
    glm::vec3 position = owner->getWorldPosition();
    glm::quat rotation = owner->getWorldRotationQuat();

    // Create character controller manager
    physx::PxControllerManager* manager = physicsManager.getControllerManager();
    if (!manager) {
        std::cerr << "CharacterController::initializeController() - Controller manager not available!" << std::endl;
        return;
    }

    // Create controller based on type
    if (controllerType == CharacterControllerType::Capsule) {
        physx::PxCapsuleControllerDesc desc;

        // Create material directly
        physx::PxMaterial* material = physicsManager.getPhysics()->createMaterial(0.5f, 0.5f, 0.1f);
        if (!material) {
            std::cerr << "CharacterController::initializeController() - Failed to create material!" << std::endl;
            return;
        }
        desc.material = material;

        // Set dimensions
        desc.height = std::max(0.1f, height);
        desc.radius = std::max(0.01f, radius);

        // Set movement properties
        desc.stepOffset = std::max(0.01f, std::min(stepOffset, desc.radius));
        desc.slopeLimit = std::cos(slopeLimit * 3.14159f / 180.0f); // Convert degrees to radians, then cos
        desc.contactOffset = std::max(0.001f, std::min(contactOffset, desc.radius * 0.1f));

        // Set position
        desc.position = physx::PxExtendedVec3(position.x, position.y + desc.radius + desc.height * 0.5f, position.z);
        desc.upDirection = physx::PxVec3(0, 1, 0);
        desc.climbingMode = physx::PxCapsuleClimbingMode::eCONSTRAINED;

        // Set user data
        desc.userData = this;

        // Validate descriptor before creating controller
        if (!desc.isValid()) {
            std::cerr << "CharacterController::initializeController() - Invalid capsule controller descriptor!" << std::endl;
            std::cerr << "  Height: " << desc.height << " (min: 0.1)" << std::endl;
            std::cerr << "  Radius: " << desc.radius << " (min: 0.01)" << std::endl;
            std::cerr << "  StepOffset: " << desc.stepOffset << " (max: radius)" << std::endl;
            std::cerr << "  ContactOffset: " << desc.contactOffset << " (max: radius * 0.1)" << std::endl;
            return;
        }

        capsuleController = static_cast<physx::PxCapsuleController*>(manager->createController(desc));
        controller = capsuleController;
    }
    else {
        physx::PxBoxControllerDesc desc;

        // Create material directly
        physx::PxMaterial* material = physicsManager.getPhysics()->createMaterial(0.5f, 0.5f, 0.1f);
        if (!material) {
            std::cerr << "CharacterController::initializeController() - Failed to create material!" << std::endl;
            return;
        }
        desc.material = material;

        // Set dimensions
        desc.halfHeight = std::max(0.05f, height * 0.5f);
        desc.halfSideExtent = std::max(0.01f, radius);
        desc.halfForwardExtent = std::max(0.01f, radius);

        // Set movement properties
        desc.stepOffset = std::max(0.01f, std::min(stepOffset, std::min(desc.halfSideExtent, desc.halfForwardExtent)));
        desc.slopeLimit = std::cos(slopeLimit * 3.14159f / 180.0f); // Convert degrees to radians, then cos
        desc.contactOffset = std::max(0.001f, std::min(contactOffset, std::min(desc.halfSideExtent, desc.halfForwardExtent) * 0.1f));

        // Set position
        desc.position = physx::PxExtendedVec3(position.x, position.y + desc.halfHeight, position.z);
        desc.upDirection = physx::PxVec3(0, 1, 0);

        // Set user data
        desc.userData = this;

        // Validate descriptor before creating controller
        if (!desc.isValid()) {
            std::cerr << "CharacterController::initializeController() - Invalid box controller descriptor!" << std::endl;
            std::cerr << "  HalfHeight: " << desc.halfHeight << " (min: 0.05)" << std::endl;
            std::cerr << "  HalfSideExtent: " << desc.halfSideExtent << " (min: 0.01)" << std::endl;
            std::cerr << "  HalfForwardExtent: " << desc.halfForwardExtent << " (min: 0.01)" << std::endl;
            return;
        }

        boxController = static_cast<physx::PxBoxController*>(manager->createController(desc));
        controller = boxController;
    }

    if (controller) {
        initialized = true;
        std::cout << "CharacterController initialized successfully at position ("
            << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
    }
    else {
        std::cerr << "CharacterController::initializeController() - Failed to create controller!" << std::endl;
    }
}

void CharacterController::move(const glm::vec3& direction) {
    if (!initialized || !controller) return;

    inputDirection = direction;

    // Call move callback
    if (onMoveCallback) {
        onMoveCallback(direction);
    }
}

void CharacterController::updateMovement(float deltaTime) {
    if (!initialized || !controller) return;

    // Calculate horizontal movement based on input
    glm::vec3 horizontalMovement(0.0f);

    if (glm::length(inputDirection) > 0.0f) {
        // Calculate speed based on movement mode
        float speed = walkSpeed;
        switch (currentMovementMode) {
        case MovementMode::Running:
            speed = runSpeed;
            break;
        case MovementMode::Crouching:
            speed = crouchSpeed;
            break;
        case MovementMode::Jumping:
        case MovementMode::Flying:
            speed = walkSpeed * airControl;
            break;
        default:
            speed = walkSpeed;
            break;
        }

        horizontalMovement = inputDirection * speed;
    }

    // Combine horizontal movement with vertical velocity (gravity/jump)
    glm::vec3 totalMovement = horizontalMovement;
    totalMovement.y = velocity.y; // Keep vertical velocity from gravity/jump

    // Apply movement to controller
    physx::PxVec3 moveVec(totalMovement.x * deltaTime,
        totalMovement.y * deltaTime,
        totalMovement.z * deltaTime);

    physx::PxControllerFilters filters;
    physx::PxControllerCollisionFlags flags = controller->move(moveVec, 0.0f, deltaTime, filters);

    // Update grounded state correctly - check if collision flags contain the down collision flag
    bool wasGrounded = isGrounded;
    isGrounded = flags.isSet(physx::PxControllerCollisionFlag::eCOLLISION_DOWN);

    // If we hit the ground, reset vertical velocity
    if (isGrounded && velocity.y <= 0.0f) {
        velocity.y = 0.0f;
        isJumping = false;
    }

    // Call grounded callback if state changed
    if (wasGrounded != isGrounded && onGroundedCallback) {
        onGroundedCallback(isGrounded);
    }
}

void CharacterController::updateGravity(float deltaTime) {
    if (!initialized || !controller) return;

    // Only apply gravity if not grounded or jumping
    if (!isGrounded || isJumping) {
        velocity.y += gravity * deltaTime;

        // Limit maximum fall speed (optional)
        const float maxFallSpeed = -20.0f;
        if (velocity.y < maxFallSpeed) {
            velocity.y = maxFallSpeed;
        }
    }
}

void CharacterController::jump() {
    if (!initialized || !controller || !isGrounded) return;

    // Apply jump impulse
    velocity.y = jumpForce;
    isJumping = true;
    isGrounded = false; // Immediately not grounded

    // Call jump callback
    if (onJumpCallback) {
        onJumpCallback(true);
    }
}

void CharacterController::setMovementMode(MovementMode mode) {
    if (currentMovementMode != mode) {
        MovementMode oldMode = currentMovementMode;
        currentMovementMode = mode;

        // Handle mode-specific logic
        switch (mode) {
        case MovementMode::Crouching:
            if (!isCrouching) {
                isCrouching = true;
                if (onCrouchCallback) onCrouchCallback(true);
            }
            break;
        case MovementMode::Running:
            if (isCrouching) {
                isCrouching = false;
                if (onCrouchCallback) onCrouchCallback(false);
            }
            if (onRunCallback) onRunCallback(true);
            break;
        case MovementMode::Walking:
            if (isCrouching) {
                isCrouching = false;
                if (onCrouchCallback) onCrouchCallback(false);
            }
            if (onRunCallback) onRunCallback(false);
            break;
        case MovementMode::Jumping:
            isJumping = true;
            break;
        case MovementMode::Flying:
            isJumping = false;
            break;
        }
    }
}

void CharacterController::syncTransformFromController() {
    if (!controller || !owner) return;

    physx::PxExtendedVec3 position = controller->getPosition();
    glm::vec3 glmPosition(static_cast<float>(position.x),
        static_cast<float>(position.y),
        static_cast<float>(position.z));

    owner->setWorldPosition(glmPosition);
    // Note: Character controllers don't have rotation, so we keep the GameObject's rotation
}

void CharacterController::syncTransformToController() {
    if (!controller || !owner) return;

    glm::vec3 position = owner->getWorldPosition();

    physx::PxExtendedVec3 pxPosition(position.x, position.y, position.z);
    controller->setPosition(pxPosition);
}

// Character controller properties
void CharacterController::setHeight(float newHeight) {
    height = newHeight;
    if (initialized && capsuleController) {
        capsuleController->setHeight(height);
    }
}

void CharacterController::setRadius(float newRadius) {
    radius = newRadius;
    if (initialized && capsuleController) {
        capsuleController->setRadius(radius);
    }
}

void CharacterController::setStepOffset(float offset) {
    stepOffset = offset;
    if (initialized && controller) {
        controller->setStepOffset(offset);
    }
}

void CharacterController::setSlopeLimit(float limit) {
    slopeLimit = limit;
    if (initialized && controller) {
        controller->setSlopeLimit(limit);
    }
}

void CharacterController::setContactOffset(float offset) {
    contactOffset = offset;
    if (initialized && controller) {
        controller->setContactOffset(offset);
    }
}

// Movement properties
void CharacterController::setWalkSpeed(float speed) {
    walkSpeed = speed;
}

void CharacterController::setRunSpeed(float speed) {
    runSpeed = speed;
}

void CharacterController::setCrouchSpeed(float speed) {
    crouchSpeed = speed;
}

void CharacterController::setJumpForce(float force) {
    jumpForce = force;
}

void CharacterController::setGravity(float gravityValue) {
    gravity = gravityValue;
}

void CharacterController::setAirControl(float control) {
    airControl = control;
}

// Manual movement methods
void CharacterController::setVelocity(const glm::vec3& newVelocity) {
    velocity = newVelocity;
}

void CharacterController::addForce(const glm::vec3& force) {
    velocity += force;
}

// Collision detection
bool CharacterController::isOnGround() const {
    return isGrounded;
}

bool CharacterController::isOnSlope() const {
    if (!controller || !isGrounded) return false;

    // Use raycast for more precise slope detection
    auto& physicsManager = PhysicsManager::getInstance();
    physx::PxScene* scene = physicsManager.getScene();

    if (!scene) return false;

    // Raycast downward from controller position
    physx::PxExtendedVec3 pos = controller->getPosition();
    physx::PxVec3 origin(static_cast<float>(pos.x),
        static_cast<float>(pos.y) + 0.1f,
        static_cast<float>(pos.z));
    physx::PxVec3 direction(0, -1, 0);
    float maxDistance = 0.5f;

    physx::PxRaycastBuffer hit;
    bool hasHit = scene->raycast(origin, direction, maxDistance, hit);

    if (hasHit && hit.hasBlock) {
        physx::PxVec3 normal = hit.block.normal;
        float angle = std::acos(normal.dot(physx::PxVec3(0, 1, 0)));
        return angle > 0.1f; // More than ~5.7 degrees
    }

    return false;
}

float CharacterController::getSlopeAngle() const {
    if (!controller || !isGrounded) return 0.0f;

    auto& physicsManager = PhysicsManager::getInstance();
    physx::PxScene* scene = physicsManager.getScene();

    if (!scene) return 0.0f;

    // Raycast downward
    physx::PxExtendedVec3 pos = controller->getPosition();
    physx::PxVec3 origin(static_cast<float>(pos.x),
        static_cast<float>(pos.y) + 0.1f,
        static_cast<float>(pos.z));
    physx::PxVec3 direction(0, -1, 0);
    float maxDistance = 0.5f;

    physx::PxRaycastBuffer hit;
    bool hasHit = scene->raycast(origin, direction, maxDistance, hit);

    if (hasHit && hit.hasBlock) {
        physx::PxVec3 normal = hit.block.normal;
        float angle = std::acos(normal.dot(physx::PxVec3(0, 1, 0)));
        return angle * 180.0f / 3.14159f; // Convert radians to degrees
    }

    return 0.0f;
}

// Teleportation
void CharacterController::teleport(const glm::vec3& position) {
    if (!controller) return;

    physx::PxExtendedVec3 pxPosition(position.x, position.y, position.z);
    controller->setPosition(pxPosition);

    // Reset velocity
    velocity = glm::vec3(0.0f);
    inputDirection = glm::vec3(0.0f);
}

void CharacterController::teleport(const glm::vec3& position, const glm::quat& rotation) {
    if (!controller) return;

    physx::PxExtendedVec3 pxPosition(position.x, position.y, position.z);
    controller->setPosition(pxPosition);

    // Reset velocity
    velocity = glm::vec3(0.0f);
    inputDirection = glm::vec3(0.0f);

    // Set rotation on the GameObject since controller doesn't handle rotation
    if (owner) {
        owner->setWorldRotationQuat(rotation);
    }
}



std::string CharacterController::serializeComponent() const {
    json j;
    j["controllerType"] = static_cast<int>(controllerType);
    j["height"] = height;
    j["radius"] = radius;
    j["stepOffset"] = stepOffset;
    j["slopeLimit"] = slopeLimit;
    j["contactOffset"] = contactOffset;
    j["walkSpeed"] = walkSpeed;
    j["runSpeed"] = runSpeed;
    j["crouchSpeed"] = crouchSpeed;
    j["jumpForce"] = jumpForce;
    j["gravity"] = gravity;
    j["airControl"] = airControl;

    j["currentMovementMode"] = static_cast<int>(currentMovementMode);
    j["isGrounded"] = isGrounded;
    j["isCrouching"] = isCrouching;
    j["isJumping"] = isJumping;

    j["velocity"] = { velocity.x, velocity.y, velocity.z };
    j["inputDirection"] = { inputDirection.x, inputDirection.y, inputDirection.z };

    // Si guardas posición o rotación, agrégalo aquí

    return j.dump();
}


void CharacterController::deserialize(const std::string& data) {
    json j = json::parse(data);

    // Primero, el tipo de controller (si es necesario)
    controllerType = static_cast<CharacterControllerType>(j.value("controllerType", 0));

    setHeight(j.value("height", 2.0f));
    setRadius(j.value("radius", 0.5f));
    setStepOffset(j.value("stepOffset", 0.5f));
    setSlopeLimit(j.value("slopeLimit", 45.0f));
    setContactOffset(j.value("contactOffset", 0.1f));

    setWalkSpeed(j.value("walkSpeed", 5.0f));
    setRunSpeed(j.value("runSpeed", 8.0f));
    setCrouchSpeed(j.value("crouchSpeed", 2.5f));
    setJumpForce(j.value("jumpForce", 5.0f));
    setGravity(j.value("gravity", 9.8f));
    setAirControl(j.value("airControl", 0.5f));

    // Estado lógico (flags internos)
    currentMovementMode = static_cast<MovementMode>(j.value("currentMovementMode", 0));
    isGrounded = j.value("isGrounded", false);
    isCrouching = j.value("isCrouching", false);
    isJumping = j.value("isJumping", false);

    if (j.contains("velocity") && j["velocity"].is_array()) {
        auto v = j["velocity"];
        velocity = glm::vec3(v[0], v[1], v[2]);
    }
    if (j.contains("inputDirection") && j["inputDirection"].is_array()) {
        auto v = j["inputDirection"];
        inputDirection = glm::vec3(v[0], v[1], v[2]);
    }

    // Si quieres reflejar el estado de movimiento en animaciones, etc.
    setMovementMode(currentMovementMode);

    // Si guardas y quieres restaurar la posición, llama aquí:
    // if (j.contains("position") && j["position"].is_array()) {
    //     auto p = j["position"];
    //     teleport(glm::vec3(p[0], p[1], p[2]));
    // }

    // Si tu controlador y GameObject ya están vivos, puedes sincronizar transform:
    syncTransformToController();
}

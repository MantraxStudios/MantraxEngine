#include "CoreWrapper.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include "../components/SceneManager.h"
#include "../components/SpriteAnimator.h"
#include "../input/InputAction.h"
#include "../input/InputSystem.h"
#include "../core/InputConfigLoader.h"
#include "../components/CharacterController.h"
#include "../components/PhysicalObject.h"
#include "../components/LightComponent.h"
#include "../components/AudioSource.h"
#include "../components/ScriptExecutor.h"
#include "../components/Rigidbody.h"
#include "../components/Collider.h"
#include "../render/Light.h"
#include "../render/Camera.h"

void CoreWrapper::Register(sol::state& lua) {
    RegisterDebug(lua);
    RegisterMaths(lua);
    RegisterInput(lua);
    RegisterGameObject(lua);
    RegisterCharacterController(lua);
    RegisterPhysicalObject(lua);
    RegisterLightComponent(lua);
    RegisterAudioSource(lua);
    RegisterScriptExecutor(lua);
    RegisterSpriteAnimator(lua);
    RegisterCamera(lua);
}

void CoreWrapper::RegisterMaths(sol::state& lua) {
    std::cout << "[CoreWrapper] RegisterMaths called - GLM includes should be working" << std::endl;

    // VECTOR3
    
    lua.new_usertype<glm::vec3>("vector3",
        sol::constructors<glm::vec3(float, float, float)>(),
        "x", &glm::vec3::x,
        "y", &glm::vec3::y,
        "z", &glm::vec3::z,
        // Métodos extra:
        "length", [](const glm::vec3& v) { return glm::length(v); },
        "normalize", [](const glm::vec3& v) { return glm::normalize(v); },
        "dot", [](const glm::vec3& a, const glm::vec3& b) { return glm::dot(a, b); },
        "cross", [](const glm::vec3& a, const glm::vec3& b) { return glm::cross(a, b); }
    );
    lua["vector3"]["new"] = [](float x, float y, float z) {
        return glm::vec3(x, y, z);
        };

    // VECTOR2
    lua.new_usertype<glm::vec2>("vector2",
        sol::constructors<glm::vec2(float, float)>(),
        "x", &glm::vec2::x,
        "y", &glm::vec2::y,
        "length", [](const glm::vec2& v) { return glm::length(v); },
        "normalize", [](const glm::vec2& v) { return glm::normalize(v); },
        "dot", [](const glm::vec2& a, const glm::vec2& b) { return glm::dot(a, b); }
    );
    lua["vector2"]["new"] = [](float x, float y) {
        return glm::vec2(x, y);
        };

    // QUATERNION
    lua.new_usertype<glm::quat>("quaternion",
        sol::constructors<glm::quat(float, float, float, float)>(),
        "w", &glm::quat::w,
        "x", &glm::quat::x,
        "y", &glm::quat::y,
        "z", &glm::quat::z,
        "length", [](const glm::quat& q) { return glm::length(q); },
        "normalize", [](const glm::quat& q) { return glm::normalize(q); },
        "conjugate", [](const glm::quat& q) { return glm::conjugate(q); },
        "inverse", [](const glm::quat& q) { return glm::inverse(q); },
        "mul", [](const glm::quat& a, const glm::quat& b) { return a * b; } // Quaternion multiplication
    );
    lua["quaternion"]["new"] = [](float w, float x, float y, float z) {
        return glm::quat(w, x, y, z);
        };

    // LIST tipo sencillo (puedes hacer vector3 list o cualquier otro tipo también)
    lua.new_usertype<std::vector<float>>("list",
        sol::constructors<std::vector<float>()>(),
        "size", [](const std::vector<float>& v) { return v.size(); },
        "get", [](const std::vector<float>& v, size_t idx) {
            if (idx < 1 || idx > v.size()) throw std::out_of_range("list index out of range (Lua is 1-based)");
            return v[idx - 1];
        },
        "set", [](std::vector<float>& v, size_t idx, float value) {
            if (idx < 1 || idx > v.size()) throw std::out_of_range("list index out of range (Lua is 1-based)");
            v[idx - 1] = value;
        },
        "push", [](std::vector<float>& v, float value) { v.push_back(value); }
    );
    lua["list"]["new"] = []() {
        return std::vector<float>();
        };

    std::cout << "[Lua] Math library registered successfully" << std::endl;
}

void CoreWrapper::RegisterDebug(sol::state& lua) {
    lua.set_function("log", [](const std::string& msg) {
        std::cout << "[Lua] " << msg << std::endl;
        });
}

void CoreWrapper::RegisterGameObject(sol::state& lua) {
    // ===== PHYSICS LAYER CONSTANTS =====
    lua["LAYER_0"] = LAYER_0;
    lua["LAYER_1"] = LAYER_1;
    lua["LAYER_2"] = LAYER_2;
    lua["LAYER_3"] = LAYER_3;
    lua["LAYER_4"] = LAYER_4;
    lua["LAYER_5"] = LAYER_5;
    lua["LAYER_6"] = LAYER_6;
    lua["LAYER_7"] = LAYER_7;
    lua["LAYER_8"] = LAYER_8;
    lua["LAYER_9"] = LAYER_9;
    lua["LAYER_10"] = LAYER_10;
    lua["LAYER_11"] = LAYER_11;
    lua["LAYER_12"] = LAYER_12;
    lua["LAYER_13"] = LAYER_13;
    lua["LAYER_14"] = LAYER_14;
    lua["LAYER_15"] = LAYER_15;
    lua["LAYER_16"] = LAYER_16;
    lua["LAYER_17"] = LAYER_17;
    lua["LAYER_18"] = LAYER_18;
    lua["LAYER_19"] = LAYER_19;
    lua["LAYER_TRIGGER"] = LAYER_TRIGGER;
    lua["LAYER_PLAYER"] = LAYER_PLAYER;
    lua["LAYER_ENEMY"] = LAYER_ENEMY;
    lua["LAYER_ENVIRONMENT"] = LAYER_ENVIRONMENT;
    lua["LAYER_SENSOR"] = LAYER_SENSOR;

    // ===== GAMEOBJECT REGISTRATION =====
    lua.new_usertype<GameObject>("GameObject",
        // --- Basic Properties ---
        "Name", &GameObject::Name,
        "Tag", &GameObject::Tag,
        "ObjectID", sol::readonly(&GameObject::ObjectID),
        "ModelPath", sol::readonly(&GameObject::ModelPath),

        // --- Transform ---
        "getPosition", &GameObject::getWorldPosition,
        "setPosition", &GameObject::setWorldPosition,
        "getRotation", &GameObject::getWorldRotationEuler,
        "setRotation", &GameObject::setWorldRotationEuler,
        "getScale", &GameObject::getWorldScale,
        "setScale", &GameObject::setWorldScale,

        "getLocalPosition", &GameObject::getLocalPosition,
        "setLocalPosition", &GameObject::setLocalPosition,
        "getLocalRotation", &GameObject::getLocalRotationEuler,
        "setLocalRotation", &GameObject::setLocalRotationEuler,
        "getLocalScale", &GameObject::getLocalScale,
        "setLocalScale", &GameObject::setLocalScale,

        // --- Hierarchy ---
        "setParent", &GameObject::setParent,
        "getParent", &GameObject::getParent,
        "addChild", &GameObject::addChild,
        "removeChild", &GameObject::removeChild,
        "getChildren", &GameObject::getChildren,
        "getChildCount", &GameObject::getChildCount,
        "getChild", &GameObject::getChild,
        "findChild", &GameObject::findChild,
        "findChildRecursive", &GameObject::findChildRecursive,

        // --- Components ---
        "getPhysicalObject", [](GameObject& obj) { return obj.getComponent<PhysicalObject>(); },
        "getAudioSource", [](GameObject& obj) { return obj.getComponent<AudioSource>(); },
        "getLight", [](GameObject& obj) { return obj.getComponent<LightComponent>(); },
        "getSpriteAnimator", [](GameObject& obj) { return obj.getComponent<SpriteAnimator>(); },
        "getCC", [](GameObject& obj) { return obj.getComponent<CharacterController>(); },
        "getRB", [](GameObject& obj) { return obj.getComponent<Rigidbody>(); },
        "getCL", [](GameObject& obj) { return obj.getComponent<Collider>(); },

        // --- Render/State ---
        "isRenderEnabled", &GameObject::isRenderEnabled,
        "setRenderEnabled", &GameObject::setRenderEnabled,
        "isTransformUpdateEnabled", &GameObject::isTransformUpdateEnabled,
        "setTransformUpdateEnabled", &GameObject::setTransformUpdateEnabled,

        // --- Physics Layers ---
        "getLayer", &GameObject::getLayer,
        "setLayer", &GameObject::setLayer,
        "getLayerMask", &GameObject::getLayerMask,
        "setLayerMask", &GameObject::setLayerMask,

        // --- Misc ---
        "isValid", &GameObject::isValid,
        "destroy", &GameObject::destroy
    );

    std::cout << "[Lua] GameObject system registered successfully" << std::endl;
}

void CoreWrapper::RegisterPhysicalObject(sol::state& lua) {
    // BodyType
    lua.new_enum<BodyType>("BodyType", {
        {"Static",   BodyType::Static},
        {"Dynamic",  BodyType::Dynamic},
        {"Kinematic",BodyType::Kinematic}
    });

    // ShapeType
    lua.new_enum<ShapeType>("ShapeType", {
        {"Box",     ShapeType::Box},
        {"Sphere",  ShapeType::Sphere},
        {"Capsule", ShapeType::Capsule},
        {"Plane",   ShapeType::Plane}
    });

    lua.new_enum<physx::PxForceMode::Enum>("ForceMode", {
        {"Force",      physx::PxForceMode::eFORCE},
        {"Impulse",    physx::PxForceMode::eIMPULSE},
        {"Velocity",   physx::PxForceMode::eVELOCITY_CHANGE},
        {"Acceleration", physx::PxForceMode::eACCELERATION}
    });

    lua.new_usertype<Collider>("Collider",
        // Nombre del componente
        "getComponentName", &Collider::getComponentName,

        // Shape
        "setShapeType", &Collider::setShapeType,
        "getShapeType", &Collider::getShapeType,
        "getShapeTypeString", &Collider::getShapeTypeString,

        // Caja
        "setBoxHalfExtents", &Collider::setBoxHalfExtents,
        "getBoxHalfExtents", &Collider::getBoxHalfExtents,

        // Esfera
        "setSphereRadius", &Collider::setSphereRadius,
        "getSphereRadius", &Collider::getSphereRadius,

        // Cápsula
        "setCapsuleRadius", &Collider::setCapsuleRadius,
        "getCapsuleRadius", &Collider::getCapsuleRadius,
        "setCapsuleHalfHeight", &Collider::setCapsuleHalfHeight,
        "getCapsuleHalfHeight", &Collider::getCapsuleHalfHeight,

        // Material
        "setFriction", &Collider::setFriction,
        "getFriction", &Collider::getFriction,
        "setRestitution", &Collider::setRestitution,
        "getRestitution", &Collider::getRestitution,

        // Trigger
        "setTrigger", &Collider::setTrigger,
        "isTrigger", &Collider::isTrigger,
        "setTriggerCallback", &Collider::setTriggerCallback,

        // Contactos
        "setContactCallback", &Collider::setContactCallback,

        // Colisiones
        "setCollisionGroup", &Collider::setCollisionGroup,
        "setCollisionMask", &Collider::setCollisionMask,
        "setLayer", &Collider::setLayer,
        "setLayerMask", &Collider::setLayerMask,
        "getLayer", &Collider::getLayer,
        "getLayerMask", &Collider::getLayerMask,
        "debugCollisionFilters", &Collider::debugCollisionFilters
    );

    lua.new_usertype<Rigidbody>("Rigidbody",
        // Nombre del componente
        "getComponentName", &Rigidbody::getComponentName,

        // Propiedades físicas
        "setMass", &Rigidbody::setMass,
        "getMass", &Rigidbody::getMass,

        "setVelocity", &Rigidbody::setVelocity,
        "getVelocity", &Rigidbody::getVelocity,

        "setDamping", &Rigidbody::setDamping,
        "getDamping", &Rigidbody::getDamping,

        "setGravityFactor", &Rigidbody::setGravityFactor,
        "getGravityFactor", &Rigidbody::getGravityFactor,

        // Tipo de cuerpo
        "setBodyType", &Rigidbody::setBodyType,
        "getBodyType", &Rigidbody::getBodyType,

        // Fuerzas / impulsos
        "addForce", &Rigidbody::addForce,
        "addTorque", &Rigidbody::addTorque,
        "addImpulse", &Rigidbody::addImpulse,

        // Estado
        "wakeUp", &Rigidbody::wakeUp,
        "isAwake", &Rigidbody::isAwake,

        // Activación
        "enable", &Rigidbody::enable,
        "disable", &Rigidbody::disable,
        "isActive", &Rigidbody::isActive
    );
}


void CoreWrapper::RegisterInput(sol::state& lua) {
    lua.set_function("ChangeScene", [](const std::string& sceneName) {
        SceneManager::getInstance().setActiveScene(sceneName);
        });

    // Registrar glm::vec2
    lua.new_usertype<glm::vec2>("vec2",
        sol::constructors<glm::vec2(), glm::vec2(float, float)>(),
        "x", &glm::vec2::x,
        "y", &glm::vec2::y
    );

    // Registrar enum InputType
    lua.new_enum<InputType>("InputType", {
        {"Vector2D", InputType::Vector2D},
        {"Value", InputType::Value},
        {"MouseAxis", InputType::MouseAxis},
        {"MouseButton", InputType::MouseButton},
        {"Button", InputType::Button}
        });

    // Registrar InputAction con mejor manejo de errores
    lua.new_usertype<InputAction>("InputAction",
        sol::no_constructor,

        "bindButtonCallback", [](std::shared_ptr<InputAction> self, sol::function func) {
            if (!self) {
                std::cout << "[Lua Error] InputAction is null in bindButtonCallback" << std::endl;
                return;
            }
            self->bindButtonCallback([func](bool pressed) {
                try {
                    func(pressed);
                }
                catch (const std::exception& e) {
                    std::cout << "[Lua Error] Exception in button callback: " << e.what() << std::endl;
                }
                });
        },

        "bindVector2DCallback", [](std::shared_ptr<InputAction> self, sol::function func) {
            if (!self) {
                std::cout << "[Lua Error] InputAction is null in bindVector2DCallback" << std::endl;
                return;
            }
            self->bindVector2DCallback([func](const glm::vec2& vec) {
                try {
                    func(vec);
                }
                catch (const std::exception& e) {
                    std::cout << "[Lua Error] Exception in vector2D callback: " << e.what() << std::endl;
                }
                });
        },

        "bindValueCallback", [](std::shared_ptr<InputAction> self, sol::function func) {
            if (!self) {
                std::cout << "[Lua Error] InputAction is null in bindValueCallback" << std::endl;
                return;
            }
            self->bindValueCallback([func](float value) {
                try {
                    func(value);
                }
                catch (const std::exception& e) {
                    std::cout << "[Lua Error] Exception in value callback: " << e.what() << std::endl;
                }
                });
        },

        "bindMouseAxisCallback", [](std::shared_ptr<InputAction> self, sol::function func) {
            if (!self) {
                std::cout << "[Lua Error] InputAction is null in bindMouseAxisCallback" << std::endl;
                return;
            }
            self->bindMouseAxisCallback([func](float delta) {
                try {
                    func(delta);
                }
                catch (const std::exception& e) {
                    std::cout << "[Lua Error] Exception in mouse axis callback: " << e.what() << std::endl;
                }
                });
        },

        // Mejorado: addKeyBindingByName con mejor manejo de errores
        "addKeyBindingByName", [](std::shared_ptr<InputAction> self, const std::string& keyName,
            sol::optional<bool> positive_opt, sol::optional<int> axis_opt) {

                if (!self) {
                    std::cout << "[Lua Error] InputAction is null in addKeyBindingByName" << std::endl;
                    return;
                }

                // Valores por defecto
                bool positive = positive_opt.value_or(true);
                int axis = axis_opt.value_or(0);

                SDL_Keycode key = InputConfigLoader::stringToKeycode(keyName);
                if (key == SDLK_UNKNOWN) {
                    std::cout << "[Lua Error] Tecla desconocida: " << keyName << std::endl;
                }
                else {
                    std::cout << "[Lua Debug] Binding key: " << keyName << " (code: " << key << ")" << std::endl;
                    self->addKeyBinding(key, positive, axis);
                }
        },

        // Mejorado: addMouseAxisBindingByName
        "addMouseAxisBindingByName", [](std::shared_ptr<InputAction> self, const std::string& axisName) {
            if (!self) {
                std::cout << "[Lua Error] InputAction is null in addMouseAxisBindingByName" << std::endl;
                return;
            }

            MouseAxisType axis = InputConfigLoader::stringToMouseAxis(axisName);
            std::cout << "[Lua Debug] Adding mouse axis binding: " << axisName << std::endl;
            self->addMouseAxisBinding(axis);
        },

        // Mejorado: addMouseButtonBindingByName
        "addMouseButtonBindingByName", [](std::shared_ptr<InputAction> self, const std::string& buttonName) {
            if (!self) {
                std::cout << "[Lua Error] InputAction is null in addMouseButtonBindingByName" << std::endl;
                return;
            }

            Uint8 button = InputConfigLoader::stringToMouseButton(buttonName);
            std::cout << "[Lua Debug] Adding mouse button binding: " << buttonName << std::endl;
            self->addMouseButtonBinding(button);
        }
    );

    // Funciones globales con mejor manejo de errores
    lua.set_function("getAction", [](const std::string& name) -> std::shared_ptr<InputAction> {
        auto action = InputSystem::getInstance().getAction(name);
        if (!action) {
            std::cout << "[Lua Warning] Action not found: " << name << std::endl;
        }
        return action;
        });

    lua.set_function("registerAction", [](const std::string& name, InputType type) -> std::shared_ptr<InputAction> {
        std::cout << "[Lua Debug] Registering action: " << name << " with type: " << static_cast<int>(type) << std::endl;
        auto action = InputSystem::getInstance().registerAction(name, type);
        if (!action) {
            std::cout << "[Lua Error] Failed to register action: " << name << std::endl;
        }
        return action;
        });
}

void CoreWrapper::RegisterCharacterController(sol::state& lua) {
    // ===== CHARACTER CONTROLLER TYPE ENUM =====
    lua.new_enum<CharacterControllerType>("CharacterControllerType", {
        {"Capsule", CharacterControllerType::Capsule},
        {"Box", CharacterControllerType::Box}
    });

    // ===== MOVEMENT MODE ENUM =====
    lua.new_enum<MovementMode>("MovementMode", {
        {"Walking", MovementMode::Walking},
        {"Running", MovementMode::Running},
        {"Crouching", MovementMode::Crouching},
        {"Jumping", MovementMode::Jumping},
        {"Flying", MovementMode::Flying}
    });

    // ===== CHARACTER CONTROLLER REGISTRATION =====
    lua.new_usertype<CharacterController>("CharacterController",
        sol::no_constructor,

        // ===== INITIALIZATION =====
        "initializeController", &CharacterController::initializeController,
        "isInitialized", &CharacterController::isInitialized,

        // ===== MOVEMENT METHODS =====
        "move", [](CharacterController* self, const glm::vec3& direction) {
            if (!self) {
                std::cout << "[Lua Error] CharacterController is null in move" << std::endl;
                return;
            }
            self->move(direction);
        },
        "jump", [](CharacterController* self) {
            if (!self) {
                std::cout << "[Lua Error] CharacterController is null in jump" << std::endl;
                return;
            }
            self->jump();
        },
        "setMovementMode", [](CharacterController* self, MovementMode mode) {
            if (!self) {
                std::cout << "[Lua Error] CharacterController is null in setMovementMode" << std::endl;
                return;
            }
            self->setMovementMode(mode);
        },

        // ===== CHARACTER CONTROLLER PROPERTIES =====
        "setHeight", &CharacterController::setHeight,
        "getHeight", &CharacterController::getHeight,
        "setRadius", &CharacterController::setRadius,
        "getRadius", &CharacterController::getRadius,
        "setStepOffset", &CharacterController::setStepOffset,
        "getStepOffset", &CharacterController::getStepOffset,
        "setSlopeLimit", &CharacterController::setSlopeLimit,
        "getSlopeLimit", &CharacterController::getSlopeLimit,
        "setContactOffset", &CharacterController::setContactOffset,
        "getContactOffset", &CharacterController::getContactOffset,

        // ===== MOVEMENT PROPERTIES =====
        "setWalkSpeed", &CharacterController::setWalkSpeed,
        "getWalkSpeed", &CharacterController::getWalkSpeed,
        "setRunSpeed", &CharacterController::setRunSpeed,
        "getRunSpeed", &CharacterController::getRunSpeed,
        "setCrouchSpeed", &CharacterController::setCrouchSpeed,
        "getCrouchSpeed", &CharacterController::getCrouchSpeed,
        "setJumpForce", &CharacterController::setJumpForce,
        "getJumpForce", &CharacterController::getJumpForce,
        "setGravity", &CharacterController::setGravity,
        "getGravity", &CharacterController::getGravity,
        "setAirControl", &CharacterController::setAirControl,
        "getAirControl", &CharacterController::getAirControl,

        // ===== STATE GETTERS =====
        "isGroundedState", &CharacterController::isGroundedState,
        "isCrouchingState", &CharacterController::isCrouchingState,
        "isJumpingState", &CharacterController::isJumpingState,
        "getMovementMode", &CharacterController::getMovementMode,
        "getVelocity", &CharacterController::getVelocity,
        "getInputDirection", &CharacterController::getInputDirection,

        // ===== MANUAL MOVEMENT =====
        "setVelocity", [](CharacterController* self, const glm::vec3& velocity) {
            if (!self) {
                std::cout << "[Lua Error] CharacterController is null in setVelocity" << std::endl;
                return;
            }
            self->setVelocity(velocity);
        },
        "addForce", [](CharacterController* self, const glm::vec3& force) {
            if (!self) {
                std::cout << "[Lua Error] CharacterController is null in addForce" << std::endl;
                return;
            }
            self->addForce(force);
        },

        // ===== COLLISION DETECTION =====
        "isOnGround", &CharacterController::isOnGround,
        "isOnSlope", &CharacterController::isOnSlope,
        "getSlopeAngle", &CharacterController::getSlopeAngle,

        // ===== TELEPORTATION =====
        "teleport", sol::overload(
            [](CharacterController* self, const glm::vec3& position) {
                if (!self) {
                    std::cout << "[Lua Error] CharacterController is null in teleport" << std::endl;
                    return;
                }
                self->teleport(position);
            },
            [](CharacterController* self, const glm::vec3& position, const glm::quat& rotation) {
                if (!self) {
                    std::cout << "[Lua Error] CharacterController is null in teleport" << std::endl;
                    return;
                }
                self->teleport(position, rotation);
            }
        ),

        // ===== CALLBACK SETTERS =====
        "setOnMoveCallback", [](CharacterController* self, sol::function callback) {
            if (!self) {
                std::cout << "[Lua Error] CharacterController is null in setOnMoveCallback" << std::endl;
                return;
            }
            self->setOnMoveCallback([callback](const glm::vec3& direction) {
                try {
                    callback(direction);
                } catch (const std::exception& e) {
                    std::cout << "[Lua Error] Exception in move callback: " << e.what() << std::endl;
                }
            });
        },
        "setOnJumpCallback", [](CharacterController* self, sol::function callback) {
            if (!self) {
                std::cout << "[Lua Error] CharacterController is null in setOnJumpCallback" << std::endl;
                return;
            }
            self->setOnJumpCallback([callback](bool jumping) {
                try {
                    callback(jumping);
                } catch (const std::exception& e) {
                    std::cout << "[Lua Error] Exception in jump callback: " << e.what() << std::endl;
                }
            });
        },
        "setOnRunCallback", [](CharacterController* self, sol::function callback) {
            if (!self) {
                std::cout << "[Lua Error] CharacterController is null in setOnRunCallback" << std::endl;
                return;
            }
            self->setOnRunCallback([callback](bool running) {
                try {
                    callback(running);
                } catch (const std::exception& e) {
                    std::cout << "[Lua Error] Exception in run callback: " << e.what() << std::endl;
                }
            });
        },
        "setOnCrouchCallback", [](CharacterController* self, sol::function callback) {
            if (!self) {
                std::cout << "[Lua Error] CharacterController is null in setOnCrouchCallback" << std::endl;
                return;
            }
            self->setOnCrouchCallback([callback](bool crouching) {
                try {
                    callback(crouching);
                } catch (const std::exception& e) {
                    std::cout << "[Lua Error] Exception in crouch callback: " << e.what() << std::endl;
                }
            });
        },
        "setOnGroundedCallback", [](CharacterController* self, sol::function callback) {
            if (!self) {
                std::cout << "[Lua Error] CharacterController is null in setOnGroundedCallback" << std::endl;
                return;
            }
            self->setOnGroundedCallback([callback](bool grounded) {
                try {
                    callback(grounded);
                } catch (const std::exception& e) {
                    std::cout << "[Lua Error] Exception in grounded callback: " << e.what() << std::endl;
                }
            });
        }
    );

    std::cout << "[Lua] CharacterController system registered successfully" << std::endl;
}


void CoreWrapper::RegisterLightComponent(sol::state& lua) {
    // ===== LIGHT TYPE ENUM =====
    lua.new_enum<LightType>("LightType", {
        {"Point", LightType::Point},
        {"Directional", LightType::Directional},
        {"Spot", LightType::Spot}
    });

    // ===== LIGHT COMPONENT REGISTRATION =====
    lua.new_usertype<LightComponent>("LightComponent",
        // ===== BASIC PROPERTIES =====
        "setColor", &LightComponent::setColor,
        "getColor", &LightComponent::getColor,
        "setIntensity", &LightComponent::setIntensity,
        "getIntensity", &LightComponent::getIntensity,
        "setEnabled", &LightComponent::setEnabled,
        "isEnabled", &LightComponent::isEnabled,

        // ===== TRANSFORM PROPERTIES =====
        "setDirection", &LightComponent::setDirection,
        "getDirection", &LightComponent::getDirection,
        "getPosition", &LightComponent::getPosition,
        "forceTransformUpdate", &LightComponent::forceTransformUpdate,
        "debugLightStatus", &LightComponent::debugLightStatus,

        // ===== ATTENUATION =====
        "setAttenuation", &LightComponent::setAttenuation,
        "getAttenuation", &LightComponent::getAttenuation,

        // ===== RANGE =====
        "setRange", &LightComponent::setRange,
        "getMinDistance", &LightComponent::getMinDistance,
        "getMaxDistance", &LightComponent::getMaxDistance,

        // ===== SPOT LIGHT PROPERTIES =====
        "setCutOffAngle", &LightComponent::setCutOffAngle,
        "getCutOffAngle", &LightComponent::getCutOffAngle,
        "setOuterCutOffAngle", &LightComponent::setOuterCutOffAngle,
        "getOuterCutOffAngle", &LightComponent::getOuterCutOffAngle,
        "setSpotRange", &LightComponent::setSpotRange,
        "getSpotRange", &LightComponent::getSpotRange,

        // ===== TYPE =====
        "getType", &LightComponent::getType
    );

    std::cout << "[Lua] LightComponent system registered successfully" << std::endl;
}

void CoreWrapper::RegisterAudioSource(sol::state& lua) {
    // ===== AUDIO SOURCE REGISTRATION =====
    lua.new_usertype<AudioSource>("AudioSource",
        // ===== SOUND CONFIGURATION =====
        "setSound", &AudioSource::setSound,
        "setVolume", &AudioSource::setVolume,
        "getVolume", &AudioSource::getVolume,
        "set3DAttributes", &AudioSource::set3DAttributes,
        "is3DEnabled", &AudioSource::is3DEnabled,
        "setMinDistance", &AudioSource::setMinDistance,
        "getMinDistance", &AudioSource::getMinDistance,
        "setMaxDistance", &AudioSource::setMaxDistance,
        "getMaxDistance", &AudioSource::getMaxDistance,

        // ===== PLAYBACK CONTROL =====
        "play", &AudioSource::play,
        "stop", &AudioSource::stop,
        "pause", &AudioSource::pause,
        "resume", &AudioSource::resume,

        // ===== STATE =====
        "isPlaying", &AudioSource::isPlaying,
        "isPaused", &AudioSource::isPaused
    );

    std::cout << "[Lua] AudioSource system registered successfully" << std::endl;
}

void CoreWrapper::RegisterScriptExecutor(sol::state& lua) {
    // ===== SCRIPT EXECUTOR REGISTRATION =====
    lua.new_usertype<ScriptExecutor>("ScriptExecutor",
        // ===== SCRIPT PROPERTIES =====
        "luaPath", &ScriptExecutor::luaPath,

        // ===== INSPECTOR HELPER METHODS =====
        "isScriptLoaded", &ScriptExecutor::isScriptLoaded,
        "getLastError", &ScriptExecutor::getLastError,
        "reloadScript", &ScriptExecutor::reloadScript,
        "hasFunction", &ScriptExecutor::hasFunction,

        // ===== TRIGGER EVENTS =====
        "onTriggerEnter", &ScriptExecutor::onTriggerEnter,
        "onTriggerExit", &ScriptExecutor::onTriggerExit
    );

    std::cout << "[Lua] ScriptExecutor system registered successfully" << std::endl;
}

void CoreWrapper::RegisterSpriteAnimator(sol::state& lua) {
    // ===== SPRITE ARRAY STRUCT REGISTRATION =====
    lua.new_usertype<SpriteArray>("SpriteArray",
        // ===== BASIC PROPERTIES =====
        "state_name", &SpriteArray::state_name
    );

    // ===== SPRITE ANIMATOR REGISTRATION =====
    lua.new_usertype<SpriteAnimator>("SpriteAnimator",
        // ===== ANIMATION STATES =====
        "SpriteStates", &SpriteAnimator::SpriteStates,
        "currentState", &SpriteAnimator::currentState,

        // ===== MATERIAL MANAGEMENT =====
        "createMaterial", &SpriteAnimator::createMaterial,
        "setMaterial", &SpriteAnimator::setMaterial,
        "getMaterial", &SpriteAnimator::getMaterial,

        // ===== MATERIAL PROPERTIES =====
        "setSpriteAlbedo", &SpriteAnimator::setSpriteAlbedo,
        "setSpriteMetallic", &SpriteAnimator::setSpriteMetallic,
        "setSpriteRoughness", &SpriteAnimator::setSpriteRoughness,
        "setSpriteEmissive", &SpriteAnimator::setSpriteEmissive,
        "setSpriteTiling", &SpriteAnimator::setSpriteTiling,

        // ===== ANIMATION CONTROL =====
        "updateMaterialTexture", &SpriteAnimator::updateMaterialTexture,
        
        // ===== ANIMATION PLAYBACK =====
        "playAnimation", &SpriteAnimator::playAnimation,
        "stopAnimation", &SpriteAnimator::stopAnimation,
        "pauseAnimation", &SpriteAnimator::pauseAnimation,
        "setAnimationSpeed", &SpriteAnimator::setAnimationSpeed,
        "setCurrentFrame", &SpriteAnimator::setCurrentFrame,
        "getCurrentFrame", &SpriteAnimator::getCurrentFrame,
        "getIsPlaying", &SpriteAnimator::getIsPlaying,
        "getPlaybackState", &SpriteAnimator::getPlaybackState,
        
        // ===== STATE MANAGEMENT METHODS =====
        "addSpriteState", &SpriteAnimator::addSpriteState,
        "setCurrentState", &SpriteAnimator::setCurrentState,
        "addTextureToState", &SpriteAnimator::addTextureToState,
        "loadFromAnimatorFile", &SpriteAnimator::loadFromAnimatorFile,
        "loadFromAnimatorData", &SpriteAnimator::loadFromAnimatorData,
        "getAnimatorFilePath", &SpriteAnimator::getAnimatorFilePath,

        // ===== DEBUGGING METHODS =====
        "enableDebugMode", &SpriteAnimator::enableDebugMode,
        "isDebugModeEnabled", &SpriteAnimator::isDebugModeEnabled,
        "forceUpdate", &SpriteAnimator::forceUpdate,
        
        // ===== VALIDATION METHODS =====
        "isValidState", &SpriteAnimator::isValidState,
        "hasValidTextures", &SpriteAnimator::hasValidTextures,
        "isMaterialValid", &SpriteAnimator::isMaterialValid,
        
        // ===== DEBUG INFO METHODS =====
        "getDebugInfo", &SpriteAnimator::getDebugInfo,
        "printDebugInfo", &SpriteAnimator::printDebugInfo,
        
        // ===== TEXTURE MANAGEMENT METHODS =====
        "loadTexture", &SpriteAnimator::loadTexture,
        "getTexture", &SpriteAnimator::getTexture,
        "preloadAllTextures", &SpriteAnimator::preloadAllTextures,
        "clearTextureCache", &SpriteAnimator::clearTextureCache,

        // ===== INSPECTOR HELPER METHODS =====
        "serializeComponent", &SpriteAnimator::serializeComponent,
        "deserialize", &SpriteAnimator::deserialize,
        "saveToFile", &SpriteAnimator::saveToFile,
        "loadFromFile", &SpriteAnimator::loadFromFile,

        // ===== COMPONENT LIFECYCLE =====
        "start", &SpriteAnimator::start,
        "update", &SpriteAnimator::update
    );

    std::cout << "[Lua] SpriteAnimator system registered successfully" << std::endl;
}

void CoreWrapper::RegisterCamera(sol::state& lua) {
    // ===== PROJECTION TYPE ENUM =====
    lua.new_enum<ProjectionType>("ProjectionType", {
        {"Perspective", ProjectionType::Perspective},
        {"Orthographic", ProjectionType::Orthographic}
    });

    // ===== CAMERA REGISTRATION =====
    lua.new_usertype<Camera>("Camera",
        sol::no_constructor,

        // ===== POSITION AND ORIENTATION =====
        "setPosition", [](Camera* self, const glm::vec3& pos) {
            if (!self) {
                std::cout << "[Lua Error] Camera is null in setPosition" << std::endl;
                return;
            }
            self->setPosition(pos);
        },
        "getPosition", [](Camera* self) -> glm::vec3 {
            if (!self) {
                std::cout << "[Lua Error] Camera is null in getPosition" << std::endl;
                return glm::vec3(0.0f);
            }
            return self->getPosition();
        },
        "setTarget", [](Camera* self, const glm::vec3& target) {
            if (!self) {
                std::cout << "[Lua Error] Camera is null in setTarget" << std::endl;
                return;
            }
            self->setTarget(target);
        },

        // ===== ROTATION =====
        "setRotation", [](Camera* self, float yaw, float pitch) {
            if (!self) {
                std::cout << "[Lua Error] Camera is null in setRotation" << std::endl;
                return;
            }
            self->setRotation(yaw, pitch);
        },
        "rotate", [](Camera* self, float yawDelta, float pitchDelta) {
            if (!self) {
                std::cout << "[Lua Error] Camera is null in rotate" << std::endl;
                return;
            }
            self->rotate(yawDelta, pitchDelta);
        },

        // ===== MOVEMENT =====
        "moveForward", [](Camera* self, float distance) {
            if (!self) {
                std::cout << "[Lua Error] Camera is null in moveForward" << std::endl;
                return;
            }
            self->moveForward(distance);
        },
        "moveRight", [](Camera* self, float distance) {
            if (!self) {
                std::cout << "[Lua Error] Camera is null in moveRight" << std::endl;
                return;
            }
            self->moveRight(distance);
        },
        "moveUp", [](Camera* self, float distance) {
            if (!self) {
                std::cout << "[Lua Error] Camera is null in moveUp" << std::endl;
                return;
            }
            self->moveUp(distance);
        },

        // ===== ORIENTATION VECTORS =====
        "getForward", [](Camera* self) -> glm::vec3 {
            if (!self) {
                std::cout << "[Lua Error] Camera is null in getForward" << std::endl;
                return glm::vec3(0.0f, 0.0f, -1.0f);
            }
            return self->getForward();
        },
        "getRight", [](Camera* self) -> glm::vec3 {
            if (!self) {
                std::cout << "[Lua Error] Camera is null in getRight" << std::endl;
                return glm::vec3(1.0f, 0.0f, 0.0f);
            }
            return self->getRight();
        },
        "getUp", [](Camera* self) -> glm::vec3 {
            if (!self) {
                std::cout << "[Lua Error] Camera is null in getUp" << std::endl;
                return glm::vec3(0.0f, 1.0f, 0.0f);
            }
            return self->getUp();
        },

        // ===== PROJECTION SETTINGS =====
        "setProjectionType", [](Camera* self, ProjectionType type, bool instant) {
            if (!self) {
                std::cout << "[Lua Error] Camera is null in setProjectionType" << std::endl;
                return;
            }
            self->setProjectionType(type, instant);
        },
        "getProjectionType", [](Camera* self) -> ProjectionType {
            if (!self) {
                std::cout << "[Lua Error] Camera is null in getProjectionType" << std::endl;
                return ProjectionType::Perspective;
            }
            return self->getProjectionType();
        },
        "isTransitioning", [](Camera* self) -> bool {
            if (!self) {
                std::cout << "[Lua Error] Camera is null in isTransitioning" << std::endl;
                return false;
            }
            return self->isTransitioning();
        },

        // ===== PERSPECTIVE SETTINGS =====
        "setFOV", [](Camera* self, float fov) {
            if (!self) {
                std::cout << "[Lua Error] Camera is null in setFOV" << std::endl;
                return;
            }
            // Note: This would need to be added to Camera class
            // For now, we'll use the existing methods
        },
        "getFOV", [](Camera* self) -> float {
            if (!self) {
                std::cout << "[Lua Error] Camera is null in getFOV" << std::endl;
                return 45.0f;
            }
            return self->getFOV();
        },
        "setAspectRatio", [](Camera* self, float aspect) {
            if (!self) {
                std::cout << "[Lua Error] Camera is null in setAspectRatio" << std::endl;
                return;
            }
            self->setAspectRatio(aspect);
        },
        "getAspectRatio", [](Camera* self) -> float {
            if (!self) {
                std::cout << "[Lua Error] Camera is null in getAspectRatio" << std::endl;
                return 16.0f / 9.0f;
            }
            return self->getAspectRatio();
        },

        // ===== ORTHOGRAPHIC SETTINGS =====
        "setOrthographicSize", [](Camera* self, float size) {
            if (!self) {
                std::cout << "[Lua Error] Camera is null in setOrthographicSize" << std::endl;
                return;
            }
            self->setOrthographicSize(size);
        },
        "getOrthographicSize", [](Camera* self) -> float {
            if (!self) {
                std::cout << "[Lua Error] Camera is null in getOrthographicSize" << std::endl;
                return 5.0f;
            }
            return self->getOrthographicSize();
        },

        // ===== CLIP PLANES =====
        "setNearClip", [](Camera* self, float nearClip) {
            if (!self) {
                std::cout << "[Lua Error] Camera is null in setNearClip" << std::endl;
                return;
            }
            // Note: This would need to be added to Camera class
        },
        "getNearClip", [](Camera* self) -> float {
            if (!self) {
                std::cout << "[Lua Error] Camera is null in getNearClip" << std::endl;
                return 0.1f;
            }
            return self->getNearClip();
        },
        "setFarClip", [](Camera* self, float farClip) {
            if (!self) {
                std::cout << "[Lua Error] Camera is null in setFarClip" << std::endl;
                return;
            }
            // Note: This would need to be added to Camera class
        },
        "getFarClip", [](Camera* self) -> float {
            if (!self) {
                std::cout << "[Lua Error] Camera is null in getFarClip" << std::endl;
                return 1000.0f;
            }
            return self->getFarClip();
        },

        // ===== FRAMEBUFFER =====
        "enableFramebuffer", [](Camera* self, bool enabled) {
            if (!self) {
                std::cout << "[Lua Error] Camera is null in enableFramebuffer" << std::endl;
                return;
            }
            self->enableFramebuffer(enabled);
        },
        "isFramebufferEnabled", [](Camera* self) -> bool {
            if (!self) {
                std::cout << "[Lua Error] Camera is null in isFramebufferEnabled" << std::endl;
                return false;
            }
            return self->isFramebufferEnabled();
        },
        "setFramebufferSize", [](Camera* self, int width, int height) {
            if (!self) {
                std::cout << "[Lua Error] Camera is null in setFramebufferSize" << std::endl;
                return;
            }
            self->setFramebufferSize(width, height);
        },

        // ===== VELOCITY (for audio) =====
        "getVelocity", [](Camera* self) -> glm::vec3 {
            if (!self) {
                std::cout << "[Lua Error] Camera is null in getVelocity" << std::endl;
                return glm::vec3(0.0f);
            }
            return self->GetVelocity();
        },

        // ===== UPDATE =====
        "update", [](Camera* self, float deltaTime) {
            if (!self) {
                std::cout << "[Lua Error] Camera is null in update" << std::endl;
                return;
            }
            self->update(deltaTime);
        }
    );

    // ===== GLOBAL CAMERA ACCESS FUNCTIONS =====
    lua.set_function("getActiveCamera", []() -> Camera* {
        auto& sceneManager = SceneManager::getInstance();
        Scene* activeScene = sceneManager.getActiveScene();
        if (!activeScene) {
            std::cout << "[Lua Error] No active scene found" << std::endl;
            return nullptr;
        }
        return activeScene->getCamera();
    });

    lua.set_function("getCameraFromScene", [](const std::string& sceneName) -> Camera* {
        auto& sceneManager = SceneManager::getInstance();
        Scene* scene = sceneManager.getScene(sceneName);
        if (!scene) {
            std::cout << "[Lua Error] Scene not found: " << sceneName << std::endl;
            return nullptr;
        }
        return scene->getCamera();
    });

    std::cout << "[Lua] Camera system registered successfully" << std::endl;
}
#include "CoreWrapper.h"
#include "../components/SceneManager.h"
#include "../input/InputAction.h"
#include "../input/InputSystem.h"
#include "../core/InputConfigLoader.h"
#include "../components/CharacterController.h"
#include "../components/PhysicalObject.h"
#include "../components/LightComponent.h"
#include "../components/AudioSource.h"
#include "../components/ScriptExecutor.h"
#include "../render/Light.h"

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
}

void CoreWrapper::RegisterMaths(sol::state& lua) {
    // ===== GLM VEC2 =====
    lua.new_usertype<glm::vec2>("vec2",
        sol::constructors<glm::vec2(), glm::vec2(float), glm::vec2(float, float)>(),

        // Propiedades
        "x", &glm::vec2::x,
        "y", &glm::vec2::y,

        // Operadores
        sol::meta_function::addition, [](const glm::vec2& a, const glm::vec2& b) { return a + b; },
        sol::meta_function::subtraction, [](const glm::vec2& a, const glm::vec2& b) { return a - b; },
        sol::meta_function::multiplication, sol::overload(
            [](const glm::vec2& a, const glm::vec2& b) { return a * b; },
            [](const glm::vec2& a, float b) { return a * b; },
            [](float a, const glm::vec2& b) { return a * b; }
        ),
        sol::meta_function::division, sol::overload(
            [](const glm::vec2& a, const glm::vec2& b) { return a / b; },
            [](const glm::vec2& a, float b) { return a / b; }
        ),
        sol::meta_function::unary_minus, [](const glm::vec2& a) { return -a; },
        sol::meta_function::to_string, [](const glm::vec2& v) {
            return "vec2(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ")";
        },

        // M�todos
        "length", [](const glm::vec2& v) { return glm::length(v); },
        "normalize", [](const glm::vec2& v) { return glm::normalize(v); },
        "dot", [](const glm::vec2& a, const glm::vec2& b) { return glm::dot(a, b); },
        "distance", [](const glm::vec2& a, const glm::vec2& b) { return glm::distance(a, b); }
    );

    // ===== GLM VEC3 =====
    lua.new_usertype<glm::vec3>("vec3",
        sol::constructors<glm::vec3(), glm::vec3(float), glm::vec3(float, float, float)>(),

        // Propiedades
        "x", &glm::vec3::x,
        "y", &glm::vec3::y,
        "z", &glm::vec3::z,

        // Operadores
        sol::meta_function::addition, [](const glm::vec3& a, const glm::vec3& b) { return a + b; },
        sol::meta_function::subtraction, [](const glm::vec3& a, const glm::vec3& b) { return a - b; },
        sol::meta_function::multiplication, sol::overload(
            [](const glm::vec3& a, const glm::vec3& b) { return a * b; },
            [](const glm::vec3& a, float b) { return a * b; },
            [](float a, const glm::vec3& b) { return a * b; }
        ),
        sol::meta_function::division, sol::overload(
            [](const glm::vec3& a, const glm::vec3& b) { return a / b; },
            [](const glm::vec3& a, float b) { return a / b; }
        ),
        sol::meta_function::unary_minus, [](const glm::vec3& a) { return -a; },
        sol::meta_function::to_string, [](const glm::vec3& v) {
            return "vec3(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ", " + std::to_string(v.z) + ")";
        },

        // M�todos
        "length", [](const glm::vec3& v) { return glm::length(v); },
        "normalize", [](const glm::vec3& v) { return glm::normalize(v); },
        "dot", [](const glm::vec3& a, const glm::vec3& b) { return glm::dot(a, b); },
        "cross", [](const glm::vec3& a, const glm::vec3& b) { return glm::cross(a, b); },
        "distance", [](const glm::vec3& a, const glm::vec3& b) { return glm::distance(a, b); },
        "reflect", [](const glm::vec3& incident, const glm::vec3& normal) { return glm::reflect(incident, normal); }
    );

    // ===== GLM QUATERNION =====
    lua.new_usertype<glm::quat>("quat",
        sol::constructors<glm::quat(), glm::quat(float, float, float, float)>(),

        // Propiedades (w, x, y, z)
        "w", &glm::quat::w,
        "x", &glm::quat::x,
        "y", &glm::quat::y,
        "z", &glm::quat::z,

        // Operadores
        sol::meta_function::multiplication, sol::overload(
            [](const glm::quat& a, const glm::quat& b) { return a * b; },
            [](const glm::quat& q, const glm::vec3& v) { return q * v; }
        ),
        sol::meta_function::to_string, [](const glm::quat& q) {
            return "quat(" + std::to_string(q.w) + ", " + std::to_string(q.x) + ", " +
                std::to_string(q.y) + ", " + std::to_string(q.z) + ")";
        },

        // M�todos
        "length", [](const glm::quat& q) { return glm::length(q); },
        "normalize", [](const glm::quat& q) { return glm::normalize(q); },
        "conjugate", [](const glm::quat& q) { return glm::conjugate(q); },
        "inverse", [](const glm::quat& q) { return glm::inverse(q); },
        "dot", [](const glm::quat& a, const glm::quat& b) { return glm::dot(a, b); },
        "slerp", [](const glm::quat& a, const glm::quat& b, float t) { return glm::slerp(a, b, t); },
        "toEuler", [](const glm::quat& q) { return glm::eulerAngles(q); },
        "toMat3", [](const glm::quat& q) { return glm::mat3_cast(q); },
        "toMat4", [](const glm::quat& q) { return glm::mat4_cast(q); }
    );

    // ===== GLM MAT3 =====
    lua.new_usertype<glm::mat3>("mat3",
        sol::constructors<glm::mat3(), glm::mat3(float)>(),

        // Operadores
        sol::meta_function::multiplication, sol::overload(
            [](const glm::mat3& a, const glm::mat3& b) { return a * b; },
            [](const glm::mat3& m, const glm::vec3& v) { return m * v; },
            [](const glm::mat3& m, float f) { return m * f; }
        ),
        sol::meta_function::addition, [](const glm::mat3& a, const glm::mat3& b) { return a + b; },
        sol::meta_function::subtraction, [](const glm::mat3& a, const glm::mat3& b) { return a - b; },

        // M�todos
        "transpose", [](const glm::mat3& m) { return glm::transpose(m); },
        "inverse", [](const glm::mat3& m) { return glm::inverse(m); },
        "determinant", [](const glm::mat3& m) { return glm::determinant(m); }
    );

    // ===== GLM MAT4 =====
    lua.new_usertype<glm::mat4>("mat4",
        sol::constructors<glm::mat4(), glm::mat4(float)>(),

        // Operadores
        sol::meta_function::multiplication, sol::overload(
            [](const glm::mat4& a, const glm::mat4& b) { return a * b; },
            [](const glm::mat4& m, const glm::vec4& v) { return m * v; },
            [](const glm::mat4& m, float f) { return m * f; }
        ),
        sol::meta_function::addition, [](const glm::mat4& a, const glm::mat4& b) { return a + b; },
        sol::meta_function::subtraction, [](const glm::mat4& a, const glm::mat4& b) { return a - b; },

        // M�todos
        "transpose", [](const glm::mat4& m) { return glm::transpose(m); },
        "inverse", [](const glm::mat4& m) { return glm::inverse(m); },
        "determinant", [](const glm::mat4& m) { return glm::determinant(m); }
    );

    // ===== GLM VEC4 =====
    lua.new_usertype<glm::vec4>("vec4",
        sol::constructors<glm::vec4(), glm::vec4(float), glm::vec4(float, float, float, float)>(),

        // Propiedades
        "x", &glm::vec4::x,
        "y", &glm::vec4::y,
        "z", &glm::vec4::z,
        "w", &glm::vec4::w,

        // Operadores
        sol::meta_function::addition, [](const glm::vec4& a, const glm::vec4& b) { return a + b; },
        sol::meta_function::subtraction, [](const glm::vec4& a, const glm::vec4& b) { return a - b; },
        sol::meta_function::multiplication, sol::overload(
            [](const glm::vec4& a, const glm::vec4& b) { return a * b; },
            [](const glm::vec4& a, float b) { return a * b; },
            [](float a, const glm::vec4& b) { return a * b; }
        ),
        sol::meta_function::division, sol::overload(
            [](const glm::vec4& a, const glm::vec4& b) { return a / b; },
            [](const glm::vec4& a, float b) { return a / b; }
        ),
        sol::meta_function::unary_minus, [](const glm::vec4& a) { return -a; },
        sol::meta_function::to_string, [](const glm::vec4& v) {
            return "vec4(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ", " +
                std::to_string(v.z) + ", " + std::to_string(v.w) + ")";
        },

        // M�todos
        "length", [](const glm::vec4& v) { return glm::length(v); },
        "normalize", [](const glm::vec4& v) { return glm::normalize(v); },
        "dot", [](const glm::vec4& a, const glm::vec4& b) { return glm::dot(a, b); }
    );

    // ===== FUNCIONES MATEM�TICAS GLOBALES =====

    // Funciones de creaci�n de quaterniones
    lua.set_function("quatFromAxisAngle", [](const glm::vec3& axis, float angle) {
        return glm::angleAxis(angle, axis);
        });

    lua.set_function("quatFromEuler", [](float x, float y, float z) {
        return glm::quat(glm::vec3(x, y, z));
        });

    lua.set_function("quatLookAt", [](const glm::vec3& direction, const glm::vec3& up) {
        return glm::quatLookAt(glm::normalize(direction), up);
        });

    // Funciones de transformaci�n de matrices
    lua.set_function("translate", [](const glm::mat4& m, const glm::vec3& v) {
        return glm::translate(m, v);
        });

    lua.set_function("rotate", [](const glm::mat4& m, float angle, const glm::vec3& axis) {
        return glm::rotate(m, angle, axis);
        });

    lua.set_function("scale", [](const glm::mat4& m, const glm::vec3& v) {
        return glm::scale(m, v);
        });

    // Funciones de proyecci�n
    lua.set_function("perspective", [](float fovy, float aspect, float near, float far) {
        return glm::perspective(fovy, aspect, near, far);
        });

    lua.set_function("ortho", [](float left, float right, float bottom, float top, float near, float far) {
        return glm::ortho(left, right, bottom, top, near, far);
        });

    lua.set_function("lookAt", [](const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up) {
        return glm::lookAt(eye, center, up);
        });

    // Funciones matem�ticas generales
    lua.set_function("radians", [](float degrees) { return glm::radians(degrees); });
    lua.set_function("degrees", [](float radians) { return glm::degrees(radians); });
    lua.set_function("mix", sol::overload(
        [](float a, float b, float t) { return glm::mix(a, b, t); },
        [](const glm::vec2& a, const glm::vec2& b, float t) { return glm::mix(a, b, t); },
        [](const glm::vec3& a, const glm::vec3& b, float t) { return glm::mix(a, b, t); },
        [](const glm::vec4& a, const glm::vec4& b, float t) { return glm::mix(a, b, t); }
    ));
    lua.set_function("clamp", sol::overload(
        [](float x, float minVal, float maxVal) { return glm::clamp(x, minVal, maxVal); },
        [](const glm::vec2& x, float minVal, float maxVal) { return glm::clamp(x, minVal, maxVal); },
        [](const glm::vec3& x, float minVal, float maxVal) { return glm::clamp(x, minVal, maxVal); }
    ));
    lua.set_function("smoothstep", [](float edge0, float edge1, float x) {
        return glm::smoothstep(edge0, edge1, x);
        });

    // Constantes matem�ticas
    lua["PI"] = glm::pi<float>();
    lua["TWO_PI"] = glm::two_pi<float>();
    lua["HALF_PI"] = glm::half_pi<float>();
    lua["EPSILON"] = glm::epsilon<float>();

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

    // ===== BOUNDING STRUCTURES =====
    lua.new_usertype<BoundingSphere>("BoundingSphere",
        sol::constructors<BoundingSphere()>(),
        "center", &BoundingSphere::center,
        "radius", &BoundingSphere::radius
    );

    lua.new_usertype<BoundingBox>("BoundingBox",
        sol::constructors<BoundingBox()>(),
        "min", &BoundingBox::min,
        "max", &BoundingBox::max
    );

    // ===== GAMEOBJECT REGISTRATION =====
    lua.new_usertype<GameObject>("GameObject",
        // Constructores
        sol::constructors<
        GameObject(),
        GameObject(const std::string&),
        GameObject(const std::string&, std::shared_ptr<Material>)
        >(),

        // ===== BASIC PROPERTIES =====
        "Name", &GameObject::Name,
        "Tag", &GameObject::Tag,
        "ObjectID", sol::readonly(&GameObject::ObjectID),
        "ModelPath", sol::readonly(&GameObject::ModelPath),

        // ===== VALIDATION =====
        "isValid", &GameObject::isValid,
        "destroy", &GameObject::destroy,

        // ===== LOCAL TRANSFORM =====
        "setLocalPosition", &GameObject::setLocalPosition,
        "setLocalScale", &GameObject::setLocalScale,
        "setLocalRotationEuler", &GameObject::setLocalRotationEuler,
        "setLocalRotationQuat", &GameObject::setLocalRotationQuat,

        "getLocalPosition", &GameObject::getLocalPosition,
        "getLocalScale", &GameObject::getLocalScale,
        "getLocalRotationQuat", &GameObject::getLocalRotationQuat,
        "getLocalRotationEuler", &GameObject::getLocalRotationEuler,

        // ===== WORLD TRANSFORM =====
        "setWorldPosition", &GameObject::setWorldPosition,
        "setWorldScale", &GameObject::setWorldScale,
        "setWorldRotationEuler", &GameObject::setWorldRotationEuler,
        "setWorldRotationQuat", &GameObject::setWorldRotationQuat,

        "getWorldPosition", &GameObject::getWorldPosition,
        "getWorldScale", &GameObject::getWorldScale,
        "getWorldRotationQuat", &GameObject::getWorldRotationQuat,
        "getWorldRotationEuler", &GameObject::getWorldRotationEuler,

        // ===== MATRIX OPERATIONS =====
        "getLocalModelMatrix", &GameObject::getLocalModelMatrix,
        "getWorldModelMatrix", &GameObject::getWorldModelMatrix,
        "getWorldToLocalMatrix", &GameObject::getWorldToLocalMatrix,

        // ===== HIERARCHY SYSTEM =====
        "setParent", [](GameObject* self, GameObject* parent) {
            if (!self) {
                std::cout << "[Lua Error] GameObject is null in setParent" << std::endl;
                return;
            }
            self->setParent(parent);
        },
        "getParent", &GameObject::getParent,
        "hasParent", &GameObject::hasParent,

        "addChild", [](GameObject* self, GameObject* child) {
            if (!self) {
                std::cout << "[Lua Error] GameObject is null in addChild" << std::endl;
                return;
            }
            if (!child) {
                std::cout << "[Lua Error] Child GameObject is null in addChild" << std::endl;
                return;
            }
            self->addChild(child);
        },
        "removeChild", [](GameObject* self, GameObject* child) {
            if (!self) {
                std::cout << "[Lua Error] GameObject is null in removeChild" << std::endl;
                return;
            }
            if (!child) {
                std::cout << "[Lua Error] Child GameObject is null in removeChild" << std::endl;
                return;
            }
            self->removeChild(child);
        },
        "getChildCount", &GameObject::getChildCount,
        "getChild", [](GameObject* self, int index) -> GameObject* {
            if (!self) {
                std::cout << "[Lua Error] GameObject is null in getChild" << std::endl;
                return nullptr;
            }
            return self->getChild(index);
        },

        // Hierarchy search methods
        "findChild", [](GameObject* self, const std::string& name) -> GameObject* {
            if (!self) return nullptr;
            return self->findChild(name);
        },
        "findChildRecursive", [](GameObject* self, const std::string& name) -> GameObject* {
            if (!self) return nullptr;
            return self->findChildRecursive(name);
        },

        // Hierarchy validation
        "isChildOf", [](GameObject* self, GameObject* potentialParent) -> bool {
            if (!self || !potentialParent) return false;
            return self->isChildOf(potentialParent);
        },
        "isParentOf", [](GameObject* self, GameObject* potentialChild) -> bool {
            if (!self || !potentialChild) return false;
            return self->isParentOf(potentialChild);
        },

        // ===== GEOMETRY AND MATERIAL =====
        "setModelPath", &GameObject::setModelPath,
        "loadModelFromPath", sol::overload(
            [](GameObject* self) -> bool {
                if (!self) return false;
                return self->loadModelFromPath();
            },
            [](GameObject* self, const std::string& path) -> bool {
                if (!self) return false;
                return self->loadModelFromPath(path);
            }
        ),
        "hasGeometry", &GameObject::hasGeometry,
        "setMaterial", &GameObject::setMaterial,
        "getMaterial", &GameObject::getMaterial,

        // ===== RENDER CONTROL =====
        "setRenderEnabled", &GameObject::setRenderEnabled,
        "isRenderEnabled", &GameObject::isRenderEnabled,
        "setTransformUpdateEnabled", &GameObject::setTransformUpdateEnabled,
        "isTransformUpdateEnabled", &GameObject::isTransformUpdateEnabled,

        // ===== BOUNDING VOLUMES =====
        "getWorldBoundingSphere", &GameObject::getWorldBoundingSphere,
        "getLocalBoundingBox", &GameObject::getLocalBoundingBox,
        "getWorldBoundingBox", &GameObject::getWorldBoundingBox,
        "getWorldBoundingBoxMin", &GameObject::getWorldBoundingBoxMin,
        "getWorldBoundingBoxMax", &GameObject::getWorldBoundingBoxMax,
        "setBoundingRadius", &GameObject::setBoundingRadius,
        "calculateBoundingVolumes", &GameObject::calculateBoundingVolumes,

        // ===== PHYSICS LAYERS =====
        "getLayer", &GameObject::getLayer,
        "setLayer", &GameObject::setLayer,
        "getLayerMask", &GameObject::getLayerMask,
        "setLayerMask", &GameObject::setLayerMask,

        // ===== COMPONENT SYSTEM =====
        "getAllComponents", [](GameObject* self, sol::this_state s) -> sol::table {
            if (!self) {
                std::cout << "[Lua Error] GameObject is null in getAllComponents" << std::endl;
                return sol::make_object(s, sol::lua_nil); // return nil
            }

            auto components = self->getAllComponents();
            sol::state_view lua(s);
            sol::table result = lua.create_table();

            for (size_t i = 0; i < components.size(); ++i) {
                result[i + 1] = components[i];
            }

            return result;
            },

        // Note: Template methods like addComponent, getComponent, removeComponent
        // need to be registered separately for each specific component type
        // This would be done in a separate function or with specific bindings

        // ===== UPDATE =====
        "update", [](GameObject* self, float deltaTime) {
            if (!self) {
                std::cout << "[Lua Error] GameObject is null in update" << std::endl;
                return;
            }
            self->update(deltaTime);
        },

        "getComponent", [](GameObject* self, const std::string& typeName) -> Component* {
            if (!self) {
                std::cout << "[Lua Error] GameObject is null in getComponent" << std::endl;
                return nullptr;
            }

            // Comparación por nombre del tipo
            if (typeName == "CharacterController") {
                return self->getComponent<CharacterController>();
            }
            if (typeName == "PhysicalObject") {
                return self->getComponent<PhysicalObject>();
            }
            if (typeName == "LightComponent") {
                return self->getComponent<LightComponent>();
            }
            if (typeName == "AudioSource") {
                return self->getComponent<AudioSource>();
            }
            if (typeName == "ScriptExecutor") {
                return self->getComponent<ScriptExecutor>();
            }

            std::cout << "[Lua Warning] getComponent: type not recognized -> " << typeName << std::endl;
            return nullptr;
        }
    );


    // ===== GLOBAL HELPER FUNCTIONS =====
    lua.set_function("createGameObject", sol::overload(
        []() -> GameObject* {
            return new GameObject();
        },
        [](const std::string& modelPath) -> GameObject* {
            return new GameObject(modelPath);
        },
        [](const std::string& modelPath, std::shared_ptr<Material> material) -> GameObject* {
            return new GameObject(modelPath, material);
        }
    ));

    lua.set_function("destroyGameObject", [](GameObject* obj) {
        if (obj) {
            obj->destroy();
            delete obj; // Note: En un sistema real, esto deber�a ser manejado por un manager
        }
        });

    // Helper functions for common operations
    lua.set_function("setGameObjectTransform", [](GameObject* obj, const glm::vec3& pos,
        const glm::vec3& rot, const glm::vec3& scale) {
            if (!obj) return;
            obj->setLocalPosition(pos);
            obj->setLocalRotationEuler(rot);
            obj->setLocalScale(scale);
        });

    lua.set_function("getGameObjectTransform",
        [](GameObject* obj, sol::this_state s) -> sol::table {
            if (!obj) {
                // Devolver nil si obj es null
                return sol::make_object(s, sol::lua_nil);
            }

            sol::state_view lua(s);
            sol::table result = lua.create_table();

            result["position"] = obj->getLocalPosition();
            result["rotation"] = obj->getLocalRotationEuler();
            result["scale"] = obj->getLocalScale();

            return result;
        }
    );

    std::cout << "[Lua] GameObject system registered successfully" << std::endl;
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

void CoreWrapper::RegisterPhysicalObject(sol::state& lua) {
    // ===== BODY TYPE ENUM =====
    lua.new_enum<BodyType>("BodyType", {
        {"Static", BodyType::Static},
        {"Dynamic", BodyType::Dynamic},
        {"Kinematic", BodyType::Kinematic}
    });

    // ===== SHAPE TYPE ENUM =====
    lua.new_enum<ShapeType>("ShapeType", {
        {"Box", ShapeType::Box},
        {"Sphere", ShapeType::Sphere},
        {"Capsule", ShapeType::Capsule},
        {"Plane", ShapeType::Plane}
    });

    // ===== PHYSICAL OBJECT REGISTRATION =====
    lua.new_usertype<PhysicalObject>("PhysicalObject",
        // ===== INITIALIZATION =====
        "initializePhysics", &PhysicalObject::initializePhysics,
        "isInitialized", &PhysicalObject::isInitialized,

        // ===== PHYSICS PROPERTIES =====
        "setMass", &PhysicalObject::setMass,
        "getMass", &PhysicalObject::getMass,
        "setVelocity", &PhysicalObject::setVelocity,
        "getVelocity", &PhysicalObject::getVelocity,
        "setDamping", &PhysicalObject::setDamping,
        "getDamping", &PhysicalObject::getDamping,
        "setFriction", &PhysicalObject::setFriction,
        "getFriction", &PhysicalObject::getFriction,
        "setRestitution", &PhysicalObject::setRestitution,
        "getRestitution", &PhysicalObject::getRestitution,
        "setGravityFactor", &PhysicalObject::setGravityFactor,
        "getGravityFactor", &PhysicalObject::getGravityFactor,

        // ===== BODY TYPE =====
        "setBodyType", &PhysicalObject::setBodyType,
        "getBodyType", &PhysicalObject::getBodyType,

        // ===== SHAPE PROPERTIES =====
        "setShapeType", &PhysicalObject::setShapeType,
        "getShapeType", &PhysicalObject::getShapeType,
        "setBoxHalfExtents", &PhysicalObject::setBoxHalfExtents,
        "getBoxHalfExtents", &PhysicalObject::getBoxHalfExtents,
        "setSphereRadius", &PhysicalObject::setSphereRadius,
        "getSphereRadius", &PhysicalObject::getSphereRadius,
        "setCapsuleRadius", &PhysicalObject::setCapsuleRadius,
        "getCapsuleRadius", &PhysicalObject::getCapsuleRadius,
        "setCapsuleHalfHeight", &PhysicalObject::setCapsuleHalfHeight,
        "getCapsuleHalfHeight", &PhysicalObject::getCapsuleHalfHeight,

        // ===== TRIGGER PROPERTIES =====
        "setTrigger", &PhysicalObject::setTrigger,
        "isTrigger", &PhysicalObject::isTrigger,

        // ===== FORCES AND IMPULSES =====
        "addForce", &PhysicalObject::addForce,
        "addTorque", &PhysicalObject::addTorque,
        "addImpulse", &PhysicalObject::addImpulse,

        // ===== STATE =====
        "wakeUp", &PhysicalObject::wakeUp,
        "isAwake", &PhysicalObject::isAwake,

        // ===== LAYER CONFIGURATION =====
        "setLayer", &PhysicalObject::setLayer,
        "getLayer", &PhysicalObject::getLayer,
        "setLayerMask", &PhysicalObject::setLayerMask,
        "getLayerMask", &PhysicalObject::getLayerMask
    );

    std::cout << "[Lua] PhysicalObject system registered successfully" << std::endl;
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

#pragma once

#include <vector>
#include <map>
#include <memory>
#include <unordered_map>
#include <string>
#include <glm/glm.hpp>
#include "../core/CoreExporter.h"
#include "../ui/Canvas.h"

class Camera;
class DefaultShaders;
class Material;
class Light;
class Frustum;
class Framebuffer;
class AssimpGeometry;
class ShadowManager;

class GameObject;


class MANTRAXCORE_API RenderPipeline {
public:
    RenderPipeline(Camera* camera, DefaultShaders* shaders);
    ~RenderPipeline();

    void AddGameObject(GameObject* object);
    void RemoveGameObject(GameObject* object);
    void AddLight(std::shared_ptr<Light> light);
    void RemoveLight(std::shared_ptr<Light> light);
    void renderFrame();
    void renderToFramebuffer(Framebuffer* framebuffer);
    void renderToScreen(); // Force render to main screen, ignoring camera framebuffer
    
    // Scene management methods
    void clearGameObjects();
    void clearLights();
    void setCamera(Camera* newCamera) { camera = newCamera; }
    Camera* getCamera() const;
    void setTargetFramebuffer(Framebuffer* framebuffer) { targetFramebuffer = framebuffer; }
    
    // PBR/Blinn-Phong toggle
    void setUsePBR(bool enabled);
    bool getUsePBR() const;
    
    // Ambient light control
    void setLowAmbient(bool enabled);
    bool getLowAmbient() const;
    void setAmbientIntensity(float intensity);
    float getAmbientIntensity() const;
    
    // Frustum culling control
    void setFrustumCulling(bool enabled);
    bool getFrustumCulling() const;
    
    // Shaders access
    DefaultShaders* getShaders() const { return shaders; }
    
    // Shadow mapping control
    void enableShadows(bool enabled);
    bool getShadowsEnabled() const;
    void setShadowMapSize(int size);
    int getShadowMapSize() const;
    void setShadowBias(float bias);
    float getShadowBias() const;
    void setShadowStrength(float strength);
    float getShadowStrength() const;
    ShadowManager* getShadowManager() const { return shadowManager; }
    
    int getVisibleObjectsCount() const;
    int getTotalObjectsCount() const;
    
    // Resource Management
    std::shared_ptr<AssimpGeometry> loadModel(const std::string& path);
    std::shared_ptr<AssimpGeometry> getModel(const std::string& path);
    
    std::shared_ptr<Material> createMaterial(const glm::vec3& albedo, const std::string& name);
    std::shared_ptr<Material> createMaterial(const glm::vec3& albedo);
    std::shared_ptr<Material> createMaterial();
    
    // Material management
    bool loadMaterialsFromConfig(const std::string& configPath);
    std::shared_ptr<Material> getMaterial(const std::string& materialName);
    bool hasMaterial(const std::string& materialName) const;
    void listMaterials() const;
    
    // Canvas management
    Canvas2D* addCanvas(int width, int height);
    void removeCanvas(size_t index);
    Canvas2D* getCanvas(size_t index = 0);
    size_t getCanvasCount() const;
    
    // Canvas size management
    void updateCanvasSize(int width, int height);
    void updateCanvasFromCameraBuffer();
    
    void clearModelCache();
    void listLoadedModels() const;
    size_t getModelCacheSize() const;

    static RenderPipeline& getInstance(Camera* camera = nullptr, DefaultShaders* shaders = nullptr) {
        static RenderPipeline* instance = nullptr;
        if (!instance && camera && shaders) {
            // Solo permite crear la instancia una vez (con argumentos)
            instance = new RenderPipeline(camera, shaders);
        }
        return *instance;
    }

    std::vector<Canvas2D*> _canvas = std::vector<Canvas2D*>();

private:
    std::vector<GameObject*> sceneObjects;
    std::vector<std::shared_ptr<Light>> lights;
    Camera* camera;
    DefaultShaders* shaders;
    Framebuffer* targetFramebuffer; // Optional framebuffer to render to
    bool usePBR; // Flag to switch between PBR and Blinn-Phong
    bool lowAmbient; // Flag to reduce ambient light
    float ambientIntensity; // Ambient light intensity multiplier
    bool frustumCullingEnabled; // Flag para habilitar/deshabilitar frustum culling
    bool shadowsEnabled; // Flag para habilitar/deshabilitar shadow mapping
    ShadowManager* shadowManager; // Shadow mapping manager
    
    int visibleObjectsCount;
    int totalObjectsCount;
    
    // Resource caches
    std::unordered_map<std::string, std::shared_ptr<AssimpGeometry>> modelCache;
    
    std::vector<std::shared_ptr<Material>> materialPool;
    
    // Canvas storage
    
    void renderInstanced();
    void renderNonInstanced();
    void renderShadowPass(); // New method for shadow rendering
    void renderShadowGeometry(); // Helper method for rendering geometry during shadow passes
    void configureMaterial(Material* material);
    void configureDefaultMaterial();
    void rebindShadowMapsAfterMaterial(GLuint program);
    void configureLighting();
    bool isObjectVisible(GameObject* object, const Frustum& cameraFrustum) const;
    
    // Estructura para agrupar objetos por material y geometría
    struct MaterialGeometryKey {
        std::shared_ptr<Material> material;
        		AssimpGeometry* geometry;
        
        bool operator<(const MaterialGeometryKey& other) const {
            if (material != other.material) {
                return material < other.material;
            }
            return geometry < other.geometry;
        }
    };

};

#include "Texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <iostream>
#include "../core/FileSystem.h"

Texture::Texture()
    : rendererID(0), localBuffer(nullptr), width(0), height(0), BPP(0) {
}

Texture::Texture(const std::string& filePath)
    : rendererID(0), localBuffer(nullptr), width(0), height(0), BPP(0) {
    loadFromFile(filePath);
}

Texture::~Texture() {
    if (rendererID != 0) {
        glDeleteTextures(1, &rendererID);
    }
}

bool Texture::loadFromFile(const std::string& filePath) {
    this->filePath = FileSystem::getProjectPath() + "\\Content\\" + filePath;
    
    std::cout << "Texture::loadFromFile: Attempting to load texture from: " << this->filePath << std::endl;
    
    //stbi_set_flip_vertically_on_load(1);
    
    // Determinar si es una textura de datos o color
    bool isDataTexture = (this->filePath.find("Normal") != std::string::npos ||
                          this->filePath.find("Metalness") != std::string::npos ||
                          this->filePath.find("Roughness") != std::string::npos ||
                          this->filePath.find("AO") != std::string::npos ||
                          this->filePath.find("Height") != std::string::npos);
    
    // Cargar con 4 canales (RGBA) para asegurar que siempre tengamos un canal alfa
    localBuffer = stbi_load(this->filePath.c_str(), &width, &height, &BPP, 4);
    if (!localBuffer) {
        std::cerr << "Error: No se pudo cargar la textura: " << this->filePath << std::endl;
        std::cerr << "STB Error: " << stbi_failure_reason() << std::endl;
        
        // Check if file exists
        std::ifstream fileCheck(this->filePath);
        if (!fileCheck.good()) {
            std::cerr << "File does not exist or is not accessible" << std::endl;
        } else {
            std::cerr << "File exists but STB failed to load it" << std::endl;
            fileCheck.close();
        }
        
        return false;
    }
    
    std::cout << "Texture loaded successfully: " << filePath << " (" << width << "x" << height << ", " << BPP << " channels)" << std::endl;
    
    // Asegurarse de que el canal alfa sea 1.0 para todas las texturas
    // Esto evita problemas con objetos que desaparecen
    unsigned char* pixels = (unsigned char*)localBuffer;
    for (int i = 0; i < width * height; i++) {
        pixels[i * 4 + 3] = 255; // Establecer canal alfa a 255 (1.0)
    }

    glGenTextures(1, &rendererID);
    glBindTexture(GL_TEXTURE_2D, rendererID);

    // Configurar parámetros de textura optimizados para PBR
    if (isDataTexture) {
        // Para texturas de datos (normal, metallic, roughness, etc.)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    } else {
        // Para texturas de color (albedo, emissive)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
    
    // Wrapping para texturas que se repiten (como Diamond Plate)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    // Anisotropic filtering para mejor calidad en ángulos oblicuos
    float maxAnisotropy;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, std::min(maxAnisotropy, 16.0f));

    // Cargar la imagen en la textura
    GLenum internalFormat = isDataTexture ? GL_RGBA8 : GL_SRGB8_ALPHA8; // sRGB para texturas de color
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, localBuffer);
    
    // Generar mipmaps para mejor calidad a distancia
    glGenerateMipmap(GL_TEXTURE_2D);
    
    glBindTexture(GL_TEXTURE_2D, 0);

    // Liberar el buffer local ya que OpenGL tiene una copia
    if (localBuffer) {
        stbi_image_free(localBuffer);
        localBuffer = nullptr;
    }

    std::cout << "Textura cargada exitosamente: " << filePath << " (" << width << "x" << height << ")" << std::endl;
    std::cout << "OpenGL Texture ID: " << rendererID << std::endl;
    return true;
}

bool Texture::loadIconFromFile(const std::string& filePath) {
    this->filePath = filePath; // Usa la ruta tal cual
    localBuffer = stbi_load(this->filePath.c_str(), &width, &height, &BPP, 4);
    if (!localBuffer) {
        std::cerr << "Error: No se pudo cargar el icono: " << this->filePath << std::endl;
        std::cerr << "STB Error: " << stbi_failure_reason() << std::endl;
        return false;
    }

    glGenTextures(1, &rendererID);
    glBindTexture(GL_TEXTURE_2D, rendererID);

    // Configurar parámetros de textura optimizados para iconos
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Cargar la imagen en la textura
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, localBuffer);
    
    glBindTexture(GL_TEXTURE_2D, 0);

    // Liberar el buffer local ya que OpenGL tiene una copia
    if (localBuffer) {
        stbi_image_free(localBuffer);
        localBuffer = nullptr;
    }

    return true;
}

void Texture::bind(unsigned int slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, rendererID);
}

void Texture::unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}
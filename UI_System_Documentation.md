# Sistema UI Canvas - MantraxEngine

## Descripción

El sistema UI Canvas de MantraxEngine proporciona una interfaz sencilla para crear elementos de interfaz de usuario utilizando SDL2. El sistema incluye botones, texto e imágenes con soporte completo para eventos y renderizado.

## Características

### Elementos UI Soportados

- **Button**: Botones interactivos con eventos hover, press y click
- **Text**: Renderizado de texto con alineación y colores personalizables
- **Image**: Carga y visualización de imágenes BMP con diferentes modos de escalado

### Funcionalidades

- **Sistema de Canvas**: Gestión centralizada de todos los elementos UI
- **Manejo de Eventos**: Eventos de mouse y teclado integrados con SDL
- **Renderizado**: Renderizado directo con SDL_Renderer
- **IDs Automáticos**: Gestión automática de identificadores únicos
- **Callbacks**: Sistema de callbacks para eventos de botones

## Estructura de Clases

```
UIElement (base class)
├── Button
├── Text
└── Image

Canvas (manager)
UIDemo (example)
UIManager (wrapper)
```

## Uso Básico

### 1. Inicialización

```cpp
#include "ui/UIManager.h"

// En tu función de inicialización
UIManager uiManager;
SDL_Renderer* renderer = ...; // Tu renderer SDL

if (!uiManager.initialize(renderer)) {
    // Error al inicializar
}
```

### 2. Renderizado y Actualización

```cpp
// En tu loop principal
while (running) {
    // Manejo de eventos
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (uiManager.handleEvent(event)) {
            // Evento manejado por UI, no procesar más
            continue;
        }
        // Procesar otros eventos...
    }

    // Actualización
    uiManager.update(deltaTime);

    // Renderizado
    uiManager.render();

    SDL_RenderPresent(renderer);
}
```

### 3. Uso Manual del Canvas

```cpp
#include "ui/Canvas.h"

// Crear canvas
Canvas canvas(renderer);

// Agregar elementos
Button* button = canvas.addButton(glm::vec2(100, 50), glm::vec2(150, 40), "Haz Click");
Text* text = canvas.addText(glm::vec2(100, 20), "Hola Mundo!");
Image* image = canvas.addImage(glm::vec2(300, 50), glm::vec2(100, 100), "imagen.bmp");

// Configurar eventos
button->setOnClick([]() {
    std::cout << "¡Botón presionado!" << std::endl;
});

// En el loop de renderizado
canvas.render();
canvas.update(deltaTime);
canvas.handleEvent(event);
```

## Ejemplos Detallados

### Botón Personalizado

```cpp
Button* customButton = canvas.addButton(glm::vec2(50, 100), glm::vec2(200, 50), "Mi Botón");

// Personalizar colores
customButton->setBackgroundColor(70, 130, 180);    // Color normal
customButton->setHoverColor(100, 149, 237);        // Color al pasar mouse
customButton->setPressedColor(50, 100, 150);       // Color al presionar
customButton->setTextColor(255, 255, 255);         // Color del texto

// Eventos
customButton->setOnClick([]() {
    std::cout << "¡Click!" << std::endl;
});

customButton->setOnHover([]() {
    std::cout << "Mouse encima" << std::endl;
});

customButton->setOnPress([]() {
    std::cout << "Botón presionado" << std::endl;
});
```

### Texto con Formato

```cpp
Text* title = canvas.addText(glm::vec2(50, 30), "Título Principal");
title->setTextColor(255, 255, 255);
title->setFontSize(24);
title->setAlignment(TextAlignment::CENTER);

Text* description = canvas.addText(glm::vec2(50, 60), "Descripción");
description->setTextColor(200, 200, 200);
description->setFontSize(16);
description->setAutoSize(true);
```

### Imagen con Escalado

```cpp
Image* logo = canvas.addImage(glm::vec2(400, 50), glm::vec2(200, 100));

// Cargar imagen
logo->loadImage(renderer, "assets/logo.bmp");

// Configurar escalado
logo->setScaleMode(ImageScaleMode::KEEP_ASPECT);

// Aplicar tinte
logo->setTint(255, 255, 255, 200); // Transparencia

// Configurar opacidad
logo->setOpacity(128);
```

## Gestión de Elementos

### Buscar Elementos

```cpp
// Por ID automático
UIElement* element = canvas.getElementById("element_1");

// Por tipo específico
Button* button = canvas.getButtonById("element_2");
Text* text = canvas.getTextById("element_3");
Image* image = canvas.getImageById("element_4");
```

### Remover Elementos

```cpp
// Por ID
canvas.removeElement("element_1");

// Por puntero
canvas.removeElement(button);

// Limpiar todo
canvas.clearAllElements();
```

### Propiedades del Canvas

```cpp
// Configurar fondo
canvas.setBackgroundColor(50, 50, 50, 255);
canvas.clearBackground(true);

// Información
size_t count = canvas.getElementCount();
bool empty = canvas.isEmpty();
```

## Controles de Demo

El sistema incluye una demo interactiva con los siguientes controles:

- **"Click Me!"**: Botón que cuenta clicks y cambia colores
- **"Button 2"**: Alterna la visibilidad de una imagen
- **"Hide UI"**: Oculta/muestra toda la interfaz
- **Texto dinámico**: Se actualiza según las interacciones
- **Imagen de prueba**: Muestra placeholder con tinte azul

### Activar/Desactivar Demo

```cpp
uiManager.toggleDemo();
bool visible = uiManager.isDemoVisible();
```

## Limitaciones Actuales

- **Texto**: Renderizado básico sin SDL_ttf (patrones simples)
- **Imágenes**: Solo formato BMP nativo de SDL (sin SDL_image)
- **Fuentes**: No hay soporte para fuentes TrueType personalizadas
- **Layouts**: No hay sistema automático de layouts

## Mejoras Futuras

1. **Integración SDL_ttf**: Para renderizado de texto real
2. **Integración SDL_image**: Soporte PNG, JPG, etc.
3. **Sistema de Layouts**: Grid, Stack, Flow layouts
4. **Más Elementos**: CheckBox, RadioButton, Slider, TextInput
5. **Temas**: Sistema de temas personalizables
6. **Animaciones**: Transiciones y efectos animados

## Compilación

Los archivos UI están organizados en:

```
src/ui/
├── UIElement.h/cpp     (Clase base)
├── Button.h/cpp        (Elemento botón)
├── Text.h/cpp          (Elemento texto)
├── Image.h/cpp         (Elemento imagen)
├── Canvas.h/cpp        (Gestor principal)
├── UIDemo.h/cpp        (Demo interactiva)
└── UIManager.h/cpp     (Wrapper simple)
```

Los archivos están configurados en el proyecto Visual Studio bajo el filtro **03_UI**.

¡El sistema UI Canvas está listo para usar!

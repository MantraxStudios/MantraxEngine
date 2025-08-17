# Sistema de UI para MantraxEngine

Este sistema de UI proporciona una interfaz completa para crear elementos de interfaz de usuario en el motor MantraxEngine.

## Estructura del Sistema

### Canvas2D
- **Archivo**: `Canvas.h` / `Canvas.cpp`
- **Propósito**: Maneja el renderizado 2D de elementos UI
- **Características**:
  - Renderizado de texto con FreeType
  - Sistema de anclajes (Anchor) para posicionamiento
  - Renderizado de quads coloreados
  - Soporte para fuentes personalizadas

### UIBehaviour
- **Archivo**: `UIBehaviour.h` / `UIBehaviour.cpp`
- **Propósito**: Clase principal que gestiona todos los componentes de UI
- **Características**:
  - Gestión de ciclo de vida de componentes
  - Manejo de entrada de mouse
  - Renderizado de componentes

## Componentes Disponibles

### UITransform
Maneja la transformación y posicionamiento de elementos UI.

```cpp
auto transform = std::make_unique<UITransform>(x, y, width, height);
transform->setAnchor(Anchor::Center);
transform->setScale(1.0f, 1.0f);
```

### UIText
Componente para renderizar texto.

```cpp
auto text = std::make_shared<UIText>("Hello World!");
text->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
text->setFontSize(16);
```

### UIButton
Botón interactivo con estados de hover y click.

```cpp
auto button = std::make_shared<UIButton>("Click Me!");
button->setNormalColor(glm::vec3(0.2f, 0.6f, 0.2f));
button->setHoverColor(glm::vec3(0.3f, 0.7f, 0.3f));
button->setOnClickCallback([]() {
    std::cout << "Button clicked!" << std::endl;
});
```

### UIImage
Componente para mostrar imágenes (actualmente renderiza quads coloreados).

```cpp
auto image = std::make_shared<UIImage>("texture.png");
image->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
```

### UIPanel
Contenedor que agrupa otros componentes UI.

```cpp
auto panel = std::make_shared<UIPanel>();
panel->setBackgroundColor(glm::vec3(0.2f, 0.2f, 0.2f));
panel->setBorderColor(glm::vec3(0.5f, 0.5f, 0.5f));
panel->setBorderWidth(2.0f);
panel->addChild(button);
panel->addChild(text);
```

## Sistema de Anclajes (Anchor)

El sistema soporta 9 tipos de anclajes para posicionamiento:

- `TopLeft` - Esquina superior izquierda
- `TopCenter` - Centro superior
- `TopRight` - Esquina superior derecha
- `CenterLeft` - Centro izquierda
- `Center` - Centro
- `CenterRight` - Centro derecha
- `BottomLeft` - Esquina inferior izquierda
- `BottomCenter` - Centro inferior
- `BottomRight` - Esquina inferior derecha

## Ejemplo de Uso Básico

```cpp
#include "UIBehaviour.h"
#include "Canvas.h"

// Crear canvas
auto canvas = std::make_unique<Canvas2D>(800, 600);
canvas->loadFont("fonts/Ubuntu-Regular.ttf", 16);

// Crear UI behaviour
auto ui = std::make_unique<UIBehaviour>();
auto transform = std::make_unique<UITransform>(0, 0, 800, 600);
ui->setTransform(transform.get());

// Crear un botón
auto button = std::make_shared<UIButton>("Hello World!");
auto buttonTransform = std::make_unique<UITransform>(100, 100, 200, 50);
button->setTransform(buttonTransform.get());
button->setOnClickCallback([]() {
    std::cout << "Button clicked!" << std::endl;
});

// Agregar a UI
ui->addComponent(button);
ui->Start();

// En el bucle de renderizado
canvas->clear(glm::vec3(0.1f, 0.1f, 0.1f));
ui->Render(canvas.get());

// Manejar entrada de mouse
ui->handleMouseInput(mouseX, mouseY, mousePressed);
```

## Ciclo de Vida de los Componentes

1. **Start()**: Llamado una vez al inicializar el componente
2. **Update()**: Llamado cada frame para actualizar lógica
3. **Render()**: Llamado cada frame para renderizar
4. **OnClick()**: Llamado cuando se hace click en el componente
5. **OnHover()**: Llamado cuando el mouse entra en el componente
6. **OnHoverExit()**: Llamado cuando el mouse sale del componente

## Características Avanzadas

### Gestión de Estados
Los componentes pueden ser habilitados/deshabilitados:

```cpp
component->setEnabled(false);
if (component->isEnabled()) {
    // Componente está activo
}
```

### Callbacks Personalizados
Los botones soportan callbacks personalizados:

```cpp
button->setOnClickCallback([]() {
    // Código personalizado aquí
});
```

### Jerarquía de Componentes
Los paneles pueden contener otros componentes:

```cpp
auto panel = std::make_shared<UIPanel>();
panel->addChild(button);
panel->addChild(text);
ui->addComponent(panel);
```

## Integración con el Motor

Para integrar este sistema con el motor principal:

1. Incluir los archivos de UI en tu proyecto
2. Crear una instancia de `UIBehaviour` en tu clase principal
3. Llamar a `Start()`, `Update()`, y `Render()` en el bucle principal
4. Conectar el sistema de entrada del motor con `handleMouseInput()`

## Dependencias

- OpenGL (GLEW)
- GLM (matemáticas)
- FreeType (fuentes)
- STL (contenedores estándar)

## Notas de Implementación

- El sistema está diseñado para ser extensible
- Los componentes pueden ser fácilmente agregados heredando de `UIComponent`
- El sistema de transformaciones es independiente y puede ser extendido
- El renderizado está optimizado para 2D con blending apropiado 
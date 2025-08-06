# Canvas Manager Window

## Descripción

La ventana **Canvas Manager** es una herramienta completa para administrar múltiples canvas 2D y sus componentes UI. Permite crear, editar, eliminar y organizar elementos de interfaz de usuario de manera visual e intuitiva.

## Características Principales

### 🎨 Gestión de Canvas
- **Crear múltiples canvas** con diferentes tamaños
- **Seleccionar y editar** canvas individuales
- **Eliminar canvas** no deseados
- **Sincronización automática** con RenderPipeline

### 🧩 Componentes UI Soportados
- **Text**: Elementos de texto con fuentes personalizables
- **Button**: Botones interactivos con callbacks
- **Image**: Imágenes y texturas
- **Panel**: Contenedores para organizar otros elementos

### 🎯 Funcionalidades Avanzadas
- **Editor visual** con interfaz ImGui
- **Propiedades en tiempo real** para cada componente
- **Sistema de transformaciones** (posición, escala, rotación)
- **Gestión de colores** y estilos
- **Integración completa** con el sistema de renderizado

## Cómo Usar

### 1. Abrir la Ventana
La ventana "Canvas Manager" se encuentra en el editor junto con otras ventanas como Hierarchy, Inspector, etc.

### 2. Crear un Canvas
1. En la sección "Create Canvas" (izquierda)
2. Configurar el ancho y alto deseados
3. Hacer clic en "Add Canvas" o "Add Default Canvas"

### 3. Seleccionar un Canvas
1. En la lista "Canvas List" (izquierda)
2. Hacer clic en el canvas deseado
3. El canvas seleccionado se resalta

### 4. Añadir Componentes
1. Seleccionar un canvas
2. En la sección "Create Component" (derecha)
3. Elegir el tipo de componente (Text, Button, Image, Panel)
4. Configurar posición y tamaño
5. Añadir propiedades específicas del componente
6. Hacer clic en "Add Component"

### 5. Editar Componentes
1. Seleccionar un canvas
2. En la lista "Components" (derecha)
3. Hacer clic en el componente deseado
4. Editar propiedades en "Component Properties"

### 6. Sincronizar con RenderPipeline
1. Hacer clic en "Sync with RenderPipeline"
2. Los canvas se sincronizan automáticamente
3. Los componentes se renderizan en el juego

## Interfaz de Usuario

### Panel Izquierdo - Gestión de Canvas
```
┌─────────────────────────┐
│ Canvas List             │
│ ├─ Canvas 0 (1920x1080)│
│ ├─ Canvas 1 (800x600)  │
│ └─ Canvas 2 (400x300)  │
├─────────────────────────┤
│ Create Canvas           │
│ Width: [1920]          │
│ Height: [1080]         │
│ [Add Canvas] [Default] │
├─────────────────────────┤
│ [Sync with RenderPipeline]
│ [Render All Components] │
└─────────────────────────┘
```

### Panel Derecho - Gestión de Componentes
```
┌─────────────────────────┐
│ Components              │
│ ├─ Text: Hello World    │
│ ├─ Button: Start        │
│ ├─ Image: logo.png      │
│ └─ Panel                │
├─────────────────────────┤
│ Create Component        │
│ Type: [Text ▼]         │
│ X: [0.0] Y: [0.0]      │
│ Width: [100] Height: [50]
│ Text: [New Text]       │
│ [Add Component]         │
├─────────────────────────┤
│ Component Properties    │
│ Position: [0, 0, 0]    │
│ Scale: [1, 1, 1]       │
│ Color: [1, 1, 1, 1]   │
└─────────────────────────┘
```

## Tipos de Componentes

### Text Component
- **Propósito**: Mostrar texto en pantalla
- **Propiedades**:
  - Text: Contenido del texto
  - Color: Color del texto (RGBA)
  - Font: Fuente a utilizar
  - Size: Tamaño de la fuente

### Button Component
- **Propósito**: Botones interactivos
- **Propiedades**:
  - Text: Texto del botón
  - Color: Color del botón
  - Hover Color: Color al pasar el mouse
  - Click Callback: Función al hacer clic

### Image Component
- **Propósito**: Mostrar imágenes y texturas
- **Propiedades**:
  - Image Path: Ruta de la imagen
  - Tiling: Repetición de la textura
  - Filter: Filtro de la imagen

### Panel Component
- **Propósito**: Contenedor para otros elementos
- **Propiedades**:
  - Color: Color de fondo
  - Border: Borde del panel
  - Children: Elementos hijos

## Integración con el Sistema

### RenderPipeline
```cpp
// Sincronizar canvas con RenderPipeline
canvasManager->SyncWithRenderPipeline();

// Renderizar todos los componentes
canvasManager->RenderAllComponents();
```

### UIBehaviour System
```cpp
// Obtener canvas específico
Canvas2D* canvas = canvasManager->GetCanvas(0);

// Obtener componentes de un canvas
auto& components = canvasManager->GetComponents(0);
for (auto* component : components) {
    component->Update();
    component->Render();
}
```

## Ejemplos de Uso

### Crear un HUD Completo
```cpp
// 1. Crear canvas para HUD
canvasManager->AddCanvas(800, 600);

// 2. Seleccionar el canvas
canvasManager->SelectCanvas(0);

// 3. Añadir elementos HUD
canvasManager->AddComponent(ComponentType::Text);   // Health
canvasManager->AddComponent(ComponentType::Text);   // Score
canvasManager->AddComponent(ComponentType::Button); // Pause
canvasManager->AddComponent(ComponentType::Image);  // Mini-map

// 4. Sincronizar con el sistema
canvasManager->SyncWithRenderPipeline();
```

### Crear Menú Principal
```cpp
// 1. Crear canvas para menú
canvasManager->AddCanvas(1920, 1080);

// 2. Seleccionar canvas
canvasManager->SelectCanvas(0);

// 3. Añadir elementos del menú
canvasManager->AddComponent(ComponentType::Text);   // Título
canvasManager->AddComponent(ComponentType::Button); // Start Game
canvasManager->AddComponent(ComponentType::Button); // Settings
canvasManager->AddComponent(ComponentType::Button); // Exit
canvasManager->AddComponent(ComponentType::Image);  // Logo

// 4. Renderizar
canvasManager->RenderAllComponents();
```

## Consejos y Mejores Prácticas

### Organización de Canvas
- **Canvas Principal**: Para menús y UI principal (1920x1080)
- **Canvas HUD**: Para elementos de juego (800x600)
- **Canvas Mini-map**: Para mapas pequeños (400x300)
- **Canvas Overlay**: Para notificaciones (1920x1080)

### Nomenclatura de Componentes
- Usar nombres descriptivos: "Health Text", "Start Button"
- Agrupar componentes relacionados
- Mantener consistencia en el nombramiento

### Optimización
- Reutilizar componentes cuando sea posible
- Usar el sistema de transformaciones para posicionar
- Sincronizar solo cuando sea necesario

## Troubleshooting

### Problemas Comunes

**Canvas no se renderiza**
- Verificar que esté sincronizado con RenderPipeline
- Comprobar que el canvas tenga componentes
- Revisar las rutas de fuentes

**Componentes no aparecen**
- Verificar que el canvas esté seleccionado
- Comprobar las propiedades de transformación
- Revisar que el componente tenga un transform válido

**Errores de fuentes**
- Verificar que las rutas de fuentes sean correctas
- Comprobar que los archivos .ttf existan
- Revisar los permisos de archivos

## Archivos Relacionados

- `CanvasManager.h` - Header principal
- `CanvasManager.cpp` - Implementación
- `UIBehaviour.h` - Sistema de componentes UI
- `Canvas.h` - Sistema de canvas 2D
- `RenderPipeline.h` - Sistema de renderizado

## Dependencias

- ImGui - Interfaz de usuario
- OpenGL - Renderizado
- FreeType - Fuentes
- GLM - Matemáticas
- STL - Contenedores estándar 
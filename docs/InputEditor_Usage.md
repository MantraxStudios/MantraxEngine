# Input Editor - Guía de Uso

## Descripción

La ventana de Input Editor permite crear, editar, borrar y gestionar todas las configuraciones de input del motor. Esta ventana sigue el patrón de diseño de las otras ventanas del editor (WindowBehaviour) y se integra perfectamente con el sistema existente.

## Características

### Funcionalidades Principales

- **Crear nuevos inputs**: Agregar nuevas acciones de input con diferentes tipos
- **Editar inputs existentes**: Modificar nombres, tipos y bindings de inputs
- **Borrar inputs**: Eliminar acciones de input no deseadas
- **Gestionar bindings**: Agregar, editar y eliminar bindings de teclado y mouse
- **Guardar/Cargar configuración**: Persistencia en formato JSON

### Tipos de Input Soportados

- **Button**: Para acciones de presionar/soltar (como saltar, disparar)
- **Value**: Para valores continuos (como movimiento vertical)
- **Vector2D**: Para movimiento 2D (como WASD)
- **MouseButton**: Para botones del mouse
- **MouseAxis**: Para movimiento del mouse y rueda

## Interfaz de Usuario

### Panel Principal

La ventana está dividida en dos columnas:

- **Columna izquierda**: Lista de todas las acciones de input registradas
- **Columna derecha**: Editor de la acción seleccionada

### Barra de Herramientas

- **Save Config**: Guarda la configuración actual en `config/input_config.json`
- **Load Config**: Carga la configuración desde `config/input_config.json`
- **Add New Input**: Abre el diálogo para crear una nueva acción de input

### Editor de Acciones

Cuando seleccionas una acción de input, puedes:

- **Editar nombre**: Cambiar el nombre de la acción
- **Cambiar tipo**: Seleccionar entre los diferentes tipos de input
- **Gestionar bindings**: Ver, editar y eliminar bindings existentes
- **Agregar bindings**: Añadir nuevos bindings de teclado o mouse
- **Aplicar cambios**: Guardar las modificaciones
- **Eliminar acción**: Borrar la acción seleccionada

## Uso del Sistema

### Crear una Nueva Acción de Input

1. Haz clic en "Add New Input"
2. Introduce el nombre de la acción
3. Selecciona el tipo de input apropiado
4. Haz clic en "Create"

### Editar una Acción Existente

1. Selecciona la acción de la lista en la columna izquierda
2. Modifica el nombre o tipo según necesites
3. Gestiona los bindings usando los botones "Add Key Binding" o "Add Mouse Binding"
4. Haz clic en "Apply Changes" para guardar

### Agregar Bindings

- **Para teclado**: Haz clic en "Add Key Binding" y selecciona la tecla
- **Para mouse**: Haz clic en "Add Mouse Binding" y selecciona el botón o eje

### Eliminar Bindings

Haz clic en el botón "Remove" junto al binding que quieres eliminar.

## Integración con el Sistema

### Carga Automática

El sistema carga automáticamente la configuración desde `config/input_config.json` al iniciar.

### Persistencia

Todas las modificaciones se guardan en el mismo archivo JSON, manteniendo la compatibilidad con el sistema existente.

### Compatibilidad

La ventana es completamente compatible con:

- El sistema de ventanas existente (WindowBehaviour)
- El sistema de input actual (InputSystem)
- El formato JSON existente
- Todas las funcionalidades del motor

## Archivos Modificados/Creados

### Nuevos Archivos

- `editor/Windows/InputEditor.h`: Header de la ventana
- `editor/Windows/InputEditor.cpp`: Implementación de la ventana
- `docs/InputEditor_Usage.md`: Esta documentación

### Archivos Modificados

- `editor/Windows/RenderWindows.h`: Agregada la nueva ventana al sistema
- `src/input/InputSystem.h`: Agregados métodos para iterar y gestionar acciones
- `src/input/InputAction.h`: Agregados métodos para modificar bindings
- `src/core/InputConfigLoader.h`: Agregada funcionalidad de guardado
- `src/core/InputConfigLoader.cpp`: Implementación del guardado

## Ejemplo de Configuración JSON

```json
{
  "input_actions": [
    {
      "name": "Move",
      "type": "Vector2D",
      "key_bindings": [
        {
          "key": "SDLK_w",
          "positive": true,
          "axis": 1
        },
        {
          "key": "SDLK_s",
          "positive": false,
          "axis": 1
        },
        {
          "key": "SDLK_d",
          "positive": true,
          "axis": 0
        },
        {
          "key": "SDLK_a",
          "positive": false,
          "axis": 0
        }
      ]
    },
    {
      "name": "Jump",
      "type": "Button",
      "key_bindings": [
        {
          "key": "SDLK_SPACE"
        }
      ]
    },
    {
      "name": "MouseLook",
      "type": "MouseAxis",
      "mouse_axis": "X"
    }
  ]
}
```

## Notas Técnicas

### Arquitectura

La ventana sigue el patrón MVC (Model-View-Controller):

- **Model**: InputSystem y InputAction
- **View**: ImGui interface
- **Controller**: InputEditor

### Rendimiento

- La lista se actualiza dinámicamente
- Los bindings se gestionan eficientemente
- El guardado es asíncrono y no bloquea la UI

### Extensibilidad

El sistema está diseñado para ser fácilmente extensible:

- Nuevos tipos de input pueden agregarse fácilmente
- Nuevos tipos de bindings pueden implementarse
- La interfaz puede personalizarse según necesidades específicas

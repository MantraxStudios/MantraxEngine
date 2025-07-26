# Sistema de Input Configurable desde JSON

## Descripción

El sistema de input del editor ahora puede cargar su configuración desde un archivo JSON, permitiendo una fácil personalización de controles sin necesidad de recompilar el código. El `InputConfigLoader` está en el core y solo se encarga de cargar la configuración, mientras que los callbacks específicos se manejan en el editor.

## Arquitectura

### Core (src/core/)

- `InputConfigLoader.h/cpp` - Carga configuración desde JSON
- `InputSystem.h/cpp` - Sistema de input base
- `InputAction.h` - Definición de acciones de input

### Editor (editor/)

- `Editor.cpp` - Maneja callbacks específicos del editor
- `config/input_config.json` - Configuración de controles

## Archivos

- `config/input_config.json` - Archivo de configuración principal
- `src/core/InputConfigLoader.h` - Header de la clase cargadora
- `src/core/InputConfigLoader.cpp` - Implementación de la clase cargadora

## Estructura del JSON

```json
{
  "input_actions": [
    {
      "name": "NombreAccion",
      "type": "TipoInput",
      "key_bindings": [...],
      "mouse_axis": "...",
      "mouse_button": "..."
    }
  ]
}
```

## Tipos de Input Soportados

### Tipos de Acción

- `"Vector2D"` - Para movimiento 2D (WASD)
- `"Value"` - Para valores únicos (Space/Shift)
- `"MouseAxis"` - Para ejes del mouse
- `"MouseButton"` - Para botones del mouse
- `"Button"` - Para teclas simples

## Configuración de Teclas

### Teclas Soportadas

#### Teclas Alfanuméricas

- `"SDLK_a"` a `"SDLK_z"` - Letras minúsculas
- `"SDLK_0"` a `"SDLK_9"` - Números

#### Teclas de Función

- `"SDLK_F1"` a `"SDLK_F24"` - Teclas de función

#### Teclas Especiales

- `"SDLK_ESCAPE"`, `"SDLK_TAB"`, `"SDLK_CAPSLOCK"`
- `"SDLK_LSHIFT"`, `"SDLK_RSHIFT"`, `"SDLK_LCTRL"`, `"SDLK_RCTRL"`
- `"SDLK_LALT"`, `"SDLK_RALT"`, `"SDLK_LGUI"`, `"SDLK_RGUI"`
- `"SDLK_ENTER"`, `"SDLK_RETURN"`, `"SDLK_BACKSPACE"`
- `"SDLK_DELETE"`, `"SDLK_INSERT"`, `"SDLK_HOME"`, `"SDLK_END"`
- `"SDLK_PAGEUP"`, `"SDLK_PAGEDOWN"`, `"SDLK_PRINTSCREEN"`
- `"SDLK_SCROLLLOCK"`, `"SDLK_PAUSE"`

#### Teclas de Dirección

- `"SDLK_UP"`, `"SDLK_DOWN"`, `"SDLK_LEFT"`, `"SDLK_RIGHT"`

#### Teclas de Puntuación

- `"SDLK_SPACE"`, `"SDLK_PERIOD"`, `"SDLK_COMMA"`, `"SDLK_SEMICOLON"`
- `"SDLK_COLON"`, `"SDLK_QUOTE"`, `"SDLK_QUOTEDBL"`, `"SDLK_MINUS"`
- `"SDLK_PLUS"`, `"SDLK_EQUALS"`, `"SDLK_SLASH"`, `"SDLK_BACKSLASH"`
- `"SDLK_BACKQUOTE"`, `"SDLK_LEFTBRACKET"`, `"SDLK_RIGHTBRACKET"`
- `"SDLK_LEFTPAREN"`, `"SDLK_RIGHTPAREN"`, `"SDLK_EXCLAIM"`, `"SDLK_QUESTION"`
- `"SDLK_AT"`, `"SDLK_HASH"`, `"SDLK_DOLLAR"`, `"SDLK_PERCENT"`
- `"SDLK_CARET"`, `"SDLK_AMPERSAND"`, `"SDLK_ASTERISK"`, `"SDLK_UNDERSCORE"`

#### Teclado Numérico

- `"SDLK_KP_0"` a `"SDLK_KP_9"` - Números del teclado numérico
- `"SDLK_KP_PLUS"`, `"SDLK_KP_MINUS"`, `"SDLK_KP_MULTIPLY"`, `"SDLK_KP_DIVIDE"`
- `"SDLK_KP_ENTER"`, `"SDLK_KP_PERIOD"`, `"SDLK_KP_EQUALS"`

#### Teclas Multimedia

- `"SDLK_AUDIOPLAY"`, `"SDLK_AUDIOSTOP"`, `"SDLK_AUDIOPREV"`, `"SDLK_AUDIONEXT"`
- `"SDLK_AUDIOMUTE"`, `"SDLK_VOLUMEUP"`, `"SDLK_VOLUMEDOWN"`, `"SDLK_MUTE"`
- `"SDLK_MEDIASELECT"`

#### Teclas del Sistema

- `"SDLK_MODE"`, `"SDLK_BRIGHTNESSDOWN"`, `"SDLK_BRIGHTNESSUP"`
- `"SDLK_DISPLAYSWITCH"`, `"SDLK_KBDILLUMTOGGLE"`, `"SDLK_KBDILLUMDOWN"`
- `"SDLK_KBDILLUMUP"`, `"SDLK_EJECT"`, `"SDLK_SLEEP"`

#### Teclas de Navegación Web (AC = Application Control)

- `"SDLK_AC_BACK"`, `"SDLK_AC_FORWARD"`, `"SDLK_AC_REFRESH"`
- `"SDLK_AC_STOP"`, `"SDLK_AC_SEARCH"`, `"SDLK_AC_BOOKMARKS"`
- `"SDLK_AC_HOME"`

#### Teclas de Aplicación

- `"SDLK_APPLICATION"`, `"SDLK_POWER"`, `"SDLK_EXECUTE"`, `"SDLK_HELP"`
- `"SDLK_MENU"`, `"SDLK_SELECT"`, `"SDLK_STOP"`, `"SDLK_AGAIN"`
- `"SDLK_UNDO"`, `"SDLK_CUT"`, `"SDLK_COPY"`, `"SDLK_PASTE"`, `"SDLK_FIND"`
- `"SDLK_WWW"`, `"SDLK_MAIL"`, `"SDLK_CALCULATOR"`, `"SDLK_COMPUTER"`

**Nota**: El sistema soporta prácticamente todas las teclas SDL disponibles. Si necesitas una tecla específica que no esté listada, puedes agregarla fácilmente al método `stringToKeycode()`.

### Configuración de Key Bindings

```json
{
  "key": "SDLK_w",
  "positive": true,
  "axis": 1
}
```

- `key` - Código de la tecla SDL
- `positive` - Si es positivo (true) o negativo (false)
- `axis` - Eje para acciones Vector2D (0=X, 1=Y)

## Configuración de Mouse

### Ejes de Mouse

- `"X"` - Movimiento horizontal
- `"Y"` - Movimiento vertical
- `"ScrollWheel"` - Rueda del mouse

### Botones de Mouse

- `"SDL_BUTTON_LEFT"`
- `"SDL_BUTTON_RIGHT"`
- `"SDL_BUTTON_MIDDLE"`

## Ejemplo Completo

```json
{
  "input_actions": [
    {
      "name": "Move",
      "type": "Vector2D",
      "key_bindings": [
        { "key": "SDLK_w", "positive": true, "axis": 1 },
        { "key": "SDLK_s", "positive": false, "axis": 1 },
        { "key": "SDLK_d", "positive": true, "axis": 0 },
        { "key": "SDLK_a", "positive": false, "axis": 0 }
      ]
    },
    {
      "name": "MouseLookX",
      "type": "MouseAxis",
      "mouse_axis": "X"
    }
  ]
}
```

## Uso

### En el Core

```cpp
// Cargar configuración desde JSON
InputConfigLoader::loadInputConfigFromJSON();

// Obtener acción configurada
auto& inputSystem = InputSystem::getInstance();
auto moveAction = inputSystem.getAction("Move");
```

### En el Editor

```cpp
void setupInputSystem(Scene* activeScene) {
    // Cargar configuración
    InputConfigLoader::loadInputConfigFromJSON();

    auto& inputSystem = InputSystem::getInstance();

    // Agregar callbacks específicos del editor
    auto moveAction = inputSystem.getAction("Move");
    if (moveAction) {
        moveAction->bindVector2DCallback([activeScene](const glm::vec2& movement) {
            // Lógica específica del editor
        });
    }
}
```

## Ventajas

- [ ] Separación clara entre core y editor
- [ ] Configuración sin recompilar
- [ ] Fácil personalización
- [ ] Reutilizable en diferentes proyectos
- [ ] Callbacks específicos por aplicación
- [ ] Manejo de errores robusto

## Flujo de Trabajo

1. **Configuración**: El JSON define qué teclas/controles se usan
2. **Carga**: `InputConfigLoader` registra las acciones en `InputSystem`
3. **Uso**: El editor obtiene las acciones y agrega sus callbacks específicos
4. **Ejecución**: Los controles funcionan con la lógica del editor

## Extensibilidad

### Agregar Nuevas Teclas

1. Agrega el mapeo en `InputConfigLoader::stringToKeycode()`
2. Actualiza la documentación

### Agregar Nuevos Tipos de Input

1. Agrega el tipo en `InputConfigLoader::stringToInputType()`
2. Implementa el procesamiento en `InputSystem`
3. Actualiza la documentación

### Agregar Callbacks en el Editor

1. Obtén la acción con `inputSystem.getAction("Nombre")`
2. Agrega el callback específico del editor
3. Maneja la lógica de negocio

## Fallback

Si el archivo JSON no se puede cargar o tiene errores:

- El sistema mostrará un mensaje de error
- Continuará funcionando con la configuración por defecto
- Los callbacks del editor seguirán funcionando si las acciones existen

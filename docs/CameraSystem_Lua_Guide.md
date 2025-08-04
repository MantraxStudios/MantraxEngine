# Sistema de Cámara en Lua - Guía de Uso

## Descripción General

El sistema de cámara en Lua permite acceder y modificar la cámara del nivel activo desde scripts Lua. Esto incluye control de posición, rotación, proyección y otros parámetros de la cámara.

## Funciones Globales

### `getActiveCamera()`

Obtiene la cámara de la escena activa.

**Retorna:** `Camera*` - Puntero a la cámara activa, o `nil` si no hay escena activa.

**Ejemplo:**

```lua
local camera = getActiveCamera()
if camera then
    local pos = camera:getPosition()
    log("Posición de la cámara: " .. pos.x .. ", " .. pos.y .. ", " .. pos.z)
end
```

### `getCameraFromScene(sceneName)`

Obtiene la cámara de una escena específica.

**Parámetros:**

- `sceneName` (string): Nombre de la escena

**Retorna:** `Camera*` - Puntero a la cámara de la escena, o `nil` si la escena no existe.

**Ejemplo:**

```lua
local camera = getCameraFromScene("MiEscena")
if camera then
    camera:setPosition(vector3(0, 5, 10))
end
```

## Métodos de la Cámara

### Posición y Orientación

#### `setPosition(pos)`

Establece la posición de la cámara.

**Parámetros:**

- `pos` (vector3): Nueva posición de la cámara

**Ejemplo:**

```lua
camera:setPosition(vector3(0, 5, 10))
```

#### `getPosition()`

Obtiene la posición actual de la cámara.

**Retorna:** `vector3` - Posición actual de la cámara

#### `setTarget(target)`

Establece el punto al que mira la cámara.

**Parámetros:**

- `target` (vector3): Punto objetivo

#### `setRotation(yaw, pitch)`

Establece la rotación de la cámara en grados.

**Parámetros:**

- `yaw` (float): Rotación horizontal en grados
- `pitch` (float): Rotación vertical en grados

#### `rotate(yawDelta, pitchDelta)`

Rota la cámara relativo a su rotación actual.

**Parámetros:**

- `yawDelta` (float): Cambio en rotación horizontal
- `pitchDelta` (float): Cambio en rotación vertical

### Movimiento

#### `moveForward(distance)`

Mueve la cámara hacia adelante.

**Parámetros:**

- `distance` (float): Distancia a mover

#### `moveRight(distance)`

Mueve la cámara hacia la derecha.

**Parámetros:**

- `distance` (float): Distancia a mover

#### `moveUp(distance)`

Mueve la cámara hacia arriba.

**Parámetros:**

- `distance` (float): Distancia a mover

### Vectores de Orientación

#### `getForward()`

Obtiene el vector forward de la cámara.

**Retorna:** `vector3` - Vector forward normalizado

#### `getRight()`

Obtiene el vector right de la cámara.

**Retorna:** `vector3` - Vector right normalizado

#### `getUp()`

Obtiene el vector up de la cámara.

**Retorna:** `vector3` - Vector up normalizado

### Configuración de Proyección

#### `setProjectionType(type, instant)`

Cambia el tipo de proyección de la cámara.

**Parámetros:**

- `type` (ProjectionType): Tipo de proyección (Perspective/Orthographic)
- `instant` (bool): Si es true, el cambio es instantáneo

**Ejemplo:**

```lua
camera:setProjectionType(ProjectionType.Perspective, true)
camera:setProjectionType(ProjectionType.Orthographic, false)
```

#### `getProjectionType()`

Obtiene el tipo de proyección actual.

**Retorna:** `ProjectionType` - Tipo de proyección actual

#### `isTransitioning()`

Verifica si la cámara está en transición entre tipos de proyección.

**Retorna:** `bool` - True si está en transición

### Parámetros de Perspectiva

#### `getFOV()`

Obtiene el campo de visión (FOV) de la cámara.

**Retorna:** `float` - FOV en grados

#### `setAspectRatio(aspect)`

Establece la relación de aspecto de la cámara.

**Parámetros:**

- `aspect` (float): Nueva relación de aspecto

#### `getAspectRatio()`

Obtiene la relación de aspecto actual.

**Retorna:** `float` - Relación de aspecto actual

#### `getNearClip()`

Obtiene el plano near de recorte.

**Retorna:** `float` - Distancia del plano near

#### `getFarClip()`

Obtiene el plano far de recorte.

**Retorna:** `float` - Distancia del plano far

### Parámetros Ortográficos

#### `setOrthographicSize(size)`

Establece el tamaño ortográfico de la cámara.

**Parámetros:**

- `size` (float): Nuevo tamaño ortográfico

#### `getOrthographicSize()`

Obtiene el tamaño ortográfico actual.

**Retorna:** `float` - Tamaño ortográfico actual

### Framebuffer

#### `enableFramebuffer(enabled)`

Habilita o deshabilita el framebuffer de la cámara.

**Parámetros:**

- `enabled` (bool): True para habilitar, false para deshabilitar

#### `isFramebufferEnabled()`

Verifica si el framebuffer está habilitado.

**Retorna:** `bool` - True si está habilitado

#### `setFramebufferSize(width, height)`

Establece el tamaño del framebuffer.

**Parámetros:**

- `width` (int): Ancho del framebuffer
- `height` (int): Alto del framebuffer

### Velocidad (para Audio)

#### `getVelocity()`

Obtiene la velocidad de la cámara (útil para efectos de audio 3D).

**Retorna:** `vector3` - Velocidad de la cámara

### Actualización

#### `update(deltaTime)`

Actualiza la cámara (calcula velocidad, transiciones, etc.).

**Parámetros:**

- `deltaTime` (float): Tiempo transcurrido desde el último frame

## Enumeraciones

### ProjectionType

```lua
ProjectionType.Perspective   -- Proyección perspectiva
ProjectionType.Orthographic  -- Proyección ortográfica
```

## Ejemplos de Uso

### Control Básico de Cámara

```lua
function update(deltaTime)
    local camera = getActiveCamera()
    if not camera then return end

    local speed = 5.0 * deltaTime

    -- Movimiento con WASD
    if getAction("MoveForward"):getValue() > 0 then
        camera:moveForward(speed)
    end

    if getAction("MoveRight"):getValue() > 0 then
        camera:moveRight(speed)
    end
end
```

### Cámara de Seguimiento

```lua
function followTarget(targetObject, offset)
    local camera = getActiveCamera()
    if not camera then return end

    local targetPos = targetObject:getPosition()
    local cameraPos = vector3(
        targetPos.x + offset.x,
        targetPos.y + offset.y,
        targetPos.z + offset.z
    )

    camera:setPosition(cameraPos)
    camera:setTarget(targetPos)
end
```

### Cambio de Proyección

```lua
function toggleProjection()
    local camera = getActiveCamera()
    if not camera then return end

    local currentType = camera:getProjectionType()

    if currentType == ProjectionType.Perspective then
        camera:setProjectionType(ProjectionType.Orthographic, false)
    else
        camera:setProjectionType(ProjectionType.Perspective, false)
    end
end
```

### Efectos de Cámara

```lua
function onTriggerEnter(other)
    local camera = getActiveCamera()
    if camera then
        -- Cambiar a ultra-wide cuando entra en trigger
        camera:setAspectRatio(21.0 / 9.0)
    end
end

function onTriggerExit(other)
    local camera = getActiveCamera()
    if camera then
        -- Restaurar aspect ratio normal
        camera:setAspectRatio(16.0 / 9.0)
    end
end
```

## Notas Importantes

1. **Verificación de Nulos:** Siempre verifica que la cámara no sea `nil` antes de usarla.
2. **Transiciones:** Las transiciones entre tipos de proyección son suaves por defecto.
3. **Coordenadas:** Las coordenadas siguen el sistema de coordenadas del motor.
4. **Rendimiento:** Las operaciones de cámara son eficientes, pero evita llamarlas en exceso en el update.
5. **Audio 3D:** La velocidad de la cámara se calcula automáticamente para efectos de audio 3D.

## Errores Comunes

1. **Cámara nula:** Siempre verifica que `getActiveCamera()` no retorne `nil`.
2. **Escena no activa:** Asegúrate de que haya una escena activa antes de usar la cámara.
3. **Parámetros inválidos:** Los ángulos de pitch están limitados entre -89 y 89 grados.
4. **Transiciones:** Las transiciones de proyección pueden tomar tiempo, verifica con `isTransitioning()`.

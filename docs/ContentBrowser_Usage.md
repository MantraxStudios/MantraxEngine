# ContentBrowser - Guía de Uso

## Crear Scripts Lua

El ContentBrowser de MantraxEngine permite crear scripts Lua de forma fácil y rápida.

### Pasos para crear un script Lua:

1. **Abrir el ContentBrowser**
   - El ContentBrowser se encuentra en la ventana "Content Browser" del editor

2. **Navegar a la carpeta deseada**
   - Usa el TreeView para navegar a la carpeta donde quieres crear el script
   - Puedes usar el botón "Up" para subir de nivel

3. **Abrir el menú contextual**
   - Haz clic derecho en el área vacía del TreeView (no en un archivo)
   - Se abrirá un menú contextual

4. **Seleccionar "Create" > "Lua Script"**
   - En el menú contextual, selecciona "Create"
   - Luego selecciona "Lua Script"

5. **Configurar el nombre del script**
   - Se abrirá un popup para ingresar el nombre del script
   - El nombre se completará automáticamente con la extensión `.lua`
   - Presiona Enter o haz clic en "Create" para crear el script

### Estructura del script creado

El script creado tendrá la siguiente estructura básica:

```lua
-- NombreDelScript.lua
-- Created by MantraxEngine ContentBrowser

function OnStart()
    -- Initialize your script here
    print("Script started: NombreDelScript.lua")
end

function OnTick()
    -- Update your script here
end

function OnDestroy()
    -- Cleanup your script here
end
```

### Funciones disponibles

- **OnStart()**: Se llama cuando el script se inicializa
- **OnTick()**: Se llama cada frame (para actualizaciones)
- **OnDestroy()**: Se llama cuando el script se destruye

### Solución de problemas

Si el popup no se abre:

1. **Verificar que estás en una carpeta**
   - Asegúrate de hacer clic derecho en el área vacía, no en un archivo

2. **Usar el botón de prueba**
   - En la barra de menú del ContentBrowser, usa el botón "Test Script Popup"

3. **Verificar la consola**
   - Revisa la consola del editor para mensajes de error

### Características adicionales

- **Editar scripts**: Haz clic derecho en un archivo `.lua` y selecciona "Edit Script"
- **Abrir en editor externo**: Selecciona "Open in Text Editor" para abrir en Notepad
- **Renombrar**: Usa "Rename" en el menú contextual
- **Eliminar**: Usa "Delete" en el menú contextual

### Notas importantes

- Los scripts se crean en la carpeta actual del ContentBrowser
- El ContentBrowser solo funciona dentro del directorio `Content` del proyecto
- Los scripts creados son compatibles con el sistema de scripting de MantraxEngine 
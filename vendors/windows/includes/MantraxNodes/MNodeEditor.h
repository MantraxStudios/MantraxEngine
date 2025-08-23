#pragma once
#include <imgui/imgui.h>
#include <vector>
#include <string>
#include <functional>
#include <iostream>
#include <algorithm>
#include <map>
#include <any>
#include <chrono>
#include <cmath>
#include <array>
#include <set>
#include "MNodeEngine.h"
#include "Externals/GameObjectNode.h"
#include "Externals/DebugNodes.h"
#include "Externals/EventsNode.h"
#include "Externals/MathNodes.h"
#include "Externals/ConstNode.h"

class MNodeEditor
{
public:
    MNodeEngine *engine = new MNodeEngine();
    std::vector<CustomNode> &customNodes = engine->customNodes;
    std::vector<Connection> &connections = engine->connections;

    // NODES IMPLEMENTS
    GameObjectNode *NodesGM = new GameObjectNode();
    DebugNodes *NodesDB = new DebugNodes();
    EventsNode *NodesEV = new EventsNode();
    MathNodes *NodesMath = new MathNodes();
    ConstNode *NodesConst = new ConstNode();

    int connectingFromNode = -1;
    int connectingFromPin = -1;
    int editingNodeId = -1;    // ID del nodo que se está editando
    char textBuffer[256] = ""; // Buffer para el texto de entrada
    bool isConnecting = false; // Flag para indicar si estamos en modo conexión

    // Variables para el menú contextual
    bool showContextMenu = false;
    ImVec2 contextMenuPos = ImVec2(0, 0);
    int contextMenuNodeId = -1;

    // Variables para el menú contextual de creación de nodos (estilo Unreal)
    bool showCreateNodeMenu = false;
    ImVec2 createNodeMenuPos = ImVec2(0, 0);

    // Variables para el movimiento del panel (estilo Unreal)
    ImVec2 panelOffset = ImVec2(0, 0);  // Offset del panel
    ImVec2 lastMousePos = ImVec2(0, 0); // Última posición del ratón para pan
    bool isPanning = false;             // Flag para indicar si estamos haciendo pan

    MNodeEditor()
    {
        NodesGM->RegisterNodes(*engine);
        NodesDB->RegisterNodes(*engine);
        NodesEV->RegisterNodes(*engine);
        NodesMath->RegisterNodes(*engine);
        NodesConst->RegisterNodes(*engine);
    }

    // Colores estilo Blender
    struct BlenderColors
    {
        static ImU32 GetCategoryColor(NodeCategory category)
        {
            // Todos los headers son grises (100, 100, 100, 255)
            return IM_COL32(100, 100, 100, 255);
        }

        static ImU32 GetCategoryColorDark(NodeCategory category)
        {
            // Versión más oscura del gris (70, 70, 70, 255)
            return IM_COL32(70, 70, 70, 255);
        }

        static constexpr ImU32 NodeBackground = IM_COL32(62, 62, 66, 240);
        static constexpr ImU32 NodeBackgroundSelected = IM_COL32(90, 90, 94, 255);
        static constexpr ImU32 NodeBorder = IM_COL32(100, 100, 100, 255);
        static constexpr ImU32 NodeBorderSelected = IM_COL32(255, 190, 60, 255);
        static constexpr ImU32 NodeHeaderText = IM_COL32(220, 220, 220, 255);
        static constexpr ImU32 PinExec = IM_COL32(255, 255, 255, 255);
        static constexpr ImU32 PinFloat = IM_COL32(160, 160, 164, 255);
        static constexpr ImU32 PinVector = IM_COL32(100, 200, 200, 255);
        static constexpr ImU32 PinColor = IM_COL32(255, 255, 100, 255);
        static constexpr ImU32 PinString = IM_COL32(100, 200, 100, 255);
        static constexpr ImU32 ConnectionLine = IM_COL32(180, 180, 180, 200);
        static constexpr ImU32 ConnectionLineSelected = IM_COL32(255, 255, 255, 255);
    };

    CustomNode *GetNodeById(int id)
    {
        for (auto &n : customNodes)
        {
            if (n.n.id == id || n.nodeId == id)
                return &n;
        }
        return nullptr;
    }

    CustomNode *GetCustomNodeById(int id)
    {
        for (auto &cn : customNodes)
        {
            if (cn.nodeId == id || cn.n.id == id)
                return &cn;
        }
        return nullptr;
    }

    // Función para obtener el color de un pin según su tipo
    ImU32 GetPinColor(bool isExec, const std::any *value = nullptr)
    {
        if (isExec)
            return BlenderColors::PinExec;

        if (!value)
            return BlenderColors::PinFloat;

        // Determinar color por tipo de datos
        if (value->type() == typeid(std::string))
            return BlenderColors::PinString;
        else if (value->type() == typeid(float) || value->type() == typeid(int))
            return BlenderColors::PinFloat;
        else if (value->type() == typeid(bool))
            return BlenderColors::PinColor;
        else if (value->type() == typeid(Vector2))
            return BlenderColors::PinVector;
        else if (value->type() == typeid(Vector3))
            return BlenderColors::PinVector;
        else if (value->type() == typeid(Matrix3x3))
            return BlenderColors::PinColor;
        else if (value->type() == typeid(Matrix4x4))
            return BlenderColors::PinColor;
        else if (value->type() == typeid(GameObject *))
            return BlenderColors::PinFloat;

        return BlenderColors::PinFloat;
    }

    void Draw()
    {
        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        ImVec2 window_pos = ImGui::GetWindowPos();
        ImVec2 mousePos = ImGui::GetIO().MousePos - window_pos;

        // Manejar pan con el botón medio del ratón
        if (ImGui::IsMouseDown(ImGuiMouseButton_Middle))
        {
            if (!isPanning)
            {
                isPanning = true;
                lastMousePos = mousePos;
            }
            else
            {
                ImVec2 delta = mousePos - lastMousePos;
                panelOffset += delta;
                lastMousePos = mousePos;
            }
        }
        else
        {
            isPanning = false;
        }

        // Manejar clic derecho en el espacio vacío para crear nodos
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && !ImGui::IsAnyItemHovered())
        {
            showCreateNodeMenu = true;
            // Capturar la posición absoluta del mouse para crear el nodo en esa ubicación
            // Esto permite que los nodos aparezcan exactamente donde haces clic derecho
            createNodeMenuPos = ImGui::GetIO().MousePos;
        }

        // Aplicar transformación del panel
        ImVec2 transformedMousePos = mousePos - panelOffset;

        // Mostrar estado de conexión en la parte superior
        if (isConnecting && connectingFromNode != -1)
        {
            Node *fromNode = &GetNodeById(connectingFromNode)->n;
            if (fromNode && connectingFromPin < fromNode->outputs.size())
            {
                ImVec2 statusPos = window_pos + ImVec2(10, 30);
                std::string statusText = "Connecting from: " + fromNode->title + " (Pin " + std::to_string(connectingFromPin) + ")";

                // Fondo del texto
                ImVec2 textSize = ImGui::CalcTextSize(statusText.c_str());
                draw_list->AddRectFilled(
                    statusPos - ImVec2(5, 2),
                    statusPos + textSize + ImVec2(5, 2),
                    IM_COL32(0, 0, 0, 180), 4.0f);

                // Texto del estado
                draw_list->AddText(statusPos, IM_COL32(255, 255, 255, 255), statusText.c_str());
            }
        }

        // Actualizar valores de entrada desde conexiones en cada frame
        for (auto &c : connections)
        {
            CustomNode *sourceNode = GetCustomNodeById(c.fromNodeId);
            CustomNode *targetNode = GetCustomNodeById(c.toNodeId);

            if (sourceNode && targetNode)
            {
                // Solo transferir datos para pins que no son de ejecución
                if (c.fromPinId < sourceNode->n.outputs.size() &&
                    c.toPinId < targetNode->n.inputs.size() &&
                    !sourceNode->n.outputs[c.fromPinId].isExec)
                {
                    // Si el nodo fuente es un nodo de datos, ejecutarlo para mantener valores actualizados
                    if (sourceNode->n.inputs.empty() || !sourceNode->n.inputs[0].isExec)
                    {
                        if (sourceNode->n.execFunc)
                        {
                            sourceNode->n.execFunc(&sourceNode->n);
                        }
                    }

                    auto outputValue = sourceNode->outputValues.find(c.fromPinId);
                    if (outputValue != sourceNode->outputValues.end())
                    {
                        targetNode->inputValues[c.toPinId] = outputValue->second;

                        // Debug: mostrar cuando se actualizan valores
                        if (sourceNode->n.title == "String" && c.fromPinId == 0)
                        {
                            try
                            {
                                std::string value = std::any_cast<std::string>(outputValue->second);
                            }
                            catch (...)
                            {
                                std::cout << "[FRAME UPDATE] String node output: <invalid type>" << std::endl;
                            }
                        }

                        // Debug: mostrar cuando se actualizan valores de Integer nodes
                        if (sourceNode->n.title == "Integer" && c.fromPinId == 0)
                        {
                            try
                            {
                                int value = std::any_cast<int>(outputValue->second);
                            }
                            catch (...)
                            {
                                std::cout << "[FRAME UPDATE] Integer node output: <invalid type>" << std::endl;
                            }
                        }
                    }
                }
            }
        }

        // Dibujar conexiones con estilo Blender
        for (auto &c : connections)
        {
            Node *fromNode = &GetNodeById(c.fromNodeId)->n;
            Node *toNode = &GetNodeById(c.toNodeId)->n;
            if (!fromNode || !toNode)
                continue;

            if (c.fromPinId >= fromNode->outputs.size() || c.toPinId >= toNode->inputs.size())
                continue;

            // Aplicar transformación del panel a las posiciones
            ImVec2 p1 = window_pos + panelOffset + fromNode->pos + fromNode->outputs[c.fromPinId].pos;
            ImVec2 p2 = window_pos + panelOffset + toNode->pos + toNode->inputs[c.toPinId].pos;

            // Color de la línea según el tipo de pin
            bool isExec = fromNode->outputs[c.fromPinId].isExec;
            ImU32 lineColor = isExec ? BlenderColors::PinExec : BlenderColors::ConnectionLine;

            // Línea más gruesa para conexiones de ejecución
            float thickness = isExec ? 4.0f : 4.0f;

            // Dibujar sombra de la línea
            draw_list->AddBezierCubic(
                p1, p1 + ImVec2(50, 0),
                p2 - ImVec2(50, 0), p2,
                IM_COL32(0, 0, 0, 80), thickness + 1.0f);

            // Dibujar línea principal
            draw_list->AddBezierCubic(
                p1, p1 + ImVec2(50, 0),
                p2 - ImVec2(50, 0), p2,
                lineColor, thickness);
        }

        // Dibujar nodos con estilo Blender
        for (auto &d : customNodes)
        {
            auto &n = d.n;

            ImGui::PushID(n.id);
            // Aplicar transformación del panel a las posiciones del nodo
            ImVec2 min = window_pos + panelOffset + n.pos;
            ImVec2 max = window_pos + panelOffset + n.pos + n.size;

            // Colores según categoría y estado
            ImU32 headerColor = BlenderColors::GetCategoryColor(n.category);
            ImU32 nodeColor = n.isSelected ? BlenderColors::NodeBackgroundSelected : BlenderColors::NodeBackground;
            ImU32 borderColor = n.isSelected ? BlenderColors::NodeBorderSelected : BlenderColors::NodeBorder;

            // Efecto de activación
            if (n.isActive)
            {
                headerColor = IM_COL32(255, 255, 255, 255);
                borderColor = IM_COL32(255, 255, 255, 255);
            }

            // Sombra del nodo
            draw_list->AddRectFilled(
                min + ImVec2(2, 2), max + ImVec2(2, 2),
                IM_COL32(0, 0, 0, 60), 6.0f);

            // Fondo del nodo con esquinas redondeadas estilo Blender
            draw_list->AddRectFilled(min, max, nodeColor, 10.0f);

            // Header del nodo uniforme
            ImVec2 headerMax = ImVec2(max.x, min.y + 26);

            // Dibujamos el header completo con esquinas superiores redondeadas
            draw_list->AddRectFilled(min, headerMax, headerColor, 10.0f, ImDrawFlags_RoundCornersTop);

            // Borde del nodo
            draw_list->AddRect(min, max, borderColor, 10.0f, 0, n.isSelected ? 2.0f : 4.0f);

            // Título del nodo con mejor tipografía
            ImVec2 text_size = ImGui::CalcTextSize(n.title.c_str());
            draw_list->AddText(
                min + ImVec2((n.size.x - text_size.x) * 0.5f, 5),
                BlenderColors::NodeHeaderText,
                n.title.c_str());

            // Dibujar contenido del nodo
            if (CustomNode *cn = GetCustomNodeById(n.id))
            {
                // Dibujar campos editables para pins de entrada
                float currentY = 34.0f; // Ajustado para alinear exactamente con los pins (44px - 10px para centrar texto)
                for (size_t pinIndex = 0; pinIndex < n.inputs.size(); pinIndex++)
                {
                    const Pin &pin = n.inputs[pinIndex];

                    // Mostrar etiqueta del pin si no es de ejecución
                    if (!pin.isExec)
                    {
                        auto nameIt = cn->inputNames.find((int)pinIndex);
                        std::string pinName = (nameIt != cn->inputNames.end()) ? nameIt->second : ("Pin " + std::to_string(pinIndex));

                        // Dibujar etiqueta del pin con mejor posicionamiento (centrada verticalmente con el pin)
                        ImVec2 labelPos = ImVec2(min.x + 15, min.y + currentY);
                        draw_list->AddText(labelPos, IM_COL32(200, 200, 200, 255), pinName.c_str());

                        // Campo de entrada solo si no hay conexión
                        bool hasConnection = engine->HasConnection(n.id, (int)pinIndex, true);
                        if (!hasConnection)
                        {
                            // Calcular el ancho disponible para el campo de entrada
                            // Dejar espacio para la etiqueta del pin (aproximadamente 80px) + márgenes
                            float availableWidth = n.size.x - 88.0f; // 80px para etiqueta + 8px de margen derecho

                            ImVec2 inputPos = ImVec2(min.x + 70, min.y + currentY - 2);
                            ImGui::SetCursorScreenPos(inputPos);
                            ImGui::PushItemWidth(availableWidth);

                            std::string inputId = "##" + std::to_string(n.id) + "_input_" + std::to_string(pinIndex);

                            // Manejo de diferentes tipos de datos
                            auto inputIt = cn->inputValues.find((int)pinIndex);
                            if (inputIt != cn->inputValues.end())
                            {
                                try
                                {
                                    if (inputIt->second.type() == typeid(std::string))
                                    {
                                        std::string currentValue = std::any_cast<std::string>(inputIt->second);
                                        char buf[256];
                                        strncpy(buf, currentValue.c_str(), sizeof(buf) - 1);
                                        buf[sizeof(buf) - 1] = '\0';

                                        if (ImGui::InputText(inputId.c_str(), buf, sizeof(buf)))
                                        {
                                            std::string newValue = std::string(buf);
                                            cn->SetInputValue<std::string>((int)pinIndex, newValue);

                                            // Para nodos String, actualizar inmediatamente el valor de salida
                                            if (n.title == "String" && pinIndex == 0)
                                            {
                                                cn->SetOutputValue<std::string>(0, newValue);
                                                std::cout << "[STRING] Updated output value to: " << newValue << std::endl;
                                            }
                                        }
                                    }
                                    else if (inputIt->second.type() == typeid(int))
                                    {
                                        int currentValue = std::any_cast<int>(inputIt->second);
                                        if (ImGui::DragInt(inputId.c_str(), &currentValue, 0.1f))
                                        {
                                            cn->SetInputValue<int>((int)pinIndex, currentValue);
                                        }
                                    }
                                    else if (inputIt->second.type() == typeid(float))
                                    {
                                        float currentValue = std::any_cast<float>(inputIt->second);
                                        if (ImGui::DragFloat(inputId.c_str(), &currentValue, 0.01f))
                                        {
                                            cn->SetInputValue<float>((int)pinIndex, currentValue);
                                        }
                                    }
                                    else if (inputIt->second.type() == typeid(bool))
                                    {
                                        bool currentValue = std::any_cast<bool>(inputIt->second);
                                        if (ImGui::Checkbox(inputId.c_str(), &currentValue))
                                        {
                                            cn->SetInputValue<bool>((int)pinIndex, currentValue);
                                        }
                                    }
                                }
                                catch (const std::bad_any_cast &e)
                                {
                                    ImGui::Text("Error");
                                }
                            }
                            ImGui::PopItemWidth();
                        }
                    }

                    currentY += 28.0f;
                }

                // Dibujar etiquetas para pins de salida
                float outputY = 34.0f; // Ajustado para alinear exactamente con los pins
                for (size_t pinIndex = 0; pinIndex < n.outputs.size(); pinIndex++)
                {
                    const Pin &pin = n.outputs[pinIndex];

                    if (!pin.isExec)
                    {
                        auto nameIt = cn->outputNames.find((int)pinIndex);
                        std::string pinName = (nameIt != cn->outputNames.end()) ? nameIt->second : ("Out " + std::to_string(pinIndex));

                        // Etiqueta alineada a la derecha y centrada verticalmente con el pin
                        ImVec2 text_size = ImGui::CalcTextSize(pinName.c_str());
                        ImVec2 labelPos = ImVec2(max.x - text_size.x - 8, min.y + outputY);
                        // draw_list->AddText(labelPos, IM_COL32(200, 200, 200, 255), pinName.c_str());
                    }

                    outputY += 28.0f;
                }
            }

            // Botón invisible para arrastrar el nodo (solo en el header)
            ImGui::SetCursorScreenPos(min);
            ImGui::InvisibleButton(("node" + std::to_string(n.id)).c_str(), ImVec2(n.size.x, 26));

            // Manejar selección
            if (ImGui::IsItemClicked())
            {
                // Deseleccionar otros nodos
                for (auto &other : customNodes)
                    other.n.isSelected = false;
                n.isSelected = true;
            }

            // Manejar clic derecho para menú contextual
            if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
            {
                showContextMenu = true;
                contextMenuPos = ImGui::GetIO().MousePos;
                contextMenuNodeId = n.id;

                // Deseleccionar otros nodos y seleccionar este
                for (auto &other : customNodes)
                    other.n.isSelected = false;
                n.isSelected = true;
            }

            // Solo permitir arrastrar si no estamos en modo conexión
            if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left) && !isConnecting)
            {
                // Mover el nodo
                n.pos += ImGui::GetIO().MouseDelta;
            }

            // PINS DE ENTRADA con estilo Blender
            for (size_t i = 0; i < n.inputs.size(); i++)
            {
                Pin &pin = n.inputs[i];
                // Aplicar transformación del panel a la posición del pin
                ImVec2 pinPos = window_pos + panelOffset + n.pos + pin.pos;

                bool hasConnection = engine->HasConnection(n.id, (int)i, true);

                // Obtener color del pin
                CustomNode *cn = GetCustomNodeById(n.id);
                const std::any *value = nullptr;
                if (cn)
                {
                    // Primero buscar en valores de entrada (conexiones activas)
                    auto it = cn->inputValues.find((int)i);
                    if (it != cn->inputValues.end())
                    {
                        value = &it->second;
                    }
                    else
                    {
                        // Si no hay valor de entrada, usar el valor por defecto
                        auto defaultIt = cn->defaultValues.find((int)i);
                        if (defaultIt != cn->defaultValues.end())
                        {
                            value = &defaultIt->second;
                        }
                    }
                }

                ImU32 pinColor = GetPinColor(pin.isExec, value);

                // Dibujar pin
                if (pin.isExec)
                {
                    // Pin de ejecución: cuadrado
                    float pinSize = 7.0f;
                    ImVec2 pinMin = pinPos - ImVec2(pinSize, pinSize);
                    ImVec2 pinMax = pinPos + ImVec2(pinSize, pinSize);

                    if (hasConnection)
                    {
                        // Pin conectado: cuadrado lleno
                        draw_list->AddRectFilled(pinMin, pinMax, pinColor);
                        draw_list->AddRect(pinMin, pinMax, IM_COL32(0, 0, 0, 120), 0, 1.0f);
                    }
                    else
                    {
                        // Pin desconectado: cuadrado con borde
                        draw_list->AddRectFilled(pinMin, pinMax, pinColor);
                        draw_list->AddRect(pinMin, pinMax, IM_COL32(0, 0, 0, 120), 0, 1.0f);
                    }
                }
                else
                {
                    // Pin de datos: círculo
                    if (hasConnection)
                    {
                        // Pin conectado: círculo completo
                        draw_list->AddCircleFilled(pinPos, 7, pinColor);
                    }
                    else
                    {
                        // Pin desconectado: anillo
                        draw_list->AddCircleFilled(pinPos, 7, pinColor);
                        draw_list->AddCircle(pinPos, 8, IM_COL32(255, 255, 255, 255), 0, 2.0f);
                    }
                }

                // Área de interacción (más grande para mejor usabilidad)
                ImGui::SetCursorScreenPos(pinPos - ImVec2(12, 12));
                ImGui::InvisibleButton(("inpin" + std::to_string(n.id) + "_" + std::to_string(pin.id)).c_str(), ImVec2(24, 24));

                // Highlight en hover con feedback especial para conexiones
                if (ImGui::IsItemHovered())
                {
                    if (isConnecting && connectingFromNode != -1)
                    {
                        // Highlight especial cuando estamos conectando
                        // Solo mostrar highlight si no estamos en el mismo nodo
                        if (connectingFromNode != n.id)
                        {
                            Node *fromNode = &GetNodeById(connectingFromNode)->n;
                            if (fromNode && connectingFromPin < fromNode->outputs.size())
                            {
                                bool isValidTarget = engine->IsValidConnection(fromNode, connectingFromPin, &n, (int)i);
                                ImU32 highlightColor = isValidTarget ? IM_COL32(100, 255, 100, 150) : IM_COL32(255, 100, 100, 150);
                                draw_list->AddCircle(pinPos, 8, highlightColor, 0, 2.0f);
                            }
                        }
                        else
                        {
                            // Highlight especial para el mismo nodo (sin validación)
                            draw_list->AddCircle(pinPos, 8, IM_COL32(255, 255, 0, 150), 0, 2.0f);
                        }
                    }
                    else
                    {
                        // Highlight normal
                        draw_list->AddCircle(pinPos, 7, IM_COL32(255, 255, 255, 100), 0, 1.0f);
                    }
                }

                // Doble click para eliminar conexiones
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    engine->RemoveConnectionsFromPin(n.id, (int)i, true);
                }
                // Conectar al hacer click (no al soltar)
                else if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && isConnecting && connectingFromNode != -1)
                {
                    // Usar el nuevo sistema de conexión mejorado
                    if (engine->CreateConnection(connectingFromNode, connectingFromPin, n.id, (int)i))
                    {
                        std::cout << "[SUCCESS] Connection established from node " << connectingFromNode
                                  << " pin " << connectingFromPin << " to node " << n.id << " pin " << i << std::endl;
                    }
                    else
                    {
                        std::cout << "[FAILED] Could not establish connection from node " << connectingFromNode
                                  << " pin " << connectingFromPin << " to node " << n.id << " pin " << i << std::endl;
                    }

                    connectingFromNode = -1;
                    connectingFromPin = -1;
                    isConnecting = false;
                }
            }

            // PINS DE SALIDA con estilo Blender
            for (size_t i = 0; i < n.outputs.size(); i++)
            {
                Pin &pin = n.outputs[i];
                // Aplicar transformación del panel a la posición del pin
                ImVec2 pinPos = window_pos + panelOffset + n.pos + pin.pos;

                bool hasConnection = engine->HasConnection(n.id, (int)i, false);

                // Obtener color del pin
                CustomNode *cn = GetCustomNodeById(n.id);
                const std::any *value = nullptr;
                if (cn)
                {
                    // Buscar en valores de salida
                    auto it = cn->outputValues.find((int)i);
                    if (it != cn->outputValues.end())
                    {
                        value = &it->second;
                    }
                    else
                    {
                        // Si no hay valor de salida, intentar obtener el tipo del pin de salida
                        // Buscar en valores por defecto del nodo
                        auto defaultIt = cn->defaultValues.find((int)i);
                        if (defaultIt != cn->defaultValues.end())
                        {
                            value = &defaultIt->second;
                        }
                        else
                        {
                            // Para nodos específicos, usar lógica especial
                            if (n.title == "String" && i == 0)
                            {
                                // Para nodos String, usar el valor de entrada
                                auto inputIt = cn->inputValues.find(0);
                                if (inputIt != cn->inputValues.end())
                                {
                                    value = &inputIt->second;
                                }
                            }
                            else if (n.title == "Integer" && i == 0)
                            {
                                // Para nodos Integer, usar el valor de entrada
                                auto inputIt = cn->inputValues.find(0);
                                if (inputIt != cn->inputValues.end())
                                {
                                    value = &inputIt->second;
                                }
                            }
                            else if (n.title == "Float" && i == 0)
                            {
                                // Para nodos Float, usar el valor de entrada
                                auto inputIt = cn->inputValues.find(0);
                                if (inputIt != cn->inputValues.end())
                                {
                                    value = &inputIt->second;
                                }
                            }
                            else if (n.title == "Bool" && i == 0)
                            {
                                // Para nodos Bool, usar el valor de entrada
                                auto inputIt = cn->inputValues.find(0);
                                if (inputIt != cn->inputValues.end())
                                {
                                    value = &inputIt->second;
                                }
                            }
                            else if (n.title == "Vector2" && i == 0)
                            {
                                // Para nodos Vector2, usar el valor de entrada
                                auto inputIt = cn->inputValues.find(0);
                                if (inputIt != cn->inputValues.end())
                                {
                                    value = &inputIt->second;
                                }
                            }
                            else if (n.title == "Vector3" && i == 0)
                            {
                                // Para nodos Vector3, usar el valor de entrada
                                auto inputIt = cn->inputValues.find(0);
                                if (inputIt != cn->inputValues.end())
                                {
                                    value = &inputIt->second;
                                }
                            }
                            else if (n.title == "Matrix3x3" && i == 0)
                            {
                                // Para nodos Matrix3x3, usar el valor de entrada
                                auto inputIt = cn->inputValues.find(0);
                                if (inputIt != cn->inputValues.end())
                                {
                                    value = &inputIt->second;
                                }
                            }
                            else if (n.title == "Matrix4x4" && i == 0)
                            {
                                // Para nodos Matrix4x4, usar el valor de entrada
                                auto inputIt = cn->inputValues.find(0);
                                if (inputIt != cn->inputValues.end())
                                {
                                    value = &inputIt->second;
                                }
                            }
                            else if (n.title == "GameObject" && i == 0)
                            {
                                // Para nodos GameObject, usar el valor de entrada
                                auto inputIt = cn->inputValues.find(0);
                                if (inputIt != cn->inputValues.end())
                                {
                                    value = &inputIt->second;
                                }
                            }
                        }
                    }
                }

                ImU32 pinColor = GetPinColor(pin.isExec, value);

                // Dibujar pin
                if (pin.isExec)
                {
                    // Pin de ejecución: cuadrado
                    float pinSize = 7.0f;
                    ImVec2 pinMin = pinPos - ImVec2(pinSize, pinSize);
                    ImVec2 pinMax = pinPos + ImVec2(pinSize, pinSize);

                    if (hasConnection)
                    {
                        // Pin conectado: cuadrado lleno
                        draw_list->AddRectFilled(pinMin, pinMax, pinColor);
                        draw_list->AddRect(pinMin, pinMax, IM_COL32(0, 0, 0, 120), 0, 1.0f);
                    }
                    else
                    {
                        // Pin desconectado: cuadrado con borde
                        draw_list->AddRectFilled(pinMin, pinMax, pinColor);
                        draw_list->AddRect(pinMin, pinMax, IM_COL32(0, 0, 0, 120), 0, 1.0f);
                    }
                }
                else
                {
                    // Pin de datos: círculo
                    if (hasConnection)
                    {
                        // Pin conectado: círculo completo
                        draw_list->AddCircleFilled(pinPos, 7, pinColor);
                    }
                    else
                    {
                        // Pin desconectado: anillo
                        draw_list->AddCircle(pinPos, 7, IM_COL32(255, 255, 255, 255), 0, 2.0f);
                        draw_list->AddCircleFilled(pinPos, 8, pinColor);
                    }
                }

                // Área de interacción (más grande para mejor usabilidad)
                ImGui::SetCursorScreenPos(pinPos - ImVec2(12, 12));
                ImGui::InvisibleButton(("outpin" + std::to_string(n.id) + "_" + std::to_string(pin.id)).c_str(), ImVec2(24, 24));

                // Highlight en hover con feedback especial para conexiones
                if (ImGui::IsItemHovered())
                {
                    if (isConnecting && connectingFromNode != -1)
                    {
                        // Highlight especial cuando estamos conectando
                        // Solo mostrar highlight si no estamos en el mismo nodo
                        if (connectingFromNode != n.id)
                        {
                            Node *fromNode = &GetNodeById(connectingFromNode)->n;
                            if (fromNode && connectingFromPin < fromNode->outputs.size())
                            {
                                bool isValidTarget = engine->IsValidConnection(fromNode, connectingFromPin, &n, (int)i);
                                ImU32 highlightColor = isValidTarget ? IM_COL32(100, 255, 100, 150) : IM_COL32(255, 100, 100, 150);
                                draw_list->AddCircle(pinPos, 8, highlightColor, 0, 2.0f);
                            }
                        }
                        else
                        {
                            // Highlight especial para el mismo nodo (sin validación)
                            draw_list->AddCircle(pinPos, 8, IM_COL32(255, 255, 0, 150), 0, 2.0f);
                        }
                    }
                    else
                    {
                        // Highlight normal
                        draw_list->AddCircle(pinPos, 7, IM_COL32(255, 255, 255, 100), 0, 1.0f);
                    }
                }

                // Doble click para eliminar conexiones
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    engine->RemoveConnectionsFromPin(n.id, (int)i, false);
                }
                // Iniciar conexión - cambiar a click para iniciar
                else if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && !isConnecting)
                {
                    connectingFromNode = n.id;
                    connectingFromPin = (int)i;
                    isConnecting = true;
                    std::cout << "[CONNECTION] Started connection from node " << n.id << " pin " << i << std::endl;
                }
            }
            ImGui::PopID();
        }

        // Dibujar línea de conexión temporal estilo Blender
        if (connectingFromNode != -1 && isConnecting)
        {
            Node *fromNode = &GetNodeById(connectingFromNode)->n;
            if (fromNode && connectingFromPin < fromNode->outputs.size())
            {
                // Aplicar transformación del panel a la posición del pin de origen
                ImVec2 p1 = window_pos + panelOffset + fromNode->pos + fromNode->outputs[connectingFromPin].pos;
                ImVec2 p2 = window_pos + panelOffset + transformedMousePos; // Usar la posición del mouse transformada

                bool isExec = fromNode->outputs[connectingFromPin].isExec;
                ImU32 lineColor = isExec ? IM_COL32(255, 255, 255, 200) : IM_COL32(180, 180, 180, 200);
                float thickness = isExec ? 3.0f : 2.0f;

                // Línea temporal que sigue al mouse
                draw_list->AddBezierCubic(p1, p1 + ImVec2(50, 0), p2 - ImVec2(50, 0), p2, lineColor, thickness);

                // Punto en el cursor del mouse
                ImVec2 cursorPos = window_pos + panelOffset + transformedMousePos;
                draw_list->AddCircleFilled(cursorPos, 4, lineColor);

                // Highlight del pin de origen para mejor feedback visual
                draw_list->AddCircle(p1, 8, lineColor, 0, 2.0f);
            }
        }

        // Cancelar conexión
        if (isConnecting && (ImGui::IsMouseClicked(ImGuiMouseButton_Right) || ImGui::IsKeyPressed(ImGuiKey_Escape)))
        {
            connectingFromNode = -1;
            connectingFromPin = -1;
            isConnecting = false;
            std::cout << "[CONNECTION] Connection cancelled by user" << std::endl;
        }
        // También cancelar si hacemos click en el espacio vacío
        else if (isConnecting && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsAnyItemHovered())
        {
            connectingFromNode = -1;
            connectingFromPin = -1;
            isConnecting = false;
            std::cout << "[CONNECTION] Connection cancelled - clicked on empty space" << std::endl;
        }

        // Información del nodo seleccionado (solo cuando hay uno seleccionado)
        if (contextMenuNodeId != -1)
        {
            ImGui::SetCursorScreenPos(window_pos + ImVec2(10, 10));
            CustomNode *selectedNode = GetCustomNodeById(contextMenuNodeId);
            if (selectedNode)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 100, 255));
                ImGui::Text("Selected: %s (ID: %d)", selectedNode->n.title.c_str(), contextMenuNodeId);
                ImGui::PopStyleColor();
            }
        }

        // Menú contextual para nodos seleccionados
        if (showContextMenu)
        {
            ImGui::OpenPopup("NodeContextMenu");
            showContextMenu = false;
        }

        if (ImGui::BeginPopup("NodeContextMenu"))
        {
            if (contextMenuNodeId != -1)
            {
                CustomNode *selectedNode = GetCustomNodeById(contextMenuNodeId);
                if (selectedNode)
                {
                    ImGui::Text("Node: %s", selectedNode->n.title.c_str());
                    ImGui::Separator();

                    if (ImGui::MenuItem("Delete Node", "Del"))
                    {
                        engine->DeleteNode(contextMenuNodeId);
                        contextMenuNodeId = -1;
                    }

                    if (ImGui::MenuItem("Disconnect All", "Ctrl+D"))
                    {
                        engine->RemoveAllConnectionsFromNode(contextMenuNodeId);
                        contextMenuNodeId = -1;
                    }

                    ImGui::Separator();

                    if (ImGui::MenuItem("Execute Graph"))
                    {
                        engine->ExecuteGraph();
                    }

                    if (ImGui::MenuItem("Update Values"))
                    {
                        engine->ForceUpdateAllNodeInputs();
                    }

                    if (ImGui::MenuItem("Reset View"))
                    {
                        panelOffset = ImVec2(0, 0);
                        std::cout << "[PANEL] View reset to default" << std::endl;
                    }

                    ImGui::Separator();

                    if (ImGui::MenuItem("Debug Connection System"))
                    {
                        engine->DebugConnectionSystem();
                    }

                    if (ImGui::MenuItem("Validate Connections"))
                    {
                        engine->ValidateConnectionIntegrity();
                    }

                    if (ImGui::MenuItem("Cleanup Invalid Connections"))
                    {
                        engine->CleanupInvalidConnections();
                    }

                    ImGui::Separator();

                    if (ImGui::MenuItem("Copy Node", "Ctrl+C"))
                    {
                        // TODO: Implementar copia de nodos
                        std::cout << "[MENU] Copy node not implemented yet" << std::endl;
                    }

                    if (ImGui::MenuItem("Duplicate Node", "Ctrl+D"))
                    {
                        // TODO: Implementar duplicación de nodos
                        std::cout << "[MENU] Duplicate node not implemented yet" << std::endl;
                    }
                }
            }
            ImGui::EndPopup();
        }

        // Manejar teclas de acceso rápido
        if (ImGui::IsKeyPressed(ImGuiKey_Delete) && contextMenuNodeId != -1)
        {
            engine->DeleteNode(contextMenuNodeId);
            contextMenuNodeId = -1;
        }

        if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_D))
        {
            if (contextMenuNodeId != -1)
            {
                engine->RemoveAllConnectionsFromNode(contextMenuNodeId);
                contextMenuNodeId = -1;
            }
        }

        // Menú contextual de creación de nodos (estilo Unreal)
        if (showCreateNodeMenu)
        {
            ImGui::OpenPopup("CreateNodeMenu");
            showCreateNodeMenu = false;
        }

        if (ImGui::BeginPopup("CreateNodeMenu"))
        {
            // Hacer que el nodo aparezca en el centro de la ventana del editor
            // Calcular el centro de la ventana y restar panelOffset para compensar el pan
            ImVec2 windowSize = ImGui::GetWindowSize();
            ImVec2 nodePos = (windowSize * 0.5f) - panelOffset;

            if (ImGui::MenuItem("Execute Graph"))
            {
                engine->ExecuteGraph();
            }

            for (auto &nd : engine->PrefabNodes)
            {
                if (ImGui::BeginMenu(nd.cat.c_str()))
                {
                    std::string titleNew = "> " + nd.title;

                    if (ImGui::MenuItem(titleNew.c_str()))
                    {
                        engine->CreateNode(
                            nd.title,         // 🔹 std::string
                            nd.executeFunc,   // 🔹 std::function<void(CustomNode*)>
                            nd.hasExecInput,  // 🔹 bool
                            nd.hasExecOutput, // 🔹 bool
                            nd.inputPins,     // 🔹 vector<pair<string, any>>
                            nd.outputPins,    // 🔹 vector<pair<string, any>>
                            nd.position,      // 🔹 ImVec2
                            nd.size           // 🔹 ImVec2
                        );
                    }

                    ImGui::EndMenu();
                }
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Debug Node Pins (Console)"))
            {
                std::cout << "\n[DEBUG] Available nodes:" << std::endl;
                for (const auto &node : customNodes)
                {
                    std::cout << "  " << node.n.id << ": " << node.n.title << std::endl;
                }
                std::cout << "[DEBUG] Use DebugNodePins(nodeId) to inspect specific nodes" << std::endl;
            }

            ImGui::EndPopup();
        }
    }
};
#pragma once
#include <imgui/imgui.h>
#include <vector>
#include <string>
#include <functional>
#include <iostream>
#include <algorithm>
#include <map>
#include <any>

inline ImVec2 operator+(const ImVec2& a, const ImVec2& b) { return ImVec2(a.x + b.x, a.y + b.y); }
inline ImVec2 operator-(const ImVec2& a, const ImVec2& b) { return ImVec2(a.x - b.x, a.y - b.y); }
inline ImVec2& operator+=(ImVec2& a, const ImVec2& b) { a.x += b.x; a.y += b.y; return a; }

// Pin de datos o ejecución
struct Pin
{
    int id;
    ImVec2 pos;
    bool isExec = false; // si es true, es pin de ejecución
};

// Nodo
struct Node
{
    int id;
    ImVec2 pos;
    ImVec2 size;
    std::string title;
    std::vector<Pin> inputs;
    std::vector<Pin> outputs;

    // Para nodos de ejecución
    std::string data; // para String node
    std::function<void(Node*)> execFunc; // para nodos de ejecución
    bool isEditing = false; // Para controlar si está en modo edición
};

// Conexión
struct Connection
{
    int fromNodeId, fromPinId;
    int toNodeId, toPinId;
};

struct PinInfo{
    std::string _Name = "New Pin";
    std::any _Var;
};

struct CustomNode {
    int nodeId;
    std::vector<PinInfo*> Pins;

    ~CustomNode() {
        for (auto* p : Pins) delete p;
    }

    void Draw (){
        
    }

    template<typename T>
    PinInfo* RegisterPin(const std::string& name, T& var) {
        PinInfo* pin = new PinInfo();
        pin->_Name = name;
        pin->_Var = std::ref(var);
        Pins.push_back(pin);
        return pin;
    }
};

// Editor de nodos
class SimpleNodeEditor
{
public:
    std::vector<Node> nodes;
    std::vector<Connection> connections;
    std::vector<CustomNode> customNodes;

    int connectingFromNode = -1;
    int connectingFromPin = -1;
    int editingNodeId = -1; // ID del nodo que se está editando
    char textBuffer[256] = ""; // Buffer para el texto de entrada

    Node* GetNodeById(int id)
    {
        for (auto& n : nodes)
            if (n.id == id) return &n;
        return nullptr;
    }

    CustomNode* GetCustomNodeById(int id) {
        for (auto& cn : customNodes)
            if (cn.nodeId == id) return &cn;
        return nullptr;
    }


    // Método para verificar si un pin tiene conexiones
    bool HasConnection(int nodeId, int pinId, bool isInput)
    {
        for (auto& c : connections)
        {
            if (isInput) {
                if (c.toNodeId == nodeId && c.toPinId == pinId)
                    return true;
            } else {
                if (c.fromNodeId == nodeId && c.fromPinId == pinId)
                    return true;
            }
        }
        return false;
    }

    // Método para eliminar conexiones de un pin específico
    void RemoveConnectionsFromPin(int nodeId, int pinId, bool isInput)
    {
        auto it = connections.begin();
        while (it != connections.end())
        {
            bool shouldRemove = false;
            if (isInput) {
                // Para pins de entrada, eliminar conexiones que llegan a este pin
                shouldRemove = (it->toNodeId == nodeId && it->toPinId == pinId);
            } else {
                // Para pins de salida, eliminar conexiones que salen de este pin
                shouldRemove = (it->fromNodeId == nodeId && it->fromPinId == pinId);
            }
            
            if (shouldRemove) {
                std::cout << "Removing connection: " << it->fromNodeId << ":" << it->fromPinId 
                         << " -> " << it->toNodeId << ":" << it->toPinId << std::endl;
                it = connections.erase(it);
            } else {
                ++it;
            }
        }
    }

    // Método para validar si una conexión es válida
    bool IsValidConnection(Node* fromNode, int fromPinId, Node* toNode, int toPinId)
    {
        if (!fromNode || !toNode) return false;
        
        // Verificar que los índices sean válidos
        if (fromPinId >= fromNode->outputs.size() || toPinId >= toNode->inputs.size())
            return false;

        Pin& fromPin = fromNode->outputs[fromPinId];
        Pin& toPin = toNode->inputs[toPinId];

        // Solo se pueden conectar pins del mismo tipo
        // Ejecución con ejecución, datos con datos
        if (fromPin.isExec != toPin.isExec) return false;

        // No permitir auto-conexiones
        if (fromNode->id == toNode->id) return false;

        // Verificar si ya existe una conexión en el pin de entrada
        for (auto& c : connections)
        {
            if (c.toNodeId == toNode->id && c.toPinId == toPinId)
                return false; // Ya hay una conexión en este pin de entrada
        }

        return true;
    }

    void AddNode(int id, const ImVec2& pos, const ImVec2& size, const std::string& title,
                 int numInputs = 1, int numOutputs = 1, bool isExecInput=false, bool isExecOutput=false)
    {
        Node n;
        n.id = id;
        n.pos = pos;
        n.size = size;
        n.title = title;

        for (int i = 0; i < numInputs; i++)
            n.inputs.push_back({i, ImVec2(0, size.y * (i+1)/(numInputs+1)), isExecInput});

        for (int i = 0; i < numOutputs; i++)
            n.outputs.push_back({i, ImVec2(size.x, size.y * (i+1)/(numOutputs+1)), isExecOutput});

        nodes.push_back(n);
    }

    void AddCustomNode(int id, const ImVec2& pos, const ImVec2& size, const std::string& title,
                    std::function<void(CustomNode&)> setup)
    {
        // Crear nodo gráfico
        Node n;
        n.id = id;
        n.pos = pos;
        n.size = size;
        n.title = title;
        nodes.push_back(n);

        // Crear nodo lógico
        customNodes.emplace_back();
        CustomNode& cn = customNodes.back();
        cn.nodeId = id;

        // El usuario registra pines aquí
        setup(cn);

        // Crear pines gráficos a partir de los lógicos
        for (int i = 0; i < cn.Pins.size(); i++) {
            nodes.back().inputs.push_back({
                i,
                ImVec2(0, (i == 0 ? 35 : (35 - 7) * (i + 1))),
                false
            });
        }
    }



    // Nuevo método para agregar nodo de Print
    void AddPrintNode(int id, const ImVec2& pos, const std::string& message = "Hello World!")
    {
        Node n;
        n.id = id;
        n.pos = pos;
        n.size = ImVec2(140, 80);
        n.title = "Print";
        n.data = message;

        // Pin de ejecución de entrada (rojo) - posición ajustada
        n.inputs.push_back({0, ImVec2(0, 30), true});
        // Pin de datos de entrada para el texto (amarillo) - posición ajustada
        n.inputs.push_back({1, ImVec2(0, 55), false});
        // Pin de ejecución de salida (rojo) - posición ajustada
        n.outputs.push_back({0, ImVec2(140, 30), true});

        // Función de ejecución
        n.execFunc = [](Node* node) {
            std::cout << "Print Node [" << node->id << "]: " << node->data << std::endl;
        };

        nodes.push_back(n);
    }

    // Nuevo método para agregar nodo de Start (para iniciar la ejecución)
    void AddStartNode(int id, const ImVec2& pos)
    {
        Node n;
        n.id = id;
        n.pos = pos;
        n.size = ImVec2(90, 50);
        n.title = "Start";

        // Solo pin de ejecución de salida
        n.outputs.push_back({0, ImVec2(90, 30), true});

        // Función de ejecución (no hace nada, solo inicia)
        n.execFunc = [](Node* node) {
            std::cout << "Starting execution..." << std::endl;
        };

        nodes.push_back(n);
    }

    // Nuevo método para agregar nodo de String (proporciona datos de texto)
    void AddStringNode(int id, const ImVec2& pos, const std::string& value = "Default Text")
    {
        Node n;
        n.id = id;
        n.pos = pos;
        n.size = ImVec2(120, 60);
        n.title = "String";
        n.data = value;

        // Solo pin de datos de salida - centrado verticalmente
        n.outputs.push_back({0, ImVec2(120, 35), false});

        nodes.push_back(n);
    }

    // Método para obtener el valor de un nodo conectado
    std::string GetConnectedValue(int nodeId, int pinId)
    {
        for (auto& c : connections)
        {
            if (c.toNodeId == nodeId && c.toPinId == pinId)
            {
                Node* sourceNode = GetNodeById(c.fromNodeId);
                if (sourceNode)
                    return sourceNode->data;
            }
        }
        return "";
    }

    void ExecuteFrom(Node* node)
    {
        if (!node || !node->execFunc) return;

        // Si es un nodo Print, verificar si tiene datos conectados
        if (node->title == "Print")
        {
            // Buscar conexión de datos en el pin 1 (pin de texto)
            std::string connectedValue = GetConnectedValue(node->id, 1);
            if (!connectedValue.empty())
                node->data = connectedValue;
        }

        node->execFunc(node);

        // Propagar ejecución a nodos conectados por pins de ejecución
        for (auto& c : connections)
        {
            if (c.fromNodeId == node->id)
            {
                Node* next = GetNodeById(c.toNodeId);
                if (next && c.fromPinId < node->outputs.size() && node->outputs[c.fromPinId].isExec)
                    ExecuteFrom(next);
            }
        }
    }

    // Método para ejecutar desde un nodo Start
    void ExecuteGraph()
    {
        for (auto& node : nodes)
        {
            if (node.title == "Start")
            {
                ExecuteFrom(&node);
                break;
            }
        }
    }

    void Draw()
    {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 window_pos = ImGui::GetWindowPos();
        ImVec2 mousePos = ImGui::GetIO().MousePos - window_pos;

        // Dibujar conexiones
        for (auto& c : connections)
        {
            Node* fromNode = GetNodeById(c.fromNodeId);
            Node* toNode = GetNodeById(c.toNodeId);
            if (!fromNode || !toNode) continue;

            if (c.fromPinId >= fromNode->outputs.size() || c.toPinId >= toNode->inputs.size()) continue;

            ImVec2 p1 = window_pos + fromNode->pos + fromNode->outputs[c.fromPinId].pos;
            ImVec2 p2 = window_pos + toNode->pos + toNode->inputs[c.toPinId].pos;

            // Línea más gruesa y con mejor apariencia
            draw_list->AddBezierCubic(
                p1, p1 + ImVec2(50,0),
                p2 - ImVec2(50,0), p2,
                fromNode->outputs[c.fromPinId].isExec ? IM_COL32(255,80,80,255) : IM_COL32(255,200,80,255),
                3.5f
            );
        }

        // Dibujar nodos
        for (auto& n : nodes)
        {
            ImGui::PushID(n.id);
            ImVec2 min = window_pos + n.pos;
            ImVec2 max = window_pos + n.pos + n.size;

            // Colores mejorados y más modernos
            ImU32 nodeColor, headerColor, borderColor;
            if (n.title == "Print") {
                nodeColor = IM_COL32(45, 45, 45, 255);
                headerColor = IM_COL32(60, 60, 60, 255);
                borderColor = IM_COL32(80, 80, 80, 255);
            }
            else if (n.title == "Start") {
                nodeColor = IM_COL32(45, 45, 45, 255);
                headerColor = IM_COL32(60, 60, 60, 255);
                borderColor = IM_COL32(80, 80, 80, 255);
            }
            else if (n.title == "String") {
                nodeColor = IM_COL32(45, 45, 45, 255);
                headerColor = IM_COL32(60, 60, 60, 255);
                borderColor = IM_COL32(80, 80, 80, 255);
            }
            else {
                nodeColor = IM_COL32(45, 45, 45, 255);
                headerColor = IM_COL32(60, 60, 60, 255);
                borderColor = IM_COL32(80, 80, 80, 255);
            }

            // Fondo del nodo con bordes menos redondeados
            draw_list->AddRectFilled(min, max, nodeColor, 3.0f);
            
            // Header del nodo
            ImVec2 headerMax = ImVec2(max.x, min.y + 22);
            draw_list->AddRectFilled(min, headerMax, headerColor, 3.0f, ImDrawFlags_RoundCornersTop);
            
            // Borde del nodo
            draw_list->AddRect(min, max, borderColor, 3.0f, 0, 1.5f);

            // Título del nodo
            ImVec2 text_size = ImGui::CalcTextSize(n.title.c_str());
            draw_list->AddText(min + ImVec2((n.size.x - text_size.x)*0.5f, 3), IM_COL32(255,255,255,255), n.title.c_str());

            // Área de contenido del nodo (para inputs de texto) - ALINEADO CON PINES
            if (n.title == "Print")
            {
                // Para el nodo Print, el texto debe estar alineado con el pin de datos (pin 1)
                if (n.inputs.size() > 1)
                {
                    float pinY = n.inputs[1].pos.y; // Pin de datos (índice 1)
                    ImVec2 contentMin = ImVec2(min.x + 15, min.y + pinY - 8); // Centrado en el pin
                    ImVec2 contentMax = ImVec2(max.x - 8, min.y + pinY + 8);
                    
                    // Si este nodo está siendo editado
                    if (editingNodeId == n.id)
                    {
                        ImGui::SetCursorScreenPos(contentMin);
                        ImGui::PushItemWidth(contentMax.x - contentMin.x);
                        
                        // Copiar el dato actual al buffer si acabamos de empezar a editar
                        if (strlen(textBuffer) == 0 && !n.data.empty())
                        {
                            strncpy(textBuffer, n.data.c_str(), sizeof(textBuffer) - 1);
                            textBuffer[sizeof(textBuffer) - 1] = '\0';
                        }
                        
                        ImGui::SetKeyboardFocusHere();
                        if (ImGui::InputText(("##edit" + std::to_string(n.id)).c_str(), textBuffer, sizeof(textBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
                        {
                            // Aplicar cambios al presionar Enter
                            n.data = std::string(textBuffer);
                            editingNodeId = -1;
                            memset(textBuffer, 0, sizeof(textBuffer));
                        }
                        
                        // Salir de edición si se hace click fuera o se presiona Escape
                        if (ImGui::IsKeyPressed(ImGuiKey_Escape) || 
                            (!ImGui::IsItemActive() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsItemHovered()))
                        {
                            editingNodeId = -1;
                            memset(textBuffer, 0, sizeof(textBuffer));
                        }
                        
                        ImGui::PopItemWidth();
                    }
                    else
                    {
                        // Mostrar el texto actual
                        std::string displayData = n.data.length() > 12 ? n.data.substr(0, 9) + "..." : n.data;
                        
                        // Área clickeable para editar
                        ImGui::SetCursorScreenPos(contentMin);
                        ImGui::InvisibleButton(("text" + std::to_string(n.id)).c_str(), ImVec2(contentMax.x - contentMin.x, 16));
                        
                        // Fondo del área de texto
                        if (ImGui::IsItemHovered())
                        {
                            draw_list->AddRectFilled(contentMin, contentMax, IM_COL32(255,255,255,20), 2.0f);
                        }
                        
                        // Iniciar edición con doble click
                        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                        {
                            editingNodeId = n.id;
                            memset(textBuffer, 0, sizeof(textBuffer));
                        }
                        
                        draw_list->AddText(contentMin + ImVec2(4, 0), IM_COL32(220,220,220,255), displayData.c_str());
                    }
                }
            }
            else if (n.title == "String")
            {
                // Para el nodo String, el texto debe estar centrado con el pin de salida
                if (n.outputs.size() > 0)
                {
                    float pinY = n.outputs[0].pos.y; // Pin de datos de salida
                    ImVec2 contentMin = ImVec2(min.x + 8, min.y + pinY - 8); // Centrado en el pin
                    ImVec2 contentMax = ImVec2(max.x - 15, min.y + pinY + 8);
                    
                    // Si este nodo está siendo editado
                    if (editingNodeId == n.id)
                    {
                        ImGui::SetCursorScreenPos(contentMin);
                        ImGui::PushItemWidth(contentMax.x - contentMin.x);
                        
                        // Copiar el dato actual al buffer si acabamos de empezar a editar
                        if (strlen(textBuffer) == 0 && !n.data.empty())
                        {
                            strncpy(textBuffer, n.data.c_str(), sizeof(textBuffer) - 1);
                            textBuffer[sizeof(textBuffer) - 1] = '\0';
                        }
                        
                        ImGui::SetKeyboardFocusHere();
                        if (ImGui::InputText(("##edit" + std::to_string(n.id)).c_str(), textBuffer, sizeof(textBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
                        {
                            // Aplicar cambios al presionar Enter
                            n.data = std::string(textBuffer);
                            editingNodeId = -1;
                            memset(textBuffer, 0, sizeof(textBuffer));
                        }
                        
                        // Salir de edición si se hace click fuera o se presiona Escape
                        if (ImGui::IsKeyPressed(ImGuiKey_Escape) || 
                            (!ImGui::IsItemActive() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsItemHovered()))
                        {
                            editingNodeId = -1;
                            memset(textBuffer, 0, sizeof(textBuffer));
                        }
                        
                        ImGui::PopItemWidth();
                    }
                    else
                    {
                        // Mostrar el texto actual
                        std::string displayData = n.data.length() > 10 ? n.data.substr(0, 7) + "..." : n.data;
                        
                        // Área clickeable para editar
                        ImGui::SetCursorScreenPos(contentMin);
                        ImGui::InvisibleButton(("text" + std::to_string(n.id)).c_str(), ImVec2(contentMax.x - contentMin.x, 16));
                        
                        // Fondo del área de texto
                        if (ImGui::IsItemHovered())
                        {
                            draw_list->AddRectFilled(contentMin, contentMax, IM_COL32(255,255,255,20), 2.0f);
                        }
                        
                        // Iniciar edición con doble click
                        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                        {
                            editingNodeId = n.id;
                            memset(textBuffer, 0, sizeof(textBuffer));
                        }
                        
                        draw_list->AddText(contentMin + ImVec2(4, 0), IM_COL32(220,220,220,255), displayData.c_str());
                    }
                }
            }
            else if (CustomNode* cn = GetCustomNodeById(n.id))
            {
                // Para cada PinInfo dibujamos un área de edición
                for (int i = 0; i < cn->Pins.size(); i++)
                {
                    PinInfo* p = cn->Pins[i];
                    float pinY = n.inputs[i].pos.y; // Pin de datos (índice 1)
                    ImVec2 contentMin = ImVec2(min.x + 8, min.y + pinY - 8); // Centrado en el pin
                    ImVec2 contentMax = ImVec2(max.x - 15, min.y + pinY + 8);

                    ImGui::SetCursorScreenPos(contentMin);
                    ImGui::PushItemWidth(contentMax.x - contentMin.x);

                    // Detectar tipo del pin
                    if (p->_Var.type() == typeid(std::string)) {
                        std::string& value = *std::any_cast<std::reference_wrapper<std::string>>(&p->_Var);
                        char buf[128]; strncpy(buf, value.c_str(), sizeof(buf));
                        if (ImGui::InputText(("##" + std::to_string(n.id) + "_" + p->_Name).c_str(), buf, sizeof(buf))) {
                            value = buf;
                        }
                    }
                    else if (p->_Var.type() == typeid(int)) {
                        int& value = *std::any_cast<std::reference_wrapper<int>>(&p->_Var);
                        ImGui::InputInt(("##" + std::to_string(n.id) + "_" + p->_Name).c_str(), &value);
                    }
                    else if (p->_Var.type() == typeid(float)) {
                        float& value = *std::any_cast<std::reference_wrapper<float>>(&p->_Var);
                        ImGui::InputFloat(("##" + std::to_string(n.id) + "_" + p->_Name).c_str(), &value, 0.1f, 1.0f, "%.3f");
                    }
                    else {
                        ImGui::TextUnformatted(p->_Name.c_str());
                    }

                    ImGui::PopItemWidth();
                }
            }


            // Botón invisible para arrastrar el nodo (solo en el header)
            ImGui::SetCursorScreenPos(min);
            ImGui::InvisibleButton(("node" + std::to_string(n.id)).c_str(), ImVec2(n.size.x, 22));
            if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left) && editingNodeId != n.id)
                n.pos += ImGui::GetIO().MouseDelta;

            // PINS DE ENTRADA
            for (size_t i = 0; i < n.inputs.size(); i++)
            {
                Pin& pin = n.inputs[i];
                ImVec2 pinPos = window_pos + n.pos + pin.pos;
                
                // Verificar si el pin tiene conexiones
                bool hasConnection = HasConnection(n.id, (int)i, true);
                ImU32 pinColor = pin.isExec ? IM_COL32(255,80,80,255) : IM_COL32(255,200,80,255);
                
                if (hasConnection) {
                    // Si tiene conexión, dibujar círculo completo
                    draw_list->AddCircleFilled(pinPos, 6, pinColor);
                    draw_list->AddCircle(pinPos, 6, IM_COL32(255,255,255,100), 0, 1.0f);
                } else {
                    // Si no tiene conexión, dibujar como wire
                    ImVec2 wireStart = pinPos;
                    ImVec2 wireEnd = pinPos + ImVec2(10, 0);
                    draw_list->AddCircleFilled(pinPos, 5, IM_COL32(60, 60, 60, 255));
                    draw_list->AddCircle(pinPos, 6, pinColor);
                }

                ImGui::SetCursorScreenPos(pinPos - ImVec2(8,8));
                ImGui::InvisibleButton(("inpin" + std::to_string(n.id) + "_" + std::to_string(pin.id)).c_str(), ImVec2(16,16));

                // Detectar doble click para eliminar conexiones
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    std::cout << "Double clicked on input pin " << n.id << ":" << i << std::endl;
                    RemoveConnectionsFromPin(n.id, (int)i, true);
                }
                // Conectar al soltar el mouse
                else if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Left) && connectingFromNode != -1)
                {
                    Node* fromNode = GetNodeById(connectingFromNode);
                    if (fromNode && IsValidConnection(fromNode, connectingFromPin, &n, (int)i))
                    {
                        connections.push_back({connectingFromNode, connectingFromPin, n.id, (int)i});
                    }
                    connectingFromNode = -1;
                }
            }

            // PINS DE SALIDA
            for (size_t i = 0; i < n.outputs.size(); i++)
            {
                Pin& pin = n.outputs[i];
                ImVec2 pinPos = window_pos + n.pos + pin.pos;
                
                // Verificar si el pin tiene conexiones
                bool hasConnection = HasConnection(n.id, (int)i, false);
                ImU32 pinColor = pin.isExec ? IM_COL32(255,80,80,255) : IM_COL32(255,200,80,255);
                
                if (hasConnection) {
                    draw_list->AddCircleFilled(pinPos, 6, pinColor);
                    draw_list->AddCircle(pinPos, 6, IM_COL32(255,255,255,100), 0, 1.0f);
                } else {
                    // Si no tiene conexión, dibujar como wire
                    ImVec2 wireStart = pinPos;
                    ImVec2 wireEnd = pinPos + ImVec2(-10, 0);
                    draw_list->AddCircleFilled(pinPos, 5, IM_COL32(60, 60, 60, 255));
                    draw_list->AddCircle(pinPos, 6, pinColor);
                }

                ImGui::SetCursorScreenPos(pinPos - ImVec2(8,8));
                ImGui::InvisibleButton(("outpin" + std::to_string(n.id) + "_" + std::to_string(pin.id)).c_str(), ImVec2(16,16));

                // Detectar doble click para eliminar conexiones
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    std::cout << "Double clicked on output pin " << n.id << ":" << i << std::endl;
                    RemoveConnectionsFromPin(n.id, (int)i, false);
                }
                // Iniciar conexión al arrastrar
                else if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
                {
                    connectingFromNode = n.id;
                    connectingFromPin = (int)i;
                }
            }
            ImGui::PopID();
        }

        if (connectingFromNode != -1)
        {
            Node* fromNode = GetNodeById(connectingFromNode);
            if (fromNode && connectingFromPin < fromNode->outputs.size())
            {
                ImVec2 p1 = window_pos + fromNode->pos + fromNode->outputs[connectingFromPin].pos;
                ImVec2 p2 = ImGui::GetIO().MousePos;
                
                ImU32 lineColor = fromNode->outputs[connectingFromPin].isExec ? 
                    IM_COL32(255,80,80,150) : IM_COL32(255,200,80,150);
                
                draw_list->AddBezierCubic(p1, p1+ImVec2(50,0), p2-ImVec2(50,0), p2, lineColor, 3.0f);

                if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
                    connectingFromNode = -1;
            }
        }

        // Botón para ejecutar el grafo
        ImGui::SetCursorScreenPos(window_pos + ImVec2(10, 10));
        if (ImGui::Button("Execute Graph"))
        {
            ExecuteGraph();
        }
        
        // Instrucciones
        ImGui::SetCursorScreenPos(window_pos + ImVec2(120, 10));
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Double-click text to edit");
    }
};
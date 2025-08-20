#pragma once
#include <imgui/imgui.h>
#include <vector>
#include <string>
#include <functional>
#include <iostream>
#include <algorithm>

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
};

// Conexión
struct Connection
{
    int fromNodeId, fromPinId;
    int toNodeId, toPinId;
};

// Editor de nodos
class SimpleNodeEditor
{
public:
    std::vector<Node> nodes;
    std::vector<Connection> connections;

    int connectingFromNode = -1;
    int connectingFromPin = -1;

    Node* GetNodeById(int id)
    {
        for (auto& n : nodes)
            if (n.id == id) return &n;
        return nullptr;
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

    // Nuevo método para agregar nodo de Print
    void AddPrintNode(int id, const ImVec2& pos, const std::string& message = "Hello World!")
    {
        Node n;
        n.id = id;
        n.pos = pos;
        n.size = ImVec2(120, 60);
        n.title = "Print";
        n.data = message; // El mensaje a imprimir

        // Pin de ejecución de entrada (rojo)
        n.inputs.push_back({0, ImVec2(0, 20), true});
        // Pin de datos de entrada para el texto (amarillo)
        n.inputs.push_back({1, ImVec2(0, 40), false});
        // Pin de ejecución de salida (rojo)
        n.outputs.push_back({0, ImVec2(120, 20), true});

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
        n.size = ImVec2(80, 40);
        n.title = "Start";

        // Solo pin de ejecución de salida
        n.outputs.push_back({0, ImVec2(80, 20), true});

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
        n.size = ImVec2(100, 40);
        n.title = "String";
        n.data = value;

        // Solo pin de datos de salida
        n.outputs.push_back({0, ImVec2(100, 20), false});

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

            draw_list->AddBezierCubic(
                p1, p1 + ImVec2(50,0),
                p2 - ImVec2(50,0), p2,
                fromNode->outputs[c.fromPinId].isExec ? IM_COL32(255,100,100,255) : IM_COL32(255,255,100,255),
                3.0f
            );
        }

        // Dibujar nodos
        for (auto& n : nodes)
        {
            ImVec2 min = window_pos + n.pos;
            ImVec2 max = window_pos + n.pos + n.size;

            // Color diferente para diferentes tipos de nodos
            ImU32 nodeColor = IM_COL32(100,100,200,255);
            if (n.title == "Print") nodeColor = IM_COL32(100,150,100,255);
            else if (n.title == "Start") nodeColor = IM_COL32(150,100,100,255);
            else if (n.title == "String") nodeColor = IM_COL32(150,150,100,255);

            draw_list->AddRectFilled(min, max, nodeColor, 8.0f);
            draw_list->AddRect(min, max, IM_COL32(255,255,255,255), 8.0f);

            ImVec2 text_size = ImGui::CalcTextSize(n.title.c_str());
            draw_list->AddText(min + ImVec2((n.size.x - text_size.x)*0.5f, 5), IM_COL32(255,255,255,255), n.title.c_str());

            // Mostrar datos del nodo si tiene
            if (!n.data.empty() && n.title != "Start")
            {
                std::string displayData = n.data.length() > 15 ? n.data.substr(0, 12) + "..." : n.data;
                ImVec2 data_size = ImGui::CalcTextSize(displayData.c_str());
                draw_list->AddText(min + ImVec2((n.size.x - data_size.x)*0.5f, 25), IM_COL32(200,200,200,255), displayData.c_str());
            }

            ImGui::SetCursorScreenPos(min);
            ImGui::InvisibleButton(("node" + std::to_string(n.id)).c_str(), n.size);
            if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
                n.pos += ImGui::GetIO().MouseDelta;

            // PINS DE ENTRADA
            for (size_t i = 0; i < n.inputs.size(); i++)
            {
                Pin& pin = n.inputs[i];
                ImVec2 pinPos = window_pos + n.pos + pin.pos;
                draw_list->AddCircleFilled(pinPos, 5, pin.isExec ? IM_COL32(255,100,100,255) : IM_COL32(255,255,100,255));

                ImGui::SetCursorScreenPos(pinPos - ImVec2(5,5));
                ImGui::InvisibleButton(("inpin" + std::to_string(n.id) + "_" + std::to_string(pin.id)).c_str(), ImVec2(10,10));

                // Detectar doble click para eliminar conexiones usando ImGui nativo
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    std::cout << "Double clicked on input pin " << n.id << ":" << i << std::endl;
                    RemoveConnectionsFromPin(n.id, (int)i, true);
                }
                // Conectar al soltar el mouse (solo si no hay doble click)
                else if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Left) && connectingFromNode != -1)
                {
                    // Validar que la conexión sea compatible
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
                draw_list->AddCircleFilled(pinPos, 5, pin.isExec ? IM_COL32(255,100,100,255) : IM_COL32(255,255,100,255));

                ImGui::SetCursorScreenPos(pinPos - ImVec2(5,5));
                ImGui::InvisibleButton(("outpin" + std::to_string(n.id) + "_" + std::to_string(pin.id)).c_str(), ImVec2(10,10));

                // Detectar doble click para eliminar conexiones usando ImGui nativo
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    std::cout << "Double clicked on output pin " << n.id << ":" << i << std::endl;
                    RemoveConnectionsFromPin(n.id, (int)i, false);
                }
                // Iniciar conexión al arrastrar (solo si no es doble click)
                else if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
                {
                    connectingFromNode = n.id;
                    connectingFromPin = (int)i;
                }
            }
        }

        // Línea temporal mientras se arrastra una conexión
        if (connectingFromNode != -1)
        {
            Node* fromNode = GetNodeById(connectingFromNode);
            if (fromNode && connectingFromPin < fromNode->outputs.size())
            {
                ImVec2 p1 = window_pos + fromNode->pos + fromNode->outputs[connectingFromPin].pos;
                ImVec2 p2 = ImGui::GetIO().MousePos;
                
                // Color de la línea temporal según el tipo de pin
                ImU32 lineColor = fromNode->outputs[connectingFromPin].isExec ? 
                    IM_COL32(255,100,100,200) : IM_COL32(255,255,100,200);
                
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
    }
};

// Ejemplo de uso:
/*
SimpleNodeEditor editor;

// Agregar nodos
editor.AddStartNode(1, ImVec2(50, 100));
editor.AddPrintNode(2, ImVec2(200, 100), "Hello from Print Node!");
editor.AddStringNode(3, ImVec2(50, 200), "Custom Message");
editor.AddPrintNode(4, ImVec2(350, 200), "Second Print");

// En el loop de rendering:
// editor.Draw();
*/
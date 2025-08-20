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

/*
 * SimpleNodeEditor - Sistema de nodos visual con Lambda Factory
 *
 * Ejemplo de uso:
 *
 * // Crear nodos simples usando la lambda factory
 * auto printNode = editor.CreatePrintNode(ImVec2(200, 100));
 * auto stringNode = editor.CreateStringNode("Mi mensaje", ImVec2(50, 200));
 *
 * // Crear nodos personalizados
 * auto customNode = editor.CreateNode(
 *     "Mi Nodo",                          // título
 *     [](CustomNode* node) {              // lambda de ejecución
 *         std::string input = node->GetInputValue<std::string>(0);
 *         node->SetOutputValue<std::string>(0, "Processed: " + input);
 *     },
 *     true,                               // hasExecInput
 *     true,                               // hasExecOutput
 *     {{"Input", std::string("default")}}, // pines de entrada
 *     {{"Output", std::string("")}},       // pines de salida
 *     ImVec2(100, 100)                    // posición
 * );
 */

static int NodeID = 0;

inline ImVec2 operator+(const ImVec2 &a, const ImVec2 &b) { return ImVec2(a.x + b.x, a.y + b.y); }
inline ImVec2 operator-(const ImVec2 &a, const ImVec2 &b) { return ImVec2(a.x - b.x, a.y - b.y); }
inline ImVec2 &operator+=(ImVec2 &a, const ImVec2 &b)
{
    a.x += b.x;
    a.y += b.y;
    return a;
}

enum PinType
{
    ExecuteOutput,
    ExecuteInput,
    Input,
    Output
};

struct Pin
{
    int id;
    ImVec2 pos;
    bool isExec = false;
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
    std::string data;                     // para String node
    std::function<void(Node *)> execFunc; // para nodos de ejecución
    bool isEditing = false;               // Para controlar si está en modo edición
};

// Conexión
struct Connection
{
    int fromNodeId, fromPinId;
    int toNodeId, toPinId;
};

// Declaraciones adelantadas
class SimpleNodeEditor;
class CustomNode;

struct PinInfo
{
    std::string _Name = "New Pin";
    std::any _Var;
};

// Configuración para crear pins
struct PinConfig
{
    std::string name;
    PinType type;
    std::any defaultValue;
    ImVec2 position;

    PinConfig(const std::string &n, PinType t, std::any def = std::string(""), ImVec2 pos = ImVec2(0, 0))
        : name(n), type(t), defaultValue(def), position(pos) {}
};

// Configuración para crear nodos personalizados
struct NodeConfig
{
    std::string title;
    ImVec2 size;
    std::vector<PinConfig> inputPins;
    std::vector<PinConfig> outputPins;
    std::function<void(CustomNode *)> executeFunction;

    NodeConfig(const std::string &t, ImVec2 s = ImVec2(220, 100))
        : title(t), size(s) {}
};

// Definición de CustomNode ANTES de SimpleNodeEditor
class CustomNode
{
public:
    int nodeId;
    Node n;
    std::vector<PinInfo *> Pins;
    SimpleNodeEditor *parentEditor = nullptr; // Inicializar como nullptr

    // Almacenamiento de datos del nodo
    std::map<std::string, std::any> nodeData;
    std::map<int, std::any> inputValues;    // Valores de entrada por índice de pin
    std::map<int, std::any> outputValues;   // Valores de salida por índice de pin
    std::map<int, std::string> inputNames;  // Nombres de los pins de entrada
    std::map<int, std::string> outputNames; // Nombres de los pins de salida

    void SetupNode(SimpleNodeEditor *parentEditor)
    {
        this->parentEditor = parentEditor; // Asignar el parentEditor
        n.id = NodeID;
        nodeId = n.id;
        n.pos = ImVec2(50, 100);
        n.size = ImVec2(140, 80);
        n.title = "Print";
        n.data = "Hello";

        // Pin de ejecución de entrada (rojo) - posición ajustada
        n.inputs.push_back({0, ImVec2(0, 45), true});
        // Pin de datos de entrada para el texto (amarillo) - posición ajustada
        n.inputs.push_back({1, ImVec2(0, 70), false});
        // Pin de ejecución de salida (rojo) - posición ajustada
        n.outputs.push_back({0, ImVec2(140, 45), true});

        // Función de ejecución
        n.execFunc = [](Node *node)
        {
            std::cout << "Custom Node [" << node->id << "]: " << node->data << std::endl;
        };
        NodeID++;
    }

    void Draw(); // Declaración - implementación después de SimpleNodeEditor

    template <typename T>
    PinInfo *RegisterPin(const std::string &name, T &var)
    {
        PinInfo *pin = new PinInfo();
        pin->_Name = name;
        pin->_Var = std::ref(var);
        Pins.push_back(pin);
        return pin;
    }

    // Métodos para manejar datos del nodo
    template <typename T>
    void SetInputValue(int pinIndex, const T &value)
    {
        inputValues[pinIndex] = value;
    }

    template <typename T>
    T GetInputValue(int pinIndex, const T &defaultValue = T{})
    {
        auto it = inputValues.find(pinIndex);
        if (it != inputValues.end())
        {
            try
            {
                return std::any_cast<T>(it->second);
            }
            catch (const std::bad_any_cast &)
            {
                return defaultValue;
            }
        }
        return defaultValue;
    }

    template <typename T>
    void SetOutputValue(int pinIndex, const T &value)
    {
        outputValues[pinIndex] = value;
    }

    template <typename T>
    T GetOutputValue(int pinIndex, const T &defaultValue = T{})
    {
        auto it = outputValues.find(pinIndex);
        if (it != outputValues.end())
        {
            try
            {
                return std::any_cast<T>(it->second);
            }
            catch (const std::bad_any_cast &)
            {
                return defaultValue;
            }
        }
        return defaultValue;
    }
};

// Editor de nodos
class SimpleNodeEditor
{
public:
    std::vector<CustomNode> customNodes;
    std::vector<Connection> connections;

    int connectingFromNode = -1;
    int connectingFromPin = -1;
    int editingNodeId = -1;    // ID del nodo que se está editando
    char textBuffer[256] = ""; // Buffer para el texto de entrada
    bool isConnecting = false; // Flag para indicar si estamos en modo conexión

    CustomNode *GetNodeById(int id)
    {
        for (auto &n : customNodes)
            if (n.n.id == id)
                return &n;
        return nullptr;
    }

    CustomNode *GetCustomNodeById(int id)
    {
        for (auto &cn : customNodes)
            if (cn.nodeId == id)
                return &cn;
        return nullptr;
    }

    // Función principal para crear nodos personalizados
    CustomNode *CreateCustomNode(const NodeConfig &config, ImVec2 position = ImVec2(50, 100))
    {
        CustomNode newNode;
        newNode.SetupNode(this);
        newNode.n.title = config.title;
        newNode.n.size = config.size;
        newNode.n.pos = position;

        // Limpiar pins existentes
        newNode.n.inputs.clear();
        newNode.n.outputs.clear();

        // Configurar pins de entrada
        float inputY = 45.0f; // Movido más abajo desde 30.0f
        for (size_t i = 0; i < config.inputPins.size(); i++)
        {
            const auto &pinConfig = config.inputPins[i];
            Pin inputPin;
            inputPin.id = (int)i;
            inputPin.pos = ImVec2(0, inputY);
            inputPin.isExec = (pinConfig.type == ExecuteInput);
            newNode.n.inputs.push_back(inputPin);

            // Guardar el nombre del pin
            newNode.inputNames[i] = pinConfig.name;

            // Establecer valor por defecto si es un pin de datos
            if (pinConfig.type == Input)
            {
                newNode.inputValues[i] = pinConfig.defaultValue;
            }

            inputY += 30.0f; // Aumentar espaciado entre pins
        }

        // Configurar pins de salida
        float outputY = 45.0f; // Movido más abajo desde 30.0f
        for (size_t i = 0; i < config.outputPins.size(); i++)
        {
            const auto &pinConfig = config.outputPins[i];
            Pin outputPin;
            outputPin.id = (int)i;
            outputPin.pos = ImVec2(config.size.x, outputY);
            outputPin.isExec = (pinConfig.type == ExecuteOutput);
            newNode.n.outputs.push_back(outputPin);

            // Guardar el nombre del pin
            newNode.outputNames[i] = pinConfig.name;

            outputY += 30.0f; // Aumentar espaciado entre pins
        }

        // Configurar función de ejecución
        if (config.executeFunction)
        {
            newNode.n.execFunc = [this, config](Node *node)
            {
                // Encontrar el CustomNode correspondiente
                CustomNode *customNode = this->GetCustomNodeById(node->id);
                if (customNode && config.executeFunction)
                {
                    // Actualizar valores de entrada desde conexiones antes de ejecutar
                    this->UpdateNodeInputs(customNode);
                    // Ejecutar la función personalizada
                    config.executeFunction(customNode);
                }
            };
        }

        // Ajustar tamaño del nodo basado en el número de pins
        float maxPins = std::max(config.inputPins.size(), config.outputPins.size());
        if (maxPins > 0)
        {
            newNode.n.size.y = std::max(100.0f, 45.0f + maxPins * 30.0f + 20.0f);
        }

        customNodes.push_back(newNode);
        return &customNodes.back();
    }

    // Función para actualizar los valores de entrada de un nodo desde sus conexiones
    void UpdateNodeInputs(CustomNode *node)
    {
        if (!node)
            return;

        // Revisar todas las conexiones que llegan a este nodo
        for (const auto &connection : connections)
        {
            if (connection.toNodeId == node->n.id)
            {
                // Encontrar el nodo fuente
                CustomNode *sourceNode = GetCustomNodeById(connection.fromNodeId);
                if (sourceNode)
                {
                    // Obtener el valor del pin de salida del nodo fuente
                    if (connection.fromPinId < sourceNode->n.outputs.size() &&
                        connection.toPinId < node->n.inputs.size())
                    {
                        // Solo transferir datos para pins que no son de ejecución
                        if (!sourceNode->n.outputs[connection.fromPinId].isExec)
                        {
                            auto outputValue = sourceNode->outputValues.find(connection.fromPinId);
                            if (outputValue != sourceNode->outputValues.end())
                            {
                                node->inputValues[connection.toPinId] = outputValue->second;
                            }
                        }
                    }
                }
            }
        }
    }

    // Lambda Factory para crear nodos de manera simple
    CustomNode *CreateNode(
        const std::string &title,
        std::function<void(CustomNode *)> executeFunc,
        bool hasExecInput = false,
        bool hasExecOutput = false,
        std::vector<std::pair<std::string, std::any>> inputPins = {},
        std::vector<std::pair<std::string, std::any>> outputPins = {},
        ImVec2 position = ImVec2(100, 100),
        ImVec2 size = ImVec2(220, 100))
    {
        NodeConfig config(title, size);

        // Agregar pin de ejecución de entrada si se necesita
        if (hasExecInput)
        {
            config.inputPins.push_back(PinConfig("Exec", ExecuteInput));
        }

        // Agregar pins de entrada de datos
        for (const auto &pin : inputPins)
        {
            config.inputPins.push_back(PinConfig(pin.first, Input, pin.second));
        }

        // Agregar pin de ejecución de salida si se necesita
        if (hasExecOutput)
        {
            config.outputPins.push_back(PinConfig("Exec", ExecuteOutput));
        }

        // Agregar pins de salida de datos
        for (const auto &pin : outputPins)
        {
            config.outputPins.push_back(PinConfig(pin.first, Output, pin.second));
        }

        // Asignar función de ejecución
        config.executeFunction = executeFunc;

        return CreateCustomNode(config, position);
    }

    // Funciones de conveniencia usando el lambda factory
    CustomNode *CreatePrintNode(ImVec2 position = ImVec2(200, 100))
    {
        return CreateNode(
            "Print Console",
            [](CustomNode *node)
            {
                std::string message = node->GetInputValue<std::string>(1, "Hello World");
                std::cout << "[CONSOLE] " << message << std::endl;
            },
            true,                                      // hasExecInput
            true,                                      // hasExecOutput
            {{"Message", std::string("Hello World")}}, // inputPins
            {},                                        // outputPins
            position);
    }

    CustomNode *CreateStringNode(const std::string &value = "My String", ImVec2 position = ImVec2(50, 200))
    {
        return CreateNode(
            "String",
            [value](CustomNode *node)
            {
                node->SetOutputValue<std::string>(0, value);
                std::cout << "[STRING] Output: " << value << std::endl;
            },
            false,             // hasExecInput
            false,             // hasExecOutput
            {},                // inputPins
            {{"Text", value}}, // outputPins
            position);
    }

    CustomNode *CreateMathAddNode(ImVec2 position = ImVec2(200, 200))
    {
        return CreateNode(
            "Add",
            [](CustomNode *node)
            {
                float a = node->GetInputValue<float>(0, 0.0f);
                float b = node->GetInputValue<float>(1, 0.0f);
                float result = a + b;
                node->SetOutputValue<float>(0, result);
                std::cout << "[MATH] " << a << " + " << b << " = " << result << std::endl;
            },
            false,                      // hasExecInput
            false,                      // hasExecOutput
            {{"A", 0.0f}, {"B", 0.0f}}, // inputPins
            {{"Result", 0.0f}},         // outputPins
            position);
    }

    // Nodo Start - Punto de inicio de ejecución
    CustomNode *CreateStartNode(ImVec2 position = ImVec2(50, 100))
    {
        return CreateNode(
            "Start",
            [](CustomNode *node)
            {
                std::cout << "[START] ========== Execution Started ==========" << std::endl;
                std::cout << "[START] Initializing system..." << std::endl;

                // Marcar que el sistema ha iniciado
                node->SetOutputValue<bool>(0, true);
                node->nodeData["started"] = true;
                node->nodeData["startTime"] = std::chrono::steady_clock::now();
            },
            false,               // hasExecInput
            true,                // hasExecOutput
            {},                  // inputPins
            {{"Started", true}}, // outputPins - indica que el sistema ha iniciado
            position);
    }

    // Nodo Update - Loop de actualización continua
    CustomNode *CreateUpdateNode(ImVec2 position = ImVec2(250, 100))
    {
        return CreateNode(
            "Update",
            [](CustomNode *node)
            {
                // Obtener deltaTime si está conectado, sino usar valor por defecto
                float deltaTime = node->GetInputValue<float>(1, 0.016f); // ~60 FPS por defecto

                // Incrementar frame counter
                int frameCount = node->GetInputValue<int>(2, 0) + 1;
                node->SetInputValue<int>(2, frameCount);
                node->SetOutputValue<int>(0, frameCount);

                // Calcular tiempo total transcurrido
                static auto startTime = std::chrono::steady_clock::now();
                auto currentTime = std::chrono::steady_clock::now();
                auto elapsed = std::chrono::duration<float>(currentTime - startTime).count();

                node->SetOutputValue<float>(1, elapsed);
                node->SetOutputValue<float>(2, deltaTime);

                std::cout << "[UPDATE] Frame: " << frameCount
                          << " | DeltaTime: " << deltaTime
                          << "s | Elapsed: " << elapsed << "s" << std::endl;
            },
            true,                                                         // hasExecInput
            true,                                                         // hasExecOutput
            {{"DeltaTime", 0.016f}, {"FrameCount", 0}},                   // inputPins
            {{"Frame", 0}, {"ElapsedTime", 0.0f}, {"DeltaTime", 0.016f}}, // outputPins
            position);
    }

    // Nodo Timer - Emite pulsos basados en tiempo
    CustomNode *CreateTimerNode(float interval = 1.0f, ImVec2 position = ImVec2(50, 250))
    {
        return CreateNode(
            "Timer",
            [interval](CustomNode *node)
            {
                static auto lastTime = std::chrono::steady_clock::now();
                auto currentTime = std::chrono::steady_clock::now();
                float customInterval = node->GetInputValue<float>(0, interval);

                auto elapsed = std::chrono::duration<float>(currentTime - lastTime).count();

                if (elapsed >= customInterval)
                {
                    lastTime = currentTime;
                    node->SetOutputValue<bool>(0, true);
                    std::cout << "[TIMER] Pulse! Interval: " << customInterval << "s" << std::endl;
                }
                else
                {
                    node->SetOutputValue<bool>(0, false);
                }

                node->SetOutputValue<float>(1, elapsed);
            },
            true,                                      // hasExecInput
            true,                                      // hasExecOutput
            {{"Interval", interval}},                  // inputPins
            {{"Pulse", false}, {"ElapsedTime", 0.0f}}, // outputPins
            position);
    }

    // Nodo Delay - Retrasa la ejecución
    CustomNode *CreateDelayNode(float delayTime = 1.0f, ImVec2 position = ImVec2(350, 250))
    {
        return CreateNode(
            "Delay",
            [delayTime](CustomNode *node)
            {
                float customDelay = node->GetInputValue<float>(0, delayTime);

                // Inicializar tiempo si es la primera vez
                if (node->nodeData.find("startTime") == node->nodeData.end())
                {
                    node->nodeData["startTime"] = std::chrono::steady_clock::now();
                    std::cout << "[DELAY] Starting delay of " << customDelay << " seconds..." << std::endl;
                }

                auto startTime = std::any_cast<std::chrono::steady_clock::time_point>(node->nodeData["startTime"]);
                auto currentTime = std::chrono::steady_clock::now();
                float elapsed = std::chrono::duration<float>(currentTime - startTime).count();

                node->SetOutputValue<float>(0, elapsed);
                node->SetOutputValue<float>(1, customDelay - elapsed);

                if (elapsed >= customDelay)
                {
                    node->SetOutputValue<bool>(2, true);
                    std::cout << "[DELAY] Delay completed! (" << elapsed << "s)" << std::endl;
                    // Reset para próxima ejecución
                    node->nodeData.erase("startTime");
                }
                else
                {
                    node->SetOutputValue<bool>(2, false);
                }
            },
            true,                                                                        // hasExecInput
            true,                                                                        // hasExecOutput
            {{"DelayTime", delayTime}},                                                  // inputPins
            {{"ElapsedTime", 0.0f}, {"RemainingTime", delayTime}, {"Completed", false}}, // outputPins
            position);
    }

    // Nodos adicionales con diferentes tipos de datos
    CustomNode *CreateBoolNode(bool initialValue = false, ImVec2 position = ImVec2(50, 300))
    {
        return CreateNode(
            "Boolean",
            [initialValue](CustomNode *node)
            {
                bool value = node->GetInputValue<bool>(0, initialValue);
                node->SetOutputValue<bool>(0, value);
                std::cout << "[BOOL] Value: " << (value ? "true" : "false") << std::endl;
            },
            false,                      // hasExecInput
            false,                      // hasExecOutput
            {{"Value", initialValue}},  // inputPins
            {{"Result", initialValue}}, // outputPins
            position);
    }

    CustomNode *CreateIntNode(int initialValue = 42, ImVec2 position = ImVec2(50, 350))
    {
        return CreateNode(
            "Integer",
            [initialValue](CustomNode *node)
            {
                int value = node->GetInputValue<int>(0, initialValue);
                node->SetOutputValue<int>(0, value);
                std::cout << "[INT] Value: " << value << std::endl;
            },
            false,                      // hasExecInput
            false,                      // hasExecOutput
            {{"Value", initialValue}},  // inputPins
            {{"Result", initialValue}}, // outputPins
            position);
    }

    CustomNode *CreateFloatNode(float initialValue = 3.14f, ImVec2 position = ImVec2(50, 400))
    {
        return CreateNode(
            "Float",
            [initialValue](CustomNode *node)
            {
                float value = node->GetInputValue<float>(0, initialValue);
                node->SetOutputValue<float>(0, value);
                std::cout << "[FLOAT] Value: " << value << std::endl;
            },
            false,                      // hasExecInput
            false,                      // hasExecOutput
            {{"Value", initialValue}},  // inputPins
            {{"Result", initialValue}}, // outputPins
            position);
    }

    // Método para verificar si un pin tiene conexiones
    bool HasConnection(int nodeId, int pinId, bool isInput)
    {
        for (auto &c : connections)
        {
            if (isInput)
            {
                if (c.toNodeId == nodeId && c.toPinId == pinId)
                    return true;
            }
            else
            {
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
            if (isInput)
            {
                // Para pins de entrada, eliminar conexiones que llegan a este pin
                shouldRemove = (it->toNodeId == nodeId && it->toPinId == pinId);
            }
            else
            {
                // Para pins de salida, eliminar conexiones que salen de este pin
                shouldRemove = (it->fromNodeId == nodeId && it->fromPinId == pinId);
            }

            if (shouldRemove)
            {
                std::cout << "Removing connection: " << it->fromNodeId << ":" << it->fromPinId
                          << " -> " << it->toNodeId << ":" << it->toPinId << std::endl;
                it = connections.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    // Método para validar si una conexión es válida
    bool IsValidConnection(Node *fromNode, int fromPinId, Node *toNode, int toPinId)
    {
        std::cout << "Validating connection: Node " << fromNode->id << " pin " << fromPinId
                  << " -> Node " << toNode->id << " pin " << toPinId << std::endl;

        if (!fromNode || !toNode)
        {
            std::cout << "  Invalid: null node" << std::endl;
            return false;
        }

        // Verificar que los índices sean válidos
        if (fromPinId >= fromNode->outputs.size() || toPinId >= toNode->inputs.size())
        {
            std::cout << "  Invalid: pin index out of bounds. From outputs: " << fromNode->outputs.size()
                      << ", To inputs: " << toNode->inputs.size() << std::endl;
            return false;
        }

        Pin &fromPin = fromNode->outputs[fromPinId];
        Pin &toPin = toNode->inputs[toPinId];

        // Solo se pueden conectar pins del mismo tipo
        // Ejecución con ejecución, datos con datos
        if (fromPin.isExec != toPin.isExec)
        {
            std::cout << "  Invalid: pin type mismatch. From isExec: " << fromPin.isExec
                      << ", To isExec: " << toPin.isExec << std::endl;
            return false;
        }

        // No permitir auto-conexiones
        if (fromNode->id == toNode->id)
        {
            std::cout << "  Invalid: self-connection" << std::endl;
            return false;
        }

        // Verificar si ya existe una conexión en el pin de entrada
        for (auto &c : connections)
        {
            if (c.toNodeId == toNode->id && c.toPinId == toPinId)
            {
                std::cout << "  Invalid: pin already has connection" << std::endl;
                return false; // Ya hay una conexión en este pin de entrada
            }
        }

        std::cout << "  Valid connection!" << std::endl;
        return true;
    }

    // Método para obtener el valor de un nodo conectado
    std::string GetConnectedValue(int nodeId, int pinId)
    {
        for (auto &c : connections)
        {
            if (c.toNodeId == nodeId && c.toPinId == pinId)
            {
                CustomNode *sourceNode = GetNodeById(c.fromNodeId);
                if (sourceNode)
                    return sourceNode->n.data;
            }
        }
        return "";
    }

    void ExecuteFrom(CustomNode *d)
    {
        Node *node = &d->n;
        if (!node || !node->execFunc)
            return;

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
        for (auto &c : connections)
        {
            if (c.fromNodeId == node->id)
            {
                CustomNode *next = GetNodeById(c.toNodeId);
                if (next && c.fromPinId < node->outputs.size() && node->outputs[c.fromPinId].isExec)
                    ExecuteFrom(next);
            }
        }
    }

    // Método para ejecutar desde un nodo Start
    void ExecuteGraph()
    {
        for (auto &node : customNodes)
        {
            if (node.n.title == "Start")
            {
                ExecuteFrom(&node);
                break;
            }
        }
    }

    void Draw()
    {
        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        ImVec2 window_pos = ImGui::GetWindowPos();
        ImVec2 mousePos = ImGui::GetIO().MousePos - window_pos;

        // Dibujar conexiones
        for (auto &c : connections)
        {
            Node *fromNode = &GetNodeById(c.fromNodeId)->n;
            Node *toNode = &GetNodeById(c.toNodeId)->n;
            if (!fromNode || !toNode)
                continue;

            if (c.fromPinId >= fromNode->outputs.size() || c.toPinId >= toNode->inputs.size())
                continue;

            ImVec2 p1 = window_pos + fromNode->pos + fromNode->outputs[c.fromPinId].pos;
            ImVec2 p2 = window_pos + toNode->pos + toNode->inputs[c.toPinId].pos;

            // Línea más gruesa y con mejor apariencia
            draw_list->AddBezierCubic(
                p1, p1 + ImVec2(50, 0),
                p2 - ImVec2(50, 0), p2,
                fromNode->outputs[c.fromPinId].isExec ? IM_COL32(255, 80, 80, 255) : IM_COL32(255, 200, 80, 255),
                3.5f);
        }

        // Dibujar nodos
        for (auto &d : customNodes)
        {
            auto &n = d.n;

            ImGui::PushID(n.id);
            ImVec2 min = window_pos + n.pos;
            ImVec2 max = window_pos + n.pos + n.size;

            // Colores mejorados y más modernos
            ImU32 nodeColor, headerColor, borderColor;
            nodeColor = IM_COL32(45, 45, 45, 255);
            headerColor = IM_COL32(60, 60, 60, 255);
            borderColor = IM_COL32(80, 80, 80, 255);

            // Fondo del nodo con bordes menos redondeados
            draw_list->AddRectFilled(min, max, nodeColor, 3.0f);

            // Header del nodo
            ImVec2 headerMax = ImVec2(max.x, min.y + 22);
            draw_list->AddRectFilled(min, headerMax, headerColor, 3.0f, ImDrawFlags_RoundCornersTop);

            // Borde del nodo
            draw_list->AddRect(min, max, borderColor, 3.0f, 0, 1.5f);

            // Título del nodo
            ImVec2 text_size = ImGui::CalcTextSize(n.title.c_str());
            draw_list->AddText(min + ImVec2((n.size.x - text_size.x) * 0.5f, 3), IM_COL32(255, 255, 255, 255), n.title.c_str());

            if (CustomNode *cn = GetCustomNodeById(n.id))
            {
                // Dibujar campos editables para pins de entrada que no son de ejecución
                float currentY = 45.0f; // Movido más abajo desde 30.0f
                for (size_t pinIndex = 0; pinIndex < n.inputs.size(); pinIndex++)
                {
                    const Pin &pin = n.inputs[pinIndex];

                    // Mostrar etiqueta del pin (tanto para ejecución como datos)
                    auto nameIt = cn->inputNames.find((int)pinIndex);
                    std::string pinName = (nameIt != cn->inputNames.end()) ? nameIt->second : ("Pin " + std::to_string(pinIndex));

                    // Mostrar etiqueta a la izquierda del pin
                    ImGui::SetCursorScreenPos(ImVec2(min.x + 15, min.y + currentY - 5));
                    ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), pinName.c_str());

                    // Solo mostrar campos de edición para pins de datos (no de ejecución)
                    if (!pin.isExec)
                    {
                        ImVec2 contentMin = ImVec2(min.x + 80, min.y + currentY - 10);
                        ImVec2 contentMax = ImVec2(max.x - 15, min.y + currentY + 10);

                        // Verificar si hay conexiones en este pin
                        bool hasConnection = HasConnection(n.id, (int)pinIndex, true);

                        // Solo mostrar input editable si NO hay conexión
                        if (!hasConnection)
                        {
                            ImGui::SetCursorScreenPos(contentMin);
                            ImGui::PushItemWidth(contentMax.x - contentMin.x);

                            std::string inputId = "##" + std::to_string(n.id) + "_input_" + std::to_string(pinIndex);

                            // Obtener el valor actual del input
                            auto inputIt = cn->inputValues.find((int)pinIndex);
                            if (inputIt != cn->inputValues.end())
                            {
                                try
                                {
                                    // Intentar diferentes tipos
                                    if (inputIt->second.type() == typeid(std::string))
                                    {
                                        std::string currentValue = std::any_cast<std::string>(inputIt->second);
                                        char buf[256];
                                        strncpy(buf, currentValue.c_str(), sizeof(buf) - 1);
                                        buf[sizeof(buf) - 1] = '\0';

                                        if (ImGui::InputText(inputId.c_str(), buf, sizeof(buf)))
                                        {
                                            cn->SetInputValue<std::string>((int)pinIndex, std::string(buf));
                                        }
                                    }
                                    else if (inputIt->second.type() == typeid(int))
                                    {
                                        int currentValue = std::any_cast<int>(inputIt->second);
                                        char buf[32];
                                        sprintf(buf, "%d", currentValue);
                                        if (ImGui::InputText(inputId.c_str(), buf, sizeof(buf), ImGuiInputTextFlags_CharsDecimal))
                                        {
                                            int newValue = atoi(buf);
                                            if (newValue != currentValue)
                                            {
                                                cn->SetInputValue<int>((int)pinIndex, newValue);
                                            }
                                        }
                                    }
                                    else if (inputIt->second.type() == typeid(float))
                                    {
                                        float currentValue = std::any_cast<float>(inputIt->second);
                                        char buf[32];
                                        sprintf(buf, "%.3f", currentValue);
                                        if (ImGui::InputText(inputId.c_str(), buf, sizeof(buf), ImGuiInputTextFlags_CharsDecimal))
                                        {
                                            float newValue = (float)atof(buf);
                                            if (fabs(newValue - currentValue) > 0.001f)
                                            {
                                                cn->SetInputValue<float>((int)pinIndex, newValue);
                                            }
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
                                    else
                                    {
                                        // Tipo desconocido, mostrar solo texto
                                        ImGui::Text("Pin %zu", pinIndex);
                                    }
                                }
                                catch (const std::bad_any_cast &e)
                                {
                                    ImGui::Text("Error: Pin %zu", pinIndex);
                                }
                            }
                            else
                            {
                                // No hay valor, mostrar campo de texto por defecto
                                char buf[256] = "";
                                if (ImGui::InputText(inputId.c_str(), buf, sizeof(buf)))
                                {
                                    cn->SetInputValue<std::string>((int)pinIndex, std::string(buf));
                                }
                            }

                            ImGui::PopItemWidth();
                        }
                        else
                        {
                            // Si hay conexión, mostrar indicador
                            ImGui::SetCursorScreenPos(ImVec2(min.x + 80, min.y + currentY - 5));
                            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Connected");
                        }
                    }

                    currentY += 30.0f;
                }

                // Para backwards compatibility con el sistema anterior de Pins
                for (int i = 0; i < cn->Pins.size(); i++)
                {
                    PinInfo *p = cn->Pins[i];
                    if (i < n.inputs.size())
                    {
                        float pinY = n.inputs[i].pos.y;
                        ImVec2 contentMin = ImVec2(min.x + 8, min.y + pinY - 8);
                        ImVec2 contentMax = ImVec2(max.x - 15, min.y + pinY + 8);

                        ImGui::SetCursorScreenPos(contentMin);
                        ImGui::PushItemWidth(contentMax.x - contentMin.x);

                        // Detectar tipo del pin
                        if (p->_Var.type() == typeid(std::string))
                        {
                            std::string &value = *std::any_cast<std::reference_wrapper<std::string>>(&p->_Var);
                            char buf[128];
                            strncpy(buf, value.c_str(), sizeof(buf) - 1);
                            buf[sizeof(buf) - 1] = '\0';
                            if (ImGui::InputText(("##legacy_" + std::to_string(n.id) + "_" + p->_Name).c_str(), buf, sizeof(buf)))
                            {
                                value = buf;
                            }
                        }
                        else if (p->_Var.type() == typeid(int))
                        {
                            int &value = *std::any_cast<std::reference_wrapper<int>>(&p->_Var);
                            ImGui::InputInt(("##legacy_" + std::to_string(n.id) + "_" + p->_Name).c_str(), &value);
                        }
                        else if (p->_Var.type() == typeid(float))
                        {
                            float &value = *std::any_cast<std::reference_wrapper<float>>(&p->_Var);
                            ImGui::InputFloat(("##legacy_" + std::to_string(n.id) + "_" + p->_Name).c_str(), &value, 0.1f, 1.0f, "%.3f");
                        }
                        else if (p->_Var.type() == typeid(bool))
                        {
                            bool &value = *std::any_cast<std::reference_wrapper<bool>>(&p->_Var);
                            ImGui::Checkbox(("##legacy_" + std::to_string(n.id) + "_" + p->_Name).c_str(), &value);
                        }
                        else
                        {
                            ImGui::TextUnformatted(p->_Name.c_str());
                        }

                        ImGui::PopItemWidth();
                    }
                }
            }

            // Botón invisible para arrastrar el nodo (solo en el header)
            ImGui::SetCursorScreenPos(min);
            ImGui::InvisibleButton(("node" + std::to_string(n.id)).c_str(), ImVec2(n.size.x, 22));
            // Solo permitir arrastrar el nodo si no estamos en modo conexión y el click empezó en el header
            if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left) && editingNodeId != n.id && !isConnecting)
            {
                n.pos += ImGui::GetIO().MouseDelta;
            }

            // PINS DE ENTRADA
            for (size_t i = 0; i < n.inputs.size(); i++)
            {
                Pin &pin = n.inputs[i];
                ImVec2 pinPos = window_pos + n.pos + pin.pos;

                // Verificar si el pin tiene conexiones
                bool hasConnection = HasConnection(n.id, (int)i, true);
                ImU32 pinColor = pin.isExec ? IM_COL32(255, 80, 80, 255) : IM_COL32(255, 200, 80, 255);

                if (hasConnection)
                {
                    // Si tiene conexión, dibujar círculo completo
                    draw_list->AddCircleFilled(pinPos, 6, pinColor);
                    draw_list->AddCircle(pinPos, 6, IM_COL32(255, 255, 255, 100), 0, 1.0f);
                }
                else
                {
                    // Si no tiene conexión, dibujar como wire
                    ImVec2 wireStart = pinPos;
                    ImVec2 wireEnd = pinPos + ImVec2(10, 0);
                    draw_list->AddCircleFilled(pinPos, 5, IM_COL32(60, 60, 60, 255));
                    draw_list->AddCircle(pinPos, 6, pinColor);
                }

                ImGui::SetCursorScreenPos(pinPos - ImVec2(12, 12));
                ImGui::InvisibleButton(("inpin" + std::to_string(n.id) + "_" + std::to_string(pin.id)).c_str(), ImVec2(24, 24));

                // Debug: mostrar cuando se hace hover en un pin de entrada
                if (ImGui::IsItemHovered() && isConnecting)
                {
                    std::cout << "Hovering over input pin " << n.id << ":" << i << std::endl;
                }

                // Detectar doble click para eliminar conexiones
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    std::cout << "Double clicked on input pin " << n.id << ":" << i << std::endl;
                    RemoveConnectionsFromPin(n.id, (int)i, true);
                }
                // Conectar al soltar el mouse cuando estamos arrastrando una conexión
                else if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Left) && isConnecting && connectingFromNode != -1)
                {
                    std::cout << "Attempting to connect to input pin " << n.id << ":" << i << std::endl;
                    Node *fromNode = &GetNodeById(connectingFromNode)->n;
                    if (fromNode)
                    {
                        std::cout << "From node found: " << fromNode->id << std::endl;
                        if (IsValidConnection(fromNode, connectingFromPin, &n, (int)i))
                        {
                            connections.push_back({connectingFromNode, connectingFromPin, n.id, (int)i});
                            std::cout << "Connection created: " << connectingFromNode << ":" << connectingFromPin
                                      << " -> " << n.id << ":" << i << std::endl;
                        }
                        else
                        {
                            std::cout << "Invalid connection attempted" << std::endl;
                        }
                    }
                    else
                    {
                        std::cout << "From node not found!" << std::endl;
                    }
                    connectingFromNode = -1;
                    connectingFromPin = -1;
                    isConnecting = false;
                }
            }

            // PINS DE SALIDA
            for (size_t i = 0; i < n.outputs.size(); i++)
            {
                Pin &pin = n.outputs[i];
                ImVec2 pinPos = window_pos + n.pos + pin.pos;

                // Mostrar etiqueta del pin de salida
                if (CustomNode *cn = GetCustomNodeById(n.id))
                {
                    auto nameIt = cn->outputNames.find((int)i);
                    std::string pinName = (nameIt != cn->outputNames.end()) ? nameIt->second : ("Out " + std::to_string(i));

                    // Mostrar etiqueta a la derecha del pin
                    ImVec2 text_size = ImGui::CalcTextSize(pinName.c_str());
                    ImGui::SetCursorScreenPos(ImVec2(pinPos.x - text_size.x - 15, pinPos.y - 5));
                    ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), pinName.c_str());
                }

                // Verificar si el pin tiene conexiones
                bool hasConnection = HasConnection(n.id, (int)i, false);
                ImU32 pinColor = pin.isExec ? IM_COL32(255, 80, 80, 255) : IM_COL32(255, 200, 80, 255);

                if (hasConnection)
                {
                    draw_list->AddCircleFilled(pinPos, 6, pinColor);
                    draw_list->AddCircle(pinPos, 6, IM_COL32(255, 255, 255, 100), 0, 1.0f);
                }
                else
                {
                    // Si no tiene conexión, dibujar como wire
                    ImVec2 wireStart = pinPos;
                    ImVec2 wireEnd = pinPos + ImVec2(-10, 0);
                    draw_list->AddCircleFilled(pinPos, 5, IM_COL32(60, 60, 60, 255));
                    draw_list->AddCircle(pinPos, 6, pinColor);
                }

                ImGui::SetCursorScreenPos(pinPos - ImVec2(12, 12));
                ImGui::InvisibleButton(("outpin" + std::to_string(n.id) + "_" + std::to_string(pin.id)).c_str(), ImVec2(24, 24));

                // Detectar doble click para eliminar conexiones
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    std::cout << "Double clicked on output pin " << n.id << ":" << i << std::endl;
                    RemoveConnectionsFromPin(n.id, (int)i, false);
                }
                // Iniciar conexión al empezar a arrastrar
                else if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left) && !isConnecting)
                {
                    connectingFromNode = n.id;
                    connectingFromPin = (int)i;
                    isConnecting = true;
                    std::cout << "Starting connection from node " << n.id << " pin " << i << std::endl;
                }
            }
            ImGui::PopID();
        }

        // Dibujar línea de conexión temporal y manejar cancelación
        if (connectingFromNode != -1 && isConnecting)
        {
            Node *fromNode = &GetNodeById(connectingFromNode)->n;
            if (fromNode && connectingFromPin < fromNode->outputs.size())
            {
                ImVec2 p1 = window_pos + fromNode->pos + fromNode->outputs[connectingFromPin].pos;
                ImVec2 p2 = ImGui::GetIO().MousePos;

                ImU32 lineColor = fromNode->outputs[connectingFromPin].isExec ? IM_COL32(255, 80, 80, 150) : IM_COL32(255, 200, 80, 150);

                draw_list->AddBezierCubic(p1, p1 + ImVec2(50, 0), p2 - ImVec2(50, 0), p2, lineColor, 3.0f);
            }
        }

        // Manejar cancelación DESPUÉS del procesamiento de pins para evitar conflictos
        // Cancelar conexión si se hace click derecho o se presiona ESC
        if (isConnecting && (ImGui::IsMouseClicked(ImGuiMouseButton_Right) || ImGui::IsKeyPressed(ImGuiKey_Escape)))
        {
            std::cout << "Connection cancelled" << std::endl;
            connectingFromNode = -1;
            connectingFromPin = -1;
            isConnecting = false;
        }

        // También cancelar si se suelta el mouse en el vacío (pero solo si no se procesó una conexión exitosa)
        else if (isConnecting && ImGui::IsMouseReleased(ImGuiMouseButton_Left) && !ImGui::IsAnyItemHovered())
        {
            std::cout << "Connection cancelled - released on empty space (IsAnyItemHovered: " << ImGui::IsAnyItemHovered() << ")" << std::endl;
            connectingFromNode = -1;
            connectingFromPin = -1;
            isConnecting = false;
        }

        // Botones de control
        ImGui::SetCursorScreenPos(window_pos + ImVec2(10, 10));
        if (ImGui::Button("Execute Graph"))
        {
            ExecuteGraph();
        }

        // Instrucciones
        ImGui::SetCursorScreenPos(window_pos + ImVec2(10, 40));
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Drag from output pins to input pins | Right-click/ESC to cancel | Double-click pins to disconnect");
    }
};

// Implementación de CustomNode::Draw() DESPUÉS de SimpleNodeEditor
// inline void CustomNode::Draw()
// {
//     if (!parentEditor)
//         return; // Verificar que parentEditor no sea nullptr

//     ImGui::PushID(n.id);
//     ImVec2 window_pos = ImGui::GetWindowPos();
//     ImVec2 min = window_pos + n.pos;
//     ImVec2 max = window_pos + n.pos + n.size;

//     ImDrawList *draw_list = ImGui::GetWindowDrawList();

//     ImU32 nodeColor, headerColor, borderColor;

//     nodeColor = IM_COL32(45, 45, 45, 255);
//     headerColor = IM_COL32(60, 60, 60, 255);
//     borderColor = IM_COL32(80, 80, 80, 255);

//     // Fondo del nodo con bordes menos redondeados
//     draw_list->AddRectFilled(min, max, nodeColor, 3.0f);

//     // Header del nodo
//     ImVec2 headerMax = ImVec2(max.x, min.y + 22);
//     draw_list->AddRectFilled(min, headerMax, headerColor, 3.0f, ImDrawFlags_RoundCornersTop);

//     // Borde del nodo
//     draw_list->AddRect(min, max, borderColor, 3.0f, 0, 1.5f);

//     // Título del nodo
//     ImVec2 text_size = ImGui::CalcTextSize(n.title.c_str());
//     draw_list->AddText(min + ImVec2((n.size.x - text_size.x) * 0.5f, 3), IM_COL32(255, 255, 255, 255), n.title.c_str());

//     for (int i = 0; i < Pins.size(); i++)
//     {
//         PinInfo *p = Pins[i];
//         float pinY = n.outputs[0].pos.y;
//         ImVec2 contentMin = ImVec2(min.x + 8, min.y + pinY - 8);
//         ImVec2 contentMax = ImVec2(max.x - 15, min.y + pinY + 8);

//         ImGui::SetCursorScreenPos(contentMin);
//         ImGui::PushItemWidth(contentMax.x - contentMin.x);

//         if (p->_Var.type() == typeid(std::string))
//         {
//             std::string &value = *std::any_cast<std::reference_wrapper<std::string>>(&p->_Var);
//             char buf[128];
//             strncpy(buf, value.c_str(), sizeof(buf));
//             if (ImGui::InputText(("##" + std::to_string(n.id) + "_" + p->_Name).c_str(), buf, sizeof(buf)))
//             {
//                 value = buf;
//             }
//         }
//         else if (p->_Var.type() == typeid(int))
//         {
//             int &value = *std::any_cast<std::reference_wrapper<int>>(&p->_Var);
//             ImGui::InputInt(("##" + std::to_string(n.id) + "_" + p->_Name).c_str(), &value);
//         }
//         else if (p->_Var.type() == typeid(float))
//         {
//             float &value = *std::any_cast<std::reference_wrapper<float>>(&p->_Var);
//             ImGui::InputFloat(("##" + std::to_string(n.id) + "_" + p->_Name).c_str(), &value, 0.1f, 1.0f, "%.3f");
//         }
//         else
//         {
//             ImGui::TextUnformatted(p->_Name.c_str());
//         }

//         ImGui::PopItemWidth();
//     }

//     // Botón invisible para arrastrar el nodo (solo en el header)
//     ImGui::SetCursorScreenPos(min);
//     ImGui::InvisibleButton(("node" + std::to_string(n.id)).c_str(), ImVec2(n.size.x, 22));
//     if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left) && parentEditor->editingNodeId != n.id)
//         n.pos += ImGui::GetIO().MouseDelta;

//     // PINS DE ENTRADA
//     for (size_t i = 0; i < n.inputs.size(); i++)
//     {
//         Pin &pin = n.inputs[i];
//         ImVec2 pinPos = window_pos + n.pos + pin.pos;

//         // Verificar si el pin tiene conexiones
//         bool hasConnection = parentEditor->HasConnection(n.id, (int)i, true);
//         ImU32 pinColor = pin.isExec ? IM_COL32(255, 80, 80, 255) : IM_COL32(255, 200, 80, 255);

//         if (hasConnection)
//         {
//             // Si tiene conexión, dibujar círculo completo
//             draw_list->AddCircleFilled(pinPos, 6, pinColor);
//             draw_list->AddCircle(pinPos, 6, IM_COL32(255, 255, 255, 100), 0, 1.0f);
//         }
//         else
//         {
//             // Si no tiene conexión, dibujar como wire
//             ImVec2 wireStart = pinPos;
//             ImVec2 wireEnd = pinPos + ImVec2(10, 0);
//             draw_list->AddCircleFilled(pinPos, 5, IM_COL32(60, 60, 60, 255));
//             draw_list->AddCircle(pinPos, 6, pinColor);
//         }

//         ImGui::SetCursorScreenPos(pinPos - ImVec2(8, 8));
//         ImGui::InvisibleButton(("inpin" + std::to_string(n.id) + "_" + std::to_string(pin.id)).c_str(), ImVec2(16, 16));

//         // Detectar doble click para eliminar conexiones
//         if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
//         {
//             std::cout << "Double clicked on input pin " << n.id << ":" << i << std::endl;
//             parentEditor->RemoveConnectionsFromPin(n.id, (int)i, true);
//         }
//         // Conectar al soltar el mouse
//         else if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Left) && parentEditor->connectingFromNode != -1)
//         {
//             Node *fromNode = &parentEditor->GetNodeById(parentEditor->connectingFromNode)->n;
//             if (fromNode && parentEditor->IsValidConnection(fromNode, parentEditor->connectingFromPin, &n, (int)i))
//             {
//                 parentEditor->connections.push_back({parentEditor->connectingFromNode, parentEditor->connectingFromPin, n.id, (int)i});
//             }
//             parentEditor->connectingFromNode = -1;
//         }
//     }
//     ImGui::PopID();
// }
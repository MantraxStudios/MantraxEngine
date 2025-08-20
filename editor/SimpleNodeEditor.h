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

enum NodeCategory
{
    INPUT_OUTPUT,
    MATH,
    CONVERTER,
    VECTOR,
    SHADER,
    TEXTURE,
    COLOR,
    SCRIPT,
    GROUP,
    LAYOUT
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
    NodeCategory category = INPUT_OUTPUT;

    // Para nodos de ejecución
    std::string data;                     // para String node
    std::function<void(Node *)> execFunc; // para nodos de ejecución
    bool isEditing = false;               // Para controlar si está en modo edición
    bool isSelected = false;              // Para indicar si está seleccionado
    bool isActive = false;                // Para indicar si está ejecutándose
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
    NodeCategory category;
    std::vector<PinConfig> inputPins;
    std::vector<PinConfig> outputPins;
    std::function<void(CustomNode *)> executeFunction;

    NodeConfig(const std::string &t, NodeCategory cat = INPUT_OUTPUT, ImVec2 s = ImVec2(220, 100))
        : title(t), category(cat), size(s) {}
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

        // Pin de ejecución de entrada (rojo) - posición ajustada fuera del nodo
        n.inputs.push_back({0, ImVec2(-15, 45), true});
        // Pin de datos de entrada para el texto (amarillo) - posición ajustada fuera del nodo
        n.inputs.push_back({1, ImVec2(-15, 70), false});
        // Pin de ejecución de salida (rojo) - posición ajustada fuera del nodo
        n.outputs.push_back({0, ImVec2(155, 45), true});

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

    // Colores estilo Blender
    struct BlenderColors
    {
        static ImU32 GetCategoryColor(NodeCategory category)
        {
            switch (category)
            {
            case INPUT_OUTPUT:
                return IM_COL32(35, 35, 35, 255); // Naranja rojizo (Header 1)
            case MATH:
                return IM_COL32(95, 145, 185, 255); // Azul
            case CONVERTER:
                return IM_COL32(185, 165, 95, 255); // Amarillo
            case VECTOR:
                return IM_COL32(125, 185, 95, 255); // Verde
            case SHADER:
                return IM_COL32(95, 185, 145, 255); // Verde azulado
            case TEXTURE:
                return IM_COL32(185, 95, 145, 255); // Rosa
            case COLOR:
                return IM_COL32(145, 95, 185, 255); // Púrpura
            case SCRIPT:
                return IM_COL32(185, 125, 95, 255); // Naranja
            case GROUP:
                return IM_COL32(95, 125, 185, 255); // Azul oscuro
            case LAYOUT:
                return IM_COL32(125, 125, 125, 255); // Gris
            default:
                return IM_COL32(125, 125, 125, 255); // Gris por defecto
            }
        }

        static ImU32 GetCategoryColorDark(NodeCategory category)
        {
            ImU32 color = GetCategoryColor(category);
            // Hacer más oscuro reduciendo cada componente
            return IM_COL32(
                (color >> IM_COL32_R_SHIFT & 0xFF) * 0.7f,
                (color >> IM_COL32_G_SHIFT & 0xFF) * 0.7f,
                (color >> IM_COL32_B_SHIFT & 0xFF) * 0.7f,
                255);
        }

        static constexpr ImU32 NodeBackground = IM_COL32(62, 62, 66, 240);
        static constexpr ImU32 NodeBackgroundSelected = IM_COL32(90, 90, 94, 255);
        static constexpr ImU32 NodeBorder = IM_COL32(80, 80, 84, 255);
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
        newNode.n.category = config.category;

        // Limpiar pins existentes
        newNode.n.inputs.clear();
        newNode.n.outputs.clear();

        // Configurar pins de entrada - posicionados a la izquierda del nodo
        float inputY = 35.0f; // Ajustado para el nuevo diseño
        for (size_t i = 0; i < config.inputPins.size(); i++)
        {
            const auto &pinConfig = config.inputPins[i];
            Pin inputPin;
            inputPin.id = (int)i;
            inputPin.pos = ImVec2(-8, inputY); // Más cerca del borde
            inputPin.isExec = (pinConfig.type == ExecuteInput);
            newNode.n.inputs.push_back(inputPin);

            // Guardar el nombre del pin
            newNode.inputNames[i] = pinConfig.name;

            // Establecer valor por defecto si es un pin de datos
            if (pinConfig.type == Input)
            {
                newNode.inputValues[i] = pinConfig.defaultValue;
            }

            inputY += 28.0f; // Espaciado más compacto
        }

        // Configurar pins de salida - posicionados a la derecha del nodo
        float outputY = 35.0f;
        for (size_t i = 0; i < config.outputPins.size(); i++)
        {
            const auto &pinConfig = config.outputPins[i];
            Pin outputPin;
            outputPin.id = (int)i;
            outputPin.pos = ImVec2(config.size.x + 8, outputY); // Más cerca del borde
            outputPin.isExec = (pinConfig.type == ExecuteOutput);
            newNode.n.outputs.push_back(outputPin);

            // Guardar el nombre del pin
            newNode.outputNames[i] = pinConfig.name;

            outputY += 28.0f; // Espaciado más compacto
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
            newNode.n.size.y = std::max(80.0f, 60.0f + maxPins * 28.0f);
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

    // Lambda Factory para crear nodos de manera simple (Nueva versión con NodeCategory)
    CustomNode *CreateNode(
        const std::string &title,
        std::function<void(CustomNode *)> executeFunc,
        NodeCategory category = INPUT_OUTPUT,
        bool hasExecInput = false,
        bool hasExecOutput = false,
        std::vector<std::pair<std::string, std::any>> inputPins = {},
        std::vector<std::pair<std::string, std::any>> outputPins = {},
        ImVec2 position = ImVec2(100, 100),
        ImVec2 size = ImVec2(180, 80))
    {
        NodeConfig config(title, category, size);

        // Agregar pin de ejecución de entrada si se necesita
        if (hasExecInput)
        {
            config.inputPins.push_back(PinConfig("", ExecuteInput));
        }

        // Agregar pins de entrada de datos
        for (const auto &pin : inputPins)
        {
            config.inputPins.push_back(PinConfig(pin.first, Input, pin.second));
        }

        // Agregar pin de ejecución de salida si se necesita
        if (hasExecOutput)
        {
            config.outputPins.push_back(PinConfig("", ExecuteOutput));
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

    // Versión de compatibilidad para código existente (sin NodeCategory)
    CustomNode *CreateNode(
        const std::string &title,
        std::function<void(CustomNode *)> executeFunc,
        bool hasExecInput,
        bool hasExecOutput,
        std::vector<std::pair<std::string, std::any>> inputPins = {},
        std::vector<std::pair<std::string, std::any>> outputPins = {},
        ImVec2 position = ImVec2(100, 100),
        ImVec2 size = ImVec2(250, 80))
    {
        // Llamar a la versión principal con categoría por defecto
        return CreateNode(title, executeFunc, INPUT_OUTPUT, hasExecInput, hasExecOutput, inputPins, outputPins, position, size);
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
            SCRIPT,                                    // category
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
            INPUT_OUTPUT,       // category
            false,              // hasExecInput
            false,              // hasExecOutput
            {},                 // inputPins
            {{"Value", value}}, // outputPins
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
            MATH,                       // category
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
            INPUT_OUTPUT,        // category
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
            SCRIPT,                                                       // category
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
            SCRIPT,                                    // category
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
            SCRIPT,                                                                      // category
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
            INPUT_OUTPUT,               // category
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
            MATH,                       // category
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
            MATH,                       // category
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

        // Marcar nodo como activo durante la ejecución
        node->isActive = true;

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

        // Desmarcar nodo como activo
        node->isActive = false;
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

        return BlenderColors::PinFloat;
    }

    void Draw()
    {
        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        ImVec2 window_pos = ImGui::GetWindowPos();
        ImVec2 mousePos = ImGui::GetIO().MousePos - window_pos;

        // Dibujar conexiones con estilo Blender
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

            // Color de la línea según el tipo de pin
            bool isExec = fromNode->outputs[c.fromPinId].isExec;
            ImU32 lineColor = isExec ? BlenderColors::PinExec : BlenderColors::ConnectionLine;

            // Línea más gruesa para conexiones de ejecución
            float thickness = isExec ? 3.0f : 2.0f;

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
            ImVec2 min = window_pos + n.pos;
            ImVec2 max = window_pos + n.pos + n.size;

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
            draw_list->AddRectFilled(min, max, nodeColor, 6.0f);

            // Header del nodo con gradiente sutil
            ImVec2 headerMax = ImVec2(max.x, min.y + 26);

            // Primero dibujamos el header con esquinas superiores redondeadas
            draw_list->AddRectFilled(min, headerMax, headerColor, 6.0f, ImDrawFlags_RoundCornersTop);

            // Luego añadimos el gradiente oscuro en la parte inferior del header
            draw_list->AddRectFilledMultiColor(
                ImVec2(min.x, min.y + 13), headerMax, // mitad inferior del header
                BlenderColors::GetCategoryColorDark(n.category),
                BlenderColors::GetCategoryColorDark(n.category),
                BlenderColors::GetCategoryColorDark(n.category),
                BlenderColors::GetCategoryColorDark(n.category));

            // Borde del nodo
            draw_list->AddRect(min, max, borderColor, 6.0f, 0, n.isSelected ? 2.0f : 1.0f);

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
                float currentY = 35.0f;
                for (size_t pinIndex = 0; pinIndex < n.inputs.size(); pinIndex++)
                {
                    const Pin &pin = n.inputs[pinIndex];

                    // Mostrar etiqueta del pin si no es de ejecución
                    if (!pin.isExec)
                    {
                        auto nameIt = cn->inputNames.find((int)pinIndex);
                        std::string pinName = (nameIt != cn->inputNames.end()) ? nameIt->second : ("Pin " + std::to_string(pinIndex));

                        // Dibujar etiqueta del pin con mejor posicionamiento
                        ImVec2 labelPos = ImVec2(min.x + 8, min.y + currentY + 2);
                        draw_list->AddText(labelPos, IM_COL32(200, 200, 200, 255), pinName.c_str());

                        // Campo de entrada solo si no hay conexión
                        bool hasConnection = HasConnection(n.id, (int)pinIndex, true);
                        if (!hasConnection)
                        {
                            // Calcular el ancho disponible para el campo de entrada
                            // Dejar espacio para la etiqueta del pin (aproximadamente 80px) + márgenes
                            float availableWidth = n.size.x - 88.0f; // 80px para etiqueta + 8px de margen derecho

                            ImVec2 inputPos = ImVec2(min.x + 80, min.y + currentY + 2);
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
                                            cn->SetInputValue<std::string>((int)pinIndex, std::string(buf));
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
                float outputY = 35.0f;
                for (size_t pinIndex = 0; pinIndex < n.outputs.size(); pinIndex++)
                {
                    const Pin &pin = n.outputs[pinIndex];

                    if (!pin.isExec)
                    {
                        auto nameIt = cn->outputNames.find((int)pinIndex);
                        std::string pinName = (nameIt != cn->outputNames.end()) ? nameIt->second : ("Out " + std::to_string(pinIndex));

                        // Etiqueta alineada a la derecha
                        ImVec2 text_size = ImGui::CalcTextSize(pinName.c_str());
                        ImVec2 labelPos = ImVec2(max.x - text_size.x - 8, min.y + outputY + 2);
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

            // Solo permitir arrastrar si no estamos en modo conexión
            if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left) && !isConnecting)
            {
                n.pos += ImGui::GetIO().MouseDelta;
            }

            // PINS DE ENTRADA con estilo Blender
            for (size_t i = 0; i < n.inputs.size(); i++)
            {
                Pin &pin = n.inputs[i];
                ImVec2 pinPos = window_pos + n.pos + pin.pos;

                bool hasConnection = HasConnection(n.id, (int)i, true);

                // Obtener color del pin
                CustomNode *cn = GetCustomNodeById(n.id);
                const std::any *value = nullptr;
                if (cn)
                {
                    auto it = cn->inputValues.find((int)i);
                    if (it != cn->inputValues.end())
                        value = &it->second;
                }

                ImU32 pinColor = GetPinColor(pin.isExec, value);

                // Dibujar pin con estilo Blender
                if (hasConnection || pin.isExec)
                {
                    // Pin conectado: círculo completo
                    draw_list->AddCircleFilled(pinPos, 5, pinColor);
                    draw_list->AddCircle(pinPos, 5, IM_COL32(0, 0, 0, 120), 0, 1.0f);
                }
                else
                {
                    // Pin desconectado: anillo
                    draw_list->AddCircle(pinPos, 5, pinColor, 0, 2.0f);
                    draw_list->AddCircleFilled(pinPos, 2, pinColor);
                }

                // Área de interacción
                ImGui::SetCursorScreenPos(pinPos - ImVec2(10, 10));
                ImGui::InvisibleButton(("inpin" + std::to_string(n.id) + "_" + std::to_string(pin.id)).c_str(), ImVec2(20, 20));

                // Highlight en hover
                if (ImGui::IsItemHovered())
                {
                    draw_list->AddCircle(pinPos, 7, IM_COL32(255, 255, 255, 100), 0, 1.0f);
                }

                // Doble click para eliminar conexiones
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    RemoveConnectionsFromPin(n.id, (int)i, true);
                }
                // Conectar al soltar
                else if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Left) && isConnecting && connectingFromNode != -1)
                {
                    Node *fromNode = &GetNodeById(connectingFromNode)->n;
                    if (fromNode && IsValidConnection(fromNode, connectingFromPin, &n, (int)i))
                    {
                        connections.push_back({connectingFromNode, connectingFromPin, n.id, (int)i});
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
                ImVec2 pinPos = window_pos + n.pos + pin.pos;

                bool hasConnection = HasConnection(n.id, (int)i, false);

                // Obtener color del pin
                CustomNode *cn = GetCustomNodeById(n.id);
                const std::any *value = nullptr;
                if (cn)
                {
                    auto it = cn->outputValues.find((int)i);
                    if (it != cn->outputValues.end())
                        value = &it->second;
                }

                ImU32 pinColor = GetPinColor(pin.isExec, value);

                // Dibujar pin
                if (hasConnection || pin.isExec)
                {
                    draw_list->AddCircleFilled(pinPos, 5, pinColor);
                    draw_list->AddCircle(pinPos, 5, IM_COL32(0, 0, 0, 120), 0, 1.0f);
                }
                else
                {
                    draw_list->AddCircle(pinPos, 5, pinColor, 0, 2.0f);
                    draw_list->AddCircleFilled(pinPos, 2, pinColor);
                }

                // Área de interacción
                ImGui::SetCursorScreenPos(pinPos - ImVec2(10, 10));
                ImGui::InvisibleButton(("outpin" + std::to_string(n.id) + "_" + std::to_string(pin.id)).c_str(), ImVec2(20, 20));

                // Highlight en hover
                if (ImGui::IsItemHovered())
                {
                    draw_list->AddCircle(pinPos, 7, IM_COL32(255, 255, 255, 100), 0, 1.0f);
                }

                // Doble click para eliminar conexiones
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    RemoveConnectionsFromPin(n.id, (int)i, false);
                }
                // Iniciar conexión
                else if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left) && !isConnecting)
                {
                    connectingFromNode = n.id;
                    connectingFromPin = (int)i;
                    isConnecting = true;
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
                ImVec2 p1 = window_pos + fromNode->pos + fromNode->outputs[connectingFromPin].pos;
                ImVec2 p2 = ImGui::GetIO().MousePos;

                bool isExec = fromNode->outputs[connectingFromPin].isExec;
                ImU32 lineColor = isExec ? IM_COL32(255, 255, 255, 180) : IM_COL32(180, 180, 180, 180);
                float thickness = isExec ? 3.0f : 2.0f;

                // Línea temporal con dash pattern simulado
                draw_list->AddBezierCubic(p1, p1 + ImVec2(50, 0), p2 - ImVec2(50, 0), p2, lineColor, thickness);

                // Punto en el cursor
                draw_list->AddCircleFilled(p2, 4, lineColor);
            }
        }

        // Cancelar conexión
        if (isConnecting && (ImGui::IsMouseClicked(ImGuiMouseButton_Right) || ImGui::IsKeyPressed(ImGuiKey_Escape)))
        {
            connectingFromNode = -1;
            connectingFromPin = -1;
            isConnecting = false;
        }
        else if (isConnecting && ImGui::IsMouseReleased(ImGuiMouseButton_Left) && !ImGui::IsAnyItemHovered())
        {
            connectingFromNode = -1;
            connectingFromPin = -1;
            isConnecting = false;
        }

        // UI de control con estilo Blender
        ImGui::SetCursorScreenPos(window_pos + ImVec2(10, 10));
        ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(70, 70, 74, 255));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(90, 90, 94, 255));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(50, 50, 54, 255));

        if (ImGui::Button("Execute Graph"))
        {
            ExecuteGraph();
        }

        ImGui::PopStyleColor(3);

        // Instrucciones con mejor tipografía
        ImGui::SetCursorScreenPos(window_pos + ImVec2(10, 40));
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(160, 160, 160, 255));
        ImGui::Text("Drag from output to input | Right-click/ESC to cancel | Double-click pins to disconnect");
        ImGui::PopStyleColor();
    }
};
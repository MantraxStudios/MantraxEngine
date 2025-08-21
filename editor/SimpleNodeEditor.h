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

static int NodeID = 0;

inline ImVec2 operator+(const ImVec2 &a, const ImVec2 &b) { return ImVec2(a.x + b.x, a.y + b.y); }
inline ImVec2 operator-(const ImVec2 &a, const ImVec2 &b) { return ImVec2(a.x - b.x, a.y - b.y); }
inline ImVec2 operator*(const ImVec2 &a, float scalar) { return ImVec2(a.x * scalar, a.y * scalar); }
inline ImVec2 operator/(const ImVec2 &a, float scalar) { return ImVec2(a.x / scalar, a.y / scalar); }
inline ImVec2 &operator+=(ImVec2 &a, const ImVec2 &b)
{
    a.x += b.x;
    a.y += b.y;
    return a;
}

// Estructuras de datos matemáticas
struct Vector2
{
    float x, y;

    Vector2(float x = 0.0f, float y = 0.0f) : x(x), y(y) {}

    Vector2 operator+(const Vector2 &other) const { return Vector2(x + other.x, y + other.y); }
    Vector2 operator-(const Vector2 &other) const { return Vector2(x - other.x, y - other.y); }
    Vector2 operator*(float scalar) const { return Vector2(x * scalar, y * scalar); }
    Vector2 operator/(float scalar) const { return Vector2(x / scalar, y / scalar); }

    float Length() const { return std::sqrt(x * x + y * y); }
    Vector2 Normalized() const
    {
        float len = Length();
        return len > 0 ? *this / len : Vector2();
    }
    float Dot(const Vector2 &other) const { return x * other.x + y * other.y; }
};

struct Vector3
{
    float x, y, z;

    Vector3(float x = 0.0f, float y = 0.0f, float z = 0.0f) : x(x), y(y), z(z) {}

    Vector3 operator+(const Vector3 &other) const { return Vector3(x + other.x, y + other.y, z + other.z); }
    Vector3 operator-(const Vector3 &other) const { return Vector3(x - other.x, y - other.y, z - other.z); }
    Vector3 operator*(float scalar) const { return Vector3(x * scalar, y * scalar, z * scalar); }
    Vector3 operator/(float scalar) const { return Vector3(x / scalar, y / scalar, z / scalar); }

    float Length() const { return std::sqrt(x * x + y * y + z * z); }
    Vector3 Normalized() const
    {
        float len = Length();
        return len > 0 ? *this / len : Vector3();
    }
    float Dot(const Vector3 &other) const { return x * other.x + y * other.y + z * other.z; }
    Vector3 Cross(const Vector3 &other) const
    {
        return Vector3(y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x);
    }
};

struct Matrix3x3
{
    std::array<std::array<float, 3>, 3> data;

    Matrix3x3()
    {
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                data[i][j] = (i == j) ? 1.0f : 0.0f;
    }

    Matrix3x3 operator*(const Matrix3x3 &other) const
    {
        Matrix3x3 result;
        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                result.data[i][j] = 0;
                for (int k = 0; k < 3; k++)
                {
                    result.data[i][j] += data[i][k] * other.data[k][j];
                }
            }
        }
        return result;
    }

    Vector3 operator*(const Vector3 &vec) const
    {
        return Vector3(
            data[0][0] * vec.x + data[0][1] * vec.y + data[0][2] * vec.z,
            data[1][0] * vec.x + data[1][1] * vec.y + data[1][2] * vec.z,
            data[2][0] * vec.x + data[2][1] * vec.y + data[2][2] * vec.z);
    }
};

struct Matrix4x4
{
    std::array<std::array<float, 4>, 4> data;

    Matrix4x4()
    {
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                data[i][j] = (i == j) ? 1.0f : 0.0f;
    }

    Matrix4x4 operator*(const Matrix4x4 &other) const
    {
        Matrix4x4 result;
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                result.data[i][j] = 0;
                for (int k = 0; k < 4; k++)
                {
                    result.data[i][j] += data[i][k] * other.data[k][j];
                }
            }
        }
        return result;
    }

    Vector3 operator*(const Vector3 &vec) const
    {
        float w = data[3][0] * vec.x + data[3][1] * vec.y + data[3][2] * vec.z + data[3][3];
        if (w != 0)
        {
            return Vector3(
                (data[0][0] * vec.x + data[0][1] * vec.y + data[0][2] * vec.z + data[0][3]) / w,
                (data[1][0] * vec.x + data[1][1] * vec.y + data[1][2] * vec.z + data[1][3]) / w,
                (data[2][0] * vec.x + data[2][1] * vec.y + data[2][2] * vec.z + data[2][3]) / w);
        }
        return Vector3();
    }
};

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
    std::map<int, std::any> defaultValues;  // Valores por defecto de los pins de entrada

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
        float inputY = 44.0f; // Ajustado para alinear exactamente con el centro de los campos de entrada
        for (size_t i = 0; i < config.inputPins.size(); i++)
        {
            const auto &pinConfig = config.inputPins[i];
            Pin inputPin;
            inputPin.id = (int)i;
            inputPin.pos = ImVec2(0, inputY); // Completamente integrado en el borde del nodo
            inputPin.isExec = (pinConfig.type == ExecuteInput);
            newNode.n.inputs.push_back(inputPin);

            // Guardar el nombre del pin
            newNode.inputNames[i] = pinConfig.name;

            // Establecer valor por defecto si es un pin de datos
            if (pinConfig.type == Input)
            {
                newNode.inputValues[i] = pinConfig.defaultValue;
                newNode.defaultValues[i] = pinConfig.defaultValue; // Guardar valor por defecto
                // También establecer el valor de salida inicial para nodos que solo tienen entrada
                if (config.outputPins.empty())
                {
                    newNode.outputValues[0] = pinConfig.defaultValue;
                }
            }

            inputY += 28.0f; // Espaciado más compacto
        }

        // Configurar pins de salida - posicionados a la derecha del nodo
        float outputY = 44.0f; // Ajustado para alinear exactamente con el centro de los campos de entrada
        for (size_t i = 0; i < config.outputPins.size(); i++)
        {
            const auto &pinConfig = config.outputPins[i];
            Pin outputPin;
            outputPin.id = (int)i;
            outputPin.pos = ImVec2(config.size.x, outputY); // Completamente integrado en el borde del nodo
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

    // Función para actualizar inmediatamente un valor de entrada cuando se establece una conexión
    void UpdateNodeInputsImmediately(int fromNodeId, int fromPinId, int toNodeId, int toPinId)
    {
        CustomNode *sourceNode = GetCustomNodeById(fromNodeId);
        CustomNode *targetNode = GetCustomNodeById(toNodeId);

        if (!sourceNode || !targetNode)
            return;

        // Verificar que los pins sean válidos
        if (fromPinId >= sourceNode->n.outputs.size() || toPinId >= targetNode->n.inputs.size())
            return;

        // Solo transferir datos para pins que no son de ejecución
        if (!sourceNode->n.outputs[fromPinId].isExec)
        {
            auto outputValue = sourceNode->outputValues.find(fromPinId);
            if (outputValue != sourceNode->outputValues.end())
            {
                targetNode->inputValues[toPinId] = outputValue->second;

                // Si es un nodo String conectado a Print, también actualizar el valor de salida
                if (sourceNode->n.title == "String" && fromPinId == 0)
                {
                    sourceNode->SetOutputValue<std::string>(0, std::any_cast<std::string>(outputValue->second));
                }

                std::cout << "[CONNECTION] Updated node " << toNodeId << " pin " << toPinId
                          << " with value from node " << fromNodeId << " pin " << fromPinId
                          << " = " << std::any_cast<std::string>(outputValue->second) << std::endl;
            }
        }
    }

    // Función para forzar la actualización de todos los valores de entrada
    void ForceUpdateAllNodeInputs()
    {
        for (auto &connection : connections)
        {
            UpdateNodeInputsImmediately(connection.fromNodeId, connection.fromPinId,
                                        connection.toNodeId, connection.toPinId);
        }

        // También forzar la actualización de valores de salida de nodos String
        for (auto &node : customNodes)
        {
            if (node.n.title == "String")
            {
                auto inputValue = node.inputValues.find(0);
                if (inputValue != node.inputValues.end())
                {
                    try
                    {
                        std::string value = std::any_cast<std::string>(inputValue->second);
                        node.SetOutputValue<std::string>(0, value);
                        std::cout << "[FORCE UPDATE] String node output updated to: " << value << std::endl;
                    }
                    catch (...)
                    {
                        std::cout << "[FORCE UPDATE] String node output update failed" << std::endl;
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
            [](CustomNode *node)
            {
                // Get the current input value and use it as output
                std::string inputValue = node->GetInputValue<std::string>(0, "My String");
                node->SetOutputValue<std::string>(0, inputValue);
                std::cout << "[STRING] Output: " << inputValue << std::endl;
            },
            INPUT_OUTPUT,                   // category
            false,                          // hasExecInput
            false,                          // hasExecOutput
            {{"Text", std::string(value)}}, // inputPins - Now has an input pin for editing
            {{"", std::string(value)}},     // outputPins
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
            {{"", 0.0f}},               // outputPins
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
            INPUT_OUTPUT,              // category
            false,                     // hasExecInput
            false,                     // hasExecOutput
            {{"Value", initialValue}}, // inputPins
            {{"", initialValue}},      // outputPins
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
            MATH,                      // category
            false,                     // hasExecInput
            false,                     // hasExecOutput
            {{"Value", initialValue}}, // inputPins
            {{"", initialValue}},      // outputPins
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
            MATH,                      // category
            false,                     // hasExecInput
            false,                     // hasExecOutput
            {{"Value", initialValue}}, // inputPins
            {{"", initialValue}},      // outputPins
            position);
    }

    // Nodos de vectores
    CustomNode *CreateVector2Node(Vector2 initialValue = Vector2(0, 0), ImVec2 position = ImVec2(50, 450))
    {
        return CreateNode(
            "Vector2",
            [](CustomNode *node)
            {
                // Obtener valores de entrada individuales
                float x = node->GetInputValue<float>(0, 0.0f);
                float y = node->GetInputValue<float>(1, 0.0f);

                // Crear vector resultante
                Vector2 result(x, y);
                node->SetOutputValue<Vector2>(0, result);

                std::cout << "[VECTOR2] Value: (" << x << ", " << y << ")" << std::endl;
            },
            VECTOR,                                         // category
            false,                                          // hasExecInput
            false,                                          // hasExecOutput
            {{"X", initialValue.x}, {"Y", initialValue.y}}, // inputPins separados para X e Y
            {{"", initialValue}},                           // outputPins
            position);
    }

    CustomNode *CreateVector3Node(Vector3 initialValue = Vector3(0, 0, 0), ImVec2 position = ImVec2(50, 500))
    {
        return CreateNode(
            "Vector3",
            [](CustomNode *node)
            {
                // Obtener valores de entrada individuales
                float x = node->GetInputValue<float>(0, 0.0f);
                float y = node->GetInputValue<float>(1, 0.0f);
                float z = node->GetInputValue<float>(2, 0.0f);

                // Crear vector resultante
                Vector3 result(x, y, z);
                node->SetOutputValue<Vector3>(0, result);

                std::cout << "[VECTOR3] Value: (" << x << ", " << y << ", " << z << ")" << std::endl;
            },
            VECTOR,                                                                // category
            false,                                                                 // hasExecInput
            false,                                                                 // hasExecOutput
            {{"X", initialValue.x}, {"Y", initialValue.y}, {"Z", initialValue.z}}, // inputPins separados para X, Y, Z
            {{"", initialValue}},                                                  // outputPins
            position);
    }

    // Nodos de operaciones vectoriales
    CustomNode *CreateVectorAddNode(ImVec2 position = ImVec2(200, 450))
    {
        return CreateNode(
            "Vector Add",
            [](CustomNode *node)
            {
                // Obtener componentes de los vectores A y B
                float ax = node->GetInputValue<float>(0, 0.0f);
                float ay = node->GetInputValue<float>(1, 0.0f);
                float az = node->GetInputValue<float>(2, 0.0f);

                float bx = node->GetInputValue<float>(3, 0.0f);
                float by = node->GetInputValue<float>(4, 0.0f);
                float bz = node->GetInputValue<float>(5, 0.0f);

                Vector3 a(ax, ay, az);
                Vector3 b(bx, by, bz);
                Vector3 result = a + b;

                node->SetOutputValue<Vector3>(0, result);
                std::cout << "[VECTOR ADD] (" << ax << "," << ay << "," << az << ") + ("
                          << bx << "," << by << "," << bz << ") = ("
                          << result.x << "," << result.y << "," << result.z << ")" << std::endl;
            },
            VECTOR,                                                                                     // category
            false,                                                                                      // hasExecInput
            false,                                                                                      // hasExecOutput
            {{"A.X", 0.0f}, {"A.Y", 0.0f}, {"A.Z", 0.0f}, {"B.X", 0.0f}, {"B.Y", 0.0f}, {"B.Z", 0.0f}}, // inputPins separados
            {{"", Vector3(0, 0, 0)}},                                                                   // outputPins
            position);
    }

    CustomNode *CreateVectorCrossNode(ImVec2 position = ImVec2(200, 500))
    {
        return CreateNode(
            "Vector Cross",
            [](CustomNode *node)
            {
                // Obtener componentes de los vectores A y B
                float ax = node->GetInputValue<float>(0, 1.0f);
                float ay = node->GetInputValue<float>(1, 0.0f);
                float az = node->GetInputValue<float>(2, 0.0f);

                float bx = node->GetInputValue<float>(3, 0.0f);
                float by = node->GetInputValue<float>(4, 1.0f);
                float bz = node->GetInputValue<float>(5, 0.0f);

                Vector3 a(ax, ay, az);
                Vector3 b(bx, by, bz);
                Vector3 result = a.Cross(b);

                node->SetOutputValue<Vector3>(0, result);
                std::cout << "[VECTOR CROSS] (" << ax << "," << ay << "," << az << ") x ("
                          << bx << "," << by << "," << bz << ") = ("
                          << result.x << "," << result.y << "," << result.z << ")" << std::endl;
            },
            VECTOR,                                                                                     // category
            false,                                                                                      // hasExecInput
            false,                                                                                      // hasExecOutput
            {{"A.X", 1.0f}, {"A.Y", 0.0f}, {"A.Z", 0.0f}, {"B.X", 0.0f}, {"B.Y", 1.0f}, {"B.Z", 0.0f}}, // inputPins separados
            {{"", Vector3(0, 0, 1)}},                                                                   // outputPins
            position);
    }

    CustomNode *CreateVectorDotNode(ImVec2 position = ImVec2(200, 550))
    {
        return CreateNode(
            "Vector Dot",
            [](CustomNode *node)
            {
                // Obtener componentes de los vectores A y B
                float ax = node->GetInputValue<float>(0, 1.0f);
                float ay = node->GetInputValue<float>(1, 0.0f);
                float az = node->GetInputValue<float>(2, 0.0f);

                float bx = node->GetInputValue<float>(3, 1.0f);
                float by = node->GetInputValue<float>(4, 0.0f);
                float bz = node->GetInputValue<float>(5, 0.0f);

                Vector3 a(ax, ay, az);
                Vector3 b(bx, by, bz);
                float result = a.Dot(b);

                node->SetOutputValue<float>(0, result);
                std::cout << "[VECTOR DOT] (" << ax << "," << ay << "," << az << ") · ("
                          << bx << "," << by << "," << bz << ") = " << result << std::endl;
            },
            VECTOR,                                                                                     // category
            false,                                                                                      // hasExecInput
            false,                                                                                      // hasExecOutput
            {{"A.X", 1.0f}, {"A.Y", 0.0f}, {"A.Z", 0.0f}, {"B.X", 1.0f}, {"B.Y", 0.0f}, {"B.Z", 0.0f}}, // inputPins separados
            {{"", 1.0f}},                                                                               // outputPins
            position);
    }

    CustomNode *CreateVectorNormalizeNode(ImVec2 position = ImVec2(200, 600))
    {
        return CreateNode(
            "Vector Normalize",
            [](CustomNode *node)
            {
                // Obtener componentes del vector de entrada
                float x = node->GetInputValue<float>(0, 1.0f);
                float y = node->GetInputValue<float>(1, 1.0f);
                float z = node->GetInputValue<float>(2, 1.0f);

                Vector3 value(x, y, z);
                Vector3 result = value.Normalized();

                node->SetOutputValue<Vector3>(0, result);
                std::cout << "[VECTOR NORMALIZE] (" << x << "," << y << "," << z
                          << ") -> (" << result.x << "," << result.y << "," << result.z << ")" << std::endl;
            },
            VECTOR,                                  // category
            false,                                   // hasExecInput
            false,                                   // hasExecOutput
            {{"X", 1.0f}, {"Y", 1.0f}, {"Z", 1.0f}}, // inputPins separados para X, Y, Z
            {{"", Vector3(0.577f, 0.577f, 0.577f)}}, // outputPins
            position);
    }

    // Nodos de matrices
    CustomNode *CreateMatrix3x3Node(ImVec2 position = ImVec2(350, 450))
    {
        return CreateNode(
            "Matrix 3x3",
            [](CustomNode *node)
            {
                // Obtener valores de entrada para la diagonal principal
                float diag1 = node->GetInputValue<float>(0, 1.0f);
                float diag2 = node->GetInputValue<float>(1, 1.0f);
                float diag3 = node->GetInputValue<float>(2, 1.0f);

                // Crear matriz con valores en la diagonal
                Matrix3x3 result;
                result.data[0][0] = diag1;
                result.data[1][1] = diag2;
                result.data[2][2] = diag3;

                node->SetOutputValue<Matrix3x3>(0, result);
                std::cout << "[MATRIX 3x3] Diagonal: (" << diag1 << ", " << diag2 << ", " << diag3 << ")" << std::endl;
            },
            MATH,                                                // category
            false,                                               // hasExecInput
            false,                                               // hasExecOutput
            {{"Diag1", 1.0f}, {"Diag2", 1.0f}, {"Diag3", 1.0f}}, // inputPins para diagonal
            {{"", Matrix3x3()}},                                 // outputPins
            position);
    }

    CustomNode *CreateMatrix4x4Node(ImVec2 position = ImVec2(350, 500))
    {
        return CreateNode(
            "Matrix 4x4",
            [](CustomNode *node)
            {
                // Obtener valores de entrada para la diagonal principal
                float diag1 = node->GetInputValue<float>(0, 1.0f);
                float diag2 = node->GetInputValue<float>(1, 1.0f);
                float diag3 = node->GetInputValue<float>(2, 1.0f);
                float diag4 = node->GetInputValue<float>(3, 1.0f);

                // Crear matriz con valores en la diagonal
                Matrix4x4 result;
                result.data[0][0] = diag1;
                result.data[1][1] = diag2;
                result.data[2][2] = diag3;
                result.data[3][3] = diag4;

                node->SetOutputValue<Matrix4x4>(0, result);
                std::cout << "[MATRIX 4x4] Diagonal: (" << diag1 << ", " << diag2 << ", " << diag3 << ", " << diag4 << ")" << std::endl;
            },
            MATH,                                                                 // category
            false,                                                                // hasExecInput
            false,                                                                // hasExecOutput
            {{"Diag1", 1.0f}, {"Diag2", 1.0f}, {"Diag3", 1.0f}, {"Diag4", 1.0f}}, // inputPins para diagonal
            {{"", Matrix4x4()}},                                                  // outputPins
            position);
    }

    CustomNode *CreateMatrixMultiplyNode(ImVec2 position = ImVec2(350, 550))
    {
        return CreateNode(
            "Matrix Multiply",
            [](CustomNode *node)
            {
                // Obtener valores de entrada para las matrices
                float scaleA = node->GetInputValue<float>(0, 1.0f);
                float scaleB = node->GetInputValue<float>(1, 1.0f);

                // Crear matrices escaladas
                Matrix4x4 a, b;
                a.data[0][0] = scaleA;
                a.data[1][1] = scaleA;
                a.data[2][2] = scaleA;
                a.data[3][3] = scaleA;
                b.data[0][0] = scaleB;
                b.data[1][1] = scaleB;
                b.data[2][2] = scaleB;
                b.data[3][3] = scaleB;

                Matrix4x4 result = a * b;
                node->SetOutputValue<Matrix4x4>(0, result);
                std::cout << "[MATRIX MULTIPLY] A(" << scaleA << ") * B(" << scaleB << ") = Result" << std::endl;
            },
            MATH,                                   // category
            false,                                  // hasExecInput
            false,                                  // hasExecOutput
            {{"Scale A", 1.0f}, {"Scale B", 1.0f}}, // inputPins para escalas
            {{"", Matrix4x4()}},                    // outputPins
            position);
    }

    // Nodos matemáticos avanzados
    CustomNode *CreateSinNode(ImVec2 position = ImVec2(500, 450))
    {
        return CreateNode(
            "Sin",
            [](CustomNode *node)
            {
                float angle = node->GetInputValue<float>(0, 0.0f);
                float result = std::sin(angle);
                node->SetOutputValue<float>(0, result);
                std::cout << "[SIN] sin(" << angle << ") = " << result << std::endl;
            },
            MATH,              // category
            false,             // hasExecInput
            false,             // hasExecOutput
            {{"Angle", 0.0f}}, // inputPins
            {{"", 0.0f}},      // outputPins
            position);
    }

    CustomNode *CreateCosNode(ImVec2 position = ImVec2(500, 500))
    {
        return CreateNode(
            "Cos",
            [](CustomNode *node)
            {
                float angle = node->GetInputValue<float>(0, 0.0f);
                float result = std::cos(angle);
                node->SetOutputValue<float>(0, result);
                std::cout << "[COS] cos(" << angle << ") = " << result << std::endl;
            },
            MATH,              // category
            false,             // hasExecInput
            false,             // hasExecOutput
            {{"Angle", 0.0f}}, // inputPins
            {{"", 1.0f}},      // outputPins
            position);
    }

    CustomNode *CreateLerpNode(ImVec2 position = ImVec2(500, 550))
    {
        return CreateNode(
            "Lerp",
            [](CustomNode *node)
            {
                float a = node->GetInputValue<float>(0, 0.0f);
                float b = node->GetInputValue<float>(1, 1.0f);
                float t = node->GetInputValue<float>(2, 0.5f);
                float result = a + (b - a) * t;
                node->SetOutputValue<float>(0, result);
                std::cout << "[LERP] " << a << " + (" << b << " - " << a << ") * " << t << " = " << result << std::endl;
            },
            MATH,                                    // category
            false,                                   // hasExecInput
            false,                                   // hasExecOutput
            {{"A", 0.0f}, {"B", 1.0f}, {"T", 0.5f}}, // inputPins
            {{"", 0.5f}},                            // outputPins
            position);
    }

    CustomNode *CreateClampNode(ImVec2 position = ImVec2(500, 600))
    {
        return CreateNode(
            "Clamp",
            [](CustomNode *node)
            {
                float value = node->GetInputValue<float>(0, 0.5f);
                float min = node->GetInputValue<float>(1, 0.0f);
                float max = node->GetInputValue<float>(2, 1.0f);
                float result = std::clamp(value, min, max);
                node->SetOutputValue<float>(0, result);
                std::cout << "[CLAMP] clamp(" << value << ", " << min << ", " << max << ") = " << result << std::endl;
            },
            MATH,                                            // category
            false,                                           // hasExecInput
            false,                                           // hasExecOutput
            {{"Value", 0.5f}, {"Min", 0.0f}, {"Max", 1.0f}}, // inputPins
            {{"", 0.5f}},                                    // outputPins
            position);
    }

    // Función de ejemplo para crear un grafo matemático completo
    void CreateMathGraphExample()
    {
        // Crear nodos básicos
        auto vectorA = CreateVector3Node(Vector3(1, 2, 3), ImVec2(50, 100));
        auto vectorB = CreateVector3Node(Vector3(4, 5, 6), ImVec2(50, 200));

        // Operaciones vectoriales
        auto addNode = CreateVectorAddNode(ImVec2(200, 150));
        auto crossNode = CreateVectorCrossNode(ImVec2(200, 250));
        auto dotNode = CreateVectorDotNode(ImVec2(200, 350));

        // Operaciones matemáticas
        auto sinNode = CreateSinNode(ImVec2(350, 150));
        auto cosNode = CreateCosNode(ImVec2(350, 250));
        auto lerpNode = CreateLerpNode(ImVec2(350, 350));

        // Matrices
        auto matrixA = CreateMatrix4x4Node(ImVec2(500, 150));
        auto matrixB = CreateMatrix4x4Node(ImVec2(500, 250));
        auto matrixMult = CreateMatrixMultiplyNode(ImVec2(500, 350));

        // Conectar nodos
        connections.push_back({vectorA->nodeId, 0, addNode->nodeId, 0}); // VectorA -> Add.A
        connections.push_back({vectorB->nodeId, 0, addNode->nodeId, 1}); // VectorB -> Add.B

        connections.push_back({vectorA->nodeId, 0, crossNode->nodeId, 0}); // VectorA -> Cross.A
        connections.push_back({vectorB->nodeId, 0, crossNode->nodeId, 1}); // VectorB -> Cross.B

        connections.push_back({vectorA->nodeId, 0, dotNode->nodeId, 0}); // VectorA -> Dot.A
        connections.push_back({vectorB->nodeId, 0, dotNode->nodeId, 1}); // VectorB -> Dot.B

        connections.push_back({addNode->nodeId, 0, sinNode->nodeId, 0});   // Add.Result -> Sin.Angle
        connections.push_back({crossNode->nodeId, 0, cosNode->nodeId, 0}); // Cross.Result -> Cos.Angle

        connections.push_back({matrixA->nodeId, 0, matrixMult->nodeId, 0}); // MatrixA -> Mult.A
        connections.push_back({matrixB->nodeId, 0, matrixMult->nodeId, 1}); // MatrixB -> Mult.B

        std::cout << "[MATH GRAPH] Created complete mathematical graph with vectors, matrices, and operations!" << std::endl;
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

                // Si es un pin de entrada, restaurar su valor por defecto cuando se desconecta
                if (shouldRemove)
                {
                    CustomNode *targetNode = GetCustomNodeById(nodeId);
                    if (targetNode)
                    {
                        // Restaurar el valor por defecto del pin
                        auto nameIt = targetNode->inputNames.find(pinId);
                        if (nameIt != targetNode->inputNames.end())
                        {
                            // Buscar el valor por defecto en la configuración del nodo
                            auto defaultIt = targetNode->defaultValues.find(pinId);
                            if (defaultIt != targetNode->defaultValues.end())
                            {
                                // Restaurar el valor por defecto almacenado
                                targetNode->inputValues[pinId] = defaultIt->second;

                                // Para nodos String, también actualizar el valor de salida
                                if (targetNode->n.title == "String" && pinId == 0)
                                {
                                    try
                                    {
                                        std::string defaultValue = std::any_cast<std::string>(defaultIt->second);
                                        targetNode->SetOutputValue<std::string>(0, defaultValue);
                                        std::cout << "[DISCONNECT] Restored String node to default value: " << defaultValue << std::endl;
                                    }
                                    catch (...)
                                    {
                                        std::cout << "[DISCONNECT] Failed to restore String node default value" << std::endl;
                                    }
                                }
                                else
                                {
                                    std::cout << "[DISCONNECT] Restored node " << nodeId << " pin " << pinId << " to default value" << std::endl;
                                }
                            }
                            else
                            {
                                // Si no hay valor por defecto, limpiar el valor
                                targetNode->inputValues.erase(pinId);
                                std::cout << "[DISCONNECT] Cleared input value for node " << nodeId << " pin " << pinId << " (no default)" << std::endl;
                            }
                        }
                    }
                }
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

    // Método para eliminar todas las conexiones de un nodo
    void RemoveAllConnectionsFromNode(int nodeId)
    {
        auto it = connections.begin();
        while (it != connections.end())
        {
            if (it->fromNodeId == nodeId || it->toNodeId == nodeId)
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

        // Limpiar la selección del menú contextual
        contextMenuNodeId = -1;
    }

    // Método para eliminar un nodo y todas sus conexiones
    void DeleteNode(int nodeId)
    {
        std::cout << "[DELETE] Attempting to delete node with ID: " << nodeId << std::endl;
        std::cout << "[DELETE] Total nodes before deletion: " << customNodes.size() << std::endl;

        // Primero eliminar todas las conexiones del nodo
        RemoveAllConnectionsFromNode(nodeId);

        // Luego eliminar el nodo
        auto it = customNodes.begin();
        while (it != customNodes.end())
        {
            std::cout << "[DELETE] Checking node: ID=" << it->n.id << ", nodeId=" << it->nodeId << ", title=" << it->n.title << std::endl;
            if (it->n.id == nodeId)
            {
                std::cout << "[DELETE] Found node to delete: " << nodeId << " (" << it->n.title << ")" << std::endl;
                it = customNodes.erase(it);
                std::cout << "[DELETE] Node deleted successfully" << std::endl;
                break;
            }
            else
            {
                ++it;
            }
        }

        std::cout << "[DELETE] Total nodes after deletion: " << customNodes.size() << std::endl;

        // Deseleccionar si era el nodo seleccionado
        for (auto &node : customNodes)
        {
            if (node.n.isSelected)
            {
                node.n.isSelected = false;
                break;
            }
        }

        // Limpiar la selección del menú contextual
        contextMenuNodeId = -1;
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
        ForceUpdateAllNodeInputs();

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
        else if (value->type() == typeid(Vector2))
            return BlenderColors::PinVector;
        else if (value->type() == typeid(Vector3))
            return BlenderColors::PinVector;
        else if (value->type() == typeid(Matrix3x3))
            return BlenderColors::PinVector;
        else if (value->type() == typeid(Matrix4x4))
            return BlenderColors::PinVector;

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
            createNodeMenuPos = ImGui::GetIO().MousePos;
        }

        // Aplicar transformación del panel
        ImVec2 transformedMousePos = mousePos - panelOffset;

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
                        ImVec2 labelPos = ImVec2(min.x + 8, min.y + currentY);
                        draw_list->AddText(labelPos, IM_COL32(200, 200, 200, 255), pinName.c_str());

                        // Campo de entrada solo si no hay conexión
                        bool hasConnection = HasConnection(n.id, (int)pinIndex, true);
                        if (!hasConnection)
                        {
                            // Calcular el ancho disponible para el campo de entrada
                            // Dejar espacio para la etiqueta del pin (aproximadamente 80px) + márgenes
                            float availableWidth = n.size.x - 88.0f; // 80px para etiqueta + 8px de margen derecho

                            ImVec2 inputPos = ImVec2(min.x + 80, min.y + currentY - 2);
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
                        draw_list->AddText(labelPos, IM_COL32(200, 200, 200, 255), pinName.c_str());
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

                        // Actualizar inmediatamente el valor del nodo de destino
                        UpdateNodeInputsImmediately(connectingFromNode, connectingFromPin, n.id, (int)i);
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
                // Aplicar transformación del panel a la posición del pin de origen
                ImVec2 p1 = window_pos + panelOffset + fromNode->pos + fromNode->outputs[connectingFromPin].pos;
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
                        DeleteNode(contextMenuNodeId);
                        contextMenuNodeId = -1;
                    }

                    if (ImGui::MenuItem("Disconnect All", "Ctrl+D"))
                    {
                        RemoveAllConnectionsFromNode(contextMenuNodeId);
                        contextMenuNodeId = -1;
                    }

                    ImGui::Separator();

                    if (ImGui::MenuItem("Execute Graph"))
                    {
                        ExecuteGraph();
                    }

                    if (ImGui::MenuItem("Update Values"))
                    {
                        ForceUpdateAllNodeInputs();
                    }

                    if (ImGui::MenuItem("Reset View"))
                    {
                        panelOffset = ImVec2(0, 0);
                        std::cout << "[PANEL] View reset to default" << std::endl;
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
            DeleteNode(contextMenuNodeId);
            contextMenuNodeId = -1;
        }

        if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_D))
        {
            if (contextMenuNodeId != -1)
            {
                RemoveAllConnectionsFromNode(contextMenuNodeId);
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
            ImVec2 nodePos = createNodeMenuPos - ImGui::GetWindowPos();
            std::cout << "[CREATE MENU] Mouse pos: (" << createNodeMenuPos.x << ", " << createNodeMenuPos.y << ")" << std::endl;
            std::cout << "[CREATE MENU] Window pos: (" << ImGui::GetWindowPos().x << ", " << ImGui::GetWindowPos().y << ")" << std::endl;
            std::cout << "[CREATE MENU] Final node pos: (" << nodePos.x << ", " << nodePos.y << ")" << std::endl;

            if (ImGui::BeginMenu("Input/Output"))
            {
                if (ImGui::MenuItem("Start Node"))
                {
                    CreateStartNode(nodePos);
                }
                if (ImGui::MenuItem("String Node"))
                {
                    CreateStringNode("New String", nodePos);
                }
                if (ImGui::MenuItem("Boolean Node"))
                {
                    CreateBoolNode(false, nodePos);
                }
                if (ImGui::MenuItem("Integer Node"))
                {
                    CreateIntNode(0, nodePos);
                }
                if (ImGui::MenuItem("Float Node"))
                {
                    CreateFloatNode(0.0f, nodePos);
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Math"))
            {
                if (ImGui::MenuItem("Add Node"))
                {
                    CreateMathAddNode(nodePos);
                }
                if (ImGui::MenuItem("Sin Node"))
                {
                    CreateSinNode(nodePos);
                }
                if (ImGui::MenuItem("Cos Node"))
                {
                    CreateCosNode(nodePos);
                }
                if (ImGui::MenuItem("Lerp Node"))
                {
                    CreateLerpNode(nodePos);
                }
                if (ImGui::MenuItem("Clamp Node"))
                {
                    CreateClampNode(nodePos);
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Vector"))
            {
                if (ImGui::MenuItem("Vector2 Node"))
                {
                    CreateVector2Node(Vector2(0, 0), nodePos);
                }
                if (ImGui::MenuItem("Vector3 Node"))
                {
                    CreateVector3Node(Vector3(0, 0, 0), nodePos);
                }
                if (ImGui::MenuItem("Vector Add"))
                {
                    CreateVectorAddNode(nodePos);
                }
                if (ImGui::MenuItem("Vector Cross"))
                {
                    CreateVectorCrossNode(nodePos);
                }
                if (ImGui::MenuItem("Vector Dot"))
                {
                    CreateVectorDotNode(nodePos);
                }
                if (ImGui::MenuItem("Vector Normalize"))
                {
                    CreateVectorNormalizeNode(nodePos);
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Matrix"))
            {
                if (ImGui::MenuItem("Matrix 3x3"))
                {
                    CreateMatrix3x3Node(nodePos);
                }
                if (ImGui::MenuItem("Matrix 4x4"))
                {
                    CreateMatrix4x4Node(nodePos);
                }
                if (ImGui::MenuItem("Matrix Multiply"))
                {
                    CreateMatrixMultiplyNode(nodePos);
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Script"))
            {
                if (ImGui::MenuItem("Print Console"))
                {
                    CreatePrintNode(nodePos);
                }
                if (ImGui::MenuItem("Update Node"))
                {
                    CreateUpdateNode(nodePos);
                }
                if (ImGui::MenuItem("Timer Node"))
                {
                    CreateTimerNode(1.0f, nodePos);
                }
                if (ImGui::MenuItem("Delay Node"))
                {
                    CreateDelayNode(1.0f, nodePos);
                }
                ImGui::EndMenu();
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Create Math Graph Example"))
            {
                CreateMathGraphExample();
            }

            ImGui::EndPopup();
        }
    }
};
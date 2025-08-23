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
#include <components/SceneManager.h>
#include <components/GameObject.h>

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
class MNodeEngine;
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

    NodeConfig(const std::string &t, NodeCategory cat = INPUT_OUTPUT, ImVec2 s = ImVec2(300, 100))
        : title(t), category(cat), size(s) {}
};

// Definición de CustomNode ANTES de SimpleNodeEditor
class CustomNode
{
public:
    int nodeId;
    Node n;
    std::vector<PinInfo *> Pins;

    // Almacenamiento de datos del nodo
    std::map<std::string, std::any> nodeData;
    std::map<int, std::any> inputValues;    // Valores de entrada por índice de pin
    std::map<int, std::any> outputValues;   // Valores de salida por índice de pin
    std::map<int, std::string> inputNames;  // Nombres de los pins de entrada
    std::map<int, std::string> outputNames; // Nombres de los pins de salida
    std::map<int, std::any> defaultValues;  // Valores por defecto de los pins de entrada

    void SetupNode()
    {
        n.id = NodeID;
        nodeId = n.id;
        n.pos = ImVec2(50, 100);
        n.size = ImVec2(200, 80);
        n.title = "Print";
        n.data = "Hello";

        // Pin de ejecución de entrada (rojo) - posición ajustada fuera del nodo
        n.inputs.push_back({0, ImVec2(-15, 45), true});
        // Pin de datos de entrada para el texto (amarillo) - posición ajustada fuera del nodo
        n.inputs.push_back({1, ImVec2(-15, 70), false});
        // Pin de ejecución de salida (rojo) - posición ajustada fuera del nodo
        n.outputs.push_back({0, ImVec2(215, 45), true});

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

class PremakeNode
{
public:
    PremakeNode(const std::string &cat,
                const std::string &title,
                std::function<void(CustomNode *)> executeFunc,
                NodeCategory category = INPUT_OUTPUT,
                bool hasExecInput = false,
                bool hasExecOutput = false,
                std::vector<std::pair<std::string, std::any>> inputPins = {},
                std::vector<std::pair<std::string, std::any>> outputPins = {},
                ImVec2 position = ImVec2(100, 100),
                ImVec2 size = ImVec2(180, 80))
        : cat(cat),
          title(title),
          executeFunc(std::move(executeFunc)),
          category(category),
          hasExecInput(hasExecInput),
          hasExecOutput(hasExecOutput),
          inputPins(std::move(inputPins)),
          outputPins(std::move(outputPins)),
          position(position),
          size(size)
    {
    }

    std::string cat;   // 🔹 Ahora es dueño
    std::string title; // 🔹 Ahora es dueño
    std::function<void(CustomNode *)> executeFunc;
    NodeCategory category;
    bool hasExecInput;
    bool hasExecOutput;
    std::vector<std::pair<std::string, std::any>> inputPins;
    std::vector<std::pair<std::string, std::any>> outputPins;
    ImVec2 position;
    ImVec2 size;
};

// Editor de nodos
class MNodeEngine
{
public:
    std::vector<CustomNode> customNodes;
    std::vector<Connection> connections;

    std::vector<PremakeNode> PrefabNodes;

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

    // Función principal para crear nodos personalizados
    CustomNode *CreateCustomNode(const NodeConfig &config, ImVec2 position = ImVec2(50, 100))
    {
        CustomNode newNode;
        newNode.SetupNode();
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
            inputPin.pos = ImVec2(0, inputY); // Ligeramente fuera del borde para mejor detección
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
            outputPin.pos = ImVec2(config.size.x, outputY); // Ligeramente fuera del borde para mejor detección
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
            // Si el nodo fuente es un nodo de datos (sin pins de ejecución), ejecutarlo primero
            // para asegurar que tenga valores de salida válidos
            if (sourceNode->n.inputs.empty() || !sourceNode->n.inputs[0].isExec)
            {
                // Es un nodo de datos, ejecutarlo para establecer su valor de salida
                if (sourceNode->n.execFunc)
                {
                    std::cout << "[CONNECTION] Executing data node " << sourceNode->n.title << " to get output value" << std::endl;
                    sourceNode->n.execFunc(&sourceNode->n);
                }
            }

            auto outputValue = sourceNode->outputValues.find(fromPinId);
            if (outputValue != sourceNode->outputValues.end())
            {
                targetNode->inputValues[toPinId] = outputValue->second;

                // Si es un nodo String conectado a Print, también actualizar el valor de salida
                if (sourceNode->n.title == "String" && fromPinId == 0)
                {
                    try
                    {
                        sourceNode->SetOutputValue<std::string>(0, std::any_cast<std::string>(outputValue->second));
                    }
                    catch (const std::bad_any_cast &)
                    {
                        // Handle case where String node might have different data type
                        std::cout << "[WARNING] String node output type mismatch" << std::endl;
                    }
                }

                // Debug output that handles different data types safely
                std::cout << "[CONNECTION] Updated node " << toNodeId << " pin " << toPinId
                          << " with value from node " << fromNodeId << " pin " << fromPinId;

                try
                {
                    if (outputValue->second.type() == typeid(std::string))
                    {
                        std::cout << " = " << std::any_cast<std::string>(outputValue->second);
                    }
                    else if (outputValue->second.type() == typeid(int))
                    {
                        std::cout << " = " << std::any_cast<int>(outputValue->second);
                    }
                    else if (outputValue->second.type() == typeid(float))
                    {
                        std::cout << " = " << std::any_cast<float>(outputValue->second);
                    }
                    else if (outputValue->second.type() == typeid(bool))
                    {
                        std::cout << " = " << (std::any_cast<bool>(outputValue->second) ? "true" : "false");
                    }
                    else
                    {
                        std::cout << " = <unknown type: " << outputValue->second.type().name() << ">";
                    }
                }
                catch (const std::bad_any_cast &)
                {
                    std::cout << " = <type cast error>";
                }
                std::cout << std::endl;
            }
            else
            {
                std::cout << "[WARNING] Source node " << sourceNode->n.title << " has no output value for pin " << fromPinId << std::endl;
            }
        }
    }

    // Función para forzar la actualización de todos los valores de entrada
    void ForceUpdateAllNodeInputs()
    {
        // Primero, ejecutar todos los nodos de datos para asegurar que tengan valores de salida válidos
        for (auto &node : customNodes)
        {
            // Si es un nodo de datos (sin pins de ejecución), ejecutarlo
            if (node.n.inputs.empty() || !node.n.inputs[0].isExec)
            {
                if (node.n.execFunc)
                {
                    std::cout << "[FORCE UPDATE] Executing data node " << node.n.title << " to get output value" << std::endl;
                    node.n.execFunc(&node.n);
                }
            }
        }

        // Luego actualizar todas las conexiones
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
        ImVec2 size = ImVec2(300, 80))
    {
        std::cout << "[DEBUG] CreateNode called with position: (" << position.x << ", " << position.y << ")" << std::endl;
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
        ImVec2 size = ImVec2(350, 80))
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

    CustomNode *CreateMoveNode(ImVec2 position = ImVec2(200, 100))
    {
        return CreateNode(
            "Move Object",
            [](CustomNode *node)
            {
                int multiplieMove = node->GetInputValue<int>(1, 0);
                std::cout << "[MOVE NODE] Received input value: " << multiplieMove << std::endl;

                // Debug: mostrar todos los valores de entrada
                std::cout << "[MOVE NODE] All input values:" << std::endl;
                for (const auto &input : node->inputValues)
                {
                    std::cout << "  Pin " << input.first << ": ";
                    try
                    {
                        if (input.second.type() == typeid(int))
                        {
                            std::cout << std::any_cast<int>(input.second);
                        }
                        else if (input.second.type() == typeid(std::string))
                        {
                            std::cout << std::any_cast<std::string>(input.second);
                        }
                        else if (input.second.type() == typeid(float))
                        {
                            std::cout << std::any_cast<float>(input.second);
                        }
                        else if (input.second.type() == typeid(bool))
                        {
                            std::cout << (std::any_cast<bool>(input.second) ? "true" : "false");
                        }
                        else
                        {
                            std::cout << "<unknown type: " << input.second.type().name() << ">";
                        }
                    }
                    catch (const std::bad_any_cast &)
                    {
                        std::cout << "<type cast error>";
                    }
                    std::cout << std::endl;
                }
            },
            SCRIPT,                // category
            true,                  // hasExecInput
            true,                  // hasExecOutput
            {{"Index Object", 1}}, // inputPins
            {},                    // outputPins
            position);
    }

    CustomNode *CreateStringNode(const std::string &value = "My String", ImVec2 position = ImVec2(50, 200))
    {
        std::cout << "[DEBUG] CreateStringNode called with position: (" << position.x << ", " << position.y << ")" << std::endl;
        return CreateNode(
            "String",
            [](CustomNode *node)
            {
                std::string inputValue = node->GetInputValue<std::string>(0, "My String");
                node->SetOutputValue<std::string>(0, inputValue);
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
                node->nodeData["started"] = true;
                node->nodeData["startTime"] = std::chrono::steady_clock::now();
            },
            INPUT_OUTPUT, // category
            false,        // hasExecInput
            true,         // hasExecOutput
            {},           // inputPins
            {},           // outputPins - solo pin de ejecución, sin datos
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
            false,                                                        // hasExecOutput - solo datos, sin ejecución
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
            false,                                     // hasExecOutput - solo datos, sin ejecución
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
            false,                                                                       // hasExecOutput - solo datos, sin ejecución
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

    CustomNode *CreateIntNode(int initialValue = 1, ImVec2 position = ImVec2(50, 350))
    {
        return CreateNode(
            "Integer",
            [initialValue](CustomNode *node)
            {
                int value = node->GetInputValue<int>(0, initialValue);
                node->SetOutputValue<int>(0, value);

                // Debug: verificar que el valor se estableció correctamente
                try
                {
                    int outputValue = node->GetOutputValue<int>(0, -999);
                }
                catch (...)
                {
                    std::cout << "[INT] Error getting output value" << std::endl;
                }
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

    // Método para validar si una conexión es válida (VERSIÓN MEJORADA)
    bool IsValidConnection(Node *fromNode, int fromPinId, Node *toNode, int toPinId)
    {
        if (!fromNode || !toNode)
        {
            std::cout << "  [ERROR] Invalid: null node" << std::endl;
            return false;
        }

        // Verificar que los índices sean válidos
        if (fromPinId < 0 || fromPinId >= fromNode->outputs.size())
        {
            std::cout << "  [ERROR] Invalid: fromPinId out of bounds. From outputs size: " << fromNode->outputs.size() << std::endl;
            return false;
        }

        if (toPinId < 0 || toPinId >= toNode->inputs.size())
        {
            std::cout << "  [ERROR] Invalid: toPinId out of bounds. To inputs size: " << toNode->inputs.size() << std::endl;
            return false;
        }

        Pin &fromPin = fromNode->outputs[fromPinId];
        Pin &toPin = toNode->inputs[toPinId];

        // Solo se pueden conectar pins del mismo tipo
        // Ejecución con ejecución, datos con datos
        if (fromPin.isExec != toPin.isExec)
        {
            std::cout << "  [ERROR] Invalid: pin type mismatch. From isExec: " << fromPin.isExec
                      << ", To isExec: " << toPin.isExec << std::endl;
            return false;
        }

        // No permitir auto-conexiones
        if (fromNode->id == toNode->id)
        {
            std::cout << "  [ERROR] Invalid: self-connection not allowed" << std::endl;
            return false;
        }

        // Verificar si ya existe una conexión en el pin de entrada
        for (auto &c : connections)
        {
            if (c.toNodeId == toNode->id && c.toPinId == toPinId)
            {
                std::cout << "  [ERROR] Invalid: pin already has connection" << std::endl;
                return false; // Ya hay una conexión en este pin de entrada
            }
        }

        // Verificar que no haya ciclos (conexión que vuelva al nodo de origen)
        if (WouldCreateCycle(fromNode->id, toNode->id))
        {
            std::cout << "  [ERROR] Invalid: connection would create a cycle" << std::endl;
            return false;
        }

        return true;
    }

    // Método para verificar si una conexión crearía un ciclo
    bool WouldCreateCycle(int fromNodeId, int toNodeId)
    {
        // Implementación simple: verificar si el nodo destino ya tiene una ruta hacia el nodo origen
        std::set<int> visited;
        return HasPathToNode(toNodeId, fromNodeId, visited);
    }

    // Método para verificar si existe una ruta desde un nodo hacia otro
    bool HasPathToNode(int currentNodeId, int targetNodeId, std::set<int> &visited)
    {
        if (currentNodeId == targetNodeId)
            return true;

        if (visited.find(currentNodeId) != visited.end())
            return false;

        visited.insert(currentNodeId);

        // Buscar en todas las conexiones que salen de este nodo
        for (const auto &connection : connections)
        {
            if (connection.fromNodeId == currentNodeId)
            {
                if (HasPathToNode(connection.toNodeId, targetNodeId, visited))
                    return true;
            }
        }

        return false;
    }

    // Método mejorado para verificar si un pin tiene conexiones
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

    // Método mejorado para crear conexiones
    bool CreateConnection(int fromNodeId, int fromPinId, int toNodeId, int toPinId)
    {
        std::cout << "[CONNECTION] Attempting to create connection: " << fromNodeId << ":" << fromPinId
                  << " -> " << toNodeId << ":" << toPinId << std::endl;

        CustomNode *fromNode = GetCustomNodeById(fromNodeId);
        CustomNode *toNode = GetCustomNodeById(toNodeId);

        if (!fromNode || !toNode)
        {
            std::cout << "[ERROR] One or both nodes not found" << std::endl;
            return false;
        }

        if (!IsValidConnection(&fromNode->n, fromPinId, &toNode->n, toPinId))
        {
            std::cout << "[ERROR] Connection validation failed" << std::endl;
            return false;
        }

        // Crear la conexión
        Connection newConnection;
        newConnection.fromNodeId = fromNodeId;
        newConnection.fromPinId = fromPinId;
        newConnection.toNodeId = toNodeId;
        newConnection.toPinId = toPinId;

        connections.push_back(newConnection);
        std::cout << "[SUCCESS] Connection created successfully!" << std::endl;

        // Actualizar inmediatamente el valor del nodo de destino
        UpdateNodeInputsImmediately(fromNodeId, fromPinId, toNodeId, toPinId);

        return true;
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
            if (it->n.id == nodeId || it->nodeId == nodeId)
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

    // Método de debug para diagnosticar problemas de conexión
    void DebugConnectionSystem()
    {
        std::cout << "\n=== DEBUG CONNECTION SYSTEM ===" << std::endl;
        std::cout << "Total nodes: " << customNodes.size() << std::endl;
        std::cout << "Total connections: " << connections.size() << std::endl;

        for (const auto &node : customNodes)
        {
            std::cout << "Node ID: " << node.n.id << " (nodeId: " << node.nodeId << ") - " << node.n.title << std::endl;
            std::cout << "  Inputs: " << node.n.inputs.size() << std::endl;
            std::cout << "  Outputs: " << node.n.outputs.size() << std::endl;

            // Mostrar valores de entrada
            for (const auto &input : node.inputValues)
            {
                std::cout << "    Input " << input.first << ": ";
                try
                {
                    if (input.second.type() == typeid(std::string))
                        std::cout << std::any_cast<std::string>(input.second);
                    else if (input.second.type() == typeid(int))
                        std::cout << std::any_cast<int>(input.second);
                    else if (input.second.type() == typeid(float))
                        std::cout << std::any_cast<float>(input.second);
                    else if (input.second.type() == typeid(bool))
                        std::cout << (std::any_cast<bool>(input.second) ? "true" : "false");
                    else if (input.second.type() == typeid(GameObject *))
                    {
                        GameObject *obj = std::any_cast<GameObject *>(input.second);
                        if (obj)
                            std::cout << "GameObject: " << obj->Name;
                        else
                            std::cout << "GameObject: nullptr";
                    }
                    else
                        std::cout << "<unknown type: " << input.second.type().name() << ">";
                }
                catch (...)
                {
                    std::cout << "<error>";
                }
                std::cout << std::endl;
            }

            // Mostrar valores de salida
            for (const auto &output : node.outputValues)
            {
                std::cout << "    Output " << output.first << ": ";
                try
                {
                    if (output.second.type() == typeid(std::string))
                        std::cout << std::any_cast<std::string>(output.second);
                    else if (output.second.type() == typeid(int))
                        std::cout << std::any_cast<int>(output.second);
                    else if (output.second.type() == typeid(float))
                        std::cout << std::any_cast<float>(output.second);
                    else if (output.second.type() == typeid(bool))
                        std::cout << (std::any_cast<bool>(output.second) ? "true" : "false");
                    else if (output.second.type() == typeid(GameObject *))
                    {
                        GameObject *obj = std::any_cast<GameObject *>(output.second);
                        if (obj)
                            std::cout << "GameObject: " << obj->Name;
                        else
                            std::cout << "GameObject: nullptr";
                    }
                    else
                        std::cout << "<unknown type: " << output.second.type().name() << ">";
                }
                catch (...)
                {
                    std::cout << "<error>";
                }
                std::cout << std::endl;
            }
        }

        std::cout << "\nConnections:" << std::endl;
        for (const auto &conn : connections)
        {
            std::cout << "  " << conn.fromNodeId << ":" << conn.fromPinId << " -> "
                      << conn.toNodeId << ":" << conn.toPinId << std::endl;
        }
        std::cout << "=== END DEBUG ===\n"
                  << std::endl;
    }

    // Método para verificar la integridad del sistema de conexiones
    bool ValidateConnectionIntegrity()
    {
        std::cout << "[INTEGRITY] Validating connection system integrity..." << std::endl;
        bool isValid = true;

        for (const auto &conn : connections)
        {
            // Verificar que los nodos existen
            CustomNode *fromNode = GetCustomNodeById(conn.fromNodeId);
            CustomNode *toNode = GetCustomNodeById(conn.toNodeId);

            if (!fromNode)
            {
                std::cout << "[ERROR] Connection " << conn.fromNodeId << ":" << conn.fromPinId
                          << " -> " << conn.toNodeId << ":" << conn.toPinId
                          << " references non-existent fromNode " << conn.fromNodeId << std::endl;
                isValid = false;
            }

            if (!toNode)
            {
                std::cout << "[ERROR] Connection " << conn.fromNodeId << ":" << conn.fromPinId
                          << " -> " << conn.toNodeId << ":" << conn.toPinId
                          << " references non-existent toNode " << conn.toNodeId << std::endl;
                isValid = false;
            }

            if (fromNode && toNode)
            {
                // Verificar que los pins existen
                if (conn.fromPinId < 0 || conn.fromPinId >= fromNode->n.outputs.size())
                {
                    std::cout << "[ERROR] Connection " << conn.fromNodeId << ":" << conn.fromPinId
                              << " -> " << conn.toNodeId << ":" << conn.toPinId
                              << " has invalid fromPinId " << conn.fromPinId
                              << " (max: " << fromNode->n.outputs.size() - 1 << ")" << std::endl;
                    isValid = false;
                }

                if (conn.toPinId < 0 || conn.toPinId >= toNode->n.inputs.size())
                {
                    std::cout << "[ERROR] Connection " << conn.fromNodeId << ":" << conn.fromPinId
                              << " -> " << conn.toNodeId << ":" << conn.toPinId
                              << " has invalid toPinId " << conn.toPinId
                              << " (max: " << toNode->n.inputs.size() - 1 << ")" << std::endl;
                    isValid = false;
                }
            }
        }

        if (isValid)
        {
            std::cout << "[INTEGRITY] All connections are valid!" << std::endl;
        }
        else
        {
            std::cout << "[INTEGRITY] Found connection integrity issues!" << std::endl;
        }

        return isValid;
    }

    // Método para limpiar conexiones inválidas automáticamente
    void CleanupInvalidConnections()
    {
        std::cout << "[CLEANUP] Cleaning up invalid connections..." << std::endl;
        int removedCount = 0;

        auto it = connections.begin();
        while (it != connections.end())
        {
            bool shouldRemove = false;

            // Verificar que los nodos existen
            CustomNode *fromNode = GetCustomNodeById(it->fromNodeId);
            CustomNode *toNode = GetCustomNodeById(it->toNodeId);

            if (!fromNode || !toNode)
            {
                shouldRemove = true;
                std::cout << "[CLEANUP] Removing connection with non-existent node(s): "
                          << it->fromNodeId << ":" << it->fromPinId << " -> "
                          << it->toNodeId << ":" << it->toPinId << std::endl;
            }
            else
            {
                // Verificar que los pins existen
                if (it->fromPinId < 0 || it->fromPinId >= fromNode->n.outputs.size() ||
                    it->toPinId < 0 || it->toPinId >= toNode->n.inputs.size())
                {
                    shouldRemove = true;
                    std::cout << "[CLEANUP] Removing connection with invalid pin(s): "
                              << it->fromPinId << " -> "
                              << it->toPinId << std::endl;
                }
            }

            if (shouldRemove)
            {
                it = connections.erase(it);
                removedCount++;
            }
            else
            {
                ++it;
            }
        }

        if (removedCount > 0)
        {
            std::cout << "[CLEANUP] Removed " << removedCount << " invalid connections" << std::endl;
        }
        else
        {
            std::cout << "[CLEANUP] No invalid connections found" << std::endl;
        }
    }

    // Método para diagnosticar problemas de conexión específicos
    void DiagnoseConnection(int fromNodeId, int fromPinId, int toNodeId, int toPinId)
    {
        std::cout << "\n=== DIAGNOSIS: Connection " << fromNodeId << ":" << fromPinId
                  << " -> " << toNodeId << ":" << toPinId << " ===" << std::endl;

        CustomNode *fromNode = GetCustomNodeById(fromNodeId);
        CustomNode *toNode = GetCustomNodeById(toNodeId);

        if (!fromNode)
        {
            std::cout << "[ERROR] Source node " << fromNodeId << " not found!" << std::endl;
            return;
        }

        if (!toNode)
        {
            std::cout << "[ERROR] Target node " << toNodeId << " not found!" << std::endl;
            return;
        }

        std::cout << "Source node: " << fromNode->n.title << " (ID: " << fromNodeId << ")" << std::endl;
        std::cout << "Target node: " << toNode->n.title << " (ID: " << toNodeId << ")" << std::endl;

        // Verificar pins de salida del nodo fuente
        if (fromPinId >= 0 && fromPinId < fromNode->n.outputs.size())
        {
            Pin &outputPin = fromNode->n.outputs[fromPinId];
            std::cout << "Source output pin " << fromPinId << ": isExec=" << outputPin.isExec << std::endl;

            auto outputValue = fromNode->outputValues.find(fromPinId);
            if (outputValue != fromNode->outputValues.end())
            {
                std::cout << "Source output value: ";
                try
                {
                    if (outputValue->second.type() == typeid(int))
                    {
                        std::cout << std::any_cast<int>(outputValue->second);
                    }
                    else if (outputValue->second.type() == typeid(std::string))
                    {
                        std::cout << std::any_cast<std::string>(outputValue->second);
                    }
                    else if (outputValue->second.type() == typeid(float))
                    {
                        std::cout << std::any_cast<float>(outputValue->second);
                    }
                    else
                    {
                        std::cout << "<unknown type: " << outputValue->second.type().name() << ">";
                    }
                }
                catch (const std::bad_any_cast &)
                {
                    std::cout << "<type cast error>";
                }
                std::cout << std::endl;
            }
            else
            {
                std::cout << "Source output value: <not set>" << std::endl;
            }
        }
        else
        {
            std::cout << "[ERROR] Invalid source pin ID: " << fromPinId << std::endl;
        }

        // Verificar pins de entrada del nodo destino
        if (toPinId >= 0 && toPinId < toNode->n.inputs.size())
        {
            Pin &inputPin = toNode->n.inputs[toPinId];
            std::cout << "Target input pin " << toPinId << ": isExec=" << inputPin.isExec << std::endl;

            auto inputValue = toNode->inputValues.find(toPinId);
            if (inputValue != toNode->inputValues.end())
            {
                std::cout << "Target input value: ";
                try
                {
                    if (inputValue->second.type() == typeid(int))
                    {
                        std::cout << std::any_cast<int>(inputValue->second);
                    }
                    else if (inputValue->second.type() == typeid(std::string))
                    {
                        std::cout << std::any_cast<std::string>(inputValue->second);
                    }
                    else if (inputValue->second.type() == typeid(float))
                    {
                        std::cout << std::any_cast<float>(inputValue->second);
                    }
                    else
                    {
                        std::cout << "<unknown type: " << inputValue->second.type().name() << ">";
                    }
                }
                catch (const std::bad_any_cast &)
                {
                    std::cout << "<type cast error>";
                }
                std::cout << std::endl;
            }
            else
            {
                std::cout << "Target input value: <not set>" << std::endl;
            }
        }
        else
        {
            std::cout << "[ERROR] Invalid target pin ID: " << toPinId << std::endl;
        }

        std::cout << "=== END DIAGNOSIS ===\n"
                  << std::endl;
    }

    // Función para mostrar la estructura de pins de cualquier nodo
    void DebugNodePins(int nodeId)
    {
        CustomNode *node = GetCustomNodeById(nodeId);
        if (!node)
        {
            std::cout << "[DEBUG] Node " << nodeId << " not found!" << std::endl;
            return;
        }

        std::cout << "\n[DEBUG] Node: " << node->n.title << " (ID: " << node->n.id << ")" << std::endl;
        std::cout << "  Inputs (" << node->n.inputs.size() << "):" << std::endl;
        for (size_t i = 0; i < node->n.inputs.size(); i++)
        {
            Pin &pin = node->n.inputs[i];
            std::cout << "    " << i << ": isExec=" << pin.isExec << " (pos: " << pin.pos.x << ", " << pin.pos.y << ")" << std::endl;
        }

        std::cout << "  Outputs (" << node->n.outputs.size() << "):" << std::endl;
        for (size_t i = 0; i < node->n.outputs.size(); i++)
        {
            Pin &pin = node->n.outputs[i];
            std::cout << "  " << i << ": isExec=" << pin.isExec << " (pos: " << pin.pos.x << ", " << pin.pos.y << ")" << std::endl;
        }
        std::cout << std::endl;
    }

    // Método para probar la conexión entre Integer y Move nodes
    void TestIntegerToMoveConnection()
    {
        std::cout << "\n=== TESTING INTEGER TO MOVE CONNECTION ===" << std::endl;

        // Buscar nodos Integer y Move
        CustomNode *integerNode = nullptr;
        CustomNode *moveNode = nullptr;

        for (auto &node : customNodes)
        {
            if (node.n.title == "Integer")
            {
                integerNode = &node;
            }
            else if (node.n.title == "Move Object")
            {
                moveNode = &node;
            }
        }

        if (!integerNode)
        {
            std::cout << "[ERROR] No Integer node found!" << std::endl;
            return;
        }

        if (!moveNode)
        {
            std::cout << "[ERROR] No Move Object node found!" << std::endl;
            return;
        }

        std::cout << "Integer node ID: " << integerNode->nodeId << std::endl;
        std::cout << "Move node ID: " << moveNode->nodeId << std::endl;

        // Ejecutar el nodo Integer para establecer su valor de salida
        if (integerNode->n.execFunc)
        {
            std::cout << "[TEST] Executing Integer node..." << std::endl;
            integerNode->n.execFunc(&integerNode->n);
        }

        // Verificar el valor de salida del Integer
        auto outputValue = integerNode->outputValues.find(0);
        if (outputValue != integerNode->outputValues.end())
        {
            std::cout << "[TEST] Integer output value: ";
            try
            {
                if (outputValue->second.type() == typeid(int))
                {
                    std::cout << std::any_cast<int>(outputValue->second);
                }
                else
                {
                    std::cout << "<wrong type: " << outputValue->second.type().name() << ">";
                }
            }
            catch (const std::bad_any_cast &)
            {
                std::cout << "<type cast error>";
            }
            std::cout << std::endl;
        }
        else
        {
            std::cout << "[TEST] Integer output value: <not set>" << std::endl;
        }

        // Verificar el valor de entrada del Move
        auto inputValue = moveNode->inputValues.find(1);
        if (inputValue != moveNode->inputValues.end())
        {
            std::cout << "[TEST] Move input value (pin 1): ";
            try
            {
                if (inputValue->second.type() == typeid(int))
                {
                    std::cout << std::any_cast<int>(inputValue->second);
                }
                else
                {
                    std::cout << "<wrong type: " << inputValue->second.type().name() << ">";
                }
            }
            catch (const std::bad_any_cast &)
            {
                std::cout << "<type cast error>";
            }
            std::cout << std::endl;
        }
        else
        {
            std::cout << "[TEST] Move input value (pin 1): <not set>" << std::endl;
        }

        // Verificar si hay una conexión entre ellos
        bool connectionFound = false;
        for (const auto &conn : connections)
        {
            if (conn.fromNodeId == integerNode->nodeId && conn.fromPinId == 0 &&
                conn.toNodeId == moveNode->nodeId && conn.toPinId == 1)
            {
                connectionFound = true;
                std::cout << "[TEST] Connection found: Integer(0) -> Move(1)" << std::endl;
                break;
            }
        }

        if (!connectionFound)
        {
            std::cout << "[TEST] No connection found between Integer and Move nodes" << std::endl;
        }

        std::cout << "=== END TEST ===\n"
                  << std::endl;
    }

    void ExecuteFrom(CustomNode *d)
    {
        Node *node = &d->n;
        if (!node || !node->execFunc)
            return;

        node->isActive = true;

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
        // ForceUpdateAllNodeInputs();

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

        // Determinar color por tipo de datos con colores específicos
        if (value->type() == typeid(std::string))
            return IM_COL32(100, 200, 100, 255); // Verde para String
        else if (value->type() == typeid(int))
            return IM_COL32(255, 150, 50, 255); // Naranja para Int
        else if (value->type() == typeid(float))
            return IM_COL32(100, 150, 255, 255); // Azul para Float
        else if (value->type() == typeid(bool))
            return BlenderColors::PinColor;
        else if (value->type() == typeid(Vector2))
            return IM_COL32(255, 100, 200, 255); // Rosa para Vector2
        else if (value->type() == typeid(Vector3))
            return IM_COL32(200, 100, 255, 255); // Púrpura para Vector3
        else if (value->type() == typeid(Matrix3x3))
            return IM_COL32(255, 200, 100, 255); // Amarillo para Matrix3x3
        else if (value->type() == typeid(Matrix4x4))
            return IM_COL32(100, 255, 200, 255); // Verde azulado para Matrix4x4
        else if (value->type() == typeid(GameObject *))
            return IM_COL32(255, 100, 100, 255); // Rojo para GameObject pointers

        return BlenderColors::PinFloat;
    }
};
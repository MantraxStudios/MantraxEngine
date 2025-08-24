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
#include <queue>
#include "../components/GameObject.h"
#include "../core/CoreExporter.h"

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
class MANTRAXCORE_API CustomNode
{
public:
    int nodeId;
    GameObject *_SelfObject;
    Node n;
    std::vector<PinInfo *> Pins;

    // Almacenamiento de datos del nodo
    std::map<std::string, std::any> nodeData;
    std::map<int, std::any> inputValues;    // Valores de entrada por índice de pin
    std::map<int, std::any> outputValues;   // Valores de salida por índice de pin
    std::map<int, std::string> inputNames;  // Nombres de los pins de entrada
    std::map<int, std::string> outputNames; // Nombres de los pins de salida
    std::map<int, std::any> defaultValues;  // Valores por defecto de los pins de entrada

    void SetupNode();
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

class MANTRAXCORE_API PremakeNode
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

    std::string cat;
    std::string title;
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
class MANTRAXCORE_API MNodeEngine
{
public:
    GameObject *_SelfObject;

    MNodeEngine(GameObject *obj);

    std::vector<CustomNode>
        customNodes;
    std::vector<Connection> connections;

    std::vector<PremakeNode> PrefabNodes;

    mutable std::vector<int> topologicalOrder;
    mutable bool topologicalOrderDirty = true;

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

    std::vector<int> CalculateTopologicalOrder() const;

    CustomNode *GetNodeById(int id);

    CustomNode *GetCustomNodeById(int id);

    // Función principal para crear nodos personalizados
    CustomNode *CreateCustomNode(const NodeConfig &config, ImVec2 position = ImVec2(50, 100));

    // Función para actualizar los valores de entrada de un nodo desde sus conexiones
    void UpdateNodeInputs(CustomNode *node);

    // Función para actualizar inmediatamente un valor de entrada cuando se establece una conexión
    void UpdateNodeInputsImmediately(int fromNodeId, int fromPinId, int toNodeId, int toPinId);

    // Función para forzar la actualización de todos los valores de entrada
    void ForceUpdateAllNodeInputs();

    bool IsDataNode(const CustomNode &node) const;

    void PropagateNodeOutputs(CustomNode *node);

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
        ImVec2 size = ImVec2(500, 80));

    // Versión de compatibilidad para código existente (sin NodeCategory)
    CustomNode *CreateNode(
        const std::string &title,
        std::function<void(CustomNode *)> executeFunc,
        bool hasExecInput,
        bool hasExecOutput,
        std::vector<std::pair<std::string, std::any>> inputPins = {},
        std::vector<std::pair<std::string, std::any>> outputPins = {},
        ImVec2 position = ImVec2(100, 100),
        ImVec2 size = ImVec2(350, 80));

    // Método para validar si una conexión es válida (VERSIÓN MEJORADA)
    bool IsValidConnection(Node *fromNode, int fromPinId, Node *toNode, int toPinId);
    // Método para verificar si una conexión crearía un ciclo
    bool WouldCreateCycle(int fromNodeId, int toNodeId);

    // Método para verificar si existe una ruta desde un nodo hacia otro
    bool HasPathToNode(int currentNodeId, int targetNodeId, std::set<int> &visited);

    // Método mejorado para verificar si un pin tiene conexiones
    bool HasConnection(int nodeId, int pinId, bool isInput);
    // Método mejorado para crear conexiones
    bool CreateConnection(int fromNodeId, int fromPinId, int toNodeId, int toPinId);
    // Método para eliminar todas las conexiones de un nodo
    void RemoveAllConnectionsFromNode(int nodeId);

    // Método para eliminar un nodo y todas sus conexiones
    void DeleteNode(int nodeId);

    // Método para obtener el valor de un nodo conectado
    std::string GetConnectedValue(int nodeId, int pinId);

    // Método de debug para diagnosticar problemas de conexión
    void DebugConnectionSystem();

    // Método para verificar la integridad del sistema de conexiones
    bool ValidateConnectionIntegrity();
    // Método para limpiar conexiones inválidas automáticamente
    void CleanupInvalidConnections();
    void ExecuteFrom(CustomNode *d);

    void EvaluateDataDependencies(CustomNode *node);
    // NUEVO: Evaluación recursiva de dependencias
    void EvaluateDataDependenciesRecursive(CustomNode *node, std::set<int> &visited);

    void ExecuteGraph();

    // MEJORADO: Eliminar conexiones con limpieza de orden topológico
    void RemoveConnectionsFromPin(int nodeId, int pinId, bool isInput);
};
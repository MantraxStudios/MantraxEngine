#include "MNodeEngine.h"

#include "GameObjectNode.h"
#include "DebugNodes.h"
#include "EventsNode.h"
#include "MathNodes.h"
#include "ConstNode.h"
#include "ConvertsNode.h"
#include "ShaderNode.h"
#include "ConditionsNode.h"
#include "DescomposerNode.h"
#include "AudioNode.h"
#include "RigidBodyNode.h"

static int NodeID = 0;

void CustomNode::SetupNode()
{
    n.id = NodeID;
    nodeId = n.id;
    n.pos = ImVec2(50, 100);
    n.size = ImVec2(400, 80);
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

MNodeEngine::MNodeEngine(GameObject *obj)
{
    _SelfObject = obj;

    GameObjectNode *NodesGM = new GameObjectNode();
    DebugNodes *NodesDB = new DebugNodes();
    EventsNode *NodesEV = new EventsNode();
    MathNodes *NodesMath = new MathNodes();
    ConstNode *NodesConst = new ConstNode();
    ConvertsNode *NodesConvert = new ConvertsNode();
    ConditionNodes *NodesCondition = new ConditionNodes();
    ShaderNode *NodesShader = new ShaderNode();
    AudioNode *NodeAudio = new AudioNode();
    DescomposerNode *NodeDescomposer = new DescomposerNode();
    RigidBodyNode *NodeRigidbody = new RigidBodyNode();

    NodesGM->RegisterNodes(*this);
    NodesDB->RegisterNodes(*this);
    NodesEV->RegisterNodes(*this);
    NodesMath->RegisterNodes(*this);
    NodesConst->RegisterNodes(*this);
    NodesConvert->RegisterNodes(*this);
    NodesCondition->RegisterNodes(*this);
    NodesShader->RegisterNodes(*this);
    NodeAudio->RegisterNodes(*this);
    NodeDescomposer->RegisterNodes(*this);
    NodeRigidbody->RegisterNodes(*this);
}

// Lambda Factory para crear nodos de manera simple (Nueva versión con NodeCategory)
CustomNode *MNodeEngine::CreateNode(
    const std::string &title,
    std::function<void(CustomNode *)> executeFunc,
    NodeCategory category,
    bool hasExecInput,
    bool hasExecOutput,
    std::vector<std::pair<std::string, std::any>> inputPins,
    std::vector<std::pair<std::string, std::any>> outputPins,
    ImVec2 position,
    ImVec2 size)
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
    if (hasExecOutput && title != "Branch" && title != "For Loop")
    {
        config.outputPins.push_back(PinConfig("", ExecuteOutput));
    }
    else if (hasExecOutput && title == "Branch" || hasExecOutput && title == "For Loop")
    {
        config.outputPins.push_back(PinConfig("", ExecuteOutput));
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
CustomNode *MNodeEngine::CreateNode(
    const std::string &title,
    std::function<void(CustomNode *)> executeFunc,
    bool hasExecInput,
    bool hasExecOutput,
    std::vector<std::pair<std::string, std::any>> inputPins,
    std::vector<std::pair<std::string, std::any>> outputPins,
    ImVec2 position,
    ImVec2 size)
{
    // Llamar a la versión principal con categoría por defecto
    return CreateNode(title, executeFunc, INPUT_OUTPUT, hasExecInput, hasExecOutput, inputPins, outputPins, position, size);
}

// Método para validar si una conexión es válida (VERSIÓN MEJORADA)
bool MNodeEngine::IsValidConnection(Node *fromNode, int fromPinId, Node *toNode, int toPinId)
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
bool MNodeEngine::WouldCreateCycle(int fromNodeId, int toNodeId)
{
    // Implementación simple: verificar si el nodo destino ya tiene una ruta hacia el nodo origen
    std::set<int> visited;
    return HasPathToNode(toNodeId, fromNodeId, visited);
}

// Método para verificar si existe una ruta desde un nodo hacia otro
bool MNodeEngine::HasPathToNode(int currentNodeId, int targetNodeId, std::set<int> &visited)
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
bool MNodeEngine::HasConnection(int nodeId, int pinId, bool isInput)
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
bool MNodeEngine::CreateConnection(int fromNodeId, int fromPinId, int toNodeId, int toPinId)
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

    // Marcar orden topológico como sucio
    topologicalOrderDirty = true;

    std::cout << "[SUCCESS] Connection created successfully!" << std::endl;

    // Actualizar inmediatamente todo el grafo de dependencias
    ForceUpdateAllNodeInputs();

    return true;
}

// Método para eliminar todas las conexiones de un nodo
void MNodeEngine::RemoveAllConnectionsFromNode(int nodeId)
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
void MNodeEngine::DeleteNode(int nodeId)
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
std::string MNodeEngine::GetConnectedValue(int nodeId, int pinId)
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
void MNodeEngine::DebugConnectionSystem()
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
bool MNodeEngine::ValidateConnectionIntegrity()
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
void MNodeEngine::CleanupInvalidConnections()
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

void MNodeEngine::ExecuteFrom(CustomNode *d)
{
    Node *node = &d->n;
    if (!node || !node->execFunc)
        return;

    // NUEVO: Antes de ejecutar cualquier nodo, asegurar que todas las dependencias de datos estén actualizadas
    EvaluateDataDependencies(d);

    node->isActive = true;
    node->execFunc(node);

    // Resto del código de ejecución se mantiene igual...
    if (node->title != "Branch" && node->title != "For Loop")
    {
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
    else if (node->title == "Branch")
    {
        // Asegurar que la condición esté evaluada antes de la ejecución
        UpdateNodeInputs(d);

        bool condition = d->GetInputValue<bool>(1, false);
        int truePinIndex = 0;
        int falsePinIndex = 1;

        for (auto &c : connections)
        {
            if (c.fromNodeId != node->id)
                continue;

            if ((condition && c.fromPinId == truePinIndex) ||
                (!condition && c.fromPinId == falsePinIndex))
            {
                CustomNode *next = GetNodeById(c.toNodeId);
                if (next && node->outputs[c.fromPinId].isExec)
                    ExecuteFrom(next);
            }
        }
    }
    else if (node->title == "For Loop")
    {
        // Asegurar que los valores de inicio y fin estén evaluados
        UpdateNodeInputs(d);

        int start = d->GetInputValue<int>(1, 0);
        int end = d->GetInputValue<int>(2, 0);

        std::vector<Connection> loopBodyConns;
        std::vector<Connection> completedConns;

        for (auto &c : connections)
        {
            if (c.fromNodeId != node->id)
                continue;
            if (c.fromPinId == 1)
                loopBodyConns.push_back(c);
            if (c.fromPinId == 0)
                completedConns.push_back(c);
        }

        for (int i = start; i < end; i++)
        {
            d->SetOutputValue<int>(2, i);

            for (auto &loopConn : loopBodyConns)
            {
                CustomNode *next = GetNodeById(loopConn.toNodeId);
                if (next && next != d)
                    ExecuteFrom(next);
            }
        }

        for (auto &compConn : completedConns)
        {
            CustomNode *next = GetNodeById(compConn.toNodeId);
            if (next && next != d)
                ExecuteFrom(next);
        }
    }

    node->isActive = false;
}

void MNodeEngine::EvaluateDataDependencies(CustomNode *node)
{
    if (!node)
        return;

    std::set<int> visited;
    EvaluateDataDependenciesRecursive(node, visited);
}

// NUEVO: Evaluación recursiva de dependencias
void MNodeEngine::EvaluateDataDependenciesRecursive(CustomNode *node, std::set<int> &visited)
{
    if (!node || visited.find(node->n.id) != visited.end())
    {
        return; // Ya procesado o nulo
    }

    visited.insert(node->n.id);

    // Primero, evaluar todas las dependencias de datos de este nodo
    for (const auto &connection : connections)
    {
        if (connection.toNodeId == node->n.id)
        {
            // Solo procesar conexiones de datos, no de ejecución
            CustomNode *sourceNode = GetCustomNodeById(connection.fromNodeId);
            if (sourceNode && connection.fromPinId < sourceNode->n.outputs.size() &&
                !sourceNode->n.outputs[connection.fromPinId].isExec)
            {
                // Evaluar recursivamente las dependencias del nodo fuente
                EvaluateDataDependenciesRecursive(sourceNode, visited);

                // Si el nodo fuente es un nodo de datos, ejecutarlo
                if (IsDataNode(*sourceNode) && sourceNode->n.execFunc)
                {
                    std::cout << "[EVAL DEPS] Executing data dependency: " << sourceNode->n.title << std::endl;
                    sourceNode->n.execFunc(&sourceNode->n);
                }

                // Propagar el valor inmediatamente
                auto outputValue = sourceNode->outputValues.find(connection.fromPinId);
                if (outputValue != sourceNode->outputValues.end())
                {
                    node->inputValues[connection.toPinId] = outputValue->second;
                    std::cout << "[EVAL DEPS] Propagated value from " << sourceNode->n.title
                              << " to " << node->n.title << std::endl;
                }
            }
        }
    }
}

void MNodeEngine::ExecuteGraph()
{
    ForceUpdateAllNodeInputs();
    for (auto &node : customNodes)
    {
        if (node.n.title == "On Start")
        {
            ExecuteFrom(&node);
            break;
        }
    }
}

void MNodeEngine::ExecuteGraphOnTick()
{
    ForceUpdateAllNodeInputs();
    for (auto &node : customNodes)
    {
        if (node.n.title == "On Tick")
        {
            ExecuteFrom(&node);
            break;
        }
    }
}

// MEJORADO: Eliminar conexiones con limpieza de orden topológico
void MNodeEngine::RemoveConnectionsFromPin(int nodeId, int pinId, bool isInput)
{
    auto it = connections.begin();
    bool removedAny = false;

    while (it != connections.end())
    {
        bool shouldRemove = false;
        if (isInput)
        {
            shouldRemove = (it->toNodeId == nodeId && it->toPinId == pinId);

            if (shouldRemove)
            {
                CustomNode *targetNode = GetCustomNodeById(nodeId);
                if (targetNode)
                {
                    auto defaultIt = targetNode->defaultValues.find(pinId);
                    if (defaultIt != targetNode->defaultValues.end())
                    {
                        targetNode->inputValues[pinId] = defaultIt->second;

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
                    }
                    else
                    {
                        targetNode->inputValues.erase(pinId);
                    }
                }
            }
        }
        else
        {
            shouldRemove = (it->fromNodeId == nodeId && it->fromPinId == pinId);
        }

        if (shouldRemove)
        {
            std::cout << "Removing connection: " << it->fromNodeId << ":" << it->fromPinId
                      << " -> " << it->toNodeId << ":" << it->toPinId << std::endl;
            it = connections.erase(it);
            removedAny = true;
        }
        else
        {
            ++it;
        }
    }

    // Marcar orden topológico como sucio si se removió alguna conexión
    if (removedAny)
    {
        topologicalOrderDirty = true;
    }
}

std::vector<int> MNodeEngine::CalculateTopologicalOrder() const
{
    std::vector<int> result;
    std::map<int, int> inDegree;
    std::map<int, std::vector<int>> adjacencyList;

    // Inicializar grados de entrada para todos los nodos
    for (const auto &node : customNodes)
    {
        inDegree[node.n.id] = 0;
        adjacencyList[node.n.id] = std::vector<int>();
    }

    // Calcular grados de entrada basándose SOLO en conexiones de datos
    for (const auto &conn : connections)
    {
        // Verificar que ambos nodos existen
        auto fromNode = std::find_if(customNodes.begin(), customNodes.end(),
                                     [&](const CustomNode &n)
                                     { return n.n.id == conn.fromNodeId; });
        auto toNode = std::find_if(customNodes.begin(), customNodes.end(),
                                   [&](const CustomNode &n)
                                   { return n.n.id == conn.toNodeId; });

        if (fromNode != customNodes.end() && toNode != customNodes.end())
        {
            // Solo considerar conexiones de datos para el orden topológico
            if (conn.fromPinId < fromNode->n.outputs.size() &&
                !fromNode->n.outputs[conn.fromPinId].isExec)
            {
                inDegree[conn.toNodeId]++;
                adjacencyList[conn.fromNodeId].push_back(conn.toNodeId);
            }
        }
    }

    // Cola para procesamiento
    std::queue<int> zeroInDegreeQueue;

    // Agregar nodos sin dependencias de datos
    for (const auto &pair : inDegree)
    {
        if (pair.second == 0)
        {
            zeroInDegreeQueue.push(pair.first);
        }
    }

    // Procesamiento topológico
    while (!zeroInDegreeQueue.empty())
    {
        int currentNode = zeroInDegreeQueue.front();
        zeroInDegreeQueue.pop();
        result.push_back(currentNode);

        // Reducir grado de entrada de nodos dependientes
        for (int neighbor : adjacencyList[currentNode])
        {
            inDegree[neighbor]--;
            if (inDegree[neighbor] == 0)
            {
                zeroInDegreeQueue.push(neighbor);
            }
        }
    }

    std::cout << "[TOPO] Calculated topological order: ";
    for (int id : result)
    {
        std::cout << id << " ";
    }
    std::cout << std::endl;

    return result;
}

CustomNode *MNodeEngine::GetNodeById(int id)
{
    for (auto &n : customNodes)
    {
        if (n.n.id == id || n.nodeId == id)
            return &n;
    }
    return nullptr;
}

CustomNode *MNodeEngine::GetCustomNodeById(int id)
{
    for (auto &cn : customNodes)
    {
        if (cn.nodeId == id || cn.n.id == id)
            return &cn;
    }
    return nullptr;
}

// Función principal para crear nodos personalizados
CustomNode *MNodeEngine::CreateCustomNode(const NodeConfig &config, ImVec2 position)
{
    CustomNode newNode;
    newNode._SelfObject = _SelfObject;
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

        // Los pins ExecuteOutput son pins de ejecución (rojos)
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
void MNodeEngine::UpdateNodeInputs(CustomNode *node)
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
void MNodeEngine::UpdateNodeInputsImmediately(int fromNodeId, int fromPinId, int toNodeId, int toPinId)
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
void MNodeEngine::ForceUpdateAllNodeInputs()
{
    // Recalcular orden topológico si es necesario
    if (topologicalOrderDirty)
    {
        topologicalOrder = CalculateTopologicalOrder();
        topologicalOrderDirty = false;
    }

    // Ejecutar nodos de datos en orden topológico
    for (int nodeId : topologicalOrder)
    {
        auto nodeIt = std::find_if(customNodes.begin(), customNodes.end(),
                                   [nodeId](const CustomNode &n)
                                   { return n.n.id == nodeId; });

        if (nodeIt != customNodes.end())
        {
            CustomNode &node = *nodeIt;

            // Actualizar valores de entrada desde conexiones
            UpdateNodeInputs(&node);

            // Si es un nodo de datos (sin pins de ejecución o con título específico), ejecutarlo
            if (IsDataNode(node) && node.n.execFunc)
            {
                node.n.execFunc(&node.n);

                // Propagar inmediatamente sus valores de salida
                PropagateNodeOutputs(&node);
            }
        }
    }
}

bool MNodeEngine::IsDataNode(const CustomNode &node) const
{
    // Un nodo es de datos si no tiene pins de ejecución de entrada
    // o si es un tipo específico que siempre debe ejecutarse
    return node.n.inputs.empty() ||
           !node.n.inputs[0].isExec ||
           node.n.title == "String" ||
           node.n.title == "Number" ||
           node.n.title == "Boolean" ||
           node.n.title == "Get Variable" ||
           node.n.title == "Math";
}

void MNodeEngine::PropagateNodeOutputs(CustomNode *node)
{
    if (!node)
        return;

    for (const auto &connection : connections)
    {
        if (connection.fromNodeId == node->n.id)
        {
            // Solo propagar datos, no ejecución
            if (connection.fromPinId < node->n.outputs.size() &&
                !node->n.outputs[connection.fromPinId].isExec)
            {
                CustomNode *targetNode = GetCustomNodeById(connection.toNodeId);
                if (targetNode)
                {
                    auto outputValue = node->outputValues.find(connection.fromPinId);
                    if (outputValue != node->outputValues.end())
                    {
                        targetNode->inputValues[connection.toPinId] = outputValue->second;
                        // std::cout << "[PROPAGATE] " << node->n.title << " -> " << targetNode->n.title
                        //           << " (pin " << connection.toPinId << ")" << std::endl;
                    }
                }
            }
        }
    }
}
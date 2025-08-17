#pragma once

#include <iostream>
#include <imgui/imgui.h>
#include <optional>

using namespace std;

class UIBuilder
{
public:
    static void DrawIcon(unsigned int texture_id, ImVec2 size_icon = ImVec2(24, 24))
    {
        ImGui::BeginGroup();

        ImGui::Image((void*)(intptr_t)texture_id, size_icon);
        ImGui::SameLine();

        ImGui::EndGroup();
        ImGui::SameLine();
    }

    static void Drag(std::string DRAG_NAME, std::string OBJECT_NAME)
    {
        if (ImGui::BeginDragDropSource())
        {
            ImGui::SetDragDropPayload(DRAG_NAME.c_str(), OBJECT_NAME.c_str(), OBJECT_NAME.size() + 1);
            ImGui::Text("%s", OBJECT_NAME.c_str());
            ImGui::EndDragDropSource();
        }
    }

    static std::optional<std::string> Drag_Objetive(std::string DRAG_NAME)
    {
        if (ImGui::BeginDragDropTarget())
        {
            ImGuiDragDropFlags target_flags = 0;
            target_flags |= ImGuiDragDropFlags_AcceptBeforeDelivery;
            target_flags |= ImGuiDragDropFlags_AcceptNoDrawDefaultRect;

            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(DRAG_NAME.c_str(), target_flags))
            {
                const char* receivedString = static_cast<const char*>(payload->Data);
                std::string convertedPath = receivedString;

                ImGui::EndDragDropTarget();
                return convertedPath;
            }

            ImGui::EndDragDropTarget();
        }

        return std::nullopt;
    }

    static std::string InputText(const std::string& Name, const std::string& value, ImVec2 size = ImVec2(-1, 30))
    {
        char GetName[128];

    #ifdef _WIN32
        strcpy_s(GetName, value.c_str()); // Windows
    #else
        strncpy(GetName, value.c_str(), sizeof(GetName)); // Linux/macOS
        GetName[sizeof(GetName) - 1] = '\0'; // asegurar terminación nula
    #endif

        ImGui::SetNextItemWidth(size.x);

        ImGui::InputTextMultiline(
            Name.c_str(),
            GetName, sizeof(GetName),
            ImVec2(size.x, size.y),
            ImGuiInputTextFlags_AutoSelectAll);

        return std::string(GetName);
    }

    static int Int(const std::string& name, int value)
    {
        ImGui::DragInt(name.c_str(), &value);
        return value;
    }

    static glm::vec2 Vector2(string Name, glm::vec2 vector)
    {
        float v[2] = {
            vector.x,
            vector.y };

        ImGui::DragFloat2(Name.c_str(), v);

        return glm::vec2(v[0], v[1]);
    }

    static bool Toggle(string Name, bool valueB)
    {
        ImGui::Checkbox(Name.c_str(), &valueB);
        return valueB;
    }

    static glm::vec3 Vector3(string Name, glm::vec3 vector)
    {
        float v[3] = {
            vector.x,
            vector.y,
            vector.z };

        ImGui::DragFloat3(Name.c_str(), v, 0.01f);

        return glm::vec3(v[0], v[1], v[2]);
    }

    static float Slider(string name, float value, float min, float max)
    {
        ImGui::SliderFloat(name.c_str(), &value, min, max);
        return value;
    }

    static float Float(const std::string& name, float value, ImVec2 size = ImVec2(-1, 30))
    {
        ImGui::DragFloat(name.c_str(), &value);
        return value;
    }

};
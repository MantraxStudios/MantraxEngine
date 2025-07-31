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

            if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(DRAG_NAME.c_str(), target_flags))
                {
                    const char* receivedString = static_cast<const char*>(payload->Data);
                    std::string convertedPath = receivedString;

                    ImGui::EndDragDropTarget();
                    return convertedPath;
                }
            }

            ImGui::EndDragDropTarget();
        }

        return std::nullopt;
    }
};
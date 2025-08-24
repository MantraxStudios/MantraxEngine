#include "EditorNode.h"
#include "components/SceneManager.h"
#include "components/GameObject.h"
#include "render/Camera.h"
#include "render/Framebuffer.h"
#include "Selection.h"
#include "../EUI/EditorInfo.h"
#include "../SceneSaver.h"

void EditorNode::OnRenderGUI()
{
    for (size_t i = 0; i < Engines_Nodes.size();)
    {
        auto *glyphEditor = Engines_Nodes[i];

        if (glyphEditor->engine == nullptr)
        {
            delete glyphEditor;
            Engines_Nodes.erase(Engines_Nodes.begin() + i);
            continue;
        }

        std::string windowName = "Glyphs##" + std::to_string(glyphEditor->editorID);

        bool open = true;
        ImGui::SetNextWindowSizeConstraints(
            ImVec2(400, 400),
            ImVec2(FLT_MAX, FLT_MAX));

        if (ImGui::Begin(windowName.c_str(), &open))
        {
            glyphEditor->isHovering = ImGui::IsWindowHovered();
            glyphEditor->Draw();
        }
        ImGui::End();

        if (!open)
        {
            // Usuario cerró la ventana → liberar memoria y quitar del vector
            delete glyphEditor;
            Engines_Nodes.erase(Engines_Nodes.begin() + i);
            continue; // no incrementamos i
        }

        i++; // sólo avanzamos si no eliminamos
    }
}

void EditorNode::OpenNewEditor(MNodeEngine *engine)
{
    for (MNodeEditor *_Node : Engines_Nodes)
    {
        if (_Node)
        {
            if (_Node->engine == engine)
            {
                return;
            }
        }
    }

    static int MakedEditorID;

    MNodeEditor *newEditor = new MNodeEditor();
    newEditor->editorID = MakedEditorID;
    newEditor->SetupNodeEditor(engine);

    Engines_Nodes.push_back(newEditor);
    MakedEditorID++;
}

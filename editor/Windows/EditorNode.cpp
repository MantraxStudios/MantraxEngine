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
    for (auto &glyphEditor : Engines_Nodes)
    {
        if (glyphEditor->engine == nullptr)
            return;

        std::string windowName = "Glyphs##" + std::to_string(glyphEditor->editorID);
        ImGui::SetNextWindowSizeConstraints(
            ImVec2(400, 400),        // tamaño mínimo (width=200, height=150)
            ImVec2(FLT_MAX, FLT_MAX) // tamaño máximo (sin límite aquí)
        );
        ImGui::Begin(windowName.c_str());
        glyphEditor->Draw();
        ImGui::End();
    }
}

void EditorNode::OpenNewEditor(MNodeEngine *engine)
{
    static int MakedEditorID;

    MNodeEditor *newEditor = new MNodeEditor();
    newEditor->editorID = MakedEditorID;
    newEditor->SetupNodeEditor(engine);

    Engines_Nodes.push_back(newEditor);
    MakedEditorID++;
}

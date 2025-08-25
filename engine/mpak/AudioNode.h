#pragma once
#include <iostream>
#include <vector>
#include <components/GameObject.h>
#include <components/AudioSource.h>
#include <components/SceneManager.h>
#include "MNodeEngine.h"
#include <imgui/imgui.h>

class AudioNode
{
public:
    void RegisterNodes(MNodeEngine &engine, ImVec2 position = ImVec2(300, 100))
    {
        PremakeNode getAudioNode(
            "Audio",
            "Get Audio",
            [](CustomNode *node)
            {
                GameObject *object = node->GetInputValue<GameObject *>(0, nullptr);

                if (object == nullptr)
                    object = node->_SelfObject;

                AudioSource *source = object->getComponent<AudioSource>();

                if (source != nullptr)
                {
                    node->SetOutputValue<AudioSource *>(0, source);
                }
            },
            SCRIPT,                              // CATEGORY
            false,                               // EXECUTE PIN INPUT
            false,                               // EXECUTE PIN OUT
            {{"Object", (GameObject *)nullptr}}, // INPUT PINS
            {{"Audio", (AudioSource *)nullptr}}, // OUTPUT PINS
            position                             // PIN POSITION
        );

        PremakeNode playSound(
            "Audio",
            "Play",
            [](CustomNode *node)
            {
                AudioSource *source = node->GetInputValue<AudioSource *>(1, nullptr);

                if (source != nullptr)
                {
                    source->play();
                }
            },
            SCRIPT,                              // CATEGORY
            true,                                // EXECUTE PIN INPUT
            true,                                // EXECUTE PIN OUT
            {{"Audio", (AudioSource *)nullptr}}, // INPUT PINS
            {},                                  // OUTPUT PINS
            position                             // PIN POSITION
        );

        PremakeNode stopSound(
            "Audio",
            "Stop",
            [](CustomNode *node)
            {
                AudioSource *source = node->GetInputValue<AudioSource *>(1, nullptr);

                if (source != nullptr)
                {
                    source->stop();
                }
            },
            SCRIPT,                              // CATEGORY
            true,                                // EXECUTE PIN INPUT
            true,                                // EXECUTE PIN OUT
            {{"Audio", (AudioSource *)nullptr}}, // INPUT PINS
            {},                                  // OUTPUT PINS
            position                             // PIN POSITION
        );

        PremakeNode pauseSound(
            "Audio",
            "Pause",
            [](CustomNode *node)
            {
                AudioSource *source = node->GetInputValue<AudioSource *>(1, nullptr);

                if (source != nullptr)
                {
                    source->pause();

                    node->SetOutputValue<bool>(1, source->isPaused());
                }
            },
            SCRIPT,                              // CATEGORY
            true,                                // EXECUTE PIN INPUT
            true,                                // EXECUTE PIN OUT
            {{"Audio", (AudioSource *)nullptr}}, // INPUT PINS
            {{"Is Paused", false}},              // OUTPUT PINS
            position                             // PIN POSITION
        );

        PremakeNode resumeSound(
            "Audio",
            "Resume",
            [](CustomNode *node)
            {
                AudioSource *source = node->GetInputValue<AudioSource *>(1, nullptr);

                if (source != nullptr)
                {
                    source->resume();
                }
            },
            SCRIPT,                              // CATEGORY
            true,                                // EXECUTE PIN INPUT
            true,                                // EXECUTE PIN OUT
            {{"Audio", (AudioSource *)nullptr}}, // INPUT PINS
            {},                                  // OUTPUT PINS
            position                             // PIN POSITION
        );

        PremakeNode setVolumeNode(
            "Audio",
            "Set Volumen",
            [](CustomNode *node)
            {
                AudioSource *source = node->GetInputValue<AudioSource *>(1, nullptr);

                if (source != nullptr)
                {
                    source->setVolume(node->GetInputValue<float>(2, 1.0f));
                }
            },
            SCRIPT,                                                // CATEGORY
            true,                                                  // EXECUTE PIN INPUT
            true,                                                  // EXECUTE PIN OUT
            {{"Audio", (AudioSource *)nullptr}, {"Volume", 1.0f}}, // INPUT PINS
            {},                                                    // OUTPUT PINS
            position                                               // PIN POSITION
        );

        PremakeNode setSpatialSound(
            "Audio",
            "Set Spatial",
            [](CustomNode *node)
            {
                AudioSource *source = node->GetInputValue<AudioSource *>(1, nullptr);

                if (source != nullptr)
                {
                    source->set3DAttributes(node->GetInputValue<bool>(2, true));
                }
            },
            SCRIPT,                                                 // CATEGORY
            true,                                                   // EXECUTE PIN INPUT
            true,                                                   // EXECUTE PIN OUT
            {{"Audio", (AudioSource *)nullptr}, {"Spatial", true}}, // INPUT PINS
            {},                                                     // OUTPUT PINS
            position                                                // PIN POSITION
        );

        PremakeNode setSpatialRange(
            "Audio",
            "Set Spatial Range",
            [](CustomNode *node)
            {
                AudioSource *source = node->GetInputValue<AudioSource *>(1, nullptr);

                if (source != nullptr)
                {
                    glm::vec2 sptValues = node->GetInputValue<glm::vec2>(2, glm::vec2(1.0f));
                    source->setMinDistance(sptValues.x);
                    source->setMaxDistance(sptValues.y);
                }
            },
            SCRIPT,                                                                   // CATEGORY
            true,                                                                     // EXECUTE PIN INPUT
            true,                                                                     // EXECUTE PIN OUT
            {{"Audio", (AudioSource *)nullptr}, {"Spatial", glm::vec2(5.0f, 10.0f)}}, // INPUT PINS
            {},                                                                       // OUTPUT PINS
            position                                                                  // PIN POSITION
        );

        PremakeNode getSpatialRange(
            "Audio",
            "Get Spatial Range",
            [](CustomNode *node)
            {
                AudioSource *source = node->GetInputValue<AudioSource *>(0, nullptr);

                if (source != nullptr)
                {
                    node->SetOutputValue<glm::vec2>(0, glm::vec2(source->getMinDistance(), source->getMaxDistance()));
                }
            },
            SCRIPT,                                // CATEGORY
            false,                                 // EXECUTE PIN INPUT
            false,                                 // EXECUTE PIN OUT
            {{"Audio", (AudioSource *)nullptr}},   // INPUT PINS
            {{"Spatial", glm::vec2(5.0f, 10.0f)}}, // OUTPUT PINS
            position                               // PIN POSITION
        );

        PremakeNode isSpatialSound(
            "Audio",
            "Is Spatial Sound",
            [](CustomNode *node)
            {
                AudioSource *source = node->GetInputValue<AudioSource *>(0, nullptr);

                if (source != nullptr)
                {
                    node->SetOutputValue<bool>(0, source->is3DEnabled());
                }
            },
            SCRIPT,                              // CATEGORY
            false,                               // EXECUTE PIN INPUT
            false,                               // EXECUTE PIN OUT
            {{"Audio", (AudioSource *)nullptr}}, // INPUT PINS
            {{"Is Spatial", true}},              // OUTPUT PINS
            position                             // PIN POSITION
        );

        PremakeNode isPlayingNode(
            "Audio",
            "Is Playing",
            [](CustomNode *node)
            {
                AudioSource *source = node->GetInputValue<AudioSource *>(0);

                if (source != nullptr)
                {
                    node->SetOutputValue<bool>(0, source->isPlaying());
                }
            },
            SCRIPT,                              // CATEGORY
            false,                               // EXECUTE PIN INPUT
            false,                               // EXECUTE PIN OUT
            {{"Audio", (AudioSource *)nullptr}}, // INPUT PINS
            {{"Is Playing", false}},             // OUTPUT PINS
            position                             // PIN POSITION
        );

        engine.PrefabNodes.push_back(getAudioNode);
        engine.PrefabNodes.push_back(playSound);
        engine.PrefabNodes.push_back(stopSound);
        engine.PrefabNodes.push_back(pauseSound);
        engine.PrefabNodes.push_back(resumeSound);
        engine.PrefabNodes.push_back(setVolumeNode);
        engine.PrefabNodes.push_back(setSpatialSound);
        engine.PrefabNodes.push_back(setSpatialRange);
        engine.PrefabNodes.push_back(getSpatialRange);
        engine.PrefabNodes.push_back(isSpatialSound);
        engine.PrefabNodes.push_back(isPlayingNode);
    }
};
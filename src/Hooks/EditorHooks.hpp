#pragma once

#include <Geode/Geode.hpp>
#include <cocos2d.h>

namespace CoEngine {

    class EditorHookManager {
    private:
        bool m_uiInjected = false;
        
        EditorHookManager() = default;

    public:
        ~EditorHookManager() = default;

        // Singleton Accessor
        static EditorHookManager* get();

        /**
         * Intercepts the instantiation of the editor layer.
         * Used to set up initial environmental variables and execute theme applications.
         */
        void handleEditorInit(geode::LevelEditorLayer* editorLayer);

        /**
         * Hooks directly into the delta-time update cycle of the editor.
         * Drives asynchronous object batch generation without dropping game frames.
         */
        void handleEditorUpdate(float deltaTime);

        /**
         * Intercepts the destruction sequence of the level editor layer.
         * Performs memory flushes and tracking boundary resets.
         */
        void handleEditorShutdown();

        /**
         * Safely injects custom engine buttons, panels, and trigger icons
         * directly into the Geometry Dash editor UI overlay.
         */
        void handleUISetup(geode::EditorUI* editorUI);
        
        // Target trigger method wired to the injected interface button
        void onEngineMenuTrigger(cocos2d::CCObject* sender);
    };
}

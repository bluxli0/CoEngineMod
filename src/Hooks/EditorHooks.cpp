#include "EditorHooks.hpp"
#include <UI/InteractionMediator.hpp>
#include <Styles/StylePalette.hpp>
#include <Core/TaskManager.hpp>
#include <Core/EngineCommandBus.hpp>

// Include Geode's modifier infrastructure headers explicitly
#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/modify/EditorUI.hpp>

using namespace geode::prelude;

namespace CoEngine {

    EditorHookManager* EditorHookManager::get() {
        static EditorHookManager instance;
        return &instance;
    }

    void EditorHookManager::handleEditorInit(LevelEditorLayer* editorLayer) {
        CO_LOG_INFO("EditorHookManager: LevelEditorLayer context successfully captured.");
        
        // Automatically inject and apply current style color sheets on level entry
        StylePalette::get()->applyPaletteToCurrentLevel();
        
        m_uiInjected = false;
    }

    void EditorHookManager::handleEditorUpdate(float deltaTime) {
        // Crucial component loop connection:
        // Ticks the async queue runner every single frame to step object placement
        TaskManager::get()->schedulerTick(deltaTime);
    }

    void EditorHookManager::handleEditorShutdown() {
        CO_LOG_INFO("EditorHookManager: Cleaning layout tracks on editor exit.");
        
        // Flush memory allocations and clear queues to prevent dangling thread handles
        TaskManager::get()->flushQueue();
    }

    void EditorHookManager::handleUISetup(EditorUI* editorUI) {
        if (m_uiInjected) return;
        
        CO_LOG_INFO("EditorHookManager: Injecting custom engine interface nodes...");

        // Safely acquire an existing editor control group layout (e.g., the right sidebar menu)
        auto editorMenu = editorUI->getChildByID("undo-menu");
        if (!editorMenu) {
            // Fall back to target layout arrays safely if custom nodes fail to resolve
            editorMenu = editorUI->m_playtestMenu;
        }

        if (!editorMenu) {
            CO_LOG_ERROR("Failed to locate an anchor menu to inject custom engine buttons into.");
            return;
        }

        // Create a custom engine menu item using standard Cocos2d textures
        auto spriteIcon = CCSprite::createWithSpriteFrameName("GJ_plusBtn_001.png"); // Placeholder plus layout asset
        if (spriteIcon) {
            // Tint the button uniquely to differentiate your engine visually from standard editor keys
            spriteIcon->setColor({ 0, 240, 255 }); 
            
            auto customBtn = CCMenuItemSpriteExtra::create(
                spriteIcon,
                editorUI,
                menu_selector(EditorHookManager::onEngineMenuTrigger)
            );
            
            customBtn->setID("coengine-trigger-button");
            
            // Append the component onto the menu system array grid safely
            editorMenu->addChild(customBtn);
            editorMenu->updateLayout();
        }

        m_uiInjected = true;
    }

    void EditorHookManager::onEngineMenuTrigger(CCObject* sender) {
        CO_LOG_INFO("Engine action button pressed. Dispatched event context to command bus.");
        
        // Create context mappings capturing current editor limits bounds
        CommandContext context;
        context.startX = 0.0f;
        context.endX = 5000.0f;
        context.targetGamemode = 0; // Baseline cube configuration tracking parameter
        
        // Safely dispatch a fire-and-forget payload over the global bus channel
        EngineCommandBus::get()->publish(CommandID::TRIGGER_GENERATION, context);
    }
}

// ============================================================================
// GEODE HOOK INJECTIONS
// ============================================================================

class $modify(CoLevelEditorLayer, LevelEditorLayer) {
    bool init(GJGameLevel* level, bool p1) {
        if (!LevelEditorLayer::init(level, p1)) {
            return false;
        }
        
        // Delegate setup tasks out to our architectural manager
        CoEngine::EditorHookManager::get()->handleEditorInit(this);
        
        // Standard scheduler registration hook to safely enable custom frame-updates
        this->scheduleUpdate();
        return true;
    }

    void update(float dt) {
        LevelEditorLayer::update(dt);
        CoEngine::EditorHookManager::get()->handleEditorUpdate(dt);
    }

    void destructor() {
        CoEngine::EditorHookManager::get()->handleEditorShutdown();
        LevelEditorLayer::~LevelEditorLayer();
    }
};

class $modify(CoEditorUI, EditorUI) {
    bool init(LevelEditorLayer* editorLayer) {
        if (!EditorUI::init(editorLayer)) {
            return false;
        }
        
        // Pass instance hooks downwards right after base interface construction processes finish
        CoEngine::EditorHookManager::get()->handleUISetup(this);
        return true;
    }
};

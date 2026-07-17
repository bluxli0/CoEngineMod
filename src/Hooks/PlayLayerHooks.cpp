#include "PlayLayerHooks.hpp"
#include <Core/EngineCommandBus.hpp>
#include <Core/TaskManager.hpp>

// Include Geode's macro hooks injection systems
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

namespace CoEngine {

    PlayLayerHookManager* PlayLayerHookManager::get() {
        static PlayLayerHookManager instance;
        return &instance;
    }

    void PlayLayerHookManager::handlePlayLayerInit(PlayLayer* playLayer) {
        if (!playLayer) return;

        CO_LOG_INFO("PlayLayerHookManager: Live gameplay canvas context verified.");
        m_isLiveGameplayActive = true;

        // Signal the command bus to prepare runtime resources for live playback
        CommandContext initContext;
        if (playLayer->m_level) {
            // Log target info using Geode's standard string formatting if needed
            CO_LOG_INFO("Loading Level: %s", playLayer->m_level->m_levelName.c_str());
        }
        
        EngineCommandBus::get()->publish(CommandID::INITIALIZE_PLAY_SESSION, initContext);
    }

    void PlayLayerHookManager::handlePlayLayerUpdate(float deltaTime) {
        if (!m_isLiveGameplayActive) return;

        // Drive minor runtime physics mutations or local procedural adjustments here
        // Note: Chunk streaming commands are generally broadcast via PlayerHooks, 
        // but global timers or score/UI updates live safely here.
    }

    void PlayLayerHookManager::handleLevelCompleted() {
        CO_LOG_INFO("PlayLayerHookManager: Level completion criteria satisfied.");

        CommandContext winContext;
        EngineCommandBus::get()->publish(CommandID::RECORD_SESSION_METRICS, winContext);
    }

    void PlayLayerHookManager::handlePlayLayerDestroy() {
        if (!m_isLiveGameplayActive) return;
        m_isLiveGameplayActive = false;

        CO_LOG_INFO("PlayLayerHookManager: Terminating play session. Tearing down asset matrices.");

        // Clean up memory structures instantly so data doesn't bleed into the next level load
        TaskManager::get()->flushQueue();
        
        CommandContext teardownContext;
        EngineCommandBus::get()->publish(CommandID::TERMINATE_PLAY_SESSION, teardownContext);
    }
}

// ============================================================================
// GEODE HOOK INJECTIONS
// ============================================================================

class $modify(CoPlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontRunActions) {
        if (!PlayLayer::init(level, useReplay, dontRunActions)) {
            return false;
        }

        // Delegate to our framework manager
        CoEngine::PlayLayerHookManager::get()->handlePlayLayerInit(this);
        return true;
    }

    void update(float dt) {
        PlayLayer::update(dt);
        CoEngine::PlayLayerHookManager::get()->handlePlayLayerUpdate(dt);
    }

    void levelComplete() {
        PlayLayer::levelComplete();
        CoEngine::PlayLayerHookManager::get()->handleLevelCompleted();
    }

    void onQuit() {
        PlayLayer::onQuit();
        CoEngine::PlayLayerHookManager::get()->handlePlayLayerDestroy();
    }

    // Guard safety mapping: capture unexpected exits or destructions
    ~CoPlayLayer() {
        CoEngine::PlayLayerHookManager::get()->handlePlayLayerDestroy();
    }
};

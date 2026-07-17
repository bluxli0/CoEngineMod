#include "InteractionMediator.hpp"
#include <Core/EngineCommandBus.hpp>
#include <Core/Generator.hpp>
#include <Core/TaskManager.hpp>
#include <Core/SyncEngine.hpp>
#include <Core/UndoRedoManager.hpp>

using namespace geode::prelude;

namespace CoEngine {

    InteractionMediator* InteractionMediator::get() {
        static InteractionMediator instance;
        return &instance;
    }

    bool InteractionMediator::initialize() {
        if (m_isInitialized) return true;

        CO_LOG_INFO("Initializing UI/Core Interaction Mediator orchestrator...");

        // Anchor lambda tracking callbacks to resolve bus communications cleanly
        EngineCommandBus::get()->subscribe(CommandID::TRIGGER_GENERATION, [this](const CommandContext& ctx) {
            this->handleGenerationRequest(ctx);
        });

        EngineCommandBus::get()->subscribe(CommandID::CANCEL_GENERATION, [this](const CommandContext& ctx) {
            this->handleCancelRequest(ctx);
        });

        EngineCommandBus::get()->subscribe(CommandID::UPDATE_SETTINGS, [this](const CommandContext& ctx) {
            this->handleSettingsUpdate(ctx);
        });

        m_isInitialized = true;
        return true;
    }

    void InteractionMediator::shutdown() {
        m_isInitialized = false;
        CO_LOG_INFO("Interaction Mediator offline. Decoupling routing pipelines.");
    }

    void InteractionMediator::handleGenerationRequest(const CommandContext& context) {
        CO_LOG_INFO("Mediator intercepted TRIGGER_GENERATION event. Beginning coordination sequence...");

        // 1. Verify and resolve active Geometry Dash LevelEditorLayer instance handles
        auto editor = geode::LevelEditorLayer::get();
        if (!editor) {
            CO_LOG_ERROR("Orchestration Failure: Active LevelEditorLayer context context window not found.");
            return;
        }

        // 2. Read live parameter state values dynamically using Geode modifier schemas
        GenerationSettings runtimeSettings;
        runtimeSettings.bpm = static_cast<float>(Mod::get()->getSettingValue<double>("bpm"));
        runtimeSettings.usePlayerClicks = Mod::get()->getSettingValue<bool>("use-player-clicks");
        runtimeSettings.performanceBudget = static_cast<int>(Mod::get()->getSettingValue<int64_t>("performance-budget"));
        
        // 3. Feed context configuration overrides if custom values are passed in the payload
        if (context.startX < context.endX) {
            runtimeSettings.bpm = (context.settingsPayload.bpm > 0.0f) ? context.settingsPayload.bpm : runtimeSettings.bpm;
        }

        // 4. Synchronize state definitions evenly across the backend systems
        SyncEngine::get()->configureAudioSync(runtimeSettings.bpm, 0); // Default to 1.0x speed calculations
        Generator::get()->updateSettings(runtimeSettings);
        TaskManager::get()->setFrameBudget(runtimeSettings.performanceBudget);

        // 5. Open a single historical transaction boundary mapping block
        UndoRedoManager::get()->beginTransaction("Procedural Layout Spawning Step");

        // 6. Run abstract vector generation via the Generator math algorithms
        float buildStart = context.startX;
        float buildEnd = (context.endX > buildStart) ? context.endX : buildStart + 3000.0f;
        int activeGamemode = context.targetGamemode;

        CO_LOG_INFO("Mediator invoking structural layout synthesis from X: %.2f to X: %.2f", buildStart, buildEnd);
        const auto& skeletonArray = Generator::get()->generateSkeleton(buildStart, buildEnd, activeGamemode);

        if (skeletonArray.empty()) {
            CO_LOG_WARN("Algorithmic compilation returned an empty layout map. Aborting transaction state.");
            UndoRedoManager::get()->clearHistory();
            return;
        }

        // 7. Stream the generated dataset nodes array right into the async TaskManager queue
        TaskManager::get()->submitGenerationTask(skeletonArray, editor);
        
        // Note: To capture item pointers for tracking, TaskManager calls 
        // UndoRedoManager::get()->recordObjectCreation(gameObject) inside its loop tick.
        // Once the queue finishes completely, it invokes endTransaction() directly.
    }

    void InteractionMediator::handleCancelRequest(const CommandContext& context) {
        CO_LOG_WARN("Mediator intercepting emergency system cancel flag. Clearing active pipelines.");
        TaskManager::get()->flushQueue();
        UndoRedoManager::get()->clearHistory();
    }

    void InteractionMediator::handleSettingsUpdate(const CommandContext& context) {
        CO_LOG_INFO("Mediator re-routing global setting package parameters down to core variables.");
        Generator::get()->updateSettings(context.settingsPayload);
        TaskManager::get()->setFrameBudget(context.settingsPayload.performanceBudget);
    }
}

#pragma once

#include <Geode/Geode.hpp>
#include <CoEngineCommon.hpp>

namespace CoEngine {

    class InteractionMediator {
    private:
        bool m_isInitialized = false;

        // Command processing router targets
        void handleGenerationRequest(const CommandContext& context);
        void handleCancelRequest(const CommandContext& context);
        void handleSettingsUpdate(const CommandContext& context);

    public:
        InteractionMediator() = default;
        ~InteractionMediator() = default;

        // Singleton implementation aligned with Core subsystems
        static InteractionMediator* get();

        /**
         * Binds the internal orchestrator slots to the global EngineCommandBus hooks.
         * This must be called inside your main entry execution block on startup.
         */
        bool initialize();
        
        /**
         * Safely detaches the system listeners during engine teardown sequences.
         */
        void shutdown();

        // Inline configuration accessor
        bool isInitialized() const { return m_isInitialized; }
    };
}

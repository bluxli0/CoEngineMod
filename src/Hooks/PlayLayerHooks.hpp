#pragma once

#include <Geode/Geode.hpp>

namespace CoEngine {

    class PlayLayerHookManager {
    private:
        bool m_isLiveGameplayActive = false;
        
        PlayLayerHookManager() = default;

    public:
        ~PlayLayerHookManager() = default;

        // Singleton Accessor
        static PlayLayerHookManager* get();

        /**
         * Intercepts standard gameplay initialization.
         * Sets up state machines for procedural generation streaming when a level is played normally.
         */
        void handlePlayLayerInit(geode::PlayLayer* playLayer);

        /**
         * Monitored delta-time update step tied directly to active play states.
         */
        void handlePlayLayerUpdate(float deltaTime);

        /**
         * Fired when the player successfully reaches the end of the coordinate tracks.
         * Triggers telemetry recording and session save states.
         */
        void handleLevelCompleted();

        /**
         * Cleans up running game loops and state trackers when exiting back to menu layers.
         */
        void handlePlayLayerDestroy();

        // Queries
        bool isLiveGameplayActive() const { return m_isLiveGameplayActive; }
    };
}

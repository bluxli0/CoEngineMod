#pragma once

#include <Geode/Geode.hpp>

namespace CoEngine {

    class PlayerHookManager {
    private:
        float m_lastTrackedX = 0.0f;
        float m_currentVelocityX = 0.0f;
        bool m_isDead = false;

        PlayerHookManager() = default;

    public:
        ~PlayerHookManager() = default;

        // Singleton Accessor
        static PlayerHookManager* get();

        /**
         * Intercepts the player physics processing step.
         * Drives live metrics calculation and coordinates real-time procedural chunk loading.
         */
        void handlePlayerUpdate(geode::PlayerObject* player, float deltaTime);

        /**
         * Intercepts interaction events when the player collides with an object.
         * Useful for listening to custom structural triggers or monitoring game mode changes.
         */
        void handlePlayerCollision(geode::PlayerObject* player, geode::GameObject* gameObject);

        /**
         * Fired when the player crashes or initiates a hard reset frame.
         */
        void handlePlayerDeath(geode::PlayerObject* player);

        /**
         * Fired during level resets or checkpoint rollbacks.
         * Restores internal tracking metrics to match the restart coordinate state.
         */
        void handlePlayerReset(geode::PlayerObject* player);

        // State Queries
        float getTrackedX() const { return m_lastTrackedX; }
        float getVelocityX() const { return m_currentVelocityX; }
        bool isPlayerDead() const { return m_isDead; }
    };
}

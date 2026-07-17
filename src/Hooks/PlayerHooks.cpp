#include "PlayerHooks.hpp"
#include <Core/EngineCommandBus.hpp>
#include <Core/TaskManager.hpp>

// Include Geode's modifier tracking components explicitly
#include <Geode/modify/PlayerObject.hpp>

using namespace geode::prelude;

namespace CoEngine {

    PlayerHookManager* PlayerHookManager::get() {
        static PlayerHookManager instance;
        return &instance;
    }

    void PlayerHookManager::handlePlayerUpdate(PlayerObject* player, float deltaTime) {
        if (!player) return;

        // Extract the exact current horizontal position
        float currentX = player->m_position.x;

        // Compute actual running horizontal speed (units per second)
        if (deltaTime > 0.0f) {
            m_currentVelocityX = (currentX - m_lastTrackedX) / deltaTime;
        }

        m_lastTrackedX = currentX;

        // --- Procedural Generation Hook ---
        // If the player approaches a new layout sector boundary, we dispatch an async payload
        // to dynamically load the next chunk before it enters the gameplay viewport.
        static float lastChunkBoundary = 0.0f;
        if (std::abs(currentX - lastChunkBoundary) > 1200.0f) { // 1200 units is roughly ~40 editor blocks
            lastChunkBoundary = currentX;

            CommandContext chunkContext;
            chunkContext.startX = currentX + 1500.0f; // Generate 1500 units ahead of the player
            chunkContext.endX = currentX + 3000.0f;
            chunkContext.targetGamemode = static_cast<int>(player->m_isShip); // Sample state configuration

            EngineCommandBus::get()->publish(CommandID::STREAM_LOAD_CHUNK, chunkContext);
        }
    }

    void PlayerHookManager::handlePlayerCollision(PlayerObject* player, GameObject* gameObject) {
        if (!player || !gameObject) return;

        // Check for specific interactive object definitions via their base integer types
        int objectID = gameObject->m_objectID;

        // Optional logic tracking: Intercept custom or unique interaction profiles
        // (e.g., catching structural mod triggers before standard physics code triggers)
        if (objectID == 36) { // Yellow Jump Orb
            // Core telemetry logging or verification hook can go here if tracking jumps
        }
    }

    void PlayerHookManager::handlePlayerDeath(PlayerObject* player) {
        if (m_isDead) return;
        m_isDead = true;

        CO_LOG_INFO("PlayerHookManager: Player crash state verified.");
        
        // Notify tracking components to temporarily freeze active runtime scripts
        CommandContext freezeContext;
        EngineCommandBus::get()->publish(CommandID::FREEZE_RUNTIME_ENGINES, freezeContext);
    }

    void PlayerHookManager::handlePlayerReset(PlayerObject* player) {
        m_isDead = false;
        if (player) {
            m_lastTrackedX = player->m_position.x;
        }
        m_currentVelocityX = 0.0f;

        CO_LOG_INFO("PlayerHookManager: Reset completed. Syncing position to: %.2f", m_lastTrackedX);

        // Purge structural objects out of active volatile preview buffers on reset
        TaskManager::get()->flushQueue();
    }
}

// ============================================================================
// GEODE HOOK INJECTIONS
// ============================================================================

class $modify(CoPlayerObject, PlayerObject) {
    void update(float dt) {
        PlayerObject::update(dt);
        
        // Route physics processing frame downstream to our centralized runner
        CoEngine::PlayerHookManager::get()->handlePlayerUpdate(this, dt);
    }

    void collidedWithObject(GameObject* obj) {
        PlayerObject::collidedWithObject(obj);
        CoEngine::PlayerHookManager::get()->handlePlayerCollision(this, obj);
    }

    void playerDestroyed(bool p0) {
        PlayerObject::playerDestroyed(p0);
        CoEngine::PlayerHookManager::get()->handlePlayerDeath(this);
    }

    void resetObject() {
        PlayerObject::resetObject();
        CoEngine::PlayerHookManager::get()->handlePlayerReset(this);
    }
};

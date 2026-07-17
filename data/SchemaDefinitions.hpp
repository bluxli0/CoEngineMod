#pragma once

#include <string>
#include <vector>
#include <cocos2d.h>

namespace CoEngine {

    /**
     * Unique routing identifiers for the decoupled EngineCommandBus pub/sub highway.
     */
    enum class CommandID : uint32_t {
        STREAM_LOAD_CHUNK = 0,     // Fired by physics capturing loops to render layout ahead
        FREEZE_RUNTIME_ENGINES,    // Fired during sudden player crash/death states
        INITIALIZE_PLAY_SESSION,   // Triggered when entering PlayLayer context
        TERMINATE_PLAY_SESSION,    // Triggered on exiting back to editor/menus
        RECORD_SESSION_METRICS,    // Dispatched on level completion runs
        UPDATE_DASHBOARD_UI,       // Pushes real-time throughput metrics to HUD
        PUSH_UNDO_TRANSACTION      // Logged when mutations occur in the level matrix
    };

    /**
     * Universal execution context payload.
     * Packages various primitive and structural data states into a single vehicle
     * to keep core algorithms isolated from the UI layer.
     */
    struct CommandContext {
        float startX = 0.0f;
        float endX = 0.0f;
        int32_t targetGamemode = 0;   // Maps to native GD gamemode indexes (Cube, Ship, etc.)
        int32_t targetObjectID = 0;   // Native item recipe ID reference
        std::string payloadString = "";
        bool flagValue = false;
        cocos2d::CCPoint genericPosition = {0.0f, 0.0f};
    };

    /**
     * Blueprint layout data for a single procedurally generated element.
     */
    struct GeneratedObjectRecipe {
        int32_t objectID = 1;         // Native Geometry Dash item lookup ID
        float posX = 0.0f;
        float posY = 0.0f;
        float rotation = 0.0f;
        float scale = 1.0f;
        bool isFlippedX = false;
        bool isFlippedY = false;
        std::vector<int> editorGroups; // Assigned Z-order/Trigger action groups
    };

    /**
     * Defines chunk boundaries for the async generation loops.
     */
    struct ChunkGridBounds {
        int32_t chunkIndex = 0;
        float worldStartX = 0.0f;
        float worldEndX = 0.0f;
        bool isLoaded = false;
    };
}

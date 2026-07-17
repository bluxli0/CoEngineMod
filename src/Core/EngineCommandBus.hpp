#pragma once

#include <Geode/Geode.hpp>
#include <CoEngineCommon.hpp>
#include <functional>
#include <map>
#include <vector>
#include <string>

namespace CoEngine {

    // Unique identifies for decoupled engine actions
    enum class CommandID {
        TRIGGER_GENERATION,  // Dispatched by UI to spin up a build
        CANCEL_GENERATION,   // Halts TaskManager immediately
        UPDATE_SETTINGS,     // Syncs dashboard adjustments across systems
        APPLY_STYLE_SCHEMA,  // Flushes layout textures and re-maps style IDs
        TRIGGER_UNDO,        // Calls transaction reversion
        TRIGGER_REDO         // Calls transaction progression
    };

    // A unified data packet passed alongside events without heap reallocations
    struct CommandContext {
        float startX = 0.0f;
        float endX = 0.0f;
        int targetGamemode = 0;
        std::string payloadString = "";
        GenerationSettings settingsPayload;
    };

    // Define the type signature for asynchronous system subscribers
    using CommandCallback = std::function<void(const CommandContext&)>;

    class EngineCommandBus {
    private:
        std::map<CommandID, std::vector<CommandCallback>> m_registry;

    public:
        EngineCommandBus() = default;
        ~EngineCommandBus() = default;

        // Static singleton accessor 
        static EngineCommandBus* get();

        bool initialize();
        
        /**
         * Register an engine system listener. 
         * Core modules use this to anchor their execution loops to the bus.
         */
        void subscribe(CommandID command, CommandCallback callback);
        
        /**
         * Broadcast an event command down into all registered systems.
         * Safe to invoke from any background or frame-tick pipeline thread.
         */
        void dispatch(CommandID command, const CommandContext& context);
        
        /**
         * Completely flushes the observer dictionary to handle mod teardowns 
         * or editor state closures cleanly.
         */
        void clearAllSubscribers();
    };
}

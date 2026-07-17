#include "EngineCommandBus.hpp"

namespace CoEngine {

    EngineCommandBus* EngineCommandBus::get() {
        static EngineCommandBus instance;
        return &instance;
    }

    bool EngineCommandBus::initialize() {
        CO_LOG_INFO("Initializing Centralized EngineCommandBus System Base...");
        clearAllSubscribers();
        return true;
    }

    void EngineCommandBus::subscribe(CommandID command, CommandCallback callback) {
        if (!callback) {
            CO_LOG_WARN("Attempted to bind an empty/null callback interface to the message hub.");
            return;
        }
        
        m_registry[command].push_back(callback);
    }

    void EngineCommandBus::dispatch(CommandID command, const CommandContext& context) {
        auto location = m_registry.find(command);
        
        // If no background system has registered an anchor to this event, exit silently
        if (location == m_registry.end()) {
            return;
        }

        // Run through all registered subsystem endpoints instantly
        for (const auto& processCallback : location->second) {
            if (processCallback) {
                try {
                    processCallback(context);
                } 
                catch (const std::exception& error) {
                    CO_LOG_ERROR("Fatal breakdown caught inside CommandBus processing pipeline stream: %s", error.what());
                }
            }
        }
    }

    void EngineCommandBus::clearAllSubscribers() {
        m_registry.clear();
        CO_LOG_INFO("EngineCommandBus subscription maps successfully dropped.");
    }
}

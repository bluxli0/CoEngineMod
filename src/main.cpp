#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <CoEngineCommon.hpp>
#include <Core/Generator.hpp>
#include <Core/EngineCommandBus.hpp>
#include <Core/UndoRedoManager.hpp>

using namespace geode::prelude;

/**
 * Early-stage execution block.
 * This runs the moment the Geode loader injects the Co Engine DLL/dylib into the game process.
 * We use this to spin up core logic systems before the UI or graphics layers attempt to load.
 */
$execute {
    CO_LOG_INFO("Loading Co Engine mod context safely...");

    // Initialize Core Logic Singletons
    if (!CoEngine::Generator::get()->initialize()) {
        CO_LOG_ERROR("Failed to initialize Generator Core system!");
    } else {
        CO_LOG_INFO("Generator Core successfully online.");
    }

    // You can initialize other early subsystem singletons here as they are built:
    // CoEngine::EngineCommandBus::get()->initialize();
    // CoEngine::UndoRedoManager::get()->initialize();

    CO_LOG_INFO("Co Engine core runtime components fully loaded into memory.");
}

/**
 * Mod lifecycle event hooks.
 * This class handles standard game-level notifications, checking for updates, 
 * and preparing custom configurations on startup.
 */
class $modify(MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) {
            return false;
        }

        static bool primaryBootSequenceRun = false;
        if (!primaryBootSequenceRun) {
            CO_LOG_INFO("Geometry Dash main menu loaded. Verifying Co Engine integrity...");

            // Pull initial values from Geode's saved settings schema
            auto bpmSetting = Mod::get()->getSettingValue<double>("bpm");
            auto intensitySetting = Mod::get()->getSettingValue<double>("performance-budget");

            CO_LOG_INFO("Saved User Profile Config loaded -> Initial Setup BPM: %.2f", bpmSetting);
            
            // Toggle boot flag so this diagnostic warning block only runs once per launch
            primaryBootSequenceRun = true;
        }

        return true;
    }
};

#pragma once

#include "CoEngineCommon.hpp"

// Forward declarations to decouple dependencies from data schemas
namespace CoEngine {
    enum class CommandID;
    struct CommandContext;
    struct GeneratedObjectRecipe;
}

namespace CoEngine {

    /**
     * Contract interface for lifecycle-managed core systems.
     * Enforced on Generator, TaskManager, SyncEngine, and UndoRedoManager.
     */
    class IEngineService {
    public:
        virtual ~IEngineService() = default;

        /**
         * Invoked during the primary $execute boot block.
         * Must allocate resources and return true if baseline requirements are satisfied.
         */
        virtual bool initialize() = 0;

        /**
         * Invoked during mod teardown or context reset sequences to prevent memory bleeding.
         */
        virtual void shutdown() = 0;
    };

    /**
     * Contract interface for event dispatch orchestration.
     * Implemented by any component that listens to the centralized EngineCommandBus wire.
     */
    class ICommandSubscriber {
    public:
        virtual ~ICommandSubscriber() = default;

        /**
         * Fired by the EngineCommandBus whenever an observed event matches registration keys.
         */
        virtual void onCommandExecuted(CommandID command, const CommandContext& context) = 0;
    };

    /**
     * Contract interface for the Undo/Redo historical transaction tracking stack.
     */
    class IUndoableAction {
    public:
        virtual ~IUndoableAction() = default;

        /**
         * Applies the mutation changes onto the live LevelEditorLayer map layer.
         */
        virtual void execute() = 0;

        /**
         * Completely reverses the structural changes, restoring precise original object indices.
         */
        virtual void undo() = 0;

        /**
         * Debug footprint tracking descriptor for history logs.
         */
        virtual std::string getActionDescription() const = 0;
    };

    /**
     * Contract interface for asset rendering configurations.
     * Enforced across Style structures to stream uniform customization values.
     */
    class IStyleProvider {
    public:
        virtual ~IStyleProvider() = default;

        /**
         * Requests the explicit Geometry Dash object ID based on recipe types and noise thresholds.
         */
        virtual int resolveObjectID(const std::string& recipeType, float noiseValue) = 0;

        /**
         * Fetches structural color parameters mapped across LevelSettings tracks.
         */
        virtual cocos2d::ccColor3B getPaletteColor(int colorChannelID) = 0;
    };
}

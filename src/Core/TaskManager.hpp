#pragma once

#include <Geode/Geode.hpp>
#include <CoEngineCommon.hpp>
#include <Core/Generator.hpp>
#include <queue>
#include <vector>

namespace CoEngine {

    class TaskManager {
    private:
        std::queue<GameplayNode> m_taskQueue;
        geode::LevelEditorLayer* m_activeEditor = nullptr;
        
        size_t m_totalNodesQueued = 0;
        int m_frameBudget = 150;
        bool m_isProcessing = false;

        // Diagnostic timing counters
        double m_accumulatedProcessTime = 0.0;

    public:
        TaskManager() = default;
        ~TaskManager() = default;

        // Singleton implementation matching Generator interface
        static TaskManager* get();

        // Execution Hooks
        bool initialize();
        void setFrameBudget(int budget);
        
        /**
         * Takes the computed abstract structure nodes from the Generator, 
         * flushes any dead queues, and prepares the asynchronous processing loop.
         */
        void submitGenerationTask(const std::vector<GameplayNode>& nodes, geode::LevelEditorLayer* editor);
        
        /**
         * Frame tick update driver. This must be wired up to the editor update 
         * loop within your EditorHooks file.
         */
        void onUpdateTick(float dt);
        
        /**
         * Clears all remaining item queues safely if a build is cancelled 
         * or if the player exits the LevelEditorLayer suddenly.
         */
        void flushQueue();

        // Metrics and Dashboard getters
        bool isProcessing() const { return m_isProcessing; }
        float getCompletionPercentage() const;
        size_t getRemainingTaskCount() const { return m_taskQueue.size(); }
    };
}

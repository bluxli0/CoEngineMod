#include "TaskManager.hpp"
#include <chrono>

using namespace geode::prelude;

namespace CoEngine {

    TaskManager* TaskManager::get() {
        static TaskManager instance;
        return &instance;
    }

    bool TaskManager::initialize() {
        CO_LOG_INFO("Initializing Anti-Lag Multi-Frame Task Manager Subsystem...");
        flushQueue();
        return true;
    }

    void TaskManager::setFrameBudget(int budget) {
        if (budget < 10) budget = 10; // Fail-safe protection boundary
        m_frameBudget = budget;
        CO_LOG_INFO("TaskManager processing configuration adjusted to: %d nodes/frame maximum.", m_frameBudget);
    }

    void TaskManager::flushQueue() {
        std::queue<GameplayNode> emptyQueue;
        std::swap(m_taskQueue, emptyQueue);
        
        m_activeEditor = nullptr;
        m_totalNodesQueued = 0;
        m_isProcessing = false;
        m_accumulatedProcessTime = 0.0;
    }

    void TaskManager::submitGenerationTask(const std::vector<GameplayNode>& nodes, geode::LevelEditorLayer* editor) {
        if (!editor) {
            CO_LOG_ERROR("Aborting submission: Destination LevelEditorLayer context is a null pointer!");
            return;
        }

        // If a task is running, clear it before appending to prevent mesh intersections
        if (m_isProcessing) {
            CO_LOG_WARN("Active sequence calculation interrupted. Overwriting active generation buffer.");
            flushQueue();
        }

        m_activeEditor = editor;
        m_totalNodesQueued = nodes.size();

        for (const auto& node : nodes) {
            m_taskQueue.push(node);
        }

        m_isProcessing = !m_taskQueue.empty();
        
        CO_LOG_INFO("Task queue successfully initialized. Buffered %zu procedural components for assembly loop.", m_totalNodesQueued);
    }

    float TaskManager::getCompletionPercentage() const {
        if (m_totalNodesQueued == 0) return 100.0f;
        size_t processed = m_totalNodesQueued - m_taskQueue.size();
        return (static_cast<float>(processed) / static_cast<float>(m_totalNodesQueued)) * 100.0f;
    }

    void TaskManager::onUpdateTick(float dt) {
        if (!m_isProcessing || m_taskQueue.empty() || !m_activeEditor) {
            if (m_isProcessing) {
                // If the state didn't clean up cleanly on finish, self-correct here
                m_isProcessing = false;
                CO_LOG_INFO("TaskManager processing cycle completed. Total profile timing metric: %.4f seconds.", m_accumulatedProcessTime);
            }
            return;
        }

        // Start performance tracking profile window
        auto startClock = std::chrono::high_resolution_clock::now();
        int objectsInstantiatedThisFrame = 0;

        // Process strictly up to our context budget limit allocations
        while (objectsInstantiatedThisFrame < m_frameBudget && !m_taskQueue.empty()) {
            GameplayNode node = m_taskQueue.front();
            m_taskQueue.pop();

            // Convert abstract node coordinate specs to internal Cocos2d geometry placement mapping
            cocos2d::CCPoint targetPosition = { node.offsetX, node.offsetY };

            /**
             * Create object directly using Geode's LevelEditorLayer bindings.
             * The parameters represent: (ID, position, bypassUndoAllocation)
             * We set undo logic to false inside the generation queue loops because 
             * our UndoRedoManager groups these into singular transactions later.
             */
            auto gameObject = m_activeEditor->createObject(node.fallbackObjectId, targetPosition, false);
            
            if (gameObject) {
                // Perform post-instantiation transformations if defined by layout settings
                objectsInstantiatedThisFrame++;
            }
        }

        auto endClock = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> processDuration = endClock - startClock;
        m_accumulatedProcessTime += processDuration.count();

        // Check if queue has been successfully exhausted
        if (m_taskQueue.empty()) {
            m_isProcessing = false;
            CO_LOG_INFO("Asynchronous block compilation task sequence closed out safely.");
        }
    }
}

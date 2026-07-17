#pragma once

#include "SchemaDefinitions.hpp"
#include <stdint.h>
#include <mutex>

namespace CoEngine {

    enum class EngineMode : uint8_t {
        IDLE = 0,
        GENERATING,
        PLAYTESTING,
        ERROR_STATE
    };

    /**
     * Performance telemetries observed by the TaskManager frame budget allocator.
     */
    struct RuntimeMetrics {
        uint32_t activeObjectCount = 0;
        uint32_t recycledObjectCount = 0;
        float averageFrameProcessingTimeMs = 0.0f;
        float generatorThroughput = 0.0f; // Blocks built per second
    };

    /**
     * High-level state tracking block.
     * Thread-safe layout structure ensuring background parsing doesn't crash 
     * the main Cocos2d-x rendering thread.
     */
    class ProjectState {
    private:
        std::mutex m_stateMutex;
        
        uint32_t m_currentSeed = 1337;
        double m_currentBPM = 120.0;
        float m_levelLengthUnits = 0.0f;
        EngineMode m_currentMode = EngineMode::IDLE;
        RuntimeMetrics m_metrics;

        ProjectState() = default;

    public:
        // Singleton Instance Access
        static ProjectState* get() {
            static ProjectState instance;
            return &instance;
        }

        // Thread-safe Setters and Getters
        void setSeed(uint32_t seed) {
            std::lock_guard<std::mutex> lock(m_stateMutex);
            m_currentSeed = seed;
        }

        uint32_t getSeed() {
            std::lock_guard<std::mutex> lock(m_stateMutex);
            return m_currentSeed;
        }

        void setBPM(double bpm) {
            std::lock_guard<std::mutex> lock(m_stateMutex);
            m_currentBPM = bpm;
        }

        double getBPM() {
            std::lock_guard<std::mutex> lock(m_stateMutex);
            return m_currentBPM;
        }

        void setEngineMode(EngineMode mode) {
            std::lock_guard<std::mutex> lock(m_stateMutex);
            m_currentMode = mode;
        }

        EngineMode getEngineMode() {
            std::lock_guard<std::mutex> lock(m_stateMutex);
            return m_currentMode;
        }

        void updateMetrics(const RuntimeMetrics& updatedMetrics) {
            std::lock_guard<std::mutex> lock(m_stateMutex);
            m_metrics = updatedMetrics;
        }

        RuntimeMetrics getMetrics() {
            std::lock_guard<std::mutex> lock(m_stateMutex);
            return m_metrics;
        }
    };
}

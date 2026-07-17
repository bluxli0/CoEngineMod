#include "SyncEngine.hpp"
#include <cmath>

namespace CoEngine {

    SyncEngine* SyncEngine::get() {
        static SyncEngine instance;
        return &instance;
    }

    bool SyncEngine::initialize() {
        CO_LOG_INFO("Initializing SyncEngine Rhythmic Translation Layer...");
        m_timelineGrid.clear();
        return true;
    }

    void SyncEngine::configureAudioSync(float bpm, int speedMode) {
        if (bpm <= 0.0f) {
            CO_LOG_WARN("Invalid BPM processed (%.2f). Defaulting to 128.0 BPM.", bpm);
            m_bpm = 128.0f;
        } else {
            m_bpm = bpm;
        }
        
        m_activeSpeedMode = speedMode;
        CO_LOG_INFO("SyncEngine reconfigured. BPM: %.2f | Speed Factor Mode: %d", m_bpm, m_activeSpeedMode);
    }

    float SyncEngine::getUnitsPerSecond(int speedMode) const {
        // Industry-standard hardcoded Geometry Dash engine constants
        switch (speedMode) {
            case -1: return 251.16f; // 0.5x Speed
            case 1:  return 387.42f; // 2.0x Speed
            case 2:  return 468.00f; // 3.0x Speed
            case 3:  return 576.00f; // 4.0x Speed
            default: return 311.58f; // 1.0x Base Speed
        }
    }

    float SyncEngine::getXPositionForBeat(float beat) const {
        float secondsPerBeat = 60.0f / m_bpm;
        float totalTimeInSeconds = beat * secondsPerBeat;
        return totalTimeInSeconds * getUnitsPerSecond(m_activeSpeedMode);
    }

    float SyncEngine::getBeatForXPosition(float xPos) const {
        if (xPos <= 0.0f) return 0.0f;
        float unitsPerSecond = getUnitsPerSecond(m_activeSpeedMode);
        float totalTimeInSeconds = xPos / unitsPerSecond;
        float secondsPerBeat = 60.0f / m_bpm;
        return totalTimeInSeconds / secondsPerBeat;
    }

    float SyncEngine::getTimeForXPosition(float xPos) const {
        if (xPos <= 0.0f) return 0.0f;
        return xPos / getUnitsPerSecond(m_activeSpeedMode);
    }

    const std::vector<BeatMarker>& SyncEngine::calculateRhythmGrid(float startX, float endX) {
        m_timelineGrid.clear();
        
        if (startX >= endX) {
            CO_LOG_WARN("Rhythm parsing bound error: Start coordinate is equal to or greater than end target.");
            return m_timelineGrid;
        }

        // Figure out where our target beats start and end structurally along the layout length
        float startBeat = std::floor(getBeatForXPosition(startX));
        float endBeat = std::ceil(getBeatForXPosition(endX));
        
        float secondsPerBeat = 60.0f / m_bpm;
        float speedUnitsPerSecond = getUnitsPerSecond(m_activeSpeedMode);

        CO_LOG_INFO("Structuring music-map boundary grids from Beat %.1f to Beat %.1f", startBeat, endBeat);

        // Discretize musical increments at 0.5 beat steps (Eighth note resolution)
        for (float currentBeat = startBeat; currentBeat <= endBeat; currentBeat += 0.5f) {
            BeatMarker marker;
            marker.beatNumber = currentBeat;
            marker.absoluteTimeSeconds = currentBeat * secondsPerBeat;
            marker.absoluteX = marker.absoluteTimeSeconds * speedUnitsPerSecond;

            // Determine rhythmic significance weightings via clean modulo parsing
            float integerPart;
            float fractionalPart = std::modf(currentBeat, &integerPart);

            if (fractionalPart > 0.01f) {
                marker.rhythmRole = BeatType::SUBDIVISION;
            } else {
                int beatIndex = static_cast<int>(integerPart);
                if (beatIndex % 32 == 0 && beatIndex != 0) {
                    marker.rhythmRole = BeatType::SYNC_DROP; // Sectional change every 32 beats
                } else if (beatIndex % 4 == 0) {
                    marker.rhythmRole = BeatType::DOWNBEAT;   // Bar start marker
                } else {
                    marker.rhythmRole = BeatType::UPBEAT;     // Inside bar fillers
                }
            }

            m_timelineGrid.push_back(marker);
        }

        CO_LOG_INFO("Successfully constructed grid line arrays. Analyzed timeline points: %zu", m_timelineGrid.size());
        return m_timelineGrid;
    }
}

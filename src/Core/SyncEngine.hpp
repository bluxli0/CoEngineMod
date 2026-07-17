#pragma once

#include <Geode/Geode.hpp>
#include <CoEngineCommon.hpp>
#include <vector>

namespace CoEngine {

    enum class BeatType {
        DOWNBEAT,     // Major stress (Beat 1 of a measure)
        UPBEAT,       // Minor stress (Beats 2, 3, 4)
        SUBDIVISION,  // Eighth or sixteenth notes for dense hazard placements
        SYNC_DROP     // Sectional climax point
    };

    struct BeatMarker {
        float beatNumber;
        float absoluteX;
        float absoluteTimeSeconds;
        BeatType rhythmRole;
    };

    class SyncEngine {
    private:
        float m_bpm = 128.0f;
        int m_activeSpeedMode = 0; // GD Speed constraints: -1 (0.5x) to 3 (4x)
        std::vector<BeatMarker> m_timelineGrid;

        // Core conversion factor constants
        float getUnitsPerSecond(int speedMode) const;

    public:
        SyncEngine() = default;
        ~SyncEngine() = default;

        static SyncEngine* get();

        // Lifecycle and Configuration Initialization
        bool initialize();
        void configureAudioSync(float bpm, int speedMode);

        // Grid Translation Math Pipeline
        float getXPositionForBeat(float beat) const;
        float getBeatForXPosition(float xPos) const;
        float getTimeForXPosition(float xPos) const;

        /**
         * Rhythmic Blueprint Generation
         * Populates a layout map with structural musical points (drops, downbeats)
         * so the Generator knows exactly when to spike obstacle density.
         */
        const std::vector<BeatMarker>& calculateRhythmGrid(float startX, float endX);
        
        // Inline accessors
        float getActiveBPM() const { return m_bpm; }
        const std::vector<BeatMarker>& getTimelineGrid() const { return m_timelineGrid; }
    };
}

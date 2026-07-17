#pragma once

#include <Geode/Geode.hpp>
#include <include/CoEngineCommon.hpp>
#include <vector>
#include <string>
#include <random>

namespace CoEngine {

    // Pool tags for structural asset allocation
    enum class PoolTag : uint32_t {
        STRUCTURE = 0,
        HAZARD,
        JUMP_ORB,
        TIMING_CRITICAL,
        PLATFORM_BASE,
        DECORATION
    };

    // Tracks global engine state configurations
    enum class GenerationMode {
        LINEAR_HORIZONTAL,  // Classic scrolling layouts
        PLATFORMER_SPATIAL,  // Open 2D room/grid layouts
        NOISE_TERRAIN       // Perlin noise driven terrain tracking
    };

    struct GenerationSettings {
        float bpm = 120.0f;
        float intensity = 0.5f;
        uint32_t seed = 1337;
        GenerationMode mode = GenerationMode::LINEAR_HORIZONTAL;
    };

    struct GameplayNode {
        float offsetX;
        float offsetY;
        PoolTag primaryTag;
        int fallbackObjectId;
        float patternWeight;
    };

    struct GameplayPattern {
        std::string patternName;
        float requiredMinIntensity;
        float requiredMaxIntensity;
        std::vector<GameplayNode> nodes;
        float weight;
        float totalWidth; // Dynamic track separation check
    };

    class Generator {
    private:
        GenerationSettings m_currentSettings;
        std::vector<GameplayPattern> m_patternLibrary;
        std::vector<GameplayNode> m_generatedSkeleton;
        
        // High-Performance Engine PRNG Instance
        std::mt19937 m_rngInstance;
        
        void initializePatternLibrary();
        GameplayPattern selectPatternForContext(float intensity);
        float calculateSpeedDeltas(int speedMod);

        // Core Generation Pipelines
        void processLinearHorizontal(float startX, float endX);
        void processPlatformerSpatial(float startX, float endX);
        void processNoiseTerrain(float startX, float endX);

        Generator() = default;

    public:
        ~Generator() = default;

        static Generator* get();

        bool initialize();
        void updateSettings(const GenerationSettings& newSettings);
        void clearActiveSkeleton();
        
        // Dispatches processing down the requested pipeline configuration
        const std::vector<GameplayNode>& generateSkeleton(float startX, float endX, int currentGamemode);

        // Accessors
        const std::vector<GameplayNode>& getActiveSkeleton() const { return m_generatedSkeleton; }
        GenerationSettings getSettings() const { return m_currentSettings; }
    };
}

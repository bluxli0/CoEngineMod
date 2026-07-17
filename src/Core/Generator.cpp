#include "Generator.hpp"
#include <Utils/MathUtils.hpp>
#include <algorithm>
#include <cmath>

namespace CoEngine {

    Generator* Generator::get() {
        static Generator instance;
        return &instance;
    }

    bool Generator::initialize() {
        CO_LOG_INFO("Initializing Procedural Generation Core...");
        
        // Seed PRNG once to protect memory frame targets
        m_rngInstance.seed(m_currentSettings.seed);
        
        initializePatternLibrary();
        clearActiveSkeleton();
        return true;
    }

    void Generator::updateSettings(const GenerationSettings& newSettings) {
        m_currentSettings = newSettings;
        m_rngInstance.seed(m_currentSettings.seed);
        CO_LOG_INFO("Generator settings updated. Active Seed: %u | Mode ID: %d", 
            m_currentSettings.seed, static_cast<int>(m_currentSettings.mode));
    }

    void Generator::clearActiveSkeleton() {
        m_generatedSkeleton.clear();
    }

    void Generator::initializePatternLibrary() {
        m_patternLibrary.clear();

        // 1. LINEAR / HORIZONTAL: Standard Jump Profile
        GameplayPattern basicJump;
        basicJump.patternName = "basic_floor_jump";
        basicJump.requiredMinIntensity = 0.0f;
        basicJump.requiredMaxIntensity = 0.6f;
        basicJump.weight = 1.0f;
        basicJump.totalWidth = 180.0f; 
        basicJump.nodes = {
            { 0.0f,   0.0f, PoolTag::STRUCTURE, 1, 1.0f },
            { 60.0f,  0.0f, PoolTag::HAZARD,    8, 1.0f },
            { 120.0f, 0.0f, PoolTag::STRUCTURE, 1, 1.0f }
        };
        m_patternLibrary.push_back(basicJump);

        // 2. PLATFORMER / SPATIAL: Vertical Drop Structural Rig
        GameplayPattern verticalRoom;
        verticalRoom.patternName = "platformer_chasm_drop";
        verticalRoom.requiredMinIntensity = 0.2f;
        verticalRoom.requiredMaxIntensity = 1.0f;
        verticalRoom.weight = 0.7f;
        verticalRoom.totalWidth = 300.0f;
        verticalRoom.nodes = {
            { 0.0f,    0.0f,   PoolTag::PLATFORM_BASE, 91, 1.0f },
            { 90.0f,  -60.0f,  PoolTag::PLATFORM_BASE, 91, 1.0f },
            { 180.0f, -120.0f, PoolTag::PLATFORM_BASE, 91, 1.0f },
            { 270.0f,  0.0f,   PoolTag::PLATFORM_BASE, 91, 1.0f }
        };
        m_patternLibrary.push_back(verticalRoom);
        
        CO_LOG_INFO("Successfully cached %zu multi-mode templates.", m_patternLibrary.size());
    }

    const std::vector<GameplayNode>& Generator::generateSkeleton(float startX, float endX, int currentGamemode) {
        clearActiveSkeleton();
        
        if (startX >= endX) {
            CO_LOG_ERROR("Invalid vector parsing span bounds passed to Generator.");
            return m_generatedSkeleton;
        }

        // Branch directly into the specialized generative engine system pipeline
        switch (m_currentSettings.mode) {
            case GenerationMode::PLATFORMER_SPATIAL:
                processPlatformerSpatial(startX, endX);
                break;
            case GenerationMode::NOISE_TERRAIN:
                processNoiseTerrain(startX, endX);
                break;
            case GenerationMode::LINEAR_HORIZONTAL:
            default:
                processLinearHorizontal(startX, endX);
                break;
        }

        return m_generatedSkeleton;
    }

    // ============================================================================
    // PIPELINE STRATEGIES
    // ============================================================================

    // 1. LINEAR & HORIZONTAL GENERATION: Fixed scrolling logic
    void Generator::processLinearHorizontal(float startX, float endX) {
        float currentTrackX = startX;
        
        while (currentTrackX < endX) {
            GameplayPattern currentPattern = selectPatternForContext(m_currentSettings.intensity);

            for (const auto& node : currentPattern.nodes) {
                GameplayNode physicalNode = node;
                physicalNode.offsetX += currentTrackX;
                m_generatedSkeleton.push_back(physicalNode);
            }
            // Dynamic width protection prevents overlapping assets
            currentTrackX += currentPattern.totalWidth; 
        }
    }

    // 2. SPATIAL & PLATFORMER GENERATION: Multidirectional heights and free room navigation
    void Generator::processPlatformerSpatial(float startX, float endX) {
        float currentTrackX = startX;
        float currentTrackY = 0.0f; // Track multi-axis tracking values dynamically

        std::uniform_real_distribution<float> heightDist(-90.0f, 90.0f);

        while (currentTrackX < endX) {
            GameplayPattern currentPattern = selectPatternForContext(m_currentSettings.intensity);
            
            // Randomly shift height channels to test vertical exploration mapping
            currentTrackY += std::floor(heightDist(m_rngInstance) / MathUtils::GRID_UNIT) * MathUtils::GRID_UNIT;

            for (const auto& node : currentPattern.nodes) {
                GameplayNode physicalNode = node;
                physicalNode.offsetX += currentTrackX;
                physicalNode.offsetY += currentTrackY; 
                m_generatedSkeleton.push_back(physicalNode);
            }
            currentTrackX += currentPattern.totalWidth + 60.0f; // Injects gaps for horizontal jumps
        }
    }

    // 3. NOISE GENERATION: Mathematical structural terrain mapping via Perlin Noise
    void Generator::processNoiseTerrain(float startX, float endX) {
        int startGridX = static_cast<int>(std::floor(startX / MathUtils::GRID_UNIT));
        int endGridX = static_cast<int>(std::floor(endX / MathUtils::GRID_UNIT));

        for (int x = startGridX; x <= endGridX; ++x) {
            float worldX = static_cast<float>(x) * MathUtils::GRID_UNIT;
            
            // Sample continuous Perlin calculations
            float noiseVal = MathUtils::getPerlinNoise2D(static_cast<float>(x) * 0.15f, 0.0f, m_currentSettings.seed);
            int targetHeightBlocks = 2 + static_cast<int>(std::floor((noiseVal + 1.0f) * 3.0f));

            // Generate structural terrain heights
            for (int y = 0; y < targetHeightBlocks; ++y) {
                GameplayNode terrainNode;
                terrainNode.offsetX = worldX;
                terrainNode.offsetY = static_cast<float>(y) * MathUtils::GRID_UNIT;
                terrainNode.primaryTag = PoolTag::STRUCTURE;
                terrainNode.fallbackObjectId = 1; // Native base block ID
                terrainNode.patternWeight = 1.0f;

                m_generatedSkeleton.push_back(terrainNode);
            }
        }
    }

    // ============================================================================
    // UTILITIES
    // ============================================================================

    GameplayPattern Generator::selectPatternForContext(float intensity) {
        std::vector<GameplayPattern> validOptions;
        float totalWeight = 0.0f;

        for (const auto& pattern : m_patternLibrary) {
            if (intensity >= pattern.requiredMinIntensity && intensity <= pattern.requiredMaxIntensity) {
                validOptions.push_back(pattern);
                totalWeight += pattern.weight;
            }
        }

        if (validOptions.empty()) {
            return m_patternLibrary.front();
        }

        // Thread-safe, high-speed random evaluation bounds mapping
        std::uniform_real_distribution<float> dis(0.0f, totalWeight);
        float targetWeight = dis(m_rngInstance);

        float currentWeightSum = 0.0f;
        for (const auto& opt : validOptions) {
            currentWeightSum += opt.weight;
            if (targetWeight <= currentWeightSum) {
                return opt;
            }
        }

        return validOptions.back();
    }

    float Generator::calculateSpeedDeltas(int speedMod) {
        switch (speedMod) {
            case -1: return 251.16f;
            case 1:  return 387.42f;
            case 2:  return 468.0f;
            case 3:  return 576.0f;
            default: return 311.58f;
        }
    }
}

#pragma once

#include <Geode/Geode.hpp>
#include <cocos2d.h>
#include <cmath>

namespace CoEngine {

    class MathUtils {
    private:
        static const int s_permutation[512];
        static float fade(float t);
        static float lerp(float t, float a, float b);
        static float grad(int hash, float x, float y);

    public:
        // Geometry Dash standard: 1 block grid space = 30.0 units
        static constexpr float GRID_UNIT = 30.0f;

        /**
         * Computes a deterministic, seed-based hash for coordinate sets.
         */
        static float getSeededRandom(uint32_t seed, int x, int y);

        /**
         * Standard 2D Perlin Noise for procedural height calculations.
         */
        static float getPerlinNoise2D(float x, float y, uint32_t seed);

        /**
         * Grid snapping conversions.
         */
        static cocos2d::CCPoint gridToWorld(int gridX, int gridY);
        static cocos2d::CCPoint worldToGrid(const cocos2d::CCPoint& worldPos);

        /**
         * Easing math functions.
         */
        static float easeInOutQuad(float t);
        static float easeOutCubic(float t);
    };
}

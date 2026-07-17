#pragma once

#include <Geode/Geode.hpp>
#include <cocos2d.h>
#include <unordered_map>
#include <string>

namespace CoEngine {

    struct ColorChannelConfig {
        int channelID;
        cocos2d::ccColor3B color;
        bool blending = false;
        float opacity = 1.0f;
    };

    class StylePalette {
    private:
        // Key maps to semantic colors (e.g., "primary_neon", "hazard_glow")
        std::unordered_map<std::string, ColorChannelConfig> m_paletteChannels;
        
        StylePalette() = default;

    public:
        ~StylePalette() = default;

        // Singleton Accessor
        static StylePalette* get();

        /**
         * Clears active runtime translations and populates color maps from parsed JSON objects.
         */
        bool loadFromJSON(matjson::Value const& paletteObject);

        /**
         * Takes the current theme configurations and forcefully writes them right into the 
         * active LevelSettingsObject data structures within the current Geometry Dash editor session.
         */
        void applyPaletteToCurrentLevel();

        /**
         * Resolves a semantic color name to its assigned level color channel integer.
         * @return The exact ID (1-999) if found, otherwise returns a fallback index channel.
         */
        int getChannelID(std::string const& semanticColorKey, int fallbackChannel = 1) const;

        /**
         * Resets the channel layout to a safe, default vanilla color scheme block.
         */
        void injectDefaults();
    };
}

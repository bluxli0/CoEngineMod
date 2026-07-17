#include "StylePalette.hpp"

using namespace geode::prelude;

namespace CoEngine {

    StylePalette* StylePalette::get() {
        static StylePalette instance;
        if (instance.m_paletteChannels.empty()) {
            instance.injectDefaults();
        }
        return &instance;
    }

    void StylePalette::injectDefaults() {
        m_paletteChannels.clear();

        // Baseline architectural color setup configs
        m_paletteChannels["structural_base"]  = {1, {255, 255, 255}, false, 1.0f}; // Clear White
        m_paletteChannels["structural_detail"]= {2, {0, 180, 255},   true,  0.8f}; // Translucent Blue
        m_paletteChannels["hazard_accent"]    = {3, {255, 50, 50},   false, 1.0f}; // Alarm Red
        m_paletteChannels["ambient_decoration"]= {4, {140, 50, 200},  true,  0.5f}; // Low Alpha Purple
    }

    bool StylePalette::loadFromJSON(matjson::Value const& paletteObject) {
        if (!paletteObject.is_object()) return false;

        m_paletteChannels.clear();
        CO_LOG_INFO("Processing theme palette layout map arrays...");

        for (const auto& [colorKey, colorData] : paletteObject.as_object()) {
            if (!colorData.contains("channel") || !colorData.contains("hex")) continue;

            ColorChannelConfig config;
            config.channelID = colorData["channel"].as_int();

            // Parse hex color values safely (expects format "#RRGGBB")
            std::string hexStr = colorData["hex"].as_string();
            if (hexStr.length() == 7 && hexStr[0] == '#') {
                long hexValue = std::strtol(hexStr.substr(1).c_str(), nullptr, 16);
                config.color.r = static_cast<unsigned char>((hexValue >> 16) & 0xFF);
                config.color.g = static_cast<unsigned char>((hexValue >> 8) & 0xFF);
                config.color.b = static_cast<unsigned char>(hexValue & 0xFF);
            } else {
                config.color = {255, 255, 255}; // Error fallback flag
            }

            if (colorData.contains("blending")) {
                config.blending = colorData["blending"].as_bool();
            }
            if (colorData.contains("opacity")) {
                config.opacity = static_cast<float>(colorData["opacity"].as_double());
            }

            m_paletteChannels[colorKey] = config;
            CO_LOG_INFO("Registered Color semantic: '%s' to GD Channel %d", colorKey.c_str(), config.channelID);
        }
        return true;
    }

    void StylePalette::applyPaletteToCurrentLevel() {
        auto editor = geode::LevelEditorLayer::get();
        if (!editor || !editor->m_levelSettings) {
            CO_LOG_WARN("Cannot apply theme colors: Active LevelEditorLayer framework handle missing.");
            return;
        }

        CO_LOG_INFO("Streaming active style colors into internal level configuration tracks...");

        // Note: Geometry Dash stores colors inside an internal dictionary matrix on m_levelSettings.
        // We iterate and forcefully overwrite or inject the LevelSettings mapping.
        for (const auto& [key, config] : m_paletteChannels) {
            // Geode provides safe wrapping methods via the LevelSettings system handles if available,
            // or we fall back to setting up standard ColorAction frames.
            auto colorAction = editor->m_levelSettings->getColorAction(config.channelID);
            if (colorAction) {
                colorAction->m_color = config.color;
                colorAction->m_blending = config.blending;
                colorAction->m_opacity = config.opacity;
                colorAction->updatePublicColor();
            }
        }
    }

    int StylePalette::getChannelID(std::string const& semanticColorKey, int fallbackChannel) const {
        auto trackingIterator = m_paletteChannels.find(semanticColorKey);
        if (trackingIterator != m_paletteChannels.end()) {
            return trackingIterator->second.channelID;
        }
        return fallbackChannel;
    }
}

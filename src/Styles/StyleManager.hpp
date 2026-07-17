#pragma once

#include <Geode/Geode.hpp>
#include <CoEngineCommon.hpp>
#include <filesystem>
#include <unordered_map>
#include <string>

namespace CoEngine {

    struct StyleTheme {
        std::string themeID;
        std::string displayName;
        std::string author;
        
        // Maps generalized structural labels (e.g., "ground_block") to internal game Object IDs
        std::unordered_map<std::string, int> objectMappings;
        
        // Extra decoration attributes (e.g., colors, layer offsets)
        std::unordered_map<std::string, std::string> metadata;
    };

    class StyleManager {
    private:
        std::unordered_map<std::string, StyleTheme> m_loadedThemes;
        std::string m_activeThemeID = "default";
        
        StyleManager() = default;

        // Visual layout defaults fallback generator
        void injectFallbackTheme();

    public:
        ~StyleManager() = default;

        // Singleton access framework
        static StyleManager* get();

        /**
         * Scans a target mod directory path for *.json style configurations.
         * @param directoryPath Target directory containing template configurations.
         * @return Total count of successfully verified and parsed themes.
         */
        size_t discoverThemes(std::filesystem::path const& directoryPath);

        /**
         * Parses an individual theme configuration file directly into the tracking map matrix.
         * @param filePath Full path to the targeted JSON schema file.
         */
        bool loadThemeFile(std::filesystem::path const& filePath);

        /**
         * Switches the active layout template lookup pointer context.
         * @param themeID The string key designation of the desired aesthetic skin.
         */
        bool setActiveTheme(std::string const& themeID);

        /**
         * Queries the active theme style sheet to match a semantic string tag to a concrete asset.
         * @param semanticKey High-level intent designation (e.g., "hazard_spike_small").
         * @param fallbackID The absolute Object ID returned if the tag mapping definition is missing.
         */
        int resolveObjectID(std::string const& semanticKey, int fallbackID = 1) const;

        // Accessors
        std::string getActiveThemeID() const { return m_activeThemeID; }
        std::vector<std::string> getAvailableThemeIDs() const;
        StyleTheme const* getActiveTheme() const;
    };
}

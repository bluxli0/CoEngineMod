#include "StyleManager.hpp"
#include <fstream>

using namespace geode::prelude;

namespace CoEngine {

    StyleManager* StyleManager::get() {
        static StyleManager instance;
        // Self-initialize structural fallbacks on first reference loop instantiation
        if (instance.m_loadedThemes.empty()) {
            instance.injectFallbackTheme();
        }
        return &instance;
    }

    void StyleManager::injectFallbackTheme() {
        StyleTheme fallback;
        fallback.themeID = "default";
        fallback.displayName = "Classic Vanilla";
        fallback.author = "CoEngine Team";

        // Hardcoded standard baseline fallback assignments mapping basic structural pieces
        fallback.objectMappings["block_solid"] = 1;         // Standard outline block
        fallback.objectMappings["hazard_spike"] = 8;        // Standard upward obstacle spike
        fallback.objectMappings["hazard_spike_small"] = 39; // Mini obstacle spike
        fallback.objectMappings["jump_pad"] = 35;           // Yellow vertical launch pad
        fallback.objectMappings["jump_ring"] = 36;          // Yellow orb mechanism trigger

        m_loadedThemes["default"] = fallback;
        CO_LOG_INFO("StyleManager baseline structural fallback rules injected.");
    }

    size_t StyleManager::discoverThemes(std::filesystem::path const& directoryPath) {
        if (!std::filesystem::exists(directoryPath) || !std::filesystem::is_directory(directoryPath)) {
            CO_LOG_WARN("Theme search path not found: %s", directoryPath.string().c_str());
            return 0;
        }

        size_t successCount = 0;
        for (const auto& entry : std::filesystem::directory_iterator(directoryPath)) {
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                if (loadThemeFile(entry.path())) {
                    successCount++;
                }
            }
        }

        CO_LOG_INFO("Theme discovery scan finished. Registered %zu style layouts from disk.", successCount);
        return successCount;
    }

    bool StyleManager::loadThemeFile(std::filesystem::path const& filePath) {
        std::ifstream fileStream(filePath);
        if (!fileStream.is_open()) {
            CO_LOG_ERROR("Failed to open file stream for skin config: %s", filePath.string().c_str());
            return false;
        }

        std::string fileContent((std::istreambuf_iterator<char>(fileStream)), std::istreambuf_iterator<char>());
        fileStream.close();

        // Parse standard json payload buffer using modern Geode core schemas
        std::string parseError;
        auto jsonParseResult = matjson::parse(fileContent);
        
        if (!jsonParseResult.has_value()) {
            CO_LOG_ERROR("JSON Layout Schema Parsing Exception in file [%s]: %s", 
                filePath.filename().string().c_str(), jsonParseResult.error().c_str());
            return false;
        }

        auto rootJson = jsonParseResult.value();

        // Validation pass to verify crucial metadata records exist before loading
        if (!rootJson.contains("theme-id") || !rootJson.contains("mappings")) {
            CO_LOG_ERROR("Skipping malformed style layout: %s (Missing 'theme-id' or 'mappings')", filePath.filename().string().c_str());
            return false;
        }

        StyleTheme parsedTheme;
        parsedTheme.themeID = rootJson["theme-id"].as_string();
        parsedTheme.displayName = rootJson.contains("display-name") ? rootJson["display-name"].as_string() : parsedTheme.themeID;
        parsedTheme.author = rootJson.contains("author") ? rootJson["author"].as_string() : "Unknown Contributor";

        // Extract object mapping configurations securely
        auto mappingsObj = rootJson["mappings"];
        for (const auto& [semanticKey, jsonValue] : mappingsObj.as_object()) {
            if (jsonValue.is_number()) {
                parsedTheme.objectMappings[semanticKey] = jsonValue.as_int();
            }
        }

        // Catch optional rendering instructions payload vectors if passed
        if (rootJson.contains("metadata") && rootJson["metadata"].is_object()) {
            auto metaObj = rootJson["metadata"];
            for (const auto& [metaKey, metaValue] : metaObj.as_object()) {
                parsedTheme.metadata[metaKey] = metaValue.as_string();
            }
        }

        // Cache the completed object into memory matrices
        std::string key = parsedTheme.themeID;
        m_loadedThemes[key] = parsedTheme;
        
        CO_LOG_INFO("Successfully loaded theme sheet: '%s' by [%s]", parsedTheme.displayName.c_str(), parsedTheme.author.c_str());
        return true;
    }

    bool StyleManager::setActiveTheme(std::string const& themeID) {
        if (m_loadedThemes.find(themeID) == m_loadedThemes.end()) {
            CO_LOG_WARN("Theme switch aborted. Requested key designation ID '%s' is not registered.", themeID.c_str());
            return false;
        }
        m_activeThemeID = themeID;
        CO_LOG_INFO("Active theme interface switched successfully to: '%s'", m_activeThemeID.c_str());
        return true;
    }

    int StyleManager::resolveObjectID(std::string const& semanticKey, int fallbackID) const {
        const auto& activeTheme = m_loadedThemes.at(m_activeThemeID);
        auto trackingIterator = activeTheme.objectMappings.find(semanticKey);

        if (trackingIterator != activeTheme.objectMappings.end()) {
            return trackingIterator->second;
        }

        // Global secondary lookup pass to check back within default template rules before dropping
        const auto& globalFallbackTheme = m_loadedThemes.at("default");
        auto secondaryIterator = globalFallbackTheme.objectMappings.find(semanticKey);
        if (secondaryIterator != globalFallbackTheme.objectMappings.end()) {
            return secondaryIterator->second;
        }

        return fallbackID;
    }

    std::vector<std::string> StyleManager::getAvailableThemeIDs() const {
        std::vector<std::string> registrationKeys;
        for (const auto& [themeKey, themeData] : m_loadedThemes) {
            registrationKeys.push_back(themeKey);
        }
        return registrationKeys;
    }

    StyleTheme const* StyleManager::getActiveTheme() const {
        auto search = m_loadedThemes.find(m_activeThemeID);
        if (search != m_loadedThemes.end()) {
            return &(search->second);
        }
        return &(m_loadedThemes.at("default"));
    }
}

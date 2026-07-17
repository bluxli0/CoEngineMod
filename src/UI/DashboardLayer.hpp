#pragma once

#include <Geode/Geode.hpp>
#include <CoEngineCommon.hpp>

namespace CoEngine {

    class DashboardLayer : public geode::Popup<std::string const&> {
    protected:
        // Core Geode setup lifecycle override
        bool setup(std::string const& activePresetName) override;
        
        // Internal UI state nodes
        cocos2d::CCLabelBMFont* m_statusLabel = nullptr;
        cocos2d::CCLabelBMFont* m_progressLabel = nullptr;
        geode::ButtonSprite* m_generateBtnSprite = nullptr;

        // UI Initialization helpers
        void createBrandingHeader(cocos2d::CCSize windowSize);
        void createControlButtons(cocos2d::CCSize windowSize);
        void createStatusPanel(cocos2d::CCSize windowSize);

    public:
        // Static creation method conforming to Cocos2d-x conventions
        static DashboardLayer* create(std::string const& activePresetName = "classic");
        
        // Frame tick update handler to monitor live progress bar states
        void updateUIState(float dt);

        // Action Callbacks
        void onGenerate(cocos2d::CCObject* sender);
        void onUndo(cocos2d::CCObject* sender);
        void onRedo(cocos2d::CCObject* sender);
        void onOpenTimeline(cocos2d::CCObject* sender);
    };
}

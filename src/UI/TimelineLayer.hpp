#pragma once

#include <Geode/Geode.hpp>
#include <CoEngineCommon.hpp>
#include <Core/SyncEngine.hpp>

namespace CoEngine {

    class TimelineLayer : public geode::Popup<std::string const&> {
    private:
        cocos2d::CCNode* m_gridContainer = nullptr;
        cocos2d::CCLabelBMFont* m_viewRangeLabel = nullptr;
        
        float m_scrollViewportX = 0.0f;       // Horizontal panning window position
        const float m_viewportWidthUnits = 3000.0f; // Amount of level space shown at once
        const float m_uiScaleFactor = 0.12f;       // Converts level units into UI pixels

        // Layout Renderers
        bool setup(std::string const& templateContext) override;
        void drawTimelineTrack();
        void refreshGridElements();
        
        // Style color router based on the musical role of the beat
        cocos2d::ccColor3B getColorForBeat(BeatType role);

    public:
        // Standard Cocos2d memory instantiation sequence
        static TimelineLayer* create(std::string const& templateContext = "");

        // User Input Triggers
        void onPanLeft(cocos2d::CCObject* sender);
        void onPanRight(cocos2d::CCObject* sender);
        void onSyncConfigSync(cocos2d::CCObject* sender);
    };
}

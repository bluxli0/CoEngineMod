#include "TimelineLayer.hpp"
#include <Core/EngineCommandBus.hpp>

using namespace geode::prelude;

namespace CoEngine {

    TimelineLayer* TimelineLayer::create(std::string const& templateContext) {
        auto instance = new TimelineLayer();
        // Creates a wider, landscape-oriented layout panel optimized for timelines (460x240)
        if (instance && instance->initAnchored(460.0f, 240.0f, templateContext, "GJ_square02.png")) {
            instance->autorelease();
            return instance;
        }
        CC_SAFE_DELETE(instance);
        return nullptr;
    }

    bool TimelineLayer::setup(std::string const& templateContext) {
        auto size = m_bgSprite->getContentSize();
        this->setTitle("Rhythm & Sync Timeline");
        this->setID("CoEngineTimelinePopup");

        // 1. Initialize Context Container Node
        m_gridContainer = cocos2d::CCNode::create();
        m_gridContainer->setPosition({ size.width / 2.0f, size.height / 2.0f });
        m_mainLayer->addChild(m_gridContainer);

        // 2. Track Window Navigation Display
        m_viewRangeLabel = cocos2d::CCLabelBMFont::create("Range: 0 - 3000", "goldFont.fnt");
        m_viewRangeLabel->setPosition({ size.width / 2.0f, size.height - 40.0f });
        m_viewRangeLabel->setScale(0.4f);
        m_mainLayer->addChild(m_viewRangeLabel);

        // 3. Mount Track Mechanics and Controller Options
        drawTimelineTrack();
        refreshGridElements();
        return true;
    }

    void TimelineLayer::drawTimelineTrack() {
        auto size = m_bgSprite->getContentSize();
        auto controlMenu = cocos2d::CCMenu::create();
        controlMenu->setPosition({ 0.0f, 0.0f });
        m_mainLayer->addChild(controlMenu);

        // Linear horizontal track spine baseline backing
        auto trackBar = cocos2d::CCLayerColor::create({ 30, 30, 32, 255 }, 400.0f, 60.0f);
        trackBar->setPosition({ (size.width - 400.0f) / 2.0f, 80.0f });
        trackBar->setID("TimelineSpineBackground");
        m_gridContainer->addChild(trackBar, -1);

        // Navigation Button: Scroll Left
        auto leftSpr = geode::ButtonSprite::create("<", "bigFont.fnt", "GJ_button_02.png");
        leftSpr->setScale(0.6f);
        auto leftBtn = geode::CCMenuItemSpriteExtra::create(leftSpr, this, menu_selector(TimelineLayer::onPanLeft));
        leftBtn->setPosition({ 25.0f, 110.0f });
        controlMenu->addChild(leftBtn);

        // Navigation Button: Scroll Right
        auto rightSpr = geode::ButtonSprite::create(">", "bigFont.fnt", "GJ_button_02.png");
        rightSpr->setScale(0.6f);
        auto rightBtn = geode::CCMenuItemSpriteExtra::create(rightSpr, this, menu_selector(TimelineLayer::onPanRight));
        rightBtn->setPosition({ size.width - 25.0f, 110.0f });
        controlMenu->addChild(rightBtn);

        // Diagnostic verification utility hotkey action
        auto refreshSpr = geode::ButtonSprite::create("Re-Sync", "bigFont.fnt", "GJ_button_01.png");
        refreshSpr->setScale(0.5f);
        auto refreshBtn = geode::CCMenuItemSpriteExtra::create(refreshSpr, this, menu_selector(TimelineLayer::onSyncConfigSync));
        refreshBtn->setPosition({ size.width / 2.0f, 35.0f });
        controlMenu->addChild(refreshBtn);
    }

    void TimelineLayer::refreshGridElements() {
        // Drop any pre-existing ticker graphics from the viewport
        m_gridContainer->removeAllChildrenWithCleanup(true);

        // Redraw track bar backing since container was cleared
        auto size = m_bgSprite->getContentSize();
        auto trackBar = cocos2d::CCLayerColor::create({ 20, 20, 22, 255 }, 400.0f, 60.0f);
        trackBar->setPosition({ -200.0f, -40.0f }); // Center relative to container origin
        m_gridContainer->addChild(trackBar, 0);

        float endXWindow = m_scrollViewportX + m_viewportWidthUnits;
        m_viewRangeLabel->setString(fmt::format("Grid Bounds: {:.0f} - {:.0f} Units", m_scrollViewportX, endXWindow).c_str());

        // Extract timing elements matching active viewport domain coordinates from Core
        auto syncEngine = SyncEngine::get();
        auto gridPoints = syncEngine->calculateRhythmGrid(m_scrollViewportX, endXWindow);

        for (const auto& marker : gridPoints) {
            // Find proportional horizontal pixel destination mapping
            float relativeDeltaX = marker.absoluteX - m_scrollViewportX;
            float targetPixelX = -200.0f + (relativeDeltaX * m_uiScaleFactor);

            // Bounds check to truncate drawing leaks
            if (targetPixelX < -200.0f || targetPixelX > 200.0f) continue;

            // Tailor dimensions based on tick hierarchy to create a clean timeline rhythm view
            float barHeight = 25.0f;
            float barWidth = 1.5f;

            if (marker.rhythmRole == BeatType::SYNC_DROP)  { barHeight = 45.0f; barWidth = 3.0f; }
            if (marker.rhythmRole == BeatType::DOWNBEAT)   { barHeight = 35.0f; barWidth = 2.0f; }
            if (marker.rhythmRole == BeatType::SUBDIVISION) { barHeight = 12.0f; barWidth = 1.0f; }

            // Instantiation of visual line markers using native colors
            auto tickColor = getColorForBeat(marker.rhythmRole);
            auto visualTick = cocos2d::CCLayerColor::create(
                { tickColor.r, tickColor.g, tickColor.b, 255 }, barWidth, barHeight
            );
            visualTick->setPosition({ targetPixelX, -10.0f });
            m_gridContainer->addChild(visualTick, 1);

            // Add numeric designations above major bar downbeats
            if (marker.rhythmRole == BeatType::DOWNBEAT || marker.rhythmRole == BeatType::SYNC_DROP) {
                auto beatText = cocos2d::CCLabelBMFont::create(
                    fmt::format("{:.0f}", marker.beatNumber).c_str(), "chatFont.fnt"
                );
                beatText->setPosition({ targetPixelX, 28.0f });
                beatText->setScale(0.45f);
                m_gridContainer->addChild(beatText, 2);
            }
        }
    }

    cocos2d::ccColor3B TimelineLayer::getColorForBeat(BeatType role) {
        switch (role) {
            case BeatType::SYNC_DROP:     return { 255, 75, 75 };   // Vibrant Red Section Drop
            case BeatType::DOWNBEAT:      return { 255, 180, 50 };  // Gold Bar Starts
            case BeatType::SUBDIVISION:   return { 90, 95, 110 };   // Dark slate blue eighths
            default:                      return { 240, 240, 245 }; // Clean White standard upbeats
        }
    }

    void TimelineLayer::onPanLeft(cocos2d::CCObject* sender) {
        m_scrollViewportX -= 1000.0f;
        if (m_scrollViewportX < 0.0f) m_scrollViewportX = 0.0f;
        refreshGridElements();
    }

    void TimelineLayer::onPanRight(cocos2d::CCObject* sender) {
        m_scrollViewportX += 1000.0f;
        refreshGridElements();
    }

    void TimelineLayer::onSyncConfigSync(cocos2d::CCObject* sender) {
        CO_LOG_INFO("Timeline panel manually re-aligning tracking matrices to core...");
        
        // Extract parameters dynamically from game settings context profile bounds
        float savedBPM = static_cast<float>(Mod::get()->getSettingValue<double>("bpm"));
        SyncEngine::get()->configureAudioSync(savedBPM, 0); // Re-map coordinates
        
        refreshGridElements();
    }
}

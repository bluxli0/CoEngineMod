#include "DashboardLayer.hpp"
#include <Core/EngineCommandBus.hpp>
#include <Core/UndoRedoManager.hpp>
#include <Core/TaskManager.hpp>

using namespace geode::prelude;

namespace CoEngine {

    DashboardLayer* DashboardLayer::create(std::string const& activePresetName) {
        auto instance = new DashboardLayer();
        // Sets popup dimensions: Width = 380, Height = 250
        if (instance && instance->initAnchored(380.0f, 250.0f, activePresetName, "GJ_square01.png")) {
            instance->autorelease();
            return instance;
        }
        CC_SAFE_DELETE(instance);
        return nullptr;
    }

    bool DashboardLayer::setup(std::string const& activePresetName) {
        auto windowSize = m_bgSprite->getContentSize();
        
        // Configure standard Geode window properties
        this->setTitle("CoEngine Dashboard");
        this->setID("CoEngineDashboardPopup");

        // Build UI Sub-Sections
        createBrandingHeader(windowSize);
        createStatusPanel(windowSize);
        createControlButtons(windowSize);

        // Schedule a high-frequency update tick to track background task allocations
        this->schedule(schedule_selector(DashboardLayer::updateUIState));

        CO_LOG_INFO("DashboardLayer UI loaded successfully with preset context: %s", activePresetName.c_str());
        return true;
    }

    void DashboardLayer::createBrandingHeader(cocos2d::CCSize windowSize) {
        // Safe texture lookup: tries your custom logo, falls back to stock gold star if missing
        auto logoSprite = cocos2d::CCSprite::createWithSpriteFrameName("CoEngine_logo.png");
        if (!logoSprite) {
            logoSprite = cocos2d::CCSprite::createWithSpriteFrameName("achievement_01.png");
        }

        if (logoSprite) {
            logoSprite->setPosition({ windowSize.width / 2.0f, windowSize.height - 35.0f });
            logoSprite->setScale(0.6f);
            m_mainLayer->addChild(logoSprite);
        }
    }

    void DashboardLayer::createStatusPanel(cocos2d::CCSize windowSize) {
        // Engine status display
        m_statusLabel = cocos2d::CCLabelBMFont::create("Status: Idle", "bigFont.fnt");
        m_statusLabel->setPosition({ windowSize.width / 2.0f, windowSize.height - 75.0f });
        m_statusLabel->setScale(0.4f);
        m_mainLayer->addChild(m_statusLabel);

        // Async percentage metric label
        m_progressLabel = cocos2d::CCLabelBMFont::create("Progress: 0.0%", "goldFont.fnt");
        m_progressLabel->setPosition({ windowSize.width / 2.0f, windowSize.height - 95.0f });
        m_progressLabel->setScale(0.45f);
        m_progressLabel->setVisible(false); // Only reveal when actively building
        m_mainLayer->addChild(m_progressLabel);
    }

    void DashboardLayer::createControlButtons(cocos2d::CCSize windowSize) {
        auto actionMenu = cocos2d::CCMenu::create();
        actionMenu->setPosition({ 0.0f, 0.0f });
        m_mainLayer->addChild(actionMenu);

        // 1. Primary Action: Generate Button
        m_generateBtnSprite = geode::ButtonSprite::create("GENERATE", "goldFont.fnt", "GJ_button_01.png");
        auto generateBtn = geode::CCMenuItemSpriteExtra::create(
            m_generateBtnSprite, this, menu_selector(DashboardLayer::onGenerate)
        );
        generateBtn->setPosition({ windowSize.width / 2.0f, 45.0f });
        actionMenu->addChild(generateBtn);

        // 2. Transaction Control: Undo Button
        auto undoSprite = geode::ButtonSprite::create("Undo", "bigFont.fnt", "GJ_button_02.png");
        undoSprite->setScale(0.6f);
        auto undoBtn = geode::CCMenuItemSpriteExtra::create(
            undoSprite, this, menu_selector(DashboardLayer::onUndo)
        );
        undoBtn->setPosition({ (windowSize.width / 2.0f) - 95.0f, 45.0f });
        actionMenu->addChild(undoBtn);

        // 3. Transaction Control: Redo Button
        auto redoSprite = geode::ButtonSprite::create("Redo", "bigFont.fnt", "GJ_button_02.png");
        redoSprite->setScale(0.6f);
        auto redoBtn = geode::CCMenuItemSpriteExtra::create(
            redoSprite, this, menu_selector(DashboardLayer::onRedo)
        );
        redoBtn->setPosition({ (windowSize.width / 2.0f) + 95.0f, 45.0f });
        actionMenu->addChild(redoBtn);

        // 4. Advanced Layer: View Timeline Panel Hook
        auto timelineSprite = geode::ButtonSprite::create("Timeline Editor", "bigFont.fnt", "GJ_button_04.png");
        timelineSprite->setScale(0.5f);
        auto timelineBtn = geode::CCMenuItemSpriteExtra::create(
            timelineSprite, this, menu_selector(DashboardLayer::onOpenTimeline)
        );
        timelineBtn->setPosition({ windowSize.width / 2.0f, 95.0f });
        actionMenu->addChild(timelineBtn);
    }

    void DashboardLayer::updateUIState(float dt) {
        auto taskManager = TaskManager::get();

        if (taskManager->isProcessing()) {
            m_statusLabel->setString("Status: Assembling Chunks...");
            m_statusLabel->setColor({ 0, 255, 100 }); // Turn green during active run
            
            m_progressLabel->setVisible(true);
            m_progressLabel->setString(fmt::format("Progress: {:.1f}%", taskManager->getCompletionPercentage()).c_str());
            
            m_generateBtnSprite->setColor({ 100, 100, 100 }); // Gray out button while running
        } else {
            m_statusLabel->setString("Status: Ready");
            m_statusLabel->setColor({ 255, 255, 255 });
            m_progressLabel->setVisible(false);
            m_generateBtnSprite->setColor({ 255, 255, 255 });
        }
    }

    void DashboardLayer::onGenerate(cocos2d::CCObject* sender) {
        if (TaskManager::get()->isProcessing()) return;

        CO_LOG_INFO("Dashboard UI requested engine generation event broadcast.");
        
        // Populate and pass parameters via the decoupled command packet
        CommandContext context;
        context.startX = 0.0f;
        context.endX = 5000.0f; // Standard default construction segment length
        context.targetGamemode = 0; // Cube mode execution default

        EngineCommandBus::get()->dispatch(CommandID::TRIGGER_GENERATION, context);
    }

    void DashboardLayer::onUndo(cocos2d::CCObject* sender) {
        auto currentEditor = geode::LevelEditorLayer::get();
        if (currentEditor) {
            UndoRedoManager::get()->executeUndo(currentEditor);
        }
    }

    void DashboardLayer::onRedo(cocos2d::CCObject* sender) {
        auto currentEditor = geode::LevelEditorLayer::get();
        if (currentEditor) {
            UndoRedoManager::get()->executeRedo(currentEditor);
        }
    }

    void DashboardLayer::onOpenTimeline(cocos2d::CCObject* sender) {
        // Safety warning fallback until TimelineLayer file logic is explicitly written
        geode::FLAlertLayer::create("Timeline", "Timeline layout context is currently spinning up.", "OK")->show();
    }
}

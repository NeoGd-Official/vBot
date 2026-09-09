#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/ui/Popup.hpp>

#include <filesystem>

using namespace geode::prelude;
namespace fs = std::filesystem;

class VBotPopup : public geode::Popup {
protected:
    bool init() {
        if (!Popup::init(720.f, 400.f))
            return false;

        this->setTitle("vBot");

        auto size = m_mainLayer->getContentSize();

        // =====================================================
        // LEFT PANEL - BOT / MACRO
        // =====================================================

        auto leftPanel = CCLayerColor::create(
            ccc4(30, 40, 85, 230),
            330.f,
            300.f
        );

        leftPanel->setPosition(15.f, 55.f);
        m_mainLayer->addChild(leftPanel);

        auto macroTitle = CCLabelBMFont::create(
            "BOT / MACRO",
            "bigFont.fnt"
        );

        macroTitle->setScale(0.65f);
        macroTitle->setPosition(165.f, 270.f);
        leftPanel->addChild(macroTitle);

        auto recordButton = makeButton(
            "Record",
            menu_selector(VBotPopup::onRecord)
        );

        auto playbackButton = makeButton(
            "Playback",
            menu_selector(VBotPopup::onPlayback)
        );

        auto pauseButton = makeButton(
            "Pause",
            menu_selector(VBotPopup::onPause)
        );

        auto stopButton = makeButton(
            "Stop",
            menu_selector(VBotPopup::onStop)
        );

        recordButton->setPosition(90.f, 215.f);
        playbackButton->setPosition(240.f, 215.f);
        pauseButton->setPosition(90.f, 160.f);
        stopButton->setPosition(240.f, 160.f);

        leftPanel->addChild(recordButton);
        leftPanel->addChild(playbackButton);
        leftPanel->addChild(pauseButton);
        leftPanel->addChild(stopButton);

        auto saveButton = makeButton(
            "Save Macro",
            menu_selector(VBotPopup::onSave)
        );

        auto loadButton = makeButton(
            "Load Macro",
            menu_selector(VBotPopup::onLoad)
        );

        auto editButton = makeButton(
            "Edit Macro",
            menu_selector(VBotPopup::onEdit)
        );

        saveButton->setPosition(75.f, 95.f);
        loadButton->setPosition(165.f, 95.f);
        editButton->setPosition(255.f, 95.f);

        leftPanel->addChild(saveButton);
        leftPanel->addChild(loadButton);
        leftPanel->addChild(editButton);

        auto retryButton = makeButton(
            "Auto Retry",
            menu_selector(VBotPopup::onAutoRetry)
        );

        retryButton->setPosition(165.f, 40.f);
        leftPanel->addChild(retryButton);

        // =====================================================
        // RIGHT PANEL - SETTINGS
        // =====================================================

        auto rightPanel = CCLayerColor::create(
            ccc4(30, 40, 85, 230),
            330.f,
            300.f
        );

        rightPanel->setPosition(375.f, 55.f);
        m_mainLayer->addChild(rightPanel);

        auto settingsTitle = CCLabelBMFont::create(
            "SETTINGS",
            "bigFont.fnt"
        );

        settingsTitle->setScale(0.65f);
        settingsTitle->setPosition(165.f, 270.f);
        rightPanel->addChild(settingsTitle);

        auto tpsButton = makeButton(
            "TPS Bypass",
            menu_selector(VBotPopup::onTPS)
        );

        auto speedButton = makeButton(
            "Speedhack",
            menu_selector(VBotPopup::onSpeedhack)
        );

        auto noclipButton = makeButton(
            "Noclip",
            menu_selector(VBotPopup::onNoclip)
        );

        auto trajectoryButton = makeButton(
            "Trajectory",
            menu_selector(VBotPopup::onTrajectory)
        );

        tpsButton->setPosition(90.f, 215.f);
        speedButton->setPosition(240.f, 215.f);
        noclipButton->setPosition(90.f, 160.f);
        trajectoryButton->setPosition(240.f, 160.f);

        rightPanel->addChild(tpsButton);
        rightPanel->addChild(speedButton);
        rightPanel->addChild(noclipButton);
        rightPanel->addChild(trajectoryButton);

        auto frameButton = makeButton(
            "Frame Stepper",
            menu_selector(VBotPopup::onFrameStepper)
        );

        auto physicsButton = makeButton(
            "2.1 Physics",
            menu_selector(VBotPopup::onPhysics)
        );

        auto inputButton = makeButton(
            "Input Display",
            menu_selector(VBotPopup::onInputDisplay)
        );

        auto hitboxButton = makeButton(
            "Hitboxes",
            menu_selector(VBotPopup::onHitboxes)
        );

        frameButton->setPosition(90.f, 100.f);
        physicsButton->setPosition(240.f, 100.f);
        inputButton->setPosition(90.f, 45.f);
        hitboxButton->setPosition(240.f, 45.f);

        rightPanel->addChild(frameButton);
        rightPanel->addChild(physicsButton);
        rightPanel->addChild(inputButton);
        rightPanel->addChild(hitboxButton);

        // =====================================================
        // STATUS
        // =====================================================

        auto status = CCLabelBMFont::create(
            "vBot | Ready",
            "bigFont.fnt"
        );

        status->setScale(0.45f);
        status->setPosition(
            size.width / 2.f,
            28.f
        );

        m_mainLayer->addChild(status);

        return true;
    }

    CCMenuItemSpriteExtra* makeButton(
        const char* text,
        SEL_MenuHandler callback
    ) {
        auto sprite = ButtonSprite::create(text);
        sprite->setScale(0.62f);

        return CCMenuItemSpriteExtra::create(
            sprite,
            this,
            callback
        );
    }

    // =========================================================
    // BOT
    // =========================================================

    void onRecord(CCObject*) {
        log::info("vBot: Record pressed!");
    }

    void onPlayback(CCObject*) {
        log::info("vBot: Playback pressed!");
    }

    void onPause(CCObject*) {
        log::info("vBot: Pause pressed!");
    }

    void onStop(CCObject*) {
        log::info("vBot: Stop pressed!");
    }

    void onSave(CCObject*) {
        log::info("vBot: Save Macro pressed!");
    }

    void onLoad(CCObject*) {
        log::info("vBot: Load Macro pressed!");
    }

    void onEdit(CCObject*) {
        log::info("vBot: Edit Macro pressed!");
    }

    void onAutoRetry(CCObject*) {
        log::info("vBot: Auto Retry pressed!");
    }

    // =========================================================
    // SETTINGS
    // =========================================================

    void onTPS(CCObject*) {
        log::info("vBot: TPS Bypass pressed!");
    }

    void onSpeedhack(CCObject*) {
        log::info("vBot: Speedhack pressed!");
    }

    void onNoclip(CCObject*) {
        log::info("vBot: Noclip pressed!");
    }

    void onTrajectory(CCObject*) {
        log::info("vBot: Trajectory pressed!");
    }

    void onFrameStepper(CCObject*) {
        log::info("vBot: Frame Stepper pressed!");
    }

    void onPhysics(CCObject*) {
        log::info("vBot: 2.1 Physics pressed!");
    }

    void onInputDisplay(CCObject*) {
        log::info("vBot: Input Display pressed!");
    }

    void onHitboxes(CCObject*) {
        log::info("vBot: Hitboxes pressed!");
    }

public:
    static VBotPopup* create() {
        auto ret = new VBotPopup();

        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }

        delete ret;
        return nullptr;
    }
};


// =============================================================
// vBot MENU BUTTON
// =============================================================

class $modify(VBotMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init())
            return false;

        auto winSize =
            CCDirector::sharedDirector()->getWinSize();

        auto menu = CCMenu::create();

        menu->setPosition(
            winSize.width - 45.f,
            winSize.height - 45.f
        );

        auto icon = CCSprite::create(
            "vbot-button.png"_spr
        );

        if (!icon) {
            log::error(
                "vBot: Failed to load vbot-button.png"
            );
            return true;
        }

        icon->setScale(0.18f);

        auto button = CCMenuItemSpriteExtra::create(
            icon,
            this,
            menu_selector(VBotMenuLayer::onVBot)
        );

        menu->addChild(button);
        this->addChild(menu);

        return true;
    }

    void onVBot(CCObject*) {
        auto popup = VBotPopup::create();

        if (popup)
            popup->show();
    }
};


// =============================================================
// CREATE vBot FOLDERS
// =============================================================

$on_mod(Loaded) {
    log::info("vBot loaded successfully!");

    auto saveDir = Mod::get()->getSaveDir();

    std::error_code error;

    fs::create_directories(saveDir / "renders", error);
    fs::create_directories(saveDir / "macros", error);
    fs::create_directories(saveDir / "configs", error);
    fs::create_directories(saveDir / "logs", error);
    fs::create_directories(saveDir / "cache", error);

    if (error) {
        log::error(
            "vBot: Failed to create folders: {}",
            error.message()
        );
    }
    else {
        log::info("vBot: Storage folders ready!");
    }
}

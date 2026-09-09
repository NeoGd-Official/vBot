#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/ui/Popup.hpp>

using namespace geode::prelude;

class VBotPopup : public geode::Popup<> {
protected:
    bool init() {
        if (!Popup::init(280.f, 180.f))
            return false;

        this->setTitle("vBot");

        auto status = CCLabelBMFont::create(
            "Bot: Ready",
            "bigFont.fnt"
        );

        status->setScale(0.6f);
        status->setPosition(
            m_mainLayer->getContentSize() / 2.f
            + CCPoint{0.f, 35.f}
        );

        m_mainLayer->addChild(status);

        auto startButton = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Start Bot"),
            this,
            menu_selector(VBotPopup::onStart)
        );

        auto stopButton = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Stop Bot"),
            this,
            menu_selector(VBotPopup::onStop)
        );

        auto menu = CCMenu::create();

        startButton->setPosition(-55.f, -20.f);
        stopButton->setPosition(55.f, -20.f);

        menu->addChild(startButton);
        menu->addChild(stopButton);

        menu->setPosition(
            m_mainLayer->getContentSize() / 2.f
        );

        m_mainLayer->addChild(menu);

        return true;
    }

    void onStart(CCObject*) {
        log::info("vBot: Start pressed!");

        FLAlertLayer::create(
            "vBot",
            "Bot started!",
            "OK"
        )->show();
    }

    void onStop(CCObject*) {
        log::info("vBot: Stop pressed!");

        FLAlertLayer::create(
            "vBot",
            "Bot stopped!",
            "OK"
        )->show();
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


class $modify(VBotMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init())
            return false;

        auto winSize = CCDirector::sharedDirector()->getWinSize();

        auto menu = CCMenu::create();

        menu->setPosition(
            winSize.width - 45.f,
            winSize.height - 45.f
        );

        // Your custom vBot icon
        auto icon = CCSprite::create(
            "vbot-button.png"_spr
        );

        if (!icon) {
            log::error("vBot: Failed to load vbot-button.png");
            return true;
        }

        // Keep the icon a reasonable size
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


$on_mod(Loaded) {
    log::info("vBot loaded successfully!");
}

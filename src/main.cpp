#include <Geode/Geode.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <optional>
#include <algorithm>
#include <system_error>

using namespace geode::prelude;
namespace fs = std::filesystem;


// =============================================================
// vBot Macro Data
// =============================================================

struct VBotInput {
    int frame;
    std::string input;
};

struct VBotMacro {
    int tps = 240;
    bool physics21 = false;
    bool frameFix = true;
    std::vector<VBotInput> inputs;
};


// =============================================================
// vBot Runtime State
// =============================================================

namespace VBotState {

static VBotMacro currentMacro;

static bool recording = false;
static bool recordingPaused = false;
static bool playing = false;

static int currentFrame = 0;

}


// =============================================================
// .vbr ENCODER / DECODER
// =============================================================

namespace VBR {

static std::string encodeInput(const std::string& input) {
    if (input == "jump")
        return "J";

    if (input == "release")
        return "R";

    if (input == "left")
        return "L";

    if (input == "right")
        return "D";

    if (input == "up")
        return "U";

    if (input == "down")
        return "N";

    return input;
}

static std::string decodeInput(const std::string& input) {
    if (input == "J")
        return "jump";

    if (input == "R")
        return "release";

    if (input == "L")
        return "left";

    if (input == "D")
        return "right";

    if (input == "U")
        return "up";

    if (input == "N")
        return "down";

    return input;
}


static std::string encode(const VBotMacro& macro) {
    std::ostringstream out;

    out << "Bot:vBot\n";
    out << "TPS:" << macro.tps << "\n";

    // Only meaningful when enabled.
    out << "Physics2.1:"
        << (macro.physics21 ? "Yes" : "No")
        << "\n";

    out << "FrameFix:"
        << (macro.frameFix ? "Yes" : "No")
        << "\n";

    out << "Inputs:\n";

    for (const auto& input : macro.inputs) {
        out
            << input.frame
            << "="
            << encodeInput(input.input)
            << "\n";
    }

    return out.str();
}


static std::optional<VBotMacro> decode(
    const std::string& data
) {
    std::istringstream stream(data);

    std::string line;

    VBotMacro macro;

    bool validBot = false;
    bool foundInputs = false;

    while (std::getline(stream, line)) {

        if (line == "Bot:vBot") {
            validBot = true;
            continue;
        }

        if (line.rfind("TPS:", 0) == 0) {
            try {
                macro.tps =
                    std::stoi(line.substr(4));
            }
            catch (...) {
                return std::nullopt;
            }

            continue;
        }

        if (line.rfind("Physics2.1:", 0) == 0) {
            macro.physics21 =
                line.substr(11) == "Yes";

            continue;
        }

        if (line.rfind("FrameFix:", 0) == 0) {
            macro.frameFix =
                line.substr(9) == "Yes";

            continue;
        }

        if (line == "Inputs:") {
            foundInputs = true;
            continue;
        }

        if (!foundInputs || line.empty())
            continue;

        auto separator = line.find('=');

        if (separator == std::string::npos)
            continue;

        try {
            int frame =
                std::stoi(
                    line.substr(0, separator)
                );

            std::string input =
                line.substr(separator + 1);

            if (input.empty())
                continue;

            macro.inputs.push_back({
                frame,
                decodeInput(input)
            });
        }
        catch (...) {
            return std::nullopt;
        }
    }

    if (!validBot || !foundInputs)
        return std::nullopt;

    if (macro.tps <= 0)
        macro.tps = 240;

    return macro;
}

}


// =============================================================
// Storage
// =============================================================

static fs::path getMacroDirectory() {
    auto path =
        Mod::get()->getSaveDir() / "macros";

    std::error_code error;

    fs::create_directories(path, error);

    return path;
}


static fs::path getRenderDirectory() {
    auto path =
        Mod::get()->getSaveDir() / "renders";

    std::error_code error;

    fs::create_directories(path, error);

    return path;
}


static bool saveMacro(
    const fs::path& file
) {
    std::ofstream output(file);

    if (!output.is_open()) {
        log::error(
            "vBot: Could not write {}",
            file.string()
        );

        return false;
    }

    output << VBR::encode(
        VBotState::currentMacro
    );

    output.close();

    log::info(
        "vBot: Saved .vbr: {}",
        file.string()
    );

    return true;
}


static bool loadMacro(
    const fs::path& file
) {
    std::ifstream input(file);

    if (!input.is_open()) {
        log::error(
            "vBot: Could not read {}",
            file.string()
        );

        return false;
    }

    std::stringstream buffer;

    buffer << input.rdbuf();

    auto decoded =
        VBR::decode(buffer.str());

    if (!decoded) {
        log::error(
            "vBot: Invalid .vbr file."
        );

        return false;
    }

    VBotState::currentMacro =
        *decoded;

    log::info(
        "vBot: Loaded macro. TPS={}, Inputs={}",
        VBotState::currentMacro.tps,
        VBotState::currentMacro.inputs.size()
    );

    return true;
}


// =============================================================
// TPS INPUT POPUP
// =============================================================

class TPSEditPopup : public geode::Popup {
protected:

    TextInput* m_input = nullptr;

    bool init() {

        if (!Popup::init(300.f, 180.f))
            return false;

        this->setTitle("TPS");

        auto size =
            m_mainLayer->getContentSize();

        auto label =
            CCLabelBMFont::create(
                "Enter TPS:",
                "bigFont.fnt"
            );

        label->setScale(0.55f);

        label->setPosition(
            size.width / 2.f,
            115.f
        );

        m_mainLayer->addChild(label);

        m_input =
            TextInput::create(
                170.f,
                "TPS",
                "bigFont.fnt"
            );

        m_input->setCommonFilter(
            CommonFilter::Uint
        );

        m_input->setMaxCharCount(6);

        m_input->setString(
            std::to_string(
                VBotState::currentMacro.tps
            ),
            false
        );

        m_input->setPosition(
            size.width / 2.f,
            80.f
        );

        m_mainLayer->addChild(m_input);

        auto menu =
            CCMenu::create();

        menu->setPosition(
            size.width / 2.f,
            38.f
        );

        auto apply =
            CCMenuItemSpriteExtra::create(
                ButtonSprite::create(
                    "Apply"
                ),
                this,
                menu_selector(
                    TPSEditPopup::onApply
                )
            );

        menu->addChild(apply);

        m_mainLayer->addChild(menu);

        return true;
    }


    void onApply(CCObject*) {

        if (!m_input)
            return;

        int value = 0;

        try {
            value =
                std::stoi(
                    m_input->getString()
                );
        }
        catch (...) {
            value = 0;
        }

        // Keep TPS sane.
        value =
            std::clamp(
                value,
                1,
                10000
            );

        VBotState::currentMacro.tps =
            value;

        log::info(
            "vBot: TPS set to {}",
            value
        );

        this->onClose(nullptr);
    }


public:

    static TPSEditPopup* create() {

        auto ret =
            new TPSEditPopup();

        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }

        delete ret;
        return nullptr;
    }
};


// =============================================================
// vBot Popup
// =============================================================

class VBotPopup : public geode::Popup {

protected:

    CCLabelBMFont* m_status = nullptr;

    bool init() {

        auto winSize =
            CCDirector::sharedDirector()
                ->getWinSize();

        // 75% of the screen.
        // Leaves approximately 1/8 on every side.
        float popupWidth =
            winSize.width * 0.75f;

        float popupHeight =
            winSize.height * 0.75f;

        popupWidth =
            std::max(
                popupWidth,
                420.f
            );

        popupHeight =
            std::max(
                popupHeight,
                280.f
            );

        if (!Popup::init(
            popupWidth,
            popupHeight
        ))
            return false;

        this->setTitle("vBot");

        auto size =
            m_mainLayer->getContentSize();

        float panelGap = 10.f;

        float panelWidth =
            (size.width - panelGap * 3.f)
            / 2.f;

        float panelHeight =
            size.height - 70.f;

        // =====================================================
        // LEFT PANEL
        // =====================================================

        auto leftPanel =
            CCLayerColor::create(
                ccc4(
                    30,
                    40,
                    85,
                    230
                ),
                panelWidth,
                panelHeight
            );

        leftPanel->setPosition(
            panelGap,
            45.f
        );

        m_mainLayer->addChild(leftPanel);

        auto leftTitle =
            CCLabelBMFont::create(
                "BOT / MACRO",
                "bigFont.fnt"
            );

        leftTitle->setScale(0.55f);

        leftTitle->setPosition(
            panelWidth / 2.f,
            panelHeight - 28.f
        );

        leftPanel->addChild(leftTitle);

        // IMPORTANT:
        // CCMenuItemSpriteExtra objects MUST be
        // inside a CCMenu to receive menu input.

        auto leftMenu =
            CCMenu::create();

        leftMenu->setPosition(
            0.f,
            0.f
        );

        leftPanel->addChild(
            leftMenu
        );

        auto record =
            makeButton(
                "Record",
                menu_selector(
                    VBotPopup::onRecord
                )
            );

        auto playback =
            makeButton(
                "Playback",
                menu_selector(
                    VBotPopup::onPlayback
                )
            );

        auto pause =
            makeButton(
                "Pause",
                menu_selector(
                    VBotPopup::onPause
                )
            );

        auto stop =
            makeButton(
                "Stop",
                menu_selector(
                    VBotPopup::onStop
                )
            );

        float x1 =
            panelWidth * 0.28f;

        float x2 =
            panelWidth * 0.72f;

        float y1 =
            panelHeight - 85.f;

        float y2 =
            panelHeight - 135.f;

        record->setPosition(
            x1,
            y1
        );

        playback->setPosition(
            x2,
            y1
        );

        pause->setPosition(
            x1,
            y2
        );

        stop->setPosition(
            x2,
            y2
        );

        leftMenu->addChild(record);
        leftMenu->addChild(playback);
        leftMenu->addChild(pause);
        leftMenu->addChild(stop);

        auto save =
            makeButton(
                "Save Macro",
                menu_selector(
                    VBotPopup::onSave
                )
            );

        auto load =
            makeButton(
                "Load Macro",
                menu_selector(
                    VBotPopup::onLoad
                )
            );

        auto edit =
            makeButton(
                "Edit Macro",
                menu_selector(
                    VBotPopup::onEdit
                )
            );

        save->setPosition(
            panelWidth * 0.22f,
            panelHeight * 0.29f
        );

        load->setPosition(
            panelWidth * 0.50f,
            panelHeight * 0.29f
        );

        edit->setPosition(
            panelWidth * 0.78f,
            panelHeight * 0.29f
        );

        leftMenu->addChild(save);
        leftMenu->addChild(load);
        leftMenu->addChild(edit);

        auto retry =
            makeButton(
                "Auto Retry",
                menu_selector(
                    VBotPopup::onAutoRetry
                )
            );

        retry->setPosition(
            panelWidth / 2.f,
            panelHeight * 0.12f
        );

        leftMenu->addChild(retry);

        // =====================================================
        // RIGHT PANEL
        // =====================================================

        auto rightPanel =
            CCLayerColor::create(
                ccc4(
                    30,
                    40,
                    85,
                    230
                ),
                panelWidth,
                panelHeight
            );

        rightPanel->setPosition(
            panelGap * 2.f
                + panelWidth,
            45.f
        );

        m_mainLayer->addChild(
            rightPanel
        );

        auto rightTitle =
            CCLabelBMFont::create(
                "SETTINGS",
                "bigFont.fnt"
            );

        rightTitle->setScale(0.55f);

        rightTitle->setPosition(
            panelWidth / 2.f,
            panelHeight - 28.f
        );

        rightPanel->addChild(
            rightTitle
        );

        auto rightMenu =
            CCMenu::create();

        rightMenu->setPosition(
            0.f,
            0.f
        );

        rightPanel->addChild(
            rightMenu
        );

        // -----------------------------------------------------
        // TPS
        // -----------------------------------------------------

        auto tps =
            makeButton(
                "TPS Bypass",
                menu_selector(
                    VBotPopup::onTPS
                )
            );

        auto speedhack =
            makeButton(
                "Speedhack",
                menu_selector(
                    VBotPopup::onSpeedhack
                )
            );

        auto noclip =
            makeButton(
                "Noclip",
                menu_selector(
                    VBotPopup::onNoclip
                )
            );

        auto trajectory =
            makeButton(
                "Trajectory",
                menu_selector(
                    VBotPopup::onTrajectory
                )
            );

        tps->setPosition(
            x1,
            y1
        );

        speedhack->setPosition(
            x2,
            y1
        );

        noclip->setPosition(
            x1,
            y2
        );

        trajectory->setPosition(
            x2,
            y2
        );

        rightMenu->addChild(tps);
        rightMenu->addChild(speedhack);
        rightMenu->addChild(noclip);
        rightMenu->addChild(trajectory);

        auto frame =
            makeButton(
                "Frame Stepper",
                menu_selector(
                    VBotPopup::onFrameStepper
                )
            );

        auto physics =
            makeButton(
                "2.1 Physics",
                menu_selector(
                    VBotPopup::onPhysics
                )
            );

        auto input =
            makeButton(
                "Input Display",
                menu_selector(
                    VBotPopup::onInputDisplay
                )
            );

        auto hitboxes =
            makeButton(
                "Hitboxes",
                menu_selector(
                    VBotPopup::onHitboxes
                )
            );

        frame->setPosition(
            x1,
            panelHeight * 0.29f
        );

        physics->setPosition(
            x2,
            panelHeight * 0.29f
        );

        input->setPosition(
            x1,
            panelHeight * 0.12f
        );

        hitboxes->setPosition(
            x2,
            panelHeight * 0.12f
        );

        rightMenu->addChild(frame);
        rightMenu->addChild(physics);
        rightMenu->addChild(input);
        rightMenu->addChild(hitboxes);

        // =====================================================
        // STATUS
        // =====================================================

        m_status =
            CCLabelBMFont::create(
                "vBot | Ready",
                "bigFont.fnt"
            );

        m_status->setScale(0.38f);

        m_status->setPosition(
            size.width / 2.f,
            22.f
        );

        m_mainLayer->addChild(
            m_status
        );

        return true;
    }


    CCMenuItemSpriteExtra* makeButton(
        const char* text,
        SEL_MenuHandler callback
    ) {
        auto sprite =
            ButtonSprite::create(
                text
            );

        sprite->setScale(
            0.52f
        );

        return CCMenuItemSpriteExtra::create(
            sprite,
            this,
            callback
        );
    }


    void setStatus(
        const std::string& text
    ) {
        if (!m_status)
            return;

        m_status->setString(
            text.c_str()
        );
    }


    // =========================================================
    // RECORDING
    // =========================================================

    void onRecord(CCObject*) {

        VBotState::currentMacro.inputs.clear();

        VBotState::currentFrame = 0;

        VBotState::recording = true;

        VBotState::recordingPaused = false;

        VBotState::playing = false;

        setStatus(
            "vBot | Recording"
        );

        log::info(
            "vBot: Recording started."
        );
    }


    void onPlayback(CCObject*) {

        if (
            VBotState::currentMacro
                .inputs.empty()
        ) {
            setStatus(
                "vBot | No macro loaded"
            );

            log::warn(
                "vBot: Playback requested with no inputs."
            );

            return;
        }

        VBotState::playing = true;

        VBotState::recording = false;

        setStatus(
            "vBot | Playing"
        );

        log::info(
            "vBot: Playback started."
        );
    }


    void onPause(CCObject*) {

        if (!VBotState::recording) {
            log::info(
                "vBot: Nothing is recording."
            );

            return;
        }

        VBotState::recordingPaused =
            !VBotState::recordingPaused;

        setStatus(
            VBotState::recordingPaused
                ? "vBot | Recording Paused"
                : "vBot | Recording"
        );
    }


    void onStop(CCObject*) {

        VBotState::recording = false;

        VBotState::recordingPaused = false;

        VBotState::playing = false;

        setStatus(
            "vBot | Stopped"
        );

        log::info(
            "vBot: Recording/playback stopped."
        );
    }


    // =========================================================
    // FILES
    // =========================================================

    void onSave(CCObject*) {

        auto file =
            getMacroDirectory()
            / "macro.vbr";

        if (saveMacro(file)) {

            setStatus(
                "vBot | Saved"
            );
        }
        else {

            setStatus(
                "vBot | Save failed"
            );
        }
    }


    void onLoad(CCObject*) {

        auto file =
            getMacroDirectory()
            / "macro.vbr";

        if (loadMacro(file)) {

            setStatus(
                "vBot | Loaded"
            );
        }
        else {

            setStatus(
                "vBot | Load failed"
            );
        }
    }


    void onEdit(CCObject*) {

        log::info(
            "vBot: .vBot editor requested."
        );

        FLAlertLayer::create(
            "vBot Macro Editor",
            "The .vBot editor uses raw JavaScript text. "
            "Use the web converter to convert a .vbr "
            "macro into editable .vBot JavaScript.",
            "OK"
        )->show();
    }


    void onAutoRetry(CCObject*) {

        log::info(
            "vBot: Auto Retry enabled."
        );

        setStatus(
            "vBot | Auto Retry"
        );
    }


    // =========================================================
    // SETTINGS
    // =========================================================

    void onTPS(CCObject*) {

        auto popup =
            TPSEditPopup::create();

        if (popup)
            popup->show();
    }


    void onSpeedhack(CCObject*) {

        log::info(
            "vBot: Speedhack toggled."
        );

        setStatus(
            "vBot | Speedhack"
        );
    }


    void onNoclip(CCObject*) {

        log::info(
            "vBot: Noclip toggled."
        );

        setStatus(
            "vBot | Noclip"
        );
    }


    void onTrajectory(CCObject*) {

        log::info(
            "vBot: Trajectory toggled."
        );

        setStatus(
            "vBot | Trajectory"
        );
    }


    void onFrameStepper(CCObject*) {

        log::info(
            "vBot: Frame Stepper enabled."
        );

        setStatus(
            "vBot | Frame Stepper"
        );
    }


    void onPhysics(CCObject*) {

        VBotState::currentMacro.physics21 =
            !VBotState::currentMacro.physics21;

        setStatus(
            VBotState::currentMacro.physics21
                ? "vBot | 2.1 Physics ON"
                : "vBot | 2.1 Physics OFF"
        );

        log::info(
            "vBot: 2.1 Physics = {}",
            VBotState::currentMacro.physics21
        );
    }


    void onInputDisplay(CCObject*) {

        log::info(
            "vBot: Input Display toggled."
        );

        setStatus(
            "vBot | Input Display"
        );
    }


    void onHitboxes(CCObject*) {

        log::info(
            "vBot: Hitboxes toggled."
        );

        setStatus(
            "vBot | Hitboxes"
        );
    }


public:

    static VBotPopup* create() {

        auto ret =
            new VBotPopup();

        if (ret && ret->init()) {

            ret->autorelease();

            return ret;
        }

        delete ret;

        return nullptr;
    }
};


// =============================================================
// LEVEL PAUSE MENU ONLY
// =============================================================

class $modify(VBotPauseLayer, PauseLayer) {

    void customSetup() {

        PauseLayer::customSetup();

        auto winSize =
            CCDirector::sharedDirector()
                ->getWinSize();

        auto menu =
            CCMenu::create();

        menu->setPosition(
            winSize.width - 45.f,
            45.f
        );

        auto icon =
            CCSprite::create(
                "vbot-button.png"_spr
            );

        if (!icon) {

            log::error(
                "vBot: Failed to load vbot-button.png"
            );

            return;
        }

        icon->setScale(
            0.16f
        );

        auto button =
            CCMenuItemSpriteExtra::create(
                icon,
                this,
                menu_selector(
                    VBotPauseLayer::onVBot
                )
            );

        menu->addChild(button);

        this->addChild(
            menu,
            100
        );
    }


    void onVBot(CCObject*) {

        auto popup =
            VBotPopup::create();

        if (popup)
            popup->show();
    }
};


// =============================================================
// STORAGE
// =============================================================

$on_mod(Loaded) {

    log::info(
        "vBot loaded successfully!"
    );

    auto saveDir =
        Mod::get()->getSaveDir();

    std::error_code error;

    fs::create_directories(
        saveDir / "renders",
        error
    );

    fs::create_directories(
        saveDir / "macros",
        error
    );

    fs::create_directories(
        saveDir / "configs",
        error
    );

    fs::create_directories(
        saveDir / "logs",
        error
    );

    fs::create_directories(
        saveDir / "cache",
        error
    );

    if (error) {

        log::error(
            "vBot: Failed to create folders: {}",
            error.message()
        );
    }
    else {

        log::info(
            "vBot: Storage folders ready!"
        );
    }

    log::info(
        "vBot: Save directory: {}",
        saveDir.string()
    );

    log::info(
        "vBot: Macro directory: {}",
        (saveDir / "macros").string()
    );

    log::info(
        "vBot: Render directory: {}",
        (saveDir / "renders").string()
    );
}

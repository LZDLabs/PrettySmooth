// PrettySmooth - by LZ
// O menu roxo do PrettySmooth: presets, abas e todas as opções.

#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <Geode/ui/TextArea.hpp>
#include <Geode/ui/GeodeUI.hpp>
#include <Geode/ui/Notification.hpp>

#include "Theme.hpp"
#include "../Config.hpp"
#include "../Runtime.hpp"
#include "../Lang.hpp"

#include <algorithm>
#include <functional>

using namespace geode::prelude;

namespace {
    // Remove tags de cor tipo <cp> e </c> das descrições
    std::string stripTags(std::string const& text) {
        std::string out;
        out.reserve(text.size());
        for (size_t i = 0; i < text.size(); i++) {
            if (text[i] == '<') {
                auto end = text.find('>', i);
                if (end != std::string::npos && end - i <= 4) {
                    i = end;
                    continue;
                }
            }
            out += text[i] == '\n' ? ' ' : text[i];
        }
        return out;
    }

    std::string presetBlurb(std::string_view name) {
        auto key = std::string("blurb.") + std::string(name);
        return ps::tr(key);
    }

    std::string presetLabel(std::string_view name) {
        return ps::tr(std::string("preset.") + std::string(name));
    }

    // Valores do botão de FPS (pula direto para os mais comuns)
    std::vector<int64_t> const& fpsSteps() {
        static std::vector<int64_t> steps = {
            30, 45, 60, 75, 90, 120, 144, 165, 180, 200, 240, 280, 300, 360, 480, 540, 720, 1000
        };
        return steps;
    }

    std::string formatInt(std::string_view key, int64_t value) {
        if (key == "fps-target") return fmt::format("{}", value);
        if (key == "particle-amount") return fmt::format("{}%", value);
        if (key == "editor-throttle") return value <= 1 ? std::string("Normal") : fmt::format("1/{}", value);
        return fmt::format("{}", value);
    }

    struct TabDef {
        char const* name;
        std::vector<char const*> keys;
    };

    std::vector<TabDef> const& tabs() {
        static std::vector<TabDef> defs = {
            { "General", { "language", "fps-unlock", "fps-target", "vsync-off", "frame-pacing", "high-priority", "precise-timer", "fps-counter" } },
            { "Levels",  { "level-particles", "particle-amount", "no-shaders", "no-shake", "no-glow", "force-ldm",
                           "hide-background", "hide-middleground", "no-trails", "no-wave-trail" } },
            { "Menus",   { "menu-particles", "static-menu", "transitions" } },
            { "Editor",  { "editor-particles", "editor-no-glow", "editor-no-shaders", "editor-grid", "editor-throttle" } },
            { "System",  { "texture-quality", "low-textures", "clean-memory", "#clean-now" } },
        };
        return defs;
    }
}

class PSPopup : public geode::Popup {
protected:
    float m_width = 440.f;
    float m_height = 296.f;
    int m_currentTab = 0;

    std::vector<std::pair<std::string, ps::ui::PurpleButtonSprite*>> m_presetButtons;
    std::vector<ps::ui::PurpleButtonSprite*> m_tabButtons;
    geode::ScrollLayer* m_scroll = nullptr;
    SimpleTextArea* m_desc = nullptr;
    CCLabelBMFont* m_presetLabel = nullptr;
    CCMenu* m_contentMenu = nullptr;
    std::vector<std::function<void()>> m_refreshers;

    // (não se chama init() para não sobrescrever o CCLayer::init() virtual)
    bool setup() {
        auto win = CCDirector::sharedDirector()->getWinSize();
        m_width = std::min(440.f, win.width - 16.f);
        m_height = std::min(296.f, win.height - 16.f);

        if (!Popup::init(m_width, m_height, "square02b_001.png", { 0.f, 0.f, 80.f, 80.f })) return false;

        // Tudo roxo
        this->setColor(ps::theme::BG_DIM);
        this->setOpacity(150);
        m_bgSprite->setColor(ps::theme::BG);

        auto border = ps::ui::createPanel({ m_width + 4.f, m_height + 4.f }, ps::theme::BORDER);
        border->setPosition({ m_width / 2, m_height / 2 });
        m_mainLayer->addChild(border, -1);

        this->setCloseButtonSpr(
            ps::ui::PurpleButtonSprite::create("X", { 26.f, 26.f }, ps::theme::ACCENT_DARK, .55f)
        );

        this->buildHeader();
        this->buildPresetRow();
        this->buildTabs();
        this->buildContentArea();
        this->buildFooter();

        this->buildContent();
        this->refreshAll();
        this->setDescription(presetBlurb(ps::currentPreset()));

        this->scheduleUpdate();
        return true;
    }

    // Botões que rolaram para fora da área visível continuam existindo
    // (só não são desenhados). Desativa eles para não roubarem toques das
    // abas e dos presets.
    void update(float dt) override {
        if (!m_contentMenu || !m_scroll || !m_contentMenu->getChildren()) return;

        auto bottomLeft = m_scroll->convertToWorldSpace({ 0.f, 0.f });
        auto topRight = m_scroll->convertToWorldSpace(m_scroll->getContentSize());

        for (auto item : CCArrayExt<CCMenuItem*>(m_contentMenu->getChildren())) {
            auto pos = m_contentMenu->convertToWorldSpace(item->getPosition());
            bool inside = pos.y >= bottomLeft.y && pos.y <= topRight.y;
            if (item->isEnabled() != inside) item->setEnabled(inside);
        }
    }

    // ------------------------------------------------------------
    // Layout
    // ------------------------------------------------------------
    void buildHeader() {
        auto title = CCLabelBMFont::create("PrettySmooth", "bigFont.fnt");
        title->setScale(.7f);
        title->setColor(ps::theme::TEXT);
        title->setPosition({ m_width / 2, m_height - 19.f });
        m_mainLayer->addChild(title, 2);

        // "sombra" lilás do título
        auto glow = CCLabelBMFont::create("PrettySmooth", "bigFont.fnt");
        glow->setScale(.7f);
        glow->setColor(ps::theme::ACCENT);
        glow->setOpacity(160);
        glow->setPosition({ m_width / 2 + 1.5f, m_height - 20.5f });
        m_mainLayer->addChild(glow, 1);

        auto dev = CCLabelBMFont::create(ps::tr("ui.by").c_str(), "bigFont.fnt");
        dev->setScale(.3f);
        dev->setColor(ps::theme::TEXT_MUTED);
        dev->setPosition({ m_width / 2, m_height - 36.f });
        m_mainLayer->addChild(dev, 2);

        // Botão para as configurações do Geode (mesmas opções, lista completa)
        auto menu = CCMenu::create();
        menu->setPosition({ 0.f, 0.f });
        menu->setContentSize({ m_width, m_height });
        auto geodeSpr = ps::ui::PurpleButtonSprite::create("Geode", { 52.f, 18.f }, ps::theme::ACCENT_DARK, .35f);
        auto geodeBtn = CCMenuItemExt::createSpriteExtra(geodeSpr, [](CCMenuItemSpriteExtra*) {
            geode::openSettingsPopup(Mod::get());
        });
        geodeBtn->setPosition({ m_width - 36.f, m_height - 19.f });
        menu->addChild(geodeBtn);
        m_mainLayer->addChild(menu, 5);
    }

    void buildPresetRow() {
        auto const& names = ps::presetNames();
        float margin = 15.f;
        float gap = 4.f;
        float count = static_cast<float>(names.size());
        float w = (m_width - margin * 2 - gap * (count - 1)) / count;
        float y = m_height - 62.f;

        auto menu = CCMenu::create();
        menu->setPosition({ 0.f, 0.f });
        menu->setContentSize({ m_width, m_height });

        for (size_t i = 0; i < names.size(); i++) {
            auto const& name = names[i];
            auto spr = ps::ui::PurpleButtonSprite::create(presetLabel(name), { w, 24.f }, ps::theme::ACCENT_DARK, .4f);
            auto btn = CCMenuItemExt::createSpriteExtra(spr, [this, name](CCMenuItemSpriteExtra*) {
                this->onPreset(name);
            });
            btn->setPosition({ margin + w / 2 + i * (w + gap), y });
            menu->addChild(btn);
            m_presetButtons.push_back({ name, spr });
        }
        m_mainLayer->addChild(menu, 5);
    }

    void buildTabs() {
        auto const& defs = tabs();
        float margin = 15.f;
        float gap = 3.f;
        float count = static_cast<float>(defs.size());
        float w = (m_width - margin * 2 - gap * (count - 1)) / count;
        float y = m_height - 91.f;

        auto menu = CCMenu::create();
        menu->setPosition({ 0.f, 0.f });
        menu->setContentSize({ m_width, m_height });

        for (size_t i = 0; i < defs.size(); i++) {
            auto spr = ps::ui::PurpleButtonSprite::create(ps::tr(std::string("tab.") + defs[i].name), { w, 20.f }, ps::theme::OFF, .38f);
            auto btn = CCMenuItemExt::createSpriteExtra(spr, [this, i](CCMenuItemSpriteExtra*) {
                if (m_currentTab == static_cast<int>(i)) return;
                m_currentTab = static_cast<int>(i);
                this->buildContent();
                this->refreshAll();
            });
            btn->setPosition({ margin + w / 2 + i * (w + gap), y });
            menu->addChild(btn);
            m_tabButtons.push_back(spr);
        }
        m_mainLayer->addChild(menu, 5);
    }

    CCSize contentSize() const {
        return { m_width - 30.f, std::max(m_height - 106.f - 64.f, 60.f) };
    }

    void buildContentArea() {
        auto size = this->contentSize();

        auto bg = ps::ui::createPanel({ size.width + 6.f, size.height + 6.f }, ps::theme::PANEL_DARK);
        bg->setPosition({ m_width / 2, 64.f + size.height / 2 });
        m_mainLayer->addChild(bg, 1);

        m_scroll = ScrollLayer::create(size);
        m_scroll->setPosition({ 15.f, 64.f });
        m_mainLayer->addChild(m_scroll, 2);
    }

    void buildFooter() {
        m_presetLabel = CCLabelBMFont::create("", "bigFont.fnt");
        m_presetLabel->setAnchorPoint({ 0.f, .5f });
        m_presetLabel->setScale(.32f);
        m_presetLabel->setPosition({ 17.f, 53.f });
        m_mainLayer->addChild(m_presetLabel, 3);

        auto version = CCLabelBMFont::create(
            fmt::format("v{}", Mod::get()->getVersion().toNonVString()).c_str(), "bigFont.fnt"
        );
        version->setAnchorPoint({ 1.f, .5f });
        version->setScale(.28f);
        version->setColor(ps::theme::TEXT_MUTED);
        version->setPosition({ m_width - 17.f, 53.f });
        m_mainLayer->addChild(version, 3);

        auto descBG = ps::ui::createPanel({ m_width - 24.f, 38.f }, ps::theme::PANEL_DARK);
        descBG->setPosition({ m_width / 2, 27.f });
        m_mainLayer->addChild(descBG, 1);

        m_desc = SimpleTextArea::create("", "chatFont.fnt", .55f, m_width - 40.f);
        m_desc->setAlignment(kCCTextAlignmentCenter);
        m_desc->setWrappingMode(WrappingMode::WORD_WRAP);
        m_desc->setMaxLines(3);
        m_desc->setColor({ 230, 210, 255, 255 });
        m_desc->setAnchorPoint({ .5f, .5f });
        m_desc->setPosition({ m_width / 2, 27.f });
        m_mainLayer->addChild(m_desc, 3);
    }

    // ------------------------------------------------------------
    // Conteúdo da aba
    // ------------------------------------------------------------
    void buildContent() {
        m_refreshers.clear();
        m_contentMenu = nullptr;
        auto content = m_scroll->m_contentLayer;
        content->removeAllChildrenWithCleanup(true);

        auto size = this->contentSize();
        float rowH = 25.f;

        // Só as opções que existem nesta plataforma (ex: VSync é só PC)
        std::vector<std::string> keys;
        for (auto key : tabs()[m_currentTab].keys) {
            if (key[0] == '#' || Mod::get()->hasSetting(key)) keys.push_back(key);
        }

        float total = std::max(size.height, rowH * keys.size() + 4.f);
        content->setContentSize({ size.width, total });

        auto menu = CCMenu::create();
        menu->setPosition({ 0.f, 0.f });
        menu->setContentSize({ size.width, total });
        menu->ignoreAnchorPointForPosition(false);
        menu->setAnchorPoint({ 0.f, 0.f });
        content->addChild(menu, 2);
        m_contentMenu = menu;

        for (size_t i = 0; i < keys.size(); i++) {
            float y = total - 2.f - rowH * (i + .5f);
            this->addRow(content, menu, keys[i], y, size.width, i % 2 == 0);
        }

        m_scroll->scrollToTop();
    }

    void addRow(CCNode* content, CCMenu* menu, std::string const& key, float y, float width, bool even) {
        auto rowBG = ps::ui::createPanel({ width - 4.f, 23.f }, even ? ps::theme::PANEL : ps::theme::PANEL_ALT);
        rowBG->setPosition({ width / 2, y });
        content->addChild(rowBG, 0);

        std::string name;
        std::string desc;

        name = ps::tr("name." + key);
        desc = ps::tr("desc." + key);

        auto label = CCLabelBMFont::create(name.c_str(), "bigFont.fnt");
        label->setAnchorPoint({ 0.f, .5f });
        label->limitLabelWidth(width * .52f, .36f, .1f);
        label->setPosition({ 10.f, y });
        content->addChild(label, 1);

        // Botão "?" mostra a descrição embaixo
        if (!desc.empty()) {
            auto infoSpr = ps::ui::PurpleButtonSprite::create("?", { 16.f, 16.f }, ps::theme::ACCENT_DARK, .38f);
            auto infoBtn = CCMenuItemExt::createSpriteExtra(infoSpr, [this, name, desc](CCMenuItemSpriteExtra*) {
                this->setDescription(name + ": " + desc);
            });
            infoBtn->setPosition({ width * .58f, y });
            menu->addChild(infoBtn);
        }

        float right = width - 8.f;

        // Ação: liberar memória
        if (key == "#clean-now") {
            auto spr = ps::ui::PurpleButtonSprite::create(ps::tr("ui.free"), { 70.f, 19.f }, ps::theme::ACCENT, .4f);
            auto btn = CCMenuItemExt::createSpriteExtra(spr, [this](CCMenuItemSpriteExtra*) {
                CCTextureCache::sharedTextureCache()->removeUnusedTextures();
                Notification::create(ps::tr("ui.memory-freed"), NotificationIcon::Success)->show();
            });
            btn->setPosition({ right - 35.f, y });
            menu->addChild(btn);
            return;
        }

        auto setting = Mod::get()->getSetting(key);
        if (!setting) return;

        // ---- Liga / desliga ----
        if (geode::cast::typeinfo_pointer_cast<BoolSettingV3>(setting)) {
            auto spr = ps::ui::PurpleButtonSprite::create("OFF", { 52.f, 19.f }, ps::theme::OFF, .42f);
            auto btn = CCMenuItemExt::createSpriteExtra(spr, [this, key](CCMenuItemSpriteExtra*) {
                auto mod = Mod::get();
                mod->setSettingValue<bool>(key, !mod->getSettingValue<bool>(key));
                this->onValueChanged(key);
            });
            btn->setPosition({ right - 26.f, y });
            menu->addChild(btn);

            m_refreshers.push_back([spr, key] {
                bool on = Mod::get()->getSettingValue<bool>(key);
                spr->setText(on ? "ON" : "OFF");
                spr->setFill(on ? ps::theme::ACCENT : ps::theme::OFF);
                spr->setTextColor(on ? ps::theme::TEXT : ps::theme::TEXT_MUTED);
            });
            return;
        }

        // ---- Lista de opções (toque para trocar) ----
        if (auto str = geode::cast::typeinfo_pointer_cast<StringSettingV3>(setting)) {
            auto options = str->getEnumOptions().value_or(std::vector<std::string>{});
            auto spr = ps::ui::PurpleButtonSprite::create("", { 104.f, 19.f }, ps::theme::ACCENT_DARK, .38f);
            auto btn = CCMenuItemExt::createSpriteExtra(spr, [this, key, options](CCMenuItemSpriteExtra*) {
                if (options.empty()) return;
                auto mod = Mod::get();
                auto current = mod->getSettingValue<std::string>(key);
                auto it = std::find(options.begin(), options.end(), current);
                size_t next = it == options.end() ? 0 : (static_cast<size_t>(it - options.begin()) + 1) % options.size();
                mod->setSettingValue<std::string>(key, options[next]);
                if (key == "language") {
                    // Reabre o menu no idioma novo (no próximo frame, fora do callback do botão)
                    this->retain();
                    queueInMainThread([self = this] {
                        self->onClose(nullptr);
                        self->release();
                        ps::ui::openSettingsPopup();
                    });
                    return;
                }
                this->onValueChanged(key);
            });
            btn->setPosition({ right - 52.f, y });
            menu->addChild(btn);

            m_refreshers.push_back([spr, key] {
                auto value = Mod::get()->getSettingValue<std::string>(key);
                spr->setText("< " + ps::tr("val." + value) + " >");
                bool off = value == "Off" || value == "Instant" || value == "Low";
                bool normal = value == "Normal" || value == "Game";
                spr->setFill(off ? ps::theme::ACCENT : (normal ? ps::theme::OFF : ps::theme::ACCENT_DARK));
                spr->setTextColor(normal ? ps::theme::TEXT_MUTED : ps::theme::TEXT);
            });
            return;
        }

        // ---- Número: - valor + ----
        if (auto num = geode::cast::typeinfo_pointer_cast<IntSettingV3>(setting)) {
            int64_t minV = num->getMinValue().value_or(0);
            int64_t maxV = num->getMaxValue().value_or(1000);
            int64_t step = key == "particle-amount" ? 5 : 1;

            auto change = [this, key, minV, maxV, step](int dir) {
                auto mod = Mod::get();
                auto value = mod->getSettingValue<int64_t>(key);
                if (key == "fps-target") {
                    auto const& steps = fpsSteps();
                    if (dir > 0) {
                        auto it = std::upper_bound(steps.begin(), steps.end(), value);
                        value = it == steps.end() ? maxV : *it;
                    }
                    else {
                        auto it = std::lower_bound(steps.begin(), steps.end(), value);
                        value = it == steps.begin() ? minV : *(it - 1);
                    }
                }
                else {
                    value += dir * step;
                }
                value = std::clamp(value, minV, maxV);
                mod->setSettingValue<int64_t>(key, value);
                this->onValueChanged(key);
            };

            auto plusSpr = ps::ui::PurpleButtonSprite::create("+", { 22.f, 19.f }, ps::theme::ACCENT, .5f);
            auto plus = CCMenuItemExt::createSpriteExtra(plusSpr, [change](CCMenuItemSpriteExtra*) { change(1); });
            plus->setPosition({ right - 11.f, y });
            menu->addChild(plus);

            auto valueLabel = CCLabelBMFont::create("", "bigFont.fnt");
            valueLabel->setPosition({ right - 22.f - 33.f, y });
            content->addChild(valueLabel, 1);

            auto minusSpr = ps::ui::PurpleButtonSprite::create("-", { 22.f, 19.f }, ps::theme::ACCENT, .5f);
            auto minus = CCMenuItemExt::createSpriteExtra(minusSpr, [change](CCMenuItemSpriteExtra*) { change(-1); });
            minus->setPosition({ right - 22.f - 66.f - 11.f, y });
            menu->addChild(minus);

            m_refreshers.push_back([valueLabel, key] {
                auto value = Mod::get()->getSettingValue<int64_t>(key);
                valueLabel->setString(formatInt(key, value).c_str());
                valueLabel->limitLabelWidth(60.f, .4f, .1f);
                valueLabel->setColor(ps::theme::TEXT_SOFT);
            });
            return;
        }
    }

    // ------------------------------------------------------------
    // Ações
    // ------------------------------------------------------------
    void onPreset(std::string const& name) {
        bool hadLowTextures = ps::cfg().lowTextures;

        // O listener global (main.cpp) aplica o preset quando esta setting
        // muda (o Geode avisa mesmo se o valor for igual ao anterior).
        Mod::get()->setSettingValue<std::string>("preset", name);

        this->refreshAll();

        auto text = presetBlurb(name);
        if (hadLowTextures != ps::cfg().lowTextures) {
            text += " " + ps::tr("ui.restart-short");
        }
        this->setDescription(text);

        if (name == "UltraSmooth") {
            Notification::create(ps::tr("ui.ultra-on"), NotificationIcon::Success)->show();
        }
    }

    void onValueChanged(std::string const& key) {
        this->refreshAll();
        if (auto setting = Mod::get()->getSetting(key); setting && setting->requiresRestart()) {
            this->setDescription(ps::tr("ui.restart"));
        }
    }

    void setDescription(std::string const& text) {
        if (m_desc) m_desc->setText(text);
    }

    void refreshAll() {
        auto preset = ps::currentPreset();

        for (auto& [name, spr] : m_presetButtons) {
            bool active = name == preset;
            bool ultra = name == "UltraSmooth";
            if (ultra) {
                spr->setFill(active ? ps::theme::ULTRA : ps::theme::ULTRA_DARK);
                spr->setBorder(active ? ps::theme::TEXT : ps::theme::BORDER);
            }
            else {
                spr->setFill(active ? ps::theme::ACCENT : ps::theme::ACCENT_DARK);
                spr->setBorder(active ? ps::theme::TEXT : ps::theme::BORDER);
            }
            spr->setTextColor(active ? ps::theme::TEXT : ps::theme::TEXT_SOFT);
        }

        for (size_t i = 0; i < m_tabButtons.size(); i++) {
            bool active = static_cast<int>(i) == m_currentTab;
            m_tabButtons[i]->setFill(active ? ps::theme::ACCENT : ps::theme::OFF);
            m_tabButtons[i]->setTextColor(active ? ps::theme::TEXT : ps::theme::TEXT_MUTED);
        }

        for (auto& refresh : m_refreshers) refresh();

        if (m_presetLabel) {
            m_presetLabel->setString(fmt::format("{}: {}", ps::tr("ui.preset"), presetLabel(preset)).c_str());
            m_presetLabel->setColor(preset == "UltraSmooth" ? ps::theme::ULTRA : ps::theme::TEXT_SOFT);
        }
    }

public:
    static PSPopup* create() {
        auto ret = new PSPopup();
        if (ret->setup()) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }
};

namespace ps::ui {
    void openSettingsPopup() {
        if (auto popup = PSPopup::create()) {
            popup->show();
        }
    }
}

// =====================================================================
// Aviso de primeira vez: o lag não some com 100% de garantia
// =====================================================================
class PSNotice : public geode::Popup {
protected:
    CCLabelBMFont* m_titleLabel = nullptr;
    SimpleTextArea* m_text = nullptr;
    ps::ui::PurpleButtonSprite* m_enSpr = nullptr;
    ps::ui::PurpleButtonSprite* m_ptSpr = nullptr;
    ps::ui::PurpleButtonSprite* m_okSpr = nullptr;

    bool setup() {
        auto win = CCDirector::sharedDirector()->getWinSize();
        float w = std::min(360.f, win.width - 20.f);
        float h = std::min(230.f, win.height - 20.f);
        if (!Popup::init(w, h, "square02b_001.png", { 0.f, 0.f, 80.f, 80.f })) return false;

        this->setColor(ps::theme::BG_DIM);
        m_bgSprite->setColor(ps::theme::BG);
        auto border = ps::ui::createPanel({ w + 4.f, h + 4.f }, ps::theme::BORDER);
        border->setPosition({ w / 2, h / 2 });
        m_mainLayer->addChild(border, -1);
        m_closeBtn->setVisible(false);

        if (auto logo = ps::ui::createPSButtonSprite(.8f)) {
            logo->setPosition({ 28.f, h - 26.f });
            m_mainLayer->addChild(logo, 2);
        }

        m_titleLabel = CCLabelBMFont::create("", "bigFont.fnt");
        m_titleLabel->setPosition({ w / 2 + 12.f, h - 26.f });
        m_mainLayer->addChild(m_titleLabel, 2);

        auto textBG = ps::ui::createPanel({ w - 24.f, h - 100.f }, ps::theme::PANEL_DARK);
        textBG->setPosition({ w / 2, h / 2 + 4.f });
        m_mainLayer->addChild(textBG, 1);

        m_text = SimpleTextArea::create("", "chatFont.fnt", .62f, w - 40.f);
        m_text->setAlignment(kCCTextAlignmentCenter);
        m_text->setWrappingMode(WrappingMode::WORD_WRAP);
        m_text->setMaxLines(8);
        m_text->setColor({ 235, 220, 255, 255 });
        m_text->setAnchorPoint({ .5f, .5f });
        m_text->setPosition({ w / 2, h / 2 + 4.f });
        m_mainLayer->addChild(m_text, 2);

        auto menu = CCMenu::create();
        menu->setPosition({ 0.f, 0.f });
        menu->setContentSize({ w, h });

        m_enSpr = ps::ui::PurpleButtonSprite::create("English", { 74.f, 20.f }, ps::theme::OFF, .38f);
        auto en = CCMenuItemExt::createSpriteExtra(m_enSpr, [this](CCMenuItemSpriteExtra*) { this->setLanguage("English"); });
        en->setPosition({ 52.f, 22.f });
        menu->addChild(en);

        m_ptSpr = ps::ui::PurpleButtonSprite::create("Portugues", { 74.f, 20.f }, ps::theme::OFF, .38f);
        auto pt = CCMenuItemExt::createSpriteExtra(m_ptSpr, [this](CCMenuItemSpriteExtra*) { this->setLanguage("Portuguese"); });
        pt->setPosition({ 132.f, 22.f });
        menu->addChild(pt);

        m_okSpr = ps::ui::PurpleButtonSprite::create("OK", { 70.f, 24.f }, ps::theme::ACCENT, .5f);
        auto ok = CCMenuItemExt::createSpriteExtra(m_okSpr, [this](CCMenuItemSpriteExtra*) { this->onClose(nullptr); });
        ok->setPosition({ w - 50.f, 22.f });
        menu->addChild(ok);

        m_mainLayer->addChild(menu, 5);
        this->refreshTexts();
        return true;
    }

    void setLanguage(std::string const& lang) {
        Mod::get()->setSettingValue<std::string>("language", lang);
        this->refreshTexts();
    }

    void refreshTexts() {
        m_titleLabel->setString(ps::tr("notice.title").c_str());
        m_titleLabel->limitLabelWidth(m_size.width - 70.f, .55f, .1f);
        m_text->setText(ps::tr("notice.text"));
        bool pt = ps::isPortuguese();
        m_enSpr->setFill(pt ? ps::theme::OFF : ps::theme::ACCENT);
        m_ptSpr->setFill(pt ? ps::theme::ACCENT : ps::theme::OFF);
        m_okSpr->setText(ps::tr("ui.ok"));
    }

public:
    void onClose(CCObject* sender) override {
        Mod::get()->setSavedValue<bool>("notice-shown", true);
        Popup::onClose(sender);
    }

    static PSNotice* create() {
        auto ret = new PSNotice();
        if (ret->setup()) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }
};

namespace ps::ui {
    void showFirstLaunchNotice() {
        if (Mod::get()->getSavedValue<bool>("notice-shown", false)) return;
        if (auto popup = PSNotice::create()) {
            popup->show();
        }
    }
}

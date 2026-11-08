// PrettySmooth - by LZ
#include "Theme.hpp"

#include "../Config.hpp"
#include "../Runtime.hpp"

#include <Geode/ui/OverlayManager.hpp>

#include <algorithm>

using namespace geode::prelude;

namespace ps::ui {
    namespace {
        // A textura "square02b_001.png" é um quadrado branco arredondado de
        // 80x80 (dá para pintar de qualquer cor). Ela é criada maior e
        // depois diminuída, assim os cantos ficam redondinhos em qualquer
        // tamanho de botão.
        constexpr char const* SQUARE = "square02b_001.png";

        NineSlice* makeSlice(CCSize const& size, ccColor3B color, GLubyte opacity = 255) {
            float scale = std::clamp(size.height / 56.f, 0.15f, 0.45f);
            auto slice = NineSlice::create(SQUARE);
            slice->setScale(scale);
            slice->setContentSize(size / scale);
            slice->setColor(color);
            slice->setOpacity(opacity);
            return slice;
        }
    }

    NineSlice* createPanel(CCSize const& size, ccColor3B color, GLubyte opacity) {
        return makeSlice(size, color, opacity);
    }

    // ----------------------------------------------------------------
    // PurpleButtonSprite
    // ----------------------------------------------------------------
    bool PurpleButtonSprite::init(std::string const& text, CCSize const& size, ccColor3B fill,
                                  float textScale, char const* font) {
        if (!CCSprite::init()) return false;

        this->setContentSize(size);
        m_textScale = textScale;

        m_border = makeSlice(size, theme::BORDER);
        m_border->setPosition(size / 2);
        this->addChild(m_border, 0);

        auto inner = CCSize(std::max(size.width - 3.f, 1.f), std::max(size.height - 3.f, 1.f));
        m_fill = makeSlice(inner, fill);
        m_fill->setPosition(size / 2);
        this->addChild(m_fill, 1);

        // Brilho sutil na metade de cima
        auto shine = makeSlice(CCSize(inner.width - 4.f, std::max(inner.height / 2.f - 1.f, 1.f)), theme::TEXT, 28);
        shine->setPosition({ size.width / 2, size.height * .70f });
        this->addChild(shine, 2);

        m_label = CCLabelBMFont::create(text.c_str(), font);
        m_label->setPosition({ size.width / 2, size.height / 2 + 1.f });
        this->addChild(m_label, 3);
        this->setText(text);

        return true;
    }

    PurpleButtonSprite* PurpleButtonSprite::create(std::string const& text, CCSize const& size, ccColor3B fill,
                                                   float textScale, char const* font) {
        auto ret = new PurpleButtonSprite();
        if (ret->init(text, size, fill, textScale, font)) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }

    void PurpleButtonSprite::setText(std::string const& text) {
        if (!m_label) return;
        m_label->setString(text.c_str());
        m_label->limitLabelWidth(this->getContentWidth() - 8.f, m_textScale, .1f);
    }

    void PurpleButtonSprite::setFill(ccColor3B color) {
        if (m_fill) m_fill->setColor(color);
    }

    void PurpleButtonSprite::setBorder(ccColor3B color) {
        if (m_border) m_border->setColor(color);
    }

    void PurpleButtonSprite::setTextColor(ccColor3B color) {
        if (m_label) m_label->setColor(color);
    }

    // ----------------------------------------------------------------
    // Botão "PS" (menu principal, pausa e editor)
    // ----------------------------------------------------------------
    CCNode* createPSButtonSprite(float scale) {
        // Sprites avulsos de mods (resources/sprites) são arquivos, então
        // carregam com CCSprite::create (não com createWithSpriteFrameName)
        CCSprite* spr = CCSprite::create("ps_button.png"_spr);
        if (!spr) {
            // Se o sprite não carregar por algum motivo, desenha um botão
            spr = PurpleButtonSprite::create("PS", { 42.f, 42.f }, theme::ACCENT, .7f);
        }
        spr->setScale(scale);
        return spr;
    }

    // ----------------------------------------------------------------
    // Contador de FPS roxo
    // ----------------------------------------------------------------
    namespace {
        CCLabelBMFont* g_fpsLabel = nullptr;
    }

    void updateFPSCounter() {
        if (!cfg().fpsCounter) {
            if (g_fpsLabel) g_fpsLabel->setVisible(false);
            return;
        }

        if (!g_fpsLabel) {
            g_fpsLabel = CCLabelBMFont::create("-- FPS", "bigFont.fnt");
            g_fpsLabel->setID("fps-counter"_spr);
            g_fpsLabel->setAnchorPoint({ 0.f, 1.f });
            g_fpsLabel->setScale(.4f);
            g_fpsLabel->setColor(theme::TEXT_SOFT);
            g_fpsLabel->setOpacity(225);
            g_fpsLabel->setZOrder(9999);
            OverlayManager::get()->addChild(g_fpsLabel);
        }

        auto win = CCDirector::sharedDirector()->getWinSize();
        g_fpsLabel->setPosition({ 5.f, win.height - 4.f });
        g_fpsLabel->setVisible(true);
        refreshFPSCounterText();
    }

    void refreshFPSCounterText() {
        if (!g_fpsLabel || !g_fpsLabel->isVisible()) return;
        auto fps = runtime::currentFPS();
        auto text = fps > 0.f ? fmt::format("{:.0f} FPS", fps) : std::string("-- FPS");
        g_fpsLabel->setString(text.c_str());

        // Fica rosa-choque quando o UltraSmooth está ativo
        g_fpsLabel->setColor(currentPreset() == "UltraSmooth" ? theme::ULTRA : theme::TEXT_SOFT);
    }
}

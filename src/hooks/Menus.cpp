// PrettySmooth - by LZ
// Menus: fundo estático, transições rápidas e botões "PS" para abrir o
// menu roxo (menu principal, pausa e pausa do editor).

#include <Geode/Geode.hpp>
#include <Geode/modify/MenuGameLayer.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/EditorPauseLayer.hpp>
#include <Geode/modify/CCTransitionFade.hpp>

#include "../Config.hpp"
#include "../Runtime.hpp"

#include <algorithm>

using namespace geode::prelude;

// ---------------------------------------------------------------------
// Menu principal estático: o fundo para de rolar e os ícones somem.
// ---------------------------------------------------------------------
class $modify(PSMenuGameLayer, MenuGameLayer) {
    struct Fields {
        bool hidPlayer = false;
    };

    void update(float dt) {
        if (ps::cfg().staticMenu) {
            if (m_playerObject && m_playerObject->isVisible()) {
                m_playerObject->setVisible(false);
                m_fields->hidPlayer = true;
            }
            return;
        }
        if (m_fields->hidPlayer) {
            m_fields->hidPlayer = false;
            if (m_playerObject) m_playerObject->setVisible(true);
        }
        MenuGameLayer::update(dt);
    }
};

// ---------------------------------------------------------------------
// Transições (fade) mais rápidas ou instantâneas.
// As duas versões de create/init passam por esta função.
// ---------------------------------------------------------------------
class $modify(PSTransitionFade, CCTransitionFade) {
    bool initWithDuration(float duration, CCScene* scene, ccColor3B const& color) {
        switch (ps::cfg().transitions) {
            case ps::TransitionMode::Fast: duration *= 0.4f; break;
            // 0 exato é arriscado para alguns mods/código do jogo; 1-2 frames
            case ps::TransitionMode::Instant: duration = std::min(duration, 0.03f); break;
            default: break;
        }
        return CCTransitionFade::initWithDuration(duration, scene, color);
    }
};

// ---------------------------------------------------------------------
// Botões "PS"
// ---------------------------------------------------------------------
namespace {
    CCMenu* createCornerMenu(CCObject* target, SEL_MenuHandler selector, CCPoint pos, float scale) {
        auto menu = CCMenu::create();
        menu->setID("prettysmooth-corner-menu"_spr);
        menu->setPosition({ 0.f, 0.f });

        auto btn = CCMenuItemSpriteExtra::create(ps::ui::createPSButtonSprite(scale), target, selector);
        btn->setID("prettysmooth-button"_spr);
        btn->setPosition(pos);
        menu->addChild(btn);
        return menu;
    }

    // Adiciona o botão PS só no próximo frame, depois que a tela terminou de
    // montar e o geode.node-ids (e mods como BetterEdit) já organizaram os
    // menus. Adicionar durante o customSetup bagunçava a contagem de menus.
    void addCornerButtonLater(CCLayer* layer, SEL_MenuHandler selector, bool rightSide) {
        Ref<CCLayer> ref = layer;
        queueInMainThread([ref, selector, rightSide] {
            auto layer = ref.data();
            if (!layer || layer->getChildByID("prettysmooth-corner-menu"_spr)) return;
            auto win = CCDirector::sharedDirector()->getWinSize();
            CCPoint pos = rightSide ? CCPoint{ win.width - 24.f, 24.f } : CCPoint{ 26.f, 26.f };
            layer->addChild(createCornerMenu(layer, selector, pos, rightSide ? .7f : .75f), 10);
            handleTouchPriority(layer);
        });
    }
}

class $modify(PSMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;

        auto btn = CCMenuItemSpriteExtra::create(
            ps::ui::createPSButtonSprite(1.f), this, menu_selector(PSMenuLayer::onPrettySmooth)
        );
        btn->setID("prettysmooth-button"_spr);

        if (auto menu = this->getChildByID("bottom-menu")) {
            menu->addChild(btn);
            menu->updateLayout();
        }
        else {
            // Fallback: canto inferior esquerdo
            auto fallback = CCMenu::create();
            fallback->setID("prettysmooth-menu"_spr);
            fallback->setPosition({ 0.f, 0.f });
            btn->setPosition({ 30.f, 30.f });
            fallback->addChild(btn);
            this->addChild(fallback, 10);
        }
        return true;
    }

    void onPrettySmooth(CCObject*) {
        ps::ui::openSettingsPopup();
    }
};

class $modify(PSPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();
        addCornerButtonLater(this, menu_selector(PSPauseLayer::onPrettySmooth), false);
    }

    void onPrettySmooth(CCObject*) {
        ps::ui::openSettingsPopup();
    }
};

class $modify(PSEditorPauseLayer, EditorPauseLayer) {
    void customSetup() {
        EditorPauseLayer::customSetup();
        addCornerButtonLater(this, menu_selector(PSEditorPauseLayer::onPrettySmooth), true);
    }

    void onPrettySmooth(CCObject*) {
        ps::ui::openSettingsPopup();
    }
};

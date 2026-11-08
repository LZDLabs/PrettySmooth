// PrettySmooth - by LZ
// Fases (e playtest do editor): shaders, tremor, glow, LDM, fundo,
// middleground e rastros.

#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/HardStreak.hpp>
#include <Geode/modify/CCMotionStreak.hpp>

#include "../Config.hpp"
#include "../Runtime.hpp"

using namespace geode::prelude;

namespace {
    bool isEditorLayer(GJBaseGameLayer* layer) {
        return typeinfo_cast<LevelEditorLayer*>(layer) != nullptr;
    }

    void setGlowVisible(GJBaseGameLayer* l, bool visible) {
        CCNode* layers[] = {
            l->m_glowLayerT1, l->m_glowLayerT2, l->m_glowLayerT3, l->m_glowLayerT4,
            l->m_glowLayerB1, l->m_glowLayerB2, l->m_glowLayerB3, l->m_glowLayerB4,
            l->m_glowLayerB5,
        };
        for (auto node : layers) {
            if (node) node->setVisible(visible);
        }
    }
}

class $modify(PSBaseGameLayer, GJBaseGameLayer) {
    struct Fields {
        bool hidGlow = false;
        bool hidBackground = false;
        bool hidMiddleground = false;
    };

    // Sem tremor de câmera
    void shakeCamera(float duration, float strength, float interval) {
        if (ps::cfg().noShake) return;
        GJBaseGameLayer::shakeCamera(duration, strength, interval);
    }

    // Sem shaders: os triggers de shader simplesmente não ativam
    void triggerShaderCommand(ShaderGameObject* object) {
        bool editor = isEditorLayer(this);
        if (editor ? ps::cfg().editorNoShaders : ps::cfg().noShaders) return;
        GJBaseGameLayer::triggerShaderCommand(object);
    }

    void psApplyTweaks() {
        auto const& c = ps::cfg();
        auto fields = m_fields.self();
        bool editor = isEditorLayer(this);

        // Glow
        bool noGlow = editor ? c.editorNoGlow : c.noGlow;
        if (noGlow) {
            setGlowVisible(this, false);
            fields->hidGlow = true;
        }
        else if (fields->hidGlow) {
            setGlowVisible(this, true);
            fields->hidGlow = false;
        }

        // Fundo e middleground (só nas fases; o editor fica como está)
        if (!editor) {
            this->psEnforceHidden();

            if (!c.hideBackground && fields->hidBackground) {
                if (m_background) m_background->setVisible(true);
                fields->hidBackground = false;
            }
            if (!c.hideMiddleground && fields->hidMiddleground) {
                if (m_middleground) m_middleground->setVisible(true);
                fields->hidMiddleground = false;
            }
        }
    }

    // Barato o bastante para rodar todo frame: o jogo pode trocar o fundo
    // ou carregar o middleground depois (triggers / carregamento async).
    void psEnforceHidden() {
        auto const& c = ps::cfg();
        if (c.hideBackground && m_background && m_background->isVisible()) {
            m_background->setVisible(false);
            m_fields->hidBackground = true;
        }
        if (c.hideMiddleground && m_middleground && m_middleground->isVisible()) {
            m_middleground->setVisible(false);
            m_fields->hidMiddleground = true;
        }
    }
};

class $modify(PSPlayLayer, PlayLayer) {
    struct Fields {
        bool restoreLDM = false;
        bool oldLDM = false;
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        // Força o Low Detail Mode só durante o carregamento da fase. A
        // escolha salva do jogador é restaurada depois, então nada muda
        // no save.
        bool force = ps::cfg().forceLDM && level;
        bool old = force ? level->m_lowDetailModeToggled : false;
        if (force) level->m_lowDetailModeToggled = true;

        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) {
            if (force) level->m_lowDetailModeToggled = old;
            return false;
        }

        if (force) {
            m_fields->restoreLDM = true;
            m_fields->oldLDM = old;
            m_lowDetailMode = true;
        }

        static_cast<PSBaseGameLayer*>(static_cast<GJBaseGameLayer*>(this))->psApplyTweaks();
        return true;
    }

    void psRestoreLDM() {
        if (!m_fields->restoreLDM) return;
        m_fields->restoreLDM = false;
        if (m_level) m_level->m_lowDetailModeToggled = m_fields->oldLDM;
    }

    void createObjectsFromSetupFinished() {
        PlayLayer::createObjectsFromSetupFinished();
        this->psRestoreLDM();
    }

    void onQuit() {
        this->psRestoreLDM();
        PlayLayer::onQuit();
    }

    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);
        auto const& c = ps::cfg();
        if (c.hideBackground || c.hideMiddleground) {
            static_cast<PSBaseGameLayer*>(static_cast<GJBaseGameLayer*>(this))->psEnforceHidden();
        }
    }
};

// Rastro da wave
class $modify(PSHardStreak, HardStreak) {
    void updateStroke(float dt) {
        if (ps::cfg().noWaveTrail) {
            this->clear();
            return;
        }
        HardStreak::updateStroke(dt);
    }
};

// Rastros do jogador / nave
class $modify(PSMotionStreak, CCMotionStreak) {
    void update(float dt) {
        if (ps::cfg().noTrails) return;
        CCMotionStreak::update(dt);
    }

    void draw() {
        if (ps::cfg().noTrails) return;
        CCMotionStreak::draw();
    }
};

namespace ps::game {
    void applyLayerTweaks(GJBaseGameLayer* layer) {
        if (!layer) return;
        static_cast<PSBaseGameLayer*>(layer)->psApplyTweaks();
    }

    void applyToCurrentLayer() {
        auto gm = GameManager::get();
        if (gm->m_playLayer) applyLayerTweaks(gm->m_playLayer);
        if (gm->m_levelEditorLayer) applyLayerTweaks(gm->m_levelEditorLayer);
    }
}

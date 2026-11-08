// PrettySmooth - by LZ
// Editor: glow, grade e atualização de visibilidade espaçada.

#include <Geode/Geode.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/modify/DrawGridLayer.hpp>

#include "../Config.hpp"
#include "../Runtime.hpp"

using namespace geode::prelude;

class $modify(PSLevelEditorLayer, LevelEditorLayer) {
    struct Fields {
        int frame = 0;
        float acc = 0.f;
    };

    bool init(GJGameLevel* level, bool noUI) {
        if (!LevelEditorLayer::init(level, noUI)) return false;
        ps::game::applyLayerTweaks(this);
        return true;
    }

    // Em fases gigantes, recalcular a visibilidade de milhares de objetos
    // todo frame é o que mais pesa no editor. Fora do playtest dá para
    // fazer isso a cada N frames (o dt acumulado é repassado).
    void updateVisibility(float dt) {
        int n = ps::cfg().editorThrottle;
        auto fields = m_fields.self();

        if (n > 1 && m_playbackMode == PlaybackMode::Not) {
            fields->acc += dt;
            if (++fields->frame < n) return;
            dt = fields->acc;
            fields->frame = 0;
            fields->acc = 0.f;
        }
        else {
            fields->frame = 0;
            fields->acc = 0.f;
        }
        LevelEditorLayer::updateVisibility(dt);
    }
};

class $modify(PSDrawGridLayer, DrawGridLayer) {
    void draw() {
        if (!ps::cfg().editorGrid) return;
        DrawGridLayer::draw();
    }
};

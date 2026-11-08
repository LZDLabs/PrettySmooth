// PrettySmooth - by LZ
// Partículas: Normal / Reduzidas / Desligadas, separado para
// fases, menus e editor.

#include <Geode/Geode.hpp>
#include <Geode/modify/CCParticleSystem.hpp>
#include <Geode/modify/CCParticleSystemQuad.hpp>

#include "../Config.hpp"

#include <algorithm>

using namespace geode::prelude;

class $modify(PSParticleSystem, CCParticleSystem) {
    // O loop de emissão do cocos é:
    //   while (particleCount < totalParticles && emitCounter > rate) addParticle();
    // Baixar o "totalParticles" só durante o update limita a emissão em
    // qualquer plataforma (mesmo se o addParticle estiver inline). As
    // partículas que já existem morrem normalmente e os sistemas com
    // "auto remove" continuam sendo removidos.
    void update(float dt) {
        auto mode = ps::particleModeFor(ps::currentContext());
        if (mode == ps::ParticleMode::Normal) {
            CCParticleSystem::update(dt);
            return;
        }

        unsigned int total = m_uTotalParticles;
        unsigned int limit = 0;
        if (mode == ps::ParticleMode::Reduced) {
            limit = static_cast<unsigned int>(static_cast<float>(total) * ps::cfg().particleAmount);
            limit = std::max(1u, limit);
        }

        if (limit >= total) {
            CCParticleSystem::update(dt);
            return;
        }

        // retain/release: o update pode remover o sistema da cena
        this->retain();
        m_uTotalParticles = limit;
        CCParticleSystem::update(dt);
        m_uTotalParticles = total;
        this->release();
    }
};

class $modify(PSParticleSystemQuad, CCParticleSystemQuad) {
    // Desligadas: some na hora, inclusive as que já estavam na tela
    void draw() {
        if (ps::particleModeFor(ps::currentContext()) == ps::ParticleMode::Off) {
            return;
        }
        CCParticleSystemQuad::draw();
    }
};

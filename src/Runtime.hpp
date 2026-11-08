#pragma once

// PrettySmooth - by LZ
// Funções compartilhadas entre os arquivos de hooks e a UI.

#include "Config.hpp"

class GJBaseGameLayer;

namespace ps {
    namespace runtime {
        // Contexto (menu/fase/editor) calculado uma vez por frame.
        Context cachedContext();
        void updateContext();
        // Chamado quando o jogo termina de carregar (antes disso o
        // GameManager não é tocado).
        void setGameLoaded();

        // Contador de frames para o contador de FPS.
        void tickFrame(float realDt);
        float currentFPS();
    }

    namespace engine {
        void applyFPS();
        void applyVSync();
        void applyPriority();
        void applyTimer();
        void applyAll();
        // Pede para liberar texturas não usadas daqui a alguns segundos.
        void scheduleMemoryCleanup(float delay);
        void updateMemoryCleanup(float dt);
    }

    namespace game {
        // Aplica as opções visuais (glow, fundo, middleground) numa camada.
        void applyLayerTweaks(GJBaseGameLayer* layer);
        // Reaplica na fase/editor que estiver aberto agora.
        void applyToCurrentLayer();
    }

    namespace ui {
        void openSettingsPopup();
        // Aviso de primeira vez (só aparece uma vez)
        void showFirstLaunchNotice();
        // Botão "PS" roxo usado no menu, pausa e editor.
        cocos2d::CCNode* createPSButtonSprite(float scale = 1.f);
        void updateFPSCounter();
        void refreshFPSCounterText();
    }
}

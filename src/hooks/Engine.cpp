// PrettySmooth - by LZ
// Motor: FPS, VSync, Frame Pacing, prioridade, timer e limpeza de memória.
// Vale para o jogo INTEIRO (menus, fases e editor).

#include <Geode/Geode.hpp>
#include <Geode/modify/CCScheduler.hpp>
#include <Geode/modify/CCDisplayLinkDirector.hpp>
#include <Geode/modify/GameManager.hpp>

#ifdef GEODE_IS_WINDOWS
    #include <Geode/modify/CCApplication.hpp>
#endif

#include "../Config.hpp"
#include "../Runtime.hpp"

#include <algorithm>
#include <cmath>


using namespace geode::prelude;

// =====================================================================
// Contexto (menu / fase / editor) e contador de frames
// =====================================================================
namespace ps::runtime {
    namespace {
        Context g_context = Context::Menu;
        bool g_gameLoaded = false;
        int g_frames = 0;
        float g_frameTime = 0.f;
        float g_fps = 0.f;
    }

    Context cachedContext() {
        return g_context;
    }

    void setGameLoaded() {
        g_gameLoaded = true;
    }

    void updateContext() {
        if (!g_gameLoaded) return;

        auto gm = GameManager::get();
        Context next = Context::Menu;
        if (gm->m_levelEditorLayer) next = Context::Editor;
        else if (gm->m_playLayer) next = Context::Level;

        // Saiu de uma fase/editor -> libera memória daqui a pouco
        if (next == Context::Menu && g_context != Context::Menu && cfg().cleanMemory) {
            engine::scheduleMemoryCleanup(1.5f);
        }
        g_context = next;
    }

    void tickFrame(float realDt) {
        g_frames++;
        g_frameTime += realDt;
        if (g_frameTime >= 0.5f) {
            g_fps = g_frames / g_frameTime;
            g_frames = 0;
            g_frameTime = 0.f;
            if (cfg().fpsCounter) ui::refreshFPSCounterText();
        }
    }

    float currentFPS() {
        return g_fps;
    }
}

// =====================================================================
// Frame Pacing Suave
// Suaviza o delta time com uma média móvel e "devolve" a diferença aos
// poucos. O tempo total entregue ao jogo é igual ao tempo real, então a
// música e a física continuam sincronizadas; só some a micro-travada.
// =====================================================================
namespace {
    struct FramePacer {
        double avg = 0.0;
        double debt = 0.0;
        bool ready = false;

        void reset() {
            avg = 0.0;
            debt = 0.0;
            ready = false;
        }

        float smooth(float dt) {
            // Picos enormes (loading, alt-tab) passam direto
            if (!(dt > 0.f) || dt > 0.1f) {
                reset();
                return dt;
            }
            if (!ready) {
                avg = dt;
                debt = 0.0;
                ready = true;
                return dt;
            }

            avg += (dt - avg) * 0.12;
            debt += dt;

            double out = avg + (debt - avg) * 0.25;

            // Nunca deixa a dívida passar de 50ms: entrega tudo de uma vez
            if (std::abs(debt - out) > 0.05) {
                out = debt;
            }
            if (out < 0.0) out = 0.0;

            debt -= out;
            return static_cast<float>(out);
        }
    };

    FramePacer g_pacer;
}

class $modify(PSScheduler, CCScheduler) {
    void update(float dt) {
        ps::runtime::updateContext();
        ps::runtime::tickFrame(dt);
        ps::engine::updateMemoryCleanup(dt);

        // Só nos menus. Nas fases (e no editor) o dt real é entregue intacto:
        // o GD 2.2 processa os toques por timestamp, e mexer no dt faz os
        // cliques falharem, atrasarem ou "clicarem sozinhos".
        if (ps::cfg().framePacing && ps::runtime::cachedContext() == ps::Context::Menu) {
            dt = g_pacer.smooth(dt);
        }
        else {
            g_pacer.reset();
        }
        CCScheduler::update(dt);
    }
};

// =====================================================================
// FPS
// =====================================================================
namespace {
    bool g_fpsOverridden = false;
}

class $modify(PSDirector, CCDisplayLinkDirector) {
    void setAnimationInterval(double interval) {
        // Com "Destravar FPS" ligado, ninguém muda o FPS além do PrettySmooth
        if (ps::cfg().fpsUnlock) {
            interval = 1.0 / static_cast<double>(ps::cfg().fpsTarget);
        }
        CCDisplayLinkDirector::setAnimationInterval(interval);
    }
};

class $modify(PSGameManager, GameManager) {
    void updateCustomFPS() {
        GameManager::updateCustomFPS();
        if (ps::cfg().fpsUnlock) {
            ps::engine::applyFPS();
        }
    }
};

// =====================================================================
// VSync (somente Windows)
// =====================================================================
#ifdef GEODE_IS_WINDOWS
namespace {
    bool g_vsyncForced = false;
}

class $modify(PSApplication, CCApplication) {
    void toggleVerticalSync(bool enable) {
        if (ps::cfg().vsyncOff) enable = false;
        CCApplication::toggleVerticalSync(enable);
    }
};
#endif

// =====================================================================
// Funções de aplicação
// =====================================================================
namespace ps::engine {
    namespace {
        float g_cleanupTimer = -1.f;
        bool g_priorityChanged = false;
        bool g_timerChanged = false;

#ifdef GEODE_IS_WINDOWS
        using TimePeriodFn = unsigned int(__stdcall*)(unsigned int);

        TimePeriodFn getWinmmFn(char const* name) {
            static HMODULE winmm = LoadLibraryA("winmm.dll");
            if (!winmm) return nullptr;
            return reinterpret_cast<TimePeriodFn>(GetProcAddress(winmm, name));
        }
#endif
    }

    void applyFPS() {
        auto director = CCDirector::sharedDirector();
        if (cfg().fpsUnlock) {
            director->setAnimationInterval(1.0 / static_cast<double>(cfg().fpsTarget));
            g_fpsOverridden = true;
        }
        else if (g_fpsOverridden) {
            // Devolve o controle do FPS ao jogo
            g_fpsOverridden = false;
            GameManager::get()->updateCustomFPS();
        }
    }

    void applyVSync() {
#ifdef GEODE_IS_WINDOWS
        auto app = CCApplication::sharedApplication();
        if (cfg().vsyncOff) {
            app->toggleVerticalSync(false);
            g_vsyncForced = true;
        }
        else if (g_vsyncForced) {
            g_vsyncForced = false;
            app->toggleVerticalSync(GameManager::get()->m_vsyncEnabled);
        }
#endif
    }

    void applyPriority() {
        bool want = cfg().highPriority;
        if (!want && !g_priorityChanged) return;

#if defined(GEODE_IS_WINDOWS)
        SetPriorityClass(GetCurrentProcess(), want ? HIGH_PRIORITY_CLASS : NORMAL_PRIORITY_CLASS);
        SetThreadPriority(GetCurrentThread(), want ? THREAD_PRIORITY_ABOVE_NORMAL : THREAD_PRIORITY_NORMAL);
#endif
        g_priorityChanged = want;
    }

    void applyTimer() {
#ifdef GEODE_IS_WINDOWS
        bool want = cfg().preciseTimer;
        if (want == g_timerChanged) return;
        if (auto fn = getWinmmFn(want ? "timeBeginPeriod" : "timeEndPeriod")) {
            fn(1);
            g_timerChanged = want;
        }
#endif
    }

    void applyTextureQuality() {
        // Usa a própria opção de qualidade de textura do jogo (vale depois de
        // reiniciar, igual ao menu de gráficos do GD).
        if (cfg().texQuality > 0) {
            GameManager::get()->m_texQuality = cfg().texQuality;
        }
    }

    void applyAll() {
        applyTextureQuality();
        applyFPS();
        applyVSync();
        applyPriority();
        applyTimer();
    }

    void scheduleMemoryCleanup(float delay) {
        g_cleanupTimer = delay;
    }

    void updateMemoryCleanup(float dt) {
        if (g_cleanupTimer < 0.f) return;
        g_cleanupTimer -= dt;
        if (g_cleanupTimer > 0.f) return;
        g_cleanupTimer = -1.f;

        // Só limpa quando não tem fase/editor aberto
        if (runtime::cachedContext() != Context::Menu) return;

        // Remove apenas texturas que ninguém mais usa (o próprio cocos faz
        // isso em avisos de pouca memória, então é seguro).
        CCTextureCache::sharedTextureCache()->removeUnusedTextures();
        log::debug("PrettySmooth: texturas não usadas liberadas");
    }
}

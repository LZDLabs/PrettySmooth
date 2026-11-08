#pragma once

// PrettySmooth - by LZ
// Configuração em cache. Os hooks rodam todo frame, então eles NUNCA leem
// as settings do Geode diretamente: leem esta struct simples, que é
// atualizada sempre que alguma setting muda.

#include <Geode/Geode.hpp>

#include <string>
#include <string_view>
#include <vector>

namespace ps {
    enum class ParticleMode { Normal, Reduced, Off };
    enum class TransitionMode { Normal, Fast, Instant };

    struct Config {
        // Geral / FPS
        bool fpsUnlock = false;
        int fpsTarget = 240;
        bool vsyncOff = false;
        bool framePacing = true;
        bool highPriority = false;
        bool preciseTimer = true;
        bool fpsCounter = false;

        // Fases
        ParticleMode levelParticles = ParticleMode::Reduced;
        float particleAmount = 0.6f; // 0.05 .. 1.0
        bool noShaders = false;
        bool noShake = false;
        bool noGlow = false;
        bool forceLDM = false;
        bool hideBackground = false;
        bool hideMiddleground = false;
        bool noTrails = false;
        bool noWaveTrail = false;

        // Menus
        ParticleMode menuParticles = ParticleMode::Reduced;
        bool staticMenu = false;
        TransitionMode transitions = TransitionMode::Fast;

        // Editor
        ParticleMode editorParticles = ParticleMode::Reduced;
        bool editorNoGlow = false;
        bool editorNoShaders = false;
        bool editorGrid = true;
        int editorThrottle = 1;

        // Sistema
        bool lowTextures = false;
        int texQuality = 0; // 0 = jogo decide, 1 = baixa, 2 = média, 3 = alta
        bool cleanMemory = true;

        // Idioma
        bool portuguese = false;
    };

    // Acesso rápido (sem lookup) à configuração atual.
    Config const& cfg();

    // Relê todas as settings do Geode para a struct em cache.
    void reloadConfig();

    // Aplica coisas que precisam de chamada explícita (FPS, VSync,
    // prioridade, timer, contador, camadas da fase/editor atual).
    void applyRuntime();

    // ---------- Presets ----------
    inline constexpr std::string_view PRESET_CUSTOM = "Custom";

    std::vector<std::string> const& presetNames();
    std::string currentPreset();
    // Aplica um preset (muda várias settings de uma vez).
    void applyPreset(std::string_view name);
    // true enquanto um preset está sendo aplicado (evita recursão).
    bool isApplyingPreset();

    // Helpers de contexto usados pelos hooks
    enum class Context { Menu, Level, Editor };
    Context currentContext();
    ParticleMode particleModeFor(Context ctx);

    // Converte strings das settings
    ParticleMode parseParticleMode(std::string_view str);
    TransitionMode parseTransitionMode(std::string_view str);
}

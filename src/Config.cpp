// PrettySmooth - by LZ
#include "Config.hpp"
#include "Runtime.hpp"

#include <algorithm>
#include <variant>

using namespace geode::prelude;

namespace ps {
    namespace {
        Config g_cfg;
        bool g_applyingPreset = false;

        bool getBool(std::string_view key, bool def) {
            auto mod = Mod::get();
            if (!mod->hasSetting(key)) return def;
            return mod->getSettingValue<bool>(key);
        }

        int64_t getInt(std::string_view key, int64_t def) {
            auto mod = Mod::get();
            if (!mod->hasSetting(key)) return def;
            return mod->getSettingValue<int64_t>(key);
        }

        std::string getString(std::string_view key, std::string_view def) {
            auto mod = Mod::get();
            if (!mod->hasSetting(key)) return std::string(def);
            return mod->getSettingValue<std::string>(key);
        }

        // ---------- Tabela de presets ----------
        using Value = std::variant<bool, int64_t, std::string>;
        struct Entry {
            char const* key;
            Value value;
        };

        // Valores em comum: "jogo original"
        std::vector<Entry> vanillaEntries() {
            return {
                {"fps-unlock", false},
                {"vsync-off", false},
                {"frame-pacing", false},
                {"high-priority", false},
                {"precise-timer", false},
                {"level-particles", std::string("Normal")},
                {"particle-amount", int64_t(100)},
                {"no-shaders", false},
                {"no-shake", false},
                {"no-glow", false},
                {"force-ldm", false},
                {"hide-background", false},
                {"hide-middleground", false},
                {"no-trails", false},
                {"no-wave-trail", false},
                {"menu-particles", std::string("Normal")},
                {"static-menu", false},
                {"transitions", std::string("Normal")},
                {"editor-particles", std::string("Normal")},
                {"editor-no-glow", false},
                {"editor-no-shaders", false},
                {"editor-grid", true},
                {"editor-throttle", int64_t(1)},
                {"low-textures", false},
                {"texture-quality", std::string("Game")},
                {"clean-memory", false},
            };
        }

        // Sobrescreve valores de uma lista base
        std::vector<Entry> withOverrides(std::vector<Entry> base, std::vector<Entry> const& overrides) {
            for (auto const& ov : overrides) {
                auto it = std::find_if(base.begin(), base.end(), [&](Entry const& e) {
                    return std::string_view(e.key) == ov.key;
                });
                if (it != base.end()) it->value = ov.value;
                else base.push_back(ov);
            }
            return base;
        }

        std::vector<Entry> balancedEntries() {
            return withOverrides(vanillaEntries(), {
                {"frame-pacing", true},
                {"precise-timer", true},
                {"level-particles", std::string("Reduced")},
                {"particle-amount", int64_t(60)},
                {"menu-particles", std::string("Reduced")},
                {"editor-particles", std::string("Reduced")},
                {"transitions", std::string("Fast")},
                {"clean-memory", true},
            });
        }

        std::vector<Entry> performanceEntries() {
            return withOverrides(balancedEntries(), {
                {"high-priority", true},
                {"particle-amount", int64_t(35)},
                {"no-shaders", true},
                {"no-shake", true},
                {"force-ldm", true},
                {"menu-particles", std::string("Off")},
                {"editor-particles", std::string("Off")},
                {"editor-no-shaders", true},
                {"texture-quality", std::string("Medium")},
            });
        }

        // Batata: deixa o jogo FEIO para rodar em qualquer coisa
        std::vector<Entry> potatoEntries() {
            return withOverrides(performanceEntries(), {
                {"level-particles", std::string("Off")},
                {"particle-amount", int64_t(15)},
                {"no-glow", true},
                {"hide-background", true},
                {"hide-middleground", true},
                {"no-trails", true},
                {"static-menu", true},
                {"transitions", std::string("Instant")},
                {"editor-no-glow", true},
                {"editor-throttle", int64_t(2)},
                {"low-textures", true},
                {"texture-quality", std::string("Low")},
                {"fps-unlock", true},
                {"fps-target", int64_t(60)},
            });
        }

        // UltraSmooth: a potência máxima. Tudo do Batata + FPS destravado,
        // VSync off, frame pacing, prioridade alta e timer preciso.
        std::vector<Entry> ultraEntries() {
            return withOverrides(potatoEntries(), {
                {"fps-unlock", true},
                {"fps-target", int64_t(360)},
                {"vsync-off", true},
                {"frame-pacing", true},
                {"high-priority", true},
                {"precise-timer", true},
                {"particle-amount", int64_t(5)},
                {"no-wave-trail", true},
                {"editor-throttle", int64_t(3)},
            });
        }

        std::vector<Entry> entriesFor(std::string_view name) {
            if (name == "Vanilla") return vanillaEntries();
            if (name == "Balanced") return balancedEntries();
            if (name == "Performance") return performanceEntries();
            if (name == "Potato") return potatoEntries();
            if (name == "UltraSmooth") return ultraEntries();
            return {};
        }
    }

    Config const& cfg() {
        return g_cfg;
    }

    ParticleMode parseParticleMode(std::string_view str) {
        if (str == "Off") return ParticleMode::Off;
        if (str == "Reduced") return ParticleMode::Reduced;
        return ParticleMode::Normal;
    }

    TransitionMode parseTransitionMode(std::string_view str) {
        if (str == "Instant") return TransitionMode::Instant;
        if (str == "Fast") return TransitionMode::Fast;
        return TransitionMode::Normal;
    }

    void reloadConfig() {
        Config c;

        c.fpsUnlock = getBool("fps-unlock", false);
        c.fpsTarget = static_cast<int>(std::clamp<int64_t>(getInt("fps-target", 240), 30, 1000));
        c.vsyncOff = getBool("vsync-off", false);
        c.framePacing = getBool("frame-pacing", true);
        c.highPriority = getBool("high-priority", false);
        c.preciseTimer = getBool("precise-timer", false);
        c.fpsCounter = getBool("fps-counter", false);

        c.levelParticles = parseParticleMode(getString("level-particles", "Reduced"));
        c.particleAmount = static_cast<float>(std::clamp<int64_t>(getInt("particle-amount", 60), 5, 100)) / 100.f;
        c.noShaders = getBool("no-shaders", false);
        c.noShake = getBool("no-shake", false);
        c.noGlow = getBool("no-glow", false);
        c.forceLDM = getBool("force-ldm", false);
        c.hideBackground = getBool("hide-background", false);
        c.hideMiddleground = getBool("hide-middleground", false);
        c.noTrails = getBool("no-trails", false);
        c.noWaveTrail = getBool("no-wave-trail", false);

        c.menuParticles = parseParticleMode(getString("menu-particles", "Reduced"));
        c.staticMenu = getBool("static-menu", false);
        c.transitions = parseTransitionMode(getString("transitions", "Fast"));

        c.editorParticles = parseParticleMode(getString("editor-particles", "Reduced"));
        c.editorNoGlow = getBool("editor-no-glow", false);
        c.editorNoShaders = getBool("editor-no-shaders", false);
        c.editorGrid = getBool("editor-grid", true);
        c.editorThrottle = static_cast<int>(std::clamp<int64_t>(getInt("editor-throttle", 1), 1, 4));

        c.lowTextures = getBool("low-textures", false);
        c.cleanMemory = getBool("clean-memory", true);

        auto tq = getString("texture-quality", "Game");
        c.texQuality = tq == "Low" ? 1 : tq == "Medium" ? 2 : tq == "High" ? 3 : 0;
        c.portuguese = getString("language", "English") == "Portuguese";

        g_cfg = c;
    }

    void applyRuntime() {
        engine::applyAll();
        ui::updateFPSCounter();
        game::applyToCurrentLayer();
    }

    std::vector<std::string> const& presetNames() {
        static std::vector<std::string> names = {
            "Vanilla", "Balanced", "Performance", "Potato", "UltraSmooth"
        };
        return names;
    }

    std::string currentPreset() {
        return getString("preset", "Balanced");
    }

    bool isApplyingPreset() {
        return g_applyingPreset;
    }

    void applyPreset(std::string_view name) {
        auto entries = entriesFor(name);
        if (entries.empty()) return;

        auto mod = Mod::get();
        g_applyingPreset = true;
        for (auto const& e : entries) {
            if (!mod->hasSetting(e.key)) continue; // ex: vsync-off só existe no Windows
            std::visit([&](auto const& v) {
                using T = std::decay_t<decltype(v)>;
                if (mod->getSettingValue<T>(e.key) != v) {
                    mod->setSettingValue<T>(e.key, v);
                }
            }, e.value);
        }
        if (currentPreset() != name) {
            mod->setSettingValue<std::string>("preset", std::string(name));
        }
        g_applyingPreset = false;

        reloadConfig();
        applyRuntime();
    }

    Context currentContext() {
        return runtime::cachedContext();
    }

    ParticleMode particleModeFor(Context ctx) {
        switch (ctx) {
            case Context::Level: return g_cfg.levelParticles;
            case Context::Editor: return g_cfg.editorParticles;
            default: return g_cfg.menuParticles;
        }
    }
}

// PrettySmooth - by LZ
// Otimiza o Geometry Dash inteiro: menus, fases e editor (PC e celular).

#include <Geode/Geode.hpp>

#include "Config.hpp"
#include "Runtime.hpp"

using namespace geode::prelude;

namespace {
    void onSettingChanged(std::string_view key) {
        // Títulos não são settings de verdade
        if (key.starts_with("title-")) return;

        if (key == "preset") {
            if (ps::isApplyingPreset()) return;
            auto value = ps::currentPreset();
            if (value != ps::PRESET_CUSTOM) {
                // applyPreset já relê a config e aplica tudo
                ps::applyPreset(value);
            }
            return;
        }

        // Enquanto um preset é aplicado, cada setting dispara este callback;
        // a config é relida uma vez só no final do applyPreset.
        if (ps::isApplyingPreset()) return;

        // O usuário mexeu em algo manualmente -> preset vira "Personalizado".
        // (o contador de FPS não faz parte dos presets)
        if (key != "fps-counter" && key != "language" && ps::currentPreset() != ps::PRESET_CUSTOM) {
            Mod::get()->setSettingValue<std::string>("preset", std::string(ps::PRESET_CUSTOM));
        }

        ps::reloadConfig();
        ps::applyRuntime();
    }
}

$on_mod(Loaded) {
    ps::reloadConfig();

    // Texturas 16-bit precisam ser definidas antes das texturas carregarem.
    if (ps::cfg().lowTextures) {
        CCTexture2D::setDefaultAlphaPixelFormat(kCCTexture2DPixelFormat_RGBA4444);
    }

    listenForAllSettingChanges([](std::string_view key, std::shared_ptr<SettingV3>) {
        onSettingChanged(key);
    });

    log::info("PrettySmooth loaded (preset: {})", ps::currentPreset());
}

$on_game(Loaded) {
    // O jogo terminou de carregar: aplica FPS, VSync, prioridade, etc.
    ps::runtime::setGameLoaded();
    ps::applyRuntime();
    ps::ui::showFirstLaunchNotice();
}

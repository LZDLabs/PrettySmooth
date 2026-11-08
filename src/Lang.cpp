// PrettySmooth - by LZ
#include "Lang.hpp"
#include "Config.hpp"

#include <unordered_map>

namespace ps {
    namespace {
        struct Text {
            std::string en;
            std::string pt;
        };

        std::unordered_map<std::string_view, Text> const& table() {
            static std::unordered_map<std::string_view, Text> t = {
                // ---------- Setting names ----------
                { "name.language", { "Language", "Idioma" } },
                { "name.fps-unlock", { "Custom FPS limit", "Limite de FPS próprio" } },
                { "name.fps-target", { "Target FPS", "FPS alvo" } },
                { "name.vsync-off", { "Force VSync off", "Forçar VSync desligado" } },
                { "name.frame-pacing", { "Smooth Frame Pacing", "Frame Pacing Suave" } },
                { "name.high-priority", { "High priority", "Prioridade alta" } },
                { "name.precise-timer", { "High precision timer", "Timer de alta precisão" } },
                { "name.fps-counter", { "Purple FPS counter", "Contador de FPS roxo" } },
                { "name.level-particles", { "Level particles", "Partículas nas fases" } },
                { "name.particle-amount", { "Particle amount", "Quantidade de partículas" } },
                { "name.no-shaders", { "Disable shaders", "Desligar shaders" } },
                { "name.no-shake", { "Disable camera shake", "Desligar tremor de câmera" } },
                { "name.no-glow", { "Remove glow", "Remover glow" } },
                { "name.force-ldm", { "Force Low Detail Mode", "Forçar Low Detail Mode" } },
                { "name.hide-background", { "Hide background", "Esconder fundo" } },
                { "name.hide-middleground", { "Hide middleground", "Esconder middleground" } },
                { "name.no-trails", { "Disable trails", "Desligar rastros" } },
                { "name.no-wave-trail", { "Disable wave trail", "Desligar rastro da wave" } },
                { "name.menu-particles", { "Menu particles", "Partículas nos menus" } },
                { "name.static-menu", { "Static main menu", "Menu principal estático" } },
                { "name.transitions", { "Transitions", "Transições" } },
                { "name.editor-particles", { "Editor particles", "Partículas no editor" } },
                { "name.editor-no-glow", { "Editor without glow", "Editor sem glow" } },
                { "name.editor-no-shaders", { "Editor without shaders", "Editor sem shaders" } },
                { "name.editor-grid", { "Draw editor grid", "Desenhar grade do editor" } },
                { "name.editor-throttle", { "Editor visibility updates", "Atualização do editor" } },
                { "name.texture-quality", { "Texture quality", "Qualidade das texturas" } },
                { "name.low-textures", { "16-bit textures", "Texturas 16-bit" } },
                { "name.clean-memory", { "Free memory after levels", "Limpar memória ao sair" } },
                { "name.#clean-now", { "Free memory now", "Liberar memória agora" } },

                // ---------- Setting descriptions ----------
                { "desc.language", { "Language of the PrettySmooth menus.", "Idioma dos menus do PrettySmooth." } },
                { "desc.fps-unlock", { "Ignores the game's FPS limit and uses 'Target FPS'. On weak phones, locking at 60 is steadier than an unstable 90.",
                                       "Ignora o limite de FPS do jogo e usa o 'FPS alvo'. Em celular fraco, travar em 60 fica mais estável que 90 oscilando." } },
                { "desc.fps-target", { "FPS used when 'Custom FPS limit' is on. On mobile the real maximum is the screen refresh rate.",
                                       "FPS usado quando o 'Limite de FPS próprio' está ligado. No celular o máximo real é a taxa da tela." } },
                { "desc.vsync-off", { "Turns VSync off even if the game tries to turn it on. Less input lag (may tear).",
                                      "Desliga o VSync mesmo se o jogo tentar ligar. Menos input lag (pode ter tearing)." } },
                { "desc.frame-pacing", { "Smooths frame times in MENUS to remove micro-stutters. Never touches levels, so clicks stay exact.",
                                         "Suaviza o tempo dos frames nos MENUS. Nunca mexe nas fases, então os cliques continuam exatos." } },
                { "desc.high-priority", { "Asks Windows to give Geometry Dash CPU priority.", "Pede ao Windows prioridade de CPU para o Geometry Dash." } },
                { "desc.precise-timer", { "Uses the 1ms Windows timer for a steadier FPS limiter.", "Usa o timer de 1ms do Windows para um limitador de FPS mais estável." } },
                { "desc.fps-counter", { "Shows an FPS counter in the corner of the screen.", "Mostra um contador de FPS no canto da tela." } },
                { "desc.level-particles", { "Normal, Reduced (uses the particle amount) or Off.", "Normal, Reduzidas (usa a quantidade de partículas) ou Desligadas." } },
                { "desc.particle-amount", { "Percentage of particles kept in 'Reduced' mode (levels, menus and editor).",
                                            "Porcentagem de partículas mantidas no modo 'Reduzidas' (fases, menus e editor)." } },
                { "desc.no-shaders", { "Ignores shader triggers (blur, glitch, chromatic...). Very heavy on phones.",
                                       "Ignora os triggers de shader (blur, glitch, chromatic...). Muito pesado no celular." } },
                { "desc.no-shake", { "Removes camera shakes.", "Remove os tremores de câmera." } },
                { "desc.no-glow", { "Hides object glow layers. May hide glow decorations (ugly, runs better).",
                                    "Esconde as camadas de brilho. Pode esconder decorações de glow (feio, roda melhor)." } },
                { "desc.force-ldm", { "Turns on LDM in every level that has high detail objects, without changing your saved choice.",
                                      "Ativa o LDM em toda fase que tem objetos de alto detalhe, sem mudar sua escolha salva." } },
                { "desc.hide-background", { "Hides the level background image (ugly).", "Esconde a imagem de fundo das fases (feio)." } },
                { "desc.hide-middleground", { "Hides the decorative middle layer (2.2).", "Esconde a camada decorativa do meio (2.2)." } },
                { "desc.no-trails", { "Disables the player and ship trails.", "Desliga os rastros do jogador e da nave." } },
                { "desc.no-wave-trail", { "Disables the wave trail.", "Desliga o rastro da wave." } },
                { "desc.menu-particles", { "Particles in menus.", "Partículas nos menus." } },
                { "desc.static-menu", { "Freezes the animated main menu background (no running icons).",
                                        "Congela o fundo animado do menu principal (sem ícones correndo)." } },
                { "desc.transitions", { "Speed of the fade between screens.", "Velocidade do fade entre as telas." } },
                { "desc.editor-particles", { "Particles in the editor.", "Partículas no editor." } },
                { "desc.editor-no-glow", { "Hides glow layers in the editor (the saved level doesn't change).",
                                           "Esconde o glow no editor (a fase salva não muda)." } },
                { "desc.editor-no-shaders", { "Ignores the shader preview in the editor and playtest.", "Ignora a prévia de shaders no editor e no playtest." } },
                { "desc.editor-grid", { "Off removes the grid and guide lines (much lighter in huge levels).",
                                        "Desligado remove a grade e as linhas-guia (bem mais leve em fases enormes)." } },
                { "desc.editor-throttle", { "Normal = every frame. 1/2 to 1/4 = updates object visibility every N frames outside playtest.",
                                            "Normal = todo frame. 1/2 a 1/4 = atualiza a visibilidade a cada N frames fora do playtest." } },
                { "desc.texture-quality", { "'Low' uses much less GPU memory and bandwidth (best for weak phones). Needs a restart.",
                                            "'Baixa' usa bem menos memória e banda da GPU (melhor para celular fraco). Precisa reiniciar." } },
                { "desc.low-textures", { "Loads textures in 16-bit: half the video memory, ugly gradients. Needs a restart.",
                                         "Texturas em 16-bit: metade da memória de vídeo, degradês feios. Precisa reiniciar." } },
                { "desc.clean-memory", { "Frees unused textures when leaving a level or the editor. Great for phones with little RAM.",
                                         "Libera texturas não usadas ao sair de fases ou do editor. Ótimo para pouca RAM." } },
                { "desc.#clean-now", { "Frees the textures that aren't being used right now.", "Libera agora as texturas que não estão sendo usadas." } },

                // ---------- Values ----------
                { "val.Normal", { "Normal", "Normal" } },
                { "val.Reduced", { "Reduced", "Reduzidas" } },
                { "val.Off", { "Off", "Desligadas" } },
                { "val.Fast", { "Fast", "Rápidas" } },
                { "val.Instant", { "Instant", "Instantâneas" } },
                { "val.Game", { "Game", "Do jogo" } },
                { "val.Low", { "Low", "Baixa" } },
                { "val.Medium", { "Medium", "Média" } },
                { "val.High", { "High", "Alta" } },
                { "val.English", { "English", "English" } },
                { "val.Portuguese", { "Portuguese", "Portugues" } },

                // ---------- Presets ----------
                { "preset.Custom", { "Custom", "Personalizado" } },
                { "preset.Vanilla", { "Vanilla", "Vanilla" } },
                { "preset.Balanced", { "Balanced", "Equilibrado" } },
                { "preset.Performance", { "Performance", "Desempenho" } },
                { "preset.Potato", { "Potato", "Batata" } },
                { "preset.UltraSmooth", { "UltraSmooth", "UltraSmooth" } },
                { "blurb.Vanilla", { "Vanilla: the game exactly as RobTop made it. No optimization.",
                                     "Vanilla: o jogo exatamente como o RobTop fez. Nenhuma otimização." } },
                { "blurb.Balanced", { "Balanced: fewer particles, fast transitions, menu frame pacing and memory cleanup. Almost no visual loss.",
                                      "Equilibrado: menos partículas, transições rápidas, frame pacing nos menus e limpeza de memória. Quase sem perda visual." } },
                { "blurb.Performance", { "Performance: no shaders, no shake, forced LDM, few particles and medium textures. Great for mid-range phones.",
                                         "Desempenho: sem shaders, sem tremor, LDM forçado, poucas partículas e texturas médias. Ótimo para celular médio." } },
                { "blurb.Potato", { "Potato: makes the game UGLY on purpose. No particles, glow or background, low 16-bit textures, 60 FPS lock. Runs on anything.",
                                    "Batata: deixa o jogo FEIO de propósito. Sem partículas, glow e fundo, texturas baixas 16-bit, FPS travado em 60. Roda em qualquer coisa." } },
                { "blurb.UltraSmooth", { "ULTRASMOOTH: maximum power! Everything from Potato + uncapped FPS, VSync off, high priority and turbo editor.",
                                         "ULTRASMOOTH: potência máxima! Tudo do Batata + FPS destravado, VSync off, prioridade alta e editor turbo." } },
                { "blurb.Custom", { "Custom: you changed options manually.", "Personalizado: você ajustou as opções manualmente." } },

                // ---------- UI ----------
                { "tab.General", { "General", "Geral" } },
                { "tab.Levels", { "Levels", "Fases" } },
                { "tab.Menus", { "Menus", "Menus" } },
                { "tab.Editor", { "Editor", "Editor" } },
                { "tab.System", { "System", "Sistema" } },
                { "ui.by", { "by LZ", "por LZ" } },
                { "ui.preset", { "Preset", "Preset" } },
                { "ui.free", { "Free", "Liberar" } },
                { "ui.memory-freed", { "Memory freed!", "Memória liberada!" } },
                { "ui.restart", { "This option only applies after restarting the game.", "Esta opção só vale depois de reiniciar o jogo." } },
                { "ui.restart-short", { "(Restart the game to apply the textures.)", "(Reinicie o jogo para aplicar as texturas.)" } },
                { "ui.ultra-on", { "UltraSmooth ON! Smooth for real.", "UltraSmooth ATIVADO! Liso de verdade." } },
                { "ui.ok", { "OK", "OK" } },

                // ---------- First launch notice ----------
                { "notice.title", { "Welcome to PrettySmooth!", "Bem-vindo ao PrettySmooth!" } },
                { "notice.text", {
                    "PrettySmooth reduces lag as much as possible, but it can't kill lag with a 100% guarantee. "
                    "Results depend on your device (CPU, GPU, RAM and temperature) and on the level. "
                    "Try the Performance or Potato presets on weak phones. Open the menu with the purple PS button.",
                    "O PrettySmooth reduz o lag o máximo possível, mas não consegue acabar com o lag com 100% de garantia. "
                    "O resultado depende do seu dispositivo (CPU, GPU, RAM e temperatura) e da fase. "
                    "Em celular fraco, use os presets Desempenho ou Batata. Abra o menu pelo botão PS roxo." } },
            };
            return t;
        }
    }

    namespace {
        // As fontes bitmap do GD (bigFont/chatFont) só têm ASCII
        std::string deaccent(std::string const& in) {
            static std::pair<std::string_view, char> const map[] = {
                {"á", 'a'}, {"à", 'a'}, {"â", 'a'}, {"ã", 'a'}, {"ä", 'a'},
                {"é", 'e'}, {"ê", 'e'}, {"è", 'e'}, {"í", 'i'}, {"ì", 'i'},
                {"ó", 'o'}, {"ô", 'o'}, {"õ", 'o'}, {"ò", 'o'}, {"ú", 'u'}, {"ü", 'u'},
                {"ç", 'c'}, {"Á", 'A'}, {"À", 'A'}, {"Â", 'A'}, {"Ã", 'A'}, {"É", 'E'},
                {"Ê", 'E'}, {"Í", 'I'}, {"Ó", 'O'}, {"Ô", 'O'}, {"Õ", 'O'}, {"Ú", 'U'}, {"Ç", 'C'},
            };
            std::string out;
            out.reserve(in.size());
            for (size_t i = 0; i < in.size();) {
                bool hit = false;
                for (auto const& [from, to] : map) {
                    if (in.compare(i, from.size(), from) == 0) {
                        out += to;
                        i += from.size();
                        hit = true;
                        break;
                    }
                }
                if (!hit) out += in[i++];
            }
            return out;
        }
    }

    bool isPortuguese() {
        return cfg().portuguese;
    }

    std::string const& tr(std::string_view key) {
        auto const& t = table();
        auto it = t.find(key);
        if (it == t.end()) {
            static thread_local std::string fallback;
            fallback = std::string(key);
            return fallback;
        }
        static std::unordered_map<std::string_view, std::string> ptCache;
        if (!isPortuguese()) return it->second.en;
        auto cached = ptCache.find(key);
        if (cached == ptCache.end()) {
            cached = ptCache.emplace(it->first, deaccent(it->second.pt)).first;
        }
        return cached->second;
    }
}

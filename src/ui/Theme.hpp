#pragma once

// PrettySmooth - by LZ
// Tema roxo + botões usados em todos os menus do mod.

#include <Geode/Geode.hpp>
#include <Geode/ui/NineSlice.hpp>

#include <string>

namespace ps::theme {
    using cocos2d::ccColor3B;

    inline ccColor3B const BG_DIM        { 22, 4, 40 };     // escurecimento atrás do popup
    inline ccColor3B const BG            { 46, 16, 86 };    // fundo do popup
    inline ccColor3B const BORDER        { 190, 140, 255 }; // borda lilás
    inline ccColor3B const PANEL         { 70, 30, 128 };   // linhas / painéis
    inline ccColor3B const PANEL_ALT     { 60, 24, 112 };
    inline ccColor3B const PANEL_DARK    { 34, 10, 64 };
    inline ccColor3B const ACCENT        { 150, 78, 255 };  // botão ativo
    inline ccColor3B const ACCENT_DARK   { 98, 46, 176 };   // botão normal
    inline ccColor3B const OFF           { 52, 28, 86 };    // toggle desligado
    inline ccColor3B const ULTRA         { 214, 70, 255 };  // UltraSmooth ativo
    inline ccColor3B const ULTRA_DARK    { 120, 34, 150 };  // UltraSmooth normal
    inline ccColor3B const TEXT          { 255, 255, 255 };
    inline ccColor3B const TEXT_SOFT     { 216, 190, 255 };
    inline ccColor3B const TEXT_MUTED    { 160, 130, 205 };
}

namespace ps::ui {
    // Botão roxo com cantos arredondados. É um CCSprite (vazio) com os
    // pedaços dentro, então serve em CCMenuItemSpriteExtra e no botão de
    // fechar do Popup.
    class PurpleButtonSprite : public cocos2d::CCSprite {
    protected:
        geode::NineSlice* m_border = nullptr;
        geode::NineSlice* m_fill = nullptr;
        cocos2d::CCLabelBMFont* m_label = nullptr;
        float m_textScale = .5f;

        bool init(std::string const& text, cocos2d::CCSize const& size, cocos2d::ccColor3B fill,
                  float textScale, char const* font);

    public:
        static PurpleButtonSprite* create(
            std::string const& text, cocos2d::CCSize const& size,
            cocos2d::ccColor3B fill = theme::ACCENT_DARK, float textScale = .5f,
            char const* font = "bigFont.fnt"
        );

        void setText(std::string const& text);
        void setFill(cocos2d::ccColor3B color);
        void setBorder(cocos2d::ccColor3B color);
        void setTextColor(cocos2d::ccColor3B color);
    };

    // Painel (fundo arredondado) sem texto
    geode::NineSlice* createPanel(cocos2d::CCSize const& size, cocos2d::ccColor3B color, GLubyte opacity = 255);
}

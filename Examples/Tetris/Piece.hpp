#pragma once

#include "AppCUI.hpp"

enum class PieceType
{
    I = 0,
    O = 1,
    T = 2,
    S = 3,
    L = 4,
    Z = 5,
    J = 6,

    End
};

class Piece
{
  private:
    static const unsigned int cells = 4;
    unsigned int width              = 1;
    unsigned int height             = 1;
    AppCUI::Graphics::ColorPair color{ AppCUI::Graphics::Color::White, AppCUI::Graphics::Color::Transparent };
    char matrix[cells][cells]{ 0 };
    AppCUI::Utils::Reference<AppCUI::Controls::Control> control = nullptr;
    const PieceType type;

  public:
    Piece(const PieceType type, const AppCUI::Utils::Reference<AppCUI::Controls::Control> control);

    bool Draw(AppCUI::Graphics::Renderer& renderer, int scale, bool center, int w, int h);
    bool Draw(AppCUI::Graphics::Renderer& renderer, int scale, const int x, const int y);
    AppCUI::Graphics::Size GetSize(PieceType type, int scale) const;
};

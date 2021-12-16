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
    int x                           = 0;
    int y                           = 0;
    AppCUI::Graphics::ColorPair color{ AppCUI::Graphics::Color::White, AppCUI::Graphics::Color::Transparent };
    char matrix[cells][cells]{ 0 };
    AppCUI::Utils::Reference<AppCUI::Controls::Control> control = nullptr;
    const PieceType type;
    bool isInitialPositionSet = false;

  public:
    Piece(const PieceType type, const AppCUI::Utils::Reference<AppCUI::Controls::Control> control, int x, int y);

    bool Draw(AppCUI::Graphics::Renderer& renderer, int scale, bool center, int w, int h);
    bool Draw(AppCUI::Graphics::Renderer& renderer, int scale);
    AppCUI::Graphics::Size GetSize(int scale) const;
    void UpdatePosition(int x, int y);
    void SetPosition(int x, int y);
    void SetInitialPosition(int x, int y);
    bool IsInitialPositionSet() const;
    int GetBlockWidth(int scale) const;
    int GetBlockHeight(int scale) const;
    int GetLeftXPosition() const;
    int GetRightXPosition(int scale) const;
    int GetTopYPosition() const;
    int GetBottomYPosition(int scale) const;
    bool TouchedTheBottom(int scale, int height) const;
};

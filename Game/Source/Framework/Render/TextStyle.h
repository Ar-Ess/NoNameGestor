#pragma once

#include "Framework/Render/Color.h"

struct TextStyle
{
    // Font size
    const unsigned int size = 16;

    // Normal Color
    const Color color = Color::White;

    // TTF_STYLE_BOLD
    const bool bold = false;

    // TTF_STYLE_ITALIC
    const bool italic = false;

    // TTF_STYLE_UNDERLINE
    const bool underline = false;

    // TTF_STYLE_STRIKETHROUGH
    const bool strikethrough = false;

    // Shadow
    const bool shade = false;
    const Color shadeColor = Color::Black;

    // Outline
    const unsigned int outline = 0;
    const Color outlineColor = Color::Black;

    // Wraping
    const float wrapWidth = 0;

    // Align
    const TextAlignment align = TextAlignment::LEFT;

    // Kerning
    const bool kerning = true;

};


#pragma once

#include "CommonTypes.h"

// defines font data
class GtaFontData final
{
public:
    GtaFontData() = default;

    // load font characters and palette from input stream
    bool LoadFromStream(std::istream& inStream);
    void Clear();

public:
    // defines info about single character of font
    struct CharData
    {
        std::vector<unsigned char> mBits; // bitmap raw data

        short mCharWidth = 0;
    };

    std::vector<CharData> mChars;
    Palette256 mPalette;

    short mCharsHeight = 0;
};
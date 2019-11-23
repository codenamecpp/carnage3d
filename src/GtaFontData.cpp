#include "stdafx.h"
#include "GtaFontData.h"

bool GtaFontData::LoadFromStream(std::istream& inStream)
{
    // read header
    unsigned char numChars = 0;
    READ_I8(inStream, numChars);

    unsigned char charsHeight = 0;
    READ_I8(inStream, charsHeight);

    // clear previous content
    Clear();

    mChars.resize(numChars);
    for (int iChar = 0; iChar < numChars; ++iChar)
    {
        char charWidth = 0;
        READ_I8(inStream, charWidth);

        int charDataLength = charWidth * charsHeight;
        mChars[iChar].mCharWidth = charWidth;
        mChars[iChar].mBits.resize(charDataLength);

        if (!inStream.read(reinterpret_cast<char*>(&mChars[iChar].mBits[0]), charDataLength))
        {
            debug_assert(false);
        }
    }

    // read palette entries
    for (int icolor = 0; icolor < CountOf(mPalette.mColors); ++icolor)
    {
        READ_I8(inStream, mPalette.mColors[icolor].mG);
        READ_I8(inStream, mPalette.mColors[icolor].mG);
        READ_I8(inStream, mPalette.mColors[icolor].mB);
        mPalette.mColors[icolor].mA = 0xFF;
    }
    return true;
}

void GtaFontData::Clear()
{
    mChars.clear();
    mCharsHeight = 0;
}
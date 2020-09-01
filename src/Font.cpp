#include "stdafx.h"
#include "Font.h"
#include "MemoryManager.h"
#include "GpuTexture2D.h"
#include "stb_rect_pack.h"

static const int MaxFontAtlasTextureSize = 2048;

static const unsigned char AnsiCharsOffsetTable[] =
{
     0,  1,  2, 0,  3,  0,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 0, 0,
    14, 15, 16, 0, 17,  0,  0, 18, 19, 20, 21,  0,  0, 22, 23, 24,
    25,  0, 26, 0, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36,  0,  0,
    37, 38, 39, 0, 40,  0,  0, 41, 42, 43, 44,
};

Font::Font(const std::string& fontName)
    : mFontName(fontName)
{  
}

Font::~Font()
{
    Unload();
}

bool Font::IsLoaded() const
{
    return mFontTexture != nullptr;
}

bool Font::LoadFromFile()
{
    if (IsLoaded())
        return true;

    std::ifstream inStream;
    if (!gFiles.OpenBinaryFile(mFontName, inStream))
        return false;

    // read header
    unsigned char numChars = 0;
    READ_I8(inStream, numChars);

    unsigned char charsHeight = 0;
    READ_I8(inStream, charsHeight);

    mLineHeight = charsHeight;
    mBaseCharCode = '!';

    mFontData.mRawCharacters.resize(numChars);
    for (int iChar = 0; iChar < numChars; ++iChar)
    {
        char charWidth = 0;
        READ_I8(inStream, charWidth);

        int charDataLength = charWidth * charsHeight;
        mFontData.mRawCharacters[iChar].mCharWidth = charWidth;
        mFontData.mRawCharacters[iChar].mCharData = new unsigned char[charDataLength];

        if (!inStream.read(reinterpret_cast<char*>(mFontData.mRawCharacters[iChar].mCharData), charDataLength))
        {
            debug_assert(false);
        }
    }

    // read palette entries
    for (int icolor = 0; icolor < CountOf(mFontData.mPalette.mColors); ++icolor)
    {
        READ_I8(inStream, mFontData.mPalette.mColors[icolor].mR);
        READ_I8(inStream, mFontData.mPalette.mColors[icolor].mG);
        READ_I8(inStream, mFontData.mPalette.mColors[icolor].mB);
        mFontData.mPalette.mColors[icolor].mA = 0xFF;
    }

    // create texture
    if (!CreateFontAtlas())
    {
        Unload();
        return false;
    }

    return true;
}

void Font::Unload()
{
    mLineHeight = 0;
    mBaseCharCode = 0;

    if (mFontTexture)
    {
        gGraphicsDevice.DestroyTexture(mFontTexture);
        mFontTexture = nullptr;
    }

    for (RawCharacter& currCharacter: mFontData.mRawCharacters)
    {
        delete [] currCharacter.mCharData;
    }
    mFontData.mRawCharacters.clear();
    mCharacters.clear();
    mFontData.mPalette.FillWithColor(0);
}

void Font::DumpCharacters(const std::string& outputPath)
{
    cxx::ensure_path_exists(outputPath);

    int icurrent = 0;
    for (const RawCharacter& currChar: mFontData.mRawCharacters)
    {
        PixelsArray spriteBitmap;
        if (!spriteBitmap.Create(eTextureFormat_RGB8, currChar.mCharWidth, mLineHeight, gMemoryManager.mFrameHeapAllocator))
        {
            debug_assert(false);
            continue;
        }

        for (int iy = 0; iy < mLineHeight; ++iy)
        for (int ix = 0; ix < currChar.mCharWidth; ++ix)
        {
            unsigned char palindex = currChar.mCharData[iy * currChar.mCharWidth + ix];

            int bitmapoffset = (iy * currChar.mCharWidth * 3) + (ix * 3);
            spriteBitmap.mData[bitmapoffset + 0] = mFontData.mPalette.mColors[palindex].mR;
            spriteBitmap.mData[bitmapoffset + 1] = mFontData.mPalette.mColors[palindex].mG;
            spriteBitmap.mData[bitmapoffset + 2] = mFontData.mPalette.mColors[palindex].mB;
        }

        std::string pathBuffer = cxx::va("%s/%d.png", outputPath.c_str(), icurrent);
        if (!spriteBitmap.SaveToFile(pathBuffer))
        {
            debug_assert(false);
            break;
        }
        ++icurrent;
    }
}

int Font::GetLineHeight() const
{
    return mLineHeight;
}

void Font::MeasureString(const std::string& text, Point& outputSize) const
{
    outputSize.x = 0;
    outputSize.y = 0;

    int maxCharCodes = (int) mCharacters.size();
    if (maxCharCodes < 1)
        return;

    int linesCounter = 0;
    int charsCounter = 0;
    int currLineWidth = 0;

    for (unsigned char currChar: text)
    {
        if (currChar == '\n')
        {
            charsCounter = 0;
            currLineWidth = 0;
            ++linesCounter;
            continue;
        }

        if (currChar == ' ')
        {
            ++charsCounter;
            currLineWidth += mFontData.mRawCharacters[0].mCharWidth;
            continue;
        }

        unsigned char charIndex = 0;
        if (currChar >= 0xC0)
        {
            if (currChar > 0xFC)
                continue;

            unsigned char baseoffset = ('z' - mBaseCharCode) + 6;
            currChar = (currChar - 0xC0);
            charIndex = baseoffset + AnsiCharsOffsetTable[currChar];
        }
        else
        {
            if (currChar < mBaseCharCode)
                continue;

            charIndex = currChar - mBaseCharCode;
        }

        if (charIndex >= maxCharCodes)
            continue;

        const RawCharacter& charData = mFontData.mRawCharacters[charIndex];
        currLineWidth += charData.mCharWidth;

        if (outputSize.x < currLineWidth)
        {
            outputSize.x = currLineWidth;
        }

        ++charsCounter;
    }

    if (charsCounter > 0)
    {
        ++linesCounter;   
    }

    outputSize.y = linesCounter * mLineHeight;
}

void Font::SetFontBaseCharCode(int charCode)
{
    debug_assert(charCode >= 0);
    mBaseCharCode = charCode;
}

void Font::DrawString(GuiContext& guiContext, const std::string& text, const Point& position, int paletteIndex)
{
    int maxCharCodes = (int) mCharacters.size();
    if (maxCharCodes < 1)
        return;

    Sprite2D spriteData;
    spriteData.mTexture = mFontTexture;
    spriteData.mScale = 1.0f;
    spriteData.mPaletteIndex = paletteIndex;
    spriteData.mOriginMode = Sprite2D::eOriginMode_TopLeft;

    int currentOffsetX = position.x;
    int currentOffsetY = position.y;
    for (unsigned char currChar: text)
    {
        if (currChar == '\n')
        {
            currentOffsetX = position.x;
            currentOffsetY += mLineHeight;
            continue;
        }

        if (currChar == ' ')
        {
            currentOffsetX += mCharacters[0].mRectangle.w;
            continue;
        }

        unsigned char charIndex = 0;
        if (currChar >= 0xC0)
        {
            if (currChar > 0xFC)
                continue;

            unsigned char baseoffset = ('z' - mBaseCharCode) + 6;
            currChar = (currChar - 0xC0);
            charIndex = baseoffset + AnsiCharsOffsetTable[currChar];
        }
        else
        {
            if (currChar < mBaseCharCode)
                continue;

            charIndex = currChar - mBaseCharCode;
        }

        if (charIndex >= maxCharCodes)
            continue;

        spriteData.mTextureRegion = mCharacters[charIndex];
        spriteData.mPosition.x = currentOffsetX * 1.0f;
        spriteData.mPosition.y = currentOffsetY * 1.0f;
        currentOffsetX += spriteData.mTextureRegion.mRectangle.w;

        guiContext.mSpriteBatch.DrawSprite(spriteData);
    }
}

bool Font::CreateFontAtlas()
{
    const int numCharacters = (int) mFontData.mRawCharacters.size();
    mCharacters.resize(numCharacters);

    // detect total layers count
    std::vector<stbrp_node> stbrp_nodes(MaxFontAtlasTextureSize);
    std::vector<stbrp_rect> stbrp_rects(numCharacters);

    for (int icharacter = 0; icharacter < numCharacters; ++icharacter)
    {
        stbrp_rects[icharacter].id = icharacter;
        stbrp_rects[icharacter].w = mFontData.mRawCharacters[icharacter].mCharWidth;
        stbrp_rects[icharacter].h = mLineHeight;
        stbrp_rects[icharacter].was_packed = 0;
    }

    int currentTextureSizeW = 128;
    int currentTextureSizeH = 128;
    int nn = 0;

    for (bool isPacked = false; !isPacked; ++nn)
    {
        // reset status
        for (stbrp_rect& rc: stbrp_rects)
        {
            rc.was_packed = 0;
        }

		stbrp_context context;
		stbrp_init_target(&context, currentTextureSizeW, currentTextureSizeH, stbrp_nodes.data(), stbrp_nodes.size());
		isPacked = stbrp_pack_rects(&context, stbrp_rects.data(), stbrp_rects.size()) > 0;
        if (!isPacked)
        {
            if (nn & 1) 
            {
				currentTextureSizeH <<= 1; // increase size
			} 
            else 
            {
				currentTextureSizeW <<= 1; // increase size
			}

			if (currentTextureSizeW > MaxFontAtlasTextureSize || currentTextureSizeH > MaxFontAtlasTextureSize) 
				return false;
        }
    }

    float tcx = 1.0f / currentTextureSizeW;
    float tcy = 1.0f / currentTextureSizeH;

    // setup texture coords
    for (stbrp_rect& rc: stbrp_rects)
    {
        mCharacters[rc.id].mRectangle.Set(rc.x, rc.y, rc.w, rc.h);
        mCharacters[rc.id].mU0 = rc.x * tcx;
        mCharacters[rc.id].mV0 = rc.y * tcy;
        mCharacters[rc.id].mU1 = (rc.x + rc.w) * tcx;
        mCharacters[rc.id].mV1 = (rc.y + rc.h) * tcy;
    }

    // allocate temporary bitmap
    PixelsArray charactersBitmap;
    if (!charactersBitmap.Create(eTextureFormat_R8UI, currentTextureSizeW, currentTextureSizeH, gMemoryManager.mFrameHeapAllocator))
        return false;

    charactersBitmap.FillWithColor(0);

    for (int icharacter = 0; icharacter < numCharacters; ++icharacter)
    {
        const RawCharacter& rawChar = mFontData.mRawCharacters[icharacter];

        // copy source image line by line
        for (int iy = 0; iy < mLineHeight; ++iy)
        {
            const void* source = rawChar.mCharData + (iy * rawChar.mCharWidth);
            void* dest = charactersBitmap.mData + (((mCharacters[icharacter].mRectangle.y + iy) * currentTextureSizeW) +
                (mCharacters[icharacter].mRectangle.x));

            ::memcpy(dest, source, rawChar.mCharWidth);
        }
    }

    // upload texture
    mFontTexture = gGraphicsDevice.CreateTexture2D(charactersBitmap.mFormat, charactersBitmap.mSizex, charactersBitmap.mSizey, charactersBitmap.mData);
    if (mFontTexture == nullptr)
    {
        debug_assert(false);
        return false;
    }
    return true;
}

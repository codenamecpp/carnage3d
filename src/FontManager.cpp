#include "stdafx.h"
#include "FontManager.h"

FontManager gFontManager;

bool FontManager::Initialize()
{
    return true;
}

void FontManager::Cleanup()
{
    for (auto& currFont: mFontsCache)
    {
        delete currFont.second;
    }
    mFontsCache.clear();
}

Font* FontManager::GetFont(const std::string& fontName)
{
    auto find_iterator = mFontsCache.find(fontName);
    if (find_iterator != mFontsCache.end())
        return find_iterator->second;

    // cache miss
    Font* fontInstance = LoadFontFromFile(fontName);
    if (fontInstance == nullptr)
    {
        return nullptr;
    }

    mFontsCache[fontName] = fontInstance;
    return fontInstance;
}

Font* FontManager::LoadFontFromFile(const std::string& fontName)
{
    std::ifstream inputStream;
    if (!gFiles.OpenBinaryFile(fontName, inputStream))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot find font file '%s'", fontName.c_str());
        return nullptr;
    }

    Font* fontInstance = new Font;
    if (!fontInstance->LoadFromStream(inputStream))
    {
        SafeDelete(fontInstance);
        gConsole.LogMessage(eLogMessage_Warning, "Cannot load font '%s'", fontName.c_str());
    }
    return fontInstance;
}

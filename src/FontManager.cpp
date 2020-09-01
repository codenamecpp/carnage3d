#include "stdafx.h"
#include "FontManager.h"

FontManager gFontManager;

bool FontManager::Initialize()
{
    return true;
}

void FontManager::Deinit()
{
    for (auto& currFont: mFontsCache)
    {
        delete currFont.second;
    }
    mFontsCache.clear();
}

Font* FontManager::GetFont(const std::string& fontName)
{
    Font* fontInstance = nullptr;

    auto find_iterator = mFontsCache.find(fontName);
    if (find_iterator != mFontsCache.end())
    {
        fontInstance = find_iterator->second;
    }

    if (fontInstance == nullptr) // cache miss
    {
        fontInstance = new Font(fontName);
        mFontsCache[fontName] = fontInstance;
    }

    debug_assert(fontInstance);
    if (!fontInstance->IsLoaded())
    {
        if (!fontInstance->LoadFromFile())
        {
            gConsole.LogMessage(eLogMessage_Warning, "Cannot load font '%s'", fontName.c_str());
        }
    }

    return fontInstance;
}

void FontManager::FlushAllFonts()
{
    for (auto& currFont: mFontsCache)
    {
        currFont.second->Unload();
    }
}

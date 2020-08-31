#pragma once

#include "Font.h"

// This class implements caching mechanism for font resources
class FontManager final: public cxx::noncopyable
{
public:
    // Initialize font manager internal resources
    bool Initialize();

    // Flush all currently cached fonts
    void Cleanup();

    // Finds font instance in the cache or loads it from a file
    // @param fontName: Font name
    Font* GetFont(const std::string& fontName);

private:
    Font* LoadFontFromFile(const std::string& fontName);

private:
    std::map<std::string, Font*> mFontsCache;
};

extern FontManager gFontManager;
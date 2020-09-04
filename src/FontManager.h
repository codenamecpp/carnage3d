#pragma once

#include "GuiDefs.h"

// This class implements caching mechanism for font resources
class FontManager final: public cxx::noncopyable
{
public:
    // Initialize font manager internal resources
    bool Initialize();

    // Flush all currently cached fonts
    void Deinit();

    // Flush all currently loaded fonts
    void FlushAllFonts();

    // Finds font instance within cache and force it to load
    // @param fontName: Font name
    // @returns font instance which might be not loaded in case of error
    Font* GetFont(const std::string& fontName);

private:
    std::map<std::string, Font*> mFontsCache;
};

extern FontManager gFontManager;
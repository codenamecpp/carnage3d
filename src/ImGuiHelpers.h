#pragma once

#include "imgui.h"

namespace ImGui
{
    inline void HorzSpacing(float spacingSize = 10.0f)
    {
        Dummy(ImVec2(0.0f, spacingSize));
    }
    inline void VertSpacing(float spacingSize = 10.0f)
    {
        Dummy(ImVec2(spacingSize, 0.0f)); 
    }
}
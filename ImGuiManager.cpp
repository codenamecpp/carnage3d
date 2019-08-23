#include "stdafx.h"
#include "ImGuiManager.h"
#include "RenderSystem.h"
#include "GpuProgram.h"
#include "GpuBuffer.h"

ImGuiManager gImGuiManager;

// imgui specific data size constants
const unsigned int Sizeof_ImGuiVertex = sizeof(ImDrawVert);
const unsigned int Sizeof_ImGuiIndex = sizeof(ImDrawIdx);

bool ImGuiManager::Initialize()
{
    // allocate buffers
    mVertexBuffer = gGraphicsDevice.CreateBuffer();
    mIndexBuffer = gGraphicsDevice.CreateBuffer();

    // initialize imgui context
    IMGUI_CHECKVERSION();

    ImGui::CreateContext();
    ImGui::StyleColorsLight();

    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr; // disable saving state
    io.LogFilename = nullptr; // disable saving log

    io.BackendRendererName          = "imgui_impl_opengl3";
    io.BackendFlags                 = ImGuiBackendFlags_HasMouseCursors | ImGuiBackendFlags_HasSetMousePos;
    io.ConfigFlags                  = ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableSetMousePos;
    io.KeyMap[ImGuiKey_Tab]         = KEYCODE_TAB;
    io.KeyMap[ImGuiKey_LeftArrow]   = KEYCODE_LEFT;
    io.KeyMap[ImGuiKey_RightArrow]  = KEYCODE_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow]     = KEYCODE_UP;
    io.KeyMap[ImGuiKey_DownArrow]   = KEYCODE_DOWN;
    io.KeyMap[ImGuiKey_PageUp]      = KEYCODE_PAGE_UP;
    io.KeyMap[ImGuiKey_PageDown]    = KEYCODE_PAGE_DOWN;
    io.KeyMap[ImGuiKey_Home]        = KEYCODE_HOME;
    io.KeyMap[ImGuiKey_End]         = KEYCODE_END;
    io.KeyMap[ImGuiKey_Insert]      = KEYCODE_INSERT;
    io.KeyMap[ImGuiKey_Delete]      = KEYCODE_DELETE;
    io.KeyMap[ImGuiKey_Backspace]   = KEYCODE_BACKSPACE;
    io.KeyMap[ImGuiKey_Space]       = KEYCODE_SPACE;
    io.KeyMap[ImGuiKey_Enter]       = KEYCODE_ENTER;
    io.KeyMap[ImGuiKey_Escape]      = KEYCODE_ENTER;
    io.KeyMap[ImGuiKey_A]           = KEYCODE_A;
    io.KeyMap[ImGuiKey_C]           = KEYCODE_C;
    io.KeyMap[ImGuiKey_V]           = KEYCODE_V;
    io.KeyMap[ImGuiKey_X]           = KEYCODE_X;
    io.KeyMap[ImGuiKey_Y]           = KEYCODE_Y;
    io.KeyMap[ImGuiKey_Z]           = KEYCODE_Z;

    int iWidth, iHeight;
    unsigned char *pcPixels;
    io.Fonts->GetTexDataAsRGBA32(&pcPixels, &iWidth, &iHeight);

    GpuTexture2D* fontTexture = gGraphicsDevice.CreateTexture2D(eTextureFormat_RGBA8, iWidth, iHeight, pcPixels);
    debug_assert(fontTexture);

    io.Fonts->TexID = fontTexture;
    io.MouseDrawCursor = true;
    return true;
}

void ImGuiManager::Deinit()
{
    ImGuiIO& io = ImGui::GetIO();

    // destroy font texture
    GpuTexture2D* fontTexture = static_cast<GpuTexture2D*>(io.Fonts->TexID);
    if (fontTexture)
    {
        gGraphicsDevice.DestroyTexture2D(fontTexture);
        io.Fonts->TexID = nullptr;
    }

    ImGui::DestroyContext();

    if (mVertexBuffer)
    {
        gGraphicsDevice.DestroyBuffer(mVertexBuffer);
        mVertexBuffer = nullptr;
    }
    
    if (mIndexBuffer)
    {
        gGraphicsDevice.DestroyBuffer(mIndexBuffer);
        mIndexBuffer = nullptr;
    }
}
 
void ImGuiManager::RenderFrame()
{
    ImGui::EndFrame();
    ImGui::Render();

    ImDrawData* imGuiDrawData = ImGui::GetDrawData();

    // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
    int fb_width = (int)(imGuiDrawData->DisplaySize.x * imGuiDrawData->FramebufferScale.x);
    int fb_height = (int)(imGuiDrawData->DisplaySize.y * imGuiDrawData->FramebufferScale.y);
    if (fb_width <= 0 || fb_height <= 0)
        return;

    RenderStates imguiRenderStates = RenderStates()
        .Disable(RenderStateFlags_FaceCulling)
        .Disable(RenderStateFlags_DepthTest)
        .SetAlphaBlend(eBlendMode_Alpha);

    gGraphicsDevice.SetRenderStates(imguiRenderStates);

    Rect2D imguiViewportRect { 
        static_cast<int>(imGuiDrawData->DisplayPos.x), static_cast<int>(imGuiDrawData->DisplayPos.y),
        static_cast<int>(imGuiDrawData->DisplaySize.x), static_cast<int>(imGuiDrawData->DisplaySize.y) 
    };
    gGraphicsDevice.SetViewportRect(imguiViewportRect);

    // todo : enable scissors

    // compute ortho matrix
    glm::mat4 projmatrix = glm::ortho(imGuiDrawData->DisplayPos.x, imGuiDrawData->DisplayPos.x + imGuiDrawData->DisplaySize.x, 
        imGuiDrawData->DisplayPos.y + imGuiDrawData->DisplaySize.y, imGuiDrawData->DisplayPos.y);

    gRenderSystem.mGuiTexColorProgram.Activate();
    gRenderSystem.mGuiTexColorProgram.mGpuProgram->SetUniform(eRenderUniform_ViewProjectionMatrix, projmatrix);

    glEnable(GL_SCISSOR_TEST);

    ImVec2 clip_off = imGuiDrawData->DisplayPos;         // (0,0) unless using multi-viewports
    ImVec2 clip_scale = imGuiDrawData->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

    // imgui primitives rendering
    for (int iCommandList = 0; iCommandList < imGuiDrawData->CmdListsCount; ++iCommandList)
    {
        const ImDrawList* cmd_list = imGuiDrawData->CmdLists[iCommandList];

         // vertex buffer generated by Dear ImGui
        mVertexBuffer->Setup(eBufferContent_Vertices, eBufferUsage_Dynamic, Sizeof_ImGuiVertex * cmd_list->VtxBuffer.Size, cmd_list->VtxBuffer.Data);
        gGraphicsDevice.BindVertexBuffer(mVertexBuffer, Vertex2D_Format::Get());

        // index buffer generated by Dear ImGui
        mIndexBuffer->Setup(eBufferContent_Indices, eBufferUsage_Dynamic, Sizeof_ImGuiIndex * cmd_list->IdxBuffer.Size, cmd_list->IdxBuffer.Data);
        gGraphicsDevice.BindIndexBuffer(mIndexBuffer);

        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; ++cmd_i)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback)
            {
                pcmd->UserCallback(cmd_list, pcmd);
                continue;
            }

            // Project scissor/clipping rectangles into framebuffer space
            ImVec4 clip_rect;
            clip_rect.x = (pcmd->ClipRect.x - clip_off.x) * clip_scale.x;
            clip_rect.y = (pcmd->ClipRect.y - clip_off.y) * clip_scale.y;
            clip_rect.z = (pcmd->ClipRect.z - clip_off.x) * clip_scale.x;
            clip_rect.w = (pcmd->ClipRect.w - clip_off.y) * clip_scale.y;

            bool should_draw = (clip_rect.x < fb_width && clip_rect.y < fb_height && clip_rect.z >= 0.0f && clip_rect.w >= 0.0f);

            if (!should_draw)
                continue;

            glScissor((int)clip_rect.x, (int)(fb_height - clip_rect.w), (int)(clip_rect.z - clip_rect.x), (int)(clip_rect.w - clip_rect.y));

            // The texture for the draw call is specified by pcmd->TextureId.
            // The vast majority of draw calls will use the Dear ImGui texture atlas, which value you have set yourself during initialization.
            GpuTexture2D* bindTexture = static_cast<GpuTexture2D*>(pcmd->TextureId);
            gGraphicsDevice.BindTexture2D(eTextureUnit_0, bindTexture);

            // We are using scissoring to clip some objects. All low-level graphics API should supports it.
            // - If your engine doesn't support scissoring yet, you may ignore this at first. You will get some small glitches
            //   (some elements visible outside their bounds) but you can fix that once everything else works!
            // - Clipping coordinates are provided in imgui coordinates space (from draw_data->DisplayPos to draw_data->DisplayPos + draw_data->DisplaySize)
            //   In a single viewport application, draw_data->DisplayPos will always be (0,0) and draw_data->DisplaySize will always be == io.DisplaySize.
            //   However, in the interest of supporting multi-viewport applications in the future (see 'viewport' branch on github),
            //   always subtract draw_data->DisplayPos from clipping bounds to convert them to your viewport space.
            // - Note that pcmd->ClipRect contains Min+Max bounds. Some graphics API may use Min+Max, other may use Min+Size (size being Max-Min)

            //ImVec2 pos = imGuiDrawData->DisplayPos;
            //MyEngineScissor((int)(pcmd->ClipRect.x - pos.x), (int)(pcmd->ClipRect.y - pos.y), (int)(pcmd->ClipRect.z - pos.x), (int)(pcmd->ClipRect.w - pos.y));

            // Render 'pcmd->ElemCount/3' indexed triangles.
            // By default the indices ImDrawIdx are 16-bits, you can change them to 32-bits in imconfig.h if your engine doesn't support 16-bits indices.

            eIndicesType indicesType = Sizeof_ImGuiIndex == 2 ? eIndicesType_i16 : eIndicesType_i32;
            gGraphicsDevice.RenderIndexedPrimitives(ePrimitiveType_Triangles, indicesType, Sizeof_ImGuiIndex * pcmd->IdxOffset, pcmd->ElemCount);
        }
    }
}

void ImGuiManager::UpdateFrame(Timespan deltaTime)
{
    ImGuiIO& io = ImGui::GetIO();

    io.DeltaTime = deltaTime.ToSeconds();   // set the time elapsed since the previous frame (in seconds)
    io.DisplaySize.x = gGraphicsDevice.mViewportRect.w * 1.0f;
    io.DisplaySize.y = gGraphicsDevice.mViewportRect.h * 1.0f;
    io.MousePos.x = gInputs.mCursorPositionX * 1.0f;
    io.MousePos.y = gInputs.mCursorPositionY * 1.0f;
    io.MouseDown[0] = gInputs.GetMouseButtonL();  // set the mouse button states
    io.MouseDown[1] = gInputs.GetMouseButtonR();
    io.MouseDown[3] = gInputs.GetMouseButtonM();

    ImGui::NewFrame();

    // todo: process all imgui windows

    ImGui::ShowDemoWindow();
    ImGui::ShowUserGuide();
}

void ImGuiManager::HandleEvent(MouseMovedInputEvent& inputEvent)
{
}

void ImGuiManager::HandleEvent(MouseScrollInputEvent& inputEvent)
{
    ImGuiIO& io = ImGui::GetIO();

    io.MouseWheelH += inputEvent.mScrollX * 1.0f;
    io.MouseWheel += inputEvent.mScrollY * 1.0f;
}

void ImGuiManager::HandleEvent(MouseButtonInputEvent& inputEvent)
{
}

void ImGuiManager::HandleEvent(KeyInputEvent& inputEvent)
{
    ImGuiIO& io = ImGui::GetIO();

    io.KeysDown[inputEvent.mKeycode] = inputEvent.mPressed;
    io.KeyCtrl = inputEvent.HasMods(KEYMOD_CTRL);
    io.KeyShift = inputEvent.HasMods(KEYMOD_SHIFT);
    io.KeyAlt = inputEvent.HasMods(KEYMOD_ALT);
}

void ImGuiManager::HandleEvent(KeyCharEvent& inputEvent)
{
    ImGuiIO& io = ImGui::GetIO();

    io.AddInputCharacter(inputEvent.mUnicodeChar);
}
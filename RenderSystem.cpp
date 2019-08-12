#include "stdafx.h"
#include "RenderSystem.h"

RenderSystem gRenderSystem;

bool RenderSystem::Initialize()
{
    LoadRenderPrograms();
    return true;
}

void RenderSystem::Deinit()
{
    FreeRenderPrograms();
}

void RenderSystem::RenderFrame()
{
    gGraphicsDevice.ClearScreen();

    // todo

    gGraphicsDevice.Present();
}

void RenderSystem::FreeRenderPrograms()
{

}

void RenderSystem::LoadRenderPrograms()
{

}

void RenderSystem::ReloadRenderPrograms()
{

}

#include "stdafx.h"
#include "CityRenderer.h"
#include "RenderSystem.h"

bool CityRenderer::Initialize()
{
    return true;
}

void CityRenderer::Deinit()
{
}

void CityRenderer::RenderFrame()
{
    gRenderSystem.mCityMeshProgram.Activate();
    gRenderSystem.mCityMeshProgram.UploadCameraTransformMatrices();

    gRenderSystem.mCityMeshProgram.Deactivate();
}
#pragma once

// renders static city mesh
class CityRenderer final: public cxx::noncopyable
{
public:
    // first time renderer initialization
    // Return false on error
    bool Initialize();

    // renderer finalization
    void Deinit();

    // render game frame routine
    void RenderFrame();

};
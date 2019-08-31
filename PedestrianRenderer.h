#pragma once

// renders pedestrians on the level
class PedestrianRenderer final: public cxx::noncopyable
{
public:
    bool Initialize();
    void Deinit();
    void RenderFrame();

private:
};

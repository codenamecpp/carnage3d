#pragma once

class TrimeshBuffer final: public cxx::noncopyable
{
public:
    TrimeshBuffer() = default;
    ~TrimeshBuffer();

    void SetVertices(unsigned int dataLength, const void* dataSource);
    void SetIndices(unsigned int dataLength, const void* dataSource);
    void Bind(const VertexFormat& vertexFormat);
    void Deinit();

public:
    GpuBuffer* mVertexBuffer = nullptr;
    GpuBuffer* mIndexBuffer = nullptr;
};
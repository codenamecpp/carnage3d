#pragma once

// renders map mesh, peds, cars and map objects
class CityRenderer final: public cxx::noncopyable
{
public:
    CityRenderer();

    bool Initialize();
    void Deinit();

    void RenderFrame();
    void InvalidateMapMesh();

private:
    // internals
    void BuildMapMesh();
    void CommitCityMeshData();

    void RenderFrameBegin();
    void RenderFrameEnd();

    void DrawCityMesh();
    void IssuePedsSprites();
    void IssueCarsSprites();
    void IssueMapObjectsSprites();
    void IssueProjectilesSprites();

    // @param sprRotate: Angle in radians
    void DrawSprite3D(GpuTexture2D* texture, const Rect2D& rcTexture, const glm::vec3& position, bool centerOrigin, float sprScale, float sprRotate);
    void DrawSprite2D(GpuTexture2D* texture, const Rect2D& rcTexture, const glm::vec2& position, bool centerOrigin, float sprScale, float sprRotate);

    void SortDrawSpritesList();
    void GenerateDrawSpritesBatches();
    void RenderDrawSpritesBatches();

private:

    // drawing operation for single sprite
    struct DrawSpriteRec
    {
        glm::vec2 mPosition;
        glm::vec2 mSize;
        glm::vec2 mCenterOffset;
        glm::vec2 mTcUv0;
        glm::vec2 mTcUv1;
        float mRotate; // rotate in radians
        float mDepth; // z coord
        GpuTexture2D* mSpriteTexture;
    };

    // single batch of drawing sprites
    struct DrawSpriteBatch
    {
        unsigned int mFirstVertex;
        unsigned int mFirstIndex;
        unsigned int mVertexCount;
        unsigned int mIndexCount;
        GpuTexture2D* mSpriteTexture;
    };

    std::vector<SpriteVertex3D> mDrawSpritesVertices;
    std::vector<DrawIndex_t> mDrawSpritesIndices;

    std::vector<DrawSpriteRec> mDrawSpritesList;
    std::list<DrawSpriteBatch> mDrawSpritesBatchesList;

    StreamingVertexCache mSpritesVertexCache; 

    Rect2D mCityMapRectangle; // cached map area in tiles

    MapMeshData mCityMeshData[MAP_LAYERS_COUNT];
    GpuBuffer* mCityMeshBufferV;
    GpuBuffer* mCityMeshBufferI;
};
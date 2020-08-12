#include "stdafx.h"
#include "GameMapManager.h"

GameMapManager gGameMap;

enum
{
    GTA_CMPFILE_VERSION_CODE = 331,
};

struct GTAFileHeaderCMP
{
    int version_code;
    union {
        struct {
            char style_number;
            char sample_number;
            char reserved_1;
            char reserved_2;
        };
        int _numbers;
    };
    int route_size;
    int object_pos_size;
    int column_size;
    int block_size;
    int nav_data_size;
};

bool GameMapManager::LoadFromFile(const std::string& filename)
{
    Cleanup();

    gConsole.LogMessage(eLogMessage_Info, "Loading map '%s'", filename.c_str());

    std::ifstream file;
    if (!gFiles.OpenBinaryFile(filename, file))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot open map data file '%s'", filename.c_str());
        return false;
    }

    GTAFileHeaderCMP header;
    if (!cxx::read_from_stream(file, header) || header.version_code != GTA_CMPFILE_VERSION_CODE)
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot read header of map data file '%s'", filename.c_str());
        return false;
    }

    if (!ReadCompressedMapData(file, header.column_size, header.block_size))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot read compressed map data from '%s'", filename.c_str());
        return false;
    }

    if (!ReadStartupObjects(file, header.object_pos_size))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot read map startup objects from '%s'", filename.c_str());
        return false;
    }

    // load corresponding style data
    std::string styleName = cxx::va("STYLE%03d.G24", header.style_number);
    if (!mStyleData.LoadFromFile(styleName))
    {
        Cleanup();
        return false;
    }
    return true;
}

void GameMapManager::Cleanup()
{
    mStyleData.Cleanup();
    for (int tiley = 0; tiley < MAP_DIMENSIONS; ++tiley)
    for (int tilex = 0; tilex < MAP_DIMENSIONS; ++tilex)
    {
        for (int tilez = 0; tilez < MAP_LAYERS_COUNT; ++tilez)
        {
            memset(&mMapTiles[tilez][tiley][tilex], 0, Sizeof_BlockStyle);
        }
    }
    mStartupObjects.clear();
}

bool GameMapManager::IsLoaded() const
{
    return mStyleData.IsLoaded();
}

bool GameMapManager::ReadCompressedMapData(std::ifstream& file, int columnLength, int blocksLength)
{
    // reading base data
    const int baseDataLength = MAP_DIMENSIONS * MAP_DIMENSIONS * sizeof(int);
    if (!file.read(reinterpret_cast<char*>(mBaseTilesData), baseDataLength))
        return false;

    // reading column data
    std::vector<unsigned short> columnData;
    if (columnLength)
    {
        assert((columnLength % sizeof(unsigned short)) == 0);
        columnData.resize(columnLength / sizeof(unsigned short));

        if (!file.read(reinterpret_cast<char*>(columnData.data()), columnLength))
            return false;
    }

    std::vector<BlockStyle> blocksData;

    const int blockSize = sizeof(unsigned short) + sizeof(unsigned char) * 6;
    if (blocksLength)
    {
        assert((blocksLength % blockSize) == 0);
        blocksData.resize(blocksLength / blockSize);

        for (BlockStyle& blockInfo: blocksData)
        {
            unsigned short type_map;
            READ_I16(file, type_map);

            blockInfo.mUpDirection = (type_map & 0x01) > 0;
            blockInfo.mDownDirection = (type_map & 0x02) > 0;
            blockInfo.mLeftDirection = (type_map & 0x04) > 0;
            blockInfo.mRightDirection = (type_map & 0x08) > 0;
            blockInfo.mGroundType = static_cast<eGroundType>((type_map >> 4) & 0x07);
            blockInfo.mIsFlat = (type_map & 0x80) > 0;
            blockInfo.mSlopeType = (type_map >> 8) & 0x3F;
            blockInfo.mLidRotation = static_cast<eLidRotation>((type_map >> 14) & 0x03);

            unsigned char type_map_ext;
            READ_I8(file, type_map_ext);

            blockInfo.mTrafficLight = (type_map_ext & 0x07);
            blockInfo.mRemap = (type_map_ext >> 3) & 0x03;
            blockInfo.mFlipTopBottomFaces = (type_map_ext & 0x20) > 0;
            blockInfo.mFlipLeftRightFaces = (type_map_ext & 0x40) > 0;
            blockInfo.mIsRailway = (type_map_ext & 0x80) > 0;

            // read sides
            READ_I8(file, blockInfo.mFaces[eBlockFace_W]);
            READ_I8(file, blockInfo.mFaces[eBlockFace_E]);
            READ_I8(file, blockInfo.mFaces[eBlockFace_N]);
            READ_I8(file, blockInfo.mFaces[eBlockFace_S]);
            READ_I8(file, blockInfo.mFaces[eBlockFace_Lid]);
        }
    }

    // decompress

    for (int tiley = 0; tiley < MAP_DIMENSIONS; ++tiley)
    for (int tilex = 0; tilex < MAP_DIMENSIONS; ++tilex)
    {
        const int columnElement = mBaseTilesData[tiley][tilex] / sizeof(unsigned short);
        assert((mBaseTilesData[tiley][tilex] % sizeof(unsigned short)) == 0);
        const int columnHeight = MAP_LAYERS_COUNT - columnData[columnElement];
        for (int tilez = 0; tilez < columnHeight; ++tilez)
        {
            int srcBlock = columnData[columnElement + columnHeight - tilez];
            mMapTiles[tilez][tiley][tilex] = blocksData[srcBlock];
        }
    }
    //FixShiftedBits();
    return true;
}

BlockStyle* GameMapManager::GetBlock(int coordx, int coordy, int layer) const
{
    debug_assert(layer > -1 && layer < MAP_LAYERS_COUNT);
    debug_assert(coordx > -1 && coordx < MAP_DIMENSIONS);
    debug_assert(coordy > -1 && coordy < MAP_DIMENSIONS);
    // remember kids, don't try this at home!
    return const_cast<BlockStyle*> (&mMapTiles[layer][coordy][coordx]);
}

BlockStyle* GameMapManager::GetBlockClamp(int coordx, int coordy, int layer) const
{
    layer = glm::clamp(layer, 0, MAP_LAYERS_COUNT - 1);
    coordx = glm::clamp(coordx, 0, MAP_DIMENSIONS - 1);
    coordy = glm::clamp(coordy, 0, MAP_DIMENSIONS - 1);
    // remember kids, don't try this at home!
    return const_cast<BlockStyle*> (&mMapTiles[layer][coordy][coordx]);
}

void GameMapManager::FixShiftedBits()
{
    // as CityScape Data Structure document says:

    // The road, water, field, pavement, direction, railway & traffic light bits are set in the
    // block above the one which actually stores the graphic for the feature. This means that the very top layer
    // cannot be used for road, water, pavement, etc.

    // so we have fix that

    // one thing to keep in mind -
    // slopes are still stored in block above since they used for mesh generation

    for (int tiley = 0; tiley < MAP_DIMENSIONS; ++tiley)
    for (int tilex = 0; tilex < MAP_DIMENSIONS; ++tilex)
    {
        for (int tilez = 0; tilez < MAP_LAYERS_COUNT - 2; ++tilez)
        {
            BlockStyle& currBlock = mMapTiles[tilez][tiley][tilex];
            BlockStyle& aboveBlock = mMapTiles[tilez + 1][tiley][tilex];

            currBlock.mLeftDirection = aboveBlock.mLeftDirection;
            currBlock.mRightDirection = aboveBlock.mRightDirection;
            currBlock.mDownDirection = aboveBlock.mDownDirection;
            currBlock.mUpDirection = aboveBlock.mUpDirection;
            currBlock.mGroundType = aboveBlock.mGroundType;
            currBlock.mTrafficLight = aboveBlock.mTrafficLight;
        }

        // top most block set to air
        BlockStyle& topBlock = mMapTiles[MAP_LAYERS_COUNT - 1][tiley][tilex];
        topBlock.mLeftDirection = 0;
        topBlock.mRightDirection = 0;
        topBlock.mDownDirection = 0;
        topBlock.mUpDirection = 0;
        topBlock.mGroundType = eGroundType_Air;
        topBlock.mTrafficLight = 0;
    }
}

float GameMapManager::GetHeightAtPosition(const glm::vec3& position, bool excludeWater) const
{
    int mapcoordx = (int) position.x;
    int mapcoordy = (int) position.z;
    int maplayer = (int) (position.y + 0.5f);

    float height = maplayer * 1.0f; // reset height to ground

    for (;height > 0.0f;)
    {
        BlockStyle* blockData = GetBlockClamp(mapcoordx, mapcoordy, maplayer);

        // slope
        int slope = blockData->mSlopeType;

        if (slope) // compute slope height
        {
            int cx = ConvertMapToPixels(position.x - mapcoordx);
            int cy = ConvertMapToPixels(position.z - mapcoordy);

            int pix_height = GameMapHelpers::GetSlopeHeight(slope, cx, cy);
            height += ConvertPixelsToMap(pix_height);
            break;
        }

        if (blockData->mGroundType == eGroundType_Air || (blockData->mGroundType == eGroundType_Water && excludeWater)) // fall through non solid block
        {
            height -= MAP_BLOCK_LENGTH;
            --maplayer;
            continue;
        }

        break;
    }
    return height;
}

bool GameMapManager::TraceSegment2D(const glm::vec2& origin, const glm::vec2& destination, float height, glm::vec2& outPoint)
{
    glm::ivec2 mapcoord_start = origin;
    glm::ivec2 mapcoord_end = destination;

    int mapcoord_z = (int) height;

    glm::vec2 direction = glm::normalize(destination - origin);

    // find all cells intersecting with line

    float posX = origin.x;
    float posY = origin.y;

    glm::ivec2 mapcoord_curr = mapcoord_start;

    float sideDistX;
    float sideDistY;

    //length of ray from one x or y-side to next x or y-side
    float deltaDistX = std::abs(1.0f / direction.x);
    float deltaDistY = std::abs(1.0f / direction.y);

    //what direction to step in x or y-direction (either +1 or -1)
    int stepX;
    int stepY;

    int side; //was a NS or a EW wall hit?

    //calculate step and initial sideDist
    if (direction.x < 0.0f)
    {
        stepX = -1;
        sideDistX = (posX - mapcoord_curr.x) * deltaDistX;
    }
    else
    {
        stepX = 1;
        sideDistX = (mapcoord_curr.x + 1.0f - posX) * deltaDistX;
    }

    if (direction.y < 0.0f)
    {
        stepY = -1;
        sideDistY = (posY - mapcoord_curr.y) * deltaDistY;
    }
    else
    {
        stepY = 1;
        sideDistY = (mapcoord_curr.y + 1.0f - posY) * deltaDistY;
    }

    //perform DDA
    const int MaxSteps = 16;
    for (int istep = 0; ; ++istep)
    {
        if (istep == MaxSteps)
            return false;

        //jump to next map square, OR in x-direction, OR in y-direction
        if (sideDistX < sideDistY)
        {
            sideDistX += deltaDistX;
            mapcoord_curr.x += stepX;
            side = 0;
        }
        else
        {
            sideDistY += deltaDistY;
            mapcoord_curr.y += stepY;
            side = 1;
        }

        // detect hit
        BlockStyle* blockData = GetBlockClamp(mapcoord_curr.x, mapcoord_curr.y, mapcoord_z);
        if (blockData->mGroundType == eGroundType_Building)
        {
            float perpWallDist;
            if (side == 0) perpWallDist = (mapcoord_curr.x - posX + (1 - stepX) / 2) / direction.x;
            else           perpWallDist = (mapcoord_curr.y - posY + (1 - stepY) / 2) / direction.y;

            outPoint = (origin + direction * perpWallDist);
            return true;
        }

        if (mapcoord_curr == mapcoord_end)
            break;
    }

    return false;
}

bool GameMapManager::ReadStartupObjects(std::ifstream& file, int dataSize)
{
    const unsigned int RecordSize = 14;
    debug_assert(dataSize % RecordSize == 0);

    int numRecords = dataSize / RecordSize;

    mStartupObjects.resize(numRecords);
    for (StartupObjectPosStruct& currRecord: mStartupObjects)
    {
        READ_I16(file, currRecord.mX);
        READ_I16(file, currRecord.mY);
        READ_I16(file, currRecord.mZ);

        READ_I8(file, currRecord.mType);
        READ_I8(file, currRecord.mRemap);

        READ_I16(file, currRecord.mRotation);
        READ_I16(file, currRecord.mPitch);
        READ_I16(file, currRecord.mRoll);
    }

    // remove duplicates -
    // objects list contains a number of identical car records for some reason
    // so it better get rid of them
    std::set<StartupObjectPosStruct> uniqueObjects {mStartupObjects.begin(), mStartupObjects.end()};
    mStartupObjects.assign(uniqueObjects.begin(), uniqueObjects.end());
    return true;
}

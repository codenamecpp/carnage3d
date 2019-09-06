#include "stdafx.h"
#include "CityMapManager.h"

CityMapManager gMapManager;

//////////////////////////////////////////////////////////////////////////

template<typename TValue>
inline bool read_from_stream(std::ifstream& filestream, TValue& outputValue)
{
    if (!filestream.read(reinterpret_cast<char*>(&outputValue), sizeof(outputValue)))
        return false;

    return true;
}

// helpers
#define READ_DATA(filestream, destination, datatype) \
    { \
        datatype _$data; \
        if (!read_from_stream(filestream, _$data)) \
            return false; \
        \
        destination = _$data; \
    }

#define READ_I8(filestream, destination) READ_DATA(filestream, destination, unsigned char)
#define READ_I16(filestream, destination) READ_DATA(filestream, destination, unsigned short)
#define READ_I32(filestream, destination) READ_DATA(filestream, destination, int)
#define READ_BOOL(filestream, destination) \
    { \
        unsigned char _$data; \
        if (!read_from_stream(filestream, _$data)) \
            return false; \
        \
        destination = _$data > 0; \
    }

//////////////////////////////////////////////////////////////////////////

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

bool CityMapManager::LoadFromFile(const char* filename)
{
    Cleanup();

    std::ifstream file;
    if (!gFiles.OpenBinaryFile(filename, file))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot open map data file '%s'", filename);
        return false;
    }

    GTAFileHeaderCMP header;
    if (!read_from_stream(file, header) || header.version_code != GTA_CMPFILE_VERSION_CODE)
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot read header of map data file '%s'", filename);
        return false;
    }

    if (!ReadCompressedMapData(file, header.column_size, header.block_size))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot read compressed map data from '%s'", filename);
        return false;
    }

    // load corresponding style data
    char styleName[16];
    snprintf(styleName, CountOf(styleName), "style%03d.g24", header.style_number);

    if (!mStyleData.LoadFromFile(styleName))
    {
        Cleanup();
        return false;
    }
    return true;
}

void CityMapManager::Cleanup()
{
    mStyleData.Cleanup();
    for (int tiley = 0; tiley < MAP_DIMENSIONS; ++tiley)
    for (int tilex = 0; tilex < MAP_DIMENSIONS; ++tilex)
    {
        for (int tilez = 0; tilez < MAP_LAYERS_COUNT; ++tilez)
        {
            memset(&mMapTiles[tilez][tiley][tilex], 0, Sizeof_BlockStyleData);
        }
    }
}

bool CityMapManager::IsLoaded() const
{
    return mStyleData.IsLoaded();
}

bool CityMapManager::ReadCompressedMapData(std::ifstream& file, int columnLength, int blocksLength)
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

    std::vector<BlockStyleData> blocksData;

    const int blockSize = sizeof(unsigned short) + sizeof(unsigned char) * 6;
    if (blocksLength)
    {
        assert((blocksLength % blockSize) == 0);
        blocksData.resize(blocksLength / blockSize);

        for (BlockStyleData& blockInfo: blocksData)
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
    FixShiftedBits();
    return true;
}

BlockStyleData* CityMapManager::GetBlock(int tilex, int tiley, int tilez)
{
    return &mMapTiles[tilez][tiley][tilex];
}

BlockStyleData* CityMapManager::GetBlockClamp(int tilex, int tiley, int tilez)
{
    tilex = glm::clamp(tilex, 0, MAP_DIMENSIONS - 1);
    tiley = glm::clamp(tiley, 0, MAP_DIMENSIONS - 1);
    tilez = glm::clamp(tilez, 0, MAP_LAYERS_COUNT - 1);

    return &mMapTiles[tilez][tiley][tilex];
}

void CityMapManager::FixShiftedBits()
{
    // as CityScape Data Structure document says:

    // The road, water, field, pavement, direction, railway & traffic light bits are set in the
    // block above the one which actually stores the graphic for the feature. This means that the very top layer
    // cannot be used for road, water, pavement, etc.

    // so we have fix that

    for (int tiley = 0; tiley < MAP_DIMENSIONS; ++tiley)
    for (int tilex = 0; tilex < MAP_DIMENSIONS; ++tilex)
    {
        for (int tilez = 0; tilez < MAP_LAYERS_COUNT - 2; ++tilez)
        {
            BlockStyleData& currBlock = mMapTiles[tilez][tiley][tilex];
            BlockStyleData& aboveBlock = mMapTiles[tilez + 1][tiley][tilex];

            currBlock.mLeftDirection = aboveBlock.mLeftDirection;
            currBlock.mRightDirection = aboveBlock.mRightDirection;
            currBlock.mDownDirection = aboveBlock.mDownDirection;
            currBlock.mUpDirection = aboveBlock.mUpDirection;
            currBlock.mGroundType = aboveBlock.mGroundType;
            currBlock.mTrafficLight = aboveBlock.mTrafficLight;
        }

        // top most block set to air
        BlockStyleData& topBlock = mMapTiles[MAP_LAYERS_COUNT - 1][tiley][tilex];
        topBlock.mLeftDirection = 0;
        topBlock.mRightDirection = 0;
        topBlock.mDownDirection = 0;
        topBlock.mUpDirection = 0;
        topBlock.mGroundType = eGroundType_Air;
        topBlock.mTrafficLight = 0;
    }
}

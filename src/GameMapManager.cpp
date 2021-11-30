#include "stdafx.h"
#include "GameMapManager.h"
#include "CarnageGame.h"
#include "cvars.h"

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

    gConsole.LogMessage(eLogMessage_Info, "Loading map data '%s'", filename.c_str());

    std::ifstream file;
    if (!gFiles.OpenBinaryFile(filename, file))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot open map data file");
        return false;
    }

    GTAFileHeaderCMP header;
    if (!cxx::read_from_stream(file, header) || header.version_code != GTA_CMPFILE_VERSION_CODE)
    {
        gConsole.LogMessage(eLogMessage_Warning, "Invalid map file header");
        return false;
    }

    if (!ReadCompressedMapData(file, header.column_size, header.block_size))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot read compressed map data");
        return false;
    }

    if (!ReadStartupObjects(file, header.object_pos_size))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot read startup objects data");
        return false;
    }

    if (!ReadRoutes(file, header.route_size))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot read routes data");
        return false;
    }

    if (!ReadServiceBaseLocations(file))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot read service base locations data");
        return false;
    }

    if (!ReadNavData(file, header.nav_data_size))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot read nav data");
        return false;
    }

    // load corresponding style data
    std::string styleName = GetStyleFileName(header.style_number);

    gConsole.LogMessage(eLogMessage_Info, "Loading style data '%s'", styleName.c_str());
    if (!mStyleData.LoadFromFile(styleName))
    {
        Cleanup();
        return false;
    }
    mStyleFileNumber = header.style_number;
    mAudioFileNumber = header.style_number; // sample_number is always 0 for some reason
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
            memset(&mMapTiles[tilez][tiley][tilex], 0, Sizeof_BlockInfo);
        }
    }
    mStartupObjects.clear();
    for (int ibase = 0; ibase < eAccidentServise_COUNT; ++ibase)
    {
        mAccidentServicesBases[ibase].clear();
    }
    mStyleFileNumber = 0;
    mAudioFileNumber = 0;
}

bool GameMapManager::IsLoaded() const
{
    return mStyleData.IsLoaded();
}

bool GameMapManager::ReadCompressedMapData(std::istream& file, int columnLength, int blocksLength)
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

    std::vector<MapBlockInfo> blocksData;

    const int blockSize = sizeof(unsigned short) + sizeof(unsigned char) * 6;
    if (blocksLength)
    {
        assert((blocksLength % blockSize) == 0);
        blocksData.resize(blocksLength / blockSize);

        for (MapBlockInfo& blockInfo: blocksData)
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

            switch (type_map_ext & 0x07)
            {
                case 0: blockInfo.mTrafficHint = eTrafficHint_None; break;
                case 1: blockInfo.mTrafficHint = eTrafficHint_TrafficLights; break;
                case 2:
                case 3: 
                    debug_assert(false);
                break;
                case 4: blockInfo.mTrafficHint = eTrafficHint_TrainTurnEnd; break;
                case 5: blockInfo.mTrafficHint = eTrafficHint_TrainTurnStart; break;
                case 6: blockInfo.mTrafficHint = eTrafficHint_TrainStationEnd; break;
                case 7: blockInfo.mTrafficHint = eTrafficHint_TrainStationStart; break;
            };
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

const MapBlockInfo* GameMapManager::GetBlockInfo(int coordx, int coordz, int layer) const
{
    layer = glm::clamp(layer, 0, MAP_LAYERS_COUNT - 1);
    coordx = glm::clamp(coordx, 0, MAP_DIMENSIONS - 1);
    coordz = glm::clamp(coordz, 0, MAP_DIMENSIONS - 1);

    return &mMapTiles[layer][coordz][coordx];
}

const MapBlockInfo* GameMapManager::GetNeighbourBlock(int coordx, int coordz, int layer, eMapDirection2D dir) const
{
    int neighbour_coordx = coordx;
    int neighbour_coordz = coordz;
    int neighbour_layer = layer;

    switch (dir)
    {
    // straight
        case eMapDirection2D_N: neighbour_coordz -= 1; break;
        case eMapDirection2D_E: neighbour_coordx += 1; break;
        case eMapDirection2D_S: neighbour_coordz += 1; break;
        case eMapDirection2D_W: neighbour_coordx -= 1; break;
    // diagonals
        case eMapDirection2D_NE:
            neighbour_coordz -= 1;
            neighbour_coordx += 1;
        break;
        case eMapDirection2D_NW:
            neighbour_coordz -= 1;
            neighbour_coordx -= 1;
        break;
        case eMapDirection2D_SE:
            neighbour_coordz += 1;
            neighbour_coordx += 1;
        break;
        case eMapDirection2D_SW:
            neighbour_coordz += 1;
            neighbour_coordx -= 1;
        break;
    }
    return GetBlockInfo(neighbour_coordx, neighbour_coordz, neighbour_layer);
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
            MapBlockInfo& currBlock = mMapTiles[tilez][tiley][tilex];
            MapBlockInfo& aboveBlock = mMapTiles[tilez + 1][tiley][tilex];

            currBlock.mLeftDirection = aboveBlock.mLeftDirection;
            currBlock.mRightDirection = aboveBlock.mRightDirection;
            currBlock.mDownDirection = aboveBlock.mDownDirection;
            currBlock.mUpDirection = aboveBlock.mUpDirection;
            currBlock.mGroundType = aboveBlock.mGroundType;
            currBlock.mTrafficHint = aboveBlock.mTrafficHint;
        }

        // top most block set to air
        MapBlockInfo& topBlock = mMapTiles[MAP_LAYERS_COUNT - 1][tiley][tilex];
        topBlock.mLeftDirection = 0;
        topBlock.mRightDirection = 0;
        topBlock.mDownDirection = 0;
        topBlock.mUpDirection = 0;
        topBlock.mGroundType = eGroundType_Air;
        topBlock.mTrafficHint = eTrafficHint_None;;
    }
}

float GameMapManager::GetWaterLevelAtPosition2(const glm::vec2& position) const
{
    glm::ivec2 blockPosition = Convert::MetersToMapUnits(position);

    for (int i = MAP_LAYERS_COUNT; i > 0; --i)
    {
        const MapBlockInfo* blockData = GetBlockInfo(blockPosition.x, blockPosition.y, i - 1);
        if (blockData->mGroundType == eGroundType_Water)
        {
            float waterHeight = Convert::MapUnitsToMeters(i - 1.0f);
            return waterHeight;
        }
    }
    return 0.0f;
}

const DistrictInfo* GameMapManager::GetDistrictAtPosition2(const glm::vec2& position) const
{
    glm::ivec2 blockPosition = Convert::MetersToMapUnits(position);

    return GetDistrict(blockPosition.x, blockPosition.y);
}

const DistrictInfo* GameMapManager::GetDistrict(int coordx, int coordy) const
{
    const Point point (coordx, coordy);
    for (const DistrictInfo& currDistrict: mDistricts)
    {
        if (currDistrict.mArea.PointWithin(point))
            return &currDistrict;
    }
    debug_assert(false); // shouldn't happen
    return nullptr;
}

const DistrictInfo* GameMapManager::GetDistrictByIndex(int districtIndex) const
{
    for (const DistrictInfo& currInfo: mDistricts)
    {
        if (currInfo.mSampleIndex == districtIndex)
            return &currInfo;
    }
    debug_assert(false);
    return nullptr;
}

float GameMapManager::GetHeightAtPosition(const glm::vec3& position, bool excludeWater) const
{
    // get map block position in which we are located
    glm::ivec3 mapBlock = Convert::MetersToMapUnits(position);

    float currentHeight = (float) mapBlock.y; // set current height to ground, map units
    for (; currentHeight > 0.0f;)
    {
        const MapBlockInfo* blockData = GetBlockInfo(mapBlock.x, mapBlock.z, mapBlock.y); // y is map layer

        // compute slope height
        if (blockData->mSlopeType) 
        {
            // subposition within block
            float cx = Convert::MetersToMapUnits(position.x) - mapBlock.x;
            float cy = Convert::MetersToMapUnits(position.z) - mapBlock.z;

            currentHeight += GameMapHelpers::GetSlopeHeight(blockData->mSlopeType, cx, cy);

            break;
        }

        if (blockData->mGroundType == eGroundType_Air || (blockData->mGroundType == eGroundType_Water && excludeWater)) // fall through non solid block
        {
            currentHeight -= 1.0f;
            mapBlock.y -= 1;
            continue;
        }

        break; // bail out
    }
    return Convert::MapUnitsToMeters(currentHeight);
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
        const MapBlockInfo* blockData = GetBlockInfo(mapcoord_curr.x, mapcoord_curr.y, mapcoord_z);
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

bool GameMapManager::ReadStartupObjects(std::istream& file, int dataSize)
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

bool GameMapManager::ReadRoutes(std::istream& file, int dataSize)
{
    file.seekg(dataSize, std::ios::cur);
    return true;
}

bool GameMapManager::ReadServiceBaseLocations(std::ifstream& file)
{
    struct LocationData
    {
        unsigned char x;
        unsigned char y;
        unsigned char z;
    };
    const int LocationDataSize = sizeof(LocationData);
    const int MaxLocations = 6;

    struct AllLocationData
    {
        LocationData police[MaxLocations];
        LocationData hospitals[MaxLocations];
        LocationData unused1[MaxLocations];
        LocationData unused2[MaxLocations];
        LocationData fireStations[MaxLocations];
        LocationData unused3[MaxLocations];
    };
    AllLocationData locations;

    const int DataSize = sizeof(locations);
    static_assert(DataSize == (MaxLocations * 6 * LocationDataSize), "Invalid locations data size");

    if (!file.read((char*)&locations, DataSize))
    {
        debug_assert(false);
        return false;
    }

    // police stations
    for (int i = 0; i < MaxLocations; ++i)
    {   
        if ((locations.police[i].x) || (locations.police[i].y) || (locations.police[i].z))
        {
            mAccidentServicesBases[eAccidentServise_PoliceStation].push_back({
                locations.police[i].x, 
                INVERT_MAP_LAYER(locations.police[i].z), 
                locations.police[i].y});
        }
    }

    // hospitals
    for (int i = 0; i < MaxLocations; ++i)
    {
        if ((locations.hospitals[i].x) || (locations.hospitals[i].y) || (locations.hospitals[i].z))
        {
            mAccidentServicesBases[eAccidentServise_Hospital].push_back({
                locations.hospitals[i].x, 
                INVERT_MAP_LAYER(locations.hospitals[i].z), 
                locations.hospitals[i].y});
        }
    }

    // fire stations
    for (int i = 0; i < MaxLocations; ++i)
    {
        if ((locations.fireStations[i].x) || (locations.fireStations[i].y) || (locations.fireStations[i].z))
        {
            mAccidentServicesBases[eAccidentServise_FireStation].push_back({
                locations.fireStations[i].x, 
                INVERT_MAP_LAYER(locations.fireStations[i].z), 
                locations.fireStations[i].y});
        }
    }

    return true;
}

bool GameMapManager::ReadNavData(std::ifstream& file, int dataSize)
{
    struct nav_data_struct
    {
        unsigned char x, y;
        unsigned char w, h;
        unsigned char sam;
        char name[30];
    };
    const int NavDataStructSize = sizeof(nav_data_struct);

    mDistricts.clear();

    int sectorsCount = dataSize / NavDataStructSize;
    debug_assert((dataSize % NavDataStructSize) == 0);

    nav_data_struct navDataStruct;
    for (int i = 0; i < sectorsCount; ++i)
    {
        if (!cxx::read_from_stream(file, navDataStruct))
        {
            debug_assert(false);
            return false;
        }

        if (navDataStruct.w == 0 || navDataStruct.h == 0)
            continue;

        mDistricts.emplace_back();
        DistrictInfo& navSector = mDistricts.back();
        navSector.mArea.x = navDataStruct.x;
        navSector.mArea.y = navDataStruct.y;
        navSector.mArea.w = navDataStruct.w;
        navSector.mArea.h = navDataStruct.h;
        navSector.mSampleIndex = navDataStruct.sam;
        navSector.mDebugName = navDataStruct.name;
    }

    // sort by size
    std::sort(mDistricts.begin(), mDistricts.end(),
        [](const DistrictInfo& lhs, const DistrictInfo& rhs)
        {
            if (lhs.mArea.w != rhs.mArea.w)
                return (lhs.mArea.w < rhs.mArea.w);

            return (lhs.mArea.h < rhs.mArea.h);
        });
    return true;
}

std::string GameMapManager::GetStyleFileName(int styleNumber) const
{
    if (gCvarGameVersion.mValue == eGtaGameVersion_MissionPack2_London61)
    {
        std::string styleName = cxx::va("Sty%03d.g24", styleNumber);
        return styleName;
    }

    if (gCvarGameVersion.mValue == eGtaGameVersion_MissionPack1_London69)
    {
        std::string styleName = cxx::va("Style%03d.g24", styleNumber);
        return styleName;
    }

    debug_assert((gCvarGameVersion.mValue == eGtaGameVersion_Demo) || (gCvarGameVersion.mValue == eGtaGameVersion_Full));
    
    std::string styleName = cxx::va("STYLE%03d.G24", styleNumber);
    return styleName;
}

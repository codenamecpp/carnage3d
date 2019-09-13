#include "stdafx.h"
#include "PhysicsManager.h"
#include "GameMapData.h"

PhysicsManager gPhysics;

bool PhysicsManager::RaycastMapWall(const glm::vec3& startPoint, const glm::vec3& endPoint, glm::vec3& outPoint)
{
    MapCoord startMapPos = startPoint;
    MapCoord endMapPos = endPoint;

    if (startMapPos == endMapPos)
        return false;

    if (startMapPos.z != endMapPos.z)
    {
        debug_assert(false);
    }

    glm::vec3 direction = glm::normalize(endPoint - startPoint);

    // find all cells intersecting with line

    float posX = startPoint.x;
    float posY = startPoint.y;

    MapCoord currMapPos = startMapPos;

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
        sideDistX = (posX - currMapPos.x) * deltaDistX;
    }
    else
    {
        stepX = 1;
        sideDistX = (currMapPos.x + 1.0f - posX) * deltaDistX;
    }

    if (direction.y < 0.0f)
    {
        stepY = -1;
        sideDistY = (posY - currMapPos.y) * deltaDistY;
    }
    else
    {
        stepY = 1;
        sideDistY = (currMapPos.y + 1.0f - posY) * deltaDistY;
    }

    //perform DDA
    const int MaxSteps = 16;
    for (int istep = 0; ; ++istep)
    {
        if (istep == MaxSteps)
        {
            debug_assert(false);
            return false;
        }

        //jump to next map square, OR in x-direction, OR in y-direction
        if (sideDistX < sideDistY)
        {
            sideDistX += deltaDistX;
            currMapPos.x += stepX;
            side = 0;
        }
        else
        {
            sideDistY += deltaDistY;
            currMapPos.y += stepY;
            side = 1;
        }

        // detect hit
        BlockStyleData* blockData = gGameMap.GetBlockClamp(currMapPos);
        if (blockData->mGroundType == eGroundType_Building)
        {
            float perpWallDist;
            if (side == 0) perpWallDist = (currMapPos.x - posX + (1 - stepX) / 2) / direction.x;
            else           perpWallDist = (currMapPos.y - posY + (1 - stepY) / 2) / direction.y;

            outPoint = (startPoint + direction * perpWallDist);
            return true;
        }

        if (currMapPos == endMapPos)
            break;
    }

    return false;
}

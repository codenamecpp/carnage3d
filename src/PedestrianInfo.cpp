#include "stdafx.h"
#include "PedestrianInfo.h"

void PedestrianInfo::AddFears(PedestrianFearFlags fearFlags)
{
    mFearFlags = mFearFlags | fearFlags;
}

void PedestrianInfo::RemoveFears(PedestrianFearFlags fearFlags)
{
    mFearFlags = (mFearFlags & ~fearFlags);
}

bool PedestrianInfo::SetupFromConfg(cxx::json_document_node configNode)
{
    Clear();

    if (!cxx::json_get_attribute(configNode, "ped_type", mTypeID))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Unknown pedestrian type ID");
        return false;
    }

    if (cxx::json_node_string remapTypeNode = configNode["remap_type"])
    {
        std::string remapTypeString = remapTypeNode.get_value();
        if (remapTypeString == "random_civilian")
        {
            mRemapType = ePedestrianRemapType_RandomCivilian;
        }
    }
    cxx::json_get_attribute(configNode, "remap_index", mRemapIndex);

    // read fears
    if (cxx::json_node_array fearsNode = configNode["fears"])
    {
        for (cxx::json_node_string currFear = fearsNode.first_child();
            currFear; currFear = currFear.next_sibling())
        {
            std::string fearName = currFear.get_value();

            if (fearName == "players")
            {
                AddFears(PedestrianFearFlags_Players);
                continue;
            }

            if (fearName == "police")
            {
                AddFears(PedestrianFearFlags_Police);
                continue;
            }

            if (fearName == "gunShots")
            {
                AddFears(PedestrianFearFlags_GunShots);
                continue;
            }

            if (fearName == "explosions")
            {
                AddFears(PedestrianFearFlags_Explosions);
                continue;
            }

            if (fearName == "deadPeds")
            {
                AddFears(PedestrianFearFlags_DeadPeds);
                continue;
            }

            debug_assert(false);
            gConsole.LogMessage(eLogMessage_Warning, "Unknown pedestrian fear flag '%s'", fearName.c_str());
        }
    }

    return true;
}

void PedestrianInfo::Clear()
{
    mTypeID = ePedestrianType_Civilian;
    mFearFlags = PedestrianFearFlags_None;
    mRemapIndex = NO_REMAP;
    mRemapType = ePedestrianRemapType_Index;
}